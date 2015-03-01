#include <stdio.h>
#include "common.h"

void print_usage(const char *name) {
  printf("Usage:\n%s shader.glsl\n", name);
}

static struct file_t source;
static struct shader_t shader = {0};
static filemon_t filemon;
static const char *filename;


void shapa_init(int argc, char *argv[]) {
  CHECK(argc > 1, "pass an input file");

  filename = argv[1];

  CHECK(0 != file_read(&source, filename), "cannot read input file");

  CHECK(0 != (filemon = filemon_create(filename)), "cannot monitor file changes");

  shader_init();
}

void shapa_process(float time) {
  if (filemon_changed(filemon) == 1) {
    if (0 != file_read(&source, filename)) {
      struct shader_t newshader;
      if (0 != shader_create(&newshader, source.text, source.size)) {
        shader_destroy(&shader);
        shader = newshader;
      } else {
        int size;
        fprintf(stderr, "shader error:\n\t%s\n", shader_get_last_error(&size));
      }
    }
  }

  shader_paint(&shader, time);
}

void shapa_deinit() {
  shader_destroy(&shader);
  filemon_close(filemon);
}
