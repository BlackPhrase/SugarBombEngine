/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013 Robert Beckebans
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

//#include "SbAL_SoundSample.hpp" // for idSoundSample_OpenAL, etc
#include "SbSoundVoice.hpp" // for idSoundVoice_Base

namespace sbe::SbSound
{

static const int MAX_QUEUED_BUFFERS = 3;

/*
================================================
idSoundVoice_OpenAL
================================================
*/
class SbSoundVoice_OpenAL : public idSoundVoice_Base
{
public:
	SbSoundVoice_OpenAL();
	~SbSoundVoice_OpenAL();

	void SetPosition(const idVec3 &p)
	{
		SbSoundVoice_Base::SetPosition(p);

		alSource3f(openalSource, AL_POSITION, -p.y, p.z, -p.x);
	}

	void SetGain(float gain)
	{
		SbSoundVoice_Base::SetGain(gain);

		alSourcef(openalSource, AL_GAIN, (gain) < (1.0f) ? (gain) : (1.0f));
	}

	void SetPitch(float p)
	{
		SbSoundVoice_Base::SetPitch(p);

		alSourcef(openalSource, AL_PITCH, p);
	}

	void Create(const SbSoundSample *leadinSample, const SbSoundSample *loopingSample);

	// Start playing at a particular point in the buffer.  Does an Update() too
	void Start(int offsetMS, int ssFlags);

	// Stop playing.
	void Stop();

	// Stop consuming buffers
	void Pause();

	// Start consuming buffers again
	void UnPause();

	// Sends new position/volume/pitch information to the hardware
	bool Update();

	// returns the RMS levels of the most recently processed block of audio, SSF_FLICKER must have been passed to Start
	float GetAmplitude();

	// returns true if we can re-use this voice
	bool CompatibleFormat(SbSoundSample_OpenAL *s);

	uint32 GetSampleRate() const
	{
		return sampleRate;
	}

	// callback function
	void OnBufferStart(SbSoundSample_OpenAL *sample, int bufferNumber);

private:
	friend class SbSoundHardware_OpenAL;

	// Returns true when all the buffers are finished processing
	bool IsPlaying();

	// Called after the voice has been stopped
	void FlushSourceBuffers();

	// Destroy the internal hardware resource
	void DestroyInternal();

	// Helper function used by the initial start as well as for looping a streamed buffer
	int RestartAt(int offsetSamples);

	// Helper function to submit a buffer
	int SubmitBuffer(SbSoundSample_OpenAL *sample, int bufferNumber, int offset);

	// Adjust the voice frequency based on the new sample rate for the buffer
	void SetSampleRate(uint32 newSampleRate, uint32 operationSet);

	//IXAudio2SourceVoice* 	pSourceVoice;
	bool triggered;
	ALuint openalSource;
	ALuint openalStreamingOffset;
	ALuint openalStreamingBuffer[3];
	ALuint lastopenalStreamingBuffer[3];

	SbSoundSample_OpenAL *leadinSample;
	SbSoundSample_OpenAL *loopingSample;

	// These are the fields from the sample format that matter to us for voice reuse
	uint16 formatTag;
	uint16 numChannels;

	uint32 sourceVoiceRate;
	uint32 sampleRate;

	bool hasVUMeter;
	bool paused;
};

/*
================================================
idSoundVoice
================================================
*/
class SbSoundVoice : public SbSoundVoice_OpenAL
{
};

}; // namespace sbe::SbSound