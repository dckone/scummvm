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
#include "common/rect.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"

#include "alg/game_maddog.h"
#include "alg/graphics.h"
#include "alg/scene.h"

#define MADDOG_VIDEO_POS_X 56
#define MADDOG_VIDEO_POS_Y 8

namespace Alg {

GameMaddog::GameMaddog(AlgEngine *vm) : Game(vm) {
}

GameMaddog::~GameMaddog() {
}

void GameMaddog::init() {
	debug("GameMaddog::init");
	registerScriptFunctions();
	loadLibArchive("MADDOG.LIB");
	_sceneInfo->loadScnFile("MADDOG.SCN");
    _startscene = _sceneInfo->getStartScene();

	verifyScriptFunctions();

	_menuzone = new Zone();
    _menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";
	_menuzone->addRect(0x0C, 0xAC, 0x3D, 0xBF, nullptr, 0, "SHOTMENU", "0"); // TODO verify, nullptr, menu or scene1?
	_menuzone->addRect(0x00, 0xA6, 0x013F, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_bott // TODO find out what it does
	_menuzone->addRect(0x00, 0x00, 0x3B, 0xC7, nullptr, 0, "DEFAULT", "0"); // _mm_left // TODO find out what it does

	_submenzone = new Zone();
    _submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";
	_submenzone->addRect(0x8A, 0x3B, 0xC2, 0x48, nullptr, 0, "STARTBOT", "0"); // TODO verify, nullptr, menu or scene1?
	_submenzone->addRect(0x8A, 0x4E, 0xC2, 0x59, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x8A, 0x60, 0xC2, 0x6B, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0xE3, 0x3B, 0x011B, 0x48, nullptr, 0, "RECTSAVE", "0"); // TODO verify 0x011B
	_submenzone->addRect(0xE3, 0x4E, 0x011B, 0x59, nullptr, 0, "RECTLOAD", "0"); // TODO verify 0x011B
	_submenzone->addRect(0xE3, 0x60, 0x011B, 0x6B, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0x42, 0x34, 0x5C, 0x4E, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0x42, 0x53, 0x5C, 0x70, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0x42, 0x72, 0x62, 0x8A, nullptr, 0, "RECTHARD", "0");

	_background = AlgGraphics::loadVgaBackground("BACKGRND.VGA", _palette);

    _shot = _LoadSound("blow.8b");
    _emptysound = _LoadSound("empty.8b");
    _savesound = _LoadSound("saved.8b");
    _loadsound = _LoadSound("loaded.8b");
    _skullsound = _LoadSound("skull.8b");
    _easysound = _LoadSound("deputy.8b");
    _avgsound = _LoadSound("sheriff.8b");
    _hardsound = _LoadSound("marshall.8b");

    // _saveimage2 = _SafeAlloc("saveimage2", 0x2EE); // TODO

    // _gun = _ReadAni("gun.ani"); // TODO
    // _numbers = _ReadAni("numbers.ani"); // TODO
    // _shoticon = _ReadAni("bullet.ani", 0); // TODO
    // _emptyicon = _ReadAni("bullet.ani", 1); // TODO

	_aniAmmo = AlgGraphics::loadAniImage("AMMO.ANI");
	_aniBullet = AlgGraphics::loadAniImage("BULLET.ANI");
	_aniClip = AlgGraphics::loadAniImage("CLIP.ANI");
	_aniHat = AlgGraphics::loadAniImage("HAT.ANI");
	_aniShootout = AlgGraphics::loadAniImage("SHOOTOUT.ANI");

	_aniGun = AlgGraphics::loadTransparentAniImage("GUN.ANI");
	_aniHole = AlgGraphics::loadTransparentAniImage("HOLE.ANI");
	_aniKnife = AlgGraphics::loadTransparentAniImage("KNIFE.ANI");

	Graphics::Surface aniGun0 = (*_aniGun)[8];
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	CursorMan.pushCursor(aniGun0.getPixels(), aniGun0.w, aniGun0.h, aniGun0.w / 2, aniGun0.h / 2, 0, false, &pixelFormat);
	CursorMan.showMouse(true);
}

void GameMaddog::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new ScriptFunctionPoint(this, &func);
	ZONE_PTRFB_FUNCTION("DEFAULT", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("SKULL", _zone_skullhole);
	ZONE_PTRFB_FUNCTION("GLOBALHIT", _zone_globalhit);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new ScriptFunctionSceneRect(this, &func);
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

#define PRE_OPS_FUNCTION(name, func) _scenePreOps[name] = new ScriptFunctionScene(this, &func);
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

#define INS_OPS_FUNCTION(name, func) _sceneInsOps[name] = new ScriptFunctionScene(this, &func);
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

#define NXT_SCN_FUNCTION(name, func) _sceneNxtScn[name] = new ScriptFunctionScene(this, &func);
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

	_sceneShowMsg["DEFAULT"] = new ScriptFunctionScene(this, &_scene_sm_donothing);
	_sceneWepDwn["DEFAULT"] = new ScriptFunctionScene(this, &_scene_default_wepdwn);
	_sceneScnScr["DEFAULT"] = new ScriptFunctionScene(this, &_scene_default_score);
	_sceneNxtFrm["DEFAULT"] = new ScriptFunctionScene(this, &_scene_nxtfrm);
}

void GameMaddog::verifyScriptFunctions() {
    Common::Array<Scene> scenes = _sceneInfo->getScenes();
	Common::Array<Scene>::iterator scene;
    Common::Array<Zone>::iterator zone;
    Common::Array<Rect>::iterator rect;
	for (scene = scenes.begin(); scene != scenes.end(); ++scene) {
        getScriptFunctionScene(PREOP, scene->preop);
        // TODO: SHOWMSG
        getScriptFunctionScene(INSOP, scene->insop);
        getScriptFunctionScene(WEPDWN, scene->wepdwn);
        getScriptFunctionScene(SCNSCR, scene->scnscr);
        getScriptFunctionScene(NXTFRM, scene->nxtfrm);
        getScriptFunctionScene(NXTSCN, scene->nxtscn);
    	for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
            getScriptFunctionZonePtrFb(zone->ptrfb);
            for (rect = zone->rects.begin(); rect != zone->rects.end(); ++rect) {
                getScriptFunctionRectHit(rect->rectHit);
            }
        }
    }
}

ScriptFunctionPoint GameMaddog::getScriptFunctionZonePtrFb(Common::String name) {
    ScriptFunctionPointMap::iterator it = _zonePtrFb.find(name);
	if (it != _zonePtrFb.end()) {
        return (*(*it)._value);
    } else {
		error("Could not find zonePtrFb function: %s", name.c_str());
    }
}

ScriptFunctionSceneRect GameMaddog::getScriptFunctionRectHit(Common::String name) {
    ScriptFunctionSceneRectMap::iterator it = _rectHitFuncs.find(name);
	if (it != _rectHitFuncs.end()) {
        return (*(*it)._value);
    } else {
		error("Could not find rectHit function: %s", name.c_str());
    }
}

ScriptFunctionScene GameMaddog::getScriptFunctionScene(SceneFuncType type, Common::String name) {
    ScriptFunctionSceneMap *functionMap;
    switch (type) {
        case PREOP:
            functionMap = &_scenePreOps;
            break;
        case SHOWMSG:
            functionMap = &_sceneShowMsg;
            break;
        case INSOP:
            functionMap = &_sceneInsOps;
            break;
        case WEPDWN:
            functionMap = &_sceneWepDwn;
            break;
        case SCNSCR:
            functionMap = &_sceneScnScr;
            break;
        case NXTFRM:
            functionMap = &_sceneNxtFrm;
            break;
        case NXTSCN:
            functionMap = &_sceneNxtScn;
            break;
        default:
    		error("Unkown scene script type: %u", type);
            break;
    }
    ScriptFunctionSceneMap::iterator it;
    it = functionMap->find(name);
	if (it != functionMap->end()) {
        return (*(*it)._value);
    } else {
		error("Could not find scene type %u function: %s", type, name.c_str());
    }
}

void GameMaddog::callScriptFunctionZonePtrFb(Common::String name, Common::Point *point) {
	ScriptFunctionPoint function = getScriptFunctionZonePtrFb(name);
    function(point);
}

void GameMaddog::callScriptFunctionRectHit(Common::String name, Scene *scene, Rect *rect) {
	ScriptFunctionSceneRect function = getScriptFunctionRectHit(name);
    function(scene, rect);
}

void GameMaddog::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
    debug("callScriptFunctionScene: %u, %s", type, name.c_str());
	ScriptFunctionScene function = getScriptFunctionScene(type, name);
    function(scene);
}

