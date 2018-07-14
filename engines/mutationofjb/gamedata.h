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

#ifndef MUTATIONOFJB_GAMEDATA_H
#define MUTATIONOFJB_GAMEDATA_H

#include "common/scummsys.h"
#include "mutationofjb/inventory.h"

namespace Common {
class ReadStream;
}

namespace MutationOfJB {

enum {
	MAX_ENTITY_NAME_LENGTH = 0x14
};

/*
	There are 4 types of entities present in the game data:
	- Door
	- Object
	- Static
	- Bitmap
*/

struct Door {
	/*
		Door name.
		Can be empty - deactivates door completely.
	*/
	char _name[MAX_ENTITY_NAME_LENGTH + 1];
	/*
		Scene ID where the door leads.
		Can be 0 - you can hover your mouse over it, but clicking it doesn't do anything (unless scripted).
	*/
	uint8  _destSceneId;
	/* X coordinate for player's position after going through the door. */
	uint16 _destX;
	/* Y coordinate for player's position after going through the door. */
	uint16 _destY;
	/* X coordinate of the door rectangle. */
	uint16 _x;
	/* Y coordinate of the door rectangle. */
	uint8  _y;
	/* Width of the door rectangle. */
	uint16 _width;
	/* Height of the door rectangle. */
	uint8  _height;
	/* X coordinate for position towards player will walk after clicking the door. */
	uint16 _walkToX;
	/* Y coordinate for position towards player will walk after clicking the door. */
	uint8  _walkToY;
	/* Unknown for now - likely not even used. */
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Object {
	uint8  _AC;
	uint8  _FA;
	uint8  _FR;
	uint8  _NA;
	uint8  _FS;
	uint8  _unknown;
	uint8  _CA;
	uint16 _x;
	uint8  _y;
	uint16 _XL;
	uint8  _YL;
	uint16 _WX;
	uint8  _WY;
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Static {
	uint8  _active;
	char _name[MAX_ENTITY_NAME_LENGTH + 1];
	uint16 _x;
	uint8  _y;
	uint16 _width;
	uint8  _height;
	uint16 _walkToX;
	uint8  _walkToY;
	uint8  _SP;

	bool loadFromStream(Common::ReadStream &stream);
};

struct Bitmap {
	uint8  _frame;
	uint8  _isVisible;
	uint16 _x1;
	uint8  _y1;
	uint16 _x2;
	uint8  _y2;

	bool loadFromStream(Common::ReadStream &stream);
};

struct ExhaustedChoice {
	/*
		1 bit - context
		3 bits - choice index
		4 bits - choice list index
	*/
	uint8 _encodedData;

	uint8 getContext() const { return (_encodedData >> 7) & 0x1; }
	uint8 getChoiceIndex() const { return (_encodedData >> 4) & 0x7; }
	uint8 getChoiceListIndex() const { return _encodedData & 0xF; }

	ExhaustedChoice() : _encodedData(0) {}
	ExhaustedChoice(uint8 context, uint8 choiceIndex, uint8 choiceListIndex) :
		_encodedData(((context & 0x1) << 7) | ((choiceIndex & 0x7) << 4) | (choiceListIndex & 0xF)) {}
};

struct Scene {
	Door *getDoor(uint8 objectId);
	Object *getObject(uint8 objectId, bool ignoreNo = false);
	Static *getStatic(uint8 staticId, bool ignoreNo = false);

	uint8 getNoDoors(bool ignoreNo = false) const;
	uint8 getNoObjects(bool ignoreNo = false) const;
	uint8 getNoStatics(bool ignoreNo = false) const;

	Door *findDoor(int16 x, int16 y, int *index = nullptr);
	Static *findStatic(int16 x, int16 y, int *index = nullptr);
	Bitmap *findBitmap(int16 x, int16 y, int *index = nullptr);

	void addExhaustedChoice(uint8 context, uint8 choiceIndex, uint8 choiceIndexList);
	bool isChoiceExhausted(uint8 context, uint8 choiceIndex, uint8 choiceIndexList) const;

	uint8 _startup;
	uint8 _unknown001;
	uint8 _unknown002;
	uint8 _unknown003;
	uint8 _DL;

	uint8 _noDoors;
	Door _doors[5];

	uint8 _noObjects;
	Object _objects[9];

	uint8 _noStatics;
	Static _statics[15];

	Bitmap _bitmaps[10];

	uint16 _obstacleY1;
	uint8 _palRotStart;
	uint8 _palRotEnd;
	uint8 _palRotPeriod;

	/* Points to the first free item in exhausted choices list. */
	uint8 _exhaustedChoiceNext;
	ExhaustedChoice _exhaustedChoices[79];

	bool loadFromStream(Common::ReadStream &stream);
};

struct ConversationInfo {
	struct Item {
		uint8 _choice;
		uint8 _response;
		uint8 _nextLineIndex;
	};

	typedef Common::Array<Item> Items;
	struct Line {
		Items _items;
	};

	Common::Array<Line> _lines;
	uint8 _context;
	uint8 _objectId;
	uint8 _color;
};

struct GameData {
public:
	GameData();
	Scene *getScene(uint8 sceneId);
	Scene *getCurrentScene();
	Inventory &getInventory();

	bool loadFromStream(Common::ReadStream &stream);

	uint8 _currentScene;
	uint8 _lastScene;
	bool _partB;
	Inventory _inventory;
	Common::String _currentAPK;
	ConversationInfo _conversationInfo;
private:
	Scene _scenes[45];
};

enum Colors {
	WHITE = 0xC6,
	DARKGRAY = 0xC2,
	LIGHTGRAY = 0xC4,
	GREEN = 0xC8,
	ORANGE = 0xCA,
	DARKBLUE = 0xD6,
	LIGHTBLUE = 0xDA,
	BROWN = 0xDC
};

}

#endif
