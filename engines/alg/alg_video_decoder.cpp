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

#include "common/textconsole.h"
#include "graphics/surface.h"
#include "audio/decoders/raw.h"

#include "alg/alg_video_decoder.h"

namespace Alg {

bool AlgVideoDecoder::loadStream(Common::SeekableReadStream *stream) {
	_stream = stream;
	uint16 chunkType = stream->readUint16LE();
	uint32 chunkSize = stream->readUint32LE();
	uint16 numChunks = stream->readUint16LE();
	uint16 frameRate = stream->readUint16LE();
	uint16 videoMode = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();
	uint16 typeRaw = stream->readUint16LE();
	uint16 typeInter = stream->readUint16LE();
	uint16 typeIntraHh = stream->readUint16LE();
	uint16 typeInterHh = stream->readUint16LE();
	uint16 typeIntraHhv = stream->readUint16LE();
	uint16 typeInterHhv = stream->readUint16LE();
	uint16 audioType = 0;
	(void)chunkType;
	(void)numChunks;
	(void)videoMode;
	(void)typeRaw;
	(void)typeInter;
	(void)typeIntraHh;
	(void)typeInterHh;
	(void)typeIntraHhv;
	(void)typeInterHhv;
	if (chunkSize == 0x18) {
		audioType = stream->readUint16LE();
		assert(audioType == 21);
	}
	assert(chunkType == 0x00);
	assert(chunkSize == 0x16 || chunkSize == 0x18);
	assert(frameRate == 10);
	assert(videoMode == 0x13);
	assert(typeRaw == 0x02);
	assert(typeInter == 0x05);
	assert(typeIntraHh == 0x0c);
	assert(typeInterHh == 0x0d);
	assert(typeIntraHhv == 0x0e);
	assert(typeInterHhv == 0x0f);

	_videoTrack = new AlgVideoTrack(width, height, frameRate);
	addTrack(_videoTrack);

	if (audioType != 0) {
		_audioTrack = new AlgAudioTrack(8000);
		addTrack(_audioTrack);
	}

	return true;
}

void AlgVideoDecoder::skipNumberOfFrames(uint32 num) {
	uint32 videoFramesSkipped = 0;
	while (videoFramesSkipped < num && _stream->pos() < _stream->size()) {
		uint16 chunkType = _stream->readUint16LE();
		uint32 chunkSize = _stream->readUint32LE();
		switch (chunkType) {
		case MKTAG16(0x00, 0x08):
		case MKTAG16(0x00, 0x0c):
		case MKTAG16(0x00, 0x0e):
		case MKTAG16(0x00, 0x05):
		case MKTAG16(0x00, 0x0d):
		case MKTAG16(0x00, 0x0f):
		case MKTAG16(0x00, 0x02):
			videoFramesSkipped++;
			break;
		}
		_stream->skip(chunkSize);
	}
	// find next keyframe
	bool nextKeyframeFound = false;
	while (!nextKeyframeFound && _stream->pos() < _stream->size()) {
		uint16 chunkType = _stream->readUint16LE();
		uint32 chunkSize = _stream->readUint32LE();
		switch (chunkType) {
		case MKTAG16(0x00, 0x08):
		case MKTAG16(0x00, 0x0c):
		case MKTAG16(0x00, 0x0e):
			nextKeyframeFound = true;
			_stream->seek(-6, SEEK_CUR);
			break;
		case MKTAG16(0x00, 0x05):
		case MKTAG16(0x00, 0x0d):
		case MKTAG16(0x00, 0x0f):
		case MKTAG16(0x00, 0x02):
			_stream->skip(chunkSize);
			videoFramesSkipped++;
			break;
		default:
			_stream->skip(chunkSize);
		}
	}
}

AlgVideoDecoder::AlgVideoTrack::AlgVideoTrack(uint16 width, uint16 height, uint16 frameRate) {
	_curFrame = -1;
	_width = width;
	_height = height;
	_frameRate = frameRate;

	_surface = new Graphics::Surface();
	_surface->create(_width, _height, Graphics::PixelFormat::createFormatCLUT8());
}

AlgVideoDecoder::AlgVideoTrack::~AlgVideoTrack() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

void AlgVideoDecoder::readNextPacket() {
	bool gotVideoFrame = false;
	while (!gotVideoFrame && !_stream->eos() && _stream->pos() < _stream->size()) {
		uint16 chunkType = _stream->readUint16LE();
		uint32 chunkSize = _stream->readUint32LE();
		switch (chunkType) {
		case MKTAG16(0x00, 0x00):
			error("AlgVideoDecoder::readNextChunk(): got repeated header chunk");
			break;
		case MKTAG16(0x00, 0x30):
			_videoTrack->updatePalette(_stream->readStream(chunkSize), false);
			break;
		case MKTAG16(0x00, 0x31):
			_videoTrack->updatePalette(_stream->readStream(chunkSize), true);
			break;
		case MKTAG16(0x00, 0x15):
			_audioTrack->readAudioData(_stream->readStream(chunkSize), 8000);
			break;
		case MKTAG16(0x00, 0x16):
			_audioTrack->readAudioData(_stream->readStream(chunkSize), 11000);
			break;
		case MKTAG16(0x00, 0x08):
			_videoTrack->decodeIntraFrame(_stream->readStream(chunkSize), 0, 0);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x0c):
			_videoTrack->decodeIntraFrame(_stream->readStream(chunkSize), 1, 0);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x0e):
			_videoTrack->decodeIntraFrame(_stream->readStream(chunkSize), 1, 1);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x05):
			_videoTrack->decodeInterFrame(_stream->readStream(chunkSize), 0, 0);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x0d):
			_videoTrack->decodeInterFrame(_stream->readStream(chunkSize), 1, 0);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x0f):
			_videoTrack->decodeInterFrame(_stream->readStream(chunkSize), 1, 1);
			gotVideoFrame = true;
			break;
		case MKTAG16(0x00, 0x02):
			warning("AlgVideoDecoder::readNextChunk(): raw video not supported");
			_stream->skip(chunkSize);
			break;
		default:
			error("AlgVideoDecoder::readNextChunk(): Unknown chunk encountered: %d", chunkType);
		}
	}
}

