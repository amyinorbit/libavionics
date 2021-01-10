//===--------------------------------------------------------------------------------------------===
// module.c - Implementation of threaded avionics
//
// Created by Amy Parent <developer@amyparent.com>
// Copyright (c) 2020 Amy Parent
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libavionics/module.h>
#include <ccore/log.h>
#include <ccore/memory.h>
#include <stdint.h>
#include <pthread.h>

struct av_module_t {
    av_module_init_f init;
    av_module_update_f update;
    av_module_fini_f fini;

    void *data;
    double interval;
    bool stop;

    pthread_cond_t cv;
    pthread_mutex_t mt;
    pthread_t thread;
};


// TODO: move that to Ccore!

#if WIN32
#include <windows.h>
#else /* !WIN32 */
#include <sys/time.h>
#endif /* !WIN32 */
#include <stdlib.h>

#include "time.h"

uint64_t
cc_microtime(void)
{
#if WIN32
    LARGE_INTEGER val, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&val);
    return (((double)val.QuadPart / (double)freq.QuadPart) * 1000000.0);
#else    /* !IBM */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000000llu) + tv.tv_usec);
#endif    /* !IBM */
}

static void cond_wait_until(pthread_cond_t *cv, pthread_mutex_t *mt, uint64_t t) {
    struct timespec abs;
    abs.tv_sec = t / 1000000UL;
    abs.tv_nsec = (t % 1000000UL) * 1000UL;
    pthread_cond_timedwait(cv, mt, &abs);
}


static void *module_thread(void *refcon) {
    av_module_t *module = refcon;
    uint64_t t = cc_microtime();
    pthread_mutex_lock(&module->mt);

    if(module->init) module->init(module->data);

    while(!module->stop) {
        cond_wait_until(&module->cv, &module->mt, t + module->interval);
        if(module->stop) break;
        uint64_t new_t = cc_microtime();
        uint64_t delta_us = new_t - t;
        if(new_t >= t + module->interval) {
            t += delta_us;
        }
        pthread_mutex_unlock(&module->mt);

        if(module->update) module->update((double)delta_us / 1e6, module->data);

        pthread_mutex_lock(&module->mt);
        pthread_cond_broadcast(&module->cv);
    }

    if(module->fini) module->fini(module->data);
    CCINFO("stopping module thread");
    pthread_mutex_unlock(&module->mt);
    return NULL;
}


av_module_t *av_module_new(
    double fps,
    av_module_init_f init,
    av_module_update_f update,
    av_module_fini_f fini,
    void *data
) {
    CCASSERT(update);

    av_module_t *module = cc_alloc(sizeof(av_module_t));
    module->interval = 1e6/fps;
    module->stop = false;
    module->init = init;
    module->update = update;
    module->fini = fini;
    module->data = data;

    pthread_cond_init(&module->cv, NULL);
    pthread_mutex_init(&module->mt, NULL);
    pthread_create(&module->thread, NULL, module_thread, module);
    return module;
}

void av_module_delete(av_module_t *module) {
    CCASSERT(module);
    CCASSERT(!module->stop);
    pthread_mutex_lock(&module->mt);
    module->stop = true;
    pthread_mutex_unlock(&module->mt);
    pthread_cond_broadcast(&module->cv);
    pthread_join(module->thread, NULL);
    pthread_cond_destroy(&module->cv);
    pthread_mutex_destroy(&module->mt);
    cc_free(module);
}

void av_module_wait(av_module_t *module) {
    CCASSERT(module);
    CCASSERT(!module->stop);

    pthread_mutex_lock(&module->mt);
    pthread_cond_wait(&module->cv, &module->mt);
    pthread_mutex_unlock(&module->mt);
}
