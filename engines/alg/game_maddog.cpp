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

    switch (_lives[_player]) {
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
    unsigned int mask = 0xFF >> (8 - max));

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
    _cur_scene = map_right();
}

void GameMaddog::_rect_shotleft() {
    _cur_scene = map_left();
}

void GameMaddog::_rect_shotmenu() {
	_inmenu = true;
	_DoMenu();
	_inmenu = false;
}

void GameMaddog::_rect_exit() {
	_exit_caught = 2;
	_exit();
}

void GameMaddog::_rect_continue() {
    _in_menu = 0;
    _fired = 0;

    if (_lives == 0) {
        NewGame();
        _ret_scene = nullptr;
        _cur_scene = pick_town();
    }
}

void GameMaddog::_rect_start() {
    _in_menu = false;
    _fired = false;
    _in_menu = false;
    ResetParams();
    NewGame();
    UpdateStat();
    _cur_scene = _startscene;
}

void GameMaddog::_rect_startbottles() {
    _in_menu = 0;
    _fired = 0;
    ResetParams();
    NewGame();
    UpdateStat();
	_cur_scene = "scene7";
}

// Script functions: Scene PreOps
void GameMaddog::_scene_po_drawrct() {
	// TODO fix
	if (rect->name.equals("scene28")) {
		_cur_scene = pick_town();
    }
}

void GameMaddog::_scene_po_pause() {
	if (scene->name.equals("scene28")) {
		_cur_scene = pick_town();
	}

    _had_pause = false;
    _pause_time = false;
}

void GameMaddog::_scene_pso_shootout() {
    char buffer[80];  // Assuming 80 bytes is enough; adjust if necessary

	extern long min_f;
	extern long max_f;

    __fstrcpy(buffer, scene->rangeString);
    sscanf(buffer, "#%ldto%ld", &min_f, &max_f);

    _shots = 0;
    _in_shootout = 1;

    UpdateStat();

    RestoreCursor();
    DrawCAnImage(0xA000, _reloadicon, word_28172, 0xB0, 0x40, _so_store, word_2816A);
    DoCursor();

    _pp_flgs = 3;

    return 0;
}

void GameMaddog::_scene_pso_mdshootout() {
    char buffer[80];  // Assuming 80 bytes is enough; adjust if necessary

    __fstrcpy(buffer, scene->rangeString);
    sscanf(buffer, "#%ldto%ld", &min_f, &max_f);

    _shots = 0;
    _in_shootout = 1;

    UpdateStat();

    RestoreCursor();
    DrawCAnImage(0xA000, _reloadicon, word_28172, 0xB0, 0x40, _so_store, word_2816A);
    DoCursor();

    return 0;
}

void GameMaddog::_scene_pso_fadein() {
	error("Not implemented: _scene_pso_fadein");
}

void GameMaddog::_scene_pso_paus_fi() {
	_scene_po_pause();
	_scene_pso_fadein();
}

void GameMaddog::_scene_pso_preread() {
    if (stricmp(scene->sceneName, "scene28") == 0) {
        _cur_scene = pick_town();
    }

    _pp_flgs = 3;
}

void GameMaddog::_scene_pso_paus_pr() {
	_scene_po_pause();
	_scene_pso_preread();
}

// Script functions: Scene Scene InsOps
void GameMaddog::_scene_iso_donothing() {
	// do nothing
}

void GameMaddog::_scene_iso_pause() {
    if (_had_pause != 0) {
        goto check_pause_time;
    }

    if (insopParam > arg6 || (insopParam == arg6 && (uint16_t)(insopParam >> 16) > arg4)) {
        goto check_pause_time;
    }

    if ((int16_t)(scene->dataParam1 >> 16) < 0 || ((int16_t)(scene->dataParam1 >> 16) == 0 && (uint16_t)scene->dataParam1 == 0)) {
        goto check_pause_time;
    }

    uint32_t temp = insopParam - _videoFrameSkip + 1;
    if (temp > arg6 || (temp == arg6 && (uint16_t)temp > arg4)) {
        goto set_had_pause;
    }

    temp = insopParam + _videoFrameSkip - 1;
    if (temp < arg6 || (temp == arg6 && (uint16_t)temp < arg4)) {
        goto set_had_pause;
    }

    _game_timer = 0;

    uint32_t pause_duration;
    LXMUL(&pause_duration, scene->dataParam1, 0x90FF);

    _pause_time = pause_duration;

    Pause(pause_duration);

    uint32_t current_time = GetUsTime();
    _pause_time += current_time;

set_had_pause:
    _had_pause = 1;

check_pause_time:
    if (_pause_time != 0 || word_26012 != 0) {
        uint32_t current_time = GetUsTime();
        if (current_time > ((uint32_t)word_26012 << 16 | _pause_time)) {
            word_26012 = 0;
            _pause_time = 0;
        }
    }

    return false;
}

