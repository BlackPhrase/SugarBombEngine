/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans
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

#pragma once

//#include "SbXA2_SoundVoice.hpp" // for idSoundVoice_XAudio2
//#include "SbSoundDefines.hpp"

//#include "framework/DebugGraph.h"

#include "idlib/containers/StaticList.h"

namespace sbe::SbSound
{

class SbSoundSample_XAudio2;
class SbSoundVoice_XAudio2;
// RB
class SbSoundHardware_XAudio2;

struct IXAudio2;
struct IXAudio2MasteringVoice;
struct IXAudio2SubmixVoice;
class idSoundVoice;
class idSoundSample;
class idDebugGraph;

/*
================================================
idSoundEngineCallback
================================================
*/
class idSoundEngineCallback : public IXAudio2EngineCallback
{
public:
	SbSoundHardware_XAudio2 *hardware;

private:
	// Called by XAudio2 just before an audio processing pass begins.
	STDMETHOD_( void, OnProcessingPassStart )( THIS ) {}
	
	// Called just after an audio processing pass ends.
	STDMETHOD_( void, OnProcessingPassEnd )( THIS ) {}
	
	// Called in the event of a critical system error which requires XAudio2
	// to be closed down and restarted.  The error code is given in Error.
	STDMETHOD_( void, OnCriticalError )( THIS_ HRESULT Error );
};

/*
================================================
idSoundHardware_XAudio2
================================================
*/

class SbSoundHardware_XAudio2
{
public:
	SbSoundHardware_XAudio2();

	void Init();
	void Shutdown();

	void Update();

	SbSoundVoice *AllocateVoice(const SbSoundSample *leadinSample, const SbSoundSample *loopingSample);
	void FreeVoice(SbSoundVoice *voice);

	// video playback needs this
	IXAudio2 *GetIXAudio2() const
	{
		return pXAudio2;
	};

	int GetNumZombieVoices() const
	{
		return zombieVoices.Num();
	}
	int GetNumFreeVoices() const
	{
		return freeVoices.Num();
	}

protected:
	friend class SbSoundSample_XAudio2;
	friend class SbSoundVoice_XAudio2;

private:
	IXAudio2 *pXAudio2;
	IXAudio2MasteringVoice *pMasterVoice;
	IXAudio2SubmixVoice *pSubmixVoice;

	idSoundEngineCallback soundEngineCallback;
	int lastResetTime;

	int outputChannels;
	int channelMask;

	idDebugGraph *vuMeterRMS;
	idDebugGraph *vuMeterPeak;
	int vuMeterPeakTimes[8];

	// Can't stop and start a voice on the same frame, so we have to double this to handle the worst case scenario of stopping all voices and starting a full new set
	idStaticList<SbSoundVoice_XAudio2, MAX_HARDWARE_VOICES * 2> voices;
	idStaticList<SbSoundVoice_XAudio2 *, MAX_HARDWARE_VOICES * 2> zombieVoices;
	idStaticList<SbSoundVoice_XAudio2 *, MAX_HARDWARE_VOICES * 2> freeVoices;
};

/*
================================================
idSoundHardware
================================================
*/
class idSoundHardware : public SbSoundHardware_XAudio2
{
};

}; // namespace sbe::SbSound