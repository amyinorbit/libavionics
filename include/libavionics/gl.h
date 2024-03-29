//===--------------------------------------------------------------------------------------------===
// gl.h - GL defines and stuff
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include "glad.h"
#include <stdint.h>
#include <stdbool.h>

#if IBM
#include <GL/gl.h>
// #include <GL/glu.h>
#elif LIN
#define TRUE 1
#define FALSE 0
#include <GL/gl.h>
// #include <GL/glu.h>
#else
#define TRUE 1
#define FALSE 0
#if __GNUC__
#include <OpenGL/gl.h>
// #include <OpenGL/glu.h>
#else
#include <gl.h>
// #include <glu.h>
#endif
#include <string.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

GLuint gl_create_program(const char *vertex, const char *fragment);
GLuint gl_load_shader(const char *source, int type);
GLuint gl_load_tex(const char *path, int *w, int *h);
void gl_ortho(float proj[16], float x, float y, float width, float height);
