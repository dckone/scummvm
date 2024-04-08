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

#ifndef CRYO_HARDLINE_H
#define CRYO_HARDLINE_H

#include "common/array.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/queue.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/substream.h"
#include "graphics/surface.h"
#include "video/hnm_decoder.h"

#include "cryo/defs.h"
#include "cryo/sound.h"

namespace Cryo {

class CryoEngine;

class HardlineGame {
public:
	HardlineGame(CryoEngine *vm);
	~HardlineGame();

	void run();

private:
	class FontCharacter {
	public:
		FontCharacter(uint8 width, uint8 height, Common::Array<uint8> *bytes) { _width = width; _height = height; _bytes = bytes; }
		uint8 getWidth() { return _width; }
		uint8 getHeight() { return _height; }
		Common::Array<uint8> *getBytes() { return _bytes; }

	private:
		uint8 _width;
		uint8 _height;
		Common::Array<uint8> *_bytes;
	};

	class Animation {
	public:
		Animation(uint16 frameCount, uint32 spriteOffset) {_frameCount = frameCount; _spriteOffset = spriteOffset;}
		uint8 getFrameCount() { return _frameCount; }
		uint8 getSpriteOffset() { return _spriteOffset; }

	private:
		uint16 _frameCount;
		uint32 _spriteOffset;
	};

	class ScriptAction {
	public:
		ScriptAction(uint8 type, uint32 param1, uint32 param2, uint32 param3);
		uint8 getType() { return _type; }
		uint32 getParam1() { return _param1; }
		uint32 getParam2() { return _param2; }
		uint32 getParam3() { return _param3; }

	private:
		uint8 _type;
		uint32 _param1;
		uint32 _param2;
		uint32 _param3;
	};

	class ScriptSection {
	public:
		ScriptSection();
		Common::Array<uint8> *getData() { return _data; }
		Common::Array<ScriptAction> *getActions() { return _actions; }
		void addData(uint8 data) { _data->push_back(data); }
		void addAction(ScriptAction *action) { _actions->push_back(*action); }

	private:
		Common::Array<uint8> *_data;
		Common::Array<ScriptAction> *_actions;
	};

	class SceneArea {
	public:
		SceneArea(uint8 type, uint8 scriptId, uint8 scriptSectionId, uint8 unk1, uint8 unk2, uint8 unk3, uint16 posX, uint16 posY, uint16 width, uint16 height);
		uint8 getType() { return _type; }
		uint8 getScriptId() { return _scriptId; }
		uint8 getScriptSectionId() { return _scriptSectionId; }
		uint8 getUnk1() { return _unk1; }
		uint8 getUnk2() { return _unk2; }
		uint8 getUnk3() { return _unk3; }
		uint16 getPosX() { return _posX; }
		uint16 getPosY() { return _posY; }
		uint16 getWidth() { return _width; }
		uint16 getHeight() { return _height; }

	private:
		uint8 _type;
		uint8 _scriptId;
		uint8 _scriptSectionId;
		uint8 _unk1;
		uint8 _unk2;
		uint8 _unk3;
		uint16 _posX;
		uint16 _posY;
		uint16 _width;
		uint16 _height;
	};

	CryoEngine *_vm;

	bool _quitFlag;

	uint8 _currentBigfile;
	uint32 _currentScene;

	Common::File *_bigFile;
	Common::SafeSeekableSubReadStream *_hnmReadStream;
	Video::HNMDecoder *_videoDecoder;

	Graphics::Surface *_screen;

	Common::Array< Common::Array<uint32> > _bigfileOffsets;
	Common::Array< Graphics::Surface > _sprites;
	Common::Array< Animation > _animations;
	Common::Array< Common::Array<ScriptSection> > _scripts;
	Common::Array<SceneArea> _sceneAreas;
	Common::Array<FontCharacter> _font;

	Common::Queue< Common::KeyState > _keysPressed;
	uint _lastMouseButton;

	void initialize();
	void shutdown();
	void intro();
	void gameLoop();

	void executeScriptSection(uint8 scriptId, uint32 scriptSectionId);

	void drawCurrentScene();
	void drawCursor();

	void loadPlnFile(uint8 id);
	void loadZrcFile(uint8 id);
	void loadSmallFnt();
	void loadRegleTxt();
	void loadObjObt();
	void loadSpriteBank();
	void loadSectTse();
	void loadTanimDat();
	void loadWavFiles();

	void loadMovie(uint8 bigfileId, uint32 movieId, bool loop);
	void playMovie(uint8 bigfileId, uint32 movieId);
	const Graphics::Surface *getNextMovieFrame();

	bool pollEvents();
	bool checkKeysPressed();
	void clearKeys() { _keysPressed.clear(); }
	Common::KeyState getNextKey();
	uint getCurrentMouseButton() { return _lastMouseButton; }

	// Debug methods
	void debug_renderBoxesToScreen();
};

} // namespace Cryo

#endif
