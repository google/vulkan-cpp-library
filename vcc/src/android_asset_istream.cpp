/*
 * Copyright 2016 Google Inc. All Rights Reserved.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cassert>
#include <vcc/android_asset_istream.h>
#include <vcc/util.h>

namespace android {
namespace internal {

typedef std::unique_ptr<AAsset, decltype(&AAsset_close)> asset_type;

struct stream_asset_istreambuf : public std::streambuf {

	stream_asset_istreambuf(asset_type &&asset, size_t buff_sz = 256,
			size_t put_back = 8)
			: asset(std::forward<asset_type>(asset)),
				put_back(std::max(put_back, size_t(1))),
				buffer(std::max(buff_sz, put_back) + put_back) {
		reset();
	}

	virtual ~stream_asset_istreambuf() {}

	void reset() {
		char *const end = buffer.data();
    setg(end, end, end);
  }

	std::streambuf::int_type underflow() {
		if (gptr() < egptr())
			return traits_type::to_int_type(*gptr());

		char *const base = buffer.data();
		char *start = base;

		if (eback() == base) {
				std::memmove(base, egptr() - put_back, put_back);
				start += put_back;
		}

		const int n(AAsset_read(asset.get(), start,
			buffer.size() - (start - base)));
		if (n == 0)
			return traits_type::eof();
		else if (n < 0) {
			std::stringstream ss;
			ss << "error while streaming: \"" << n << "\"";
			throw std::runtime_error(ss.str());
		}

		setg(base, start, start + n);

		return traits_type::to_int_type(*gptr());
	}

	std::streampos seekoff(std::streamoff off,
			std::ios_base::seekdir way, std::ios_base::openmode which) {
		assert(which == std::ios_base::in);
		int whence;
		switch (way) {
			case std::ios_base::beg:
				whence = SEEK_SET;
				break;
			case std::ios_base::cur:
				whence = SEEK_CUR;
				break;
			case std::ios_base::end:
				whence = SEEK_END;
				break;
			default:
				assert(!"invalid seekdir");
				break;
		}
		const int pos(AAsset_seek(asset.get(), off, whence));
		assert(pos != -1);
		reset();
		return std::streampos(pos);
	}

	std::streampos seekpos(std::streampos sp, std::ios_base::openmode which) {
		assert(which == std::ios_base::in);
		const int pos(AAsset_seek(asset.get(), sp, SEEK_SET));
		assert(pos != -1);
		reset();
		return std::streampos(pos);
	}

private:
	asset_type asset;
	const size_t put_back;
  std::vector<char> buffer;
};

struct map_asset_istreambuf : public std::streambuf {

	map_asset_istreambuf(asset_type &&asset, const char *start, size_t size)
			: asset(std::forward<asset_type>(asset)), start(start),
				end(start + size), current(start) {}

	virtual ~map_asset_istreambuf() {}

	std::streambuf::int_type underflow() {
		return current < end
			? traits_type::to_int_type(*current) : traits_type::eof();
	}

	std::streambuf::int_type uflow() {
		return current < end
			? traits_type::to_int_type(*current++) : traits_type::eof();
	}

	std::streampos seekoff(std::streamoff off,
			std::ios_base::seekdir way, std::ios_base::openmode which) {
		assert(which == std::ios_base::in);
		current = current + off;
		return std::streampos(current - start);
	}

	std::streampos seekpos(std::streampos sp, std::ios_base::openmode which) {
		assert(which == std::ios_base::in);
		current = start + sp;
		return std::streampos(sp);
	}

private:
	asset_type asset;
	const char *const start, *const end;
	const char *current;
};

std::unique_ptr<std::streambuf> make_asset_istreambuf(AAssetManager *mgr,
		const char *filename) {
	asset_type asset(
				AAssetManager_open(mgr, filename, AASSET_MODE_STREAMING),
				&AAsset_close);
	if (!asset.get()) {
		std::stringstream ss;
		ss << "failed to open \"" << filename << "\" for streaming";
		std::string string(ss.str());
		VCC_PRINT("%s", string.c_str());
		throw std::runtime_error(std::move(string));
	}
	const char *const buffer((const char *) AAsset_getBuffer(asset.get()));
	return std::unique_ptr<std::streambuf>(buffer
			? (std::streambuf *) new map_asset_istreambuf(std::move(asset), buffer,
					AAsset_getLength(asset.get()))
			: (std::streambuf *) new stream_asset_istreambuf(std::move(asset)));
}

}  // namespace internal
}  // namespace android

