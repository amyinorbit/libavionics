//===--------------------------------------------------------------------------------------------===
// display.h - Private data tructures required for av_display
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <libavionics/display.h>
#include <libavionics/gl.h>
#include <ccore/log.h>
#include <ccore/math.h>
#include <XPLMGraphics.h>
// #include "glad.h"

typedef struct renderer_t {
    unsigned quad_vbo;
    unsigned quad_ibo;
    unsigned quad_shader;
    
    struct {
        int vtx_pos;
        int vtx_tex0;
        int pvm;
        int tex;
        int alpha;
    } loc;
    
    vec2_t last_pos;
    vec2_t last_size;
} renderer_t;

struct av_display_s {
    unsigned width, height;
    unsigned texture;

    bool is_back_ready;
    pthread_mutex_t mt;
    unsigned front, back;
    unsigned pbos[2];
    void *current;
    cairo_surface_t *surface;
    cairo_t *cairo;
    
    renderer_t renderer;
    // OpenGL renderer
};

struct av_target_s {
    vec2_t size;
    float proj[16];
};

static inline void check_gl(const char *where, int line) {
    GLenum error = glGetError();
    if(error == GL_NO_ERROR) return;
    cc_log(LOG_WARN, where, "OpenGL error code 0x%04x line %d", error, line);
}

#ifdef GL_DEBUG
#define CHECK_GL() check_gl(__FUNCTION__, __LINE__)
#else
#define CHECK_GL()
#endif

void renderer_init(renderer_t *r);
void renderer_deinit(renderer_t *r);
