//===--------------------------------------------------------------------------------------------===
// dref.c - Thin abstraction over X-Plane's dataref system
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libavionics/xplane.h>
#include <ccore/log.h>
#include <ccore/math.h>
#include <ccore/string.h>
#include <XPLMPlugin.h>
#include <stdarg.h>
#include <stdio.h>

#define DATAREF_EXPORT_MSG (0x01000000)

static bool drt_dre_resolved = false;
static XPLMPluginID datareftool = XPLM_NO_PLUGIN_ID;
static XPLMPluginID datarefedit = XPLM_NO_PLUGIN_ID;


bool dref_find(dref_t *dr, const char *format, ...) {
    CCASSERT(dr);
    CCASSERT(format);
    dr->value = NULL;
    dr->count = 0;
    
    va_list args;
    va_start(args, format);
    vsnprintf(dr->name, sizeof(dr->name), format, args);
    va_end(args);
    
    dr->dref = XPLMFindDataRef(dr->name);
    if(dr->dref == NULL) {
        CCWARN("dataref `%s` not found", dr->name);
        return false;
    }
    
    dr->is_writeable = XPLMCanWriteDataRef(dr->dref);
    dr->type = XPLMGetDataRefTypes(dr->dref);
    return true;
}

bool dref_find_strict(dref_t *dr, const char *format, ...) {
    CCASSERT(dr);
    CCASSERT(format);
    
    dr->value = NULL;
    dr->count = 0;

    va_list args;
    va_start(args, format);
    vsnprintf(dr->name, sizeof(dr->name), format, args);
    va_end(args);

    dr->dref = XPLMFindDataRef(dr->name);
    if(dr->dref == NULL) {
        CCERROR("dataref `%s` not found", dr->name);
        abort();
        return false;
    }

    dr->is_writeable = XPLMCanWriteDataRef(dr->dref);
    dr->type = XPLMGetDataRefTypes(dr->dref);
    return true;
}

static float get_float_cb(void *user_data) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Float);
    CCASSERT(dr->value);
    return *(float*)dr->value;
}

static double get_double_cb(void *user_data) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Double);
    CCASSERT(dr->value);
    return *(double*)dr->value;
}

static int get_int_cb(void *user_data) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Int);
    CCASSERT(dr->value);
    return *(int*)dr->value;
}

static int get_float_array_cb(void *user_data, float *out, int offset, int max) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_FloatArray);
    CCASSERT(dr->value);
    
    if(!out) return dr->count;
    if(offset >= dr->count) return 0;
    
    const float *src = dr->value;
    int to_copy = cc_min(dr->count - offset, max);
    memcpy(out, &src[offset], to_copy * sizeof(float));
    return to_copy;
}

static int get_int_array_cb(void *user_data, int *out, int offset, int max) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_IntArray);
    CCASSERT(dr->value);
    
    if(!out) return dr->count;
    if(offset >= dr->count) return 0;
    
    const int *src = dr->value;
    int to_copy = cc_min(dr->count - offset, max);
    memcpy(out, &src[offset], to_copy * sizeof(int));
    return to_copy;
}

static int get_byte_array_cb(void *user_data, void *out, int offset, int max) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Data);
    CCASSERT(dr->value);
    
    if(!out) return dr->count;
    if(offset >= dr->count) return 0;
    
    const char *src = dr->value;
    int to_copy = cc_min(dr->count - offset, max);
    memcpy(out, src+offset, to_copy);
    return to_copy;
}

static void set_float_cb(void *user_data, float value) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Float);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    *(float*)dr->value = value;
}

static void set_double_cb(void *user_data, double value) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Double);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    *(double*)dr->value = value;
}

static void set_int_cb(void *user_data, int value) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_Int);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    *(int*)dr->value = value;
}

static void set_float_array_cb(void *user_data, float *in, int offset, int count) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_FloatArray);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    float *out = dr->value;
    int to_copy = cc_min(dr->count - offset, count);
    memcpy(&out[offset], in, to_copy * sizeof(float));
}

static void set_int_array_cb(void *user_data, int *in, int offset, int count) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_IntArray);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    int *out = dr->value;
    int to_copy = cc_min(dr->count - offset, count);
    memcpy(&out[offset], in, to_copy * sizeof(int));
}

static void set_byte_array_cb(void *user_data, void *in, int offset, int count) {
    CCASSERT(user_data);
    dref_t *dr = user_data;
    CCASSERT(dr->type & xplmType_FloatArray);
    CCASSERT(dr->value);
    CCASSERT(dr->is_writeable); // TODO: make that assertion verbose in the log [ccore]
    char *out = dr->value;
    int to_copy = cc_min(dr->count - offset, count);
    memcpy(out + offset, in, to_copy);
}

static void dref_create(
    dref_t *dr,
    const char *name,
    bool is_writeable,
    void *value,
    XPLMDataTypeID type
) {
    CCASSERT(dr);
    CCASSERT(value);
    
    dr->value = value;
    dr->count = 0;
    dr->is_writeable = is_writeable;
    dr->type = type;
    string_copy(dr->name, name, sizeof(dr->name));
    
    dr->dref = XPLMRegisterDataAccessor(
        dr->name,
        dr->type,
        dr->is_writeable,
        get_int_cb,
        set_int_cb,
        get_float_cb,
        set_float_cb,
        get_double_cb,
        set_double_cb,
        get_int_array_cb,
        set_int_array_cb,
        get_float_array_cb,
        set_float_array_cb,
        get_byte_array_cb,
        set_byte_array_cb,
        dr,
        dr
    );
        
    if(!drt_dre_resolved) {
        datarefedit = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
        datareftool = XPLMFindPluginBySignature("com.leecbaker.datareftool");
        drt_dre_resolved = true;
    }
    
    if(datareftool != XPLM_NO_PLUGIN_ID) {
        XPLMSendMessageToPlugin(datarefedit, DATAREF_EXPORT_MSG, (void*)dr->name);
    }
    
    if(datarefedit != XPLM_NO_PLUGIN_ID) {
        XPLMSendMessageToPlugin(datarefedit, DATAREF_EXPORT_MSG, (void*)dr->name);
    }
}


