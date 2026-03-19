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

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/decoders/raw.h"

#include "alg/alg_3do_decoder.h"

namespace Alg {

Alg3doVideoDecoder::Alg3doVideoDecoder()
	: _stream(0), _videoTrack(0) {
	_sampleStart = 0;
	_videoChunkSamples = new Common::Array<SampleIndex>();
	_audioChunkSamples = new Common::Array<SampleIndex>();
}

Alg3doVideoDecoder::~Alg3doVideoDecoder() {
	close();
	delete _videoChunkSamples;
	delete _audioChunkSamples;
}

bool Alg3doVideoDecoder::loadStream(Common::SeekableReadStream *stream) {

	uint32 audioSampleRate = 22050;
	uint32 frameRate = 12;

	close();
	_videoChunkSamples->clear();
	_audioChunkSamples->clear();
	_stream = stream;
	_sampleStart = 0;

	uint32 filmChunkTag = _stream->readUint32BE();
	_sampleStart = _stream->readUint32BE();
	_stream->skip(8); // Unknown, always 0x00
	uint32 fdscChunkTag = _stream->readUint32BE();
	_stream->skip(4); // fdscChunkSize
	uint32 videoCodecTag = _stream->readUint32BE();
	uint32 videoHeight = _stream->readUint32BE();
	uint32 videoWidth = _stream->readUint32BE();
	uint32 stabChunkTag = _stream->readUint32BE();
	_stream->skip(4); // stabChunkSize
	_stream->skip(4); // baseClock
	uint32 sampleEntryCount = _stream->readUint32BE(); // sample entry size

	(void)filmChunkTag;
	(void)fdscChunkTag;
	(void)videoCodecTag;
	(void)stabChunkTag;

	assert(filmChunkTag == MKTAG('F', 'I', 'L', 'M'));
	assert(fdscChunkTag == MKTAG('F', 'D', 'S', 'C'));
	assert(videoCodecTag == MKTAG('c', 'v', 'i', 'd'));
	assert(videoHeight == 240);
	assert(videoWidth == 320);
	assert(stabChunkTag == MKTAG('S', 'T', 'A', 'B'));

	for(uint32 i = 0; i < sampleEntryCount; i++) {
		SampleIndex index;
		index.offset = _stream->readUint32BE();
		index.size = _stream->readUint32BE();
		uint32 sampleInfo1 = _stream->readUint32BE();
		uint32 sampleInfo2 = _stream->readUint32BE();
		(void)sampleInfo2;
		if (sampleInfo1 == 0xFFFFFFFF) {
			// audio chunk
			_audioChunkSamples->push_back(index);
		} else {
			// video chunk
			_videoChunkSamples->push_back(index);
		}
	}

	_videoTrack = new StreamVideoTrack(videoWidth, videoHeight, frameRate, _videoChunkSamples->size());
	addTrack(_videoTrack);

	_audioTrack = new StreamAudioTrack(audioSampleRate);
	addTrack(_audioTrack);

	// TODO FIXME HACK: fully queue all available audio
	for (uint32 i = 0; i < _audioChunkSamples->size(); i++) {
		SampleIndex index = (*_audioChunkSamples)[i];
		_stream->seek(_sampleStart + index.offset, SEEK_SET);
		_audioTrack->readAudioData(_stream->readStream(index.size));
		_currentAudioSampleIndex++;
	}

	return true;
}

void Alg3doVideoDecoder::close() {
	Video::VideoDecoder::close();

	delete _stream; _stream = 0;
	_videoTrack = 0;
	_audioTrack = 0;
	_currentAudioSampleIndex = 0;
}

void Alg3doVideoDecoder::readNextPacket() {
	uint32 curFrame = _videoTrack->getCurFrame();
	uint32 curAudioSampleIndex = _videoTrack->getCurFrame();
	if (curFrame < _videoChunkSamples->size()) {
		SampleIndex index = (*_videoChunkSamples)[curFrame];
		_stream->seek(_sampleStart + index.offset, SEEK_SET);
		_videoTrack->decodeFrame(_stream->readStream(index.size));
	}
}

Alg3doVideoDecoder::StreamVideoTrack::StreamVideoTrack(uint32 width, uint32 height, uint32 frameRate, uint32 frameCount) {
	_cinepakDecoder = new Image::CinepakDecoder(32);
	_width = width;
	_height = height;
	_frameRate = frameRate;
	_frameCount = frameCount;
}

Alg3doVideoDecoder::StreamVideoTrack::~StreamVideoTrack() {
	delete _cinepakDecoder;
}

bool Alg3doVideoDecoder::StreamVideoTrack::endOfTrack() const {
	return getCurFrame() >= getFrameCount() - 1;
}

Graphics::PixelFormat Alg3doVideoDecoder::StreamVideoTrack::getPixelFormat() const {
	return _cinepakDecoder->getPixelFormat();
}

bool Alg3doVideoDecoder::StreamVideoTrack::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	return _cinepakDecoder->setOutputPixelFormat(format);
}

void Alg3doVideoDecoder::StreamVideoTrack::decodeFrame(Common::SeekableReadStream *stream) {
	_surface = _cinepakDecoder->decodeFrame(*stream);
	_curFrame++;
}

Alg3doVideoDecoder::StreamAudioTrack::StreamAudioTrack(uint32 rate) : AudioTrack(Audio::Mixer::SoundType::kPlainSoundType) {
	setRate(rate);
	_sampleRate = rate;
	_audioStream = Audio::makePacketizedRawStream(rate, 0);
}

Alg3doVideoDecoder::StreamAudioTrack::~StreamAudioTrack() {
	delete _audioStream;
}

void Alg3doVideoDecoder::StreamAudioTrack::readAudioData(Common::SeekableReadStream *stream) {
	_audioStream->queuePacket(stream);
	uint32 audioLengthMSecs = (uint32)((stream->size() / (double)_sampleRate) * 1000.0);
	_totalAudioQueued += audioLengthMSecs;
}

} // End of namespace Video
