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

#include "graphics/managed_surface.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Graphics {

const int SCALE_THRESHOLD = 0x100;

ManagedSurface::ManagedSurface() :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr) {
}

ManagedSurface::ManagedSurface(ManagedSurface &surf) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr) {
	*this = surf;
}

ManagedSurface::ManagedSurface(int width, int height) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr) {
	create(width, height);
}

ManagedSurface::ManagedSurface(int width, int height, const Graphics::PixelFormat &pixelFormat) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr) {
	create(width, height, pixelFormat);
}

ManagedSurface::ManagedSurface(ManagedSurface &surf, const Common::Rect &bounds) :
		w(_innerSurface.w), h(_innerSurface.h), pitch(_innerSurface.pitch), format(_innerSurface.format),
		_disposeAfterUse(DisposeAfterUse::NO), _owner(nullptr) {
	create(surf, bounds);
}

ManagedSurface::~ManagedSurface() {
	free();
}

ManagedSurface &ManagedSurface::operator=(ManagedSurface &surf) {
	// Free any current surface
	free();

	if (surf._disposeAfterUse == DisposeAfterUse::YES) {
		// Create a new surface and copy the pixels from the source surface
		create(surf.w, surf.h, surf.format);
		Common::copy((const byte *)surf.getPixels(), (const byte *)surf.getPixels() +
			surf.w * surf.h * surf.format.bytesPerPixel, (byte *)this->getPixels());
	} else {
		// Source isn't managed, so simply copy its fields
		_owner = surf._owner;
		_offsetFromOwner = surf._offsetFromOwner;
		void *srcPixels = surf._innerSurface.getPixels();
		_innerSurface.setPixels(srcPixels);
		_innerSurface.w = surf.w;
		_innerSurface.h = surf.h;
		_innerSurface.pitch = surf.pitch;
		this->format = surf.format;
	}

	return *this;
}

void ManagedSurface::setPixels(void *newPixels) {
	free();
	_innerSurface.setPixels(newPixels);
}

void ManagedSurface::create(uint16 width, uint16 height) {
	create(width, height, PixelFormat::createFormatCLUT8());
}

void ManagedSurface::create(uint16 width, uint16 height, const PixelFormat &pixelFormat) {
	free();
	_innerSurface.create(width, height, pixelFormat);

	_disposeAfterUse = DisposeAfterUse::YES;
	markAllDirty();
}

void ManagedSurface::create(ManagedSurface &surf, const Common::Rect &bounds) {
	free();

	_offsetFromOwner = Common::Point(bounds.left, bounds.top);
	_innerSurface.setPixels(surf.getBasePtr(bounds.left, bounds.top));
	_innerSurface.pitch = surf.pitch;
	_innerSurface.format = surf.format;
	_innerSurface.w = bounds.width();
	_innerSurface.h = bounds.height();
	_owner = &surf;
	_disposeAfterUse = DisposeAfterUse::NO;
}

void ManagedSurface::free() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		_innerSurface.free();

	_disposeAfterUse = DisposeAfterUse::NO;
	_owner = nullptr;
	_offsetFromOwner = Common::Point(0, 0);
}

bool ManagedSurface::clip(Common::Rect &srcBounds, Common::Rect &destBounds) {
	if (destBounds.left >= this->w || destBounds.top >= this->h ||
			destBounds.right <= 0 || destBounds.bottom <= 0)
		return false;

	// Clip the bounds if necessary to fit on-screen
	if (destBounds.right > this->w) {
		srcBounds.right -= destBounds.right - this->w;
		destBounds.right = this->w;
	}

	if (destBounds.bottom > this->h) {
		srcBounds.bottom -= destBounds.bottom - this->h;
		destBounds.bottom = this->h;
	}

	if (destBounds.top < 0) {
		srcBounds.top += -destBounds.top;
		destBounds.top = 0;
	}

	if (destBounds.left < 0) {
		srcBounds.left += -destBounds.left;
		destBounds.left = 0;
	}

	return true;
}

void ManagedSurface::blitFrom(const Surface &src) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Point(0, 0));
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Point &destPos) {
	blitFrom(src, Common::Rect(0, 0, src.w, src.h), destPos);
}

