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

typedef struct mfd_display_s mfd_display_t;

/// Allocates memory for a new
mfd_display_t *display_new(unsigned width, unsigned height);

///
void display_delete(mfd_display_t *display);

///
void *display_get_back_buffer(mfd_display_t *display);

///
void display_finish_back_buffer(mfd_display_t *display);

///
void display_upload(mfd_display_t *display);

///
cairo_t *display_get_cairo(const mfd_display_t *display);

///
unsigned display_get_texture(const mfd_display_t *display);


#ifdef __cplusplus
} /* extern "C" */
#endif
