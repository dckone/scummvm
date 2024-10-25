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

#include "common/events.h"
#include "common/substream.h"
#include "graphics/paletteman.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "alg/graphics.h"
#include "alg/scene.h"

#include "alg/game.h"

namespace Alg {

Game::Game(AlgEngine *vm) {
	_vm = vm;
	_in_menu = false;
	_palette = new uint8[257 * 3]();
	// blue for rect display
	_palette[5] = 0xFF;
	_paletteDirty = true;
	_screen = new Graphics::Surface();
	_rnd = new Common::RandomSource("alg");
	_screen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_videoDecoder = new AlgVideoDecoder();
	_videoDecoder->setPalette(_palette);
	_sceneInfo = new SceneInfo();
}

Game::~Game() {
	_libFile.close();
	_libFileEntries.clear();
	delete _rnd;
	delete[] _palette;
	delete _screen;
	delete _background;
	delete _videoDecoder;
	delete _sceneInfo;
}

bool Game::pollEvents() {
	Common::Event event;
	bool hasEvents = false;
	_lastMouseButton = 0;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_MOUSEMOVE) {
			_lastMousePos = event.mouse;
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			_lastMouseButton = 1;
			_lastMousePos = event.mouse;
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			_lastMouseButton = 2;
			_lastMousePos = event.mouse;
		}
		hasEvents = true;
	}
	return hasEvents;
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
		_libFileEntries[entryName] = entryOffset;
	}
	_libFile.seek(0);
	_videoDecoder->setStream(_libFile.readStream(_libFile.size()));
}

Audio::SeekableAudioStream* Game::_LoadSoundFile(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (!file->open(path)) {
		warning("_LoadSound: Could not open file \"%s\"", path.toString().c_str());
		delete file;
		return nullptr;
	}
	return Audio::makeRawStream(new Common::SeekableSubReadStream(file, 0, file->size(), DisposeAfterUse::NO), 8000, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
}

void Game::_PlaySound(Audio::SeekableAudioStream *stream) {
    if (stream != nullptr) {
        stream->rewind();
        g_system->getMixer()->stopHandle(_sfxAudioHandle);
        g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_sfxAudioHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
    }
}

void Game::loadScene(Scene *scene) {
	Common::String sceneFileName = Common::String::format("%s.mm", scene->name.c_str());
    Common::HashMap<Common::String, uint32>::iterator it = _libFileEntries.find(sceneFileName);
	if (it != _libFileEntries.end()) {
		debug("loaded scene %s", scene->name.c_str());
		_videoDecoder->loadVideoFromStream(it->_value);
    } else {
		error("Cannot find scene %s in libfile", scene->name.c_str());
    }
}

void Game::updateScreen() {
    if (!_in_menu) {
		Graphics::Surface *frame = _videoDecoder->getVideoFrame();
    	_screen->copyRectToSurface(frame->getPixels(), frame->pitch, _videoPosX, _videoPosY, frame->w, frame->h);
    }
    _debug_drawRects = true; // TODO: remove
   	if(_debug_drawRects) {
    	debug_drawZoneRects();
    }
	if (_paletteDirty || _videoDecoder->isPaletteDirty()) {
		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
		_paletteDirty = false;
	}
    g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->w, _screen->h);
	g_system->updateScreen();
}

void Game::debug_drawZoneRects() {
	if(_in_menu || _cur_scene == "") {
		 return; 
	}
	Scene *targetScene = _sceneInfo->findScene(_cur_scene);
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
