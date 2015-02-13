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

#include "common/scummsys.h"
#include "xeen/dialogs_char_info.h"
#include "xeen/dialogs_party.h"
#include "xeen/dialogs_query.h"
#include "xeen/character.h"
#include "xeen/events.h"
#include "xeen/party.h"
#include "xeen/xeen.h"

namespace Xeen {

PartyDialog::PartyDialog(XeenEngine *vm) : ButtonContainer(), 
		PartyDrawer(vm), _vm(vm) {
	initDrawStructs();
}

void PartyDialog::show(XeenEngine *vm) {
	PartyDialog *dlg = new PartyDialog(vm);
	dlg->execute();
	delete dlg;
}

void PartyDialog::execute() {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;
	bool modeFlag = false;
	int startingChar = 0;

	loadButtons();
	setupBackground();

	while (!_vm->shouldQuit()) {
		_vm->_mode = MODE_1;

		// Build up a list of available characters in the Roster that are on the
		// same side of Xeen as the player is currently on
		_charList.clear();
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			Character &player = party._roster[i];
			if (player._name.empty() || player._xeenSide != (map._loadDarkSide ? 1 : 0))
				continue;

			_charList.push_back(i);
		}

		Window &w = screen._windows[11];
		w.open();
		setupFaces(startingChar, false);
		w.writeString(Common::String::format(PARTY_DIALOG_TEXT, _partyDetails.c_str()));
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));
		screen.loadPalette("mm4.pal");

		if (modeFlag) {
			screen._windows[0].update();
			events.setCursor(0);
			screen.fadeIn(4);
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut(4);
				screen._windows[0].update();
			}

			doScroll(_vm, false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn(4);
			}
		}

		bool breakFlag = false;
		while (!_vm->shouldQuit() && !breakFlag) {
			do {
				events.pollEventsAndWait();
				checkEvents(_vm);
			} while (!_vm->shouldQuit() && !_buttonValue);

			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_e:
			case Common::KEYCODE_x:
				if (party._activeParty.size() == 0) {
					ErrorScroll::show(_vm, NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (int idx = 4; idx >= 0; --idx) {
							events.updateGameCounter();
							screen.frameWindow(idx);
							w.update();

							while (events.timeElapsed() < 1)
								events.pollEventsAndWait();
						}
					}

					w.close();
					party._mazeId = party._priorMazeId;

					party.copyPartyToRoster();
					_vm->_saves->writeCharFile();
					return;
				}
				break;

			case Common::KEYCODE_F1:
			case Common::KEYCODE_F2:
			case Common::KEYCODE_F3:
			case Common::KEYCODE_F4:
			case Common::KEYCODE_F5:
			case Common::KEYCODE_F6:
				// Show character info
				_buttonValue -= Common::KEYCODE_F1;
				if (_buttonValue < (int)party._activeParty.size())
					CharacterInfo::show(_vm, _buttonValue);
				break;

			case Common::KEYCODE_1:
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
			case Common::KEYCODE_4:
				_buttonValue -= Common::KEYCODE_1 - 7;
				if ((_buttonValue - 7 + startingChar) < (int)_charList.size()) {
					// Check if the selected character is already in the party
					uint idx = 0;
					for (; idx < party._activeParty.size(); ++idx) {
						if (_charList[_buttonValue - 7 + startingChar] ==
							party._activeParty[idx]._rosterId)
							break;
					}

					// Only add the character if they're not already in the party
					if (idx == party._activeParty.size()) {
						if (party._activeParty.size() == MAX_ACTIVE_PARTY) {
							sound.playFX(21);
							ErrorScroll::show(_vm, YOUR_PARTY_IS_FULL);
						} else {
							// Add the character to the active party
							party._activeParty.push_back(party._roster[
								_charList[_buttonValue - 7 + startingChar]]);
								startingCharChanged(startingChar);
						}
					}
				}
				break;

			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				// Up arrow
				if (startingChar > 0) {
					startingChar -= 4;
					startingCharChanged(startingChar);
				}
				break;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				// Down arrow
				if (startingChar < ((int)_charList.size() - 4)) {
					startingChar += 4;
					startingCharChanged(startingChar);
				}
				break;

			case Common::KEYCODE_c:
				// Create
				if (_charList.size() == XEEN_TOTAL_CHARACTERS) {
					ErrorScroll::show(_vm, YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut(4);
					w.close();

					createChar();

					party.copyPartyToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut(4);
					modeFlag = true;
					breakFlag = true;
				}
				break;

			case Common::KEYCODE_d:
				// Delete character
				if (_charList.size() > 0) {
					int charButtonValue = selectCharacter(true, startingChar);
					if (charButtonValue != 0) {
						int charIndex = charButtonValue - Common::KEYCODE_1 + startingChar;
						Character &c = party._roster[_charList[charIndex]];
						if (c.hasSpecialItem()) {
							ErrorScroll::show(_vm, HAS_SLAYER_SWORD);
						} else {
							Common::String msg = Common::String::format(SURE_TO_DELETE_CHAR,
								c._name.c_str(), CLASS_NAMES[c._class]);
							if (Confirm::show(_vm, msg)) {
								// If the character is in the party, remove it
								for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
									if (party._activeParty[idx]._rosterId == c._rosterId) {
										party._activeParty.remove_at(idx);
										break;
									}
								}

								// Empty the character in the roster
								c.clear();

								// Rebuild the character list
								_charList.clear();
								for (int idx = 0; idx < XEEN_TOTAL_CHARACTERS; ++idx) {
									Character &c = party._roster[idx];
									if (!c._name.empty() && c._savedMazeId == party._priorMazeId) {
										_charList.push_back(idx);
									}
								}

								startingCharChanged(startingChar);
							}
						}
					}
				}
				break;

			case Common::KEYCODE_r:
				// Remove character
				if (party._activeParty.size() > 0) {
					int charButtonValue = selectCharacter(false, startingChar);
					if (charButtonValue != 0) {
						party.copyPartyToRoster();
						party._activeParty.remove_at(charButtonValue - Common::KEYCODE_F1);
					}
					startingCharChanged(startingChar);
				}
				break;
		
			default:
				break;
			}
		}
	}
}

