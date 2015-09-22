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

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_exit		//Needed for IRIX's unistd.h

#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/algorithm.h"

#include <retro_dirent.h>
#include <retro_stat.h>
#include <file/file_path.h>
#include <stdio.h>

void POSIXFilesystemNode::setFlags()
{
   const char *fspath = _path.c_str();

   _isValid     = path_is_valid(fspath);
   _isDirectory = path_is_directory(fspath);
}

POSIXFilesystemNode::POSIXFilesystemNode(const Common::String &p)
{
	assert(p.size() > 0);

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/"))
   {
		const char *home = getenv("HOME");
		if (home != NULL && strlen(home) < MAXPATHLEN)
      {
			_path = home;
			// Skip over the tilda.  We know that p contains at least
			// two chars, so this is safe:
			_path += p.c_str() + 1;
		}
	}
   else
		_path = p;

	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');

	setFlags();
}

AbstractFSNode *POSIXFilesystemNode::getChild(const Common::String &n) const {
	assert(!_path.empty());
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	// We assume here that _path is already normalized (hence don't bother to call
	//  Common::normalizePath on the final path).
	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return makeNode(newPath);
}

bool POSIXFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

	struct RDIR *dirp = retro_opendir(_path.c_str());

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((retro_readdir(dirp)))
   {
      const char *d_name = retro_dirent_get_name(dirp);

      // Skip 'invisible' files if necessary
      if (d_name[0] == '.' && !hidden)
         continue;
      // Skip '.' and '..' to avoid cycles
      if ((d_name[0] == '.' && d_name[1] == 0) || (d_name[0] == '.' && d_name[1] == '.'))
         continue;

      // Start with a clone of this node, with the correct path set
      POSIXFilesystemNode entry(*this);
      entry._displayName = d_name;

      if (_path.lastChar() != '/')
         entry._path += '/';
      entry._path       += entry._displayName;

      entry._isValid     = true;
      entry._isDirectory = retro_dirent_is_dir(dirp, d_name);

      // Skip files that are invalid for some reason (e.g. because we couldn't
      // properly stat them).
      if (!entry._isValid)
         continue;

      // Honor the chosen mode
      if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
            (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
         continue;

      myList.push_back(new POSIXFilesystemNode(entry));
   }
	retro_closedir(dirp);

	return true;
}

AbstractFSNode *POSIXFilesystemNode::getParent() const
{
	if (_path == "/")
		return 0;	// The filesystem root has no parent

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end-1) != '/')
		end--;

	if (end == start)
		return 0;

	return makeNode(Common::String(start, end));
}

Common::SeekableReadStream *POSIXFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), false);
}

Common::WriteStream *POSIXFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}
