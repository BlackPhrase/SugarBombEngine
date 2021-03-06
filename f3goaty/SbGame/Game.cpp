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

#include <cstdio>

#include "Game.hpp"

namespace f3goaty
{

void CGame::Init()
{
	printf("Hello Game!\n");
};

void CGame::Shutdown()
{
};

void CGame::RunFrame(sbe::idUserCmdMgr &aCmdMgr, sbe::gameReturn_t &aGameReturn)
{
};

void CGame::ClientRunFrame(sbe::idUserCmdMgr &aCmdMgr, bool abLastPredictFrame, sbe::gameReturn_t &aGameReturn)
{
};

bool CGame::Draw(int anClientNum)
{
	return false;
};

void CGame::Shell_CreateMenu(bool abInGame)
{
};

void CGame::Shell_Cleanup()
{
};

void CGame::Shell_Show(bool abShow)
{
};

void CGame::Shell_SyncWithSession()
{
};

}; // namespace f3goaty