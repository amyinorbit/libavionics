//===--------------------------------------------------------------------------------------------===
// display.c - mt-cairo display implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libmfd/display.h>
#include <ccore/memory.h>
#include <ccore/log.h>
#include <stdlib.h>
#include <string.h>
#include "glad.h"

struct mfd_display_s {
    unsigned width, height;
    unsigned texture;

    bool is_back_ready;
    pthread_mutex_t mt;
    unsigned front, back;
    unsigned pbos[2];
    void *current;
    cairo_surface_t *surface;
    cairo_t *cairo;
};


static void init_buffer(mfd_display_t *display, int idx) {
    CCASSERT(display);
    size_t size = display->width * display->height * 4;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
}

void *map_buffer(mfd_display_t *display, int idx) {
    CCASSERT(display);
    size_t size = display->width * display->height * 4;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
    return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
}

void unmap_buffer(mfd_display_t *display, int idx) {
    CCASSERT(display);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
}

mfd_display_t *display_new(unsigned width, unsigned height) {
    CCASSERT(width > 0);
    CCASSERT(height > 0);

    mfd_display_t *display = cc_alloc(sizeof(mfd_display_t));
    display->width = width;
    display->height = height;

    display->front = 0;
    display->back = 1;
    display->is_back_ready = true;
    display->current = NULL;


    // Create our transfer buffers
    glGenBuffers(2, display->pbos);
    init_buffer(display, 0);
    init_buffer(display, 1);

    display->current = map_buffer(display, display->back);

    // Create the texture
    glGenTextures(1, &display->texture);
    glBindTexture(GL_TEXTURE_2D, display->texture);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[display->front]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    display->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    CCASSERT(display->surface);
    display->cairo = cairo_create(display->surface);
    CCASSERT(display->cairo);

    pthread_mutex_init(&display->mt, NULL);
    return display;
}

void display_delete(mfd_display_t *display) {
    CCASSERT(display);
    pthread_mutex_destroy(&display->mt);
    glDeleteTextures(1, &display->texture);
    glDeleteBuffers(2, display->pbos);
    cairo_destroy(display->cairo);
    cairo_surface_destroy(display->surface);

    cc_free(display);
}

void *display_get_back_buffer(mfd_display_t *display) {
    if(display->is_back_ready) return NULL; // We haven't drawn the old frame yet
    return display->current;
}

void display_finish_back_buffer(mfd_display_t *display) {
    CCASSERT(display);
    if(!display->current) return;
    pthread_mutex_lock(&display->mt);
    const void *src = cairo_image_surface_get_data(display->surface);
    memcpy(display->current, src, 4 * display->width * display->height);
    display->is_back_ready = true;
    pthread_mutex_unlock(&display->mt);
}

void display_upload(mfd_display_t *display) {
    CCASSERT(display);
    if(!display->is_back_ready) return;
    pthread_mutex_lock(&display->mt);

    // First, upload the data from the back glBufferData
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[display->front]);
    glBindTexture(GL_TEXTURE_2D, display->texture);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0,
        display->width, display->height,
        GL_BGRA, GL_UNSIGNED_BYTE, NULL
    );
    // glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    unmap_buffer(display, display->back);
    display->back = (display->back + 1) % 2;
    display->front = (display->front + 1) % 2;
    display->current = map_buffer(display, display->back);
    display->is_back_ready = false;

    // CCDEBUG("done. front: %d, back: %d. buffer: %p", display->front, display->back, display->current);

    pthread_mutex_unlock(&display->mt);
}

cairo_t *display_get_cairo(const mfd_display_t *display) {
    CCASSERT(display);
    CCASSERT(display->cairo);
    return display->cairo;
}

unsigned display_get_texture(const mfd_display_t *display) {
    CCASSERT(display);
    return display->texture;
}
