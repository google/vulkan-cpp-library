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
#ifndef _OPENVR_GLUT_OBJECT_H_
#define _OPENVR_GLUT_OBJECT_H_

#include <functional>

struct glut_object {
	typedef std::function<void()> void_callback_type;
	typedef void_callback_type draw_callback_type;
	typedef void_callback_type idle_callback_type;
	typedef std::function<void(int, int)> resize_callback_type;

	glut_object(const char *title, int width, int height);

	int run(const draw_callback_type &draw_callback,
		const idle_callback_type &idle_callback,
		const resize_callback_type &resize_callback);
};

#endif  // _OPENVR_GLUT_OBJECT_H_
