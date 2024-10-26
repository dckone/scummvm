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

#ifndef ALG_GAME_MADDOG_H
#define ALG_GAME_MADDOG_H

#include "common/hashmap.h"
#include "common/rect.h"

#include "gui/debugger.h"

#include "alg/game.h"
#include "alg/scene.h"

#define MADDOG_VIDEO_POS_X 56
#define MADDOG_VIDEO_POS_Y 8

namespace Alg {

typedef Common::Functor1Mem<Scene *, void, GameMaddog> ScriptFunctionScene;
typedef Common::Functor1Mem<Rect *, void, GameMaddog> ScriptFunctionRect;
typedef Common::Functor1Mem<Common::Point *, void, GameMaddog> ScriptFunctionPoint;
typedef Common::HashMap<Common::String, ScriptFunctionScene *> ScriptFunctionSceneMap;
typedef Common::HashMap<Common::String, ScriptFunctionRect *> ScriptFunctionRectMap;
typedef Common::HashMap<Common::String, ScriptFunctionPoint *> ScriptFunctionPointMap;

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
	ScriptFunctionRect getScriptFunctionRectHit(Common::String name);
	ScriptFunctionScene getScriptFunctionScene(SceneFuncType type, Common::String name);
	void callScriptFunctionZonePtrFb(Common::String name, Common::Point *point);
	void callScriptFunctionRectHit(Common::String name, Rect *rect);
	void callScriptFunctionScene(SceneFuncType type, Common::String name, Scene *scene);

	ScriptFunctionPointMap _zonePtrFb;
	ScriptFunctionRectMap _rectHitFuncs;
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
	Common::Array<Graphics::Surface> *_gun;
	Common::Array<Graphics::Surface> *_numbers;
	Graphics::Surface _shoticon;
	Graphics::Surface _emptyicon;
	Graphics::Surface _liveicon;
	Graphics::Surface _deadicon;
	Graphics::Surface _reloadicon;
	Graphics::Surface _drawicon;
	Graphics::Surface _knife;
	Graphics::Surface _bullethole;

	// sounds
	Audio::SeekableAudioStream *_shotsound;
	Audio::SeekableAudioStream *_emptysound;
	Audio::SeekableAudioStream *_savesound;
	Audio::SeekableAudioStream *_loadsound;
	Audio::SeekableAudioStream *_skullsound;
	Audio::SeekableAudioStream *_easysound;
	Audio::SeekableAudioStream *_avgsound;
	Audio::SeekableAudioStream *_hardsound;

	// constants
	const uint32 _fight[3] = {208, 228, 243};
	const uint32 _ambush[3] = {192, 193, 192};
	const uint32 _hotel_scenes[10] = {77, 77, 87, 87, 89, 89, 97, 97, 105, 105};
	const uint32 _bottle_toss[14] = {171, 171, 174, 174, 175, 175, 178, 178, 179, 179, 182, 182, 183, 183};
	const uint32 _pausdifscal[4] = {0x00000100, 0x00000100, 0x00800000, 0x00400000};
	const uint32 _rectdifscal[4] = {0x00000100, 0x00000100, 0x00C00000, 0x00800000};
	const uint8 _diffpos[4][2] = {{0, 0}, {0x4D, 0x43}, {0x4E, 0x66}, {0x4F, 0x80}};
	const uint8 _livepos[3][2] = {{0x03, 0x5E}, {0x03, 0x76}, {0x03, 0x8E}};
	const uint8 _shotpos[12][2] = {{0x3, 0x5}, {0x0D, 0x5}, {0x17, 0x5}, {0x21, 0x5}, {0x3, 0x21}, {0x0D, 0x21}, {0x17, 0x21}, {0x21, 0x21}, {0x3, 0x3D}, {0x0D, 0x3D}, {0x17, 0x3D}, {0x21, 0x3D}};

	// gamestate
	uint8 _bad_men = 0;
	uint8 _bad_men_bits = 0;
	bool _bartender_alive = 0;
	uint16 _been_to = 0;
	uint8 _bottles = 0;
	uint8 _botmask = 0;
	uint8 _difficulty = 1;
	uint8 _emptycnt = 0;
	bool _fired = 0;
	bool _got_clue = false;
	uint16 _got_into = 0;
	uint8 _gun_time = 0;
	bool _had_skull = 0;
	bool _had_lantern = 0;
	bool _hide_out_front = 0;
	bool _holster = false;
	uint8 _inholster = 0;
	bool _in_shootout = 0;
	int8 _lives = 0;
	int8 _map0 = 0;
	int8 _map1 = 0;
	int8 _map2 = 0;
	uint8 _map_pos = 0;
	uint8 _max_map_pos = 0;
	long int _min_f;
	long int _max_f;
	uint8 _num_players = 1; // this is hiding 2-player mode
	uint8 _olddif;
	int8 _oldlives = 0;
	int32 _oldscore = -1;
	uint8 _oldshots = 0;
	uint8 _oldwhichgun;
	uint8 _people_killed = 0;
	uint8 _player = 1; // this is hiding 2-player mode
	uint8 _pro_clue = 0;
	int32 _score = 0;
	bool _shotfired = false;
	uint8 _shots = 0;
	uint8 _sheriff_cnt = 0; // unused
	uint8 _shoot_out_cnt = 0;
	uint8 _whichgun = 0;

