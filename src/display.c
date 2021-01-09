//===--------------------------------------------------------------------------------------------===
// display.c - mt-cairo display implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libavionics/display.h>
#include <ccore/memory.h>
#include <ccore/log.h>
#include <stdlib.h>
#include <string.h>
#include "glad.h"

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
};


static void init_buffer(av_display_t *display, int idx) {
    CCASSERT(display);
    size_t size = display->width * display->height * 4;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void *map_buffer(av_display_t *display, int idx) {
    CCASSERT(display);
    size_t size = display->width * display->height * 4;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
    void *buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    return buffer;
}

void unmap_buffer(av_display_t *display, int idx) {
    CCASSERT(display);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[idx]);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

av_display_t *av_display_new(unsigned width, unsigned height) {
    CCASSERT(width > 0);
    CCASSERT(height > 0);

    av_display_t *display = cc_alloc(sizeof(av_display_t));
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
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    display->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    CCASSERT(display->surface);
    display->cairo = cairo_create(display->surface);
    CCASSERT(display->cairo);

    pthread_mutex_init(&display->mt, NULL);
    return display;
}

void av_display_delete(av_display_t *display) {
    CCASSERT(display);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    pthread_mutex_destroy(&display->mt);
    glDeleteTextures(1, &display->texture);
    glDeleteBuffers(2, display->pbos);
    cairo_destroy(display->cairo);
    cairo_surface_destroy(display->surface);

    cc_free(display);
}

void *av_display_get_back_buffer(av_display_t *display) {
    if(display->is_back_ready) return NULL; // We haven't drawn the old frame yet
    return display->current;
}

void av_display_finish_back_buffer(av_display_t *display) {
    CCASSERT(display);
    if(!display->current) return;
    const void *src = cairo_image_surface_get_data(display->surface);
    memcpy(display->current, src, 4 * display->width * display->height);
    pthread_mutex_lock(&display->mt);
    display->is_back_ready = true;
    pthread_mutex_unlock(&display->mt);
}

void av_display_upload(av_display_t *display) {
    CCASSERT(display);
    if(!display->is_back_ready) return;

    // First, upload the data from the back glBufferData
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, display->pbos[display->front]);
    glBindTexture(GL_TEXTURE_2D, display->texture);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, 0,
        display->width, display->height,
        GL_BGRA, GL_UNSIGNED_BYTE, NULL
    );
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    unmap_buffer(display, display->back);
    
    pthread_mutex_lock(&display->mt);
    display->back = (display->back + 1) % 2;
    display->front = (display->front + 1) % 2;
    display->current = map_buffer(display, display->back);
    display->is_back_ready = false;

    pthread_mutex_unlock(&display->mt);
}

cairo_t *av_display_get_cairo(const av_display_t *display) {
    CCASSERT(display);
    CCASSERT(display->cairo);
    return display->cairo;
}

unsigned av_display_get_texture(const av_display_t *display) {
    CCASSERT(display);
    return display->texture;
}
