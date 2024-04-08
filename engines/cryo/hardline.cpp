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

#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/substream.h"
#include "graphics/screen.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"
#include "audio/mixer.h"
#include "image/codecs/hlz.h"
#include "image/bmp.h"
#include "video/hnm_decoder.h"

#include "cryo/defs.h"
#include "cryo/cryo.h"
#include "cryo/platdefs.h"
#include "cryo/cryolib.h"
#include "cryo/sound.h"

#include "cryo/hardline.h"

namespace Cryo {

#define TOTAL_BIGFILES 39

#define GAME_START_BIGFILE 1
#define GAME_START_SCENE 7

// #define GAME_START_BIGFILE 2
// #define GAME_START_SCENE 31

#define SCRIPT_ACTION_TYPE_PLAYMOVIE 1
#define SCRIPT_ACTION_TYPE_GOTOSCENE 4

HardlineGame::HardlineGame(CryoEngine *vm) : _vm(vm) {
}

HardlineGame::~HardlineGame() {
}

void HardlineGame::run() {
	_quitFlag = false;

	debug("starting initialization phase");
	initialize();
	debug("finished initialization phase");

	debug("starting intro");
	// intro();
	debug("finished intro");

	debug("starting main game loop");
	while (!_quitFlag) {
		gameLoop();
	}
	debug("finished main game loop");

	shutdown();
}

void HardlineGame::initialize() {

	_bigfileOffsets.reserve(TOTAL_BIGFILES);

	_bigFile = new Common::File;
	_hnmReadStream = nullptr;
	_videoDecoder = new Video::HNMDecoder(g_system->getScreenFormat(), false, nullptr);

	_currentBigfile = GAME_START_BIGFILE;
	_currentScene = GAME_START_SCENE;

	_screen = new Graphics::Surface;
	_screen->create(320, 150, Graphics::PixelFormat::createFormatCLUT8());

	debug("loading PLN files");
	for (int i = 0; i < TOTAL_BIGFILES; i++) {
		loadPlnFile(i);
	}
	debug("finished loading PLN files");

	debug("loading ZRC files");
	for (int i = 0; i < TOTAL_BIGFILES; i++) {
		loadZrcFile(i);
	}
	debug("finished loading ZRC files");

	debug("loading small.fnt");
	loadSmallFnt();
	debug("finished loading small.fnt");

	debug("loading regle.txt");
	loadRegleTxt();
	debug("finished loading regle.txt");

	debug("loading obj.obt");
	loadObjObt();
	debug("finished loading obj.obt");

	debug("loading and decompressing sprite bank");
	loadSpriteBank();
	debug("finished loading and decompressing sprite bank");

	debug("loading sect.tse");
	loadSectTse();
	debug("finished loading sect.tse");

	// TODO: CHECK: SAV file loading occurs in the game here. i doubt they are needed.

	debug("loading tanim.dat");
	loadTanimDat();
	debug("finished loading tanim.dat");

	// TODO: IMPLEMENT: start loading of WAV files here
}

void HardlineGame::shutdown() {
	_bigfileOffsets.clear();
	_scripts.clear();
	_sprites.clear();
	_animations.clear();
	_screen->free();
	delete _videoDecoder;
}

void HardlineGame::intro() {
	// TODO: Escape skips all of these at once and jumps directly to the first scene
	playMovie(1, 25);
	playMovie(1, 24);
	playMovie(1, 31);
	playMovie(1, 27);
	playMovie(1, 28);
	playMovie(1, 26);
}

void HardlineGame::loadPlnFile(uint8 plnId) {

	Common::File plnFile;

	Common::Path plnFilePath = Common::Path(Common::String::format("PLNZRC/PLANS%d.PLN", plnId));

	debug("parsing PLN file %s", plnFilePath.toString().c_str());

	plnFile.open(plnFilePath);

	assert(plnFile.isOpen());

	uint32 bytesRead = 0;

	int count1 = plnFile.readUint16LE();
	int count2 = plnFile.readUint16LE();

	assert(count1 > 0 && count2 == count1);

	bytesRead += 4;

	Common::Array<uint32> *offsets = new Common::Array<uint32>(0);

	offsets->reserve(count1);

	for (int i = 0; i < count1; i++) {
		uint8 unknown = plnFile.readByte();
		uint32 offset = plnFile.readUint32LE();
		// debug("PLN unknown byte: %d", unknown);
		assert(unknown >= 0 && unknown <= 4);
		offsets->push_back(offset);
		bytesRead += 5;
	}

	_bigfileOffsets.push_back(*offsets);

	assert(bytesRead == plnFile.size());

	plnFile.close();
}

void HardlineGame::loadZrcFile(uint8 zrcId) {

	Common::File zrcFile;

	Common::Path zrcFilePath = Common::Path(Common::String::format("PLNZRC/ZR%d.ZRC", zrcId));

	debug("parsing ZRC file %s", zrcFilePath.toString().c_str());

	zrcFile.open(zrcFilePath);

	assert(zrcFile.isOpen());

	int count = zrcFile.readByte();
	int count2 = zrcFile.readByte();

	assert(count > 0 && count2 == count);

	int bytesRead = 2;

	Common::Array<ScriptSection> scriptSections;

	scriptSections.reserve(count);

	int totalSectionSize = 0;

	for (int i = 0; i < count; i++) {
		uint8 sectionUnknown1 = zrcFile.readByte();
		uint16 sectionUnknown2 = zrcFile.readUint16LE();
		uint8 sectionSize = zrcFile.readByte();
		bytesRead += 4;
		// debug("sectionUnknown1: %d", sectionUnknown1);
		// debug("sectionUnknown2: %d", sectionUnknown2);
		// debug("sectionSize: %d", sectionSize);
		assert(sectionSize <= 6);
		totalSectionSize += sectionSize;
		ScriptSection *scriptSection = new ScriptSection();
		for (int j = 0; j < sectionSize; j++) {
			uint8 sectionData = zrcFile.readByte();
			bytesRead++;
			// debug("sectionData: %d", sectionData);
			scriptSection->addData(sectionData);
		}
		scriptSections.push_back(*scriptSection);
	}

	debug("totalSectionSize: %d", totalSectionSize);

	// debug("total sections: %d", count);

	for (int i = 0; i < count; i++) {
		Common::Array<uint8> *scriptSectionData = scriptSections[i].getData();
		for (uint32 j = 0; j < scriptSections[i].getData()->size(); j++) {
			uint8 actionSize = zrcFile.readByte();
			bytesRead++;
			for (int k = 0; k < actionSize; k++) {
				uint8 actionType = zrcFile.readByte();
				uint32 actionParam1 = 0;
				uint32 actionParam2 = 0;
				uint32 actionParam3 = 0;
				if(actionType == 13) {
					actionParam1 = zrcFile.readUint16LE();
					actionParam2 = zrcFile.readUint16LE();
					actionParam3 = zrcFile.readUint32LE();
				} else {
					actionParam1 = zrcFile.readUint32LE();
					actionParam2 = zrcFile.readUint32LE();
				}
				// debug("k: %d", k);
				// debug("actionType: %d", actionType);
				// debug("actionParam1: %d", actionParam1);
				// debug("actionParam2: %d", actionParam2);
				// debug("actionParam3: %d", actionParam3);
				bytesRead += 9;

				ScriptAction *scriptAction = new ScriptAction(actionType, actionParam1, actionParam2, actionParam3);
				scriptSections[i].addAction(scriptAction);
			}
		}
	}

	_scripts.push_back(scriptSections);

	assert(bytesRead == zrcFile.size());

	zrcFile.close();
}

void HardlineGame::loadSmallFnt() {

	Common::File fntFile;

	fntFile.open("DATA/SMALL.FNT");

	assert(fntFile.isOpen());

	uint32 count = fntFile.readUint32LE();
	uint32 height = fntFile.readUint32LE();

	fntFile.skip(8);

	for(uint32 i = 0; i < count; i++) {
		uint8 width = fntFile.readByte();
		Common::Array<uint8> bytes = Common::Array<uint8>(width * height);
		if(width) {
			for(uint8 y = 0; y < height; y++) {
				for(uint8 x = 0; x < width; x++) {
					bytes.push_back(fntFile.readByte());
				}
			}
		}
		FontCharacter character = FontCharacter(width, (uint8) height, &bytes);
		_font.push_back(character);
	}

	fntFile.close();
}

void HardlineGame::loadRegleTxt() {
	// TODO: implement
}

void HardlineGame::loadObjObt() {

	Common::File objObtFile;

	objObtFile.open("DATA/OBJ.OBT");

	assert(objObtFile.isOpen());

	uint32 bytesRead = 0;

	uint8 count1 = objObtFile.readByte();
	uint8 count2 = objObtFile.readByte();

	bytesRead += 2;

	assert(count1 == 50);
	assert(count2 == 22);

	for (uint32 i = 0; i < count2; i++) {
		uint8 objectId = objObtFile.readByte();
		uint16 spriteOffset = objObtFile.readUint16LE();
		bytesRead += 3;
		debug("i: %d\tobject Id %d\tspriteOffset %d", i, objectId, spriteOffset);
	}

	for (uint32 i = 0; i < count2; i++) {
		uint8 count3 = objObtFile.readByte();
		bytesRead++;
		for (uint32 j = 0; j < count3; j++) {
			uint8 unk3 = objObtFile.readByte();
			uint32 unk4 = objObtFile.readUint32LE();
			// uint16 unk40 = objObtFile.readByte();
			// uint16 unk41 = objObtFile.readByte();
			// uint16 unk42 = objObtFile.readByte();
			// uint16 unk43 = objObtFile.readByte();
			uint32 unk5 = objObtFile.readUint32LE();
			// debug("i: %d\tj: %d\tsub-record: %d\t%d\t%d\t%d\t%d\t%d", i, j, unk3, unk40, unk41, unk42, unk43, unk5);
			bytesRead += 9;
			debug("i: %d\tj: %d\tsub-record: %d\t%d\t%d", i, j, unk3, unk4, unk5);
		}
	}

	assert(bytesRead == objObtFile.size());

	objObtFile.close();
}

void HardlineGame::loadSpriteBank() {

	Common::File spriteBankFile;

	spriteBankFile.open("DATA/BANK.SPR");

	assert(spriteBankFile.isOpen());

	uint32 count = spriteBankFile.readUint32LE();

	_sprites.reserve(count);

	Common::Array<uint32> offsets;

	for (uint32 i = 0; i < count; i++) {
		uint32 offset = spriteBankFile.readUint32LE();
		offsets.push_back(offset);
	}

	for (uint32 i = 0; i < count; i++) {
		spriteBankFile.seek(offsets[i]);
		uint32 size = spriteBankFile.readUint32LE();
		uint16 width = spriteBankFile.readUint16LE();
		uint16 height = spriteBankFile.readUint16LE();
		spriteBankFile.skip(5); // 5 unknown bytes
		byte *bytes = new byte[width * height]();
		Image::HLZDecoder::decodeFrameInPlace(*spriteBankFile.readStream(size - 13), size - 13, bytes);

		Graphics::Surface *sprite = new Graphics::Surface;
		sprite->init(width, height, width, bytes, Graphics::PixelFormat::createFormatCLUT8());
		_sprites.push_back(*sprite);
	}

	spriteBankFile.close();

	/*
	// dump all sprites to PGM
	for (uint32 i = 0; i < count; i++) {
		Common::String dumpFileName = Common::String::format("SPRITE%08d.pgm", i);
		Common::String dumpFileHeader = Common::String::format("P5\n#\n%d %d\n255\n", _sprites[i].w, _sprites[i].h);
		Common::DumpFile dumpFile;
		dumpFile.open(dumpFileName);
		dumpFile.writeString(dumpFileHeader);
		for (int32 j = 0; j < _sprites[i].w * _sprites[i].h; j++) {
			dumpFile.writeByte(((byte *)_sprites[i].getPixels())[j]);
		}
	}
	*/
}

void HardlineGame::loadSectTse() {

	Common::File sectTseFile;

	sectTseFile.open("DATA/SECT.TSE");

	assert(sectTseFile.isOpen());

	uint32 bytesRead = 0;

	uint32 count = sectTseFile.readUint32LE();

	bytesRead += 4;

	for (uint32 i = 0; i < count; i++) {
		uint32 unk1 = sectTseFile.readUint32LE();
		uint32 unk2 = sectTseFile.readUint32LE();
		uint32 unk3 = sectTseFile.readUint32LE();
		uint32 unk4 = sectTseFile.readUint32LE();
		uint32 unk5 = sectTseFile.readUint32LE();
		uint32 unk6 = sectTseFile.readUint32LE();

		bytesRead += 24;

		assert(unk1 <= 88);
		assert(unk2 >= 1 || unk2 <= 87);
		assert(unk3 >= 2 || unk4 <= 87);
		assert(unk4 == 0 || unk4 == 5);
		assert(unk5 == 5 || unk5 == 32);
		assert(unk6 == 0);

		debug("%d\t%d\t%d\t%d\t%d\t%d", unk1, unk2, unk3, unk4, unk5, unk6);
	}

	assert(bytesRead == sectTseFile.size());

	sectTseFile.close();
}

void HardlineGame::loadTanimDat() {

	Common::File tanimDatFile;

	tanimDatFile.open("DATA/TANIM.DAT");

	assert(tanimDatFile.isOpen());

	uint32 bytesRead = 0;

	uint32 count = tanimDatFile.readUint32LE();

	bytesRead += 4;

	_animations.reserve(count);

	for (uint32 i = 0; i < count; i++) {
		uint16 frameCount = tanimDatFile.readUint16LE();
		uint32 spriteOffset = tanimDatFile.readUint32LE();
		bytesRead += 6;
		_animations.push_back(Animation(frameCount, spriteOffset));
	}

	assert(bytesRead == tanimDatFile.size());
}

void HardlineGame::loadWavFiles() {
	// TODO: IMPLEMENT
}

void HardlineGame::loadMovie(uint8 bigfileId, uint32 movieId, bool loop) {

	_videoDecoder->setLoop(loop);

	uint32 offset = _bigfileOffsets[bigfileId][movieId];

	Common::Path bigfilePath = Common::Path(Common::String::format("LIEUX/%d.BF", bigfileId));

	if (_bigFile->isOpen()) {
		_bigFile->close();
	}

	_bigFile->open(bigfilePath);

	if (!_bigFile->isOpen()) {
		warning("Could not load bigfile %s", bigfilePath.toString().c_str());
		return;
	}

	debug("loading HNM4 movie at offset %d, bigfileId %d movieId %d", offset, bigfileId, movieId);

	_hnmReadStream = new Common::SafeSeekableSubReadStream(_bigFile, offset, _bigFile->size(), DisposeAfterUse::NO);

	if (!_videoDecoder->loadStream(_hnmReadStream)) {
		warning("Could not load HNM4 movie");
		delete _hnmReadStream;
		return;
	}

	_videoDecoder->start();
}

const Graphics::Surface *HardlineGame::getNextMovieFrame() {

	const Graphics::Surface *frame = nullptr;

	if (_videoDecoder->needsUpdate()) {
		frame = _videoDecoder->decodeNextFrame();
		if (frame) {
			if (_videoDecoder->hasDirtyPalette()) {
				const byte *palette = _videoDecoder->getPalette();
				g_system->getPaletteManager()->setPalette(palette, 0, 256);
			}
		}
	}

	return frame;
}

void HardlineGame::playMovie(uint8 bigfileId, uint32 movieId) {

	loadMovie(bigfileId, movieId, false);

	while (!_videoDecoder->endOfVideo()) {

		if (pollEvents() && checkKeysPressed()) {
			// TODO: check for movie skip. special case for intro
		}

		const Graphics::Surface *frame = getNextMovieFrame();
		if (frame) {
			_screen->copyFrom(*frame);
			g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _videoDecoder->getWidth(), _videoDecoder->getHeight());
			g_system->updateScreen();
		}
	}
}

