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

#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"
#include "common/rect.h"

#include "image/cel_3do.h"

#include "alg/alg.h"
#include "alg/graphics.h"

namespace Alg {

Graphics::Surface *AlgGraphics::loadVgaBackground(const Common::Path &path, Graphics::Palette *palette) {
	Common::File vgaFile;
	if (!vgaFile.open(path)) {
		error("AlgGraphics::loadVgaBackground(): Can't open background file '%s'", path.toString().c_str());
	}
	uint16 width = vgaFile.readUint16LE();
	uint16 height = vgaFile.readUint16LE();
	uint8 paletteEntries = vgaFile.readByte();
	uint8 paletteStart = vgaFile.readByte();
	assert(width >= 317 && width <= 320);
	assert(height == 200);
	assert(paletteStart == 0x10);
	for (uint32 i = paletteStart; i < (paletteStart + paletteEntries); i++) {
		byte r = vgaFile.readByte();
		byte g = vgaFile.readByte();
		byte b = vgaFile.readByte();
		palette->set(i, r, g, b);
	}
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	vgaFile.read(surface->getPixels(), width * height);
	Common::Rect backgroundRect = Common::Rect(0, 0, width, height);
	surface->flipVertical(backgroundRect);
	vgaFile.close();
	return surface;
}

// for "normal" ani images
Common::Array<Graphics::Surface *> *AlgGraphics::loadAniImage(const Common::Path &path, Graphics::Palette *palette) {
	Common::Array<Graphics::Surface *> *images = new Common::Array<Graphics::Surface *>();
	Common::File aniFile;
	if (!aniFile.open(path)) {
		error("AlgGraphics::loadAniImage(): Can't open image file '%s'", path.toString().c_str());
	}
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	for (uint32 i = paletteStart; i < (paletteStart + paletteEntries); i++) {
		byte r = aniFile.readByte();
		byte g = aniFile.readByte();
		byte b = aniFile.readByte();
		palette->set(i, r, g, b);
	}
	uint16 length, width, height;
	while (aniFile.pos() < aniFile.size()) {
		width = height = 0;
		uint64 aniSectionOffset = aniFile.pos();
		while (aniFile.pos() < aniFile.size()) {
			length = aniFile.readUint16LE();
			if (length == 0) {
				break;
			}
			width = length;
			height++;
			aniFile.skip(2 + length);
		}
		if (width > 0) {
			aniFile.seek(aniSectionOffset, SEEK_SET);
			Graphics::Surface *aniImage = new Graphics::Surface();
			aniImage->create(width, height, Graphics::PixelFormat::createFormatCLUT8());
			for (uint16 y = 0; y < height; y++) {
				aniFile.skip(4);
				aniFile.read(aniImage->getBasePtr(0, y), width);
			}
			images->push_back(aniImage);
		}
	}
	aniFile.close();
	return images;
}

// for ani images that use relative positioning.
// because these are meant to be drawn directly onto a 320x200 screen, they use relative offsets assuming that resolution.
// as we don't always want to draw directly to screen, we draw to the center of a virtual screen and then copy from a centered subrect.
Common::Array<Graphics::Surface *> *AlgGraphics::loadScreenCoordAniImage(const Common::Path &path, Graphics::Palette *palette) {
	Common::Array<Graphics::Surface *> *images = new Common::Array<Graphics::Surface *>();
	Common::File aniFile;
	if (!aniFile.open(path)) {
		error("AlgGraphics::loadScreenCoordAniImage(): Can't open image file '%s'", path.toString().c_str());
	}
	uint8 paletteEntries = aniFile.readByte();
	uint8 paletteStart = aniFile.readByte();
	for (uint32 i = paletteStart; i < (paletteStart + paletteEntries); i++) {
		byte r = aniFile.readByte();
		byte g = aniFile.readByte();
		byte b = aniFile.readByte();
		palette->set(i, r, g, b);
	}
	uint16 length = 0;
	int16 offset = 0;
	uint32 dest = 0;
	uint32 x = 0, y = 0;
	while (aniFile.pos() < aniFile.size()) {
		Graphics::Surface *renderTarget = new Graphics::Surface();
		renderTarget->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
		uint32 centerOffset = (renderTarget->w * renderTarget->h / 2) + (renderTarget->w / 2);
		while (true) {
			length = aniFile.readUint16LE();
			if (length == 0) {
				break;
			}
			offset = aniFile.readSint16LE();
			dest = centerOffset + offset;
			y = dest / renderTarget->w;
			x = dest - (y * renderTarget->w);
			aniFile.read(renderTarget->getBasePtr(x, y), length);
		}
		Graphics::Surface *aniImage = new Graphics::Surface();
		aniImage->create(96, 96, Graphics::PixelFormat::createFormatCLUT8());
		Common::Rect subSectionRect;
		subSectionRect.left = (renderTarget->w / 2) - (aniImage->w / 2);
		subSectionRect.top = (renderTarget->h / 2) - (aniImage->h / 2);
		subSectionRect.right = (renderTarget->w / 2) + (aniImage->w / 2);
		subSectionRect.bottom = (renderTarget->h / 2) + (aniImage->h / 2);
		aniImage->copyRectToSurface(*renderTarget, 0, 0, subSectionRect);
		images->push_back(aniImage);
		renderTarget->free();
		delete renderTarget;
	}
	aniFile.close();
	return images;
}

Graphics::Surface *AlgGraphics::load3doCelImage(const Common::Path &path) {
	Image::Cel3DODecoder decoder;
	Common::File celFile;
	if (!celFile.open(path)) {
		error("AlgGraphics::loadCelImage(): Can't open CEL file '%s'", path.toString().c_str());
	}
	if (!decoder.loadStream(celFile)) {
		error("AlgGraphics::loadCelImage(): Can't decode CEL file '%s'", path.toString().c_str());
	}
	Graphics::Surface *conv = decoder.getSurface()->convertTo(Graphics::PixelFormat::createFormatRGBA32());
	return conv;
}

Graphics::Surface *AlgGraphics::load3doImgImage(const Common::Path &path) {
	Common::File imgFile;
	if (!imgFile.open(path)) {
		error("AlgGraphics::load3doImgImage(): Can't open IMG file '%s'", path.toString().c_str());
	}

	uint32 imgTag = imgFile.readUint32BE();
	uint32 headerSize = imgFile.readUint32BE();
	uint32 width = imgFile.readUint32BE();
	uint32 height = imgFile.readUint32BE();
	uint32 bytesPerRow = imgFile.readUint32BE();
	uint8 bitsPerPixel = imgFile.readByte();
	uint8 numComponents = imgFile.readByte();
	uint8 numPlanes = imgFile.readByte();
	uint8 colorspace = imgFile.readByte();
	uint8 compressionType = imgFile.readByte();
	uint8 hvFormat = imgFile.readByte();
	uint8 pixelOrder = imgFile.readByte();
	uint8 version = imgFile.readByte();

	(void)imgTag;
	(void)headerSize;
	(void)bytesPerRow;
	(void)bitsPerPixel;
	(void)numComponents;
	(void)numPlanes;
	(void)colorspace;
	(void)compressionType;
	(void)hvFormat;
	(void)pixelOrder;
	(void)version;

	assert(imgTag == MKTAG('I','M','A','G'));
	assert(headerSize == 0x1C);
	assert(bytesPerRow == width * 2);
	assert(bitsPerPixel == 16);
	assert(numComponents == 3);
	assert(numPlanes == 1);
	assert(colorspace == 0);
	assert(compressionType == 0);
	assert(hvFormat == 0);
	assert(pixelOrder == 1);
	assert(version == 0);

	uint32 pdatTag = imgFile.readUint32BE();
	uint32 pdatSize = imgFile.readUint32BE();
	(void)pdatTag;
	assert(pdatTag == MKTAG('P','D','A','T'));
	Graphics::PixelFormat format(2, 5, 5, 5, 1, 10, 5, 0, 15);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, format);
	for (uint32 y = 0; y < height; y += 2) {
		for (uint32 x = 0; x < width; x += 2) {
			surface->setPixel(x, y, imgFile.readUint16BE());
			surface->setPixel(x, y + 1, imgFile.readUint16BE());
			surface->setPixel(x + 1, y, imgFile.readUint16BE());
			surface->setPixel(x + 1, y + 1, imgFile.readUint16BE());
		}
	}
	imgFile.read(surface->getPixels(), pdatSize - 8);
	Graphics::Surface *conv = surface->convertTo(Graphics::PixelFormat::createFormatRGBA32());
	imgFile.close();
	return conv;
}

