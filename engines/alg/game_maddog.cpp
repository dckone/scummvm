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
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"

#include "alg/game_maddog.h"
#include "alg/graphics.h"
#include "alg/scene.h"

#define MADDOG_VIDEO_POS_X 56
#define MADDOG_VIDEO_POS_Y 8

#define MAP_NONE 0
#define MAP_LEFT 1
#define MAP_RIGHT 2

namespace Alg {

GameMaddog::GameMaddog(AlgEngine *vm) : Game(vm) {
}

GameMaddog::~GameMaddog() {
}

void GameMaddog::init() {
	debug("GameMaddog::init");
	loadLibArchive("MADDOG.LIB");
	_sceneInfo->loadScnFile("MADDOG.SCN");

	Zone *menuZone = new Zone();
	menuZone->ptrfb = "GLOBALHIT";
	menuZone->addRect(139, 60, 194, 70, "scene1", "0", "STARTBOT", "0");
	menuZone->addRect(139, 80, 194, 88, "scene1", "0", "STARTMENU", "0");
	menuZone->addRect(139, 97, 194, 106, "scene1", "0", "CONTMENU", "0");
	menuZone->addRect(230, 60, 284, 70, "scene1", "0", "RECTSAVE", "0");
	menuZone->addRect(230, 80, 284, 88, "scene1", "0", "RECTLOAD", "0");
	menuZone->addRect(230, 97, 284, 106, "scene1", "0", "EXITMENU", "0");
	menuZone->addRect(64, 50, 90, 80, "scene1", "0", "RECTEASY", "0");
	menuZone->addRect(64, 84, 90, 115, "scene1", "0", "RECTAVG", "0");
	menuZone->addRect(64, 116, 100, 140, "scene1", "0", "RECTHARD", "0");

	Scene *menuScene = new Scene();
	menuScene->name = "menu";
	menuScene->zones.push_back(*menuZone);
	_sceneInfo->addScene(menuScene);

	_background = AlgGraphics::loadVgaBackground("BACKGRND.VGA", _palette);

	_aniAmmo = AlgGraphics::loadAniImage("AMMO.ANI");
	_aniBullet = AlgGraphics::loadAniImage("BULLET.ANI");
	_aniClip = AlgGraphics::loadAniImage("CLIP.ANI");
	_aniHat = AlgGraphics::loadAniImage("HAT.ANI");
	_aniNumbers = AlgGraphics::loadAniImage("NUMBERS.ANI");
	_aniShootout = AlgGraphics::loadAniImage("SHOOTOUT.ANI");

	_aniGun = AlgGraphics::loadTransparentAniImage("GUN.ANI");
	_aniHole = AlgGraphics::loadTransparentAniImage("HOLE.ANI");
	_aniKnife = AlgGraphics::loadTransparentAniImage("KNIFE.ANI");

	Graphics::Surface aniGun0 = (*_aniGun)[8];
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.pushCursor(aniGun0.getPixels(), aniGun0.w, aniGun0.h, aniGun0.w / 2, aniGun0.h / 2, 0, false, &pixelFormat);
	CursorMan.showMouse(true);

	registerScriptFunctions();
}

void GameMaddog::registerScriptFunctions() {

#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb.push_back(new ScriptFunctionEntry(name, new ScriptFunction(this, &func)));
	ZONE_PTRFB_FUNCTION("DEFAULT", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("SKULL", _zone_skullhole);
	ZONE_PTRFB_FUNCTION("GLOBALHIT", _zone_globalhit);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs.push_back(new ScriptFunctionEntry(name, new ScriptFunction(this, &func)));
	RECT_HIT_FUNCTION("HIDEFRONT", _rect_hidefront);
	RECT_HIT_FUNCTION("HIDEREAR", _rect_hiderear);
	RECT_HIT_FUNCTION("NEWSCENE", _rect_newscene);
	RECT_HIT_FUNCTION("MENUSELECT", _rect_menuselect);
	RECT_HIT_FUNCTION("SKULL", _rect_skull);
	RECT_HIT_FUNCTION("KILLMAN", _rect_killman);
	RECT_HIT_FUNCTION("KILLWOMAN", _rect_killwoman);
	RECT_HIT_FUNCTION("PROSPSIGN", _rect_prospsign);
	RECT_HIT_FUNCTION("MINESIGN", _rect_minesign);
	RECT_HIT_FUNCTION("MINEITEM1", _rect_mineitem1);
	RECT_HIT_FUNCTION("MINEITEM2", _rect_mineitem2);
	RECT_HIT_FUNCTION("MINEITEM3", _rect_mineitem3);
	RECT_HIT_FUNCTION("MINELANTERN", _rect_minelantern);
	RECT_HIT_FUNCTION("SHOTHIDEOUT", _rect_shothideout);
	RECT_HIT_FUNCTION("SHOTRIGHT", _rect_shotright);
	RECT_HIT_FUNCTION("SHOTLEFT", _rect_shotleft);
	RECT_HIT_FUNCTION("SHOTMENU", _rect_shotmenu);
	RECT_HIT_FUNCTION("EXITMENU", _rect_exit);
	RECT_HIT_FUNCTION("CONTMENU", _rect_continue);
	RECT_HIT_FUNCTION("STARTMENU", _rect_start);
	RECT_HIT_FUNCTION("STARTBOT", _rect_startbottles);
	RECT_HIT_FUNCTION("DEFAULT", _rect_newscene);
#undef RECT_HIT_FUNCTION

#define PRE_OPS_FUNCTION(name, func) _scenePreOps.push_back(new ScriptFunctionEntry(name, new ScriptFunction(this, &func)));
	PRE_OPS_FUNCTION("DRAWRCT", _scene_po_drawrct);
	PRE_OPS_FUNCTION("PAUSE", _scene_po_pause);
	PRE_OPS_FUNCTION("PRESHOOTOUT", _scene_pso_shootout);
	PRE_OPS_FUNCTION("MDSHOOTOUT", _scene_pso_mdshootout);
	PRE_OPS_FUNCTION("FADEIN", _scene_pso_fadein);
	PRE_OPS_FUNCTION("PAUSFI", _scene_pso_paus_fi);
	PRE_OPS_FUNCTION("PREREAD", _scene_pso_preread);
	PRE_OPS_FUNCTION("PAUSPR", _scene_pso_paus_pr);
	PRE_OPS_FUNCTION("DEFAULT", _scene_po_drawrct);
#undef PRE_OPS_FUNCTION

#define INS_OPS_FUNCTION(name, func) _sceneInsOps.push_back(new ScriptFunctionEntry(name, new ScriptFunction(this, &func)));
	INS_OPS_FUNCTION("DEFAULT", _scene_iso_donothing);
	INS_OPS_FUNCTION("PAUSE", _scene_iso_pause);
	INS_OPS_FUNCTION("STARTGAME", _scene_iso_startgame);
	INS_OPS_FUNCTION("SKIPSALOON", _scene_iso_skipsaloon);
	INS_OPS_FUNCTION("SKIPSALOON2", _scene_iso_skipsaloon2);
	INS_OPS_FUNCTION("CHECKSALOON", _scene_iso_checksaloon);
	INS_OPS_FUNCTION("INTOSTABLE", _scene_iso_intostable);
	INS_OPS_FUNCTION("INTOFFICE", _scene_iso_intoffice);
	INS_OPS_FUNCTION("INTOBANK_SP", _scene_iso_intobank);
	INS_OPS_FUNCTION("CHKBARTNDR", _scene_iso_chkbartndr);
	INS_OPS_FUNCTION("DIDHIDEOUT", _scene_iso_didhideout);
	INS_OPS_FUNCTION("DIDSIGNPOST", _scene_iso_didsignpost);
	INS_OPS_FUNCTION("DOSHOOTOUT", _scene_iso_doshootout);
	INS_OPS_FUNCTION("MDSHOOTOUT", _scene_iso_mdshootout);
	INS_OPS_FUNCTION("SHOOTPAST", _scene_iso_shootpast);
	INS_OPS_FUNCTION("SPAUSE", _scene_iso_spause);
	INS_OPS_FUNCTION("SHOTINTO24", _scene_iso_shotinto24);
	INS_OPS_FUNCTION("SHOTINTO116", _scene_iso_shotinto116);
#undef INS_OPS_FUNCTION

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn.push_back(new ScriptFunctionEntry(name, new ScriptFunction(this, &func)));
	NXT_SCN_FUNCTION("DEFAULT", _scene_default_nxtscn);
	NXT_SCN_FUNCTION("PICKBOTTLE", _scene_nxtscn_pickbottle);
	NXT_SCN_FUNCTION("DIED", _scene_nxtscn_died);
	NXT_SCN_FUNCTION("AUTOSEL", _scene_nxtscn_autosel);
	NXT_SCN_FUNCTION("FINSALOON", _scene_nxtscn_finsaloon);
	NXT_SCN_FUNCTION("FINOFFICE", _scene_nxtscn_finoffice);
	NXT_SCN_FUNCTION("FINSTABLE", _scene_nxtscn_finstable);
	NXT_SCN_FUNCTION("FINBANK", _scene_nxtscn_finbank);
	NXT_SCN_FUNCTION("PICSALOON", _scene_nxtscn_picsaloon);
	NXT_SCN_FUNCTION("KILLMAN", _scene_nxtscn_killman);
	NXT_SCN_FUNCTION("KILLWOMAN", _scene_nxtscn_killwoman);
	NXT_SCN_FUNCTION("BANK", _scene_nxtscn_bank);
	NXT_SCN_FUNCTION("STABLE", _scene_nxtscn_stable);
	NXT_SCN_FUNCTION("SAVPROSP", _scene_nxtscn_savprosp);
	NXT_SCN_FUNCTION("PICKTOSS", _scene_nxtscn_picktoss);
	NXT_SCN_FUNCTION("HITTOSS", _scene_nxtscn_hittoss);
	NXT_SCN_FUNCTION("MISSTOSS", _scene_nxtscn_misstoss);
	NXT_SCN_FUNCTION("PICKSIGN", _scene_nxtscn_picksign);
	NXT_SCN_FUNCTION("BROCKMAN", _scene_nxtscn_brockman);
	NXT_SCN_FUNCTION("LROCKMAN", _scene_nxtscn_lrockman);
	NXT_SCN_FUNCTION("HOTELMEN", _scene_nxtscn_hotelmen);
	NXT_SCN_FUNCTION("DRAWGUN", _scene_nxtscn_drawgun);
#undef NXT_SCN_FUNCTION

	_sceneShowMsg.push_back(new ScriptFunctionEntry("DEFAULT", new ScriptFunction(this, &_scene_sm_donothing)));
	_sceneWepDwn.push_back(new ScriptFunctionEntry("DEFAULT", new ScriptFunction(this, &_scene_default_wepdwn)));
	_sceneScnScr.push_back(new ScriptFunctionEntry("DEFAULT", new ScriptFunction(this, &_scene_default_score)));
	_sceneNxtFrm.push_back(new ScriptFunctionEntry("DEFAULT", new ScriptFunction(this, &_scene_nxtfrm)));
}

void GameMaddog::updateScreen() {
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);
	if (!_in_menu) {
		Graphics::Surface *frame = _videoDecoder->getFrame();
		_screen->copyRectToSurface(frame->getPixels(), frame->pitch, MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y, frame->w, frame->h);
	}
	debug_drawZoneRects();

	Graphics::Surface aniHat0 = (*_aniHat)[0];
	_screen->copyRectToSurface(aniHat0.getPixels(), aniHat0.pitch, 0, 120, aniHat0.w, aniHat0.h);
	g_system->copyRectToScreen(_screen->getPixels(), _screen->w, 0, 0, _screen->w, _screen->h);
	if (_paletteDirty || _videoDecoder->isPaletteDirty()) {

		// TODO remove
		/*
		for(uint32 i = 0; i < 256 * 3; i += 3) {
			uint8 r = _palette[i];
			uint8 g = _palette[i + 1];
			uint8 b = _palette[i + 2];
			if(r == 0xFF && g == 0x00 && b == 0xFF) {
				debug("found pink entry: %d", i / 3);
			}
		}
		*/

		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
		_paletteDirty = false;
	}
	g_system->updateScreen();
}

void GameMaddog::beforeScene(Scene *scene) {
	_paused = false;
	_pausedFrames = 0;
	_hadPause = false;
	_skipToNextScene = false;
	_rectHit = false;

	if (scene->preop.equals("PAUSPR")) {
	} else {
		error("Unknown PREOP: %s", scene->preop.c_str());
	}
}

void GameMaddog::duringScene(Scene *scene, uint32 currentFrame) {
	if (scene->insop.equals("SHOTINTO116")) {
	} else {
		error("Unknown INSOP: %s", scene->insop.c_str());
	}
}

void GameMaddog::afterScene(Scene *scene) {
	if (scene->nxtscn.equals("DRAWGUN")) {
	} else {
		error("Unknown NXTSCN: %s", scene->nxtscn.c_str());
	}
}

void GameMaddog::rectHit(Rect *rect, int16 x, int16 y) {
	if (rect->function.equals("RECTHARD")) {
	} else {
		error("Unknown RECT function: %s", rect->function.c_str());
	}
	_rectHit = true;
}

void GameMaddog::UpdateStat() {
	// TODO implement
	/* 
    unsigned short i, offset;

    if (_lives != oldlive) {
        if (_lives > oldlive) {
            for (i = oldlive, offset = oldlive * 4; i < _lives; i++, offset += 4) {
                DrawAnImage(live_coords[offset/2], live_coords[offset/2 + 1], _liveicon, word_2817A, 0xA000);
            }
        } else {
            for (i = _lives, offset = _lives * 4; i < oldlive; i++, offset += 4) {
                DrawAnImage(live_coords[offset/2], live_coords[offset/2 + 1], _deadicon, word_28176, 0xA000);
            }
        }
        oldlive = _lives;
    }

    if (_shots != oldshots) {
        if (_shots > oldshots) {
            for (i = oldshots, offset = oldshots * 4; i < _shots; i++, offset += 4) {
                DrawAnImage(shot_coords[offset/2], shot_coords[offset/2 + 1], _shoticon, word_28182, 0xA000);
            }
        } else {
            for (i = _shots, offset = _shots * 4; i < oldshots; i++, offset += 4) {
                DrawAnImage(shot_coords[offset/2], shot_coords[offset/2 + 1], _emptyicon, word_2817E, 0xA000);
            }
        }
        oldshots = _shots;
    }
	*/
}

void GameMaddog::_default_bullethole(int x, y) {
    if (x >= 59 && y <= 166) {
        RestoreCursor();
        DrawCAnImage(x, y, _bullethole, word_2815E, _saveimage, 0xA000);
        DoCursor();
        _oldx = x;
        _oldx = y;
        _shotfired = 1;
        DoShot();
    }
}

Common::String GameMaddog::_die() {
    Common::String newScene = nullptr;
    UpdateStat();

    switch (_lives[player_index]) {
        case 2:
            newScene = "scene150";
            break;
        case 1:
            newScene = "scene152";
            break;
        case 0:
            newScene = "scene153";
            break;
        default:
			int nextSceneNum = (_rnd->getRandomNumber(1)) + 148;
			newScene = Common::String::format("scene%d", nextSceneNum);
            break;
    }

	_cur_scene = newScene;
}

uint32 _pick_rand(uint32 max, uint32 *mask_ptr) {
    uint32 result, mask;
    // reset mask_ptr if full
    if (*mask_ptr == (0xFF >> (8 - max))) {
        *mask_ptr = 0;
    }
    do {
        long random_val = rand() % 0x8000;
        result = (unsigned int)((random_val * max) / 0x8000);
        if (result >= max) {
            result = 0;
        }
        mask = 1 << result;

    } while (*mask_ptr & mask);

    *mask_ptr |= mask;
    return result * 2;
}

Common::String GameMaddog::_pick_town() {
    Common::String pickedScene = nullptr;

    _had_skull = 0;
    _map_pos = 0;
    _bad_men_bits = 0;
    _shoot_out_cnt++;

    if (_shoot_out_cnt % 5 == 0) {
        if (_shoot_out_cnt > 15 || _shoot_out_cnt <= 0) {
            _shoot_out_cnt = 5;
        }
		// surprise shootout!
		int pickedSceneNum = (_shoot_out_cnt / 5) + 106);
		pickedScene = Common::String::format("scene%d", pickedSceneNum);
    }
    else if (_been_to == 0) {
        pickedScene = "scene28";
    }
    else if (_been_to & 0x100) {
        pickedScene = "scene250";
    }
    else if (_been_to & 0x80) {
        pickedScene = "scene76";
    }
    else if (_been_to & 0x40) {
        pickedScene = "scene214";
    }
    else if (_been_to >= 15) {
        pickedScene = "scene186";
    }
    else {
		pickedScene = Common::String::format("scene%d", _been_to + 29);
    }