bool HardlineGame::pollEvents() {
	Common::Event event;
	int buttonMask;
	bool hasEvents = false;

	// Don't take into transitional clicks for the drag
	buttonMask = g_system->getEventManager()->getButtonState();

	int transitionalMask = 0;

	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_KEYDOWN) {
			_keysPressed.push(event.kbd);
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			transitionalMask |= Common::EventManager::LBUTTON;
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			transitionalMask |= Common::EventManager::RBUTTON;
		}
		hasEvents = true;
	}

	// Merge current button state with any buttons pressed since last poll
	// That's to avoid missed clicks
	buttonMask = g_system->getEventManager()->getButtonState() |
				 transitionalMask;
	if (buttonMask & 0x1) {
		_lastMouseButton = 1;
	} else if (buttonMask & 0x2) {
		_lastMouseButton = 2;
	} else {
		_lastMouseButton = 0;
	}

	return hasEvents;
}

Common::KeyState HardlineGame::getNextKey() {
	if (_keysPressed.empty()) {
		return Common::KeyState();
	} else {
		return _keysPressed.pop();
	}
}

bool HardlineGame::checkKeysPressed() {
	Common::KeyCode kc = getNextKey().keycode;
	if (kc != Common::KEYCODE_INVALID) {
		clearKeys();
		return true;
	} else {
		return false;
	}
}

