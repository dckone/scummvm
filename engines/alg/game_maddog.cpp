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

#include "common/debug.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"

#include "alg/game_maddog.h"
#include "alg/graphics.h"
#include "alg/scene.h"

namespace Alg {

GameMaddog::GameMaddog(AlgEngine *vm) : Game(vm) {
}

GameMaddog::~GameMaddog() {
}

void GameMaddog::init() {
	_videoPosX = MADDOG_VIDEO_POS_X;
	_videoPosY = MADDOG_VIDEO_POS_Y;

	loadLibArchive("MADDOG.LIB");
	_sceneInfo->loadScnFile("MADDOG.SCN");
	_startscene = _sceneInfo->getStartScene();

	registerScriptFunctions();
	verifyScriptFunctions();

	_menuzone = new Zone();
	_menuzone->name = "MainMenu";
	_menuzone->ptrfb = "GLOBALHIT";
	_menuzone->addRect(0x0C, 0xAC, 0x3D, 0xBF, nullptr, 0, "SHOTMENU", "0");
	_menuzone->addRect(0x00, 0xA6, 0x013F, 0xC7, nullptr, 0, "DEFAULT", "0");
	_menuzone->addRect(0x00, 0x00, 0x3B, 0xC7, nullptr, 0, "DEFAULT", "0");

	_submenzone = new Zone();
	_submenzone->name = "SubMenu";
	_submenzone->ptrfb = "GLOBALHIT";
	_submenzone->addRect(0x8A, 0x3B, 0xC2, 0x48, nullptr, 0, "STARTBOT", "0");
	_submenzone->addRect(0x8A, 0x4E, 0xC2, 0x59, nullptr, 0, "STARTMENU", "0");
	_submenzone->addRect(0x8A, 0x60, 0xC2, 0x6B, nullptr, 0, "CONTMENU", "0");
	_submenzone->addRect(0xE3, 0x3B, 0x011B, 0x48, nullptr, 0, "RECTSAVE", "0");
	_submenzone->addRect(0xE3, 0x4E, 0x011B, 0x59, nullptr, 0, "RECTLOAD", "0");
	_submenzone->addRect(0xE3, 0x60, 0x011B, 0x6B, nullptr, 0, "EXITMENU", "0");
	_submenzone->addRect(0x42, 0x34, 0x5C, 0x4E, nullptr, 0, "RECTEASY", "0");
	_submenzone->addRect(0x42, 0x53, 0x5C, 0x70, nullptr, 0, "RECTAVG", "0");
	_submenzone->addRect(0x42, 0x72, 0x62, 0x8A, nullptr, 0, "RECTHARD", "0");

	_shotsound = _LoadSoundFile("blow.8b");
	_emptysound = _LoadSoundFile("empty.8b");
	_savesound = _LoadSoundFile("saved.8b");
	_loadsound = _LoadSoundFile("loaded.8b");
	_skullsound = _LoadSoundFile("skull.8b");
	_easysound = _LoadSoundFile("deputy.8b");
	_avgsound = _LoadSoundFile("sheriff.8b");
	_hardsound = _LoadSoundFile("marshall.8b");

	_gun = AlgGraphics::loadAniCursor("gun.ani");
	_numbers = AlgGraphics::loadAniImage("numbers.ani");
	Common::Array<Graphics::Surface> *bullet = AlgGraphics::loadAniImage("bullet.ani");
	_shoticon = (*bullet)[0];
	_emptyicon = (*bullet)[1];
	Common::Array<Graphics::Surface> *hat = AlgGraphics::loadAniImage("hat.ani");
	_liveicon = (*hat)[0];
	_deadicon = (*hat)[1];
	Common::Array<Graphics::Surface> *shootout = AlgGraphics::loadAniImage("shootout.ani");
	_reloadicon = (*shootout)[0];
	_drawicon = (*shootout)[1];
	Common::Array<Graphics::Surface> *knife = AlgGraphics::loadAniImage("knife.ani");
	_knife = (*knife)[0];
	Common::Array<Graphics::Surface> *hole = AlgGraphics::loadAniCursor("hole.ani");
	_bullethole = (*hole)[0];

	_background = AlgGraphics::loadVgaBackground("BACKGRND.VGA", _palette);
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);

	_MoveMouse();
}

