/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2019 SugarBombEngine Developers

This file is part of SugarBombEngine

SugarBombEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SugarBombEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SugarBombEngine. If not, see <http://www.gnu.org/licenses/>.

In addition, SugarBombEngine is using id Tech 4 (BFG) pieces and thus
subject to certain additional terms (all header and source files which 
contains such pieces has this additional part appended to the license 
header). You should have received a copy of these additional terms 
stated in a separate file (LICENSE-idTech4) which accompanied the 
SugarBombEngine source code. If not, please request a copy in 
writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
Suite 120, Rockville, Maryland 20850 USA.

*******************************************************************************
*/

/// @file

// DG: SoundSample seems to be XAudio-independent, so this is just a copy of idSoundSample_XAudio2

#include "idlib/precompiled.h"

#include "CoreLibs/SbSystem/IFile.hpp"
#include "CoreLibs/SbSystem/IFileSystem.hpp"

#include "framework/CVar.hpp"

#include "SbSoundStub.hpp"
#include "SbSoundSystem.hpp"
#include "snd_local.h"

namespace sbe::SbSound
{

extern idCVar s_useCompression;
extern idCVar s_noSound;

#define GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( x ) x

const uint32 SOUND_MAGIC_IDMSA = 0x6D7A7274;

extern idCVar sys_lang;

#define MIN_SAMPLE_RATE  1000	// Minimum audio sample rate supported (this value is from XAudio2)

/*
========================
AllocBuffer
========================
*/
static void* AllocBuffer( int size, const char* name )
{
	return Mem_Alloc( size, TAG_AUDIO );
}

/*
========================
FreeBuffer
========================
*/
static void FreeBuffer( void* p )
{
	return Mem_Free( p );
}

/*
========================
idSoundSample_XAudio2::idSoundSample_XAudio2
========================
*/
idSoundSample::idSoundSample(idSys *apSys, idFileSystem *apFileSystem) : mpSys(apSys), fileSystem(apFileSystem)
{
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	loaded = false;
	neverPurge = false;
	levelLoadReferenced = false;
	
	memset( &format, 0, sizeof( format ) );
	
	totalBufferSize = 0;
	
	playBegin = 0;
	playLength = 0;
	
	lastPlayedTime = 0;
}

/*
========================
idSoundSample::~idSoundSample
========================
*/
SbSoundSample::~SbSoundSample()
{
	FreeData();
}

/*
========================
idSoundSample::WriteGeneratedSample
========================
*/
void idSoundSample::WriteGeneratedSample( idFile* fileOut )
{
	fileOut->WriteBig( SOUND_MAGIC_IDMSA );
	fileOut->WriteBig( timestamp );
	fileOut->WriteBig( loaded );
	fileOut->WriteBig( playBegin );
	fileOut->WriteBig( playLength );
	SbWaveFile::WriteWaveFormatDirect( format, fileOut );
	fileOut->WriteBig( ( int )amplitude.Num() );
	fileOut->Write( amplitude.Ptr(), amplitude.Num() );
	fileOut->WriteBig( totalBufferSize );
	fileOut->WriteBig( ( int )buffers.Num() );
	for( int i = 0; i < buffers.Num(); i++ )
	{
		fileOut->WriteBig( buffers[ i ].numSamples );
		fileOut->WriteBig( buffers[ i ].bufferSize );
		fileOut->Write( buffers[ i ].buffer, buffers[ i ].bufferSize );
	};
}
/*
========================
idSoundSample::WriteAllSamples
========================
*/
void SbSoundSample::WriteAllSamples( const idStr& sampleName )
{
	SbSoundSample* samplePC = new SbSoundSample(mpSys, fileSystem);
	{
		idStrStatic< MAX_OSPATH > inName = sampleName;
		inName.Append( ".msadpcm" );
		idStrStatic< MAX_OSPATH > inName2 = sampleName;
		inName2.Append( ".wav" );
		
		idStrStatic< MAX_OSPATH > outName = "generated/";
		outName.Append( sampleName );
		outName.Append( ".idwav" );
		
		if( samplePC->LoadWav( inName ) || samplePC->LoadWav( inName2 ) )
		{
			idFile* fileOut = fileSystem->OpenFileWrite( outName, "fs_basepath" );
			samplePC->WriteGeneratedSample( fileOut );
			delete fileOut;
		}
	}
	delete samplePC;
}

/*
========================
idSoundSample::LoadGeneratedSound
========================
*/
bool SbSoundSample::LoadGeneratedSample( const idStr& filename )
{
	idFileLocal fileIn( fileSystem->OpenFileReadMemory( filename ) );
	if( fileIn != nullptr )
	{
		uint32 magic;
		fileIn->ReadBig( magic );
		fileIn->ReadBig( timestamp );
		fileIn->ReadBig( loaded );
		fileIn->ReadBig( playBegin );
		fileIn->ReadBig( playLength );
		SbWaveFile::ReadWaveFormatDirect( format, fileIn );
		int num;
		fileIn->ReadBig( num );
		amplitude.Clear();
		amplitude.SetNum( num );
		fileIn->Read( amplitude.Ptr(), amplitude.Num() );
		fileIn->ReadBig( totalBufferSize );
		fileIn->ReadBig( num );
		buffers.SetNum( num );
		for( int i = 0; i < num; i++ )
		{
			fileIn->ReadBig( buffers[ i ].numSamples );
			fileIn->ReadBig( buffers[ i ].bufferSize );
			buffers[ i ].buffer = AllocBuffer( buffers[ i ].bufferSize, GetName() );
			fileIn->Read( buffers[ i ].buffer, buffers[ i ].bufferSize );
			buffers[ i ].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[ i ].buffer );
		}
		return true;
	}
	return false;
}
/*
========================
idSoundSample::Load
========================
*/
void SbSoundSample::LoadResource()
{
	FreeData();
	
	if( idStr::Icmpn( GetName(), "_default", 8 ) == 0 )
	{
		MakeDefault();
		return;
	}
	
	if( s_noSound.GetBool() )
	{
		MakeDefault();
		return;
	}
	
	loaded = false;
	
	for( int i = 0; i < 2; i++ )
	{
		idStrStatic< MAX_OSPATH > sampleName = GetName();
		if( ( i == 0 ) && !sampleName.Replace( "/vo/", va( "/vo/%s/", sys_lang.GetString() ) ) )
		{
			i++;
		}
		idStrStatic< MAX_OSPATH > generatedName = "generated/";
		generatedName.Append( sampleName );
		
		{
			if( s_useCompression.GetBool() )
			{
				sampleName.Append( ".msadpcm" );
			}
			else
			{
				sampleName.Append( ".wav" );
			}
			generatedName.Append( ".idwav" );
		}
		loaded = LoadGeneratedSample( generatedName ) || LoadWav( sampleName );
		
		if( !loaded && s_useCompression.GetBool() )
		{
			sampleName.SetFileExtension( "wav" );
			loaded = LoadWav( sampleName );
		}
		
		if( loaded )
		{
			if( cvarSystem->GetCVarBool( "fs_buildresources" ) )
			{
				fileSystem->AddSamplePreload( GetName() );
				WriteAllSamples( GetName() );
				
				if( sampleName.Find( "/vo/" ) >= 0 )
				{
					for( int i = 0; i < mpSys->GetLangsNum(); i++ ) // TODO: GetLangCount?
					{
						const char* lang = mpSys->GetLangName( i ); // TODO: replace with sys_lang cvar?
						if( idStr::Icmp( lang, ID_LANG_ENGLISH ) == 0 )
						{
							continue;
						}
						idStrStatic< MAX_OSPATH > locName = GetName();
						locName.Replace( "/vo/", va( "/vo/%s/", mpSys->GetLangName( i ) ) ); // TODO: replace with sys_lang cvar?
						WriteAllSamples( locName );
					}
				}
			}
			return;
		}
	}
	
	if( !loaded )
	{
		// make it default if everything else fails
		MakeDefault();
	}
	return;
}