void dref_create_i32(dref_t *dr, const char *name, bool is_writeable, int *value) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_Int);
}

void dref_create_f32(dref_t *dr, const char *name, bool is_writeable, float *value) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_Float);
}

void dref_create_f64(dref_t *dr, const char *name, bool is_writeable, double *value) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_Double);
}

void dref_create_fv(dref_t *dr, const char *name, bool is_writeable, float *value, int size) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_FloatArray);
    dr->count = size;
}

void dref_create_iv(dref_t *dr, const char *name, bool is_writeable, float *value, int size) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_IntArray);
    dr->count = size;
}

void dref_create_bv(dref_t *dr, const char *name, bool is_writeable, void *value, int size) {
    CCASSERT(dr);
    CCASSERT(value);
    dref_create(dr, name, is_writeable, value, xplmType_Data);
    dr->count = size;
}

void dref_delete(dref_t *dr) {
    CCASSERT(dr);
    CCASSERT(dr->value);
    CCASSERT(dr->dref);
    XPLMUnregisterDataAccessor(dr->dref);
    dr->dref = NULL;
    dr->type = 0;
    dr->value = NULL;
    dr->is_writeable = false;
}


int dref_get_i32(const dref_t *dr) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    
    if(dr->type & xplmType_Int)
        return XPLMGetDatai(dr->dref);
    if(dr->type & xplmType_Float)
        return XPLMGetDataf(dr->dref);
    if(dr->type & xplmType_Double)
        return XPLMGetDatad(dr->dref);
    CCASSERT(false); // Print verbose message here [ccore]
    return 0;
}

float dref_get_f32(const dref_t *dr) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    
    if(dr->type & xplmType_Float)
        return XPLMGetDataf(dr->dref);
    if(dr->type & xplmType_Double)
        return XPLMGetDatad(dr->dref);
    if(dr->type & xplmType_Int)
        return XPLMGetDatai(dr->dref);
    CCASSERT(false); // Print verbose message here [ccore]
    return NAN;
}

double dref_get_f64(const dref_t *dr) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    
    if(dr->type & xplmType_Double)
        return XPLMGetDatad(dr->dref);
    if(dr->type & xplmType_Float)
        return XPLMGetDataf(dr->dref);
    if(dr->type & xplmType_Int)
        return XPLMGetDatai(dr->dref);
    CCASSERT(false); // Print verbose message here [ccore]
    return NAN;
}

int dref_get_fv(const dref_t *dr, float *out, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_FloatArray);
    
    return XPLMGetDatavf(dr->dref, out, offset, size);
}

int dref_get_iv(const dref_t *dr, int *out, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_IntArray);
    
    return XPLMGetDatavi(dr->dref, out, offset, size);
}

int dref_get_bv(const dref_t *dr, void *out, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_Data);
    
    return XPLMGetDatab(dr->dref, out, offset, size);
}


void dref_set_i32(const dref_t *dr, int value) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->is_writeable);
    
    if(dr->type & xplmType_Int)
        XPLMSetDatai(dr->dref, value);
    else if(dr->type & xplmType_Float)
        XPLMSetDataf(dr->dref, value);
    else if(dr->type & xplmType_Double)
        XPLMSetDatad(dr->dref, value);
    else
        CCASSERT(false); // Print verbose message here [ccore]
}

void dref_set_f32(const dref_t *dr, float value) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->is_writeable);
    
    if(dr->type & xplmType_Float)
        XPLMSetDataf(dr->dref, value);
    else if(dr->type & xplmType_Double)
        XPLMSetDatad(dr->dref, value);
    else if(dr->type & xplmType_Int)
        XPLMSetDatai(dr->dref, value);
    else
        CCASSERT(false); // Print verbose message here [ccore]
}

void dref_set_f64(const dref_t *dr, double value) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->is_writeable);

    if(dr->type & xplmType_Double)
        XPLMSetDatad(dr->dref, value);
    else if(dr->type & xplmType_Float)
        XPLMSetDataf(dr->dref, value);
    else if(dr->type & xplmType_Int)
        XPLMSetDatai(dr->dref, value);
    else
        CCASSERT(false); // Print verbose message here [ccore]
}

void dref_set_fv(const dref_t *dr, float *in, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_FloatArray);
    CCASSERT(dr->is_writeable);
    
    XPLMSetDatavf(dr->dref, in, offset, size);
}

void dref_set_iv(const dref_t *dr, int *in, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_IntArray);
    CCASSERT(dr->is_writeable);
    
    XPLMSetDatavi(dr->dref, in, offset, size);
}

void dref_set_bv(const dref_t *dr, void *in, int offset, int size) {
    CCASSERT(dr);
    CCASSERT(dr->dref);
    CCASSERT(dr->type & xplmType_Data);
    CCASSERT(dr->is_writeable);
    
    XPLMSetDatab(dr->dref, in, offset, size);
}