Common::Error GameMaddog::run() {
    debug("GameMaddog::run");
	init();
    _NewGame();
    _cur_scene = _startscene;
    Common::String oldscene;
    while(!_vm->shouldQuit()) {
        oldscene = _cur_scene;
        _SetFrame();
        _fired = 0;
        Scene *scene = _sceneInfo->findScene(_cur_scene);
        loadScene(scene);
        callScriptFunctionScene(PREOP, scene->preop, scene);
        _frm = _GetFrame(scene);
        while(_frm <= scene->endFrame && _cur_scene == oldscene && !_vm->shouldQuit()) {
            // TODO: call scene->messageFunc
            callScriptFunctionScene(INSOP, scene->insop, scene);
            bool weaponStatus = _WeaponDown();
            if (weaponStatus) {
                callScriptFunctionScene(WEPDWN, scene->wepdwn, scene);
            }
            /*
            unsigned short fired_data;
            if (__Fired(_player, &fired_data)) {
                if (weapon_states[_player] == 0) {
                    // Check global zone hit
                    void far* hitGlobalZone = CheckZone(&fired_data, _global_zone);
                    if (hitGlobalZone) {
                        hitGlobalZone->callback(&fired_data);
                    }
                    else {
                        if (_shots > 0) {
                            _shots--;
                            _UpdateStat();
                            // Check scene-specific zones
                            void far* scene_zones = *(void far**)((char far*)_cur_scene + 0xC);
                            void far* hit_scene_zone = CheckZones(&fired_data, word_2819E, _frm, &scene_zones);
                            if (hit_scene_zone) {
                                // Call scene zone hit function
                                // ((void (*far)(void far*, void far*))(scene_zones + 0x4))(&fired_data, scene_zones);
                                // Call zone-specific hit function
                                // ((void (*far)(void far*))((char far*)hit_scene_zone + 0xE))(&fired_data);
                            }
                            else {
                                // If no hit, create a bullet hole
                                _default_bullethole(fired_data);
                            }
                        }
                        else {
                            // Play empty gun sound if out of ammo
                            _default_empty_sound();
                        }
                    }
                }
            */
            if (_cur_scene == oldscene) {
                callScriptFunctionScene(NXTFRM, scene->nxtfrm, scene);
            }
            _DisplayScore();
            _videoDecoder->getNextFrame();
            _frm = _GetFrame(scene);
            updateScreen();
			pollEvents();
			checkKeysPressed();
            g_system->delayMillis(90); // TODO fix
        }
        // frame limit reached or scene changed, prepare for next scene
        _player = 0;
        _had_pause = 0;
        _pause_time = 0;
        if (_ret_scene != "") {
            _cur_scene = _ret_scene;
            _ret_scene = "";
            _pp_force = 3;
        }
        if (_sub_scene != "") {
            _ret_scene = _sub_scene;
            _sub_scene = "";
            _pp_force = 3;
        }
        if (_cur_scene == oldscene) {
            callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
        }
        if (_cur_scene == "" || _vm->shouldQuit()) {
            break; // Exit main game loop
        }
    }
    // game over, exit

    /*
	while (1) {
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
	}
    */
	return Common::kNoError;
}

