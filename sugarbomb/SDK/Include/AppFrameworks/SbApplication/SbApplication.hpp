/*
*******************************************************************************

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

*******************************************************************************
*/

/// @file

//*****************************************************************************

#pragma once

//*****************************************************************************

namespace sbe
{
struct ISystem;
};

class SbApplication
{
public:
	SbApplication(sbe::ISystem *apSystem, int argc, char **argv);
	virtual ~SbApplication();
	
	void Run();
protected:
	void Stop();
	
	bool CloseRequested() const {return mbWantClose;}
	
	virtual bool PreFrame(){return true;}
	virtual void RunFrame(){}
	virtual void PostFrame(){}
	
	sbe::ISystem *mpSystem{nullptr};
private:
	void Init();
	
	bool mbInitialized{false};
	bool mbWantClose{false};
};