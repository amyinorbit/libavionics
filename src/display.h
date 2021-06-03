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
#include <libavionics/renderer.h>
#include <libavionics/gl.h>
#include <ccore/log.h>
#include <ccore/math.h>
#include <XPLMGraphics.h>
// #include "glad.h"

struct av_quad_s {
    unsigned vbo;
    unsigned ibo;
    unsigned shader;
    unsigned tex;
    
    struct {
        int vtx_pos;
        int vtx_tex0;
        int pvm;
        int tex;
        int alpha;
    } loc;
    
    vec2_t last_pos;
    vec2_t last_size;
};

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
    
    av_quad_t quad;
    // OpenGL renderer
};

struct av_target_s {
    vec2_t size;
    vec2_t offset;
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

void av_quad_init(av_quad_t *r, unsigned texture, unsigned shader);
void av_quad_deinit(av_quad_t *r);