void PartyDialog::loadButtons() {
	_uiSprites.load("inn.icn");
	addButton(Common::Rect(16, 100, 40, 120), Common::KEYCODE_UP, &_uiSprites);
	addButton(Common::Rect(52, 100, 76, 120), Common::KEYCODE_DOWN, &_uiSprites);
	addButton(Common::Rect(87, 100, 111, 120), Common::KEYCODE_d, &_uiSprites);
	addButton(Common::Rect(122, 100, 146, 120), Common::KEYCODE_r, &_uiSprites);
	addButton(Common::Rect(157, 100, 181, 120), Common::KEYCODE_c, &_uiSprites);
	addButton(Common::Rect(192, 100, 216, 120), Common::KEYCODE_x, &_uiSprites);
	addButton(Common::Rect(0, 0, 0, 0), Common::KEYCODE_ESCAPE, &_uiSprites, false);
}

void PartyDialog::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);
}

void PartyDialog::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	_vm->_interface->assembleBorder();
}

/**
 * Sets up the faces from the avaialble roster for display in the party dialog
 */
void PartyDialog::setupFaces(int firstDisplayChar, bool updateFlag) {
	Party &party = *_vm->_party;
	Common::String charNames[4];
	Common::String charRaces[4];
	Common::String charSex[4];
	Common::String charClasses[4];
	int posIndex;
	int charId;

	// Reset the button areas for the display character images
	while (_buttons.size() > 7)
		_buttons.remove_at(7);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1, &_uiSprites, false);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2, &_uiSprites, false);
	addButton(Common::Rect(59, 59, 91, 91), Common::KEYCODE_3, &_uiSprites, false);
	addButton(Common::Rect(117, 59, 151, 91), Common::KEYCODE_4, &_uiSprites, false);


	for (posIndex = 0; posIndex < 4; ++posIndex) {
		charId = (firstDisplayChar + posIndex) >= (int)_charList.size() ? -1 :
			_charList[firstDisplayChar + posIndex];
		bool isInParty = party.isInParty(charId);

		if (charId == -1) {
			while ((int)_buttons.size() >(7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		Character &ps = party._roster[_charList[firstDisplayChar + posIndex]];
		charNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		charRaces[posIndex] = RACE_NAMES[ps._race];
		charSex[posIndex] = SEX_NAMES[ps._sex];
		charClasses[posIndex] = CLASS_NAMES[ps._class];
	}

	drawParty(updateFlag);

	// Set up the sprite set to use for each face
	for (int posIndex = 0; posIndex < 4; ++posIndex) {
		if ((firstDisplayChar + posIndex) >= (int)_charList.size())
			_faceDrawStructs[posIndex]._sprites = nullptr;
		else
			_faceDrawStructs[posIndex]._sprites = party._roster[
				_charList[firstDisplayChar + posIndex]]._faceSprites;
	}

	_partyDetails = Common::String::format(PARTY_DETAILS,
		charNames[0].c_str(), charRaces[0].c_str(), charSex[0].c_str(), charClasses[0].c_str(),
		charNames[1].c_str(), charRaces[1].c_str(), charSex[1].c_str(), charClasses[1].c_str(),
		charNames[2].c_str(), charRaces[2].c_str(), charSex[2].c_str(), charClasses[2].c_str(),
		charNames[3].c_str(), charRaces[3].c_str(), charSex[3].c_str(), charClasses[3].c_str()
		);
}

void PartyDialog::startingCharChanged(int firstDisplayChar) {
	Window &w = _vm->_screen->_windows[11];

	setupFaces(firstDisplayChar, true);
	w.writeString(Common::String::format(PARTY_DIALOG_TEXT, _partyDetails.c_str()));
	w.drawList(_faceDrawStructs, 4);

	_uiSprites.draw(w, 0, Common::Point(16, 100));
	_uiSprites.draw(w, 2, Common::Point(52, 100));
	_uiSprites.draw(w, 4, Common::Point(87, 100));
	_uiSprites.draw(w, 6, Common::Point(122, 100));
	_uiSprites.draw(w, 8, Common::Point(157, 100));
	_uiSprites.draw(w, 10, Common::Point(192, 100));

	w.update();
}

void PartyDialog::createChar() {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SpriteResource dice, icons;
	Mode oldMode = _vm->_mode;
	Common::Array<int> freeCharList;
	int selectedClass = 0;
	bool hasFadedIn = false;

	dice.load("dice.vga");
	icons.load("create.raw");

	// Add buttons
	saveButtons();
	addButton(Common::Rect(132, 98, 156, 118), Common::KEYCODE_r, &icons);
	addButton(Common::Rect(132, 128, 156, 148), Common::KEYCODE_c, &icons);
	addButton(Common::Rect(132, 158, 156, 178), Common::KEYCODE_ESCAPE, &icons);
	addButton(Common::Rect(86, 98, 110, 118), Common::KEYCODE_UP, &icons);
	addButton(Common::Rect(86, 120, 110, 140), Common::KEYCODE_DOWN, &icons);
	addButton(Common::Rect(168, 19, 192, 39), Common::KEYCODE_n, nullptr);
	addButton(Common::Rect(168, 43, 192, 63), Common::KEYCODE_i, nullptr);
	addButton(Common::Rect(168, 67, 192, 87), Common::KEYCODE_p, nullptr);
	addButton(Common::Rect(168, 91, 192, 111), Common::KEYCODE_e, nullptr);
	addButton(Common::Rect(168, 115, 192, 135), Common::KEYCODE_s, nullptr);
	addButton(Common::Rect(168, 139, 192, 159), Common::KEYCODE_a, nullptr);
	addButton(Common::Rect(168, 163, 192, 183), Common::KEYCODE_l, nullptr);
	addButton(Common::Rect(227, 19, 139, 29), 1000, nullptr);
	addButton(Common::Rect(227, 30, 139, 40), 1001, nullptr);
	addButton(Common::Rect(227, 41, 139, 51), 1002, nullptr);
	addButton(Common::Rect(227, 52, 139, 62), 1003, nullptr);
	addButton(Common::Rect(227, 63, 139, 73), 1004, nullptr);
	addButton(Common::Rect(227, 74, 139, 84), 1005, nullptr);
	addButton(Common::Rect(227, 85, 139, 95), 1006, nullptr);
	addButton(Common::Rect(227, 96, 139, 106), 1007, nullptr);
	addButton(Common::Rect(227, 107, 139, 117), 1008, nullptr);
	addButton(Common::Rect(227, 118, 139, 128), 1009, nullptr);

	// Load the background
	screen.loadBackground("create.raw");
	events.setCursor(0);

	while (!_vm->shouldQuit()) {
		int classId = -1;

		// Build up list of roster slot indexes that are free
		freeCharList.clear();
		for (uint idx = 0; idx < XEEN_TOTAL_CHARACTERS; ++idx) {
			if (party._roster[idx]._name.empty())
				freeCharList.push_back(idx);
		}		
		int charIndex = 0;
		//bool flag9 = true;

		if (freeCharList.size() == XEEN_TOTAL_CHARACTERS)
			break;

		// Get and race and sex for the given character
		Race race = (Race)((freeCharList[charIndex] / 4) % 5);
		Sex sex = (Sex)(freeCharList[charIndex] & 1);

		// Randomly determine attributes, and which classes they allow
		uint attribs[TOTAL_ATTRIBUTES];
		bool allowedClasses[TOTAL_CLASSES];
		throwDice(attribs, allowedClasses);

		// Set up display of the rolled character details
		Common::String msg;
		selectedClass = newCharDetails(attribs, allowedClasses,
			race, sex, classId, selectedClass, msg);

		// Draw the screen
		Window &w = screen._windows[0];
		icons.draw(w, 10, Common::Point(168, 19));
		icons.draw(w, 12, Common::Point(168, 43));
		icons.draw(w, 14, Common::Point(168, 67));
		icons.draw(w, 16, Common::Point(168, 91));
		icons.draw(w, 18, Common::Point(168, 115));
		icons.draw(w, 20, Common::Point(168, 139));
		icons.draw(w, 22, Common::Point(168, 163));
		for (int idx = 0; idx < 9; ++idx)
			icons.draw(w, 24 + idx * 2, Common::Point(227, 19 + 11 * idx));

		for (int idx = 0; idx < 7; ++idx)
			icons.draw(w, 50 + idx, Common::Point(195, 31 + 24 * idx));

		icons.draw(w, 57, Common::Point(62, 148));
		icons.draw(w, 58, Common::Point(62, 158));
		icons.draw(w, 59, Common::Point(62, 168));
		icons.draw(w, 61, Common::Point(220, 19));
		icons.draw(w, 64, Common::Point(220, 155));
		icons.draw(w, 65, Common::Point(220, 170));
		
		party._roster[freeCharList[charIndex]]._faceSprites->draw(
			w, 0, Common::Point(27, 102));

		icons.draw(w, 0, Common::Point(132, 98));
		icons.draw(w, 2, Common::Point(132, 128));
		icons.draw(w, 4, Common::Point(132, 158));
		icons.draw(w, 6, Common::Point(86, 98));
		icons.draw(w, 8, Common::Point(86, 120));

		w.writeString(msg);
		w.update();

		// Draw the arrow for the selected class, if applicable
		if (selectedClass != -1)
			printSelectionArrow(icons, selectedClass);

		// Draw the dice
		drawDice(dice);
		if (!hasFadedIn) {
			screen.fadeIn(4);
			hasFadedIn = true;
		}

		// Key handling loop
		while (!_vm->shouldQuit()) {
			// Animate the dice until a user action occurs
			_buttonValue = 0;
			while (!_vm->shouldQuit() && !_buttonValue)
				drawDice(dice);

			// TODO
		}
		
		// TODO: More
		error("TODO: createChar");
	}

	_vm->_mode = oldMode;
}

int PartyDialog::selectCharacter(bool isDelete, int firstDisplayChar) {
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Window &w = screen._windows[28];

	SpriteResource iconSprites;
	iconSprites.load("esc.icn");

	w.setBounds(Common::Rect(50, isDelete ? 112 : 76, 266, isDelete ? 148 : 112));
	w.open();
	w.writeString(Common::String::format(REMOVE_OR_DELETE_WHICH,
		REMOVE_DELETE[isDelete ? 1 : 0]));
	iconSprites.draw(w, 0, Common::Point(225, isDelete ? 120 : 84));
	w.update();

	saveButtons();
	addButton(Common::Rect(225, isDelete ? 120 : 84, 249, isDelete ? 140 : 104), 
		Common::KEYCODE_ESCAPE, &iconSprites);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1, &iconSprites, false);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2, &iconSprites, false);
	addButton(Common::Rect(16, 59, 48, 91), Common::KEYCODE_3, &iconSprites, false);
	addButton(Common::Rect(117, 59, 149, 91), Common::KEYCODE_4, &iconSprites, false);
	addPartyButtons(_vm);

	int result = -1, v;
	while (!_vm->shouldQuit() && result == -1) {
		_buttonValue = 0;
		while (!_vm->shouldQuit() && !_buttonValue) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}

		switch (_buttonValue) {
		case Common::KEYCODE_ESCAPE:
			result = 0;
			break;

		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			if (!isDelete) {
				v = _buttonValue - Common::KEYCODE_F1;
				if (v < (int)party._activeParty.size())
					result = _buttonValue;
			}
			break;

		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
			if (isDelete) {
				v = _buttonValue - Common::KEYCODE_1;
				if ((firstDisplayChar + v) < (int)_charList.size())
					result = _buttonValue;
			}
			break;

		default:
			break;
		}
	} 

	w.close();
	restoreButtons();
	return result == -1 ? 0 : result;
}