void ManagedSurface::blitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos) {
	Common::Rect srcBounds = srcRect;
	Common::Rect destBounds(destPos.x, destPos.y, destPos.x + srcRect.width(),
		destPos.y + srcRect.height());
	assert(src.format.bytesPerPixel == format.bytesPerPixel);

	if (!srcRect.isValidRect() || !clip(srcBounds, destBounds))
		return;

	for (int y = 0; y < srcBounds.height(); ++y) {
		const byte *srcP = (const byte *)src.getBasePtr(srcBounds.left, srcBounds.top + y);
		byte *destP = (byte *)getBasePtr(destBounds.left, destBounds.top + y);
		Common::copy(srcP, srcP + srcBounds.width() * format.bytesPerPixel, destP);
	}

	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void ManagedSurface::transBlitFrom(const Surface &src, uint transColor, bool flipped, uint overrideColor) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(0, 0, this->w, this->h),
		transColor, false, overrideColor);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Point &destPos,
		uint transColor, bool flipped, uint overrideColor) {
	transBlitFrom(src, Common::Rect(0, 0, src.w, src.h), Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, false, overrideColor);
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
		const Common::Point &destPos, uint transColor, bool flipped, uint overrideColor) {
	transBlitFrom(src, srcRect, Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h), transColor, false, overrideColor);
}

template<typename T>
void transBlit(const Surface &src, const Common::Rect &srcRect, Surface *dest, const Common::Rect &destRect, uint transColor, bool flipped, uint overrideColor) {
	int scaleX = SCALE_THRESHOLD * srcRect.width() / destRect.width();
	int scaleY = SCALE_THRESHOLD * srcRect.height() / destRect.height();

	// Loop through drawing output lines
	for (int destY = destRect.top, scaleYCtr = 0; destY < destRect.bottom; ++destY, scaleYCtr += scaleY) {
		if (destY < 0 || destY >= dest->h)
			continue;
		const T *srcLine = (const T *)src.getBasePtr(0, scaleYCtr / SCALE_THRESHOLD);
		T *destLine = (T *)dest->getBasePtr(destRect.left, destY);

		// Loop through drawing the pixels of the row
		for (int destX = destRect.left, xCtr = 0, scaleXCtr = 0; destX < destRect.right; ++destX, ++xCtr, scaleXCtr += scaleX) {
			if (destX < 0 || destX >= dest->w)
				continue;

			T srcVal = srcLine[flipped ? src.w - scaleXCtr / SCALE_THRESHOLD - 1 : scaleXCtr / SCALE_THRESHOLD];
			if (srcVal != transColor) {
				destLine[xCtr] = overrideColor ? overrideColor : srcVal;
			}
		}
	}
}

void ManagedSurface::transBlitFrom(const Surface &src, const Common::Rect &srcRect,
	const Common::Rect &destRect, uint transColor, bool flipped, uint overrideColor) {
	if (src.w == 0 || src.h == 0 || destRect.width() == 0 || destRect.height() == 0)
		return;

	if (format.bytesPerPixel == 1)
		transBlit<byte>(src, srcRect, &_innerSurface, destRect, transColor, flipped, overrideColor);
	else if (format.bytesPerPixel == 2)
		transBlit<uint16>(src, srcRect, &_innerSurface, destRect, transColor, flipped, overrideColor);
	else if (format.bytesPerPixel == 4)
		transBlit<uint32>(src, srcRect, &_innerSurface, destRect, transColor, flipped, overrideColor);
	else
		error("Surface::transBlitFrom: bytesPerPixel must be 1, 2, or 4");

	// Mark the affected area
	addDirtyRect(destRect);
}

void ManagedSurface::markAllDirty() {
	addDirtyRect(Common::Rect(0, 0, this->w, this->h));
}

void ManagedSurface::addDirtyRect(const Common::Rect &r) {
	if (_owner) {
		Common::Rect bounds = r;
		bounds.clip(Common::Rect(0, 0, this->w, this->h));
		bounds.translate(_offsetFromOwner.x, _offsetFromOwner.y);
		_owner->addDirtyRect(bounds);
	}
}

void ManagedSurface::clear(uint color) {
	fillRect(getBounds(), color);
}

} // End of namespace Graphics