Common::Array<Graphics::Surface *> *AlgGraphics::load3doFont(const Common::Path &path) {
	Common::File fontFile;
	if (!fontFile.open(path)) {
		error("AlgGraphics::load3doFont(): Can't open font file '%s'", path.toString().c_str());
	}

	uint16 tag1 = fontFile.readUint16BE();
	uint16 tag2 = fontFile.readUint16BE();
	uint32 widthTableOffset = fontFile.readUint32BE();
	uint32 pixelOffset = fontFile.readUint32BE();
	uint32 charWidth = fontFile.readUint32BE();
	uint32 charHeight = fontFile.readUint32BE();
	uint32 bitsPerPixel = fontFile.readUint32BE();
	uint32 numEntries = fontFile.readUint32BE();

	(void)tag1;
	(void)tag2;
	(void)widthTableOffset;
	(void)bitsPerPixel;

	assert(tag1 == 0x15);
	assert(tag2 == 0x24);
	assert(widthTableOffset == 0x1C);
	assert(bitsPerPixel == 4);

	Common::Array<uint8> widthTable;
	for (uint32 i = 0; i < numEntries; i++) {
		widthTable.push_back(fontFile.readByte());
	}

	// note: 0-7 are a scale from dark to bright, then 8-15 repeat another scale from dark to bright
	// note: index 1, 8, 9 and 13 seem to be never used
	uint8 palette[16] = {0x00, 0x00, 0xA5, 0xA5, 0xC6, 0xC6, 0xFF, 0xFF, 0x00, 0x00, 0xA5, 0xA5, 0xC6, 0xC6, 0xFF, 0xFF};

	fontFile.seek(pixelOffset, SEEK_SET);
	auto entries = new Common::Array<Graphics::Surface *>();
	for (uint16 i = 0; i < numEntries; i++) {
		Graphics::Surface *renderTarget = new Graphics::Surface();
		auto pixelFormat = Graphics::PixelFormat::createFormatRGBA32();
		renderTarget->create(charWidth * 2, charHeight, Graphics::PixelFormat::createFormatRGBA32());
		for (uint32 y = 0; y < charHeight; y++) {
			for (uint32 x = 0; x < charWidth * 2;) {
				byte tupel = fontFile.readByte();
				byte pixel1 = (tupel & 0xF0) >> 4;
				byte pixel2 = (tupel & 0x0F);
				uint32 rgbaPixel1 = pixelFormat.ARGBToColor((pixel1 == 0 ? 0x00 : 0xFF), palette[pixel1], palette[pixel1], palette[pixel1]);
				uint32 rgbaPixel2 = pixelFormat.ARGBToColor((pixel2 == 0 ? 0x00 : 0xFF), palette[pixel2], palette[pixel2], palette[pixel2]);
				renderTarget->setPixel(x, y, rgbaPixel1);
				renderTarget->setPixel(x + 1, y, rgbaPixel2);
				x += 2;
			}
		}
		uint8 targetWidth = widthTable[i];
		if (targetWidth > 0) {
			Graphics::Surface *surface = new Graphics::Surface();
			surface->create(targetWidth, charHeight, Graphics::PixelFormat::createFormatRGBA32());
			surface->copyRectToSurface(*renderTarget, 0, 0, Common::Rect(0, 0, targetWidth, charHeight));
			entries->push_back(surface);
		}
		renderTarget->free();
		delete renderTarget;
	}
	return entries;
}

void AlgGraphics::drawImage(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x;
	int32 dstY = y;
	Common::Rect subRect = Common::Rect(0, 0, src->w, src->h);
	if (dstX < 0) {
		subRect.left -= dstX;
		dstX = 0;
	}
	if (dstY < 0) {
		subRect.top -= dstY;
		dstY = 0;
	}
	if (dstX + src->w > dst->w) {
		subRect.right -= dstX + src->w - dst->w;
	}
	if (dstY + src->h > dst->h) {
		subRect.bottom -= dstY + src->h - dst->h;
	}
	dst->copyRectToSurfaceWithKey(*src, dstX, dstY, subRect, 0x00);
}

void AlgGraphics::drawImageCentered(Graphics::Surface *dst, Graphics::Surface *src, int32 x, int32 y) {
	int32 dstX = x - (src->w / 2);
	int32 dstY = y - (src->h / 2);
	drawImage(dst, src, dstX, dstY);
}

} // End of namespace Alg
