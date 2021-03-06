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

//#include "CoreLibs/SbSound/SbSound.h"
//#include "SbWaveFile.hpp"
//#include "SbSoundDefines.hpp"
//#include "SbSoundVoice.hpp"

#if defined(USE_OPENAL)
#	include "openal/SbAL_Defines.hpp"
#elif defined(_MSC_VER) // DG: stub out xaudio for MinGW etc
#	include "xaudio2/SbXA2_Defines.hpp"
#else // not _MSC_VER => MinGW, GCC, ...
#	include "stub/SoundStub.h" // just a stub for now
#endif // _MSC_VER ; DG end

//#include "SbSoundFade.hpp"
//#include "SbSoundChannel.hpp"
//#include "SbSoundWorld.hpp"
//#include "SbSoundEmitter.hpp"
//#include "SbSoundSystem.hpp"