void GameMaddog::updateScreen() {
    debug("GameMaddog::updateScreen");
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);
	if (!_in_menu) {
		Graphics::Surface *frame = _videoDecoder->getVideoFrame();
		_screen->copyRectToSurface(frame->getPixels(), frame->pitch, MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y, frame->w, frame->h);
	}
	// debug_drawZoneRects();

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

// TODO fix
/*
bool GameMaddog::__Fired(int playerIndex, Point* coordinates)
{
    _fired = 0;
    if (_read_input(&x, &y))
    {
        if (firedStatus[playerIndex])
            return false;

        _fired = 1;
        coordinates->x = x;
        coordinates->y = y;
        firedStatus[playerIndex] = 1;
        return true;
    }
    else
    {
        firedStatus[playerIndex] = 0;
        return false;
    }
   return false;
}
*/

void GameMaddog::_DoMenu()
{
    // unsigned long zone_result;
    // unsigned char event;

    // photoPlay(5, 0, 0, 0, 0x38, 0x20, &curfrm, &timlft, 0);

    uint32 startTime = _GetUsTime();

    _RestoreCursor();
    // SWPSCRN(oldscrn, word_27CDA);
    _DoCursor();

    _in_menu = 1;

    while (_in_menu)
    {
        // TODO fix
        /*
        if (__Fired(_player, &event))
        {
            zone_result = CheckZone(&event, _submenzone);
            if (zone_result)
            {
                // TODO fix
                // _submenzone->field_4(&event, global_zone->field_18, global_zone->field_1A);
                // zone_result[3](&zone_result, &event);
            }
        }
        */
    }

    _RestoreCursor();
    // SWPSCRN(oldscrn, word_27CDA);
    _DoCursor();

    if (_had_pause)
    {
        unsigned long end_time = _GetUsTime();
        unsigned long time_diff = end_time - startTime;
        _pause_time += time_diff;
    }
}

uint32 GameMaddog::_GetUsTime() {
    uint32 millis = g_system->getMillis();
    return millis * 1000;
}

void GameMaddog::_Pause(unsigned long pause_time) {
    // photoPlay(5, 0, 0, 0, 0x38, 0x20, &curfrm, &timlft, 0);
    _NextFrameTime += pause_time;
    if (_NextFrameTime > 0) {
        g_system->delayMillis(pause_time / 1000);
    }
}

void GameMaddog::_NewGame() {
    _shots = 6;
    _lives = 3;
    _score = 0;
    _holster = 0;
    _UpdateStat();
    _sub_scene = "";
}

void GameMaddog::_ResetParams() {
    _been_to = 0;
    _bottles = 0;
    _botmask = 0;
    _got_into = 0;
    _had_skull = 0;
    _bad_men = 0;
    _bad_men_bits = 0;
    _people_killed = 0;
    _hide_out_front = 0;
    _difficulty = 1;
    _gun_time = 0;
    _pro_clue = 0;
    _got_clue = 0;
    _had_lantern = 0;
    _map_pos = 0;
    _shoot_out_cnt = 0;
    _max_map_pos = 0;
    _sheriff_cnt = 0;
    _in_shootout = 0;
    _ret_scene = "";
    _sub_scene = "";
}

