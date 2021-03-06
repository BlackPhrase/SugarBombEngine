/*
*******************************************************************************

Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2019 BlackPhrase

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

#include "precompiled.h"
#include "SbInputSystem.hpp"

#include "framework/CVar.hpp"

/*
===========
GetInputAPI
============
*/
#ifndef SBE_SINGLE_BINARY
static sbe::inputExport_t inputExport;

sbe::idCmdSystem *cmdSystem{nullptr};
sbe::idCVarSystem *cvarSystem{nullptr};
sbe::ISys *sys{nullptr};
//sbe::IFileSystem *fileSystem{nullptr};

sbe::idCVar *sbe::idCVar::staticVars{nullptr};

C_EXPORT sbe::inputExport_t *GetInputAPI(sbe::inputImport_t *import)
{
	if(import->version == sbe::INPUT_API_VERSION)
	{
		// set interface pointers used by the module
		sys = import->sys;
		//common = import->common; // TODO: remove
		cmdSystem = import->cmdSystem;
		cvarSystem = import->cvarSystem;
		//fileSystem = import->fileSystem;
	};
	
	// set interface pointers used by idLib
	idLib::sys = sys;
	//idLib::common = common; // TODO: remove
	idLib::cvarSystem = cvarSystem;
	//idLib::fileSystem = fileSystem;
	
	// setup export interface
	inputExport.version = sbe::INPUT_API_VERSION;
	inputExport.inputSystem = CreateInputSystem(sys);
	
	return &inputExport;
};
#endif // SBE_SINGLE_BINARY