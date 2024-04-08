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
#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"

#include "alg/graphics.h"
#include "alg/scene.h"
#include "alg/game_maddog.h"

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
}

void GameMaddog::updateScreen() {
	_screen->copyRectToSurface(_background->getPixels(), _background->pitch, 0, 0, _background->w, _background->h);
	if(!_inMenu) {
		Graphics::Surface *frame = _videoDecoder->getFrame();
		_screen->copyRectToSurface(frame->getPixels(), frame->pitch, MADDOG_VIDEO_POS_X, MADDOG_VIDEO_POS_Y, frame->w, frame->h);
	}
	debug_drawZoneRects();

	Graphics::Surface aniHat0 = (*_aniHat)[0];
	_screen->copyRectToSurface(aniHat0.getPixels(), aniHat0.pitch, 0, 120, aniHat0.w, aniHat0.h);
	g_system->copyRectToScreen(_screen->getPixels(), _screen->w, 0, 0, _screen->w, _screen->h);
	if(_paletteDirty || _videoDecoder->isPaletteDirty()) {

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
	_nextScene = "";
	_paused = false;
	_pausedFrames = 0;
	_hadPause = false;
	_skipToNextScene = false;
	_rectHit = false;

	if(scene->preop.equals("DEFAULT") || scene->preop.equals("DRAWRCT")) { /* do nothing */ }
	else if(scene->preop.equals("PAUSE")) { psoPause(scene); }
	else if(scene->preop.equals("PRESHOOTOUT")) { psoShootout(scene); }
	else if(scene->preop.equals("MDSHOOTOUT")) { psoMdShootout(scene); }
	else if(scene->preop.equals("FADEIN")) { psoFadeIn(scene); }
	else if(scene->preop.equals("PAUSFI")) { psoPause(scene); psoFadeIn(scene); }
	else if(scene->preop.equals("PREREAD")) { psoPreRead(scene);}
	else if(scene->preop.equals("PAUSPR")) { psoPause(scene); psoPreRead(scene); }
	else {
		error("Unknown PREOP: %s", scene->preop.c_str());
	}
}

void GameMaddog::duringScene(Scene *scene, uint32 currentFrame) {
	if(scene->insop.equals("DEFAULT")) { /* do nothing */ }
	else if(scene->insop.equals("PAUSE")) { scene_iso_pause(scene, currentFrame); }
	else if(scene->insop.equals("STARTGAME")) { scene_iso_startgame(scene, currentFrame); }
	else if(scene->insop.equals("SKIPSALOON")) { scene_iso_skipsaloon(scene, currentFrame); }
	else if(scene->insop.equals("SKIPSALOON2")) { scene_iso_skipsaloon2(scene, currentFrame); }
	else if(scene->insop.equals("CHECKSALOON")) { scene_iso_checksaloon(scene, currentFrame); }
	else if(scene->insop.equals("INTOSTABLE")) { scene_iso_intostable(scene, currentFrame); }
	else if(scene->insop.equals("INTOFFICE")) { scene_iso_intoffice(scene, currentFrame); }
	else if(scene->insop.equals("INTOBANK_SP")) { scene_iso_intobank(scene, currentFrame); }
	else if(scene->insop.equals("CHKBARTNDR")) { scene_iso_chkbartndr(scene, currentFrame); }
	else if(scene->insop.equals("DIDHIDEOUT")) { scene_iso_didhideout(scene, currentFrame); }
	else if(scene->insop.equals("DIDSIGNPOST")) {scene_iso_didsignpost(scene, currentFrame); }
	else if(scene->insop.equals("DOSHOOTOUT")) { scene_iso_doshootout(scene, currentFrame); }
	else if(scene->insop.equals("MDSHOOTOUT")) {scene_iso_mdshootout(scene, currentFrame); }
	else if(scene->insop.equals("SHOOTPAST")) { scene_iso_shootpast(scene, currentFrame); }
	else if(scene->insop.equals("SPAUSE")) { scene_iso_spause(scene, currentFrame); }
	else if(scene->insop.equals("SHOTINTO116")) {scene_iso_shotinto116(scene, currentFrame); }
	else {
		error("Unknown INSOP: %s", scene->insop.c_str());
	}
}

void GameMaddog::afterScene(Scene *scene) {
	if(scene->nxtscn.equals("DEFAULT")) { nxtscnDefault(scene); }
	else if(scene->nxtscn.equals("PICKBOTTLE")) { scene_nxtscn_pickbottle(scene); }
	else if(scene->nxtscn.equals("DIED")) { nxtscnDied(scene); }
	else if(scene->nxtscn.equals("AUTOSEL")) { nxtscnAutoSel(scene); }
	else if(scene->nxtscn.equals("FINSALOON")) { nxtscnFinSaloon(scene); }
	else if(scene->nxtscn.equals("FINOFFICE")) { nxtscnFinOffice(scene); }
	else if(scene->nxtscn.equals("FINSTABLE")) { nxtscnFinStable(scene); }
	else if(scene->nxtscn.equals("PICSALOON")) { nxtscnPicSaloon(scene); }
	else if(scene->nxtscn.equals("KILLMAN")) { nxtscnKillPerson(scene, "scene155"); }
	else if(scene->nxtscn.equals("KILLWOMAN")) { nxtscnKillPerson(scene, "scene154"); }
	else if(scene->nxtscn.equals("BANK")) { nxtscnBank(scene); }
	else if(scene->nxtscn.equals("STABLE")) { nxtscnStable(scene); }
	else if(scene->nxtscn.equals("SAVPROSP")) { nxtscnSavProsp(scene); }
	else if(scene->nxtscn.equals("PICKTOSS")) { nxtscnPickToss(scene); }
	else if(scene->nxtscn.equals("HITTOSS")) { nxtscnHitToss(scene); }
	else if(scene->nxtscn.equals("MISSTOSS")) { nxtscnMissToss(scene, false); }
	else if(scene->nxtscn.equals("PICKSIGN")) { nxtscnPickSign(scene); }
	else if(scene->nxtscn.equals("BROCKMAN")) { nxtscnBRockMan(scene); }
	else if(scene->nxtscn.equals("LROCKMAN")) { nxtscnLRockMan(scene); }
	else if(scene->nxtscn.equals("HOTELMEN")) { nxtscnHotelMen(scene); }
	else if(scene->nxtscn.equals("DRAWGUN")) { nxtscnDrawGun(scene); }
	else {
		error("Unknown NXTSCN: %s", scene->nxtscn.c_str());
	}
}

void GameMaddog::rectHit(Rect *rect, int16 x, int16 y) {
	if (rect->function.equals("HIDEFRONT")) { rectHideFront(rect, x, y); }
	else if (rect->function.equals("HIDEREAR")) { rectHideRear(rect, x, y); }
	else if (rect->function.equals("NEWSCENE") || rect->function.equals("DEFAULT")) { rectNewScene(rect, x, y); }
	else if (rect->function.equals("MENUSELECT")) { rectMenuSelect(rect, x, y); }
	else if (rect->function.equals("SKULL")) { rectSkull(rect, x, y); }
	else if (rect->function.equals("KILLMAN")) { nxtscnKillPerson(nullptr, "scene155"); }
	else if (rect->function.equals("KILLWOMAN")) { nxtscnKillPerson(nullptr, "scene154"); }
	else if (rect->function.equals("PROSPSIGN")) { rectProspSign(rect, x, y); }
	else if (rect->function.equals("MINESIGN")) { rectMineSign(rect, x, y); }
	else if (rect->function.equals("MINEITEM1")) { rectMineItem1(rect, x, y); }
	else if (rect->function.equals("MINEITEM2")) { rectMineItem2(rect, x, y); }
	else if (rect->function.equals("MINEITEM3")) { rectMineItem3(rect, x, y); }
	else if (rect->function.equals("MINELANTERN")) { rectMineLantern(rect, x, y); }
	else if (rect->function.equals("SHOTHIDEOUT")) { rectShotHideout(rect, x, y); }
	else if (rect->function.equals("SHOTRIGHT")) { rectShotDirection(rect, x, y, MAP_RIGHT); }
	else if (rect->function.equals("SHOTLEFT")) { rectShotDirection(rect, x, y, MAP_LEFT); }
	else if (rect->function.equals("SHOTMENU")) { rectShotMenu(); }
	else if (rect->function.equals("STARTMENU")) { rectStartGame(); }
	else if (rect->function.equals("STARTBOT")) { rectStartBottles(); }
	else if (rect->function.equals("CONTMENU")) { rectContinue(); }
	else if (rect->function.equals("EXITMENU")) { rectExit(); }
	else if (rect->function.equals("RECTSAVE")) { rectSave(); }
	else if (rect->function.equals("RECTLOAD")) { rectLoad(); }
	else if (rect->function.equals("RECTEASY")) { rectEasy(); }
	else if (rect->function.equals("RECTAVG")) { rectAverage(); }
	else if (rect->function.equals("RECTHARD")) { rectHard(); }
	else {
		error("Unknown RECT function: %s", rect->function.c_str());
	}
	_rectHit = true;
}

void GameMaddog::nxtscnDefault(Scene *scene) {
	if(scene->next.equals("scene28")) {
		_nextScene = pickTown();
		return;
	}
	Game::nxtscnDefault(scene); // ???
}

void GameMaddog::scene_nxtscn_pickbottle(Scene *scene) {
	// TODO: original game never picks the same random number twice
	// see _botmask
	if(_bottles >= 4) {
		_nextScene = "scene253";
	} else {
		uint32 random = _rnd->getRandomNumber(5);
		uint32 nextSceneId = 11 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
	}
	_bottles++;
}

void GameMaddog::nxtscnDied(Scene *scene) {
	debug("GameMaddog::nxtscnDied");
	// TODO: how do scenes 148 and 149 work? Apparently only for lives > 3, no idea how this should work
	_badMenBits = 0;
	_badMen = 0;
	_hadLantern = false;
	_hadSkull = false;
	_gotClue = false;
	if(_lives > 0 && !_debug_godMode) {
		_lives--;
	}
	if(_lives == 2) {
		_subScene = "scene150";
		_retScene = pickTown(); // TODO: not sure about this
	} else if(_lives == 1) {
		_subScene = "scene152";
		_retScene = pickTown(); // TODO: not sure about this
	} else if(_lives == 0) {
		_subScene = "scene153";
		_nextScene = "scene255";
	} else {
		_nextScene = pickTown();
	}
}

void GameMaddog::nxtscnAutoSel(Scene *scene) {
	error("Not implemented: _scene_nxtscn_autosel");
}

void GameMaddog::nxtscnFinSaloon(Scene *scene) {
	_beenTo |= 1;
	_nextScene = pickTown();
}

void GameMaddog::nxtscnFinOffice(Scene *scene) {
	_beenTo |= 8;
	_nextScene = pickTown();
}

void GameMaddog::nxtscnFinStable(Scene *scene) {
	_beenTo |= 2;
	_nextScene = pickTown();
}

void GameMaddog::nxtscnPicSaloon(Scene *scene) {
	if((_beenTo & 1) == 1) {
		_nextScene = "scene118";
	} else {
		_nextScene = "scene119";
	}
}

void GameMaddog::nxtscnKillPerson(Scene *scene, Common::String firstKillScene) {
	_lives--;
	_badMen = 0;
	_badMenBits = 0;
	if (_peopleKilled == 0) {
		_nextScene = firstKillScene;
	} else {
		if(_rnd->getRandomBit()) {
			_nextScene = "scene156";
		} else {
			_nextScene = "scene157";
		}
	}
	if(_lives == 0) {
		_nextScene = "scene212";
	}
	_peopleKilled++;
}

void GameMaddog::nxtscnBank(Scene *scene) {
	uint8 totalEnemies = 8 + (_difficulty * 2);
	if (_badMen < totalEnemies - 1) {
		uint32 random = _rnd->getRandomNumber(5);
		uint32 nextSceneId = 51 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
		_badMen++;
	} else if (_badMen < totalEnemies) {
		_nextScene = "scene65";
		_badMen++;
	} else {
		_badMen = 0;
		_beenTo |= 4;
		if (_rnd->getRandomBit()) {
			_nextScene = "scene48";
			_hideOutFront = 0;
		} else {
			_nextScene = "scene49";
			_hideOutFront = 1;
		}
	}
}

void GameMaddog::nxtscnStable(Scene *scene) {
	int totalEnemies = 6 + (_difficulty * 2);
	if (_badMen < totalEnemies - 1) {
		uint32 random = _rnd->getRandomNumber(5);
		uint32 nextSceneId = 131 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
		_badMen++;
	} else {
		_nextScene = "scene143";
		_badMen = 0;
	}
}

void GameMaddog::nxtscnSavProsp(Scene *scene) {
	// _gun_time = 1
	_proClue = _rnd->getRandomNumber(2);
	_beenTo |= 0x10;
	uint32 nextSceneId = 160 + _proClue;
	_nextScene = Common::String::format("scene%d", nextSceneId);
}

void GameMaddog::nxtscnPickToss(Scene *scene) {
	uint32 random = _rnd->getRandomNumber(6);
	switch(random) {
	case 0:
		_nextScene = "scene171";
		break;
	case 1:
		_nextScene = "scene174";
		break;
	case 2:
		_nextScene = "scene175";
		break;
	case 3:
		_nextScene = "scene178";
		break;
	case 4:
		_nextScene = "scene179";
		break;
	case 5:
		_nextScene = "scene182";
		break;
	case 6:
		_nextScene = "scene183";
		break;
	default:
		error("toss number invalid");
	}
}

void GameMaddog::nxtscnHitToss(Scene *scene) {
	_score += 100;
	nxtscnMissToss(scene, true);
}

void GameMaddog::nxtscnMissToss(Scene *scene, bool hit) {
	int totalTosses = 3;
	if (_badMen < totalTosses) {
		_nextScene = scene->next;
		_badMen++;
	} else {
		_nextScene = "scene185";
		_beenTo |= 0x200;
		_badMen = 0;
	}
}

void GameMaddog::nxtscnPickSign(Scene *scene) {
	_nextScene = pickSign();
}

void GameMaddog::nxtscnBRockMan(Scene *scene) {
	int totalEnemies = 9 + (_difficulty * 2);
	if (_badMen < totalEnemies) {
		uint32 random = _rnd->getRandomNumber(6);
		uint32 nextSceneId = 229 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
		_badMen++;
	} else {
		_nextScene = pickSign();
		_badMen = 0;
	}
}

void GameMaddog::nxtscnLRockMan(Scene *scene) {
	int totalEnemies = 4 + (_difficulty * 2);
	if (_badMen < totalEnemies) {
		uint32 random = _rnd->getRandomNumber(2);
		uint32 nextSceneId = 229 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
		_badMen++;
	} else {
		_nextScene = pickSign();
		_badMen = 0;
	}
}

void GameMaddog::nxtscnHotelMen(Scene *scene) {
	int totalEnemies = 9 + (_difficulty * 2);
	if (_badMen < totalEnemies - 1) {
		uint32 random = _rnd->getRandomNumber(14);
		uint32 nextSceneId = 77 + (random * 2);
		_nextScene = Common::String::format("scene%d", nextSceneId);
		_badMen++;
	} else {
		_nextScene = "scene250";
		_badMen = 0;
	}
}

void GameMaddog::psoPause(Scene *scene) {
	if (scene->name.equals("scene28")) {
		_nextScene = pickTown();
	}
	Game::psoPause(scene);
}

void GameMaddog::psoShootout(Scene *scene) {
	_shots = 0;
	_inShootout = true;
	sscanf(scene->preopParam.c_str(), "#%uto%u", &_shootoutMinF, &_shootoutMaxF);
	// TODO: change cursor
	// TODO: draw reload info to lower bar
}

void GameMaddog::psoMdShootout(Scene *scene) {
	psoShootout(scene);
}

void GameMaddog::scene_iso_skipsaloon(Scene *scene, uint32 currentFrame) {
	// TODO: verify this is really all
	if ((_gotInto & 1) == 1) {
		_nextScene = scene->next;
		_skipToNextScene = true;
		// TODO: TEST THIS!
	}
}

void GameMaddog::scene_iso_skipsaloon2(Scene *scene, uint32 currentFrame) {
	scene_iso_pause(scene, currentFrame);
	scene_iso_skipsaloon(scene, currentFrame);
}

void GameMaddog::scene_iso_checksaloon(Scene *scene, uint32 currentFrame) {
	_gotInto &= 1;
	if(currentFrame <= 7909) {
		_bartenderAlive = true;
	} else {
		_bartenderAlive = false;
	}
}

void GameMaddog::scene_iso_intostable(Scene *scene, uint32 currentFrame) {
	_gotInto &= 2;
}

void GameMaddog::scene_iso_intoffice(Scene *scene, uint32 currentFrame) {
	_gotInto &= 8;
}

void GameMaddog::scene_iso_intobank(Scene *scene, uint32 currentFrame) {
	_gotInto &= 4;
	scene_iso_shootpast(scene, currentFrame);
}

void GameMaddog::scene_iso_chkbartndr(Scene *scene, uint32 currentFrame) {
	if(!_bartenderAlive && currentFrame >= scene->dataParam1) {
		_nextScene = scene->insopParam;
		_skipToNextScene = true;
	}
}

void GameMaddog::scene_iso_didhideout(Scene *scene, uint32 currentFrame) {
	_beenTo |= 0x80;
}

void GameMaddog::scene_iso_didsignpost(Scene *scene, uint32 currentFrame) {
	_beenTo |= 0x40;
}

void GameMaddog::scene_iso_doshootout(Scene *scene, uint32 currentFrame) {
	// TODO: bottom image shows "Holster"
	if(currentFrame >= _shootoutMinF) {
		// TODO: bottom image changes from "Holster" to "Reload"
		// TODO: implement cursor change
	}
}

void GameMaddog::scene_iso_mdshootout(Scene *scene, uint32 currentFrame) {
	_beenTo |= 0x100;
	scene_iso_doshootout(scene, currentFrame);
}

void GameMaddog::scene_iso_shotinto116(Scene *scene, uint32 currentFrame) {
	// TODO: find out why we can skip before this frame? Should not be skippable!
	uint32 targetFrame = atoi(scene->insopParam.c_str());
	if (targetFrame >= currentFrame) {
		_skippable = true;
	}
}

void GameMaddog::rectHideFront(Rect *rect, int16 x, int16 y) {
	if (_hideOutFront) {
		_nextScene = "scene211";
	} else {
		_nextScene = "scene214";
	}
	_paused = false;
	_pausedFrames = 0;
}

void GameMaddog::rectHideRear(Rect *rect, int16 x, int16 y) {
	if (_hideOutFront) {
		_nextScene = "scene214";
	} else {
		_nextScene = "scene211";
	}
	_paused = false;
	_pausedFrames = 0;
}

void GameMaddog::rectNewScene(Rect *rect, int16 x, int16 y) {
	_nextScene = rect->scene;
	_skipToNextScene = true;
}

void GameMaddog::rectMenuSelect(Rect *rect, int16 x, int16 y) {
	if (y < 87) {
		if(x < 185) {
			// stable
			if ((_beenTo & 2) == 0) {
				if (_gotInto & 2) {
					_nextScene = "scene130";
				} else {
					_nextScene = "scene122";
				}
			}
		} else {
			// saloon
			if ((_beenTo & 1) == 0) {
				if ((_gotInto & 1) == 1) {
					_nextScene = "scene69b";
				} else {
					_nextScene = "scene67";
				}
			}
		}
	} else {
		if(x < 185) {
			// office
			if ((_beenTo & 8) == 0) {
				if ((_gotInto & 8) == 8) {
					if ((_beenTo & 1) == 1) {
						_nextScene = "scene118";
					} else {
						_nextScene = "scene119";
					}
				} else {
					_nextScene = "scene114";
				}
			}
		} else {
			// bank
			if ((_beenTo & 4) == 0) {
				if(_gotInto & 4) {
					nxtscnBank(nullptr);
				} else {
					_nextScene = "scene45";
				}
			}
		}
	}
	_paused = false;
	_pausedFrames = 0;
}

void GameMaddog::rectSkull(Rect *rect, int16 x, int16 y) {
	if (!_hadSkull) {
		_hadSkull = true;
		if((_beenTo & 0x0F) == 0x0F) {
			_totalAmmo = 12;
			_ammoLeft = 12;
		} else {
			_totalAmmo = 9;
			_ammoLeft = 9;
		}
	}
	// TODO: not to be done here, but elsewhere: if not _beenTo & 0x0F _totalAmmo should be 6, otherwise should be 12 (?) i think. Test this!
}

void GameMaddog::rectProspSign(Rect *rect, int16 x, int16 y) {
	if ((_beenTo & 0x10) != 0x10) {
		_nextScene = rect->scene;
		_skipToNextScene = true;
		_paused = false;
	}
}

void GameMaddog::rectMineSign(Rect *rect, int16 x, int16 y) {
	if ((_beenTo & 0x20) != 0x20) {
		_nextScene = rect->scene;
		_skipToNextScene = true;
		_paused = false;
	}
}

void GameMaddog::rectMineItem1(Rect *rect, int16 x, int16 y) {
	if (_proClue == 0) {
		if (_hadLantern) {
			_nextScene = pickMap();
			_skipToNextScene = true;
		} else {
			_gotClue = true;
		}
	} else {
		// _pause_time = 0
		_paused = false;
		_pausedFrames = 0;
	}
}

void GameMaddog::rectMineItem2(Rect *rect, int16 x, int16 y) {
	if (_proClue == 2) {
		if (_hadLantern) {
			_nextScene = pickMap();
			_skipToNextScene = true;
		} else {
			_gotClue = true;
		}
	} else {
		// _pause_time = 0
		_paused = false;
		_pausedFrames = 0;
	}
}

void GameMaddog::rectMineItem3(Rect *rect, int16 x, int16 y) {
	if (_proClue == 1) {
		if (_hadLantern) {
			_nextScene = pickMap();
			_skipToNextScene = true;
		} else {
			_gotClue = true;
		}
	} else {
		// _pause_time = 0
		_paused = false;
		_pausedFrames = 0;
	}
}

void GameMaddog::rectMineLantern(Rect *rect, int16 x, int16 y) {
	_hadLantern = true;
	if (_gotClue) {
		_nextScene = pickMap();
		_skipToNextScene = true;
	}
}

void GameMaddog::rectShotHideout(Rect *rect, int16 x, int16 y) {
	// TODO: is there more?
	_nextScene = pickSign();
}

void GameMaddog::rectShotDirection(Rect *rect, int16 x, int16 y, uint8 direction) {
	if (_mapPos > 2) {
		_nextScene = pickSign();
		return;
	}
	uint8 mapDirection = _mapPos == 0 ? _map0 : (_mapPos == 1 ? _map1 : _map2);
	if (_mapPos > 1 && mapDirection == MAP_NONE) {
		_mapPos++;
		rectShotDirection(rect, x, y, direction);
	} else if (direction == mapDirection) {
		if (_mapPos == 0) {
			_nextScene = "scene208";
		} else if (_mapPos == 1) {
			_nextScene = "scene228";
		} else {
			_nextScene = "scene243";
		}
	} else {
		if (_mapPos == 2) {
			_nextScene = "scene193";
		} else {
			_nextScene = "scene192";
		}
	}
	_paused = false;
	_pausedFrames = 0;
}

void GameMaddog::rectStartBottles() {
	// _fired = 0;
	rectStartGame();
	_bottles = 0;
	_nextScene = "scene7";
}

void GameMaddog::rectStartGame() {
	// _fired = 0;
	_nextScene = _newGameScene;
	_skipToNextScene = true;
	_inMenu = false;
	_score = 0;
	_holster = false;
	_shots = 6;
}

void GameMaddog::rectContinue() {
	_inMenu = false;
	if(_lives == 0) {
		rectStartGame();
		_nextScene = pickTown();
	}
}

Common::String GameMaddog::pickTown() {
	_hadSkull = 0;
	_mapPos = 0;
	_badMenBits = 0;
	_shootOutCnt++;
	Common::String pickedScene;
	if (_shootOutCnt % 5 == 0) {
		// surprise shootout!
		uint8 random = _rnd->getRandomNumber(2);
		if (random == 0) {
			pickedScene = "scene107";
		} else if (random == 1) {
			pickedScene = "scene108";
		} else {
			pickedScene = "scene109";
		}
	} else {
		if (_shootOutCnt > 15) {
			_shootOutCnt = 5;
		}
		if((_beenTo & 0x100) == 0x100) {
			pickedScene = "scene250";
		} else if((_beenTo & 0x80) == 0x80) {
			pickedScene = "scene76";
		} else if((_beenTo & 0x40) == 0x40) {
			pickedScene = "scene214";
		} else if((_beenTo & 0x0F) == 0x0F) {
			pickedScene = "scene186";
		} else {
			int sceneId = 29 + _beenTo;
			pickedScene = Common::String::format("scene%d", sceneId);
		}
	}
	return pickedScene;
}

Common::String GameMaddog::pickSign() {
	// TODO: finish
	Common::String pickedScene;
	_mapPos++;

	if(_mapPos > _maxMapPos) {
		_maxMapPos = _mapPos;
	}

	if(_mapPos > 2) {
		pickedScene = "scene210";
	} else {
		int sceneId = _mapPos + 187;
		pickedScene = Common::String::format("scene%d", sceneId);
	}
	return pickedScene;
}

Common::String GameMaddog::pickMap() {
	Common::String pickedScene;
	_beenTo |= 0x20;
	uint32 random = _rnd->getRandomNumber(5);
	switch(random) {
	case 0:
		pickedScene = "scene164";
		_map0 = MAP_LEFT;
		_map1 = MAP_LEFT;
		_map2 = MAP_NONE;
		break;
	case 1:
		pickedScene = "scene165";
		_map0 = MAP_LEFT;
		_map1 = MAP_RIGHT;
		_map2 = MAP_NONE;
		break;
	case 2:
		pickedScene = "scene166";
		_map0 = MAP_RIGHT;
		_map1 = MAP_LEFT;
		_map2 = MAP_LEFT;
		break;
	case 3:
		pickedScene = "scene167";
		_map0 = MAP_RIGHT;
		_map1 = MAP_LEFT;
		_map2 = MAP_RIGHT;
		break;
	case 4:
		pickedScene = "scene168";
		_map0 = MAP_RIGHT;
		_map1 = MAP_RIGHT;
		_map2 = MAP_LEFT;
		break;
	case 5:
		pickedScene = "scene169";
		_map0 = MAP_RIGHT;
		_map1 = MAP_RIGHT;
		_map2 = MAP_RIGHT;
		break;
	default:
		error("map value inconsistent");
	}
	return pickedScene;
}

DebuggerMaddog::DebuggerMaddog(GameMaddog *game) : GUI::Debugger() {
	_game = game;
	registerVar("drawrects", &game->_debug_drawRects);
	registerVar("godmode", &game->_debug_godMode);
	registerCmd("warp",  WRAP_METHOD(DebuggerMaddog, cmdWarpTo));
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
		_beenTo = 0;
		_nextScene = "scene28";
		_skipToNextScene = true;
	} else if (val == 1) {
		_beenTo = 1;
		_nextScene = pickTown();
		_skipToNextScene = true;
	} else if (val == 2) {
		_beenTo = 15;
		_nextScene = pickTown();
		_skipToNextScene = true;
	} else if (val == 3) {
		_beenTo = 575;
		_map0 = MAP_LEFT;
		_map1 = MAP_LEFT;
		_map2 = MAP_NONE;
		_nextScene = pickTown();
		_skipToNextScene = true;
	} else if (val == 4) {
		_beenTo = 575;
		_hideOutFront = true;
		_nextScene = "scene210";
		_skipToNextScene = true;
	} else if (val == 5) {
		_beenTo = 639;
		_nextScene = "scene227";
		_skipToNextScene = true;
	} else if (val == 6) {
		_beenTo = 1023;
		_nextScene = "scene250";
		_skipToNextScene = true;
	}
}

} // End of namespace Alg