void GameMaddog::_scene_iso_startgame() {
	_ss_flag = 0;
}

void GameMaddog::_scene_iso_skipsaloon() {
// _scene_iso_skipsaloon(SceneStruct far* arg0, uint16_t arg4, uint16_t arg6)
    if (_got_into & 1) {
        if (arg6 <= 0 && (arg6 < 0 || arg4 < 0x1D89)) {
            _cur_scene = arg0->field_10;
            _pause_time = 0;
            return;
        }
    }

    if (arg6 > 0 || (arg6 == 0 && arg4 > 0x1D89)) {
        _got_into |= 1;
    }

    if (_fired != 0 && (arg6 > 0 || (arg6 == 0 && arg4 > 0x1BFD)) &&
        (arg6 < 0 || (arg6 == 0 && arg4 < 0x1E89))) {
        
        scene_result = _FindScene(arg0->field_2E, _scenelist, word_25856);
		_cur_scene = scene_result;
    }
}

void GameMaddog::_scene_iso_skipsaloon2() {
    uint32_t temp_field_2E;

    // Save the original value of field_2E
    temp_field_2E = arg0->field_2E;

    // Replace field_2E with field_52
    arg0->field_2E = arg0->field_52;

    // Call _scene_iso_pause
    _scene_iso_pause(arg0, arg1, arg2, arg3);

    // Restore the original value of field_2E
    arg0->field_2E = temp_field_2E;

    // Call _scene_iso_skipsaloon
    _scene_iso_skipsaloon(arg0, arg1, arg2, arg3);
}

void GameMaddog::_scene_iso_checksaloon() {
    _got_into |= 1;
    if (time > 7909) {
        _bartender_alive = false;
    } else {
        _bartender_alive = true;
    }
}

void GameMaddog::_scene_iso_intostable() {
	_got_into |= 2;
}

void GameMaddog::_scene_iso_intoffice() {
	_got_into |= 8;
}

void GameMaddog::_scene_iso_intobank() {
    _got_into |= 4;
    scene_iso_shootpast(time, arg);
}

void GameMaddog::_scene_iso_chkbartndr() {
    if (_bartender_alive == 0) {
        if (scene->dataParam1 <= _frm) {
			_cur_scene = scene->insopParam;
        }
    }

    if (_fired != 0) {
        if (scene->field_52 < time) {
            scene->callback(scene);
        }
    }
}

void GameMaddog::_scene_iso_didhideout() {
	_been_to |= 0x80;
}

void GameMaddog::_scene_iso_didsignpost() {
	_been_to |= 0x40;
}

void GameMaddog::_scene_iso_doshootout() {
    if (time > word_2831A || (time == word_2831A && (unsigned int)time >= min_f)) {
        if (_in_shootout) {
            _disable();
            RestoreCursor();
            DrawAnImage(0xA000, word_2816E, _drawicon, 0xB0, 0x40);
            DoCursor();
            _enable();
        }

        _in_shootout = 0;

        if (_shots != 0 || (word_281AE != 0 && _num_players == 2)) {
            if (time < word_28316 || (time == word_28316 && (unsigned int)time < max_f)) {
                scene->callback(scene);
            }
        }
    }

    return 0;
}

void GameMaddog::_scene_iso_mdshootout() {
    _been_to |= 0x100;
    _scene_iso_doshootout(scene, time); // TODO time is currentFrame
}

void GameMaddog::_scene_iso_shootpast() {
    if (_fired) {
        if (_ret_scene) {
            _cur_scene = _ret_scene;
            _ret_scene = 0;
            _pp_force = 3;
        }
        else if (_sub_scene) {
            _cur_scene = _sub_scene;
            _sub_scene = 0;
            _pp_force = 3;
        }
        else {
            scene->callback(scene);
        }
    }
}

void GameMaddog::_scene_iso_spause() {
	_scene_iso_shootpast();
	_scene_iso_pause();
}

void GameMaddog::_scene_iso_shotinto24() {
	// do nothing
}

void GameMaddog::_scene_iso_shotinto116() {
	// TODO fix
	// scene_iso_shotinto116(struct Scene far* scene, unsigned long time)
    if (_fired != 0) {
        if (time > scene->field_2E || (time == scene->field_2E && (unsigned int)time >= (unsigned int)(scene->field_2E))) {
            scene->callback(scene);
        }
    }
////////////////////////////////////////////
	uint32 targetFrame = atoi(scene->insopParam.c_str());
	if (targetFrame >= currentFrame) {
		_skippable = true;
	}
}

// Script functions: Scene Scene NxtScn
void GameMaddog::_scene_default_nxtscn() {
    if (__fstricmp(*(const char far* far*)((char far*)scene + 0x10), "scene28") == 0) {
        _cur_scene = pick_town();
    } else {
        _cur_scene = scene->next;
    }
    
}

