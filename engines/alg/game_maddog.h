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

#ifndef ALG_GAME_MADDOG_H
#define ALG_GAME_MADDOG_H

#include "common/random.h"
#include "common/str.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

class GameMaddog : public Game {
public:
	GameMaddog(AlgEngine *vm);
	~GameMaddog();
	void debug_warpTo(int val);

private:
	void init();
	void updateScreen();
	void beforeScene(Scene *scene);
	void duringScene(Scene *scene, uint32 currentFrame);
	void afterScene(Scene *scene);
	void rectHit(Rect *rect, int16 x, int16 y);

	// assets
	Common::Array<Graphics::Surface> *_aniAmmo;
	Common::Array<Graphics::Surface> *_aniBullet;
	Common::Array<Graphics::Surface> *_aniClip;
	Common::Array<Graphics::Surface> *_aniHat;
	Common::Array<Graphics::Surface> *_aniNumbers;
	Common::Array<Graphics::Surface> *_aniShootout;
	Common::Array<Graphics::Surface> *_aniGun;
	Common::Array<Graphics::Surface> *_aniHole;
	Common::Array<Graphics::Surface> *_aniKnife;

	// gamestate
	uint32 _beenTo = 0;
	uint32 _gotInto = 0;
	uint8 _bottles = 0;
	uint32 _peopleKilled = 0;
	bool _botmask = 0;
	bool _hadSkull = 0;
	uint8 _badMen = 0;
	uint32 _badMenBits = 0;
	bool _hideOutFront = 0;
	bool _difficulty = 1;
	uint8 _proClue = 0;
	bool _gotClue = 0;
	bool _hadLantern = 0;
	int8 _map = 0;
	int8 _map0 = 0;
	int8 _map1 = 0;
	int8 _map2 = 0;
	uint8 _mapPos = 0;
	bool _bartenderAlive = 0;
	uint8 _shootOutCnt = 0;
	bool _maxMapPos = 0;
	bool _sheriffCnt = 0;
	bool _inShootout = 0;

	uint32 _shootoutMinF = 0;
	uint32 _shootoutMaxF = 0;

	void nxtscnDefault(Scene *scene);
	void scene_nxtscn_pickbottle(Scene *scene);
	void nxtscnDied(Scene *scene);
	void nxtscnAutoSel(Scene *scene);
	void nxtscnFinSaloon(Scene *scene);
	void nxtscnFinOffice(Scene *scene);
	void nxtscnFinStable(Scene *scene);
	void nxtscnPicSaloon(Scene *scene);
	void nxtscnKillPerson(Scene *scene, Common::String firstKillScene);
	void nxtscnKillWoman(Scene *scene);
	void nxtscnBank(Scene *scene);
	void nxtscnStable(Scene *scene);
	void nxtscnSavProsp(Scene *scene);
	void nxtscnPickToss(Scene *scene);
	void nxtscnHitToss(Scene *scene);
	void nxtscnMissToss(Scene *scene, bool hit);
	void nxtscnPickSign(Scene *scene);
	void nxtscnBRockMan(Scene *scene);
	void nxtscnLRockMan(Scene *scene);
	void nxtscnHotelMen(Scene *scene);

	void psoPause(Scene *scene);
	void psoShootout(Scene *scene);
	void psoMdShootout(Scene *scene);

	void scene_iso_skipsaloon(Scene *scene, uint32 currentFrame);
	void scene_iso_skipsaloon2(Scene *scene, uint32 currentFrame);
	void scene_iso_checksaloon(Scene *scene, uint32 currentFrame);
	void scene_iso_intostable(Scene *scene, uint32 currentFrame);
	void scene_iso_intoffice(Scene *scene, uint32 currentFrame);
	void scene_iso_intobank(Scene *scene, uint32 currentFrame);
	void scene_iso_chkbartndr(Scene *scene, uint32 currentFrame);
	void scene_iso_didhideout(Scene *scene, uint32 currentFrame);
	void scene_iso_didsignpost(Scene *scene, uint32 currentFrame);
	void scene_iso_doshootout(Scene *scene, uint32 currentFrame);
	void scene_iso_mdshootout(Scene *scene, uint32 currentFrame);
	void scene_iso_shotinto116(Scene *scene, uint32 currentFrame);

	void rectHideFront(Rect *rect, int16 x, int16 y);
	void rectHideRear(Rect *rect, int16 x, int16 y);
	void rectNewScene(Rect *rect, int16 x, int16 y);
	void rectMenuSelect(Rect *rect, int16 x, int16 y);
	void rectSkull(Rect *rect, int16 x, int16 y);
	void rectProspSign(Rect *rect, int16 x, int16 y);
	void rectMineSign(Rect *rect, int16 x, int16 y);
	void rectMineItem1(Rect *rect, int16 x, int16 y);
	void rectMineItem2(Rect *rect, int16 x, int16 y);
	void rectMineItem3(Rect *rect, int16 x, int16 y);
	void rectMineLantern(Rect *rect, int16 x, int16 y);
	void rectShotHideout(Rect *rect, int16 x, int16 y);
	void rectShotDirection(Rect *rect, int16 x, int16 y, uint8 direction);
	void rectStartBottles();
	void rectStartGame();
	void rectContinue();

	Common::String pickTown();
	Common::String pickSign();
	Common::String pickMap();
};

class DebuggerMaddog : public GUI::Debugger {
public:
	DebuggerMaddog(GameMaddog *game);
	bool cmdWarpTo(int argc, const char **argv);
private:
	GameMaddog *_game;
};

} // End of namespace Alg

#endif
