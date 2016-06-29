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
#ifndef _OPENVR_GL_OBJECTS_H_
#define _OPENVR_GL_OBJECTS_H_

#include <cassert>
#include <GL/glew.h>
#include <vcc/util.h>

#ifdef NDEBUG
#define GLCHECK(expr) expr;
#else
#define GLCHECK(expr) expr; { GLenum err = glGetError(); \
	if (err != GL_NO_ERROR) { vcc::util::diagnostic_print( \
	__FILE__, __FUNCTION__, __LINE__, "Expression %s caused %s(0x%X)", #expr, \
	gl::error_string(err), err); } }
#endif // NDEBUG

namespace gl {

const char *error_string(GLenum err);

struct render_texture {

	render_texture() : instance(0) {}
	render_texture(render_texture &&copy) {
		instance = copy.instance;
		copy.instance = 0;
	}
	render_texture(const render_texture &) = delete;
	render_texture &operator=(render_texture &&copy) {
		destroy();
		instance = copy.instance;
		width = copy.width;
		height = copy.height;
		copy.instance = 0;
		return *this;
	}
	render_texture &operator=(const render_texture &) = delete;

	render_texture(GLint width, GLint height) : width(width), height(height) {
		GLCHECK(glGenTextures(1, &instance));
		assert(instance != 0);
		GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCHECK(glBindTexture(GL_TEXTURE_2D, instance));
		GLCHECK(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height));
		this->width = width;
		this->height = height;
	}

	~render_texture() {
		destroy();
	}

	GLuint get_instance() const {
		return instance;
	}

	GLint get_width() const {
		return width;
	}

	GLint get_height() const {
		return height;
	}

private:
	void destroy() {
		if (instance) {
			GLCHECK(glDeleteTextures(1, &instance));
		}
	}
	GLuint instance;
	GLint width, height;
};

struct framebuffer {
	framebuffer() : instance(0) {}

	framebuffer(const render_texture &texture) {
		GLCHECK(glGenFramebuffers(1, &instance));
		assert(instance != 0);
		GLCHECK(glBindFramebuffer(GL_FRAMEBUFFER, instance));
		GLCHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, texture.get_instance(), 0));
	}

	framebuffer(framebuffer &&copy) {
		instance = copy.instance;
		copy.instance = 0;
	}
	framebuffer(const framebuffer &) = delete;
	framebuffer &operator=(framebuffer &&copy) {
		destroy();
		instance = copy.instance;
		copy.instance = 0;
		return *this;
	}
	framebuffer &operator=(const framebuffer &) = delete;

	void complete(GLenum target) const {
#if !defined(NDEBUG)
		const GLenum status(glCheckFramebufferStatus(target));
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			VCC_PRINT("framebuffer(%u) status: 0x%X", instance, status);
			throw std::runtime_error("framebuffer not complete");
		}
#endif  // NDEBUG
	}

	void bind(GLenum target) const {
		GLCHECK(glBindFramebuffer(target, instance));
	}

	static void unbind(GLenum target) {
		GLCHECK(glBindFramebuffer(target, 0));
	}

	~framebuffer() {
		destroy();
	}

private:
	void destroy() {
		if (instance) {
			GLCHECK(glDeleteFramebuffers(1, &instance));
		}
	}

	GLuint instance;
};

}  // namespace gl

#endif // _OPENVR_GL_OBJECTS_H_
