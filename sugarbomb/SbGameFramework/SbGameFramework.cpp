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

//*****************************************************************************

#include "SbGameFramework.hpp"

#include "CoreLibs/SbSystem/ISystem.hpp"

#include "SbGame/IGame.hpp"

#include "SbNetwork/INetworkSystem.hpp"

#include "CoreLibs/SbRenderer/IRenderSystem.hpp"

#include "CoreLibs/SbSound/ISoundSystem.hpp"

//*****************************************************************************

namespace sbe
{
class idUserCmdMgr
{
};

struct gameReturn_t
{
	int stub;
};
};

namespace sbe::SbGameFramework
{

SbGameFramework::SbGameFramework(IRenderSystem *apRenderSystem, ISoundSystem *apSoundSystem, INetworkSystem &aNetworkSystem, IGame &aGame, ISystem &aSystem)
	: mpRenderSystem(apRenderSystem), mpSoundSystem(apSoundSystem), mNetworkSystem(aNetworkSystem), mGame(aGame), mSystem(aSystem){}

void SbGameFramework::Init()
{
	mNetworkSystem.Init();
	mGame.Init();
	
	CreateMainMenu();
};

void SbGameFramework::Shutdown()
{
	
	printf("CleanupShell();\n");
	CleanupShell();
	
	mGame.Shutdown();
	mNetworkSystem.Shutdown();
};

void SbGameFramework::Frame()
{
	idUserCmdMgr UserCmdMgrStub;
	gameReturn_t GameReturnStub;
	
	mGame.RunFrame(UserCmdMgrStub, GameReturnStub);
	mGame.ClientRunFrame(UserCmdMgrStub, false, GameReturnStub);
	mGame.Draw(0);
};

/*
========================
idCommonLocal::CreateMainMenu
========================
*/
void SbGameFramework::CreateMainMenu()
{
	//if(mpGame != nullptr) // TODO
	{
		// note which media we are going to need to load
		//declManager->BeginLevelLoad(); // TODO
		mpRenderSystem->BeginLevelLoad();
		mpSoundSystem->BeginLevelLoad();
		//uiManager->BeginLevelLoad(); // TODO
		
		// create main inside an "empty" game level load - so assets get
		// purged automagically when we transition to a "real" map
		mGame.Shell_CreateMenu(false);
		mGame.Shell_Show(true);
		mGame.Shell_SyncWithSession();
		
		// load
		mpRenderSystem->EndLevelLoad();
		mpSoundSystem->EndLevelLoad();
		//declManager->EndLevelLoad(); // TODO
		//uiManager->EndLevelLoad(""); // TODO
	};
};

/*
=================
idCommonLocal::CleanupShell
=================
*/
void SbGameFramework::CleanupShell()
{
	mGame.Shell_Cleanup();
};

}; // namespace sbe::SbGameFramework