void GameMaddog::registerScriptFunctions() {
#define ZONE_PTRFB_FUNCTION(name, func) _zonePtrFb[name] = new ScriptFunctionPoint(this, &func);
	ZONE_PTRFB_FUNCTION("DEFAULT", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("BULLETHOLE", _zone_bullethole);
	ZONE_PTRFB_FUNCTION("SKULL", _zone_skullhole);
	ZONE_PTRFB_FUNCTION("GLOBALHIT", _zone_globalhit);
#undef ZONE_PTRFB_FUNCTION

#define RECT_HIT_FUNCTION(name, func) _rectHitFuncs[name] = new ScriptFunctionRect(this, &func);
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
	RECT_HIT_FUNCTION("RECTSAVE", _rect_save);
	RECT_HIT_FUNCTION("RECTLOAD", _rect_load);
	RECT_HIT_FUNCTION("RECTEASY", _rect_easy);
	RECT_HIT_FUNCTION("RECTAVG", _rect_average);
	RECT_HIT_FUNCTION("RECTHARD", _rect_hard);
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
	for (scene = scenes.begin(); scene != scenes.end(); ++scene) {
		getScriptFunctionScene(PREOP, scene->preop);
		// TODO: SHOWMSG
		getScriptFunctionScene(INSOP, scene->insop);
		getScriptFunctionScene(WEPDWN, scene->wepdwn);
		getScriptFunctionScene(SCNSCR, scene->scnscr);
		getScriptFunctionScene(NXTFRM, scene->nxtfrm);
		getScriptFunctionScene(NXTSCN, scene->nxtscn);
		Common::Array<Zone>::iterator zone;
		for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
			getScriptFunctionZonePtrFb(zone->ptrfb);
			Common::Array<Rect>::iterator rect;
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

ScriptFunctionRect GameMaddog::getScriptFunctionRectHit(Common::String name) {
	ScriptFunctionRectMap::iterator it = _rectHitFuncs.find(name);
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

void GameMaddog::callScriptFunctionRectHit(Common::String name, Rect *rect) {
	ScriptFunctionRect function = getScriptFunctionRectHit(name);
	function(rect);
}

void GameMaddog::callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene) {
	ScriptFunctionScene function = getScriptFunctionScene(type, name);
	function(scene);
}

Common::Error GameMaddog::run() {
	init();
	_NewGame();
	_cur_scene = _startscene;
	Common::String oldscene;
	while (!_vm->shouldQuit()) {
		oldscene = _cur_scene;
		_SetFrame();
		_fired = false;
		Scene *scene = _sceneInfo->findScene(_cur_scene);
		loadScene(scene);
		Audio::PacketizedAudioStream *audioStream = _videoDecoder->getAudioStream();
		g_system->getMixer()->stopHandle(_sceneAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_sceneAudioHandle, audioStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		_nextFrameTime = _GetMsTime() + 100;
		callScriptFunctionScene(PREOP, scene->preop, scene);
		_frm = _GetFrame(scene);
		while (_frm <= scene->endFrame && _cur_scene == oldscene && !_vm->shouldQuit()) {
			_timer();
			_MoveMouse();
			// TODO: call scene->messageFunc
			callScriptFunctionScene(INSOP, scene->insop, scene);
			_holster = _WeaponDown();
			if (_holster) {
				callScriptFunctionScene(WEPDWN, scene->wepdwn, scene);
			}
			Common::Point firedCoords;
			if (__Fired(&firedCoords)) {
				if (!_holster) {
					Rect *hitGlobalRect = _CheckZone(_menuzone, &firedCoords);
					if (hitGlobalRect == nullptr) {
						if (_shots > 0) {
							_shots--;
							_UpdateStat();
							Zone *hitSceneZone = _CheckZones(scene, &firedCoords);
							if (hitSceneZone != nullptr) {
								callScriptFunctionZonePtrFb(hitSceneZone->ptrfb, &firedCoords);
							} else {
								_default_bullethole(&firedCoords);
							}
						} else {
							_default_empty_sound();
						}
					}
				}
			}
			if (_cur_scene == oldscene) {
				callScriptFunctionScene(NXTFRM, scene->nxtfrm, scene);
			}
			_DisplayScore();
			if (_pause_time > 0) {
				g_system->getMixer()->pauseHandle(_sceneAudioHandle, true);
			} else {
				g_system->getMixer()->pauseHandle(_sceneAudioHandle, false);
			}
			if (_videoDecoder->getCurrentFrame() == 0) {
				_videoDecoder->getNextFrame();
			}
			int32 remainingMillis = _nextFrameTime - _GetMsTime();
			if (remainingMillis < 10) {
				if (_videoDecoder->getCurrentFrame() > 0) {
					_videoDecoder->getNextFrame();
				}
				remainingMillis = _nextFrameTime - _GetMsTime();
				_nextFrameTime = _GetMsTime() + (remainingMillis > 0 ? remainingMillis : 0) + 100;
			}
			if (remainingMillis > 0) {
				if (remainingMillis > 15) {
					remainingMillis = 15;
				}
				g_system->delayMillis(remainingMillis);
			}
			_frm = _GetFrame(scene);
			updateScreen();
		}
		// frame limit reached or scene changed, prepare for next scene
		_player = 0;
		_had_pause = false;
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
		if (_cur_scene == "") {
			_vm->quitGame();
		}
	}
	return Common::kNoError;
}

bool GameMaddog::__Fired(Common::Point *point) {
	_fired = false;
	pollEvents();
	if (_lastMouseButton == 1) {
		if (_butdwn) {
			return false;
		}
		_fired = true;
		point->x = _lastMousePos.x;
		point->y = _lastMousePos.y;
		_butdwn = true;
		return true;
	} else {
		_butdwn = false;
		return false;
	}
}

Rect *GameMaddog::_CheckZone(Zone *zone, Common::Point *point) {
	Common::Array<Rect>::iterator rect;
	for (rect = zone->rects.begin(); rect != zone->rects.end(); ++rect) {
		if (point->x >= rect->left &&
			point->x <= rect->right &&
			point->y >= rect->top &&
			point->y <= rect->bottom) {
			callScriptFunctionRectHit(rect->rectHit, rect);
			return rect;
		}
	}
	return nullptr;
}

Zone *GameMaddog::_CheckZones(Scene *scene, Common::Point *point) {
	Common::Array<Zone>::iterator zone;
	for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
		unsigned long startFrame = zone->startFrame - _videoFrameSkip + 1;
		unsigned long endFrame = zone->endFrame + _videoFrameSkip - 1;
		if (_frm >= startFrame && _frm <= endFrame) {
			Rect *hitRect = _CheckZone(zone, point);
			if (hitRect != nullptr) {
				return zone;
			}
		}
	}
	return nullptr;
}

void GameMaddog::_DoMenu() {
	uint32 startTime = _GetMsTime();
	_RestoreCursor();
	_DoCursor();
	_in_menu = 1;
	g_system->getMixer()->pauseHandle(_sceneAudioHandle, true);
	_screen->copyRectToSurface(_background->getBasePtr(MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y), _background->pitch, MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	_ShowDifficulty(_difficulty, 0);
	while (_in_menu && !_vm->shouldQuit()) {
		Common::Point firedCoords;
		if (__Fired(&firedCoords)) {
			_CheckZone(_submenzone, &firedCoords);
		}
		g_system->delayMillis(15);
		updateScreen();
	}
	_RestoreCursor();
	_DoCursor();
	g_system->getMixer()->pauseHandle(_sceneAudioHandle, false);
	if (_had_pause) {
		unsigned long endTime = _GetMsTime();
		unsigned long timeDiff = endTime - startTime;
		_pause_time += timeDiff;
		_nextFrameTime += timeDiff;
	}
}

uint32 GameMaddog::_GetMsTime() {
	return g_system->getMillis();
}

void GameMaddog::_timer() {
	if (_game_timer < 2) {
		_game_timer++;
		return;
	}
	_game_timer = 0;
	if (_whichgun == 9) {
		if (_emptycnt > 0) {
			_emptycnt--;
		} else {
			_whichgun = 0;
			_UpdateMouse();
		}
	} else {
		if (_shotfired) {
			_whichgun++;
			if (_whichgun > 5) {
				_whichgun = 0;
				_shotfired = false;
			}
			_UpdateMouse();
		} else {
			if (_inholster > 0) {
				_inholster--;
				if (_inholster == 0 && _whichgun == 7) {
					_whichgun = 6;
					_UpdateMouse();
				}
			}
		}
	}
}

void GameMaddog::_NewGame() {
	_shots = 6;
	_lives = 3;
	_score = 0;
	_holster = false;
	_UpdateStat();
	_sub_scene = "";
}

void GameMaddog::_ResetParams() {
	_been_to = 0;
	_bottles = 0;
	_botmask = 0;
	_got_into = 0;
	_had_skull = false;
	_bad_men = 0;
	_bad_men_bits = 0;
	_people_killed = 0;
	_hide_out_front = false;
	_difficulty = 1;
	_gun_time = 0;
	_pro_clue = 0;
	_got_clue = false;
	_had_lantern = false;
	_map_pos = 0;
	_shoot_out_cnt = 0;
	_max_map_pos = 0;
	_sheriff_cnt = 0;
	_in_shootout = false;
	_ret_scene = "";
	_sub_scene = "";
}

void GameMaddog::_UpdateStat() {
	if (_lives != _oldlives) {
		if (_lives > _oldlives) {
			for (uint8 i = _oldlives; i < _lives; i++) {
				AlgGraphics::drawImage(_screen, &_liveicon, _livepos[i][0], _livepos[i][1]);
			}
		} else {
			for (uint8 i = _lives; i < _oldlives; i++) {
				AlgGraphics::drawImage(_screen, &_deadicon, _livepos[i][0], _livepos[i][1]);
			}
		}
		_oldlives = _lives;
	}
	if (_shots != _oldshots) {
		if (_shots > _oldshots) {
			for (uint8 i = _oldshots; i < _shots; i++) {
				AlgGraphics::drawImage(_screen, &_shoticon, _shotpos[i][0], _shotpos[i][1]);
			}
		} else {
			for (uint8 i = _shots; i < _oldshots; i++) {
				AlgGraphics::drawImage(_screen, &_emptyicon, _shotpos[i][0], _shotpos[i][1]);
			}
		}
		_oldshots = _shots;
	}
}

void GameMaddog::_ChangeDifficulty(int newDifficulty) {
	if (newDifficulty == _olddif) {
		return;
	}
	_ShowDifficulty(newDifficulty, 1);
	Common::Array<Scene> scenes = _sceneInfo->getScenes();
	Common::Array<Scene>::iterator scene;
	for (scene = scenes.begin(); scene != scenes.end(); ++scene) {
		if (!(scene->diff & 0x01)) {
			if (scene->preop == "PAUSE" || scene->preop == "PAUSFI" || scene->preop == "PAUSPR") {
				scene->dataParam1 = scene->dataParam1 * _pausdifscal[newDifficulty] / _pausdifscal[_olddif];
			}
		}
		Common::Array<Zone>::iterator zone;
		for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
			Common::Array<Rect>::iterator rect;
			for (rect = zone->rects.begin(); rect != zone->rects.end(); ++rect) {
				if (!(scene->diff & 0x02)) {
					uint32 width = rect->right - rect->left;
					width = width * _rectdifscal[newDifficulty] / _olddif;
					width = (rect->right - rect->left - width) / 2;
					uint32 height = rect->bottom - rect->top;
					height = height * _rectdifscal[newDifficulty] / _olddif;
					height = (rect->bottom - rect->top - height) / 2;
					rect->right += width;
					rect->left -= width;
					rect->top += height;
					rect->bottom -= height;
				}
			}
		}
	}
	_olddif = newDifficulty;
	_difficulty = newDifficulty;
}

void GameMaddog::_ShowDifficulty(int newDifficulty, int updateCursor) {
	if (updateCursor) {
		_RestoreCursor();
		// reset menu screen
		_screen->copyRectToSurface(_background->getBasePtr(MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y), _background->pitch, MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y, _videoDecoder->getWidth(), _videoDecoder->getHeight());
	}
	AlgGraphics::drawImage(_screen, &_knife, _diffpos[newDifficulty][0], _diffpos[newDifficulty][1]);
	if (updateCursor) {
		_DoCursor();
	}
	_olddif = newDifficulty;
}

void GameMaddog::_RestoreCursor() {
}

void GameMaddog::_DoCursor() {
	_oldwhichgun = _whichgun;
}

void GameMaddog::_UpdateMouse() {
	if (_oldwhichgun != _whichgun) {
		Graphics::PixelFormat pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
		Graphics::Surface cursor = (*_gun)[_whichgun];
		CursorMan.popAllCursors();
		CursorMan.pushCursor(cursor.getPixels(), cursor.w, cursor.h, cursor.w / 2, cursor.h / 2, 0, false, &pixelFormat);
		CursorMan.showMouse(true);
		_oldwhichgun = _whichgun;
	}
}

void GameMaddog::_MoveMouse() {
	if (_lastMousePos.x < 59 || _in_menu) {
		_whichgun = 8;
	} else if (_lastMousePos.y > 166) {
		if (_inholster == 0)
			_whichgun = 6;
		else
			_whichgun = 7;
	} else if (_whichgun > 5) {
		_whichgun = 0;
	}
	_UpdateMouse();
}

void GameMaddog::_DisplayScore() {
	if (_score == _oldscore) {
		return;
	}
	_oldscore = _score;
	Common::String scoreString = Common::String::format("%05d", _score);
	int posX = 0xDC;
	for (int i = 0; i < 5; i++) {
		int digit;
		if (scoreString[i] == '0') {
			digit = 9;
		} else {
			digit = scoreString[i] - '0' - 1;
		}
		AlgGraphics::drawImage(_screen, &(*_numbers)[digit], posX, 0xAD);
		posX += 10;
	}
}

bool GameMaddog::_WeaponDown() {
	if (_lastMouseButton == 2 && _lastMousePos.y > 168) {
		return true;
	}
	return false;
}

uint32 GameMaddog::_GetFrame(Scene *scene) {
	if (_videoDecoder->getCurrentFrame() == 0) {
		return scene->startFrame;
	}
	return scene->startFrame + (_videoDecoder->getCurrentFrame() * _videoFrameSkip) - _videoFrameSkip;
}

void GameMaddog::_SetFrame() {
	_pp_flgs |= _pp_force;
	_pp_flgs = 0;
	_pp_force = 0;
}

void GameMaddog::_SaveState() {
	Common::OutSaveFile *outSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(outSaveFile = g_system->getSavefileManager()->openForSaving(saveFileName))) {
		warning("Can't create file '%s', game not saved", saveFileName.c_str());
		return;
	}
	outSaveFile->writeUint32BE(MKTAG('A', 'L', 'G', 'S')); // header
	outSaveFile->writeByte(0); // version, unused for now
	outSaveFile->writeUint16LE(_been_to);
	outSaveFile->writeUint16LE(_got_into);
	outSaveFile->writeByte(_had_skull);
	outSaveFile->writeByte(_bad_men);
	outSaveFile->writeByte(_bad_men_bits);
	outSaveFile->writeByte(_people_killed);
	outSaveFile->writeByte(_hide_out_front);
	outSaveFile->writeByte(_difficulty);
	outSaveFile->writeByte(_pro_clue);
	outSaveFile->writeByte(_got_clue);
	outSaveFile->writeByte(_had_lantern);
	outSaveFile->writeByte(_map_pos);
	outSaveFile->writeByte(_shoot_out_cnt);
	outSaveFile->writeSByte(_map0);
	outSaveFile->writeSByte(_map1);
	outSaveFile->writeSByte(_map2);
	outSaveFile->writeByte(_max_map_pos);
	outSaveFile->writeByte(_bartender_alive);
	outSaveFile->writeByte(_sheriff_cnt);
	outSaveFile->writeByte(_in_shootout);
	outSaveFile->writeString(_cur_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_ret_scene);
	outSaveFile->writeByte(0);
	outSaveFile->writeString(_sub_scene);
	outSaveFile->writeByte(0);
	outSaveFile->finalize();
	delete outSaveFile;
}

void GameMaddog::_LoadState() {
	Common::InSaveFile *inSaveFile;
	Common::String saveFileName = _vm->getSaveStateName(0);
	if (!(inSaveFile = g_system->getSavefileManager()->openForLoading(saveFileName))) {
		debug("Can't load file '%s', game not loaded", saveFileName.c_str());
		return;
	}
	uint32 header = inSaveFile->readUint32BE();
	if (header != MKTAG('A', 'L', 'G', 'S')) {
		warning("Unkown save file, header: %d", header);
		return;
	}
	inSaveFile->skip(1); // version, unused for now
	_been_to = inSaveFile->readUint16LE();
	_got_into = inSaveFile->readUint16LE();
	_had_skull = inSaveFile->readByte();
	_bad_men = inSaveFile->readByte();
	_bad_men_bits = inSaveFile->readByte();
	_people_killed = inSaveFile->readByte();
	_hide_out_front = inSaveFile->readByte();
	_difficulty = inSaveFile->readByte();
	_pro_clue = inSaveFile->readByte();
	_got_clue = inSaveFile->readByte();
	_had_lantern = inSaveFile->readByte();
	_map_pos = inSaveFile->readByte();
	_shoot_out_cnt = inSaveFile->readByte();
	_map0 = inSaveFile->readSByte();
	_map1 = inSaveFile->readSByte();
	_map2 = inSaveFile->readSByte();
	_max_map_pos = inSaveFile->readByte();
	_bartender_alive = inSaveFile->readByte();
	_sheriff_cnt = inSaveFile->readByte();
	_in_shootout = inSaveFile->readByte();
	_cur_scene = inSaveFile->readString();
	_ret_scene = inSaveFile->readString();
	_sub_scene = inSaveFile->readString();
	delete inSaveFile;
	_ChangeDifficulty(_difficulty);
	_pp_force = 3;
}

void GameMaddog::_DoDiffSound(int difficulty) {
	switch (difficulty) {
	case 1:
		return _PlaySound(_easysound);
	case 2:
		return _PlaySound(_avgsound);
	case 3:
		return _PlaySound(_hardsound);
	}
}

void GameMaddog::_DoSaveSound() {
	_PlaySound(_savesound);
}

void GameMaddog::_DoLoadSound() {
	_PlaySound(_loadsound);
}

void GameMaddog::_default_empty_sound() {
	_PlaySound(_emptysound);
	_emptycnt = 3;
	_whichgun = 9;
	_UpdateMouse();
}

void GameMaddog::_DoSkullSound() {
	_PlaySound(_skullsound);
}

void GameMaddog::_DoShot() {
	_PlaySound(_shotsound);
}

void GameMaddog::_default_bullethole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
		_RestoreCursor();
		uint16 targetX = point->x - MADDOG_VIDEO_POS_X;
		uint16 targetY = point->y - MADDOG_VIDEO_POS_Y;
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bullethole, targetX, targetY);
		_DoCursor();
		_shotfired = true;
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

uint8 GameMaddog::_pick_rand(uint8 max, uint8 *maskPtr) {
	uint8 random, mask;
	// reset maskPtr if full
	if (*maskPtr == (uint8)(0xFF >> (8 - max))) {
		*maskPtr = 0;
	}
	do {
		random = _rnd->getRandomNumber(max - 1);
		mask = 1 << random;
	} while (*maskPtr & mask);
	*maskPtr |= mask;
	return random * 2;
}

uint8 GameMaddog::_pick_bad(uint8 max) {
	return _pick_rand(max, &_bad_men_bits);
}

Common::String GameMaddog::_pick_town() {
	_had_skull = false;
	_map_pos = 0;
	_bad_men_bits = 0;
	_shoot_out_cnt++;
	if (_shoot_out_cnt % 5 == 0) {
		if (_shoot_out_cnt > 15 || _shoot_out_cnt == 0) {
			_shoot_out_cnt = 5;
		}
		// surprise showdown!
		int pickedSceneNum = (_shoot_out_cnt / 5) + 106;
		return Common::String::format("scene%d", pickedSceneNum);
	} else if (_been_to == 0) {
		return "scene28";
	} else if (_been_to & 0x100) {
		return "scene250";
	} else if (_been_to & 0x80) {
		return "scene76";
	} else if (_been_to & 0x40) {
		return "scene214";
	} else if (_been_to >= 15) {
		return "scene186";
	} else {
		return Common::String::format("scene%d", _been_to + 29);
	}
}

Common::String GameMaddog::_pick_map() {
	_been_to |= 0x20;
	uint32 random = _rnd->getRandomNumber(5);
	switch (random) {
	case 0:
		_map0 = 1;
		_map1 = 1;
		_map2 = 0;
		return "scene164";
	case 1:
		_map0 = 1;
		_map1 = -1;
		_map2 = 0;
		return "scene165";
	case 2:
		_map0 = -1;
		_map1 = 1;
		_map2 = 1;
		return "scene166";
	case 3:
		_map0 = -1;
		_map1 = 1;
		_map2 = -1;
		return "scene167";
	case 4:
		_map0 = -1;
		_map1 = -1;
		_map2 = 1;
		return "scene168";
	case 5:
	default:
		_map0 = -1;
		_map1 = -1;
		_map2 = -1;
		return "scene169";
	}
}

Common::String GameMaddog::_pick_sign() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	_map_pos++;
	if (_map_pos > _max_map_pos) {
		_max_map_pos = _map_pos;
	}
	if (_map_pos <= 2 && _mapArray[_map_pos] != 0) {
		return Common::String::format("scene%d", _map_pos + 187);
	} else {
		return "scene210";
	}
}

