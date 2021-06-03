//===--------------------------------------------------------------------------------------------===
// renderer.h - OpenGL renderer utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <ccore/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct av_quad_s av_quad_t;
typedef struct av_target_s av_target_t;

av_quad_t *av_quad_new(unsigned texture, unsigned shader);
void av_quad_delete(av_quad_t *quad);

av_target_t *av_target_new(double x, double y, double width, double height);
void av_target_set_size(av_target_t *target, double width, double height);
void av_target_set_offset(av_target_t *target, double x, double y);
void av_target_delete();

void av_render_init();
void av_render_deinit();
void av_render_quad(av_target_t *target, av_quad_t *quad, vec2_t pos, vec2_t size, double alpha);

#ifdef __cplusplus
} // extern "C"
#endif

