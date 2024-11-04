/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ALG_ALG_H
#define ALG_ALG_H

#include "engines/advancedDetector.h"
#include "gui/debugger.h"

namespace Alg {

class Game;
class GameMaddog;
class GameMaddog2;
class GameJohnnyRock;

enum {
	kAlgDebugGeneral = 1 << 0,
	kAlgDebugGraphics = 1 << 1
};

class AlgEngine : public Engine {
public:
	AlgEngine(OSystem *syst, const ADGameDescription *desc);
	~AlgEngine();

	Common::Error run();

	bool hasFeature(EngineFeature f) const;

private:
	Game *_game;
	GUI::Debugger *_debugger;
};

class Console : public GUI::Debugger {
public:
	Console(AlgEngine *vm) {
	}
	virtual ~Console(void) {
	}
};

} // End of namespace Alg

#endif
