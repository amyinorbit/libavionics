//===--------------------------------------------------------------------------------------------===
// app - Basic OpenGL application helpers
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct renderer_s renderer_t;

typedef void (*app_init_f)(void *);
typedef void (*app_fini_f)(void *);
typedef void (*app_update_f)(double, const renderer_t *, void *);

typedef struct {
    const char *name;
    uint8_t version_major;
    uint8_t version_minor;
    uint32_t width;
    uint32_t height;
} app_desc_t;

void app_main(const app_desc_t *desc,
    app_init_f init,
    app_update_f update,
    app_fini_f fini,
    void *user_data);


void render_quad(const renderer_t *r, unsigned tex, unsigned x, unsigned y, unsigned w, unsigned h);