void GameMaddog::_scene_nxtscn_pickbottle() {
    bottles++;
    if (bottles < 4) {
        int rand = pick_rand(6, &_botmask);
		_cur_scene = Common::String::format("scene%d", rand + 11);
    } else {
        _cur_scene = "scene253";
    }
}

void GameMaddog::_scene_nxtscn_died() {
    _had_skull = 0;
    _bad_men_bits = 0;
	_bad_men = 0;
    _got_clue = 0;
	_had_lantern = 0;
    
    if (--_lives <= 0) {
        if (_player == 2 && _lives > 0) {
            _ret_scene = _cur_scene;
        } else {
            _lives = 0;
            _last_scene = _cur_scene;
            _sub_scene = "scene255";
        }
        _die();
        return 0;
    }

    if (__fstricmp(scene_name, "scene28") == 0) {
        _ret_scene = pick_town();
    } else {
        _ret_scene = _cur_scene;
    }

    _die();
    return 0;
}

void GameMaddog::_scene_nxtscn_autosel() {
    Common::String newScene;

    if (!(_been_to & 2)) {
        newScene = "scene122";
    }
    else if (!(_been_to & 8)) {
        newScene = "scene114";
    }
    else if (!(_been_to & 1)) {
        if (_got_into & 1) {
            newScene = "scene69";
        } else {
            newScene = "scene67";
        }
    }
    else if (!(_been_to & 4)) {
        newScene = "scene45";
    }
    else {
        newScene = "scene186";
    }

    _cur_scene = newScene;
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
	_been_to |= 4;
	_cur_scene = pick_town();
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
        int nextSceneNum = _pick_bad(0, 6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
    }

    _cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_savprosp() {
    _gun_time = 1;
    _old_score = -1;
	_pro_clue = _rnd->getRandomNumber(2);
    _been_to |= 0x10;
	_cur_scene = Common::String::format("scene%d", _pro_clue + 160);
}

void GameMaddog::_scene_nxtscn_picktoss() {
    int index = _pick_bad(7);
	_cur_scene = Common::String::format("scene%d", _bottle_toss[index]);
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
    long max_bad_men = (_difficulty * 2) + 9;
    _bad_men++;
    if (_bad_men > max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        _cur_scene = pick_sign();
    } else {
        int nextBad = pick_bad(7, 0);
		_cur_scene = Common::String::format("scene%d", nextBad + 229);
    }
}

void GameMaddog::_scene_nxtscn_lrockman() {
    long max_bad_men = (_difficulty * 2) + 4;
    _bad_men++;
    if (_bad_men > max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        next_scene = pick_sign();
    } else {
        int nextBad = _pick_bad(3, 0);
		_cur_scene = Common::String::format("scene%d", nextBad + 244);
    }
}

void GameMaddog::_scene_nxtscn_hotelmen() {
    long max_bad_men = (_difficulty * 2) + 9;

    if (_bad_men >= max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        _been_to |= 0x100;
        _cur_scene "scene250";
    } else {
        _bad_men++;
        int index = pick_bad(5, 0);
		_cur_scene = Common::String::format("scene%d", _hotel_scenes[index]);
    }
}

void GameMaddog::_scene_nxtscn_drawgun() {
    RestoreCursor();
    DrawAnImage(0xA000, word_2816A, _so_store, 0xB0, 0x40);
    DoCursor();
    scene_default_nxtscn();
}

// Script functions: ShowMsg
void GameMaddog::_scene_sm_donothing() {
	// do nothing
}

// Script functions: WepDwn
void GameMaddog::_scene_default_wepdwn() {
    _inholster = 9;
    _whichgun = 7;

    UpdateMouse();

    if (_in_shootout == 0) {
        if (_been_to >= 15) {
            if (_shots[arg] < 12) {
                _shots[arg] = 12;
            }
        } else {
            if (_shots[arg] < 6) {
                _shots[arg] = 6;
            }
        }
        UpdateStat();
    }
}

// Script functions: ScnScr
void GameMaddog::_scene_default_score() {
    if (scene->field_3C > 0 || (scene->field_3C == 0 && scene->field_3A > 0)) {
        _score += scene->field_3A;
    }
}

// Script functions: ScnNxtFrm
void GameMaddog::_scene_nxtfrm() {
    if (_pause_time || _ss_flag) {
        return 0;
    }

    int result = photoPlay(3, 0, 0, 0x38, 0x20, &_curfrm, &_timlft, 0, 0);
    if (result < 0) {
        unsigned long diff = scene->field_8 - scene->field_4 + 1;
        _curfrm = diff;
        return -1;
    }

    if (_rectflg) {
        void far* rect = scene->rect_list;
        while (rect) {
            ShowRects(rect);
            rect = ((struct Rect far*)rect)->next;
        }

        if (kbhit()) {
            getch();
            photoPlay(5, 0, 0, 0x38, 0x20, &_curfrm, &_timlft, 0, 0);
            getch();
        }
    }
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