/**
 * Roll up some random values for the attributes, and return both them as
 * well as a list of classes that the attributes meet the requirements for
 */
void PartyDialog::throwDice(uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]) {
	bool repeat = true;
	do {
		// Default all the attributes to zero
		Common::fill(&attribs[0], &attribs[TOTAL_ATTRIBUTES], 0);

		// Assign random amounts to each attribute
		for (int idx1 = 0; idx1 < 3; ++idx1) {
			for (int idx2 = 0; idx2 < TOTAL_ATTRIBUTES; ++idx2) {
				attribs[idx1] += _vm->getRandomNumber(10, 79) / 10;
			}
		}

		// Check which classes are allowed based on the rolled attributes
		checkClass(attribs, allowedClasses);

		// Only exit if the attributes allow for at least one class
		for (int idx = 0; idx < TOTAL_CLASSES; ++idx) {
			if (allowedClasses[idx])
				repeat = false;
		}
	} while (repeat);
}

/**
 * Set a list of flags for which classes the passed attribute set meet the
 * minimum requirements of
 */
void PartyDialog::checkClass(const uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]) {
	allowedClasses[CLASS_KNIGHT] = attribs[MIGHT] >= 15;
	allowedClasses[CLASS_PALADIN] = attribs[MIGHT] >= 13
		&& attribs[PERSONALITY] >= 13 && attribs[ENDURANCE] >= 13;
	allowedClasses[CLASS_ARCHER] = attribs[INTELLECT] >= 13 && attribs[ACCURACY] >= 13;
	allowedClasses[CLASS_CLERIC] = attribs[PERSONALITY] >= 13;
	allowedClasses[CLASS_SORCERER] = attribs[INTELLECT] >= 13;
	allowedClasses[CLASS_ROBBER] = attribs[LUCK] >= 13;
	allowedClasses[CLASS_NINJA] = attribs[SPEED] >= 13 && attribs[ACCURACY] >= 13;
	allowedClasses[CLASS_BARBARIAN] = attribs[ENDURANCE] >= 15;
	allowedClasses[CLASS_DRUID] = attribs[INTELLECT] >= 15 && attribs[PERSONALITY] >= 15;
	allowedClasses[CLASS_RANGER] = attribs[INTELLECT] >= 12 && attribs[PERSONALITY] >= 12
		&& attribs[ENDURANCE] >= 12 && attribs[SPEED] >= 12;
}

