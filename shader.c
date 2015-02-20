#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include "common.h"

static const float vertices[] = { 1.f, -3.f, 1.f, 1.f, -3.f, 1.f };
static const char vertex_shader_source[] =
"attribute vec4 v;varying vec2 vv2_pos;void main(){gl_Position=v;vv2_pos=v.xy;}";
static const char blitter_shader_source[] =
"uniform sampler2D us2_feedback;\n"
"varying vec2 vv2_pos;\n"
"void main() {\n"
"  gl_FragColor = texture2D(us2_feedback, vv2_pos * .5 + .5);\n"
"}";
static int vertex_shader;
static int width, height;
static char last_error_info_log[1024];
static GLsizei last_error_info_log_size;
static GLuint textures[2], framebuffer;
static int tsrc = 0;
static int scale = 2, twidth, theight;
static struct shader_t blitter;

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
  glGenTextures(COUNTOF(textures), textures);
  glGenFramebuffers(1, &framebuffer);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
  vertex_shader = create_and_compile_shader(GL_VERTEX_SHADER, vertex_shader_source, sizeof(vertex_shader_source));
  CHECK(vertex_shader != 0, "create default vertex shader");
  CHECK(0 != shader_create(&blitter, blitter_shader_source, sizeof(blitter_shader_source)), "create framebuffer blitter program");
  CHECK(-1 != blitter.ufeedback, "blitter program has no feedback");
}

void shader_resize(int new_width, int new_height) {
  int i;
  width = new_width;
  height = new_height;
  glGenTextures(2, textures);
  twidth = width / scale;
  theight = height / scale;
  for (i = 0; i < COUNTOF(textures); ++i) {
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
}

void shader_paint(const struct shader_t *shader, float time) {
  int tdst = (tsrc + 1) % 2;
  if (shader->program == 0) return;
  CHECK(glGetError() == 0, "gl broke");
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[tdst], 0);
  CHECK(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER), "framebuffer is not complete");
  glViewport(0, 0, twidth, theight);
  glBindTexture(GL_TEXTURE_2D, textures[tsrc]);
  glUseProgram(shader->program);
  if (shader->utime != -1) glUniform1f(shader->utime, time);
  if (shader->uresolution != -1) glUniform2f(shader->uresolution, twidth, theight);
  if (shader->ufeedback != -1) glUniform1i(shader->ufeedback, 0);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  tsrc = tdst;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  glBindTexture(GL_TEXTURE_2D, textures[tsrc]);
  glUseProgram(blitter.program);
  if (blitter.utime != -1) glUniform1f(blitter.utime, time);
  if (blitter.uresolution != -1) glUniform2f(blitter.uresolution, width, height);
  glUniform1i(blitter.ufeedback, 0);
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
  shader->ufeedback = glGetUniformLocation(shader->program, "us2_feedback");
  return 1;
}

void shader_destroy(struct shader_t *shader) {
  glUseProgram(0);
  glDeleteProgram(shader->program);
  glDeleteShader(shader->fragment);
  shader->program = 0;
  shader->fragment = 0;
}

