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

#ifndef ALG_GAME_H
#define ALG_GAME_H

#include "common/random.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "alg/alg.h"
#include "alg/scene.h"
#include "alg/video.h"

namespace Alg {

class Game {

public:
	Game(AlgEngine *vm);
	virtual ~Game();
	byte _debug_drawRects = 0;
	byte _debug_godMode = 0;

protected:
	virtual void init() = 0;

	AlgEngine *_vm;
	AlgVideoDecoder *_videoDecoder;
	SceneInfo *_sceneInfo;
	Common::RandomSource *_rnd;

	Common::File _libFile;
	Common::HashMap<Common::String, uint32> _libFileEntries;

	uint8 *_palette;
	bool _paletteDirty;

	Graphics::Surface *_background;
	Graphics::Surface *_screen;

	Audio::SoundHandle _sfxAudioHandle;
	Audio::SoundHandle _sceneAudioHandle;

	uint _lastMouseButton;
	Common::Point _lastMousePos;

	bool pollEvents();
	void loadLibArchive(const Common::Path &path);
	Audio::SeekableAudioStream *_LoadSoundFile(const Common::Path &path);
	void _PlaySound(Audio::SeekableAudioStream *stream);
	void loadScene(Scene *scene);
	void debug_drawZoneRects();
	void updateScreen();

	bool _butdwn = false;
	uint32 _frm; // TODO: rename to _currentFrame
	uint32 _game_timer = 0;
	bool _had_pause = false;
	bool _in_menu = false;
	uint32 _pause_time = 0;
	int32 _pp_force = 0;   // TODO: obsolete, remove
	bool _ss_flag = false; // TODO: obsolete, remove
	int32 _pp_flgs = 0;    // TODO: obsolete, remove
	uint32 _videoFrameSkip = 3;
	uint32 _nextFrameTime = 0;
	uint16 _videoPosX;
	uint16 _videoPosY;

	Common::String _cur_scene;
	Common::String _sub_scene;
	Common::String _ret_scene;
	Common::String _startscene;
};

} // End of namespace Alg

#endif
