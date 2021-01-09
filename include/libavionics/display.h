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

/// Allocates memory for a new
av_display_t *av_display_new(unsigned width, unsigned height);

///
void av_display_delete(av_display_t *display);

///
void *av_display_get_back_buffer(av_display_t *display);

///
void av_display_finish_back_buffer(av_display_t *display);

///
void av_display_upload(av_display_t *display);

///
cairo_t *av_display_get_cairo(const av_display_t *display);

///
unsigned av_display_get_texture(const av_display_t *display);


#ifdef __cplusplus
} /* extern "C" */
#endif
