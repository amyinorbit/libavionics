//===--------------------------------------------------------------------------------------------===
// display.h - MT, cairo backed rendering system
//
// Created by Amy Parent <developer@amyparent.com>
// Copyright (c) 2020 Amy Parent
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <pthread.h>
#include <cairo/cairo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct av_display_s av_display_t;

void av_display_init(double target_width, double target_height);
void av_display_deinit();

/// Creates a display manager.
av_display_t *av_display_new(unsigned width, unsigned height);

/// Deletes a display and its OpenGL resources.
void av_display_delete(av_display_t *display);

/// Copies the finished Cairo surface into the back PBO.
void av_display_finish_back_buffer(av_display_t *display);

/// Swaps buffers, and uploads the new front buffer as a texture.
void av_display_upload(av_display_t *display);

/// Returns a cairo context to draw onto the display.
cairo_t *av_display_get_cairo(const av_display_t *display);

/// 
unsigned av_display_get_texture(const av_display_t *display);

void av_display_render_gl(av_display_t *display, double x, double y, float brightness);

#ifdef __cplusplus
} /* extern "C" */
#endif
