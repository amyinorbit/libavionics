//===--------------------------------------------------------------------------------------------===
// gl.c - OpenGL quad renderer
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "gl.h"
// #include <XPLM/XPLMGraphics.h>
#include <ccore/log.h>
#include <ccore/math.h>
#include <ccore/memory.h>
#include <stdlib.h>
//
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

typedef struct renderer_t {
    unsigned quad_vbo;
    unsigned quad_ibo;
    unsigned quad_shader;
    
    struct {
        int vtx_pos;
        int vtx_tex0;
        int pvm;
        int tex;
        int alpha;
    } loc;
    
    vec2_t size;
    float proj[16];
    vec2_t last_pos;
    vec2_t last_size;
} renderer_t;

// static void gl_check(int line) {
//     GLenum status = glGetError();
//     if(status != GL_NO_ERROR) {
//         CCERROR("OpenGL error at line %d: %x", line, status);
//     }
// }

// #define GL_CHECK() gl_check(__LINE__);
#define GL_CHECK()

renderer_t *render_new(unsigned width, unsigned height) {
    // XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);

    renderer_t *r = cc_alloc(sizeof(renderer_t));
    r->last_pos = CC_VEC2_NULL;
    r->last_size = CC_VEC2_NULL;
    r->size = CC_VEC2(width, height);

    gl_ortho(r->proj, width, height);
    
    r->quad_shader = gl_create_program(vert_shader, frag_shader);
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
    
    glUniformMatrix4fv(r->loc.pvm, 1, GL_TRUE, r->proj);

    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // XPLMBindTexture2d(0, 0);

    CCDEBUG("Map Renderer VBO: %u", r->quad_vbo);
    CCDEBUG("Map Renderer IBO: %u", r->quad_ibo);
    CCDEBUG("Map Renderer Shader: %u", r->quad_shader);
    return r;
}

 void render_delete(renderer_t *r) {
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    glDeleteProgram(r->quad_shader);
    glDeleteBuffers(1, &r->quad_vbo);
    cc_free(r);
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
    
    /*
        glTexCoord2f(0, 1); glVertex2f(x, r->size.y - (y + h));
        glTexCoord2f(0, 0); glVertex2f(x, r->size.y - y);
        glTexCoord2f(1, 0); glVertex2f(x + w, r->size.y - y);
        glTexCoord2f(1, 1); glVertex2f(x + w, r->size.y - (y + h));
    */
    
    /*
    vert[0].pos.x = -0.5;//pos.x;
    vert[0].pos.y = -0.5;//pos.y;
    vert[0].tex = (vec2f_t){0, 1};

    vert[1].pos.x = -0.5;//pos.x;
    vert[1].pos.y = 0.5;//pos.y + size.y;
    vert[1].tex = (vec2f_t){0, 0};

    vert[2].pos.x = 0.5;//pos.x + size.x;
    vert[2].pos.y = 0.5;//last_pos.y + size.y;
    vert[2].tex = (vec2f_t){1, 0};

    vert[3].pos.x = 0.5;//last_pos.x + size.x;
    vert[3].pos.y = -0.5;//last_pos.y;
    vert[3].tex = (vec2f_t){1, 1};
    
    */
    
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
    GL_CHECK();
    
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

void render_quad(
    renderer_t *r,
    unsigned tex,
    unsigned x,
    unsigned y,
    unsigned w,
    unsigned h,
    float brightness
) {
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    
#if APPLE
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindBuffer(GL_ARRAY_BUFFER, r->quad_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->quad_ibo);
    
    enable_attrib(r->loc.vtx_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, pos));
    enable_attrib(r->loc.vtx_tex0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offsetof(vertex_t, tex));
    glUseProgram(r->quad_shader);
    
    prepare_vertices(r, CC_VEC2(x, y), CC_VEC2(w, h));
    
    glUniform1f(r->loc.alpha, brightness);
    glUniform1i(r->loc.tex, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    GL_CHECK();

    glDisableVertexAttribArray(r->loc.vtx_pos);
    glDisableVertexAttribArray(r->loc.vtx_tex0);
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    XPLMBindTexture2d(0, 0);
    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
    GL_CHECK();
}
