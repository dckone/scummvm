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
#include "common/file.h"
#include "common/tokenizer.h"

#include "alg/scene.h"

namespace Alg {

SceneInfo::SceneInfo() {
}

SceneInfo::~SceneInfo() {
}

void SceneInfo::loadScnFile(const Common::Path &path) {
	debug("loading scene script: %s", path.toString().c_str());
	_scnFile.open(path);
	assert(_scnFile.isOpen());
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (line.size() == 0) {
			continue;
		}
		if (line.size() > 0 && line[0] == '*') {
			continue;
		}
		if (line.size() > 1 && line[0] == '/' && line[1] == '/') {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_mainTokens, tokenizer.nextToken());
		uint32 startFrame, endFrame = 0;
		Common::String sceneName, zoneName = nullptr;
		switch (token) {
		case 0: // ;
			break;
		case 1: // ZONE
			zoneName = tokenizer.nextToken();
			startFrame = atoi(tokenizer.nextToken().c_str());
			endFrame = atoi(tokenizer.nextToken().c_str());
			parseZone(zoneName, startFrame, endFrame);
			break;
		case 2: // SCENE
			sceneName = tokenizer.nextToken();
			startFrame = atoi(tokenizer.nextToken().c_str());
			endFrame = atoi(tokenizer.nextToken().c_str());
			parseScene(sceneName, startFrame, endFrame);
			break;
		case 3: // MSG
			error("MSG Not implemented: %s", line.c_str());
			break;
		case 4: // START
			_startscene = tokenizer.nextToken();
			break;
		case 5: // GLOBAL
			error("GLOBAL Not implemented: %s", line.c_str());
			break;
		case 6: // END
			done = true;
			break;
		default:
			error("Unknown script section encountered: %s", line.c_str());
			break;
		}
	}
	_scnFile.close();
	addZonesToScenes();
}

void SceneInfo::parseScene(Common::String sceneName, uint32 startFrame, uint32 endFrame) {
	Scene *scene = new Scene();
	scene->name = sceneName;
	scene->startFrame = startFrame;
	scene->endFrame = endFrame;
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (line.size() > 1 && line[0] == '/' && line[1] == '/') {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_sceneTokens, tokenizer.nextToken());
		switch (token) {
		case 0: // EOF
		case 1: // NEXT
			scene->next = tokenizer.nextToken();
			break;
		case 2: // ZONES
			scene->zonesStart = tokenizer.nextToken();
			break;
		case 3: // PREOP
			scene->preop = tokenizer.nextToken();
			scene->preopParam = tokenizer.nextToken();
			break;
		case 4: // SHOWMSG
				// TODO: Add SHOWMSG
			break;
		case 5: // INSOP
			scene->insop = tokenizer.nextToken();
			scene->insopParam = tokenizer.nextToken();
			break;
		case 6: // WEPDWN
			scene->wepdwn = tokenizer.nextToken();
			break;
		case 7: // SCNSCR
			scene->scnscr = tokenizer.nextToken();
			scene->scnscrParam = atoi(tokenizer.nextToken().c_str());
			break;
		case 8: // NXTFRM
			scene->nxtfrm = tokenizer.nextToken();
			break;
		case 9: // NXTSCN
			scene->nxtscn = tokenizer.nextToken();
			break;
		case 10: // DATA
			scene->dataParam1 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam2 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam3 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam4 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam5 = atoi(tokenizer.nextToken().c_str());
			scene->dataParam6 = atoi(tokenizer.nextToken().c_str());
			break;
		case 11: // DIFF
			scene->diff = atoi(tokenizer.nextToken().c_str());
			break;
		case 12: // NXET
			// just a typo of NEXT, original game seems to ignore it
			tokenizer.nextToken();
			break;
		case 13: // ;
			done = true;
			break;
		default:
			error("Unknown scene token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_scenes.push_back(scene);
}

void SceneInfo::parseZone(Common::String zoneName, uint32 startFrame, uint32 endFrame) {
	Zone *zone = new Zone();
	zone->name = zoneName;
	zone->startFrame = startFrame;
	zone->endFrame = endFrame;
	bool done = false;
	while (_scnFile.pos() < _scnFile.size() && !done) {
		Common::String line = _scnFile.readLine();
		line.trim();
		if (line.size() > 1 && line[0] == '/' && line[1] == '/') {
			continue;
		}
		Common::StringTokenizer tokenizer(line, " ");
		int8 token = getToken(_zoneTokens, tokenizer.nextToken());
		Rect *rect = nullptr;
		switch (token) {
		case 0: // EOF
			break;
		case 1: // NEXT
			zone->next = tokenizer.nextToken();
			break;
		case 2: // PTRFB
			zone->ptrfb = tokenizer.nextToken();
			break;
		case 3: // RECT
			rect = new Rect();
			rect->left = atoi(tokenizer.nextToken().c_str());
			rect->top = atoi(tokenizer.nextToken().c_str());
			rect->right = atoi(tokenizer.nextToken().c_str());
			rect->bottom = atoi(tokenizer.nextToken().c_str());
			rect->scene = tokenizer.nextToken();
			rect->score = atoi(tokenizer.nextToken().c_str());
			rect->rectHit = tokenizer.nextToken();
			rect->unknown = tokenizer.nextToken();
			zone->rects.push_back(*rect);
			break;
		case 4: // ;
			done = true;
			break;
		default:
			error("Unknown zone token found: %s", line.c_str());
			break;
		}
		Common::String nextToken = tokenizer.nextToken();
		if (!nextToken.empty()) {
			error("missed token %s in line %s", nextToken.c_str(), line.c_str());
		}
	}
	_zones.push_back(zone);
}

void SceneInfo::addZonesToScenes() {
	for (uint32 i = 0; i < _scenes.size(); i++) {
		Scene *scene = _scenes[i];
		if (!scene->zonesStart.empty()) {
			Zone *zone = findZone(scene->zonesStart);
			scene->zones.push_back(zone);
			while (!zone->next.empty()) {
				zone = findZone(zone->next);
				scene->zones.push_back(zone);
			}
		}
	}
}

Zone *SceneInfo::findZone(Common::String zoneName) {
	for (uint32 i = 0; i < _zones.size(); i++) {
		if (_zones[i]->name.equalsIgnoreCase(zoneName)) {
			return _zones[i];
		}
	}
	error("Cannot find zone %s", zoneName.c_str());
}

Scene *SceneInfo::findScene(Common::String sceneName) {
	for (uint32 i = 0; i < _scenes.size(); i++) {
		if (_scenes[i]->name.equalsIgnoreCase(sceneName)) {
			return _scenes[i];
		}
	}
	error("Cannot find scene %s", sceneName.c_str());
}

void SceneInfo::addScene(Scene *scene) {
	_scenes.push_back(scene);
}

void Zone::addRect(int16 left, int16 top, int16 right, int16 bottom, Common::String scene, uint32 score, Common::String rectHit, Common::String unknown) {
	Rect *rect = new Rect();
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	rect->scene = scene;
	rect->score = score;
	rect->rectHit = rectHit;
	rect->unknown = unknown;
	rects.push_back(*rect);
}

int8 SceneInfo::getToken(const struct TokenEntry *tokenList, Common::String token) {
	for (int i = 0; tokenList[i].name != nullptr; i++) {
		if (token == tokenList[i].name) {
			return tokenList[i].value;
		}
	}
	return -1;
}

} // End of namespace Alg
