/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2018-2019 SugarBombEngine Developers

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

#pragma hdrstop

//#include <cstddef>
#include "precompiled.h"

#include "CoreLibs/SbSystem/ISystem.hpp"

#include "framework/CVar.hpp" // TODO: #include "framework/ICVarSystem.hpp"?
#include "framework/IDeclManager.hpp"

#include "idlib/Lexer.h"
#include "idlib/Str.h"
#include "idlib/Token.h"
#include "idlib/containers/List.h"
#include "idlib/containers/StrList.h"

#include "SbSoundShader.hpp"
#include "SbSoundDefines.hpp"
#include "SbSoundSystem.hpp" // TODO: soundSystemLocal
#include "SbSoundWorld.hpp"

/*
#if defined(SBE_USE_OPENAL)
#	include "openal/AL_SoundSample.h"
#else
#	include "xaudio2/XA2_SoundSample.h"
#endif
*/

namespace sbe::SbSound
{

extern idCVar s_maxSamples;

enum speakerLabel
{
	SPEAKER_LEFT = 0,
	SPEAKER_RIGHT,
	SPEAKER_CENTER,
	SPEAKER_LFE,
	SPEAKER_BACKLEFT,
	SPEAKER_BACKRIGHT
};

/*
===============
idSoundShader::Init
===============
*/
void SbSoundShader::Init()
{
	leadin = false;
	leadinVolume = 0;
	altSound = nullptr;
};

/*
===============
idSoundShader::idSoundShader
===============
*/
idSoundShader::idSoundShader(sbe::ISys *apSys)
    : mpSys(apSys)
{
	Init();
};

/*
===============
idSoundShader::~idSoundShader
===============
*/
SbSoundShader::~SbSoundShader() = default;

/*
=================
idSoundShader::Size
=================
*/
size_t SbSoundShader::Size() const
{
	return sizeof(SbSoundShader);
};

/*
===============
idSoundShader::idSoundShader::FreeData
===============
*/
void SbSoundShader::FreeData()
{
};

/*
===================
idSoundShader::SetDefaultText
===================
*/
bool SbSoundShader::SetDefaultText()
{
	idStr wavname = GetName();
	wavname.DefaultFileExtension(".wav"); // if the name has .ogg in it, that will stay

	// if there exists a wav file with the same name
	if(1) //fileSystem->ReadFile( wavname, nullptr ) != -1 ) {
	{
		char generated[2048];
		idStr::snPrintf(generated, sizeof(generated),
		                "sound %s // IMPLICITLY GENERATED\n"
		                "{\n"
		                "%s\n"
		                "}\n",
		                GetName(), wavname.c_str());
		SetText(generated);
		return true;
	}
	else
		return false;
};

/*
===================
DefaultDefinition
===================
*/
const char *SbSoundShader::DefaultDefinition() const
{
	return "{\n"
	       "\t"
	       "_default.wav\n"
	       "}";
};

/*
===============
idSoundShader::Parse

  this is called by the declManager
===============
*/
bool SbSoundShader::Parse(const char *text, const int textLength, bool allowBinaryVersion)
{
	if(soundSystemLocal.currentSoundWorld)
		soundSystemLocal.currentSoundWorld->WriteSoundShaderLoad(this);

	idLexer src;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	if(!ParseShader(nullptr, src)) // TODO
	{
		MakeDefault();
		return false;
	};
	return true;
};

/*
===============
idSoundShader::ParseShader
===============
*/
bool SbSoundShader::ParseShader(idDeclManager *declManager, idLexer &src)
{
	idToken token;

	parms.minDistance = 1;
	parms.maxDistance = 10;
	parms.volume = 1;
	parms.shakes = 0;
	parms.soundShaderFlags = 0;
	parms.soundClass = 0;

	speakerMask = 0;
	altSound = nullptr;

	entries.Clear();

	while(1)
	{
		if(!src.ExpectAnyToken(&token))
		{
			return false;
		}
		// end of definition
		else if(token == "}")
		{
			break;
		}
		// minimum number of sounds
		else if(!token.Icmp("minSamples"))
		{
			src.ParseInt();
		}
		// description
		else if(!token.Icmp("description"))
		{
			src.ReadTokenOnLine(&token);
		}
		// mindistance
		else if(!token.Icmp("mindistance"))
		{
			parms.minDistance = src.ParseFloat();
		}
		// maxdistance
		else if(!token.Icmp("maxdistance"))
		{
			parms.maxDistance = src.ParseFloat();
		}
		// shakes screen
		else if(!token.Icmp("shakes"))
		{
			src.ExpectAnyToken(&token);
			if(token.type == TT_NUMBER)
			{
				parms.shakes = token.GetFloatValue();
			}
			else
			{
				src.UnreadToken(&token);
				parms.shakes = 1.0f;
			}
		}
		// reverb
		else if(!token.Icmp("reverb"))
		{
			src.ParseFloat();
			if(!src.ExpectTokenString(","))
			{
				src.FreeSource();
				return false;
			}
			src.ParseFloat();
			// no longer supported
		}
		// volume
		else if(!token.Icmp("volume"))
		{
			parms.volume = src.ParseFloat();
		}
		// leadinVolume is used to allow light breaking leadin sounds to be much louder than the broken loop
		else if(!token.Icmp("leadinVolume"))
		{
			leadinVolume = src.ParseFloat();
			leadin = true;
		}
		// speaker mask
		else if(!token.Icmp("mask_center"))
		{
			speakerMask |= 1 << SPEAKER_CENTER;
		}
		// speaker mask
		else if(!token.Icmp("mask_left"))
		{
			speakerMask |= 1 << SPEAKER_LEFT;
		}
		// speaker mask
		else if(!token.Icmp("mask_right"))
		{
			speakerMask |= 1 << SPEAKER_RIGHT;
		}
		// speaker mask
		else if(!token.Icmp("mask_backright"))
		{
			speakerMask |= 1 << SPEAKER_BACKRIGHT;
		}
		// speaker mask
		else if(!token.Icmp("mask_backleft"))
		{
			speakerMask |= 1 << SPEAKER_BACKLEFT;
		}
		// speaker mask
		else if(!token.Icmp("mask_lfe"))
		{
			speakerMask |= 1 << SPEAKER_LFE;
		}
		// soundClass
		else if(!token.Icmp("soundClass"))
		{
			parms.soundClass = src.ParseInt();
			if(parms.soundClass < 0 || parms.soundClass >= SOUND_MAX_CLASSES)
			{
				src.Warning("SoundClass out of range");
				return false;
			}
		}
		// altSound
		else if(!token.Icmp("altSound"))
		{
			if(!src.ExpectAnyToken(&token))
			{
				return false;
			}
			altSound = declManager->FindSound(token.c_str());
		}
		// ordered
		else if(!token.Icmp("ordered"))
		{
			// no longer supported
		}
		// no_dups
		else if(!token.Icmp("no_dups"))
		{
			parms.soundShaderFlags |= SSF_NO_DUPS;
		}
		// no_flicker
		else if(!token.Icmp("no_flicker"))
		{
			parms.soundShaderFlags |= SSF_NO_FLICKER;
		}
		// plain
		else if(!token.Icmp("plain"))
		{
			// no longer supported
		}
		// looping
		else if(!token.Icmp("looping"))
		{
			parms.soundShaderFlags |= SSF_LOOPING;
		}
		// no occlusion
		else if(!token.Icmp("no_occlusion"))
		{
			parms.soundShaderFlags |= SSF_NO_OCCLUSION;
		}
		// private
		else if(!token.Icmp("private"))
		{
			parms.soundShaderFlags |= SSF_PRIVATE_SOUND;
		}
		// antiPrivate
		else if(!token.Icmp("antiPrivate"))
		{
			parms.soundShaderFlags |= SSF_ANTI_PRIVATE_SOUND;
		}
		// once
		else if(!token.Icmp("playonce"))
		{
			parms.soundShaderFlags |= SSF_PLAY_ONCE;
		}
		// global
		else if(!token.Icmp("global"))
		{
			parms.soundShaderFlags |= SSF_GLOBAL;
		}
		// unclamped
		else if(!token.Icmp("unclamped"))
		{
			parms.soundShaderFlags |= SSF_UNCLAMPED;
		}
		// omnidirectional
		else if(!token.Icmp("omnidirectional"))
		{
			parms.soundShaderFlags |= SSF_OMNIDIRECTIONAL;
		}
		else if(!token.Icmp("onDemand"))
		{
			// no longer loading sounds on demand
		}
		// the wave files
		else if(!token.Icmp("leadin"))
		{
			leadin = true;
		}
		else if(token.Find(".wav", false) != -1 || token.Find(".ogg", false) != -1)
		{
			if(token.IcmpPrefixPath("sound/vo/") == 0 || token.IcmpPrefixPath("sound/guis/") == 0)
			{
				parms.soundShaderFlags |= SSF_VO;
			}
			if(token.IcmpPrefixPath("sound/musical/") == 0)
			{
				parms.soundShaderFlags |= SSF_MUSIC;
			}
			// add to the wav list
			if(s_maxSamples.GetInteger() == 0 || (s_maxSamples.GetInteger() > 0 && entries.Num() < s_maxSamples.GetInteger()))
			{
				entries.Append(soundSystemLocal.LoadSample(token.c_str()));
			}
		}
		else
		{
			src.Warning("unknown token '%s'", token.c_str());
			return false;
		};
	};

	return true;
};

/*
===============
idSoundShader::List
===============
*/
void SbSoundShader::List()
{
	idStrList shaders;

	mpSys->Printf("%4i: %s\n", Index(), GetName());
	for(int k = 0; k < entries.Num(); k++)
	{
		const idSoundSample *objectp = entries[k];
		if(objectp)
			mpSys->Printf("      %5dms %4dKb %s\n", objectp->LengthInMsec(), (objectp->BufferSize() / 1024), objectp->GetName());
	};
};

/*
===============
idSoundShader::GetAltSound
===============
*/
const SbSoundShader *SbSoundShader::GetAltSound() const
{
	return altSound;
};

/*
===============
idSoundShader::GetMinDistance
===============
*/
float SbSoundShader::GetMinDistance() const
{
	return parms.minDistance;
};

/*
===============
idSoundShader::GetMaxDistance
===============
*/
float SbSoundShader::GetMaxDistance() const
{
	return parms.maxDistance;
};

/*
===============
idSoundShader::HasDefaultSound
===============
*/
bool SbSoundShader::HasDefaultSound() const
{
	for(int i = 0; i < entries.Num(); i++)
		if(entries[i] && entries[i]->IsDefault())
			return true;

	return false;
};

/*
===============
idSoundShader::GetParms
===============
*/
const soundShaderParms_t *SbSoundShader::GetParms() const
{
	return &parms;
};

/*
===============
idSoundShader::GetNumSounds
===============
*/
int SbSoundShader::GetNumSounds() const
{
	return entries.Num();
};

/*
===============
idSoundShader::GetSound
===============
*/
const char *SbSoundShader::GetSound(int index) const
{
	if(index >= 0 && index < entries.Num())
		return entries[index]->GetName();

	return "";
};

}; // namespace sbe::SbSound