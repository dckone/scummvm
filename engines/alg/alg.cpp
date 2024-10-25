/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/util.h"

#include "alg/alg.h"
#include "alg/game.h"
#include "alg/game_maddog.h"

namespace Alg {

AlgEngine::AlgEngine(OSystem *syst)
	: Engine(syst) {
	GameMaddog *gameMaddog = new GameMaddog(this);
	_game = gameMaddog;
	_debugger = new DebuggerMaddog(gameMaddog);
}

AlgEngine::~AlgEngine() {
	delete _game;
}

Common::Error AlgEngine::run() {
	initGraphics(320, 200);
	setDebugger(_debugger);
	return _game->run();
}

bool AlgEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

} // End of namespace Alg