/*
========================
idSoundSample::LoadWav
========================
*/
bool SbSoundSample::LoadWav( const idStr& filename )
{

	// load the wave
	SbWaveFile wave;
	if( !wave.Open( fileSystem, filename ) )
	{
		return false;
	}
	
	idStrStatic< MAX_OSPATH > sampleName = filename;
	sampleName.SetFileExtension( "amp" );
	LoadAmplitude( sampleName );
	
	const char* formatError = wave.ReadWaveFormat( format );
	if( formatError != nullptr )
	{
		idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), formatError );
		MakeDefault();
		return false;
	}
	timestamp = wave.Timestamp();
	
	totalBufferSize = wave.SeekToChunk( 'data' );
	
	if( format.basic.formatTag == idWaveFile::FORMAT_PCM || format.basic.formatTag == idWaveFile::FORMAT_EXTENSIBLE )
	{
	
		if( format.basic.bitsPerSample != 16 )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "Not a 16 bit PCM wav file" );
			MakeDefault();
			return false;
		}
		
		playBegin = 0;
		playLength = ( totalBufferSize ) / format.basic.blockSize;
		
		buffers.SetNum( 1 );
		buffers[0].bufferSize = totalBufferSize;
		buffers[0].numSamples = playLength;
		buffers[0].buffer = AllocBuffer( totalBufferSize, GetName() );
		
		
		wave.Read( buffers[0].buffer, totalBufferSize );
		
		if( format.basic.bitsPerSample == 16 )
		{
			idSwap::LittleArray( ( short* )buffers[0].buffer, totalBufferSize / sizeof( short ) );
		}
		
		buffers[0].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[0].buffer );
		
	}
	else if( format.basic.formatTag == SbWaveFile::FORMAT_ADPCM )
	{
	
		playBegin = 0;
		playLength = ( ( totalBufferSize / format.basic.blockSize ) * format.extra.adpcm.samplesPerBlock );
		
		buffers.SetNum( 1 );
		buffers[0].bufferSize = totalBufferSize;
		buffers[0].numSamples = playLength;
		buffers[0].buffer  = AllocBuffer( totalBufferSize, GetName() );
		
		wave.Read( buffers[0].buffer, totalBufferSize );
		
		buffers[0].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[0].buffer );
		
	}
	else if( format.basic.formatTag == SbWaveFile::FORMAT_XMA2 )
	{
	
		if( format.extra.xma2.blockCount == 0 )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "No data blocks in file" );
			MakeDefault();
			return false;
		}
		
		int bytesPerBlock = format.extra.xma2.bytesPerBlock;
		assert( format.extra.xma2.blockCount == ALIGN( totalBufferSize, bytesPerBlock ) / bytesPerBlock );
		assert( format.extra.xma2.blockCount * bytesPerBlock >= totalBufferSize );
		assert( format.extra.xma2.blockCount * bytesPerBlock < totalBufferSize + bytesPerBlock );
		
		buffers.SetNum( format.extra.xma2.blockCount );
		for( int i = 0; i < buffers.Num(); i++ )
		{
			if( i == buffers.Num() - 1 )
			{
				buffers[i].bufferSize = totalBufferSize - ( i * bytesPerBlock );
			}
			else
			{
				buffers[i].bufferSize = bytesPerBlock;
			}
			
			buffers[i].buffer = AllocBuffer( buffers[i].bufferSize, GetName() );
			wave.Read( buffers[i].buffer, buffers[i].bufferSize );
			buffers[i].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[i].buffer );
		}
		
		int seekTableSize = wave.SeekToChunk( 'seek' );
		if( seekTableSize != 4 * buffers.Num() )
		{
			idLib::Warning( "LoadWav( %s ) : %s", filename.c_str(), "Wrong number of entries in seek table" );
			MakeDefault();
			return false;
		}
		
		for( int i = 0; i < buffers.Num(); i++ )
		{
			wave.Read( &buffers[i].numSamples, sizeof( buffers[i].numSamples ) );
			idSwap::Big( buffers[i].numSamples );
		}
		
		playBegin = format.extra.xma2.loopBegin;
		playLength = format.extra.xma2.loopLength;
		
		if( buffers[buffers.Num() - 1].numSamples < playBegin + playLength )
		{
			// This shouldn't happen, but it's not fatal if it does
			playLength = buffers[buffers.Num() - 1].numSamples - playBegin;
		}
		else
		{
			// Discard samples beyond playLength
			for( int i = 0; i < buffers.Num(); i++ )
			{
				if( buffers[i].numSamples > playBegin + playLength )
				{
					buffers[i].numSamples = playBegin + playLength;
					// Ideally, the following loop should always have 0 iterations because playBegin + playLength ends in the last block already
					// But there is no guarantee for that, so to be safe, discard all buffers beyond this one
					for( int j = i + 1; j < buffers.Num(); j++ )
					{
						FreeBuffer( buffers[j].buffer );
					}
					buffers.SetNum( i + 1 );
					break;
				}
			}
		}
		
	}
	else
	{
		idLib::Warning( "LoadWav( %s ) : Unsupported wave format %d", filename.c_str(), format.basic.formatTag );
		MakeDefault();
		return false;
	}
	
	wave.Close();
	
	if( format.basic.formatTag == SbWaveFile::FORMAT_EXTENSIBLE )
	{
		// HACK: XAudio2 doesn't really support FORMAT_EXTENSIBLE so we convert it to a basic format after extracting the channel mask
		format.basic.formatTag = format.extra.extensible.subFormat.data1;
	}
	
	// sanity check...
	assert( buffers[buffers.Num() - 1].numSamples == playBegin + playLength );
	
	return true;
}