    return pickedScene;
}

Common::String GameMaddog::_pick_bad(uint32 max) {
    unsigned char shift = 8 - max;
    unsigned int mask = 0xFF >> shift;

    if (_bad_men_bits == mask) {
        _bad_men_bits = 0;
    }

    return _pick_rand(max, &_bad_men_bits);
}

Common::String GameMaddog::_pick_map() {
	Common::String pickedScene;
    uint32 random;

    _been_to |= 0x20;

    random = _rnd->getRandomNumber(5);

    switch (random) {
        case 0:
            _map0 = 1;
            _map1 = 1;
            _map2 = 0;
            pickedScene = "scene164";
            break;
        case 1:
            _map0 = 1;
            _map1 = -1;
            _map2 = 0;
            pickedScene = "scene165";
            break;
        case 2:
            _map0 = -1;
            _map1 = 1;
            _map2 = 1;
            pickedScene = "scene166";
            break;
        case 3:
            _map0 = -1;
            _map1 = 1;
            _map2 = -1;
            pickedScene = "scene167";
            break;
        case 4:
            _map0 = -1;
            _map1 = -1;
            _map2 = 1;
            pickedScene = "scene168";
            break;
        default:
            _map0 = -1;
            _map1 = -1;
            _map2 = -1;
            pickedScene = "scene169";
            break;
    }

    return pickedScene;
}

