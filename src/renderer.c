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

void av_render_init() {
    if(is_init) return;
    default_quad_shader = gl_create_program(vert_shader, frag_shader);
    if(!default_quad_shader) return;
    is_init = true;
}

void av_render_deinit() {
    CCASSERT(is_init);
    glDeleteProgram(default_quad_shader);
    default_quad_shader = 0;
    is_init = false;
}

av_target_t *av_target_new(double x, double y, double width, double height) {
    CCASSERT(width > 0);
    CCASSERT(height > 0);
    av_target_t *target = cc_alloc(sizeof(av_target_t));
    av_target_set_size(target, width, height);
    target->size = CC_VEC2(width, height);
    target->offset = CC_VEC2(x, y);
    gl_ortho(target->proj, target->offset.x, target->offset.y, target->size.x, target->size.y);
    return target;
}

void av_target_delete(av_target_t *target) {
    CCASSERT(target);
    cc_free(target);
}

void av_target_set_offset(av_target_t *target, double x, double y) {
    CCASSERT(target);
    target->offset = CC_VEC2(x, y);
    gl_ortho(target->proj, x, y, target->size.x, target->size.y);
    
}

void av_target_set_size(av_target_t *target, double width, double height) {
    CCASSERT(target);
    CCASSERT(width > 0);
    CCASSERT(height > 0);
    target->size = CC_VEC2(width, height);
    gl_ortho(target->proj, target->offset.x, target->offset.y, width, height);
}

av_quad_t *av_quad_new(unsigned tex, unsigned shader) {
    av_quad_t *quad = cc_alloc(sizeof(av_quad_t));
    av_quad_init(quad, tex, shader);
    return quad;
}

void av_quad_delete(av_quad_t *quad) {
    av_quad_deinit(quad);
    cc_free(quad);
}

void av_quad_init(av_quad_t *quad, unsigned tex, unsigned shader) {
    quad->last_pos = CC_VEC2_NULL;
    quad->last_size = CC_VEC2_NULL;
    
    quad->tex = tex;
    quad->shader = shader ? shader : default_quad_shader;
    glGenBuffers(1, &quad->vbo);
    glGenBuffers(1, &quad->ibo);
    
    static const GLuint indices[] = {0, 1, 2, 0, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glUseProgram(quad->shader);
    quad->loc.pvm = glGetUniformLocation(quad->shader, "pvm");
    quad->loc.tex = glGetUniformLocation(quad->shader, "tex");
    quad->loc.alpha = glGetUniformLocation(quad->shader, "alpha");
    
    quad->loc.vtx_pos = glGetAttribLocation(quad->shader, "vtx_pos");
    quad->loc.vtx_tex0 = glGetAttribLocation(quad->shader, "vtx_tex0");

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // XPLMBindTexture2d(0, 0);

    CCDEBUG("Quad VBO: %u", quad->vbo);
    CCDEBUG("Quad IBO: %u", quad->ibo);
    CCDEBUG("Quad Shader: %u", quad->shader);
}

 void av_quad_deinit(av_quad_t *quad) {
    // XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    glDeleteBuffers(1, &quad->vbo);
    glDeleteBuffers(1, &quad->ibo);
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

static void prepare_vertices(av_quad_t *quad, vec2_t pos, vec2_t size) {
    if(vec2_eq(quad->last_pos, pos) && vec2_eq(quad->last_size, size)) return;
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
    
    glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    CHECK_GL();
    
    quad->last_pos = pos;
    quad->last_size = size;
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static inline void enable_attrib(GLint index, GLint size, GLenum type,
    GLboolean normalized, size_t stride, size_t offset)
{
	if (index != -1) {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized,
		    stride, (void *)offset);
	}
}

void av_render_quad(av_target_t *target, av_quad_t *quad, vec2_t pos, vec2_t size, double alpha) {
    CCASSERT(is_init);
    CCASSERT(quad);
    CCASSERT(target);
    
#if APPLE
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quad->tex);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->ibo);
    
    enable_attrib(quad->loc.vtx_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, pos));
    enable_attrib(quad->loc.vtx_tex0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, tex));
    glUseProgram(quad->shader);
    
    prepare_vertices(quad, pos, size);
    
    glUniformMatrix4fv(quad->loc.pvm, 1, GL_TRUE, target->proj);
    glUniform1f(quad->loc.alpha, alpha);
    glUniform1i(quad->loc.tex, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    CHECK_GL();

    glDisableVertexAttribArray(quad->loc.vtx_pos);
    glDisableVertexAttribArray(quad->loc.vtx_tex0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    CHECK_GL();
}