/*
========================
idSoundSample::MakeDefault
========================
*/
void SbSoundSample::MakeDefault()
{
	FreeData();
	
	static const int DEFAULT_NUM_SAMPLES = 256;
	
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	loaded = true;
	
	memset( &format, 0, sizeof( format ) );
	format.basic.formatTag = SbWaveFile::FORMAT_PCM;
	format.basic.numChannels = 1;
	format.basic.bitsPerSample = 16;
	format.basic.samplesPerSec = MIN_SAMPLE_RATE;
	format.basic.blockSize = format.basic.numChannels * format.basic.bitsPerSample / 8;
	format.basic.avgBytesPerSec = format.basic.samplesPerSec * format.basic.blockSize;
	
	assert( format.basic.blockSize == 2 );
	
	totalBufferSize = DEFAULT_NUM_SAMPLES * 2;
	
	short* defaultBuffer = ( short* )AllocBuffer( totalBufferSize, GetName() );
	for( int i = 0; i < DEFAULT_NUM_SAMPLES; i += 2 )
	{
		defaultBuffer[i + 0] = SHRT_MIN;
		defaultBuffer[i + 1] = SHRT_MAX;
	}
	
	buffers.SetNum( 1 );
	buffers[0].buffer = defaultBuffer;
	buffers[0].bufferSize = totalBufferSize;
	buffers[0].numSamples = DEFAULT_NUM_SAMPLES;
	buffers[0].buffer = GPU_CONVERT_CPU_TO_CPU_CACHED_READONLY_ADDRESS( buffers[0].buffer );
	
	playBegin = 0;
	playLength = DEFAULT_NUM_SAMPLES;
}

