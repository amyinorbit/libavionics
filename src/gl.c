#include <libavionics/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <libavionics/stb_image.h>
#include <ccore/log.h>
#include <ccore/filesystem.h>
#include <ccore/memory.h>
#include <stdlib.h>


static bool check_shader(GLuint sh) {
    GLint is_compiled = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &is_compiled);
    if(is_compiled == GL_TRUE) return true;

    GLint length = 0;
    glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &length);

    char log[length + 1];
    glGetShaderInfoLog(sh, length, &length, log);
    log[length] = '\0';
    CCERROR("shader error: %s", log);
    return false;
}

static bool check_program(GLuint sh) {
    GLint is_compiled = 0;
    glGetProgramiv(sh, GL_LINK_STATUS, &is_compiled);
    if(is_compiled == GL_TRUE) return true;

    GLint length = 0;
    glGetProgramiv(sh, GL_INFO_LOG_LENGTH, &length);

    char log[length + 1];
    glGetProgramInfoLog(sh, length, &length, log);
    log[length] = '\0';
    CCERROR("shader error: %s", log);
    return false;
}

GLuint gl_create_program(const char *vertex, const char *fragment) {
    CCASSERT(vertex);
    CCASSERT(fragment);

    GLuint vert = gl_load_shader(vertex, GL_VERTEX_SHADER);
    if(!vert) return 0;

    GLuint frag = gl_load_shader(fragment, GL_FRAGMENT_SHADER);
    if(!frag) return 0;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    glDeleteShader(vert);
    glDeleteShader(frag);

    if(!check_program(prog)) {
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

GLuint gl_load_shader(const char *source, int type) {
    CCASSERT(source);
    CCASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER);
    GLint size = strlen(source);
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &source, &size);
    glCompileShader(sh);
    if(!check_shader(sh)) {
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

void gl_ortho(float proj[16], uint32_t width, uint32_t height) {
    CCASSERT(proj);
    float x_max = width -1;
    float y_max = height -1;
    float z_near = 1.0;
    float z_far = -1.0;

    proj[0] = 2.f / x_max;
    proj[1] = 0.f;
    proj[2] = 0.f;
    proj[3] = -1.f;

    proj[4] = 0.f;
    proj[5] = -2.f / y_max;
    proj[6] = 0.f;
    proj[7] = 1.f;

    proj[8] = 0.f;
    proj[9] = 0.f;
    proj[10] = 2.f / (z_far-z_near);
    proj[11] = (z_near+z_far)/(z_near-z_far);

    proj[12] = 0.f;
    proj[13] = 0.f;
    proj[14] = 0.f;
    proj[15] = 1.f;
}
