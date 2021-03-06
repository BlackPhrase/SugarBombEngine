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

//*****************************************************************************

#pragma once

//*****************************************************************************

namespace sbe
{

/*
===============================================================================

	Game API.

===============================================================================
*/

constexpr auto GAME_API_VERSION{8};

struct IGame;
struct IGameEdit;
struct ISystem;
struct IGameFramework;
struct idCmdSystem;
struct idCVarSystem;
struct IFileSystem;
struct IRenderSystem;
struct ISoundSystem;
struct IRenderModelManager;
struct IUserInterfaceManager;
struct IDeclManager;
struct idAASFileManager;
struct ICollisionModelManager;
struct IInputManager;
struct IDialogueManager;

struct gameImport_t
{
	int							version;				///< API version
	
	// Tier 1 - Core Engine Functionality
	
	ISystem* 						sys;					///< non-portable system services
	IGameFramework* 					common;					///< common
	idCmdSystem* 				cmdSystem;				///< console command system
	idCVarSystem* 				cvarSystem;				///< console variable system
	IFileSystem* 				fileSystem;				///< file system
	IRenderSystem* 			renderSystem;			///< render system
	ISoundSystem* 				soundSystem;			///< sound system
	IRenderModelManager* 		renderModelManager;		///< render model manager
	IUserInterfaceManager* 	uiManager;				///< user interface manager
	IDeclManager* 				declManager;			///< declaration manager
	idAASFileManager* 			AASFileManager;			///< AAS file manager
	ICollisionModelManager* 	collisionModelManager;	///< collision model manager
	IInputManager *mpInputManager{nullptr}; ///< input manager
	
	// Tier 2 - Genre-Specific Functionality
	
	IDialogueManager *dialogueManager{nullptr};
};

struct gameExport_t
{
	int							version; ///< API version
	IGame* 					game{nullptr}; ///< interface to run the game
	IGameEdit* 				gameEdit{nullptr}; ///< interface for in-game editing
};

extern "C" using GetGameAPI_t = gameExport_t *(*)( gameImport_t* import );

}; // namespace sbe