	// base functions
	uint32 _GetMsTime();
	void _timer();
	void _Pause(unsigned long pause_time);
	bool __Fired(Common::Point *point);
	void _DoMenu();
	Rect *_CheckZone(Zone *zone, Common::Point *point);
	Zone *_CheckZones(Scene *scene, Common::Point *point);
	void _NewGame();
	void _ResetParams();
	void _UpdateStat();
	void _ChangeDifficulty(int newDifficulty);
	void _ShowDifficulty(int newDifficulty, int updateCursor);
	void _RestoreCursor();
	void _DoCursor();
	void _UpdateMouse();
	void _MoveMouse();
	void _DisplayScore();
	bool _WeaponDown();
	uint32 _GetFrame(Scene *scene);
	void _SetFrame();
	void _SaveState();
	void _LoadState();
	void _DoDiffSound(int difficulty);
	void _DoSaveSound();
	void _DoLoadSound();
	void _DoSkullSound();
	void _default_empty_sound();
	void _DoShot();

	// misc game functions
	void _default_bullethole(Common::Point *point);
	void _die();
	uint8 _pick_rand(uint8 max, uint8 *mask_ptr);
	uint8 _pick_bad(uint8 max);
	Common::String _pick_town();
	Common::String _pick_map();
	Common::String _pick_sign();
	Common::String _map_right();
	Common::String _map_left();

	// Script functions: Zone
	void _zone_bullethole(Common::Point *point);
	void _zone_skullhole(Common::Point *point);
	void _zone_globalhit(Common::Point *point);

	// Script functions: RectHit
	void _rect_newscene(Rect *rect);
	void _rect_hidefront(Rect *rect);
	void _rect_hiderear(Rect *rect);
	void _rect_menuselect(Rect *rect);
	void _rect_skull(Rect *rect);
	void _rect_killman(Rect *rect);
	void _rect_killwoman(Rect *rect);
	void _rect_prospsign(Rect *rect);
	void _rect_minesign(Rect *rect);
	void _rect_mineitem1(Rect *rect);
	void _rect_mineitem2(Rect *rect);
	void _rect_mineitem3(Rect *rect);
	void _rect_minelantern(Rect *rect);
	void _rect_shothideout(Rect *rect);
	void _rect_shotright(Rect *rect);
	void _rect_shotleft(Rect *rect);
	void _rect_shotmenu(Rect *rect);
	void _rect_exit(Rect *rect);
	void _rect_continue(Rect *rect);
	void _rect_save(Rect *rect);
	void _rect_load(Rect *rect);
	void _rect_easy(Rect *rect);
	void _rect_average(Rect *rect);
	void _rect_hard(Rect *rect);
	void _rect_start(Rect *rect);
	void _rect_startbottles(Rect *rect);

	// Script functions: Scene PreOps
	void _scene_po_drawrct(Scene *scene);
	void _scene_po_pause(Scene *scene);
	void _scene_pso_shootout(Scene *scene);
	void _scene_pso_mdshootout(Scene *scene);
	void _scene_pso_fadein(Scene *scene);
	void _scene_pso_paus_fi(Scene *scene);
	void _scene_pso_preread(Scene *scene);
	void _scene_pso_paus_pr(Scene *scene);

	// Script functions: Scene Scene InsOps
	void _scene_iso_donothing(Scene *scene);
	void _scene_iso_pause(Scene *scene);
	void _scene_iso_startgame(Scene *scene);
	void _scene_iso_skipsaloon(Scene *scene);
	void _scene_iso_skipsaloon2(Scene *scene);
	void _scene_iso_checksaloon(Scene *scene);
	void _scene_iso_intostable(Scene *scene);
	void _scene_iso_intoffice(Scene *scene);
	void _scene_iso_intobank(Scene *scene);
	void _scene_iso_chkbartndr(Scene *scene);
	void _scene_iso_didhideout(Scene *scene);
	void _scene_iso_didsignpost(Scene *scene);
	void _scene_iso_doshootout(Scene *scene);
	void _scene_iso_mdshootout(Scene *scene);
	void _scene_iso_shootpast(Scene *scene);
	void _scene_iso_spause(Scene *scene);
	void _scene_iso_shotinto24(Scene *scene);
	void _scene_iso_shotinto116(Scene *scene);

	// Script functions: Scene Scene NxtScn
	void _scene_default_nxtscn(Scene *scene);
	void _scene_nxtscn_pickbottle(Scene *scene);
	void _scene_nxtscn_died(Scene *scene);
	void _scene_nxtscn_autosel(Scene *scene);
	void _scene_nxtscn_finsaloon(Scene *scene);
	void _scene_nxtscn_finoffice(Scene *scene);
	void _scene_nxtscn_finstable(Scene *scene);
	void _scene_nxtscn_finbank(Scene *scene);
	void _scene_nxtscn_picsaloon(Scene *scene);
	void _scene_nxtscn_killman(Scene *scene);
	void _scene_nxtscn_killwoman(Scene *scene);
	void _scene_nxtscn_bank(Scene *scene);
	void _scene_nxtscn_stable(Scene *scene);
	void _scene_nxtscn_savprosp(Scene *scene);
	void _scene_nxtscn_picktoss(Scene *scene);
	void _scene_nxtscn_hittoss(Scene *scene);
	void _scene_nxtscn_misstoss(Scene *scene);
	void _scene_nxtscn_picksign(Scene *scene);
	void _scene_nxtscn_brockman(Scene *scene);
	void _scene_nxtscn_lrockman(Scene *scene);
	void _scene_nxtscn_hotelmen(Scene *scene);
	void _scene_nxtscn_drawgun(Scene *scene);

	// Script functions: ShowMsg
	void _scene_sm_donothing(Scene *scene);

	// Script functions: WepDwn
	void _scene_default_wepdwn(Scene *scene);

	// Script functions: ScnScr
	void _scene_default_score(Scene *scene);

	// Script functions: ScnNxtFrm
	void _scene_nxtfrm(Scene *scene);
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
