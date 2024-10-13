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

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "alg/graphics.h"
#include "alg/scene.h"
#include "alg/video.h"
#include "alg/game.h"

namespace Alg {

Game::Game(AlgEngine *vm) {
	_vm = vm;
	_in_menu = false;
	_gameRunning = true;
	_rnd = new Common::RandomSource("alg");

	_screen = new Graphics::Surface();
	_background = new Graphics::Surface();

	_screen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_palette = new uint8[257 * 3]();

	// blue for rect display
	_palette[5] = 0xFF;

	_paletteDirty = true;

	_videoDecoder = new AlgVideoDecoder();
	_videoDecoder->setPalette(_palette);

	_sceneInfo = new SceneInfo();
}

Game::~Game() {
	delete _rnd;

	_libFile.close();
	_libFileNames.clear();
	_libFileOffsets.clear();

	delete[] _palette;
	delete _screen;
	delete _background;

	delete _videoDecoder;
	delete _sceneInfo;
}

Common::Error Game::run() {
	/*
	init();
	_cur_scene = _sceneInfo->getStartScene();
	uint32 nextFrameTime = g_system->getMillis();
	while (!_vm->shouldQuit() && _gameRunning) {
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		loadScene(scene);
		uint32 currentFrame = 0;
		beforeScene(scene);
		while((!_videoDecoder->isFinished() || getPausedFrames() || isPaused()) && !_vm->shouldQuit() && _gameRunning) {
			if(!_in_menu) {
				int32 remainingMillis = nextFrameTime - g_system->getMillis();
				if (remainingMillis < 10 || currentFrame == 0) {
					if (currentFrame == 0) {
						_videoDecoder->getNextFrame();
						currentFrame = scene->startFrame;
					} else if (getPausedFrames() == 0) {
						_videoDecoder->getNextFrame();
						currentFrame += 3;
					} else {
						decPausedFrames();
					}
					if(remainingMillis > 0) {
						g_system->delayMillis(remainingMillis);
					}
					nextFrameTime = g_system->getMillis() + 100;
				}
			}
			updateScreen();
			pollEvents();
			checkKeysPressed();
			duringScene(scene, currentFrame);
			if (_lastMouseButton) {
				_lastMouseButton = 0;
				uint32 mousePosX = g_system->getEventManager()->getMousePos().x;
				uint32 mousePosY = g_system->getEventManager()->getMousePos().y;
				if(!mouseClicked(mousePosX, mousePosY, currentFrame) && !_in_menu && isSkippable()) {
					break;
				}
			}

			if(isSkipToNextScene()) {
				break;
			}

			g_system->delayMillis(10);
		}

		debug("_cur_scene: %s", _cur_scene.c_str());

		// TODO remove
		afterScene(_sceneInfo->findScene(_cur_scene));
		Common::String nextScene = findNextScene();
		if(nextScene.empty()) {
			_cur_scene = _sceneInfo->findScene(findNextScene())->name;
		} else {
			_cur_scene = _sceneInfo->findScene(nextScene)->name;
		}
		debug("nextScene: %s", nextScene.c_str());
	}
		*/

	return Common::kNoError;
}

void Game::loadLibArchive(const Common::Path &path) {
	_libFile.open(path);
	assert(_libFile.isOpen());

	uint16 magicBytes = _libFile.readSint16LE();
	uint32 indexOffset = _libFile.readSint32LE();

	assert(magicBytes == 1020);

	_libFile.seek(indexOffset);
	uint16 indexSize = _libFile.readSint16LE();

	assert(indexSize > 0);

	while(true) {
		uint32 entryOffset = _libFile.readSint32LE();
		Common::String entryName = _libFile.readStream(13)->readString();
		if(entryName.empty()) {
			break;
		}
		_libFileOffsets.push_back(entryOffset);
		_libFileNames.push_back(entryName);
	}

	/*
	// dump all file entries to files
	for(uint32 i = 0; i < _libFileOffsets.size(); i++) {
		_libFile.seek(_libFileOffsets[i]);
		uint32 size = _libFile.readUint32LE();
		Common::String dumpFileName = Common::String::format("dump/%s", _libFileNames[i].c_str());
		Common::DumpFile dumpFile;
		dumpFile.open(dumpFileName, true);
		dumpFile.writeStream(_libFile.readStream(size), size);
		dumpFile.close();
	}
	*/

	_libFile.seek(0);
	_videoDecoder->setStream(_libFile.readStream(_libFile.size()));
}

void Game::load8bAudioSample(const Common::Path &path) {
	// TODO: implement
}

void Game::loadScene(Scene *scene) {
	Common::String sceneFileName = Common::String::format("%s.mm", scene->name.c_str());
	for(uint32 i = 0; i < _libFileNames.size(); i++) {
		if (_libFileNames[i].equalsIgnoreCase(sceneFileName)) {
			debug("loaded scene %s", scene->name.c_str());
			_videoDecoder->loadVideoFromStream(_libFileOffsets[i]);
			return;
		}
	}
	error("Cannot find scene %s in libfile", scene->name.c_str());
}

bool Game::pollEvents() {
	Common::Event event;
	bool hasEvents = false;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_KEYDOWN) {
			_keysPressed.push(event.kbd);
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			_lastMouseButton = 1;
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			_lastMouseButton = 2;
		}
		hasEvents = true;
	}

	return hasEvents;
}