HardlineGame::ScriptAction::ScriptAction(uint8 type, uint32 param1, uint32 param2, uint32 param3) {
	switch (type) {
	case SCRIPT_ACTION_TYPE_PLAYMOVIE:
	case SCRIPT_ACTION_TYPE_GOTOSCENE:
		assert(param2 == 0);
		assert(param3 == 0);
		break;
	case 13:
		assert(param3 == 0);
		break;
	default:
		assert(param3 == 0);
		warning("Unknown script action encountered. type: %d, param1: %d, param2: %d, param3: %d", type, param1, param2, param3);
	}

	_type = type;
	_param1 = param1;
	_param2 = param2;
	_param3 = param3;
}

HardlineGame::ScriptSection::ScriptSection() {
	_data = new Common::Array<uint8>();
	_actions = new Common::Array<ScriptAction>();
}

HardlineGame::SceneArea::SceneArea(uint8 type, uint8 scriptId, uint8 scriptSectionId, uint8 unk1, uint8 unk2, uint8 unk3,
	uint16 posX, uint16 posY, uint16 width, uint16 height) {

	_type = type;
	_scriptId = scriptId;
	_scriptSectionId = scriptSectionId;
	_unk1 = unk1;
	_unk2 = unk2;
	_unk3 = unk3;
	_posX = posX;
	_posY = posY;
	_width = width;
	_height = height;

	// debug("type %d, scriptId %d, unk1 %d, unk2 %d, unk3 %d, posX %d, posY %d, width %d, height %d",
	//	type, scriptId, unk1, unk2, unk3, posX, posY, width, height);

	assert(type == 1);
	assert(scriptId < 38);
	assert(unk1 == 2);
	assert(unk2 == scriptId);
	assert(unk3 == scriptSectionId);
	assert(posX <= 320);
	assert(posY <= 200);
	assert(width <= 320);
	assert(height <= 200);

	// TODO: //FIXME: everything is shifted by 25 pixels because of screen bars
	_posY -= 25;
}