void GameMaddog::_UpdateStat() {
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

void GameMaddog::_ChangeDifficulty(int newDifficulty) {
    // TODO implement
    /*
    struct Scene *scene = (struct Scene *)_scenelist;
    
    if (newDifficulty == _olddif) {
        return;
    }

    ShowDifficulty(newDifficulty, 1);

    while (scene) {
        if (!(scene->DIFF & 0x01)) {
            if ((scene->script_offset == 0x03BE) || // _scene_po_pause
                (scene->script_offset == 0x052A) || // _scene_pso_paus_fi
                (scene->script_offset == 0x057C)) { // _scene_pso_paus_pr
                
                long long newValue = (long long)scene->value * _pausdifscal[newDifficulty] / _pausdifscal[_olddif];
                scene->value = (int)newValue;
            }
        }

        if (!(scene->DIFF & 0x02)) {
            struct SubScene *subScene = scene->subScenes;
            
            while (subScene) {
                    while (current) {
						width = right - left;
						width = width * _rectdifscal[newDifficulty] / _olddif;
						width = (right - left - width) / 2;
						height = bottom - top;
						height = height * _rectdifscal[newDifficulty] / _olddif;
						height = (bottom - top - height) / 2;

						current->values[0] += width;
						current->values[2] -= width;
						current->values[1] += height;
						current->values[3] -= height;

						current = current->next;
					}

				
                subScene = subScene->next;
            }
        }

        scene = scene->next;
    }
    */

    _olddif = newDifficulty;
    _difficulty = newDifficulty;
}

void GameMaddog::_ShowDifficulty(int newDifficulty, int updateCursor) {
    if (newDifficulty == _olddif)
        return;
    if (updateCursor) {
        _RestoreCursor();
        // DrawAnImage(VGA_SEGMENT, saveknife, _diffpos[newDifficulty][0], _diffpos[newDifficulty][0]); // TODO
    }
    // DrawCAnImage(VGA_SEGMENT, saveknife, knife, _diffpos[newDifficulty][0], _diffpos[newDifficulty][0]); // TODO
    if (updateCursor) {
        _DoCursor();
    }
    _olddif = newDifficulty;
}

void GameMaddog::_RestoreCursor() {
    // _DrawAnImage(0xA000, word_282BC, _saveimage2, omy, omx); // TODO
}

void GameMaddog::_DoCursor() {
    // TODO fix
    /*
    unsigned int gun_index = _whichgun * 4;
    unsigned long gun_ptr = _gun[_whichgun];
    unsigned int gun_segment = (unsigned int)(gun_ptr >> 16);
    unsigned int gun_offset = (unsigned int)(gun_ptr & 0xFFFF);

    // _DrawCAnImage(0xA000, _saveimage2, gun_offset, gun_segment, omy, omx); // TODO

    _oldwhichgun = _whichgun;
    */
}

void _DrawAnImage(int x, int y, void* image, unsigned int destSegment, unsigned int width)
{
    // TODO implement
/*
    unsigned int offset;
    unsigned char far* src = (unsigned char far*)image;
    unsigned char far* dest;
    unsigned int count;
    unsigned int lineOffset;

    // Calculate the initial offset in video memory
    offset = (y << 5) + (y << 3) + x;  // equivalent to y * 40 + x

    // Set up the destination segment
    _asm {
        push ds
        mov es, destSegment
    }

    while (1) {
        // Read count
        count = *(unsigned int far*)src;
        src += 2;

        if (count == 0) break;

        // Read line offset
        lineOffset = *(unsigned int far*)src;
        src += 2;

        // Calculate destination address
        dest = (unsigned char far*)(offset + lineOffset);

        // Copy bytes
        _asm {
            push ds
            lds si, src
            les di, dest
            mov cx, count
            rep movsb
            pop ds
        }

        src += count;
    }

    _asm {
        pop ds
    }
*/
}

void _DrawCAnImage(int x, int y, void* image, void* saveImage, unsigned int destSegment, unsigned int srcSegment)
{
    /*
    unsigned int offset;
    unsigned char far* src = (unsigned char far*)image;
    unsigned char far* save = (unsigned char far*)saveImage;
    unsigned char far* dest;
    unsigned int count;
    unsigned int lineOffset;

    // Calculate the initial offset in video memory
    offset = (y << 5) + (y << 3) + x;  // equivalent to y * 40 + x
    while (1) {
        uint16_t count = *src++;
        *dest++ = count;
        
        uint16_t lineOffset = *src++;
        *dest++ = lineOffset;
        
        if (count == 0) break;
        
        uint16_t *srcLine = (uint16_t *)(srcSegment + lineOffset + offset);
        uint8_t *destLine = (uint8_t *)dest;
        uint8_t *saveLine = (uint8_t *)(saveBuffer + lineOffset);
        
        // Copy from source to destination
        memcpy(destLine, srcLine, count);
        dest += (count + 1) / 2;  // Adjust for word alignment
        
        // Copy from source to save buffer
        memcpy(saveLine, srcLine, count);
    }
    */
}

void GameMaddog::_UpdateMouse() {
    if (_scene_change != 0) {
        return;
    }

    if (_oldwhichgun == _whichgun) {
        // DRAWUANIMAGE(omx, omy, _gun[_whichgun], _saveimage2, 0xA000); // TODO
    } else {
        // _DrawAnImage(omx, omy, _saveimage2, 0xA000);  // TODO
        // _DrawCAnImage(omx, omy, _gun[_whichgun], _saveimage2, 0xA000);  // TODO
        _oldwhichgun = _whichgun;
    }
}

void GameMaddog::_MoveMouse(int x, int y)
{
    if (_scene_change != 0)
        return;

    if (x < 59 || _inmenu != 0)
        _whichgun = 8;
    else if (y > 166)
    {
        if (_inholster == 0)
            _whichgun = 6;
        else
            _whichgun = 7;
    }
    else if (_whichgun > 5) {
        _whichgun = 0;
    }

    // Clear old cursor
    // _DrawAnImage(0xA000, _saveimage2, omy, omx);  // TODO

    // Draw new cursor
    // _DrawCAnImage(0xA000, _saveimage2, _gun[_whichgun], y, x);  // TODO

    _oldwhichgun = _whichgun;
    _omx = x;
    _omy = y;
}

void GameMaddog::_DisplayScore() {
    // TODO fix
    /*
    char buffer[82];  // 52h = 82 in decimal
    int posX = 0xDC;

    if (_score == oldscore)
        return;  // If score hasn't changed, don't redraw

    oldscore = _score;

    // Format the score as a string
    sprintf(buffer, "%05d", _score);

    for (int i = 0; i < 5; i++)  // Assuming 5-digit score
    {
        if (buffer[i] != oldScoreDisplay[i])
        {
            int digit;
            if (buffer[i] == '0') {
                digit = 9;
            } else {
                digit = buffer[i] - '0' - 1;
            }

            DrawAnImage(0xA000, _numbers[digit], 0xAD, posX);

            oldScoreDisplay[i] = buffer[i];
        }
        posX += 10;  // Move to next digit position
    }
    */
}

bool GameMaddog::_WeaponDown() {
    /*
    if (_rightdown != 0 && my > 168) {
        _rightdown = 0;
        return true;
    }
    */
    return false;
}

uint32 GameMaddog::_GetFrame(Scene *scene) {
    // return (_videoFrameSkip * _curfrm) + _cur_scene->startFrame - frameSkip;;
    return scene->startFrame + (_videoDecoder->getCurrentFrame() * 3);
}

void GameMaddog::_SetFrame() {
    _pp_flgs |= _pp_force;
    _pp_flgs = 0;
    _pp_force = 0;
    _UpdateMouse();
    /*
    if (_rectflg != 0) {
        PutString(filename, 10, 180, 4, 1, 0xA000);
        if (frameInfo->rectInfo != NULL) {
            ShowRects(frameInfo->rectInfo);
        }
    }
    */
}

void GameMaddog::_SaveState(Common::String filename) {
    // TODO implement
}

void GameMaddog::_LoadState(Common::String filename) {
    // int dataOffset = _gamestate; // TODO
    /*
    while (*(short*)(offset + 4006) != -1)
    {
        sfgets(buffer, 80, stream);
        int type = *(short*)(offset + 4006);
        void* dataPtr = *(void**)(dataOffset);

        switch (type)
        {
            case 0:
                sscanf(buffer, "%d\n", dataPtr);
                break;
            case 1:
                sscanf(buffer, "%ld\n", dataPtr);
                break;
            case 2:
                if (stricmp(buffer, "NULL") == 0)
                    *(void**)dataPtr = NULL;
                else
                    *(void**)dataPtr = FindScene(_scenelist, buffer);
                break;
        }

        offset += 10;
        dataOffset += 10;
    }
    */

    _ChangeDifficulty(_difficulty);
    _pp_force = 3;
}

uint32 GameMaddog::_LoadSound(Common::String filename) {
    // TODO implement
    return 0;
}

void GameMaddog::_DoDiffSound(int difficulty) {
    // TODO implement
    /*
    switch(difficulty)
    {
        case 1:
            if (_easysound) {
                // SendPVoc(_easysound, _easylngth);
            }
            break;
        case 2:
            if (_avgsound) {
                // SendPVoc(_avgsound, _avglngth);
            }
            break;
        case 3:
            if (_hardsound) {
                // SendPVoc(_hardsound, _hardlngth);
            }
            break;
        default:
            // Do nothing
            break;
    }
    */
}

void GameMaddog::_DoSaveSound() {
    // TODO implement
}

void GameMaddog::_DoLoadSound() {
    // TODO implement
}

void GameMaddog::_DoSkullSound() {
    // TODO implement
}

void GameMaddog::_DoShot() {
    // This is sound, like _DoSkullSound
    // TODO implement
}

void GameMaddog::_default_bullethole(const Common::Point *point) {
    if (point->x >= 59 && point->y <= 166) {
        _RestoreCursor();
        // _DrawCAnImage(x, y, _bullethole, word_2815E, _saveimage, 0xA000);  // TODO
        _DoCursor();
        _oldx = point->x;
        _oldy = point->y;
        _shotfired = 1;
        _DoShot();
    }
}

void GameMaddog::_die() {
    Common::String newScene;
    _UpdateStat();

    switch (_lives) {
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

uint32 GameMaddog::_pick_rand(uint32 max, uint32 *maskPtr) {
    uint32 randomVal, mask;
    // reset maskPtr if full
    if (*maskPtr == (uint32)(0xFF >> (8 - max))) {
        *maskPtr = 0;
    }
    do {
        randomVal = _rnd->getRandomNumber(max);
        mask = 1 << randomVal;
    } while (*maskPtr & mask);
    *maskPtr |= mask;
    return randomVal;
}

uint32 GameMaddog::_pick_bad(uint32 max) {
    unsigned int mask = 0xFF >> (8 - max);
    if (_bad_men_bits == mask) {
        _bad_men_bits = 0;
    }
    return _pick_rand(max, &_bad_men_bits);
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
		int pickedSceneNum = (_shoot_out_cnt / 5) + 106;
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
        case 5:
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
	int32 _mapArray[3] = {_map0, _map1, _map2};
    _map_pos++;
    if (_map_pos > _max_map_pos) {
        _max_map_pos = _map_pos;
    }
    if (_map_pos <= 2 && _mapArray[_map_pos] != 0) {
		pickedScene = Common::String::format("scene%d", _map_pos + 187);
    } else {
        pickedScene = "scene210";
    }
    return pickedScene;
}

Common::String GameMaddog::_map_right() {
	Common::String pickedScene;
	int32 _mapArray[3] = {_map0, _map1, _map2};
    if (_mapArray[_map_pos] == -1) {
        if (_map_pos >= _max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[_map_pos]);
        } else {
            return _pick_sign();
        }
    } else if (_mapArray[_map_pos] == 0) {
        if (_map_pos >= _max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[_map_pos]);
        } else {
            return _pick_sign();
        }
    } else {
			pickedScene = Common::String::format("scene%d", _ambush[_map_pos]);
    }
	return pickedScene;
}

Common::String GameMaddog::_map_left() {
	Common::String pickedScene;
	int32 _mapArray[3] = {_map0, _map1, _map2};
    if (_mapArray[_map_pos] == 1) {
        if (_map_pos >= _max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[_map_pos]);
        } else {
            return _pick_sign();
        }
    } else if (_mapArray[_map_pos] == -1) {
			pickedScene = Common::String::format("scene%d", _ambush[_map_pos]);
    } else {
        if (_map_pos >= _max_map_pos) {
			pickedScene = Common::String::format("scene%d", _fight[_map_pos]);
        } else {
            return _pick_sign();
        }
    }
	return pickedScene;
}