/**
 * Return details of the generated character
 */
int PartyDialog::newCharDetails(const uint attribs[TOTAL_ATTRIBUTES],
		bool allowedClasses[TOTAL_CLASSES], Race race, Sex sex, int classId, 
		int selectedClass, Common::String &msg) {
	int foundClass = -1;
	Common::String skillStr, classStr, raceSkillStr;

	// If a selected class is provided, set the default skill for that class
	if (classId != -1 && NEW_CHAR_SKILLS[classId] != -1) {
		const char *skillP = SKILL_NAMES[NEW_CHAR_SKILLS[classId]];
		skillStr = Common::String(skillP, skillP + NEW_CHAR_SKILLS_LEN[classId]);
	}

	// If a class is provided, set the class name
	if (classId != -1) {
		classStr = Common::String::format("\t062\v168%s", CLASS_NAMES[classId]);
	}

	// Set up default skill for the race, if any
	if (NEW_CHAR_RACE_SKILLS[race] != -1) {
		raceSkillStr = SKILL_NAMES[NEW_CHAR_RACE_SKILLS[race]];
	}

	// Set up color to use for each skill string to be displayed, based
	// on whether each class is allowed or not for the given attributes
	int classColors[TOTAL_CLASSES];
	Common::fill(&classColors[0], &classColors[TOTAL_CLASSES], 0);
	for (int classNum = CLASS_KNIGHT; classNum <= CLASS_RANGER; ++classNum) {
		if (allowedClasses[classNum]) {
			if (classId == -1 && (foundClass == -1 || foundClass < classNum))
				foundClass = classNum;
			classColors[classNum] = 4;
		}
	}

	// Return stats details and character class
	msg = Common::String::format(NEW_CHAR_STATS, RACE_NAMES[race], SEX_NAMES[sex],
		attribs[MIGHT], attribs[INTELLECT], attribs[PERSONALITY],
		attribs[ENDURANCE], attribs[SPEED], attribs[ACCURACY], attribs[LUCK],
		classColors[CLASS_KNIGHT], classColors[CLASS_PALADIN],
		classColors[CLASS_ARCHER], classColors[CLASS_CLERIC],
		classColors[CLASS_SORCERER], classColors[CLASS_ROBBER],
		classColors[CLASS_NINJA], classColors[CLASS_BARBARIAN],
		classColors[CLASS_DRUID], classColors[CLASS_RANGER],
		skillStr.c_str(), raceSkillStr.c_str(), classStr.c_str()
	);
	return classId == -1 ? foundClass : selectedClass;
}

/**
 * Print the selection arrow to indicate the selected class
 */
void PartyDialog::printSelectionArrow(SpriteResource &icons, int selectedClass) {
	Window &w = _vm->_screen->_windows[0];
	icons.draw(w, 61, Common::Point(220, 19));
	icons.draw(w, 63, Common::Point(220, selectedClass * 11 + 21));
	w.update();
}

/**
 * Print the dice animation
 */
void PartyDialog::drawDice(SpriteResource &dice) {
	error("TODO: drawDice");
}

} // End of namespace Xeen
