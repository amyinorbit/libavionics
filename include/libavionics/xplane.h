//===--------------------------------------------------------------------------------------------===
// xplane.h - X-Plane utilities
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#include <stdbool.h>
#include <XPLMDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif

    
typedef struct dref_t {
    char name[128];
    XPLMDataRef dref;
    XPLMDataTypeID type;
    bool is_writeable;
    int count;
    void *value;
} dref_t;


bool dref_find(dref_t *dr, const char *format, ...);
bool dref_find_strict(dref_t *dr, const char *format, ...);

void dref_create_i32(dref_t *dr, const char *name, bool is_writeable, int *value);
void dref_create_f32(dref_t *dr, const char *name, bool is_writeable, float *value);
void dref_create_f64(dref_t *dr, const char *name, bool is_writeable, double *value);
void dref_create_fv(dref_t *dr, const char *name, bool is_writeable, float *value, int size);
void dref_create_iv(dref_t *dr, const char *name, bool is_writeable, float *value, int size);
void dref_create_bv(dref_t *dr, const char *name, bool is_writeable, void *value, int size);

void dref_delete(dref_t *dr);

int dref_get_i32(const dref_t *dr);
float dref_get_f32(const dref_t *dr);
double dref_get_f64(const dref_t *dr);
int dref_get_fv(const dref_t *dr, float *out, int offset, int size);
int dref_get_iv(const dref_t *dr, int *out, int offset, int size);
int dref_get_bv(const dref_t *dr, void *out, int offset, int size);

void dref_set_i32(const dref_t *dr, int value);
void dref_set_f32(const dref_t *dr, float value);
void dref_set_f64(const dref_t *dr, double value);
void dref_set_fv(const dref_t *dr, float *out, int offset, int size);
void dref_set_iv(const dref_t *dr, int *out, int offset, int size);
void dref_set_bv(const dref_t *dr, void *out, int offset, int size);


const char *xp_path_system();
const char *xp_path_plugin();
const char *xp_path_aircraft();

void xp_path_plugin_prefix(const char *file, char *out, int max);
void xp_path_system_prefix(const char *file, char *out, int max);
void xp_path_aircraft_prefix(const char *file, char *out, int max);

bool xp_path_file_exists(const char *file);

#ifdef __cplusplus
} /* extern "C" */
#endif
