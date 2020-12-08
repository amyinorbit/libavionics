//===--------------------------------------------------------------------------------------------===
// gl - OpenGL shortcut utils
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <string.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

GLuint gl_load_tex(const char *path, int *width, int *height);

GLuint gl_load_shader_file(const char *path, int type);
GLuint gl_load_shader(const char *source, int type);

GLuint gl_create_program(const char *vertex, const char *fragment);
GLuint gl_create_program_file(const char *vertex, const char *fragment);

void gl_ortho(float proj[16], uint32_t width, uint32_t height);
