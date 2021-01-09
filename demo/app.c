//===--------------------------------------------------------------------------------------------===
// app - description
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "app.h"
#include <ccore/log.h>
#include <stdlib.h>
#include "gl.h"
//
// static const char *quad_vert = "";
// static const char *quad_frag = "";

static const float quad_data[] = {
    0.f, 0.f,
    1.f, 0.f,
    0.f, 1.f,

    0.f, 1.f,
    1.f, 0.f,
    1.f, 1.f,
};

typedef struct renderer_s {
    unsigned vbo;
    unsigned vao;
    unsigned shader;

    float proj[16];
} renderer_t;

static void check(bool expr, const char *msg) {
    if(expr) return;
    CCERROR("error: %s", msg);
    glfwTerminate();
    abort();
}

static void gl_check(int line) {
    GLenum status = glGetError();
    if(status != GL_NO_ERROR) {
        CCERROR("OpenGL error at line %d: %x", line, status);
    }
}

#define GL_CHECK() gl_check(__LINE__);

static void renderer_init(renderer_t *r, unsigned width, unsigned height) {
    gl_ortho(r->proj, width, height);
    glGenVertexArrays(1, &r->vao);
    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_data), quad_data, GL_STATIC_DRAW);
    GL_CHECK();

    glBindVertexArray(r->vao);
    glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);

    GL_CHECK();

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    GL_CHECK();

    r->shader = gl_create_program_file("quad.vsh", "quad.fsh");
    GL_CHECK();
}

static void renderer_deinit(renderer_t *r) {
    glDeleteProgram(r->shader);
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);
}

static void error_cb(int error, const char *desc) {
    CCERROR("glfw: (%d) %s", error, desc);
}

void app_main(
    const app_desc_t *desc,
    app_init_f init,
    app_update_f update,
    app_fini_f fini,
    void *user_data
) {
    CCDEBUG("creating OpenGL %d.%d window", desc->version_major, desc->version_minor);
    check(glfwInit(), "unable to initialise GLFW");

    glfwWindowHint(GLFW_VERSION_MAJOR, desc->version_major);
    glfwWindowHint(GLFW_VERSION_MINOR, desc->version_minor);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetErrorCallback(error_cb);

    GLFWwindow *window = glfwCreateWindow(desc->width, desc->height, desc->name, NULL, NULL);
    check(window, "window creation failed");
    glfwMakeContextCurrent(window);
    gladLoadGL();

    glViewport(0, 0, desc->width, desc->height);
    glfwSwapInterval(1);

    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LESS);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double last = glfwGetTime();

    renderer_t renderer;
    renderer_init(&renderer, desc->width, desc->height);

    if(init) init(user_data);

    CCDEBUG("starting run loop");
    while (!glfwWindowShouldClose(window)) {
        glViewport(0, 0, desc->width, desc->height);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        double now = glfwGetTime();
        double delta = now - last;
        last = now;
        if(update) update(delta, &renderer, user_data);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if(fini) fini(user_data);

    renderer_deinit(&renderer);

    CCDEBUG("done, destroying OpenGL");
    glfwDestroyWindow(window);
    glfwTerminate();
}

void render_quad(const renderer_t *r, unsigned tex, unsigned x, unsigned y, unsigned w, unsigned h) {
    glBindVertexArray(r->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(r->shader);
    glUniformMatrix4fv(glGetUniformLocation(r->shader, "proj"), 1, GL_TRUE, r->proj);
    glUniform2f(glGetUniformLocation(r->shader, "size"), w, h);
    glUniform2f(glGetUniformLocation(r->shader, "pos"), x, y);
    // glUniform1ui(glGetUniformLocation(r->shader, "tex"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_CHECK();
}