void HardlineGame::debug_renderBoxesToScreen() {
	byte paletteUpdate[3];
	paletteUpdate[0] = 0xFF;
	paletteUpdate[1] = 0x00;
	paletteUpdate[2] = 0xFF;
	g_system->getPaletteManager()->setPalette(paletteUpdate, 255, 1);
	for (uint32 i = 0; i < _sceneAreas.size(); i++) {
		_screen->drawLine(_sceneAreas[i].getPosX(), _sceneAreas[i].getPosY(), _sceneAreas[i].getPosX() + _sceneAreas[i].getWidth(), _sceneAreas[i].getPosY(), 255);
		_screen->drawLine(_sceneAreas[i].getPosX(), _sceneAreas[i].getPosY(), _sceneAreas[i].getPosX(), _sceneAreas[i].getPosY() + _sceneAreas[i].getHeight(), 255);
		_screen->drawLine(_sceneAreas[i].getPosX() + _sceneAreas[i].getWidth(), _sceneAreas[i].getPosY() + _sceneAreas[i].getHeight(), _sceneAreas[i].getPosX() + _sceneAreas[i].getWidth(), _sceneAreas[i].getPosY(), 255);
		_screen->drawLine(_sceneAreas[i].getPosX() + _sceneAreas[i].getWidth(), _sceneAreas[i].getPosY() + _sceneAreas[i].getHeight(), _sceneAreas[i].getPosX(), _sceneAreas[i].getPosY() + _sceneAreas[i].getHeight(), 255);
	}
}

