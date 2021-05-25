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
void gl_ortho(float proj[16], uint32_t width, uint32_t height);

typedef struct renderer_t renderer_t;

// renderer_t *render_new(unsigned width, unsigned height);
// void render_delete(renderer_t *r);
// void render_quad(
//     renderer_t *r,
//     unsigned tex,
//     unsigned x,
//     unsigned y,
//     unsigned w,
//     unsigned h,
//     float brightness
// );
