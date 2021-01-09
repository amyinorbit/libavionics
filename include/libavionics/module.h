//===--------------------------------------------------------------------------------------------===
// module.h - Threaded avionics module building block.
//
// Created by Amy Parent <developer@amyparent.com>
// Copyright (c) 2020 Amy Parent
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct av_module_t av_module_t;

typedef void (*av_module_init_f)(void *data);
typedef void (*av_module_update_f)(double delta, void *data);
typedef void (*av_module_fini_f)(void *data);

/// Creates a new threaded avionics module.
av_module_t *av_module_new(
    double fps,
    av_module_init_f init,
    av_module_update_f update,
    av_module_fini_f fini,
    void *data
);

/// Stops, then deletes [module].
void av_module_delete(av_module_t *module);

/// Waits for [module] to complete its current update cycle.
void av_module_wait(av_module_t *module);

#ifdef __cplusplus
} /* extern "C" */
#endif

