/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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

#pragma once

#include <cstddef>
//#include "precompiled.h"

#include "CoreLibs/SbSound/SoundShaderParms.hpp"

#include "idlib/containers/List.h"

#include "framework/Decl.hpp"

/*
===============================================================================

	SOUND SHADER DECL

===============================================================================
*/

namespace sbe
{

// sound shader flags
static const int SSF_PRIVATE_SOUND = BIT(0);      // only plays for the current listenerId
static const int SSF_ANTI_PRIVATE_SOUND = BIT(1); // plays for everyone but the current listenerId
static const int SSF_NO_OCCLUSION = BIT(2);       // don't flow through portals, only use straight line
static const int SSF_GLOBAL = BIT(3);             // play full volume to all speakers and all listeners
static const int SSF_OMNIDIRECTIONAL = BIT(4);    // fall off with distance, but play same volume in all speakers
static const int SSF_LOOPING = BIT(5);            // repeat the sound continuously
static const int SSF_PLAY_ONCE = BIT(6);          // never restart if already playing on any channel of a given emitter
static const int SSF_UNCLAMPED = BIT(7);          // don't clamp calculated volumes at 1.0
static const int SSF_NO_FLICKER = BIT(8);         // always return 1.0 for volume queries
static const int SSF_NO_DUPS = BIT(9);            // try not to play the same sound twice in a row
static const int SSF_VO = BIT(10);                // VO - direct a portion of the sound through the center channel (set automatically on shaders that contain files that start with "sound/vo/")
static const int SSF_MUSIC = BIT(11);             // Music - Muted when the player is playing his own music

struct ISys;
}; // namespace sbe

struct IDeclManager;
class idLexer;

namespace sbe::SbSound
{

class SbSoundSample;

// it is somewhat tempting to make this a virtual class to hide the private
// details here, but that doesn't fit easily with the decl manager at the moment.
class SbSoundShader : public idDecl
{
public:
	SbSoundShader(sbe::ISys *apSys);
	virtual ~SbSoundShader();

	virtual size_t Size() const;
	virtual bool SetDefaultText();
	virtual const char *DefaultDefinition() const;
	virtual bool Parse(const char *text, const int textLength, bool allowBinaryVersion);
	virtual void FreeData();
	virtual void List() const;

	// so the editor can draw correct default sound spheres
	// this is currently defined as meters, which sucks, IMHO.
	virtual float GetMinDistance() const; // FIXME: replace this with a GetSoundShaderParms()
	virtual float GetMaxDistance() const;

	// returns NULL if an AltSound isn't defined in the shader.
	// we use this for pairing a specific broken light sound with a normal light sound
	virtual const SbSoundShader *GetAltSound() const;

	virtual bool HasDefaultSound() const;

	virtual const soundShaderParms_t *GetParms() const;
	virtual int GetNumSounds() const;
	virtual const char *GetSound(int index) const;
private:
	friend class SbSoundWorldLocal;
	friend class SbSoundEmitterLocal;
	friend class SbSoundChannel;

	// options from sound shader text
	soundShaderParms_t parms{}; // can be overriden on a per-channel basis

	int speakerMask{0};
	const SbSoundShader *altSound{nullptr};

	bool leadin{false};        // true if this sound has a leadin
	float leadinVolume{0.0f}; // allows light breaking leadin sounds to be much louder than the broken loop

	idList<SbSoundSample *, TAG_AUDIO> entries;
private:
	void Init();
	bool ParseShader(sbe::IDeclManager *apDeclManager, idLexer &src);
private:
	sbe::ISys *mpSys{ nullptr };
};

}; // namespace sbe::SbSound