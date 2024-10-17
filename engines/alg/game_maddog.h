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

#include "common/hashmap.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/str.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

namespace Alg {

typedef Common::Functor1Mem<const Scene*, void, GameMaddog> ScriptFunctionScene;
typedef Common::Functor2Mem<const Scene*, const Rect*, void, GameMaddog> ScriptFunctionSceneRect;
typedef Common::Functor1Mem<const Common::Point*, void, GameMaddog> ScriptFunctionPoint;
typedef Common::HashMap<Common::String, ScriptFunctionScene*> ScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, ScriptFunctionSceneRect*> ScriptFunctionSceneRectMap;
typedef Common::HashMap<Common::String, ScriptFunctionPoint*> ScriptFunctionPointMap;

class GameMaddog : public Game {

enum SceneFuncType {
	PREOP = 1,
	SHOWMSG = 2,
	INSOP = 3,
	WEPDWN = 4,
	SCNSCR = 5,
	NXTFRM = 6,
	NXTSCN = 7
};

public:
	GameMaddog(AlgEngine *vm);
	~GameMaddog();
	Common::Error run();
	void debug_warpTo(int val);
private:
	void init();
	void registerScriptFunctions();
	void verifyScriptFunctions();
	ScriptFunctionPoint getScriptFunctionZonePtrFb(Common::String name);
	ScriptFunctionSceneRect getScriptFunctionRectHit(Common::String name);
	ScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionZonePtrFb(Common::String name, Common::Point *point);
	void callScriptFunctionRectHit(Common::String name, Scene *scene, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);
	void updateScreen();

	ScriptFunctionPointMap _zonePtrFb;
	ScriptFunctionSceneRectMap _rectHitFuncs;
	ScriptFunctionSceneMap _scenePreOps;
	ScriptFunctionSceneMap _sceneShowMsg;
	ScriptFunctionSceneMap _sceneInsOps;
	ScriptFunctionSceneMap _sceneWepDwn;
	ScriptFunctionSceneMap _sceneScnScr;
	ScriptFunctionSceneMap _sceneNxtFrm;
	ScriptFunctionSceneMap _sceneNxtScn;

	Zone *_menuzone;
	Zone *_submenzone;

	// images
	Common::Array<Graphics::Surface> *_aniAmmo;
	Common::Array<Graphics::Surface> *_aniBullet;
	Common::Array<Graphics::Surface> *_aniClip;
	Common::Array<Graphics::Surface> *_aniHat;
	Common::Array<Graphics::Surface> *_aniNumbers;
	Common::Array<Graphics::Surface> *_aniShootout;
	Common::Array<Graphics::Surface> *_aniGun;
	Common::Array<Graphics::Surface> *_aniHole;
	Common::Array<Graphics::Surface> *_aniKnife;

	// sounds // TODO fix
    uint32 _shot;
    uint32 _emptysound;
    uint32 _savesound;
    uint32 _loadsound;
    uint32 _skullsound;
    uint32 _easysound;
    uint32 _avgsound;
    uint32 _hardsound;

	// globals
	uint32 _fired = 0;
	uint8 _bottles = 0;
	uint32 _botmask = 0;
	uint32 _gun_time = 0;
	int32 _old_score = 0;
	bool _inmenu = false;
	int32 _exit_caught = 0;
	int32 _pp_force = 0;
	int32 _pp_flgs = 0;
	long int _min_f;
	long int _max_f;
	uint8 _num_players = 1; // TODO this is hiding 2-player mode
	uint32 _shots = 0;
	uint32 _frm; // TODO: rename _currentFrame
	uint32 _game_timer = 0;
	uint32 _videoFrameSkip = 3;
	uint32 _oldx = 0;
	uint32 _oldy = 0;
	bool _shotfired = false;
	uint32 _whichgun;
	uint32 _oldwhichgun;
	uint32 _omx;
	uint32 _omy;
	uint8 _olddif;
	int32 _lives = 0;
	uint32 _NextFrameTime = 0;

	// gamestate
	int32 _been_to = 0;
	int32 _got_into = 0;
	bool _had_skull = 0;
	bool _had_lantern = 0;
	int32 _people_killed = 0;
	int32 _bad_men = 0;
	uint32 _bad_men_bits = 0;
	bool _hide_out_front = 0;
	uint8 _difficulty = 1;
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
	const uint32 _fight[3] = {208, 228, 243};
	const uint32 _ambush[3] = {192, 193, 192};
	const uint32 _hotel_scenes[5] = {77, 87, 89, 97, 105};
	const uint32 _bottle_toss[7] = {171, 174, 175, 178, 179, 182, 183};
	const uint32 _pausdifscal[4] = {0x00000100, 0x00000100, 0x00800000, 0x00400000};
	const uint32 _rectdifscal[4] = {0x00000100, 0x00000100, 0x00C00000, 0x00800000};
	const uint8 _diffpos[4][2] = {{0, 0}, {0x4D, 0x43}, {0x4E, 0x66}, {0x4F, 0x80}};

	// base functions
	uint32 _GetUsTime();
	void _Pause(unsigned long pause_time);
	void _DoMenu();
	void _NewGame();
	void _ResetParams();
	void _UpdateStat();
	void _ChangeDifficulty(int newDifficulty);
	void _ShowDifficulty(int newDifficulty, int updateCursor);
	void _RestoreCursor();
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse(int x, int y);
	void _DisplayScore();
	bool _WeaponDown();
	uint32 _GetFrame(Scene *scene);
	void _SetFrame();
	void _SaveState(Common::String filename);
	void _LoadState(Common::String filename);
	uint32 _LoadSound(Common::String filename);
	void _DoDiffSound(int difficulty);
	void _DoSaveSound();
	void _DoLoadSound();
	void _DoSkullSound();
	void _DoShot();