Common::String GameMaddog::_map_right() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_map_pos] == -1) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else if (_mapArray[_map_pos] == 0) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else {
		return Common::String::format("scene%d", _ambush[_map_pos]);
	}
}

Common::String GameMaddog::_map_left() {
	int8 _mapArray[3] = {_map0, _map1, _map2};
	if (_mapArray[_map_pos] == 1) {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	} else if (_mapArray[_map_pos] == -1) {
		return Common::String::format("scene%d", _ambush[_map_pos]);
	} else {
		if (_map_pos >= _max_map_pos) {
			return Common::String::format("scene%d", _fight[_map_pos]);
		} else {
			return _pick_sign();
		}
	}
}

// Script functions: Zone
void GameMaddog::_zone_bullethole(Common::Point *point) {
	_default_bullethole(point);
}

void GameMaddog::_zone_skullhole(Common::Point *point) {
	if (point->x >= 59 && point->y <= 166) {
		uint16 targetX = point->x - MADDOG_VIDEO_POS_X;
		uint16 targetY = point->y - MADDOG_VIDEO_POS_Y;
		_RestoreCursor();
		AlgGraphics::drawImageCentered(_videoDecoder->getVideoFrame(), &_bullethole, targetX, targetY);
		_DoCursor();
		_shotfired = true;

		if (_had_skull) {
			_DoShot();
		} else {
			_DoSkullSound();
		}
	}
}

