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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "dreamweb/dreamweb.h"

namespace DreamWeb {

const uint16 kKeypadx = 36+112;
const uint16 kKeypady = 72;

void DreamWebEngine::enterCode(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3) {
	RectWithCallback keypadList[] = {
		{ kKeypadx+9,kKeypadx+30,kKeypady+9,kKeypady+22,&DreamWebEngine::buttonOne },
		{ kKeypadx+31,kKeypadx+52,kKeypady+9,kKeypady+22,&DreamWebEngine::buttonTwo },
		{ kKeypadx+53,kKeypadx+74,kKeypady+9,kKeypady+22,&DreamWebEngine::buttonThree },
		{ kKeypadx+9,kKeypadx+30,kKeypady+23,kKeypady+40,&DreamWebEngine::buttonFour },
		{ kKeypadx+31,kKeypadx+52,kKeypady+23,kKeypady+40,&DreamWebEngine::buttonFive },
		{ kKeypadx+53,kKeypadx+74,kKeypady+23,kKeypady+40,&DreamWebEngine::buttonSix },
		{ kKeypadx+9,kKeypadx+30,kKeypady+41,kKeypady+58,&DreamWebEngine::buttonSeven },
		{ kKeypadx+31,kKeypadx+52,kKeypady+41,kKeypady+58,&DreamWebEngine::buttonEight },
		{ kKeypadx+53,kKeypadx+74,kKeypady+41,kKeypady+58,&DreamWebEngine::buttonNine },
		{ kKeypadx+9,kKeypadx+30,kKeypady+59,kKeypady+73,&DreamWebEngine::buttonNought },
		{ kKeypadx+31,kKeypadx+74,kKeypady+59,kKeypady+73,&DreamWebEngine::buttonEnter },
		{ kKeypadx+72,kKeypadx+86,kKeypady+80,kKeypady+94,&DreamWebEngine::quitKey },
		{ 0,320,0,200,&DreamWebEngine::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	getRidOfReels();
	loadKeypad();
	createPanel();
	showIcon();
	showOuterPad();
	showKeypad();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_pressPointer = 0;
	_getBack = 0;
	while (true) {
		delPointer();
		readMouse();
		showKeypad();
		showPointer();
		vSync();
		if (_pressCount == 0) {
			_pressed = 255;
			_graphicPress = 255;
			vSync();
		} else
			--_pressCount;

		dumpPointer();
		dumpKeypad();
		dumpTextLine();
		checkCoords(keypadList);
		if (_quitRequested || (_getBack == 1))
			break;
		if (_lightCount == 1) {
			if (_vars._lockStatus == 0)
				break;
		} else {
			if (_pressCount == 40) {
				addToPressList();
				if (_pressed == 11) {
					if (isItRight(digit0, digit1, digit2, digit3))
						_vars._lockStatus = 0;
					playChannel1(11);
					_lightCount = 120;
					_pressPointer = 0;
				}
			}
		}
	}
	_manIsOffScreen = 0;
	getRidOfTemp();
	restoreReels();
	redrawMainScrn();
	workToScreenM();
}

// Note: isItRight comes from use.asm, but is only used by enterCode(),
// so we place it here.
bool DreamWebEngine::isItRight(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3) {

	return digit0 == _pressList[0] && digit1 == _pressList[1]
		&& digit2 == _pressList[2] && digit3 == _pressList[3];
}

void DreamWebEngine::loadKeypad() {
	loadIntoTemp("DREAMWEB.G02");
}

void DreamWebEngine::quitKey() {
	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(4);
	}

	if (_mouseButton != _oldButton && (_mouseButton & 1))
		_getBack = 1;
}

void DreamWebEngine::addToPressList() {
	if (_pressPointer == 5)
		return;
	uint8 pressed = _pressed;
	if (pressed == 10)
		pressed = 0;

	_pressList[_pressPointer] = pressed;
	++_pressPointer;
}

void DreamWebEngine::buttonOne() {
	buttonPress(1);
}

void DreamWebEngine::buttonTwo() {
	buttonPress(2);
}

void DreamWebEngine::buttonThree() {
	buttonPress(3);
}

void DreamWebEngine::buttonFour() {
	buttonPress(4);
}

void DreamWebEngine::buttonFive() {
	buttonPress(5);
}

void DreamWebEngine::buttonSix() {
	buttonPress(6);
}

void DreamWebEngine::buttonSeven() {
	buttonPress(7);
}

void DreamWebEngine::buttonEight() {
	buttonPress(8);
}

void DreamWebEngine::buttonNine() {
	buttonPress(9);
}

void DreamWebEngine::buttonNought() {
	buttonPress(10);
}

void DreamWebEngine::buttonEnter() {
	buttonPress(11);
}

void DreamWebEngine::buttonPress(uint8 buttonId) {
	uint8 commandType = 100 + buttonId;
	if (_commandType != commandType) {
		_commandType = commandType;
		commandOnly(buttonId + 4);
	}
	if ((_mouseButton & 1) && (_mouseButton != _oldButton)) {
		_pressed = buttonId;
		_graphicPress = buttonId + 21;
		_pressCount = 40;
		if (buttonId != 11)
			playChannel1(10);
	}
}

void DreamWebEngine::showOuterPad() {
	showFrame(_tempGraphics, kKeypadx-3, kKeypady-4, 1, 0);
	showFrame(_tempGraphics, kKeypadx+74, kKeypady+76, 37, 0);
}

void DreamWebEngine::showKeypad() {
	singleKey(22, kKeypadx+9,  kKeypady+5);
	singleKey(23, kKeypadx+31, kKeypady+5);
	singleKey(24, kKeypadx+53, kKeypady+5);
	singleKey(25, kKeypadx+9,  kKeypady+23);
	singleKey(26, kKeypadx+31, kKeypady+23);
	singleKey(27, kKeypadx+53, kKeypady+23);
	singleKey(28, kKeypadx+9,  kKeypady+41);
	singleKey(29, kKeypadx+31, kKeypady+41);
	singleKey(30, kKeypadx+53, kKeypady+41);
	singleKey(31, kKeypadx+9,  kKeypady+59);
	singleKey(32, kKeypadx+31, kKeypady+59);
	if (_lightCount) {
		--_lightCount;
		uint8 frameIndex;
		uint16 y;
		if (_vars._lockStatus) {
			frameIndex = 36;
			y = kKeypady-1+63;
		} else {
			frameIndex = 41;
			y = kKeypady+4+63;
		}
		if ((_lightCount >= 60) && (_lightCount < 100))
			--frameIndex;
		showFrame(_tempGraphics, kKeypadx+60, y, frameIndex, 0);
	}
}

void DreamWebEngine::singleKey(uint8 key, uint16 x, uint16 y) {
	if (key == _graphicPress) {
		key += 11;
		if (_pressCount < 8)
			key -= 11;
	}
	key -= 20;
	showFrame(_tempGraphics, x, y, key, 0);
}

void DreamWebEngine::dumpKeypad() {
	multiDump(kKeypadx - 3, kKeypady - 4, 120, 90);
}

void DreamWebEngine::useMenu() {
	getRidOfReels();
	loadMenu();
	createPanel();
	showPanel();
	showIcon();
	_vars._newObs = 0;
	drawFloor();
	printSprites();
	showFrame(_tempGraphics2, kMenux-48, kMenuy-4, 4, 0);
	getUnderMenu();
	showFrame(_tempGraphics2, kMenux+54, kMenuy+72, 5, 0);
	workToScreenM();
	_getBack = 0;
	do {
		delPointer();
		putUnderMenu();
		showMenu();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpMenu();
		dumpTextLine();
		RectWithCallback menuList[] = {
			{ kMenux+54,kMenux+68,kMenuy+72,kMenuy+88,&DreamWebEngine::quitKey },
			{ 0,320,0,200,&DreamWebEngine::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(menuList);
	} while ((_getBack != 1) && !_quitRequested);
	_manIsOffScreen = 0;
	redrawMainScrn();
	getRidOfTemp();
	getRidOfTemp2();
	restoreReels();
	workToScreenM();
}

void DreamWebEngine::dumpMenu() {
	multiDump(kMenux, kMenuy, 48, 48);
}

void DreamWebEngine::getUnderMenu() {
	multiGet(_underTimedText, kMenux, kMenuy, 48, 48);
}

void DreamWebEngine::putUnderMenu() {
	multiPut(_underTimedText, kMenux, kMenuy, 48, 48);
}

// Note: showoutermenu from the asm version was unused and thus not placed here

void DreamWebEngine::showMenu() {
	++_menuCount;
	if (_menuCount == 37*2)
		_menuCount = 0;
	showFrame(_tempGraphics, kMenux, kMenuy, _menuCount / 2, 0);
}

void DreamWebEngine::loadMenu() {
	loadIntoTemp("DREAMWEB.S02"); // sprite name 3
	loadIntoTemp2("DREAMWEB.G07"); // mon. graphics 2
}

void DreamWebEngine::viewFolder() {
	_manIsOffScreen = 1;
	getRidOfAll();
	loadFolder();
	_folderPage = 0;
	showFolder();
	workToScreenM();
	_getBack = 0;
	do {
		if (_quitRequested)
			break;
		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		checkFolderCoords();
	} while (_getBack == 0);
	_manIsOffScreen = 0;
	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	getRidOfTempCharset();
	restoreAll();
	redrawMainScrn();
	workToScreenM();
}

void DreamWebEngine::nextFolder() {
	if (_folderPage == 12) {
		blank();
		return;
	}
	if (_commandType != 201) {
		_commandType = 201;
		commandOnly(16);
	}
	if ((_mouseButton == 1) && (_mouseButton != _oldButton)) {
		++_folderPage;
		folderHints();
		delPointer();
		showFolder();
		_mouseButton = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamWebEngine::folderHints() {
	if (_folderPage == 5) {
		if ((_vars._aideDead != 1) && (getLocation(13) != 1)) {
			setLocation(13);
			showFolder();
			const uint8 *string = getTextInFile1(30);
			printDirect(string, 0, 86, 141, true);
			workToScreenM();
			hangOnP(200);
		}
	} else if (_folderPage == 9) {
		if (getLocation(7) != 1) {
			setLocation(7);
			showFolder();
			const uint8 *string = getTextInFile1(31);
			printDirect(string, 0, 86, 141, true);
			workToScreenM();
			hangOnP(200);
		}
	}
}

void DreamWebEngine::lastFolder() {
	if (_folderPage == 0) {
		blank();
		return;
	}
	if (_commandType != 202) {
		_commandType = 202;
		commandOnly(17);
	}

	if ((_mouseButton == 1) && (_mouseButton != _oldButton)) {
		--_folderPage;
		delPointer();
		showFolder();
		_mouseButton = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamWebEngine::checkFolderCoords() {
	RectWithCallback folderList[] = {
		{ 280,320,160,200, &DreamWebEngine::quitKey },
		{ 143,300,6,194, &DreamWebEngine::nextFolder },
		{ 0,143,6,194, &DreamWebEngine::lastFolder },
		{ 0,320,0,200, &DreamWebEngine::blank },
		{ 0xFFFF,0,0,0, 0 }
	};
	checkCoords(folderList);
}

void DreamWebEngine::loadFolder() {
	loadIntoTemp("DREAMWEB.G09"); // folder graphics 1
	loadIntoTemp2("DREAMWEB.G10"); // folder graphics 2
	loadIntoTemp3("DREAMWEB.G11"); // folder graphics 3
	loadTempCharset("DREAMWEB.C02"); // character set 3
	loadTempText("DREAMWEB.T50"); // folder text
}

void DreamWebEngine::showFolder() {
	_commandType = 255;
	if (_folderPage) {
		useTempCharset();
		createPanel2();
		showFrame(_tempGraphics, 0, 0, 0, 0);
		showFrame(_tempGraphics, 143, 0, 1, 0);
		showFrame(_tempGraphics, 0, 92, 2, 0);
		showFrame(_tempGraphics, 143, 92, 3, 0);
		folderExit();
		if (_folderPage != 1)
			showLeftPage();
		if (_folderPage != 12)
			showRightPage();
		useCharset1();
		underTextLine();
	} else {
		createPanel2();
		showFrame(_tempGraphics3, 143-28, 0, 0, 0);
		showFrame(_tempGraphics3, 143-28, 92, 1, 0);
		folderExit();
		underTextLine();
	}
}

void DreamWebEngine::folderExit() {
	showFrame(_tempGraphics2, 296, 178, 6, 0);
}

void DreamWebEngine::showLeftPage() {
	showFrame(_tempGraphics2, 0, 12, 3, 0);
	uint16 y = 12+5;
	for (size_t i = 0; i < 9; ++i) {
		showFrame(_tempGraphics2, 0, y, 4, 0);
		y += 16;
	}
	showFrame(_tempGraphics2, 0, y, 5, 0);
	_lineSpacing = 8;
	_charShift = 91;
	_kerning = 1;
	uint8 pageIndex = _folderPage - 2;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 2, &y, 140, false);
			y += _lineSpacing;
		} while (lastChar != '\0');
	}
	_kerning = 0;
	_charShift = 0;
	_lineSpacing = 10;
	uint8 *bufferToSwap = workspace() + (48*320)+2;
	for (size_t i = 0; i < 120; ++i) {
		for (size_t j = 0; j < 65; ++j) {
			SWAP(bufferToSwap[j], bufferToSwap[130 - j]);
		}
		bufferToSwap += 320;
	}
}

void DreamWebEngine::showRightPage() {
	showFrame(_tempGraphics2, 143, 12, 0, 0);
	uint16 y = 12+37;
	for (size_t i = 0; i < 7; ++i) {
		showFrame(_tempGraphics2, 143, y, 1, 0);
		y += 16;
	}

	showFrame(_tempGraphics2, 143, y, 2, 0);
	_lineSpacing = 8;
	_kerning = 1;
	uint8 pageIndex = _folderPage - 1;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 152, &y, 140, false);
			y += _lineSpacing;
		} while (lastChar != '\0');
	}
	_kerning = 0;
	_lineSpacing = 10;
}

void DreamWebEngine::enterSymbol() {
	_manIsOffScreen = 1;
	getRidOfReels();
	loadIntoTemp("DREAMWEB.G12"); // symbol graphics
	_symbolTopX = 24;
	_symbolTopDir = 0;
	_symbolBotX = 24;
	_symbolBotDir = 0;
	redrawMainScrn();
	showSymbol();
	underTextLine();
	workToScreenM();
	_getBack = 0;
	do {
		delPointer();
		updateSymbolTop();
		updateSymbolBot();
		showSymbol();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		dumpSymbol();
		RectWithCallback symbolList[] = {
			{ kSymbolx+40,kSymbolx+64,kSymboly+2,kSymboly+16,&DreamWebEngine::quitSymbol },
			{ kSymbolx,kSymbolx+52,kSymboly+20,kSymboly+50,&DreamWebEngine::setTopLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+20,kSymboly+50,&DreamWebEngine::setTopRight },
			{ kSymbolx,kSymbolx+52,kSymboly+50,kSymboly+80,&DreamWebEngine::setBotLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+50,kSymboly+80,&DreamWebEngine::setBotRight },
			{ 0,320,0,200,&DreamWebEngine::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(symbolList);
	} while ((_getBack == 0) && !_quitRequested);
	if ((_symbolBotNum == 3) && (_symbolTopNum == 5)) {
		removeSetObject(43);
		placeSetObject(46);
		turnAnyPathOn(0, _roomNum + 12);
		_manIsOffScreen = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
		playChannel1(13);
	} else {
		removeSetObject(46);
		placeSetObject(43);
		turnAnyPathOff(0, _roomNum + 12);
		_manIsOffScreen = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
	}
}

void DreamWebEngine::quitSymbol() {
	if (_symbolTopX != 24 || _symbolBotX != 24) {
		blank();
		return;
	};

	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(18);
	}

	if (_mouseButton == _oldButton)
		return;	// notqs

	if (!(_mouseButton & 1))
		return;

	_getBack = 1;
}

void DreamWebEngine::setTopLeft() {
	if (_symbolTopDir != 0) {
		blank();
		return;
	}

	if (_commandType != 210) {
		_commandType = 210;
		commandOnly(19);
	}

	if (_mouseButton != 0)
		_symbolTopDir = 0xFF;
}

void DreamWebEngine::setTopRight() {
	if (_symbolTopDir != 0) {
		blank();
		return;
	}

	if (_commandType != 211) {
		_commandType = 211;
		commandOnly(20);
	}

	if (_mouseButton != 0)
		_symbolTopDir = 1;
}

void DreamWebEngine::setBotLeft() {
	if (_symbolBotDir != 0) {
		blank();
		return;
	}

	if (_commandType != 212) {
		_commandType = 212;
		commandOnly(21);
	}

	if (_mouseButton != 0)
		_symbolBotDir = 0xFF;
}

void DreamWebEngine::setBotRight() {
	if (_symbolBotDir != 0) {
		blank();
		return;
	}

	if (_commandType != 213) {
		_commandType = 213;
		commandOnly(22);
	}

	if (_mouseButton != 0)
		_symbolBotDir = 1;
}

void DreamWebEngine::dumpSymbol() {
	_newTextLine = 0;
	multiDump(kSymbolx, kSymboly + 20, 104, 60);
}

void DreamWebEngine::showSymbol() {
	showFrame(_tempGraphics, kSymbolx, kSymboly, 12, 0);

	showFrame(_tempGraphics, _symbolTopX + kSymbolx-44, kSymboly+20, _symbolTopNum, 32);
	uint8 nextTopSymbol = nextSymbol(_symbolTopNum);
	showFrame(_tempGraphics, _symbolTopX + kSymbolx+5, kSymboly+20, nextTopSymbol, 32);
	uint8 nextNextTopSymbol = nextSymbol(nextTopSymbol);
	showFrame(_tempGraphics, _symbolTopX + kSymbolx+54, kSymboly+20, nextNextTopSymbol, 32);

	showFrame(_tempGraphics, _symbolBotX + kSymbolx-44, kSymboly+49, 6 + _symbolBotNum, 32);
	uint8 nextBotSymbol = nextSymbol(_symbolBotNum);
	showFrame(_tempGraphics, _symbolBotX + kSymbolx+5, kSymboly+49, 6 + nextBotSymbol, 32);
	uint8 nextNextBotSymbol = nextSymbol(nextBotSymbol);
	showFrame(_tempGraphics, _symbolBotX + kSymbolx+54, kSymboly+49, 6 + nextNextBotSymbol, 32);
}

uint8 DreamWebEngine::nextSymbol(uint8 symbol) {
	uint8 result = symbol + 1;
	if (result == 6)
		return 0;
	if (result == 12)
		return 6;
	return result;
}

void DreamWebEngine::updateSymbolTop() {
	if (!_symbolTopDir)
		return; // topfinished

	if (_symbolTopDir == (byte)-1) {
		// Backward
		_symbolTopX--;
		if (_symbolTopX != (byte)-1) {
			// Not wrapping
			if (_symbolTopX != 24)
				return; // topfinished
			_symbolTopDir = 0;
		} else {
			_symbolTopX = 48;
			_symbolTopNum++;
			if (_symbolTopNum != 6)
				return; // topfinished
			_symbolTopNum = 0;
		}
	} else {
		// Forward
		_symbolTopX++;
		if (_symbolTopX != 49) {
			// Not wrapping
			if (_symbolTopX != 24)
				return; // topfinished
			_symbolTopDir = 0;
		} else {
			_symbolTopX = 0;
			_symbolTopNum--;
			if (_symbolTopNum != (byte)-1)
				return; // topfinished
			_symbolTopNum = 5;
		}
	}
}

void DreamWebEngine::updateSymbolBot() {
	if (!_symbolBotDir)
		return; // botfinished

	if (_symbolBotDir == (byte)-1) {
		// Backward
		_symbolBotX--;
		if (_symbolBotX != (byte)-1) {
			// Not wrapping
			if (_symbolBotX != 24)
				return; // botfinished
			_symbolBotDir = 0;
		} else {
			_symbolBotX = 48;
			_symbolBotNum++;
			if (_symbolBotNum != 6)
				return; // botfinished
			_symbolBotNum = 0;
		}
	} else {
		// Forward
		_symbolBotX++;
		if (_symbolBotX != 49) {
			// Not wrapping
			if (_symbolBotX != 24)
				return; // botfinished
			_symbolBotDir = 0;
		} else {
			_symbolBotX = 0;
			_symbolBotNum--;
			if (_symbolBotNum != (byte)-1)
				return; // botfinished
			_symbolBotNum = 5;
		}
	}
}

void DreamWebEngine::dumpSymBox() {
	if (_dumpX != 0xFFFF) {
		multiDump(_dumpX, _dumpY, 30, 77);
		_dumpX = 0xFFFF;
	}
}

void DreamWebEngine::useDiary() {
	getRidOfReels();
	loadIntoTemp("DREAMWEB.G14");
	loadTempText("DREAMWEB.T51");
	loadTempCharset("DREAMWEB.C02");
	createPanel();
	showIcon();
	showDiary();
	underTextLine();
	showDiaryPage();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_getBack = 0;

	RectWithCallback diaryList[] = {
		{ kDiaryx+94,kDiaryx+110,kDiaryy+97,kDiaryy+113,&DreamWebEngine::diaryKeyN },
		{ kDiaryx+151,kDiaryx+167,kDiaryy+71,kDiaryy+87,&DreamWebEngine::diaryKeyP },
		{ kDiaryx+176,kDiaryx+192,kDiaryy+108,kDiaryy+124,&DreamWebEngine::quitKey },
		{ 0,320,0,200,&DreamWebEngine::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		delPointer();
		readMouse();
		showDiaryKeys();
		showPointer();
		vSync();
		dumpPointer();
		dumpDiaryKeys();
		dumpTextLine();
		checkCoords(diaryList);
	} while (!_getBack && !_quitRequested);


	getRidOfTemp();
	getRidOfTempText();
	getRidOfTempCharset();
	restoreReels();
	_manIsOffScreen = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamWebEngine::showDiary() {
	showFrame(_tempGraphics, kDiaryx, kDiaryy + 37, 1, 0);
	showFrame(_tempGraphics, kDiaryx + 176, kDiaryy + 108, 2, 0);
}

void DreamWebEngine::showDiaryKeys() {
	if (!_pressCount)
		return; // nokeyatall

	_pressCount--;

	if (!_pressCount)
		return; // nokeyatall

	if (_pressed == 'N') {
		byte frame = (_pressCount == 1) ? 3 : 4;
		showFrame(_tempGraphics, kDiaryx + 94, kDiaryy + 97, frame, 0);
	} else {
		byte frame = (_pressCount == 1) ? 5 : 6;
		showFrame(_tempGraphics, kDiaryx + 151, kDiaryy + 71, frame, 0);
	}

	if (_pressCount == 1)
		showDiaryPage();
}

void DreamWebEngine::dumpDiaryKeys() {
	if (_pressCount == 1) {
		if (_vars._sartainDead != 1 && _diaryPage == 5 && getLocation(6) != 1) {
			// Add Sartain Industries note
			setLocation(6);
			delPointer();
			const uint8 *string = getTextInFile1(12);
			printDirect(string, 70, 106, 241, 241 & 1);
			workToScreenM();
			hangOnP(200);
			createPanel();
			showIcon();
			showDiary();
			showDiaryPage();
			workToScreenM();
			showPointer();
			return;
		} else {
			multiDump(kDiaryx + 48, kDiaryy + 15, 200, 16);
		}
	}

	multiDump(kDiaryx + 94, kDiaryy + 97, 16, 16);
	multiDump(kDiaryx + 151, kDiaryy + 71, 16, 16);
}

void DreamWebEngine::diaryKeyP() {
	if (_commandType != 214) {
		_commandType = 214;
		commandOnly(23);
	}

	if (!_mouseButton ||
		_oldButton == _mouseButton ||
		_pressCount)
		return; // notkeyp

	playChannel1(16);
	_pressCount = 12;
	_pressed = 'P';
	_diaryPage--;

	if (_diaryPage == 0xFF)
		_diaryPage = 11;
}

void DreamWebEngine::diaryKeyN() {
	if (_commandType != 213) {
		_commandType = 213;
		commandOnly(23);
	}

	if (!_mouseButton ||
		_oldButton == _mouseButton ||
		_pressCount)
		return; // notkeyn

	playChannel1(16);
	_pressCount = 12;
	_pressed = 'N';
	_diaryPage++;

	if (_diaryPage == 12)
		_diaryPage = 0;
}

void DreamWebEngine::showDiaryPage() {
	showFrame(_tempGraphics, kDiaryx, kDiaryy, 0, 0);
	_kerning = 1;
	useTempCharset();
	_charShift = 91+91;
	const uint8 *string = getTextInFile1(_diaryPage);
	uint16 y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 129, &y, 240, 240 & 1);
	y = kDiaryy + 23;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	_kerning = 0;
	_charShift = 0;
	useCharset1();
}

} // End of namespace DreamWeb
