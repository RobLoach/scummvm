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

#ifndef TITANIC_TT_TALKER_H
#define TITANIC_TT_TALKER_H

#include "titanic/core/list.h"
#include "titanic/npcs/true_talk_npc.h"
#include "titanic/support/string.h"

namespace Titanic {

class CTrueTalkManager;

class TTTalker : public ListItem {
public:
	CTrueTalkManager *_owner;
	CTrueTalkNPC *_npc;
	CString _string1;
	int _dialogueId;
	int _field24;
	int _field28;
public:
	TTTalker() : _owner(nullptr), _npc(nullptr),
		_dialogueId(0), _field24(0), _field28(0) {}
	TTTalker(CTrueTalkManager *owner, CTrueTalkNPC *npc) :
		_owner(owner), _npc(npc), _dialogueId(0), _field24(0), _field28(0) {}

	void speechStarted(const CString &dialogueStr, uint dialogueId, uint soundId);
};

class TTTalkerList : public List<TTTalker> {
};

} // End of namespace Titanic

#endif /* TITANIC_TT_TALKER_H */