void HardlineGame::gameLoop() {

	/*
	for (int i = 1; i < TOTAL_BIGFILES; i++) {
		for (uint32 j = 0; j < _bigfileOffsets[i].size(); j++) {
			uint32 bigfileOffset = _bigfileOffsets[i][j];
			if (bigfileOffset == 0xFFFFFFFF) {
				warning("offset number %d in bigfile %d is 0xFFFFFFFF, skipping!!!", j, i);
			} else {
				playMovie(i, j);
			}
		}
	}
	*/

	while (!_quitFlag) {
		drawCurrentScene();
	}

	_quitFlag = true;
}

void HardlineGame::executeScriptSection(uint8 scriptId, uint32 scriptSectionId) {

	Common::Array<ScriptAction> actions = *(_scripts[scriptId][scriptSectionId].getActions());

	debug("executeScriptSection: %d, %d", scriptId, scriptSectionId);

	debug("actions.size(): %d", actions.size());

	for (uint32 i = 0; i < actions.size(); i++) {
		ScriptAction action = actions[i];
		switch (action.getType()) {
		case SCRIPT_ACTION_TYPE_PLAYMOVIE:
			playMovie(_currentBigfile, action.getParam1());
			break;
		case SCRIPT_ACTION_TYPE_GOTOSCENE:
			debug("SCRIPT_ACTION_TYPE_GOTOSCENE: %d", action.getParam1());
			_currentScene = action.getParam1();
			break;
		case 8: // TODO: something related to fighting
			// TODO: implement
			warning("not implemented: opcode 8. fight related");
			break;
		case 13: // TODO: change bigfile? // sceneId 2 bytes, bigfileId 2 bytes?
			_currentScene = action.getParam1();
			_currentBigfile = action.getParam2();
			break;
		case 31: // TODO: here is a closed door!
			break;
		case 41: // TODO: here i get a gun!
			playMovie(_currentBigfile, action.getParam1());
			break;
		default:
			warning("unknown action: %d %d %d", action.getType(), action.getParam1(), action.getParam2());
			break;
		}
	}
}

