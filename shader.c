#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include "common.h"

static const float vertices[] = { 1.f, -3.f, 1.f, 1.f, -3.f, 1.f };
static const char vertex_shader_source[] = "attribute vec4 v;void main(){gl_Position=v;}";
static int vertex_shader;
static int width, height;
static char last_error_info_log[1024];
static GLsizei last_error_info_log_size;

const char *shader_get_last_error(int *last_error_size) {
  if (last_error_info_log_size == 0) return 0;
  *last_error_size = last_error_info_log_size;
  return last_error_info_log;
}

static GLuint create_and_compile_shader(int type, const char *source, int size) {
  GLint status;
  GLuint shader = glCreateShader(type);
  CHECK(shader != 0, "glCreateShader");
  glShaderSource(shader, 1, &source, &size);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    glGetShaderInfoLog(shader, sizeof(last_error_info_log), &last_error_info_log_size, last_error_info_log);
    glDeleteShader(shader);
    shader = 0;
  }
  return shader;
}

void shader_init() {
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  vertex_shader = create_and_compile_shader(GL_VERTEX_SHADER, vertex_shader_source, sizeof(vertex_shader_source));
  CHECK(vertex_shader != 0, "create default vertex shader");
}

void shader_resize(int new_width, int new_height) {
  width = new_width;
  height = new_height;
  glViewport(0, 0, width, height);
}

void shader_paint(const struct shader_t *shader, float time) {
  if (shader->program == 0) return;
  glUseProgram(shader->program);
  if (shader->utime != -1) glUniform1f(shader->utime, time);
  if (shader->uresolution != -1) glUniform2f(shader->uresolution, width, height);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

int shader_create(struct shader_t *shader, const char *source, int size) {
  GLint status;
  shader->program = 0;
  shader->fragment = 0;
  shader->fragment = create_and_compile_shader(GL_FRAGMENT_SHADER, source, size);
  if (shader->fragment == 0) return 0;
  shader->program = glCreateProgram();
  glAttachShader(shader->program, shader->fragment);
  glAttachShader(shader->program, vertex_shader);
  glBindAttribLocation(shader->program, 0, "v");
  glLinkProgram(shader->program);
  glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    glGetProgramInfoLog(shader->program, sizeof(last_error_info_log), &last_error_info_log_size, last_error_info_log);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->fragment);
    shader->program = 0;
    shader->fragment = 0;
    return 0;
  }
  shader->utime = glGetUniformLocation(shader->program, "uf_time");
  shader->uresolution = glGetUniformLocation(shader->program, "uv2_resolution");
  return 1;
}

void shader_destroy(struct shader_t *shader) {
  glUseProgram(0);
  glDeleteProgram(shader->program);
  glDeleteShader(shader->fragment);
  shader->program = 0;
  shader->fragment = 0;
}

