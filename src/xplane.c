//===--------------------------------------------------------------------------------------------===
// datarefs.c - Datarefs shortcuts
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <libavionics/xplane.h>
#include <ccore/log.h>
#include <ccore/filesystem.h>
#include <XPLMUtilities.h>
#include <XPLMPlugin.h>
#include <XPLMPlanes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char plugin_path[1024];
static char xsystem_path[1024];
static char aircraft_path[1024];

static void get_paths() {
    static bool init = false;
    if(!init) {
        init = true;
        XPLMGetSystemPath(xsystem_path);
        XPLMPluginID id = XPLMGetMyID();
        XPLMGetPluginInfo(id, NULL, plugin_path, NULL, NULL);
        ccfs_path_rtrim_i(plugin_path, 1);
        CCINFO("plugin path: %s", plugin_path);

        char plane_name[512];
        XPLMGetNthAircraftModel(XPLM_USER_AIRCRAFT, plane_name, aircraft_path);
        ccfs_path_rtrim_i(aircraft_path, 0);
        CCINFO("aircraft path: %s", aircraft_path);
    }
}

const char *xp_path_system() {
    get_paths();
    return xsystem_path;
}
const char *xp_path_plugin() {
    get_paths();
    return plugin_path;
}

const char *xp_path_aircraft() {
    get_paths();
    return aircraft_path;
}

void xp_path_plugin_prefix(const char *file, char *out, int max) {
    get_paths();
    ccfs_path_concat(out, max, plugin_path, file, NULL);
    CCINFO("path: %s", out);
}

void xp_path_system_prefix(const char *file, char *out, int max) {
    get_paths();
    ccfs_path_concat(out, max, xsystem_path, file, NULL);
    CCINFO("path: %s", out);
}

void xp_path_aircraft_prefix(const char *file, char *out, int max) {
    get_paths();
    ccfs_path_concat(out, max, aircraft_path, file, NULL);
    CCINFO("path: %s", out);
}

bool xp_path_file_exists(const char *file) {
    return access(file, F_OK) == 0;
}

XPLMDataRef xp_find_dr(const char *path, bool *success) {
    CCDEBUG("resolving dataref `%s`", path);
    XPLMDataRef ref = XPLMFindDataRef(path);
    if(!ref) {
        CCERROR("unresolved dataref `%s`", path);
        *success = false;
        return NULL;
    }
    return ref;
}