// Script functions: Zone
void GameMaddog::_zone_bullethole(const Common::Point *point) {
	_default_bullethole(point);
}

void GameMaddog::_zone_skullhole(const Common::Point *point) {
    if (point->x >= 59 && point->y <= 166) {
        _RestoreCursor();
        // _DrawCAnImage(zone->x, zone->y, _bullethole, _saveimage, 0xA000);  // TODO
        _DoCursor();
        _oldx = point->x;
        _oldx = point->y;
        _shotfired = 1;

        if (_had_skull) {
            _DoShot();
        } else {
            _DoSkullSound();
        }
    }
}

void GameMaddog::_zone_globalhit(const Common::Point *point) {
	// do nothing
}

// Script functions: RectHit
void GameMaddog::_rect_newscene(const Scene *scene, const Rect *rect) {
    _score += rect->score;
    _cur_scene = rect->scene;
}

void GameMaddog::_rect_hidefront(const Scene *scene, const Rect *rect) {
	if (_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_hiderear(const Scene *scene, const Rect *rect) {
	if (!_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_menuselect(const Scene *scene, const Rect *rect) {
	Common::String newScene;

	// TODO fixme
	int cursorX = 0;
	int cursorY = 0;

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
            if (_been_to & 4) return;
            if (_got_into & 4) {
                _pp_force = 3;
                _scene_nxtscn_bank(scene);
				return;
            } else {
                newScene = "scene45";
            }
        }
    }

	_cur_scene = newScene;
}

void GameMaddog::_rect_skull(const Scene *scene, const Rect *rect) {
    if (_had_skull) {
        return;
    }

    _had_skull = true;

    if (_been_to < 15) {
        _shots = 9;
    } else {
        _shots = 12;
    }

    _UpdateStat();
}

void GameMaddog::_rect_killman(const Scene *scene, const Rect *rect) {
	_scene_nxtscn_killman(scene);
}

void GameMaddog::_rect_killwoman(const Scene *scene, const Rect *rect) {
	_scene_nxtscn_killwoman(scene);
}

void GameMaddog::_rect_prospsign(const Scene *scene, const Rect *rect) {
    if (_been_to & 0x10) {
        return;
    }
    _gun_time = 1;
    _cur_scene = rect->scene;
}

void GameMaddog::_rect_minesign(const Scene *scene, const Rect *rect) {
    if (_been_to & 0x20) {
        return;
    }
    _gun_time = 1;
    _cur_scene = rect->scene;
}

void GameMaddog::_rect_mineitem1(const Scene *scene, const Rect *rect) {
    if (_pro_clue != 0) {
        _pause_time = 0;
        return;
    }

    if (_had_lantern) {
        _cur_scene = _pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_mineitem2(const Scene *scene, const Rect *rect) {
    if (_pro_clue != 2) {
        _pause_time = 0;
        return;
    }

    if (_had_lantern) {
        _cur_scene = _pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_mineitem3(const Scene *scene, const Rect *rect) {
    if (_pro_clue != 1) {
        _pause_time = 0;
        return;
    }

    if (_had_lantern) {
        _cur_scene = _pick_map();
    } else {
        _got_clue = true;
    }
}

void GameMaddog::_rect_minelantern(const Scene *scene, const Rect *rect) {
    _had_lantern = true;

    if (!_got_clue) {
        return;
    }

	_cur_scene = _pick_map();
}

void GameMaddog::_rect_shothideout(const Scene *scene, const Rect *rect) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_rect_shotright(const Scene *scene, const Rect *rect) {
    _cur_scene = _map_right();
}

void GameMaddog::_rect_shotleft(const Scene *scene, const Rect *rect) {
    _cur_scene = _map_left();
}

void GameMaddog::_rect_shotmenu(const Scene *scene, const Rect *rect) {
	_inmenu = true;
	_DoMenu();
	_inmenu = false;
}

void GameMaddog::_rect_exit(const Scene *scene, const Rect *rect) {
	_exit_caught = 2;
	// _exit(); // TODO
}

void GameMaddog::_rect_continue(const Scene *scene, const Rect *rect) {
    _in_menu = 0;
    _fired = 0;

    if (_lives == 0) {
        _NewGame();
        _ret_scene = "";
        _cur_scene = _pick_town();
    }
}

void GameMaddog::_rect_save(const Scene *scene, const Rect *rect) {
    _SaveState("state.gam");
    _DoSaveSound();
}

void GameMaddog::_rect_load(const Scene *scene, const Rect *rect) {
    _LoadState("state.gam");
    _DoLoadSound();
}

void GameMaddog::_rect_easy(const Scene *scene, const Rect *rect) {
	_DoDiffSound(1);
	_ChangeDifficulty(1);
}

void GameMaddog::_rect_average(const Scene *scene, const Rect *rect) {
	_DoDiffSound(2);
	_ChangeDifficulty(2);
}

void GameMaddog::_rect_hard(const Scene *scene, const Rect *rect) {
	_DoDiffSound(3);
	_ChangeDifficulty(3);
}

void GameMaddog::_rect_start(const Scene *scene, const Rect *rect) {
    _in_menu = false;
    _fired = false;
    if (scene->nxtscn == "DRAWGUN") {
        // scene->nxtscn(); // TODO fix
    }
    _cur_scene = _startscene;
    _ResetParams();
    _NewGame();
    _UpdateStat();
}

void GameMaddog::_rect_startbottles(const Scene *scene, const Rect *rect) {
    _in_menu = 0;
    _fired = 0;
	_cur_scene = "scene7";
    _ResetParams();
    _NewGame();
    _UpdateStat();
}

// Script functions: Scene PreOps
void GameMaddog::_scene_po_drawrct(const Scene *scene) {
	if (scene->name.equals("scene28")) {
		_cur_scene = _pick_town();
    }
}

void GameMaddog::_scene_po_pause(const Scene *scene) {
	if (scene->name.equals("scene28")) {
		_cur_scene = _pick_town();
	}
    _had_pause = false;
    _pause_time = false;
}

void GameMaddog::_scene_pso_shootout(const Scene *scene) {
    sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_min_f, &_max_f);
    _shots = 0;
    _in_shootout = 1;
    _UpdateStat();
    _RestoreCursor();
    // _DrawCAnImage(0xA000, _reloadicon, 0xB0, 0x40, _so_store); // TODO
    _DoCursor();
    _pp_flgs = 3;
}

void GameMaddog::_scene_pso_mdshootout(const Scene *scene) {
    sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_min_f, &_max_f);
    _shots = 0;
    _in_shootout = 1;
    _UpdateStat();
    _RestoreCursor();
    // _DrawCAnImage(0xA000, _reloadicon, 0xB0, 0x40, _so_store); // TODO
    _DoCursor();
}

void GameMaddog::_scene_pso_fadein(const Scene *scene) {
    if (scene->name == "scene28") {
        _cur_scene = _pick_town();
    }
    _pp_flgs = 3;
}

void GameMaddog::_scene_pso_paus_fi(const Scene *scene) {
	_scene_po_pause(scene);
	_scene_pso_fadein(scene);
}

void GameMaddog::_scene_pso_preread(const Scene *scene) {
    if (_cur_scene == "scene28") {
        _cur_scene = _pick_town();
    }
    _pp_flgs = 3;
}

void GameMaddog::_scene_pso_paus_pr(const Scene *scene) {
	_scene_po_pause(scene);
	_scene_pso_preread(scene);
}

// Script functions: Scene Scene InsOps
void GameMaddog::_scene_iso_donothing(const Scene *scene) {
	// do nothing
}

void GameMaddog::_scene_iso_pause(const Scene *scene) {
    uint32 endFrame = scene->endFrame;
    if (_had_pause) return;
    if (_frm > endFrame) return;
    if (scene->dataParam1 <= 0) return;
    unsigned long pauseStart = _frm + _videoFrameSkip;
    unsigned long pauseEnd = _frm + _videoFrameSkip - 1;
    if (pauseStart <= endFrame && pauseEnd >= endFrame) {
        _game_timer = 0;
        unsigned long pauseDuration = scene->dataParam1 * 0x90FF;
        _pause_time = pauseDuration;
        _Pause(pauseDuration);
        _pause_time += _GetUsTime();
        _had_pause = 1;
    }
    if (_pause_time != 0) {
        if (_GetUsTime() > _pause_time) {
            _pause_time = 0;
        }
    }
}

void GameMaddog::_scene_iso_startgame(const Scene *scene) {
	_ss_flag = 0;
}

void GameMaddog::_scene_iso_skipsaloon(const Scene *scene) {
    if (_got_into & 1) {
        if (_frm < 7561) {
            _cur_scene = scene->insopParam;
            _pause_time = 0;
            return;
        }
    }
    if (_frm > 7561) {
        _got_into |= 1;
    }
    if (_fired && _frm > 7165 && _frm < 7817) {
            _cur_scene = scene->insopParam;
    }
}

void GameMaddog::_scene_iso_skipsaloon2(const Scene *scene) {
    // TODO fix
    // Common::String insopParamTemp = scene->insopParam;
    // scene->insopParam = Common::String::format("%u", scene->dataParam2);
    _scene_iso_pause(scene);
    // scene->insopParam = insopParamTemp;
    _scene_iso_skipsaloon(scene);
}

void GameMaddog::_scene_iso_checksaloon(const Scene *scene) {
    _got_into |= 1;
    if (_frm > 7909) {
        _bartender_alive = false;
    } else {
        _bartender_alive = true;
    }
}

void GameMaddog::_scene_iso_intostable(const Scene *scene) {
	_got_into |= 2;
}

void GameMaddog::_scene_iso_intoffice(const Scene *scene) {
	_got_into |= 8;
}

void GameMaddog::_scene_iso_intobank(const Scene *scene) {
    _got_into |= 4;
    _scene_iso_shootpast(scene);
}

void GameMaddog::_scene_iso_chkbartndr(const Scene *scene) {
    if (!_bartender_alive) {
        if (scene->dataParam1 <= _frm) {
			_cur_scene = scene->insopParam;
        }
    }

    if (_fired) {
        if (scene->dataParam2 < _frm) {
            _cur_scene = scene->nxtscn;
        }
    }
}

void GameMaddog::_scene_iso_didhideout(const Scene *scene) {
	_been_to |= 0x80;
}

void GameMaddog::_scene_iso_didsignpost(const Scene *scene) {
	_been_to |= 0x40;
}

void GameMaddog::_scene_iso_doshootout(const Scene *scene) {
    if (_frm > (uint32)_min_f) {
        if (_in_shootout) {
            _RestoreCursor();
            // _DrawAnImage(0xA000, word_2816E, _drawicon, 0xB0, 0x40); // TODO
            _DoCursor();
        }

        _in_shootout = 0;

        if (_shots != 0 || _num_players == 2) {
            if (_frm < (uint32)_max_f) {
                _cur_scene = scene->nxtscn;
            }
        }
    }
}

void GameMaddog::_scene_iso_mdshootout(const Scene *scene) {
    _been_to |= 0x100;
    _scene_iso_doshootout(scene);
}

void GameMaddog::_scene_iso_shootpast(const Scene *scene) {
    if (_fired) {
        if (_ret_scene != "") {
            _cur_scene = _ret_scene;
            _ret_scene = "";
            _pp_force = 3;
        }
        else if (_sub_scene != "") {
            _cur_scene = _sub_scene;
            _sub_scene = "";
            _pp_force = 3;
        }
        else {
            _cur_scene = scene->nxtscn;
        }
    }
}

void GameMaddog::_scene_iso_spause(const Scene *scene) {
	_scene_iso_shootpast(scene);
	_scene_iso_pause(scene);
}

void GameMaddog::_scene_iso_shotinto24(const Scene *scene) {
	// do nothing
}

void GameMaddog::_scene_iso_shotinto116(const Scene *scene) {
	uint32 targetFrame = atoi(scene->insopParam.c_str());
    if (_fired != 0) {
        if (_frm > targetFrame) {
            _cur_scene = scene->nxtscn;
        }
    }
}

// Script functions: Scene Scene NxtScn
void GameMaddog::_scene_default_nxtscn(const Scene *scene) {
    if (scene->next == "scene28") {
        _cur_scene = _pick_town();
    } else {
        _cur_scene = scene->next;
    }
    
}

void GameMaddog::_scene_nxtscn_pickbottle(const Scene *scene) {
    _bottles++;
    if (_bottles < 4) {
        int rand = _pick_rand(6, &_botmask);
		_cur_scene = Common::String::format("scene%d", rand + 11);
    } else {
        _cur_scene = "scene253";
    }
}

void GameMaddog::_scene_nxtscn_died(const Scene *scene) {
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
            // _last_scene = _cur_scene; // TODO ??? seems unused
            _sub_scene = "scene255";
        }
        _die();
        return;
    }

    if (_cur_scene == "scene28") {
        _ret_scene = _pick_town();
    } else {
        _ret_scene = _cur_scene;
    }

    _die();
}

void GameMaddog::_scene_nxtscn_autosel(const Scene *scene) {
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

void GameMaddog::_scene_nxtscn_finsaloon(const Scene *scene) {
	_been_to |= 1;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finoffice(const Scene *scene) {
	_been_to |= 8;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finstable(const Scene *scene) {
	_been_to |= 2;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finbank(const Scene *scene) {
	_been_to |= 4;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_picsaloon(const Scene *scene) {
	if (_been_to & 1) {
		_cur_scene = "scene118";
	} else {
		_cur_scene = "scene119";
	}
}

void GameMaddog::_scene_nxtscn_killman(const Scene *scene) {
	_player--;
	if (_player <= 0) {
		_player = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
	_UpdateStat();
	_bad_men_bits = 0;
	_bad_men = 0;
	if (_people_killed == 0) {
		_cur_scene = "scene155";
	} else {
		_cur_scene = Common::String::format("scene%d", (_people_killed & 1) + 156);
	}
	_people_killed++;
}

void GameMaddog::_scene_nxtscn_killwoman(const Scene *scene) {
	_player--;
	if (_player <= 0) {
		_player = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
    _UpdateStat();
    _bad_men_bits = 0;
    _bad_men = 0;
    if (_people_killed == 0) {
        _cur_scene = "scene154";
    } else {
		_cur_scene = Common::String::format("scene%d", (_people_killed & 1) + 156);
    }
    _people_killed++;
}

void GameMaddog::_scene_nxtscn_bank(const Scene *scene) {
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
        int nextSceneNum = _pick_bad(6) + 51;
		newScene = Common::String::format("scene%d", nextSceneNum);
    }
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_stable(const Scene *scene) {
    Common::String newScene;
    long threshold = (_difficulty * 2) + 6;
    int adjustment = (_been_to & 8) ? 2 : 0;
    threshold -= adjustment;
    _bad_men++;
    if (_bad_men < threshold) {
        _bad_men_bits = 0;
        _bad_men = 0;
        newScene = "scene143";
    } else {
        int nextSceneNum = _pick_bad(6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
    }
    _cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_savprosp(const Scene *scene) {
    _gun_time = 1;
    _old_score = -1;
	_pro_clue = _rnd->getRandomNumber(2);
    _been_to |= 0x10;
	_cur_scene = Common::String::format("scene%d", _pro_clue + 160);
}

void GameMaddog::_scene_nxtscn_picktoss(const Scene *scene) {
    int index = _pick_bad(7);
	_cur_scene = Common::String::format("scene%d", _bottle_toss[index]);
}

void GameMaddog::_scene_nxtscn_hittoss(const Scene *scene) {
    if (_lives > 0) {
        _score += 100;
    }
    _scene_nxtscn_misstoss(scene);
}

void GameMaddog::_scene_nxtscn_misstoss(const Scene *scene) {
    if (++_bad_men <= 2) {
        _cur_scene = scene->next;
    } else {
        _bad_men_bits = 0;
        _bad_men = 0;
        _been_to |= 0x200;
		_cur_scene = "scene185";
    }
}

void GameMaddog::_scene_nxtscn_picksign(const Scene *scene) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_scene_nxtscn_brockman(const Scene *scene) {
    long max_bad_men = (_difficulty * 2) + 9;
    _bad_men++;
    if (_bad_men > max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        _cur_scene = _pick_sign();
    } else {
        int nextBad = _pick_bad(7);
		_cur_scene = Common::String::format("scene%d", nextBad + 229);
    }
}

void GameMaddog::_scene_nxtscn_lrockman(const Scene *scene) {
    long max_bad_men = (_difficulty * 2) + 4;
    _bad_men++;
    if (_bad_men > max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        _cur_scene = _pick_sign();
    } else {
        int nextBad = _pick_bad(3);
		_cur_scene = Common::String::format("scene%d", nextBad + 244);
    }
}

void GameMaddog::_scene_nxtscn_hotelmen(const Scene *scene) {
    long max_bad_men = (_difficulty * 2) + 9;

    if (_bad_men >= max_bad_men) {
        _bad_men_bits = 0;
        _bad_men = 0;
        _been_to |= 0x100;
        _cur_scene = "scene250";
    } else {
        _bad_men++;
        uint32 index = _pick_bad(5);
		_cur_scene = Common::String::format("scene%d", _hotel_scenes[index]);
    }
}

void GameMaddog::_scene_nxtscn_drawgun(const Scene *scene) {
    _RestoreCursor();
    // TODO: fix
    // DrawAnImage(0xA000, word_2816A, _so_store, 0xB0, 0x40);
    _DoCursor();
    _scene_default_nxtscn(scene);
}

// Script functions: ShowMsg
void GameMaddog::_scene_sm_donothing(const Scene *scene) {
	// do nothing
}

// Script functions: WepDwn
void GameMaddog::_scene_default_wepdwn(const Scene *scene) {
    _inholster = 9;
    _whichgun = 7;

    _UpdateMouse();

    if (_in_shootout == 0) {
        if (_been_to >= 15) {
            if (_shots < 12) {
                _shots = 12;
            }
        } else {
            if (_shots < 6) {
                _shots = 6;
            }
        }
        _UpdateStat();
    }
}

// Script functions: ScnScr
void GameMaddog::_scene_default_score(const Scene *scene) {
    // TODO fix
    /*
    if (scene->score > 0) {
        _score += scene->score;
    }
    */
}

// Script functions: ScnNxtFrm
void GameMaddog::_scene_nxtfrm(const Scene *scene) {
    if (_pause_time || _ss_flag) {
        return;
    }

    // TODO remove/fix
    /*
    int result = photoPlay(3, 0, 0, 0x38, 0x20, &_curfrm, &_timlft, 0, 0);
    if (result < 0) {
        unsigned long diff = scene->field_8 - scene->field_4 + 1;
        _curfrm = diff;
        return -1;
    }

    if (_debug_drawRects) {
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
    */
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
    switch (val) {
        case 0:
            _been_to = 0;
            _cur_scene = "scene28";
            break;
        case 1:
            _been_to = 1;
            _cur_scene = _pick_town();
            break;
        case 2:
            _been_to = 15;
            _cur_scene = _pick_town();
            break;
        case 3:
            _been_to = 575;
            // always go left // TODO verify
            _map0 = -1;
            _map1 = -1;
            _map2 = -1;
            _cur_scene = _pick_town();
            break;
        case 4:
            _been_to = 575;
            _hide_out_front = true;
            _cur_scene = "scene210";
            break;
        case 5:
            _been_to = 639;
            _cur_scene = "scene227";
            break;
        case 6:
            _been_to = 1023;
            _cur_scene = "scene250";
            break;
        default:
            break;
    }
}

} // End of namespace Alg
