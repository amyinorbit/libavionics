//===--------------------------------------------------------------------------------------------===
// display_render.c - OpenGL 2D rendering functions for display.c
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2021 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "display.h"
#include <ccore/math.h>
#include <ccore/memory.h>
#include <stdlib.h>

static const char *vert_shader =
    "#version 120\n"
    "uniform mat4   pvm;\n"
    "attribute vec2 vtx_pos;\n"
    "attribute vec2 vtx_tex0;\n"
    "varying vec2   tex_coord;\n"
    "void main() {\n"
    "    tex_coord = vtx_tex0;\n"
    "    gl_Position = pvm * vec4(vtx_pos, 0.0, 1.0);\n"
    // "    gl_Position = vec4(vtx_pos, 0.0, 1.0);\n"
    "}\n";

static const char *frag_shader =
    "#version 120\n"
    "uniform sampler2D	tex;\n"
    "uniform float	alpha;\n"
    "varying vec2	tex_coord;\n"
    "void main() {\n"
    "    vec4 color = texture2D(tex, tex_coord);\n"
    "    color.a *= alpha;\n"
    "    gl_FragColor = color;\n"
    "}\n";

static bool is_init = false;
static unsigned default_quad_shader = 0;

void av_display_init() {
    if(is_init) return;
    default_quad_shader = gl_create_program(vert_shader, frag_shader);
    if(!default_quad_shader) return;
    is_init = true;
}

void av_display_deinit() {
    CCASSERT(is_init);
    glDeleteProgram(default_quad_shader);
    default_quad_shader = 0;
    is_init = false;
}

av_target_t *av_target_new(double width, double height) {
    CCASSERT(width > 0);
    CCASSERT(height > 0);
    av_target_t *target = cc_alloc(sizeof(av_target_t));
    target->size = CC_VEC2(width, height);
    gl_ortho(target->proj, width, height);
    return target;
}

void av_target_delete(av_target_t *target) {
    CCASSERT(target);
    cc_free(target);
}

void renderer_init(renderer_t *r) {
    // XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    
    r->last_pos = CC_VEC2_NULL;
    r->last_size = CC_VEC2_NULL;
    
    r->quad_shader = default_quad_shader;
    glGenBuffers(1, &r->quad_vbo);
    glGenBuffers(1, &r->quad_ibo);
    
    static const GLuint indices[] = {0, 1, 2, 0, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->quad_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glUseProgram(r->quad_shader);
    r->loc.pvm = glGetUniformLocation(r->quad_shader, "pvm");
    r->loc.tex = glGetUniformLocation(r->quad_shader, "tex");
    r->loc.alpha = glGetUniformLocation(r->quad_shader, "alpha");
    
    r->loc.vtx_pos = glGetAttribLocation(r->quad_shader, "vtx_pos");
    r->loc.vtx_tex0 = glGetAttribLocation(r->quad_shader, "vtx_tex0");

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // XPLMBindTexture2d(0, 0);

    CCDEBUG("Renderer VBO: %u", r->quad_vbo);
    CCDEBUG("Renderer IBO: %u", r->quad_ibo);
    CCDEBUG("Renderer Shader: %u", r->quad_shader);
}

 void renderer_deinit(renderer_t *r) {
    // XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    glDeleteBuffers(1, &r->quad_vbo);
    glDeleteBuffers(1, &r->quad_ibo);
}

typedef struct {
    float x;
    float y;
} vec2f_t;

typedef struct {
    vec2f_t pos;
    vec2f_t tex;
} vertex_t;

static inline bool vec2_eq(vec2_t a, vec2_t b) {
    return a.x == b.x && a.y == b.y;
}

static void prepare_vertices(renderer_t *r, vec2_t pos, vec2_t size) {
    if(vec2_eq(r->last_pos, pos) && vec2_eq(r->last_size, size)) return;
    vertex_t vert[4];
    
    vert[0].pos.x = pos.x;
    vert[0].pos.y = pos.y;
    vert[0].tex = (vec2f_t){0, 0};

    vert[1].pos.x = pos.x + size.x;
    vert[1].pos.y = pos.y;
    vert[1].tex = (vec2f_t){1, 0};

    vert[2].pos.x = pos.x + size.x;
    vert[2].pos.y = pos.y + size.y;
    vert[2].tex = (vec2f_t){1, 1};

    vert[3].pos.x = pos.x;
    vert[3].pos.y = pos.y + size.y;
    vert[3].tex = (vec2f_t){0, 1};
    
    CCDEBUG("Uploading new vertex data");
    glBindBuffer(GL_ARRAY_BUFFER, r->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    CHECK_GL();
    
    r->last_pos = pos;
    r->last_size = size;
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static inline void
enable_attrib(GLint index, GLint size, GLenum type,
    GLboolean normalized, size_t stride, size_t offset)
{
	if (index != -1) {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized,
		    stride, (void *)offset);
	}
}

void av_display_render_gl(
    av_display_t *display,
    const av_target_t *target,
    double x,
    double y,
    float brightness
) {
    CCASSERT(is_init);
    CCASSERT(display);
    CCASSERT(target);

    renderer_t *r = &display->renderer;
    
#if APPLE
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, display->texture);
    glBindBuffer(GL_ARRAY_BUFFER, r->quad_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->quad_ibo);
    
    enable_attrib(r->loc.vtx_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, pos));
    enable_attrib(r->loc.vtx_tex0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, tex));
    glUseProgram(r->quad_shader);
    
    prepare_vertices(r, CC_VEC2(x, y), CC_VEC2(display->width, display->height));
    
    glUniformMatrix4fv(r->loc.pvm, 1, GL_TRUE, target->proj);
    glUniform1f(r->loc.alpha, brightness);
    glUniform1i(r->loc.tex, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    CHECK_GL();

    glDisableVertexAttribArray(r->loc.vtx_pos);
    glDisableVertexAttribArray(r->loc.vtx_tex0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    CHECK_GL();
}

