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
#include "common/file.h"
#include "common/tokenizer.h"

#include "alg/scene.h"

namespace Alg {

SceneInfo::SceneInfo() {
}

SceneInfo::~SceneInfo() {
}

void SceneInfo::loadScnFile(const Common::Path &path) {
	_scnFile.open(path);
	assert(_scnFile.isOpen());

	while(_scnFile.pos() < _scnFile.size()) {
		Common::String line = _scnFile.readLine();
		Common::StringTokenizer tokenizer(line, " ");
		Common::String command = tokenizer.nextToken();
		if(command.equalsIgnoreCase("START")) {
			parseStart(tokenizer.nextToken());
		} else if(command.equalsIgnoreCase("SCENE")) {
			Common::String sceneName = tokenizer.nextToken();
			uint32 startFrame = atoi(tokenizer.nextToken().c_str());
			uint32 endFrame = atoi(tokenizer.nextToken().c_str());
			parseScene(sceneName, startFrame, endFrame);
		} else if(command.equalsIgnoreCase("ZONE")) {
			Common::String zoneName = tokenizer.nextToken();
			uint32 startFrame = atoi(tokenizer.nextToken().c_str());
			uint32 endFrame = atoi(tokenizer.nextToken().c_str());
			parseZone(zoneName, startFrame, endFrame);
		} else if(command.equalsIgnoreCase("END")) {
			break;
		} else {
			error("Unknown script section encountered: %s", line.c_str());
		}
	}

	_scnFile.close();

	addZonesToScenes();
}

void SceneInfo::parseStart(Common::String sceneName) {
	_startScene = sceneName;
	debug("startScene: %s", _startScene.c_str());
}

void SceneInfo::parseScene(Common::String sceneName, uint32 startFrame, uint32 endFrame) {
	Scene *scene = new Scene();
	scene->name = sceneName;
	scene->startFrame = startFrame;
	scene->endFrame = endFrame;
	while(_scnFile.pos() < _scnFile.size()) {
		Common::String line = _scnFile.readLine();
		Common::StringTokenizer tokenizer(line, " ");
		Common::String command = tokenizer.nextToken();
		if(command.equalsIgnoreCase("NEXT")) {
			scene->next = tokenizer.nextToken();
		}
		else if(command.equalsIgnoreCase("ZONES")) {
			scene->zonesStart = tokenizer.nextToken();
		}
		else if(command.equalsIgnoreCase("PREOP")) {
			scene->preop = tokenizer.nextToken();
			scene->preopParam = tokenizer.nextToken();
		}
		else if(command.equalsIgnoreCase("INSOP")) {
			scene->insop = tokenizer.nextToken();
			scene->insopParam = tokenizer.nextToken();
		}
		else if(command.equalsIgnoreCase("WEPDWN")) {
			scene->wepdwn = tokenizer.nextToken();
			assert(scene->wepdwn.equals("DEFAULT")); // Always DEFAULT, at least in MDMC
		}
		else if(command.equalsIgnoreCase("SCNSCR")) {
			scene->scnscr = tokenizer.nextToken();
			scene->scnscrParam = tokenizer.nextToken();
			assert(scene->scnscr.equals("DEFAULT")); // Always DEFAULT, at least in MDMC
		}
		else if(command.equalsIgnoreCase("NXTFRM")) {
			scene->nxtfrm = tokenizer.nextToken();
			assert(scene->nxtfrm.equals("DEFAULT")); // Always DEFAULT, at least in MDMC
		}
		else if(command.equalsIgnoreCase("NXTSCN")) {
			scene->nxtscn = tokenizer.nextToken();
		}
		else if(command.equalsIgnoreCase("DATA")) {
			scene->dataParam1 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam2 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam3 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam4 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam5 = tokenizer.nextToken();
			assert(scene->dataParam5.equals("."));
		}
		else if(command.equalsIgnoreCase("DIFF")) {
			scene->diff = tokenizer.nextToken();
			assert(scene->diff.equals("3")); // Always 3, at least in MDMC
		}
		else if(command.equals(";")) {
			break;
		}
		else {
			error("Unknown scene command found: %s", command.c_str());
		}

		Common::String nextToken = tokenizer.nextToken();
		if(!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_scenes.push_back(*scene);
}

void SceneInfo::parseZone(Common::String zoneName, uint32 startFrame, uint32 endFrame) {
	Zone *zone = new Zone();
	zone->name = zoneName;
	zone->startFrame = startFrame;
	zone->endFrame = endFrame;
	while(_scnFile.pos() < _scnFile.size()) {
		Common::String line = _scnFile.readLine();
		Common::StringTokenizer tokenizer(line, " ");
		Common::String command = tokenizer.nextToken();
		if(command.equalsIgnoreCase("PTRFB")) {
			zone->ptrfb = tokenizer.nextToken();
			assert(zone->ptrfb.equals("DEFAULT") || zone->ptrfb.equals("SKULL")); // only DEFAULT or SKULL in MDMC
		}
		else if(command.equalsIgnoreCase("RECT")) {
			Rect *rect = new Rect();
			rect->left = atoi(tokenizer.nextToken().c_str());
			rect->top = atoi(tokenizer.nextToken().c_str());
			rect->right = atoi(tokenizer.nextToken().c_str());
			rect->bottom = atoi(tokenizer.nextToken().c_str());
			rect->scene = tokenizer.nextToken();
			rect->unk1 = tokenizer.nextToken();
			rect->function = tokenizer.nextToken();
			rect->unk2 = tokenizer.nextToken();
			zone->rects.push_back(*rect);
		}
		else if(command.equalsIgnoreCase("NEXT")) {
			zone->next = tokenizer.nextToken();
		}
		else if(command.equals(";")) {
			break;
		}
		else {
			error("Unknown zone command found: %s", command.c_str());
		}

		Common::String nextToken = tokenizer.nextToken();
		if(!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_zones.push_back(*zone);
}

void SceneInfo::addZonesToScenes() {
	for(uint32 i = 0; i < _scenes.size(); i++) {
		Scene *scene = &(_scenes)[i];
		if(!scene->zonesStart.empty()) {
			Zone *zone = findZone(scene->zonesStart);
			scene->zones.push_back(*zone);
			while(!zone->next.empty()) {
				zone = findZone(zone->next);
				scene->zones.push_back(*zone);
			}
		}
	}
}

Zone * SceneInfo::findZone(Common::String zoneName) {
	for(uint32 i = 0; i < _zones.size(); i++) {
		if(_zones[i].name.equalsIgnoreCase(zoneName)) {
			return &(_zones)[i];
		}
	}
	error("Cannot find zone %s", zoneName.c_str());
}

Scene * SceneInfo::findScene(Common::String sceneName) {
	for(uint32 i = 0; i < _scenes.size(); i++) {
		if(_scenes[i].name.equalsIgnoreCase(sceneName)) {
			return &(_scenes)[i];
		}
	}
	error("Cannot find scene %s", sceneName.c_str());
}

void SceneInfo::addScene(Scene *scene) {
	_scenes.push_back(*scene);
}

void Zone::addRect(int16 left, int16 top, int16 right, int16 bottom, Common::String scene, Common::String unk1, Common::String function, Common::String unk2) {
	Rect *rect = new Rect();
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	rect->scene = scene;
	rect->unk1 = unk1;
	rect->function = function;
	rect->unk2 = unk2;
	rects.push_back(*rect);
}

} // End of namespace Alg
