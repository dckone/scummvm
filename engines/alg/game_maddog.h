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

	typedef Common::Functor0Mem<void, GameMaddog> ScriptFunction;

	struct ScriptFunctionEntry {
		Common::String _name;
		const ScriptFunction *_function;
		ScriptFunctionEntry(Common::String name, const ScriptFunction *function) {
			_name = name;
			_function = function;
		}
	};

	void init();
	void registerScriptFunctions();
	void updateScreen();
	void beforeScene(Scene *scene);
	void duringScene(Scene *scene, uint32 currentFrame);
	void afterScene(Scene *scene);
	void rectHit(Rect *rect, int16 x, int16 y);

	Common::Array<const ScriptFunctionEntry *> _zonePtrFb;
	Common::Array<const ScriptFunctionEntry *> _rectHitFuncs;
	Common::Array<const ScriptFunctionEntry *> _scenePreOps;
	Common::Array<const ScriptFunctionEntry *> _sceneShowMsg;
	Common::Array<const ScriptFunctionEntry *> _sceneInsOps;
	Common::Array<const ScriptFunctionEntry *> _sceneWepDwn;
	Common::Array<const ScriptFunctionEntry *> _sceneScnScr;
	Common::Array<const ScriptFunctionEntry *> _sceneNxtFrm;
	Common::Array<const ScriptFunctionEntry *> _sceneNxtScn;

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

	// globals
	uint32 _fired = 0;
	uint8 _bottles = 0;
	uint32 _bot_mask = 0;
	uint32 _shootoutMinF = 0; // TODO needed?
	uint32 _shootoutMaxF = 0; // TODO needed?

	// gamestate
	int32 _been_to = 0;
	int32 _got_into = 0;
	bool _had_skull = 0;
	bool _had_lantern = 0;
	int32 _people_killed = 0;
	int32 _bad_men = 0;
	int32 _bad_men_bits = 0;
	bool _hide_out_front = 0;
	bool _difficulty = 1;
	int32 _pro_clue = 0;
	bool _got_clue = 0;
	int32 _shoot_out_cnt = 0;
	int32 _map = 0;
	int32 _map0 = 0;
	int32 _map1 = 0;
	int32 _map2 = 0;
	int32 _map_pos = 0;
	bool _max_map_pos = 0;
	bool _bartender_alive = 0;
	int32 _sheriff_cnt = 0;
	bool _in_shootout = 0;
	Common::String _cur_scene;
	Common::String _ret_scene;
	Common::String _sub_scene;

	// constants
	const uint32 _fight[] = {208, 228, 243};
	const uint32 _ambush[] = {192, 193, 192};

	// misc game functions
	void UpdateStat();
	void _default_bullethole(int x, y);
	Common::String _die();
	uint32 _pick_rand(uint32 max, uint32 *mask_ptr);
	uint32 _pick_bad(uint32 max);
	Common::String _pick_town();
	Common::String _pick_map();
	Common::String _pick_sign();
	Common::String _map_right();
	Common::String _map_left();

	// Script functions: Zone
	void _zone_bullethole();
	void _zone_skullhole();
	void _zone_globalhit();

	// Script functions: RectHit
	void _rect_newscene();
	void _rect_hidefront();
	void _rect_hiderear();
	void _rect_menuselect();
	void _rect_skull();
	void _rect_killman();
	void _rect_killwoman();
	void _rect_prospsign();
	void _rect_minesign();
	void _rect_mineitem1();
	void _rect_mineitem2();
	void _rect_mineitem3();
	void _rect_minelantern();
	void _rect_shothideout();
	void _rect_shotright();
	void _rect_shotleft();
	void _rect_shotmenu();
	void _rect_exit();
	void _rect_continue();
	void _rect_start();
	void _rect_startbottles();

	// Script functions: Scene PreOps
	void _scene_po_drawrct();
	void _scene_po_pause();
	void _scene_pso_shootout();
	void _scene_pso_mdshootout();
	void _scene_pso_fadein();
	void _scene_pso_paus_fi();
	void _scene_pso_preread();
	void _scene_pso_paus_pr();

	// Script functions: Scene Scene InsOps
	void _scene_iso_donothing();
	void _scene_iso_pause();
	void _scene_iso_startgame();
	void _scene_iso_skipsaloon();
	void _scene_iso_skipsaloon2();
	void _scene_iso_checksaloon();
	void _scene_iso_intostable();
	void _scene_iso_intoffice();
	void _scene_iso_intobank();
	void _scene_iso_chkbartndr();
	void _scene_iso_didhideout();
	void _scene_iso_didsignpost();
	void _scene_iso_doshootout();
	void _scene_iso_mdshootout();
	void _scene_iso_shootpast();
	void _scene_iso_spause();
	void _scene_iso_shotinto24();
	void _scene_iso_shotinto116();

	// Script functions: Scene Scene NxtScn
	void _scene_default_nxtscn();
	void _scene_nxtscn_pickbottle();
	void _scene_nxtscn_died();
	void _scene_nxtscn_autosel();
	void _scene_nxtscn_finsaloon();
	void _scene_nxtscn_finoffice();
	void _scene_nxtscn_finstable();
	void _scene_nxtscn_finbank();
	void _scene_nxtscn_picsaloon();
	void _scene_nxtscn_killman();
	void _scene_nxtscn_killwoman();
	void _scene_nxtscn_bank();
	void _scene_nxtscn_stable();
	void _scene_nxtscn_savprosp();
	void _scene_nxtscn_picktoss();
	void _scene_nxtscn_hittoss();
	void _scene_nxtscn_misstoss();
	void _scene_nxtscn_picksign();
	void _scene_nxtscn_brockman();
	void _scene_nxtscn_lrockman();
	void _scene_nxtscn_hotelmen();
	void _scene_nxtscn_drawgun();

	// Script functions: ShowMsg
	void _scene_sm_donothing();

	// Script functions: WepDwn
	void _scene_default_wepdwn();

	// Script functions: ScnScr
	void _scene_default_score();

	// Script functions: ScnNxtFrm
	void _scene_nxtfrm();
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