Common::KeyState Game::getNextKey() {
	if (_keysPressed.empty()) {
		return Common::KeyState();
	} else {
		return _keysPressed.pop();
	}
}

bool Game::checkKeysPressed() {
	Common::KeyCode kc = getNextKey().keycode;
	if (kc != Common::KEYCODE_INVALID) {
		clearKeys();
		return true;
	} else {
		return false;
	}
}

bool Game::mouseClicked(int16 x, int16 y, uint32 currentFrame) {
	Scene *scene = _sceneInfo->findScene(_cur_scene);
	if(x >= 13 && x <= 63 && y >= 174 && y <= 193) {
		// rectShotMenu(); // TODO remove
		return true;
	} else if(_in_menu) {
		Scene *menuScene = _sceneInfo->findScene("menu");
		for(uint32 i = 0; i < menuScene->zones.size(); i++) {
			for(uint32 j = 0; j < menuScene->zones[i].rects.size(); j++) {
				Rect rect = menuScene->zones[i].rects[j];
				if (rect.contains(x, y)) {
					// rectHit(&rect, x, y);
					return true;
				}
			}
		}
	} else {
		for(uint32 i = 0; i < scene->zones.size(); i++) {
			uint32 zoneStartFrame = scene->zones[i].startFrame - 2;
			uint32 zoneEndFrame = scene->zones[i].endFrame + 2;
			if(currentFrame >= zoneStartFrame && currentFrame <= zoneEndFrame) {
				for(uint32 j = 0; j < scene->zones[i].rects.size(); j++) {
					Rect rect = scene->zones[i].rects[j];
					if (rect.contains(x, y)) {
						// rectHit(&rect, x, y);
						return true;
					}
				}
			}
		}
	}

	return false;
}

// TODO remove
/*
Common::String Game::findNextScene() {
	_skippable = false;
	Common::String nextScene = _next_scene;
	if(!_sub_scene.empty()) {
		_skippable = true;
		_next_scene = _sub_scene;
		_sub_scene = "";
	} else if(_ret_scene) {
		_next_scene = _ret_scene;
		_ret_scene = "";
	}
	return _next_scene;
}
*/

void Game::debug_drawZoneRects() {
	if(!_debug_drawRects) { return; }

	Scene *targetScene = _sceneInfo->findScene(_cur_scene);
	if(_in_menu) {
		targetScene = _sceneInfo->findScene("menu");
	}

	for(uint8 j = 0; j < targetScene->zones.size(); j++) {
		Zone zone = targetScene->zones[j];
		for(uint8 k = 0; k < zone.rects.size(); k++) {
			Rect rect = zone.rects[k];
			_screen->drawLine(rect.left, rect.top, rect.right, rect.top, 1);
			_screen->drawLine(rect.left, rect.top, rect.left, rect.bottom, 1);
			_screen->drawLine(rect.right, rect.bottom, rect.right, rect.top, 1);
			_screen->drawLine(rect.right, rect.bottom, rect.left, rect.bottom, 1);
		}
	}
}

} // End of namespace Alg