void AlgVideoDecoder::AlgVideoTrack::decodeIntraFrame(Common::SeekableReadStream *stream, uint8 hh, uint8 hv) {
	uint16 x = 0, y = 0;
	int32 bytesRemaining = stream->size();
	int32 runLength = 0;
	uint8 readByte, color = 0;
	while (bytesRemaining > 0) {
		readByte = stream->readByte();
		if (readByte & 0x80) {
			runLength = 1;
			color = readByte;
			bytesRemaining--;
		} else {
			runLength = (readByte & 0x7F) + 2;
			color = stream->readByte();
			bytesRemaining -= 2;
		}
		if (color > 0) {
			memset(_surface->getBasePtr(x, y), color, runLength * (1 + hh));
			if (hv) {
				memset(_surface->getBasePtr(x, y + 1), color, runLength * (1 + hh));
			}
		}
		x += runLength + (hh * runLength);
		if (x >= _width) {
			x = 0;
			y += 1 + hv;
		}
	}
	assert(bytesRemaining == 0);
	(void)bytesRemaining;
	_curFrame++;
}

void AlgVideoDecoder::AlgVideoTrack::decodeInterFrame(Common::SeekableReadStream *stream, uint8 hh, uint8 hv) {
	uint32 bytesRead = 0;
	uint16 length = 0, x = 0, y = 0, replacementBytesLeft = 0;
	replacementBytesLeft = stream->readUint16LE();
	bytesRead += 2;
	if (replacementBytesLeft == 0) {
		stream->skip(stream->size() - 2);
		return;
	}
	Common::SeekableReadStream *replacement = stream->readStream(replacementBytesLeft);
	bytesRead += replacementBytesLeft;
	while (replacementBytesLeft > 1) {
		length = replacement->readByte();
		x = replacement->readByte() + ((length & 0x80) << 1);
		length &= 0x7F;
		replacementBytesLeft -= 2;
		if (length == 0) {
			y += x;
			continue;
		}
		for (uint32 i = 0; i < length; i++) {
			uint8 replaceArray = replacement->readByte();
			for (uint8 j = 0x80; j > 0; j = j >> 1) {
				if (replaceArray & j) {
					uint8 color = stream->readByte();
					bytesRead++;
					memset(_surface->getBasePtr(x, y), color, (1 + hh));
					if (hv) {
						memset(_surface->getBasePtr(x, y + 1), color, (1 + hh));
					}
				}
				x += 1 + hh;
			}
		}
		y += 1 + hv;
	}
	delete replacement;
	assert(bytesRead == stream->size());
	(void)bytesRead;
	_curFrame++;
}

void AlgVideoDecoder::AlgVideoTrack::updatePalette(Common::SeekableReadStream *stream, bool partial) {
	_dirtyPalette = true;
	uint32 bytesRead = 0;
	uint16 offset = 0, count = 256;
	if (partial) {
		offset = stream->readUint16LE();
		count = stream->readUint16LE();
		bytesRead += 4;
	}
	for (uint16 i = 0; i < count; i++) {
		uint8 r = stream->readByte() * 4;
		uint8 g = stream->readByte() * 4;
		uint8 b = stream->readByte() * 4;
		_palette->set(offset, r, g, b);
		bytesRead += 3;
		offset++;
	}
	assert(bytesRead == stream->size());
	(void)bytesRead;
}

AlgVideoDecoder::AlgAudioTrack::AlgAudioTrack(uint16 rate) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType) {
	setRate(rate);
	_audioStream = makePacketizedRawStream(rate, Audio::FLAG_UNSIGNED);
}

AlgVideoDecoder::AlgAudioTrack::~AlgAudioTrack() {
	delete _audioStream;
}

void AlgVideoDecoder::AlgAudioTrack::readAudioData(Common::SeekableReadStream *stream, uint16 rate) {
	_audioStream->queuePacket(stream);
}

} // End of namespace Alg