Common::String GameMaddog::_pick_sign() {
	Common::String pickedScene;

    _map_pos++;
    if (_map_pos > _max_map_pos) {
        _max_map_pos = _map_pos;
    }

    if (_map_pos <= 2 && (array_32B4[_map_pos][0] != 0 || array_32B4[_map_pos][1] != 0)) {
		pickedScene = Common::String::format("scene%d", _map_pos + 187);
    } else {
        pickedScene = "scene210";
    }

    return new_scene;
}

Common::String GameMaddog::_map_right() {
	Common::String pickedScene;
	int32 _mapArray[3] = {_map0, _map1, _map2};

    if (_mapArray[map_pos] == -1) {
        if (map_pos >= max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[map_pos * 2])
        } else {
            return pick_sign();
        }
    } else if (_mapArray[map_pos] == 0) {
        if (map_pos >= max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[map_pos * 2])
        } else {
            return pick_sign();
        }
    } else {
			pickedScene = Common::String::format("scene%d", _ambush[map_pos * 2])
    }

	return pickedScene
}

Common::String GameMaddog::_map_left() {
	Common::String pickedScene;
	int32 _mapArray[3] = {_map0, _map1, _map2};

    if (_mapArray[map_pos] == 1) {
        if (map_pos >= max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[map_pos * 2])
        } else {
            return pick_sign();
        }
    } else if (_mapArray[map_pos] == -1) {
			pickedScene = Common::String::format("scene%d", _ambush[map_pos * 2])
    } else {
        if (map_pos >= max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[map_pos * 2])
        } else {
            return pick_sign();
        }
    }

	return pickedScene
}

