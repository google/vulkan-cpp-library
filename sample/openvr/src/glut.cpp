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
#include <GL/glew.h>
#include <glut_object.h>
#include <GL/freeglut.h>
#include <vcc/util.h>

namespace {

glut_object::draw_callback_type draw_callback;
glut_object::idle_callback_type idle_callback;
glut_object::resize_callback_type resize_callback;

void draw() {
	draw_callback();
	glutSwapBuffers();
	glutPostRedisplay();
}

void idle() {
	idle_callback();
}

void resize(int width, int height) {
	resize_callback(width, height);
}

}  // anonymous namespace

glut_object::glut_object(int &argc, char **argv, const char *title, int width, int height) {

	glutInitWindowSize(width, height);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutCreateWindow(title);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		VCC_PRINT("Error: %s\n", glewGetErrorString(err));
		throw std::runtime_error("glewInit failed");
	}
}

int glut_object::run(const glut_object::draw_callback_type &draw_callback,
	const glut_object::idle_callback_type &idle_callback,
	const glut_object::resize_callback_type &resize_callback) {
	::draw_callback = draw_callback;
	::idle_callback = idle_callback;
	::resize_callback = resize_callback;

	glutDisplayFunc(draw);
	glutIdleFunc(idle);
	glutReshapeFunc(resize);

	glutMainLoop();
	return 0;
}
