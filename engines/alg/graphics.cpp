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

	debug("loading ANI file: %s", path.toString().c_str());
	Common::File aniFile;
	aniFile.open(path);
	assert(aniFile.isOpen());

	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	assert(paletteEntries == 0x70);
	assert(paletteStart == 0x10);
	// ignore palette data
	aniFile.skip(paletteEntries * 3);

	uint16 width = 0, height = 0, tempWidth = 0, section = 0, tempOffset = 0;
	uint8 *temp = new uint8[64 * 64]();

	while(aniFile.pos() < aniFile.size()) {
		tempWidth = aniFile.readUint16LE();
		if(tempWidth == 0) {
			// end of section
			Graphics::Surface *aniImage = new Graphics::Surface();
			uint8 *pixels = new uint8[width * height]();
			memcpy(pixels, temp, width * height);
			aniImage->init(width, height, width, pixels, Graphics::PixelFormat::createFormatCLUT8());
			images->push_back(*aniImage);

			section++;
			height = 0;
			tempOffset = 0;
			continue;
		}
		aniFile.skip(2);
		width = tempWidth;
		for(uint32 i = 0; i < width; i++) {
			temp[tempOffset++] = aniFile.readByte();
		}
		height++;
	}

	aniFile.close();
	delete[] temp;
	return images;
}

Common::Array<Graphics::Surface> * AlgGraphics::loadTransparentAniImage(const Common::Path &path) {
	Common::Array<Graphics::Surface> *images = new Common::Array<Graphics::Surface>();

	uint8 bufferWidth = 64, bufferHeight = 64;

	debug("loading transparent ANI file: %s", path.toString().c_str());
	Common::File aniFile;
	aniFile.open(path);
	assert(aniFile.isOpen());

	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	assert(paletteEntries == 0x70);
	assert(paletteStart == 0x10);
	// ignore palette data
	aniFile.skip(paletteEntries * 3);

	uint16 length = 0, section = 0, line = 0;
	uint16 startOffset = 0, writeOffset = 0;
	uint8 *temp = new uint8[bufferWidth * bufferHeight]();

	startOffset = ((bufferHeight / 2) * bufferWidth) + bufferWidth / 2;
	aniFile.skip(3);
	int8 startOffsetY = aniFile.readSByte();
	aniFile.seek(-4, SEEK_CUR);
	startOffset += startOffsetY * bufferWidth;

	while(aniFile.pos() < aniFile.size() - 1) {
		length = aniFile.readUint16LE();
		if(length == 0) {
			// end of section
			Graphics::Surface *aniImage = new Graphics::Surface();
			uint8 *pixels = new uint8[bufferWidth * bufferHeight]();
			memcpy(pixels, temp, bufferWidth * bufferHeight);
			aniImage->init(bufferWidth, bufferHeight, bufferWidth, pixels, Graphics::PixelFormat::createFormatCLUT8());
			images->push_back(*aniImage);

			section++;
			line = 0;
			memset(temp, 0, bufferWidth * bufferHeight);

			startOffset = ((bufferHeight / 2) * bufferWidth) + bufferWidth / 2;
			aniFile.skip(3);
			startOffsetY = aniFile.readSByte();
			aniFile.seek(-4, SEEK_CUR);
			startOffset += startOffsetY * bufferWidth;

			continue;
		}

		int32 pixelOffsetX = aniFile.readSByte();
		aniFile.skip(1);
		// properly align pixelOffsetX
		while(pixelOffsetX > 0) {
			pixelOffsetX -= 64;
		}
		while(pixelOffsetX <= -64) {
			pixelOffsetX += 64;
		}

		writeOffset = startOffset + ((startOffsetY + line) * 64) + pixelOffsetX;
		for(uint32 i = 0; i < length; i++) {
			temp[writeOffset++] = aniFile.readByte();
		}
		line++;
	}

	aniFile.close();
	delete[] temp;

	return images;
}

} // End of namespace Alg