// Script functions: Zone
void GameMaddog::_zone_bullethole() {
	_default_bullethole(zone->x, zone->y);
}

void GameMaddog::_zone_skullhole() {
    if (zone->x >= 59 && zone->y <= 166) {
        RestoreCursor();
        DrawCAnImage(zone->x, zone->y, _bullethole, word_2815E, _saveimage, 0xA000);
        DoCursor();
        _oldx = zone->x;
        _oldx = zone->y;
        _shotfired = 1;

        if (_had_skull) {
            DoShot();
        } else {
            DoSkullSound();
        }
    }
}

void GameMaddog::_zone_globalhit() {
	// do nothing
}

// Script functions: RectHit
void GameMaddog::_rect_newscene() {
    _score += scene_info->score_add;
    _cur_scene = scene_info->next_scene;
}

void GameMaddog::_rect_hidefront() {
	if (_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_hiderear() {
	if (!_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_menuselect() {
	Common::String newScene = null;

	// TODO fixme
	int cursorX = ?;
	int cursorY = ?;

    if (cursorX < 184) {
        if (cursorY < 88) {
            if (_been_to & 2) return;
            if (_got_into & 2) {
                newScene = "scene130";
            } else {
                newScene = "scene122";
            }
        } else {
            if (_been_to & 8) return;
            if (_got_into & 8) {
                if (_been_to & 1) {
                    newScene = "scene118";
                } else {
                    newScene = "scene119";
                }
            } else {
                newScene = "scene114";
            }
        }
    } else {
        if (cursorY < 88) {
            if (_been_to & 1) return;
            if (_got_into & 1) {
                _pp_force = 3;
                newScene = "scene69b";
            } else {
                newScene = "scene67";
            }
        } else {
            if (_been_to & 4) return 0;
            if (_got_into & 4) {
                _pp_force = 3;
                _scene_nxtscn_bank();
				return;
            } else {
                newScene = "scene45";
            }
        }
    }

	_cur_scene = newScene;
}

void GameMaddog::_rect_skull() {
    if (_had_skull) {
        return;
    }

    _had_skull = true;

    if (_been_to < 15) {
        player_stats[_player] = 9;
    } else {
        player_stats[_player] = 12;
    }

    UpdateStat();
    return 0;
}

void GameMaddog::_rect_killman() {
	_scene_nxtscn_killman();
}

void GameMaddog::_rect_killwoman() {
	_scene_nxtscn_killwoman();
}

void GameMaddog::_rect_prospsign() {
    if (_been_to & 0x10) {
        return;
    }
    _gun_time = 1;
    _cur_scene = rect->new_scene;
}

void GameMaddog::_rect_minesign() {
    if (_been_to & 0x20) {
        return;
    }
    _gun_time = 1;
    _cur_scene = rect->new_scene;
}

void GameMaddog::_rect_mineitem1() {
    if (_pro_clue != 0) {
        _pause_time = 0;
        return;
    }

    if (_had_lantern) {
        _cur_scene = pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_mineitem2() {
    if (_pro_clue != 2) {
        _pause_time = 0;
        return;
    }

    if (_had_lantern) {
        _cur_scene = pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_mineitem3() {
    if (_pro_clue != 1) {
        _pause_time = 0;
        return 0;
    }

    if (_had_lantern) {
        _cur_scene = pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_minelantern() {
    _had_lantern = true;

    if (!_got_clue) {
        return;
    }

	_cur_scene = pick_map();
}

void GameMaddog::_rect_shothideout() {
	_cur_scene = pick_sign();
}

void GameMaddog::_rect_shotright() {
	error("Not implemented: _rect_shotright");
}

void GameMaddog::_rect_shotleft() {
	error("Not implemented: _rect_shotleft");
}

void GameMaddog::_rect_shotmenu() {
	error("Not implemented: _rect_shotmenu");
}

void GameMaddog::_rect_exit() {
	error("Not implemented: _rect_exit");
}

void GameMaddog::_rect_continue() {
	// TODO verify
	_in_menu = false;
	if (_player == 0) {
		rectStartGame();
		_cur_scene = pick_town();
	}
}

void GameMaddog::_rect_start() {
    _in_menu = false;
    _fired = false;
    _cur_scene = _startscene;
    _in_menu = false;
    ResetParams();
    NewGame();
    UpdateStat();
}

void GameMaddog::_rect_startbottles() {
    _in_menu = 0;
    _fired = 0;
    // ResetParams(); // TODO
    // NewGame(); // TODO
    UpdateStat();
	_cur_scene = "scene7";
}

// Script functions: Scene PreOps
void GameMaddog::_scene_po_drawrct() {
	error("Not implemented: _scene_po_drawrct");
}

void GameMaddog::_scene_po_pause() {
	if (scene->name.equals("scene28")) {
		_cur_scene = pick_town();
	}
	Game::_scene_po_pause(scene);
}

void GameMaddog::_scene_pso_shootout() {
	// TODO: verify
	_shots = 0;
	_in_shootout = true;
	sscanf(scene->preopParam.c_str(), "#%uto%u", &_shootoutMinF, &_shootoutMaxF);
	// TODO: change cursor
	// TODO: draw reload info to lower bar
}

void GameMaddog::_scene_pso_mdshootout() {
	error("Not implemented: _scene_pso_mdshootout");
}

void GameMaddog::_scene_pso_fadein() {
	error("Not implemented: _scene_pso_fadein");
}

void GameMaddog::_scene_pso_paus_fi() {
	error("Not implemented: _scene_pso_paus_fi");
}

void GameMaddog::_scene_pso_preread() {
	error("Not implemented: _scene_pso_preread");
}

void GameMaddog::_scene_pso_paus_pr() {
	error("Not implemented: _scene_pso_paus_pr");
}

// Script functions: Scene Scene InsOps
void GameMaddog::_scene_iso_donothing() {
	error("Not implemented: _scene_iso_donothing");
}

void GameMaddog::_scene_iso_pause() {
	error("Not implemented: _scene_iso_pause");
}

void GameMaddog::_scene_iso_startgame() {
	error("Not implemented: _scene_iso_startgame");
}

void GameMaddog::_scene_iso_skipsaloon() {
	// TODO: verify
	if ((_got_into & 1) == 1) {
		_cur_scene = scene->next;
		_skipToNextScene = true;
		// TODO: TEST THIS!
	}
}

void GameMaddog::_scene_iso_skipsaloon2() {
	// TODO: verify
	scene_iso_pause(scene, currentFrame);
	scene_iso_skipsaloon(scene, currentFrame);
}

void GameMaddog::_scene_iso_checksaloon() {
	// TODO: verify
	_got_into &= 1;
	if (currentFrame <= 7909) {
		_bartender_alive = true;
	} else {
		_bartender_alive = false;
	}
}

void GameMaddog::_scene_iso_intostable() {
	_got_into &= 2;
}

void GameMaddog::_scene_iso_intoffice() {
	_got_into &= 8;
}

void GameMaddog::_scene_iso_intobank() {
	// TODO: verify
	_got_into &= 4;
	// _scene_iso_shootpast();
}

void GameMaddog::_scene_iso_chkbartndr() {
	// TODO: verify
	if (!_bartender_alive && currentFrame >= scene->dataParam1) {
		_cur_scene = scene->insopParam;
		_skipToNextScene = true;
	}
}

void GameMaddog::_scene_iso_didhideout() {
	_been_to |= 0x80;
}

void GameMaddog::_scene_iso_didsignpost() {
	_been_to |= 0x40;
}

void GameMaddog::_scene_iso_doshootout() {
	error("Not implemented: _scene_iso_doshootout");
}

void GameMaddog::_scene_iso_mdshootout() {
	// TODO verify
	_been_to |= 0x100;
	scene_iso_doshootout(scene, currentFrame);
}

void GameMaddog::_scene_iso_shootpast() {
	error("Not implemented: _scene_iso_shootpast");
}

void GameMaddog::_scene_iso_spause() {
	error("Not implemented: _scene_iso_spause");
}

void GameMaddog::_scene_iso_shotinto24() {
	error("Not implemented: _scene_iso_shotinto24");
}

void GameMaddog::_scene_iso_shotinto116() {
	// TODO verify
	// TODO: find out why we can skip before this frame? Should not be skippable!
	uint32 targetFrame = atoi(scene->insopParam.c_str());
	if (targetFrame >= currentFrame) {
		_skippable = true;
	}
}

// Script functions: Scene Scene NxtScn
void GameMaddog::_scene_default_nxtscn() {
	debug("GameMaddog::scene_default_nxtscn");
	// TODO verify
	if (scene->next.equals("scene28")) {
		_cur_scene = pick_town();
		return;
	}
	Game::_scene_default_nxtscn();
}

void GameMaddog::_scene_nxtscn_pickbottle() {
	// TODO verify
	_bottles++;
	if (_bottles >= 4) {
		_cur_scene = "scene253";
	} else {
		int bottleIndex = pickRandomFromMask(6, &_bot_mask);
		bottleIndex += 12;
		_cur_scene = Common::String::format("scene%d", bottleIndex);
	}
}

void GameMaddog::_scene_nxtscn_died() {
	// TODO verify
	debug("GameMaddog::nxtscnDied");
	// TODO: how do scenes 148 and 149 work? Apparently only for lives > 3, no idea how this should work
	_bad_men_bits = 0;
	_bad_men = 0;
	_had_lantern = false;
	_had_skull = false;
	_got_clue = false;
	if (_player > 0 && !_debug_godMode) {
		_player--;
	}
	if (_player == 2) {
		_sub_scene = "scene150";
		_ret_scene = pick_town(); // TODO: not sure about this
	} else if (_player == 1) {
		_sub_scene = "scene152";
		_ret_scene = pick_town(); // TODO: not sure about this
	} else if (_player == 0) {
		_sub_scene = "scene153";
		_cur_scene = "scene255";
	} else {
		_cur_scene = pick_town();
	}
}

void GameMaddog::_scene_nxtscn_autosel() {
	error("Not implemented: _scene_nxtscn_autosel");
}

void GameMaddog::_scene_nxtscn_finsaloon() {
	_been_to |= 1;
	_cur_scene = pick_town();
}

void GameMaddog::_scene_nxtscn_finoffice() {
	_been_to |= 8;
	_cur_scene = pick_town();
}

void GameMaddog::_scene_nxtscn_finstable() {
	_been_to |= 2;
	_cur_scene = pick_town();
}

void GameMaddog::_scene_nxtscn_finbank() {
	error("Not implemented: _scene_nxtscn_finbank");
}

void GameMaddog::_scene_nxtscn_picsaloon() {
	if ((_been_to & 1) == 1) {
		_cur_scene = "scene118";
	} else {
		_cur_scene = "scene119";
	}
}

void GameMaddog::_scene_nxtscn_killman() {
	_player--;
	if (_player <= 0) {
		_player = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = pick_town();
	}

	UpdateStat();

	_bad_men_bits = 0;
	_bad_men = 0;

	if (_people_killed == 0) {
		_cur_scene = "scene155";
	} else {
		_cur_scene = Common::String::format("scene%d", (_people_killed & 1) + 156);
	}
	_people_killed++;
}

void GameMaddog::_scene_nxtscn_killwoman() {
	_player--;
	if (_player <= 0) {
		_player = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = pick_town();
	}
    
    UpdateStat();
    
    _bad_men_bits = 0;
    _bad_men = 0;
    
    if (_people_killed == 0) {
        _cur_scene = "scene154";
    } else {
		_cur_scene = Common::String::format("scene%d", (_people_killed & 1) + 156);
    }
    _people_killed++;
}

void GameMaddog::_scene_nxtscn_bank() {

    Common::String newScene;
    int threshold1, threshold2;

    _bad_men++;
    threshold1 = (_difficulty * 2) + 6;
    threshold2 = (_difficulty * 2) + 8;

    if (_bad_men > threshold1 && _bad_men > threshold2) {
        _been_to |= 4;
        _bad_men_bits = 0;
        _bad_men = 0;
        if (_rnd->getRandomBit()) {
            _hide_out_front = true;
            newScene = "scene49";
        } else {
            _hide_out_front = false;
            newScene = "scene48";
        }
    } else if (_bad_men > threshold1) {
        newScene = "scene65";
    } else {
        int nextSceneNum = pick_bad(0, 6) + 51;
		newScene = Common::String::format("scene%d", nextSceneNum);
    }

	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_stable() {
    Common::String newScene = nullptr;
    long threshold = (_difficulty * 2) + 6;
    int adjustment = (_been_to & 8) ? 2 : 0;
    threshold -= adjustment;

    _bad_men++;

    if (_bad_men < threshold) {
        _bad_men_bits = 0;
        _bad_men = 0;
        newScene = "scene143";
    } else {
        int nextSceneNum = pick_bad(0, 6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
    }

    _cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_savprosp() {
	// TODO verify
	// _gun_time = 1
	_pro_clue = _rnd->getRandomNumber(2);
	_been_to |= 0x10;
	uint32 nextSceneId = 160 + _pro_clue;
	_cur_scene = Common::String::format("scene%d", nextSceneId);
}

void GameMaddog::_scene_nxtscn_picktoss() {
	// TODO verify
	uint32 random = _rnd->getRandomNumber(6);
	switch (random) {
	case 0:
		_cur_scene = "scene171";
		break;
	case 1:
		_cur_scene = "scene174";
		break;
	case 2:
		_cur_scene = "scene175";
		break;
	case 3:
		_cur_scene = "scene178";
		break;
	case 4:
		_cur_scene = "scene179";
		break;
	case 5:
		_cur_scene = "scene182";
		break;
	case 6:
		_cur_scene = "scene183";
		break;
	default:
		error("toss number invalid");
	}
}

void GameMaddog::_scene_nxtscn_hittoss() {
    if (_lives > 0) {
        _score += 100;
    }
    scene_nxtscn_misstoss();
}

void GameMaddog::_scene_nxtscn_misstoss() {
    if (++_bad_men <= 2) {
        _cur_scene = scene->next;
    } else {
        _bad_men_bits = 0;
        _bad_men = 0;
        _been_to |= 0x200;
		_cur_scene = "scene185";
    }
}

void GameMaddog::_scene_nxtscn_picksign() {
	_cur_scene = pick_sign();
}

void GameMaddog::_scene_nxtscn_brockman() {
	// TODO verify
	int totalEnemies = 9 + (_difficulty * 2);
	if (_bad_men < totalEnemies) {
		uint32 random = _rnd->getRandomNumber(6);
		uint32 nextSceneId = 229 + (random * 2);
		_cur_scene = Common::String::format("scene%d", nextSceneId);
		_bad_men++;
	} else {
		_cur_scene = pick_sign();
		_bad_men = 0;
	}
}

void GameMaddog::_scene_nxtscn_lrockman() {
	// TODO verify
	int totalEnemies = 4 + (_difficulty * 2);
	if (_bad_men < totalEnemies) {
		uint32 random = _rnd->getRandomNumber(2);
		uint32 nextSceneId = 229 + (random * 2);
		_cur_scene = Common::String::format("scene%d", nextSceneId);
		_bad_men++;
	} else {
		_cur_scene = pick_sign();
		_bad_men = 0;
	}
}

void GameMaddog::_scene_nxtscn_hotelmen() {
	// TODO verify
	int totalEnemies = 9 + (_difficulty * 2);
	if (_bad_men < totalEnemies - 1) {
		uint32 random = _rnd->getRandomNumber(14);
		uint32 nextSceneId = 77 + (random * 2);
		_cur_scene = Common::String::format("scene%d", nextSceneId);
		_bad_men++;
	} else {
		_cur_scene = "scene250";
		_bad_men = 0;
	}
}

void GameMaddog::_scene_nxtscn_drawgun() {
	error("Not implemented: _scene_nxtscn_drawgun");
}

// Script functions: ShowMsg
void GameMaddog::_scene_sm_donothing() {
	error("Not implemented: _scene_sm_donothing");
}

// Script functions: WepDwn
void GameMaddog::_scene_default_wepdwn() {
	error("Not implemented: _scene_default_wepdwn");
}

// Script functions: ScnScr
void GameMaddog::_scene_default_score() {
	error("Not implemented: _scene_default_score");
}

// Script functions: ScnNxtFrm
void GameMaddog::_scene_nxtfrm() {
	error("Not implemented: _scene_nxtfrm");
}

// Debugger methods
DebuggerMaddog::DebuggerMaddog(GameMaddog *game) : GUI::Debugger() {
	_game = game;
	registerVar("drawrects", &game->_debug_drawRects);
	registerVar("godmode", &game->_debug_godMode);
	registerCmd("warp", WRAP_METHOD(DebuggerMaddog, cmdWarpTo));
}

bool DebuggerMaddog::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: cmdWarpTo <int>");
	} else {
		int val = atoi(argv[1]);
		_game->debug_warpTo(val);
	}
	return true;
}

void GameMaddog::debug_warpTo(int val) {
	if (val == 0) {
		_been_to = 0;
		_cur_scene = "scene28";
		_skipToNextScene = true;
	} else if (val == 1) {
		_been_to = 1;
		_cur_scene = pick_town();
		_skipToNextScene = true;
	} else if (val == 2) {
		_been_to = 15;
		_cur_scene = pick_town();
		_skipToNextScene = true;
	} else if (val == 3) {
		_been_to = 575;
		_map0 = MAP_LEFT;
		_map1 = MAP_LEFT;
		_map2 = MAP_NONE;
		_next_scene = pick_town();
		_skipToNextScene = true;
	} else if (val == 4) {
		_been_to = 575;
		_hide_out_front = true;
		_cur_scene = "scene210";
		_skipToNextScene = true;
	} else if (val == 5) {
		_been_to = 639;
		_cur_scene = "scene227";
		_skipToNextScene = true;
	} else if (val == 6) {
		_been_to = 1023;
		_next_scene = "scene250";
		_skipToNextScene = true;
	}
}

} // End of namespace Alg
