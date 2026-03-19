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

#ifndef ALG_3DO_VIDEO_DECODER_H
#define ALG_3DO_VIDEO_DECODER_H

#include "common/rect.h"
#include "audio/audiostream.h"
#include "image/codecs/cinepak.h"
#include "video/video_decoder.h"

namespace Alg {

class Alg3doVideoDecoder : public Video::VideoDecoder {
public:

	struct SampleIndex {
		uint32 offset;
		uint32 size;
	};

	Alg3doVideoDecoder();
	~Alg3doVideoDecoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;
	const Graphics::Surface *getFrame() { return _videoTrack->getSurface(); }
	void close() override;

protected:
	void readNextPacket() override;

private:
	uint32 _sampleStart = 0;
	Common::Array<SampleIndex> *_videoChunkSamples = nullptr;
	Common::Array<SampleIndex> *_audioChunkSamples = nullptr;

private:
	class StreamVideoTrack : public FixedRateVideoTrack  {
	public:
		StreamVideoTrack(uint32 width, uint32 height, uint32 frameRate, uint32 frameCount);
		~StreamVideoTrack() override;

		bool endOfTrack() const override;

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override;
		bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameCount; }
		// void setNextFrameStartTime(uint32 nextFrameStartTime) { _nextFrameStartTime = nextFrameStartTime; }
		// uint32 getNextFrameStartTime() const override { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame() override { return _surface; }
		const Graphics::Surface *getSurface() { return _surface; };

		void decodeFrame(Common::SeekableReadStream *stream);

	protected:
		Common::Rational getFrameRate() const { return _frameRate; }

	private:
		const Graphics::Surface *_surface = nullptr;
		Image::CinepakDecoder *_cinepakDecoder = nullptr;
		uint32 _frameRate = 0;
		uint32 _curFrame = 0;
		uint32 _frameCount = 0;
		uint32 _nextFrameStartTime = 0;
		uint16 _width = 0, _height = 0;
	};

	class StreamAudioTrack : public AudioTrack {
	public:
		StreamAudioTrack(uint32 sampleRate);
		~StreamAudioTrack() override;

		uint32 getTotalAudioQueued() const { return _totalAudioQueued; }
		void readAudioData(Common::SeekableReadStream *stream);

	protected:
		Audio::AudioStream *getAudioStream() const override { return _audioStream; };

	private:
		Audio::PacketizedAudioStream *_audioStream = nullptr;
		uint32 _totalAudioQueued; /* total amount of milliseconds of audio, that we queued up already */
		uint32 _sampleRate;
	};

	uint16 _currentAudioSampleIndex = 0;
	Common::SeekableReadStream *_stream;
	StreamVideoTrack *_videoTrack;
	StreamAudioTrack *_audioTrack;
};

} // End of namespace Sherlock

#endif