void GameMaddog::_zone_globalhit(Common::Point *point) {
	// do nothing
}

// Script functions: RectHit
void GameMaddog::_rect_newscene(Rect *rect) {
	_score += rect->score;
	if (!rect->scene.empty()) {
		_cur_scene = rect->scene;
	}
}

void GameMaddog::_rect_hidefront(Rect *rect) {
	if (_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_hiderear(Rect *rect) {
	if (!_hide_out_front) {
		_cur_scene = "scene214";
	} else {
		_cur_scene = "scene211";
	}
}

void GameMaddog::_rect_menuselect(Rect *rect) {
	Common::String newScene;
	int cursorX = _lastMousePos.x;
	int cursorY = _lastMousePos.y;

	if (cursorX < 184) {
		if (cursorY < 88) {
			if (_been_to & 2)
				return;
			if (_got_into & 2) {
				newScene = "scene130";
			} else {
				newScene = "scene122";
			}
		} else {
			if (_been_to & 8)
				return;
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
			if (_been_to & 1)
				return;
			if (_got_into & 1) {
				_pp_force = 3;
				newScene = "scene69b";
			} else {
				newScene = "scene67";
			}
		} else {
			if (_been_to & 4)
				return;
			if (_got_into & 4) {
				_pp_force = 3;
				_scene_nxtscn_bank(nullptr);
				return;
			} else {
				newScene = "scene45";
			}
		}
	}

	_cur_scene = newScene;
}

void GameMaddog::_rect_skull(Rect *rect) {
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

void GameMaddog::_rect_killman(Rect *rect) {
	_scene_nxtscn_killman(nullptr);
}

void GameMaddog::_rect_killwoman(Rect *rect) {
	_scene_nxtscn_killwoman(nullptr);
}

void GameMaddog::_rect_prospsign(Rect *rect) {
	if (_been_to & 0x10) {
		return;
	}
	_gun_time = 1;
	_cur_scene = rect->scene;
}

void GameMaddog::_rect_minesign(Rect *rect) {
	if (_been_to & 0x20) {
		return;
	}
	_gun_time = 1;
	_cur_scene = rect->scene;
}

void GameMaddog::_rect_mineitem1(Rect *rect) {
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

void GameMaddog::_rect_mineitem2(Rect *rect) {
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

void GameMaddog::_rect_mineitem3(Rect *rect) {
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

void GameMaddog::_rect_minelantern(Rect *rect) {
	_had_lantern = true;

	if (!_got_clue) {
		return;
	}

	_cur_scene = _pick_map();
}

void GameMaddog::_rect_shothideout(Rect *rect) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_rect_shotright(Rect *rect) {
	_cur_scene = _map_right();
}

void GameMaddog::_rect_shotleft(Rect *rect) {
	_cur_scene = _map_left();
}

void GameMaddog::_rect_shotmenu(Rect *rect) {
	_DoMenu();
}

void GameMaddog::_rect_exit(Rect *rect) {
	_vm->quitGame();
}

void GameMaddog::_rect_continue(Rect *rect) {
	_in_menu = false;
	_fired = false;
	if (_lives == 0) {
		_NewGame();
		_ret_scene = "";
		_cur_scene = _pick_town();
	}
}

void GameMaddog::_rect_save(Rect *rect) {
	_SaveState();
	_DoSaveSound();
}

void GameMaddog::_rect_load(Rect *rect) {
	_LoadState();
	_DoLoadSound();
}

void GameMaddog::_rect_easy(Rect *rect) {
	_DoDiffSound(1);
	_ChangeDifficulty(1);
}

void GameMaddog::_rect_average(Rect *rect) {
	_DoDiffSound(2);
	_ChangeDifficulty(2);
}

void GameMaddog::_rect_hard(Rect *rect) {
	_DoDiffSound(3);
	_ChangeDifficulty(3);
}

void GameMaddog::_rect_start(Rect *rect) {
	_in_menu = false;
	_fired = false;
	Scene *scene = _sceneInfo->findScene(_startscene);
	if (scene->nxtscn == "DRAWGUN") {
		callScriptFunctionScene(NXTSCN, "DRAWGUN", scene);
	}
	_cur_scene = _startscene;
	_ResetParams();
	_NewGame();
	_UpdateStat();
}

void GameMaddog::_rect_startbottles(Rect *rect) {
	_in_menu = false;
	_fired = false;
	_cur_scene = "scene7";
	_ResetParams();
	_NewGame();
	_UpdateStat();
}

// Script functions: Scene PreOps
void GameMaddog::_scene_po_drawrct(Scene *scene) {
	if (scene->name.equals("scene28")) {
		_cur_scene = _pick_town();
	}
}

void GameMaddog::_scene_po_pause(Scene *scene) {
	if (scene->name.equals("scene28")) {
		_cur_scene = _pick_town();
	}
	_had_pause = false;
	_pause_time = 0;
}

void GameMaddog::_scene_pso_shootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_min_f, &_max_f);
	_shots = 0;
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadicon, 0x40, 0xB0);
	_DoCursor();
	_pp_flgs = 3;
}

void GameMaddog::_scene_pso_mdshootout(Scene *scene) {
	sscanf(scene->preopParam.c_str(), "#%ldto%ld", &_min_f, &_max_f);
	_shots = 0;
	_in_shootout = true;
	_UpdateStat();
	_RestoreCursor();
	AlgGraphics::drawImage(_screen, &_reloadicon, 0x40, 0xB0);
	_DoCursor();
}

void GameMaddog::_scene_pso_fadein(Scene *scene) {
	if (scene->name == "scene28") {
		_cur_scene = _pick_town();
	}
	_pp_flgs = 3;
}

void GameMaddog::_scene_pso_paus_fi(Scene *scene) {
	_scene_po_pause(scene);
	_scene_pso_fadein(scene);
}

void GameMaddog::_scene_pso_preread(Scene *scene) {
	if (_cur_scene == "scene28") {
		_cur_scene = _pick_town();
	}
	_pp_flgs = 3;
}

void GameMaddog::_scene_pso_paus_pr(Scene *scene) {
	_scene_po_pause(scene);
	_scene_pso_preread(scene);
}

// Script functions: Scene Scene InsOps
void GameMaddog::_scene_iso_donothing(Scene *scene) {
	// do nothing
}

void GameMaddog::_scene_iso_pause(Scene *scene) {
	bool checkPause = true;
	if (_had_pause) {
		checkPause = false;
	}
	if (_frm > scene->endFrame) {
		checkPause = false;
	}
	if (scene->dataParam1 <= 0) {
		checkPause = false;
	}
	if (checkPause) {
		unsigned long pauseStart = atoi(scene->insopParam.c_str());
		unsigned long pauseEnd = atoi(scene->insopParam.c_str()) + _videoFrameSkip + 1;
		if (_frm >= pauseStart && _frm < pauseEnd && !_had_pause) {
			_game_timer = 0;
			unsigned long pauseDuration = scene->dataParam1 * 0x90FF / 1000;
			_pause_time = pauseDuration;
			_nextFrameTime += pauseDuration;
			_pause_time += _GetMsTime();
			_had_pause = true;
		}
	}
	if (_pause_time != 0) {
		if (_GetMsTime() > _pause_time) {
			_pause_time = 0;
		}
	}
}

void GameMaddog::_scene_iso_startgame(Scene *scene) {
	_ss_flag = 0;
	_startscene = scene->insopParam;
}

void GameMaddog::_scene_iso_skipsaloon(Scene *scene) {
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

void GameMaddog::_scene_iso_skipsaloon2(Scene *scene) {
	Common::String insopParamTemp = scene->insopParam;
	scene->insopParam = Common::String::format("%u", scene->dataParam2);
	_scene_iso_pause(scene);
	scene->insopParam = insopParamTemp;
	_scene_iso_skipsaloon(scene);
}

void GameMaddog::_scene_iso_checksaloon(Scene *scene) {
	_got_into |= 1;
	if (_frm > 7909) {
		_bartender_alive = false;
	} else {
		_bartender_alive = true;
	}
}

void GameMaddog::_scene_iso_intostable(Scene *scene) {
	_got_into |= 2;
}

void GameMaddog::_scene_iso_intoffice(Scene *scene) {
	_got_into |= 8;
}

void GameMaddog::_scene_iso_intobank(Scene *scene) {
	_got_into |= 4;
	_scene_iso_shootpast(scene);
}

void GameMaddog::_scene_iso_chkbartndr(Scene *scene) {
	if (!_bartender_alive) {
		if (scene->dataParam1 <= _frm) {
			_cur_scene = scene->insopParam;
		}
	}
	if (_fired) {
		if (scene->dataParam2 < _frm) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

void GameMaddog::_scene_iso_didhideout(Scene *scene) {
	_been_to |= 0x80;
}

void GameMaddog::_scene_iso_didsignpost(Scene *scene) {
	_been_to |= 0x40;
}

void GameMaddog::_scene_iso_doshootout(Scene *scene) {
	if (_frm > (uint32)_min_f) {
		if (_in_shootout) {
			_RestoreCursor();
			AlgGraphics::drawImage(_screen, &_drawicon, 0x40, 0xB0);
			_DoCursor();
		}
		_in_shootout = false;
		if (_shots != 0 || _num_players == 2) {
			if (_frm < (uint32)_max_f) {
				callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
			}
		}
	}
}

void GameMaddog::_scene_iso_mdshootout(Scene *scene) {
	_been_to |= 0x100;
	_scene_iso_doshootout(scene);
}

void GameMaddog::_scene_iso_shootpast(Scene *scene) {
	if (_fired) {
		if (_ret_scene != "") {
			_cur_scene = _ret_scene;
			_ret_scene = "";
			_pp_force = 3;
		} else if (_sub_scene != "") {
			_cur_scene = _sub_scene;
			_sub_scene = "";
			_pp_force = 3;
		} else {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

void GameMaddog::_scene_iso_spause(Scene *scene) {
	_scene_iso_shootpast(scene);
	_scene_iso_pause(scene);
}

void GameMaddog::_scene_iso_shotinto24(Scene *scene) {
	// do nothing
}

void GameMaddog::_scene_iso_shotinto116(Scene *scene) {
	uint32 targetFrame = atoi(scene->insopParam.c_str());
	if (_fired) {
		if (_frm > targetFrame) {
			callScriptFunctionScene(NXTSCN, scene->nxtscn, scene);
		}
	}
}

// Script functions: Scene Scene NxtScn
void GameMaddog::_scene_default_nxtscn(Scene *scene) {
	// wipe background drawing from shootout
	_screen->copyRectToSurface(_background->getBasePtr(0x40, 0xB0), _background->pitch, 0x40, 0xB0, _reloadicon.w, _reloadicon.h);
	_DoCursor();
	if (scene->next == "scene28") {
		_cur_scene = _pick_town();
	} else {
		_cur_scene = scene->next;
	}
}

void GameMaddog::_scene_nxtscn_pickbottle(Scene *scene) {
	_bottles++;
	if (_bottles < 4) {
		int rand = _pick_rand(6, &_botmask);
		_cur_scene = Common::String::format("scene%d", rand + 11);
	} else {
		_cur_scene = "scene253";
	}
}

void GameMaddog::_scene_nxtscn_died(Scene *scene) {
	_had_skull = false;
	_bad_men_bits = 0;
	_bad_men = 0;
	_got_clue = false;
	_had_lantern = false;
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene255";
		_die();
		return;
	}
	if (_cur_scene == "scene28") {
		_ret_scene = _pick_town();
	} else {
		_ret_scene = scene->next;
	}
	_die();
}

void GameMaddog::_scene_nxtscn_autosel(Scene *scene) {
	Common::String newScene;
	if (!(_been_to & 2)) {
		newScene = "scene122";
	} else if (!(_been_to & 8)) {
		newScene = "scene114";
	} else if (!(_been_to & 1)) {
		if (_got_into & 1) {
			newScene = "scene69";
		} else {
			newScene = "scene67";
		}
	} else if (!(_been_to & 4)) {
		newScene = "scene45";
	} else {
		newScene = "scene186";
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_finsaloon(Scene *scene) {
	_been_to |= 1;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finoffice(Scene *scene) {
	_been_to |= 8;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finstable(Scene *scene) {
	_been_to |= 2;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_finbank(Scene *scene) {
	_been_to |= 4;
	_cur_scene = _pick_town();
}

void GameMaddog::_scene_nxtscn_picsaloon(Scene *scene) {
	if (_been_to & 1) {
		_cur_scene = "scene118";
	} else {
		_cur_scene = "scene119";
	}
}

void GameMaddog::_scene_nxtscn_killman(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
	_UpdateStat();
	_bad_men_bits = 0;
	_bad_men = 0;
	_people_killed++;
	if (_people_killed == 1) {
		_cur_scene = "scene155";
	} else {
		_cur_scene = Common::String::format("scene%d", 156 + (_people_killed & 1));
	}
}

void GameMaddog::_scene_nxtscn_killwoman(Scene *scene) {
	if (!_debug_godMode) {
		_lives--;
	}
	if (_lives <= 0) {
		_lives = 0;
		_sub_scene = "scene212";
	} else {
		_sub_scene = _pick_town();
	}
	_UpdateStat();
	_bad_men_bits = 0;
	_bad_men = 0;
	_people_killed++;
	if (_people_killed == 1) {
		_cur_scene = "scene154";
	} else {
		_cur_scene = Common::String::format("scene%d", 156 + (_people_killed & 1));
	}
}

void GameMaddog::_scene_nxtscn_bank(Scene *scene) {
	Common::String newScene;
	uint8 totalBadmen = (_difficulty * 2) + 6;
	_bad_men++;
	if (_bad_men > totalBadmen) {
		if (_bad_men > totalBadmen + 2) {
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
		} else {
			newScene = "scene65";
		}
	} else {
		int nextSceneNum = _pick_bad(6) + 51;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_stable(Scene *scene) {
	Common::String newScene;
	uint16 totalBadMen = (_difficulty * 2) + 6;
	totalBadMen -= (_been_to & 8) ? 2 : 0;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		newScene = "scene143";
	} else {
		int nextSceneNum = _pick_bad(6) + 131;
		newScene = Common::String::format("scene%d", nextSceneNum);
	}
	_cur_scene = newScene;
}

void GameMaddog::_scene_nxtscn_savprosp(Scene *scene) {
	_gun_time = 1;
	_oldscore = -1;
	_pro_clue = _rnd->getRandomNumber(2);
	_been_to |= 0x10;
	_cur_scene = Common::String::format("scene%d", _pro_clue + 160);
}

void GameMaddog::_scene_nxtscn_picktoss(Scene *scene) {
	int index = _pick_bad(7);
	_cur_scene = Common::String::format("scene%d", _bottle_toss[index]);
}

void GameMaddog::_scene_nxtscn_hittoss(Scene *scene) {
	if (_lives > 0) {
		_score += 100;
	}
	_scene_nxtscn_misstoss(scene);
}

void GameMaddog::_scene_nxtscn_misstoss(Scene *scene) {
	_bad_men++;
	if (_bad_men <= 2) {
		_cur_scene = scene->next;
	} else {
		_bad_men_bits = 0;
		_bad_men = 0;
		_been_to |= 0x200;
		_cur_scene = "scene185";
	}
}

void GameMaddog::_scene_nxtscn_picksign(Scene *scene) {
	_cur_scene = _pick_sign();
}

void GameMaddog::_scene_nxtscn_brockman(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		_cur_scene = _pick_sign();
	} else {
		int nextBad = _pick_bad(7);
		_cur_scene = Common::String::format("scene%d", nextBad + 229);
	}
}

void GameMaddog::_scene_nxtscn_lrockman(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 4;
	_bad_men++;
	if (_bad_men > totalBadMen) {
		_bad_men_bits = 0;
		_bad_men = 0;
		_cur_scene = _pick_sign();
	} else {
		int nextBad = _pick_bad(3);
		_cur_scene = Common::String::format("scene%d", nextBad + 244);
	}
}

void GameMaddog::_scene_nxtscn_hotelmen(Scene *scene) {
	long totalBadMen = (_difficulty * 2) + 9;
	if (_bad_men >= totalBadMen) {
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

void GameMaddog::_scene_nxtscn_drawgun(Scene *scene) {
	_RestoreCursor();
	_scene_default_nxtscn(scene);
}

// Script functions: ShowMsg
void GameMaddog::_scene_sm_donothing(Scene *scene) {
	// do nothing
}

// Script functions: WepDwn
void GameMaddog::_scene_default_wepdwn(Scene *scene) {
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
void GameMaddog::_scene_default_score(Scene *scene) {
	if (scene->scnscrParam > 0) {
		_score += scene->scnscrParam;
	}
}

// Script functions: ScnNxtFrm
void GameMaddog::_scene_nxtfrm(Scene *scene) {
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
		// always go right
		_map0 = -1;
		_map1 = -1;
		_map2 = -1;
		_cur_scene = _pick_town();
		break;
	case 4:
		_been_to = 575;
		_hide_out_front = true; // go to front
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

// Debugger methods
DebuggerMaddog::DebuggerMaddog(GameMaddog *game) : GUI::Debugger() {
	_game = game;
	registerVar("drawrects", &game->_debug_drawRects);
	registerVar("godmode", &game->_debug_godMode);
	registerCmd("warp", WRAP_METHOD(DebuggerMaddog, cmdWarpTo));
}

bool DebuggerMaddog::cmdWarpTo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: warp <int>");
	} else {
		int val = atoi(argv[1]);
		_game->debug_warpTo(val);
	}
	return false;
}

} // End of namespace Alg