void HardlineGame::drawCurrentScene() {

	loadMovie(_currentBigfile, _currentScene, true);

	bool sceneIsActive = true;

	while (sceneIsActive) {

		const Graphics::Surface *frame = getNextMovieFrame();
		if(!frame) {
			frame = getNextMovieFrame();
		}
		if (frame) {
			_screen->copyFrom(*frame);
		}

		drawCursor();
		g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _videoDecoder->getWidth(), _videoDecoder->getHeight());
		debug_renderBoxesToScreen();
		g_system->updateScreen();

		if (_videoDecoder->extraDataBufferSCSize > 0) {
			_sceneAreas.clear();
			Common::MemoryReadStream memoryReadStream = Common::MemoryReadStream(_videoDecoder->extraDataBufferSC, _videoDecoder->extraDataBufferSCSize, DisposeAfterUse::NO);
			Common::String dump = "";
			for (uint32 i = 0; i < _videoDecoder->extraDataBufferSCSize; i += 14) {
				uint8 type = memoryReadStream.readByte();
				uint8 scriptId = memoryReadStream.readByte();
				uint8 scriptSectionId = memoryReadStream.readByte();
				uint8 unk1 = memoryReadStream.readByte();
				uint8 unk2 = memoryReadStream.readByte();
				uint8 unk3 = memoryReadStream.readByte();
				uint16 posX = memoryReadStream.readUint16LE();
				uint16 posY = memoryReadStream.readUint16LE();
				uint16 width = memoryReadStream.readUint16LE();
				uint16 height = memoryReadStream.readUint16LE();
				if(type == 1) {
					_sceneAreas.push_back(SceneArea(type, scriptId, scriptSectionId, unk1, unk2, unk3, posX, posY, width, height));
				}
			}
		}

		if (_videoDecoder->extraDataBufferSASize > 0) {
			Common::MemoryReadStream memoryReadStream = Common::MemoryReadStream(_videoDecoder->extraDataBufferSA, _videoDecoder->extraDataBufferSASize, DisposeAfterUse::NO);
			int32 i = _videoDecoder->extraDataBufferSASize;
			while(i > 0) {
				Common::String dump = "";
				uint16 size = memoryReadStream.readUint16LE();
				// assert(size < 256);
				i -= 2;
				for(uint8 j = 2; j < size; j++) {
					uint8 byte = memoryReadStream.readByte();
					dump = dump.format("%s%02x", dump.c_str(), byte);
					i--;
				}
				// debug("segment = %s", dump.c_str());
			}
			assert(i == 0);
		}

		if (pollEvents() && checkKeysPressed()) {
			// skipVideo = true;
		}

		if(_vm->shouldQuit()) {
			_quitFlag = true;
			return;
		}

		clearKeys(); // TODO: remove

		if (_lastMouseButton) {
			int16 mouseX;
			int16 mouseY;
			_vm->getMousePosition(&mouseX, &mouseY);

			for (uint32 i = 0; i < _sceneAreas.size(); i++) {
				if (mouseX >= _sceneAreas[i].getPosX() && mouseX <= _sceneAreas[i].getPosX() + _sceneAreas[i].getWidth()
					&& mouseY >= _sceneAreas[i].getPosY() && mouseY <= _sceneAreas[i].getPosY() + _sceneAreas[i].getHeight()) {
					debug("We clicked into scene action %d", i);
					debug("Running script %d, scriptSection %d", _sceneAreas[i].getScriptId(), _sceneAreas[i].getScriptSectionId());
					sceneIsActive = false;
					executeScriptSection(_sceneAreas[i].getScriptId(), _sceneAreas[i].getScriptSectionId());
					break;
				}
			}
		}
	}
}

void HardlineGame::drawCursor() {

	Graphics::Surface sprite = _sprites[1998];

	int16 mouseX;
	int16 mouseY;
	_vm->getMousePosition(&mouseX, &mouseY);

	int16 x = mouseX - (sprite.w / 2);
	int16 y = mouseY - (sprite.h / 2);

	if (x < 0) {
		x = 0;
	} else if (x > 320 - sprite.w) {
		x = 320 - sprite.w;
	}

	if (y < 0) {
		y = 0;
	} else if (y > 150 - sprite.h) {
		y = 150 - sprite.h;
	}

	_screen->copyRectToSurface(sprite, x, y, Common::Rect(0, 0, 26, 22));
}

} // namespace Cryo
