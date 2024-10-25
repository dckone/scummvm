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
#include "common/path.h"
#include "common/rect.h"
#include "common/array.h"

#include "alg/graphics.h"

namespace Alg {

Graphics::Surface * AlgGraphics::loadVgaBackground(const Common::Path &path, uint8 *palette) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	Common::File vgaFile;
	vgaFile.open(path);
	assert(vgaFile.isOpen());

	uint16 width = vgaFile.readUint16LE();
	uint16 height = vgaFile.readUint16LE();
	uint8 paletteEntries = vgaFile.readByte();
	uint8 paletteStart = vgaFile.readByte();

	assert(width == 320);
	assert(height == 200);
	assert(paletteStart == 0x10);

	for(uint32 i = paletteStart * 3; i < (paletteStart + paletteEntries) * 3; i += 3) {
		palette[i] = vgaFile.readByte();
		palette[i + 1] = vgaFile.readByte();
		palette[i + 2] = vgaFile.readByte();
	}

	uint8 *pixels = new uint8[width * height]();
	vgaFile.read(pixels, width * height);
	surface->setPixels(pixels);
	Common::Rect backgroundRect = Common::Rect(0, 0, width, height);
	surface->flipVertical(backgroundRect);

	vgaFile.close();

	return surface;
}

Common::Array<Graphics::Surface> * AlgGraphics::loadAniImage(const Common::Path &path) {
	Common::Array<Graphics::Surface> *images = new Common::Array<Graphics::Surface>();
	Common::File aniFile;
	aniFile.open(path);
	assert(aniFile.isOpen());
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	assert(paletteEntries == 0x70);
	assert(paletteStart == 0x10);
	// ignore palette data
	aniFile.skip(paletteEntries * 3);
	uint16 length, width, height;
	while (aniFile.pos() < aniFile.size()) {
		width = height = 0;
		uint64 aniSectionOffset = aniFile.pos();
		while(aniFile.pos() < aniFile.size()) {
			length = aniFile.readUint16LE();
			if (length == 0) { break; }
			width = length;
			height++;
			aniFile.skip(2 + length);
		}
		if(width > 0) {
			aniFile.seek(aniSectionOffset, SEEK_SET);
			Graphics::Surface *aniImage = new Graphics::Surface();
			aniImage->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			for(uint16 y = 0; y < height; y++) {
				aniFile.skip(4);
				for(uint16 x = 0; x < width; x++) {
					aniImage->setPixel(x, y, aniFile.readByte());
				}
			}
			images->push_back(*aniImage);
		}
	}
	aniFile.close();
	return images;
}

Common::Array<Graphics::Surface> * AlgGraphics::loadAniCursor(const Common::Path &path) {
	Common::Array<Graphics::Surface> *images = new Common::Array<Graphics::Surface>();
	Common::File aniFile;
	aniFile.open(path);
	assert(aniFile.isOpen());
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	assert(paletteEntries == 0x70);
	assert(paletteStart == 0x10);
	// ignore palette data
	aniFile.skip(paletteEntries * 3);
	uint16 length = 0;
	int16 offset = 0;
	uint32 dest = 0;
	uint32 x, y = 0;
	while (aniFile.pos() < aniFile.size()) {
		Graphics::Surface *renderTarget = new Graphics::Surface();
		renderTarget->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
		uint32 startOffset = (320 * 100) + 160; // center
		while(1) {
			length = aniFile.readUint16LE();
			if (length == 0) { break; }
			offset = aniFile.readSint16LE();
	        dest = startOffset + offset;
			for (uint16 i = 0; i < length; i++) {
				y = dest / 320;
				x = dest - (y * 320);
				renderTarget->setPixel(x, y, aniFile.readByte());
				dest++;
			}
		}
		Graphics::Surface *aniImage = new Graphics::Surface();
		aniImage->create(128, 128, Graphics::PixelFormat::createFormatCLUT8());
		aniImage->copyRectToSurface(*renderTarget, 0, 0, Common::Rect((320/2)-64, (200/2)-64, (320/2)+64, (200/2)+64));
		images->push_back(*aniImage);
		renderTarget->free();
	}
	aniFile.close();
	return images;
}

void AlgGraphics::drawImage(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x;
	int32 dstY = y;
	Common::Rect subRect = Common::Rect(0, 0, src->w, src->h);
	if (dstX < 0) { subRect.left -= dstX; dstX=0; }
	if (dstY < 0) { subRect.top -= dstY; dstY=0; }
	if (dstX + src->w > dst->w) { subRect.right -= dstX + src->w - dst->w; }
	if (dstY + src->h > dst->h) { subRect.bottom -= dstY + src->h - dst->h; }
	dst->copyRectToSurfaceWithKey(src->getBasePtr(subRect.left, subRect.top), src->pitch, dstX, dstY, subRect.width(), subRect.height(), 0x00);
}

void AlgGraphics::drawImageCentered(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x - (src->w / 2);
	int32 dstY = y - (src->h / 2);
	Common::Rect subRect = Common::Rect(0, 0, src->w, src->h);
	if (dstX < 0) { subRect.left -= dstX; dstX=0; }
	if (dstY < 0) { subRect.top -= dstY; dstY=0; }
	if (dstX + src->w > dst->w) { subRect.right -= dstX + src->w - dst->w; }
	if (dstY + src->h > dst->h) { subRect.bottom -= dstY + src->h - dst->h; }
	dst->copyRectToSurfaceWithKey(src->getBasePtr(subRect.left, subRect.top), src->pitch, dstX, dstY, subRect.width(), subRect.height(), 0x00);
}

} // End of namespace Alg