/*
========================
idSoundSample::FreeData

Called before deleting the object and at the start of LoadResource()
========================
*/
void SbSoundSample::FreeData()
{
	if( buffers.Num() > 0 )
	{
		soundSystemLocal.StopVoicesWithSample( ( idSoundSample* )this );
		for( int i = 0; i < buffers.Num(); i++ )
		{
			FreeBuffer( buffers[i].buffer );
		}
		buffers.Clear();
	}
	amplitude.Clear();
	
	timestamp = FILE_NOT_FOUND_TIMESTAMP;
	memset( &format, 0, sizeof( format ) );
	loaded = false;
	totalBufferSize = 0;
	playBegin = 0;
	playLength = 0;
}

/*
========================
idSoundSample::LoadAmplitude
========================
*/
bool SbSoundSample::LoadAmplitude( const idStr& name )
{
	amplitude.Clear();
	idFileLocal f( fileSystem->OpenFileRead( name ) );
	if( f == nullptr )
	{
		return false;
	}
	amplitude.SetNum( f->Length() );
	f->Read( amplitude.Ptr(), amplitude.Num() );
	return true;
}

/*
========================
idSoundSample::GetAmplitude
========================
*/
float SbSoundSample::GetAmplitude( int timeMS ) const
{
	if( timeMS < 0 || timeMS > LengthInMsec() )
	{
		return 0.0f;
	}
	if( IsDefault() )
	{
		return 1.0f;
	}
	int index = timeMS * 60 / 1000;
	if( index < 0 || index >= amplitude.Num() )
	{
		return 0.0f;
	}
	return ( float )amplitude[index] / 255.0f;
}

}; // namespace sbe::SbSound