	// misc game functions
	void _default_bullethole(const Common::Point *point);
	void _die();
	uint32 _pick_rand(uint32 max, uint32 *mask_ptr);
	uint32 _pick_bad(uint32 max);
	Common::String _pick_town();
	Common::String _pick_map();
	Common::String _pick_sign();
	Common::String _map_right();
	Common::String _map_left();

	// Script functions: Zone
	void _zone_bullethole(const Common::Point *point);
	void _zone_skullhole(const Common::Point *point);
	void _zone_globalhit(const Common::Point *point);

	// Script functions: RectHit
	void _rect_newscene(const Scene *scene, const Rect *rect);
	void _rect_hidefront(const Scene *scene, const Rect *rect);
	void _rect_hiderear(const Scene *scene, const Rect *rect);
	void _rect_menuselect(const Scene *scene, const Rect *rect);
	void _rect_skull(const Scene *scene, const Rect *rect);
	void _rect_killman(const Scene *scene, const Rect *rect);
	void _rect_killwoman(const Scene *scene, const Rect *rect);
	void _rect_prospsign(const Scene *scene, const Rect *rect);
	void _rect_minesign(const Scene *scene, const Rect *rect);
	void _rect_mineitem1(const Scene *scene, const Rect *rect);
	void _rect_mineitem2(const Scene *scene, const Rect *rect);
	void _rect_mineitem3(const Scene *scene, const Rect *rect);
	void _rect_minelantern(const Scene *scene, const Rect *rect);
	void _rect_shothideout(const Scene *scene, const Rect *rect);
	void _rect_shotright(const Scene *scene, const Rect *rect);
	void _rect_shotleft(const Scene *scene, const Rect *rect);
	void _rect_shotmenu(const Scene *scene, const Rect *rect);
	void _rect_exit(const Scene *scene, const Rect *rect);
	void _rect_continue(const Scene *scene, const Rect *rect);
	void _rect_save(const Scene *scene, const Rect *rect);
	void _rect_load(const Scene *scene, const Rect *rect);
	void _rect_easy(const Scene *scene, const Rect *rect);
	void _rect_average(const Scene *scene, const Rect *rect);
	void _rect_hard(const Scene *scene, const Rect *rect);
	void _rect_start(const Scene *scene, const Rect *rect);
	void _rect_startbottles(const Scene *scene, const Rect *rect);

	// Script functions: Scene PreOps
	void _scene_po_drawrct(const Scene *scene);
	void _scene_po_pause(const Scene *scene);
	void _scene_pso_shootout(const Scene *scene);
	void _scene_pso_mdshootout(const Scene *scene);
	void _scene_pso_fadein(const Scene *scene);
	void _scene_pso_paus_fi(const Scene *scene);
	void _scene_pso_preread(const Scene *scene);
	void _scene_pso_paus_pr(const Scene *scene);

	// Script functions: Scene Scene InsOps
	void _scene_iso_donothing(const Scene *scene);
	void _scene_iso_pause(const Scene *scene);
	void _scene_iso_startgame(const Scene *scene);
	void _scene_iso_skipsaloon(const Scene *scene);
	void _scene_iso_skipsaloon2(const Scene *scene);
	void _scene_iso_checksaloon(const Scene *scene);
	void _scene_iso_intostable(const Scene *scene);
	void _scene_iso_intoffice(const Scene *scene);
	void _scene_iso_intobank(const Scene *scene);
	void _scene_iso_chkbartndr(const Scene *scene);
	void _scene_iso_didhideout(const Scene *scene);
	void _scene_iso_didsignpost(const Scene *scene);
	void _scene_iso_doshootout(const Scene *scene);
	void _scene_iso_mdshootout(const Scene *scene);
	void _scene_iso_shootpast(const Scene *scene);
	void _scene_iso_spause(const Scene *scene);
	void _scene_iso_shotinto24(const Scene *scene);
	void _scene_iso_shotinto116(const Scene *scene);

	// Script functions: Scene Scene NxtScn
	void _scene_default_nxtscn(const Scene *scene);
	void _scene_nxtscn_pickbottle(const Scene *scene);
	void _scene_nxtscn_died(const Scene *scene);
	void _scene_nxtscn_autosel(const Scene *scene);
	void _scene_nxtscn_finsaloon(const Scene *scene);
	void _scene_nxtscn_finoffice(const Scene *scene);
	void _scene_nxtscn_finstable(const Scene *scene);
	void _scene_nxtscn_finbank(const Scene *scene);
	void _scene_nxtscn_picsaloon(const Scene *scene);
	void _scene_nxtscn_killman(const Scene *scene);
	void _scene_nxtscn_killwoman(const Scene *scene);
	void _scene_nxtscn_bank(const Scene *scene);
	void _scene_nxtscn_stable(const Scene *scene);
	void _scene_nxtscn_savprosp(const Scene *scene);
	void _scene_nxtscn_picktoss(const Scene *scene);
	void _scene_nxtscn_hittoss(const Scene *scene);
	void _scene_nxtscn_misstoss(const Scene *scene);
	void _scene_nxtscn_picksign(const Scene *scene);
	void _scene_nxtscn_brockman(const Scene *scene);
	void _scene_nxtscn_lrockman(const Scene *scene);
	void _scene_nxtscn_hotelmen(const Scene *scene);
	void _scene_nxtscn_drawgun(const Scene *scene);

	// Script functions: ShowMsg
	void _scene_sm_donothing(const Scene *scene);

	// Script functions: WepDwn
	void _scene_default_wepdwn(const Scene *scene);

	// Script functions: ScnScr
	void _scene_default_score(const Scene *scene);

	// Script functions: ScnNxtFrm
	void _scene_nxtfrm(const Scene *scene);
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
