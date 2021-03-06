/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2013-2016 Robert Beckebans
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

#pragma once

#include "CoreLibs/SbSound/ISoundWorld.hpp"
#include "SbSoundFade.hpp"
#include "SbSoundDefines.hpp"

#include "idlib/Heap.h"
#include "idlib/math/Matrix.h"
#include "idlib/math/Vector.h"
#include "idlib/containers/List.h"

class idDecl;

namespace sbe
{

struct ISystem;
struct IRenderWorld;
struct idConsole;

namespace SbSound
{

class SbSoundChannel;
class SbSoundEmitterLocal;
class SbSoundShader;

//------------------------
// Listener data
//------------------------
struct listener_t
{
	idMat3 axis{}; // orientation of the listener
	idVec3 pos{};  // position in meters
	int id{0};      // the entity number, used to detect when a sound is local
	int area{0};    // area number the listener is in
};

class SbSoundWorldLocal : public idSoundWorld
{
public:
	SbSoundWorldLocal(sbe::ICommon *apCommon, idConsole *apConsole);
	virtual ~SbSoundWorldLocal();

	//------------------------
	// Functions from idSoundWorld, implemented in SoundWorld.cpp
	//------------------------

	// Called at map start
	virtual void ClearAllSoundEmitters();

	// stop all playing sounds
	virtual void StopAllSounds();

	// get a new emitter that can play sounds in this world
	virtual idSoundEmitter *AllocSoundEmitter();

	// for load games
	virtual idSoundEmitter *EmitterForIndex(int index) const;

	// query data from all emitters in the world
	virtual float CurrentShakeAmplitude() const;

	// where is the camera
	virtual void PlaceListener(const idVec3 &origin, const idMat3 &axis, const int listenerId);

	virtual void WriteSoundShaderLoad(const idSoundShader *snd);

	// fade all sounds in the world with a given shader soundClass
	// to is in Db, over is in seconds
	virtual void FadeSoundClasses(const int soundClass, const float to, const float over);

	// dumps the current state and begins archiving commands
	virtual void StartWritingDemo(idDemoFile *demo);
	virtual void StopWritingDemo();

	// read a sound command from a demo file
	virtual void ProcessDemoCommand(idDemoFile *readDemo);

	// menu sounds
	virtual int PlayShaderDirectly(const char *name, int channel = -1);

	virtual void Skip(int time);

	virtual void Pause();
	virtual void UnPause();
	virtual bool IsPaused() const
	{
		return isPaused;
	}

	virtual int GetSoundTime() const;

	// avidump
	virtual void AVIOpen(const char *path, const char *name);
	virtual void AVIClose();

	// SaveGame Support
	virtual void WriteToSaveGame(sbe::IFile *savefile);
	virtual void ReadFromSaveGame(sbe::IFile *savefile);

	virtual void SetSlowmoSpeed(float speed);
	virtual void SetEnviroSuit(bool active);

	//=======================================

	//------------------------
	// Random stuff that's not exposed outside the sound system
	//------------------------
	void Update(float afTimeStep);
	void OnReloadSound(const idDecl *decl);

	SbSoundChannel *AllocSoundChannel();
	void FreeSoundChannel(SbSoundChannel *);
public:
	// even though all these variables are public, nobody outside the sound system includes SoundWorld_local.h
	// so this is equivalent to making it private and friending all the other classes in the sound system

	SbSoundFade volumeFade{}; // master volume knob for the entire world
	SbSoundFade soundClassFade[SOUND_MAX_CLASSES];

	sbe::IRenderWorld *renderWorld{nullptr}; // for debug visualization and light amplitude sampling
	idDemoFile *writeDemo{nullptr};      // if not nullptr, archive commands here

	float currentCushionDB{0.0f}; // channels at or below this level will be faded to 0
	float shakeAmp{0.0f};         // last calculated shake amplitude

	listener_t listener{};
	idList<SbSoundEmitterLocal *, TAG_AUDIO> emitters;

	idSoundEmitter *localSound{nullptr}; // for PlayShaderDirectly()

	idBlockAlloc<SbSoundEmitterLocal, 16> emitterAllocator;
	idBlockAlloc<SbSoundChannel, 16> channelAllocator;

	SbSoundFade pauseFade{};
	int pausedTime{0};
	int accumulatedPauseTime{0};
	bool isPaused{false};

	float slowmoSpeed{0.0f};
	bool enviroSuitActive{false};
public:
	struct soundPortalTrace_t
	{
		int portalArea{0};
		const soundPortalTrace_t *prevStack{nullptr};
	};

	void ResolveOrigin(const int stackDepth, const soundPortalTrace_t *prevStack, const int soundArea, const float dist, const idVec3 &soundOrigin, SbSoundEmitterLocal *def);
private:
	idCommon *common{ nullptr };
	idConsole *console{nullptr};
};

};}; // namespace sbe::SbSound