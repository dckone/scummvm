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

#ifndef ALG_GAME_H
#define ALG_GAME_H

#include "common/keyboard.h"
#include "common/queue.h"
#include "common/random.h"
#include "common/str.h"

#include "alg/scene.h"
#include "alg/video.h"

namespace Alg {

class AlgEngine;

class Game {
public:
	Game(AlgEngine *vm);
	virtual ~Game();
	Common::Error run();
	byte _debug_drawRects = 1; // TODO: set to 0
	byte _debug_godMode = 1; // TODO: set to 0

protected:
	virtual void init() = 0;
	virtual void updateScreen() = 0;
	virtual void beforeScene(Scene *scene) = 0;
	virtual void duringScene(Scene *scene, uint32 currentFrame) = 0;
	virtual void afterScene(Scene *scene) = 0;
	virtual void rectHit(Rect *rect, int16 x, int16 y) = 0;
	bool isPaused() { return _paused; }
	uint16 getPausedFrames() { return _pausedFrames; }
	void decPausedFrames() { _pausedFrames--; }
	bool isSkipToNextScene() { return _skipToNextScene; }
	bool isSkippable() { return _skippable; }
	Common::String findNextScene();
	Common::String getNewGameScene() { return _newGameScene; }

	uint32 pickRandomFromMask(int range, uint32 *mask);
	void ChangeDifficulty(int difficulty);

	AlgEngine *_vm;
	Common::RandomSource *_rnd;

	AlgVideoDecoder *_videoDecoder;
	SceneInfo *_sceneInfo;

	Common::File _libFile;
	Common::Array<Common::String> _libFileNames;
	Common::Array<uint32> _libFileOffsets;
	uint8 *_palette;
	bool _paletteDirty;
	Graphics::Surface *_screen;
	Graphics::Surface *_background;

	Common::Queue< Common::KeyState > _keysPressed;
	uint _lastMouseButton;

	void initialize();
	void loadLibArchive(const Common::Path &path);
	void load8bAudioSample(const Common::Path &path);
	void loadScene(Scene *scene);

	void debug_drawZoneRects();

	bool pollEvents();
	bool checkKeysPressed();
	void clearKeys() { _keysPressed.clear(); }
	Common::KeyState getNextKey();
	uint getCurrentMouseButton() { return _lastMouseButton; }
	bool mouseClicked(int16 x, int16 y, uint32 currentFrame);

	bool _gunTime = 0;
	bool _gameRunning = true;

	// verified
	bool _had_pause = false;
	uint32 _pause_time = 0;

	// unverified
	uint8 _difficulty = 0;
	uint8 _player = 3;
	uint32 _shots = 0;
	bool _holster = false;
	uint32 _score = 0;
	uint8 _totalAmmo = 0;
	uint8 _ammoLeft = 0;
	bool _paused = false;
	bool _in_menu = false;
	uint32 _pausedFrames = 0;
	bool _had_pause = false;
	bool _skipToNextScene = false;
	bool _skippable = false;
	bool _rectHit = false;

	Common::String _cur_scene;
	Common::String _next_scene;
	Common::String _sub_scene;
	Common::String _ret_scene;
	Common::String _startscene;

	void _scene_default_nxtscn(Scene *scene);
	void _scene_nxtscn_drawgun(Scene *scene);

	void _scene_po_pause(Scene *scene);
	void _scene_pso_paus_pr(Scene *scene);
	void _scene_pso_paus_fi(Scene *scene);
	void _scene_pso_fadein(Scene *scene);
	void _scene_pso_preread(Scene *scene);

	void _scene_iso_pause(Scene *scene, uint32 currentFrame);
	void _scene_iso_startgame(Scene *scene, uint32 currentFrame);
	void _scene_iso_shootpast(Scene *scene, uint32 currentFrame);
	void _scene_iso_spause(Scene *scene, uint32 currentFrame);

	void _rect_newscene();
	void _rect_shotmenu();
	void _rect_start();
	void _rect_continue();
	void _rect_exit();
	void _rect_save();
	void _rect_load();
	void _rect_easy();
	void _rect_average();
	void _rect_hard();
};

} // End of namespace Alg

#endif
