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

#ifndef ALG_ALG_VIDEO_DECODER_H
#define ALG_ALG_VIDEO_DECODER_H

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "video/video_decoder.h"

#include "common/file.h"

namespace Alg {

class AlgVideoDecoder : public Video::VideoDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	virtual void readNextPacket() override;
	const Graphics::Surface *getFrame() { return _videoTrack->getSurface(); }
	void setPalette(Graphics::Palette *palette) { _videoTrack->setPalette(palette); }
	bool hasDirtyPalette() const { return _videoTrack->hasDirtyPalette(); }
	void setDirtyPalette(bool dirty) const { _videoTrack->setDirtyPalette(dirty); }
	virtual bool endOfVideo() const { return _stream->eos() || (_stream->pos() >= _stream->size()); }
	void skipNumberOfFrames(uint32 num);

private:
	class AlgVideoTrack : public FixedRateVideoTrack {
	public:
		AlgVideoTrack(uint16 width, uint16 height, uint16 frameRate);
		~AlgVideoTrack();

		bool isRewindable() const { return false; }

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const { return _surface->format;}
		int getCurFrame() const { return _curFrame; }
		void setPalette(Graphics::Palette *palette) { _palette = palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }
		const Graphics::Surface *decodeNextFrame() override { return _surface; }
		const Graphics::Surface *getSurface() { return _surface; };
		void setDirtyPalette(bool dirty) const { _dirtyPalette = dirty; }
		virtual bool endOfTrack() const override { return false; }

		void decodeIntraFrame(Common::SeekableReadStream *stream, uint8 hh, uint8 hv);
		void decodeInterFrame(Common::SeekableReadStream *stream, uint8 hh, uint8 hv);
		void updatePalette(Common::SeekableReadStream *stream, bool partial);

	protected:
		Common::Rational getFrameRate() const { return _frameRate; }

	private:
		Graphics::Surface *_surface = nullptr;
		uint16 _width = 0;
		uint16 _height = 0;
		Graphics::Palette *_palette = nullptr;
		mutable bool _dirtyPalette = false;
		int _curFrame = 0;
		uint32 _frameRate = 0;
	};

	class AlgAudioTrack : public AudioTrack {
	public:
		AlgAudioTrack(uint16 sampleRate);
		~AlgAudioTrack();
		bool isRewindable() const { return false; }
		Audio::AudioStream *getAudioStream() const override { return _audioStream; }

		void readAudioData(Common::SeekableReadStream *stream, uint16 rate);

	private:
		Audio::PacketizedAudioStream *_audioStream = nullptr;
	};

	Common::SeekableReadStream *_stream;

	AlgVideoTrack *_videoTrack = nullptr;
	AlgAudioTrack *_audioTrack = nullptr;
};

} // End of namespace Alg

#endif
