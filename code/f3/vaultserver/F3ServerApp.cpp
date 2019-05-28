/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012-2014 Robert Beckebans
Copyright (C) 2019 BlackPhrase

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/// @file

#include "F3ServerApp.hpp"
#include "framework/IGameFramework.hpp"

namespace f3bfg
{

CServerApp::CServerApp(int argc, char **argv) : SbClientApp(argc, argv)
{
};

CServerApp::~CServerApp()
{
	Shutdown();
	
	ShutdownFrameworkModule();
};

void CServerApp::PostInit()
{
	InitFrameworkModule();
	
	mpFramework->Init(0, nullptr, lpCmdLine);
	
#if TEST_FPU_EXCEPTIONS != 0
	mpFramework->Printf(Sys_FPU_GetState());
#endif
};

void CServerApp::PostFrame()
{
	// run the game
	mpFramework->Frame();
};

bool CServerApp::PreLogicUpdate()
{
	return true;
};

void CServerApp::LogicUpdate()
{
	
};

void CServerApp::Shutdown()
{
	mpFramework->Shutdown();
};

/*
=================
idCommonLocal::LoadFrameworkModule
=================
*/
void CServerApp::InitFrameworkModule()
{
#ifndef SBE_SINGLE_BINARY
	char			dllPath[ sbe::MAX_OSPATH ];
	
	sbe::frameworkImport_t	frameworkImport;
	sbe::frameworkExport_t	frameworkExport;
	sbe::GetFrameworkAPI_t	GetFrameworkAPI;
	
	mpFileSystem->FindDLL( "SbFramework", dllPath/*, true*/ );
	
	if( !dllPath[ 0 ] )
	{
		mpSys->FatalError( "couldn't find framework dynamic library" );
		return;
	};
	mpSys->DPrintf( "Loading framework DLL: '%s'\n", dllPath );
	frameworkDLL = mpSys->DLL_Load( dllPath );
	if( !frameworkDLL )
	{
		mpSys->FatalError( "couldn't load framework dynamic library" );
		return;
	};
	
	const char* functionName = "GetFrameworkAPI";
	GetFrameworkAPI = ( GetFrameworkAPI_t ) mpSys->DLL_GetProcAddress( frameworkDLL, functionName );
	if( !GetFrameworkAPI )
	{
		mpSys->DLL_Unload( frameworkDLL );
		frameworkDLL = 0;
		mpSys->FatalError( "couldn't find framework DLL API" );
		return;
	};
	
	frameworkImport.version					= sbe::FRAMEWORK_API_VERSION;
	frameworkImport.sys						= mpSys;
	frameworkImport.cmdSystem				= mpCmdSystem.get();
	frameworkImport.cvarSystem				= mpCVarSystem.get();
	frameworkImport.fileSystem				= mpFileSystem;
	frameworkImport.renderSystem				= renderSystem;
	frameworkImport.declManager				= ::declManager;
	
	frameworkExport							= *GetFrameworkAPI( &frameworkImport );
	
	if( frameworkExport.version != sbe::FRAMEWORK_API_VERSION )
	{
		mpSys->DLL_Unload( frameworkDLL );
		frameworkDLL = 0;
		mpSys->FatalError( "wrong framework DLL API version" );
		return;
	};
	
	mpFramework								= frameworkExport.framework;
	
#endif
	
	// initialize the sound object
	if( mpFramework != nullptr )
		mpFramework->Init();
};

/*
=================
idCommonLocal::UnloadFrameworkModule
=================
*/
void CServerApp::ShutdownFrameworkModule()
{
	// shut down the framework object
	if( mpFramework != nullptr )
	{
		mpFramework->Shutdown();
		mpFramework = nullptr;
	};
	
#ifndef SBE_SINGLE_BINARY
	if( frameworkDLL )
	{
		mpSys->DLL_Unload( frameworkDLL );
		frameworkDLL = 0;
	};
#endif
};

}; // namespace f3bfg