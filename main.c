#include <stdio.h>
#include "common.h"

void print_usage(const char *name) {
  printf("Usage:\n%s shader.glsl\n", name);
}

int main(int argc, char *argv[]) {
  struct file_t source;
  struct shader_t shader;
  filemon_t filemon;
  const char *filename = argv[1];
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  CHECK(0 != file_read(&source, filename), "cannot read input file");

  os_open_window();

  shader_init();

  if (0 == shader_create(&shader, source.text, source.size))
  {
    int size;
    fprintf(stderr, "shader error:\n\t%s\n", shader_get_last_error(&size));
  }

  CHECK(0 != (filemon = filemon_create(filename)), "cannot monitor file changes");

  while (os_window_process() != 0) {
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

    shader_paint(&shader, os_time());
    os_window_blit();
  }

  shader_destroy(&shader);
  filemon_close(filemon);
  os_close_window();

  return 0;
}
