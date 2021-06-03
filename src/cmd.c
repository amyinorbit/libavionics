//===--------------------------------------------------------------------------------------------===
// cmd.c - thin abstraction over the XPLMCommandRef API
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libavionics/xplane.h>
#include <ccore/log.h>
#include <ccore/string.h>
#include <stdarg.h>
#include <stdio.h>

XPLMCommandRef cmd_find(const char *name) {
    CCASSERT(name);
    return XPLMFindCommand(name);
}

XPLMCommandRef cmd_findf(const char *fmt, ...) {
    CCASSERT(fmt);
    
    char name[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(name, sizeof(name), fmt, args);
    va_end(args);
    
    return cmd_find(name);
}

XPLMCommandRef cmd_bind(const char *name, cmd_cb_t cb, bool before, void *refcon) {
    CCASSERT(name);
    CCASSERT(cb);
    
    XPLMCommandRef ref = cmd_find(name);
    if(!ref) return NULL;
    XPLMRegisterCommandHandler(ref, cb, before, refcon);
    return ref;
}

XPLMCommandRef cmd_bindf(const char *fmt, cmd_cb_t cb, bool before, void *refcon, ...) {
    CCASSERT(fmt);
    CCASSERT(cb);
    
    char name[1024];
    va_list args;
    va_start(args, refcon);
    vsnprintf(name, sizeof(name), fmt, args);
    va_end(args);
    return cmd_bind(name, cb, before, refcon);
}

XPLMCommandRef cmd_unbind(const char *name, cmd_cb_t cb, bool before, void *refcon) {
    CCASSERT(name);
    CCASSERT(cb);
    
    XPLMCommandRef ref = cmd_find(name);
    if(!ref) return NULL;
    XPLMUnregisterCommandHandler(ref, cb, before, refcon);
    return ref;
}

XPLMCommandRef cmd_unbindf(const char *fmt, cmd_cb_t cb, bool before, void *refcon, ...) {
    CCASSERT(fmt);
    CCASSERT(cb);
    
    char name[1024];
    va_list args;
    va_start(args, refcon);
    vsnprintf(name, sizeof(name), fmt, args);
    va_end(args);
    return cmd_unbind(name, cb, before, refcon);
}
