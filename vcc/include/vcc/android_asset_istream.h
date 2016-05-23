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
#ifndef _ANDROID_ASSET_ISTREAM_H_
#define _ANDROID_ASSET_ISTREAM_H_

#if defined(__ANDROID__) || defined(ANDROID)

#include <android/asset_manager.h>
#include <cstring>
#include <exception>
#include <memory>
#include <sstream>
#include <vector>

namespace android {
namespace internal {

std::unique_ptr<std::streambuf> make_asset_istreambuf(
	AAssetManager *mgr, const char *filename);

struct asset_istream_base {
	asset_istream_base(AAssetManager *mgr, const char *filename)
		: streambuf(std::unique_ptr<std::streambuf>(make_asset_istreambuf(
				mgr, filename))) {}
	std::unique_ptr<std::streambuf> streambuf;
};

}  // namespace internal

struct asset_istream
	: private internal::asset_istream_base, public std::istream {

	asset_istream(AAssetManager *mgr, const char *filename)
		: internal::asset_istream_base(mgr, filename),
			std::istream(streambuf.get()) {}
};

}  // namespace android

#endif // __ANDROID__

#endif  // _ANDROID_ASSET_ISTREAM_H_

