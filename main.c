#include <stdio.h>
#include "common.h"

void print_usage(const char *name) {
  printf("Usage:\n%s shader.glsl\n", name);
}

int main(int argc, char *argv[]) {
  struct file_t source;
  struct shader_t shader;
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
  /*filemon_t monitor = filemon_create(filename);*/

  while (os_window_process() != 0) {
    /*if (filemon_changed(monitor)) {
      shader_t newshader = shader_create(filename);
      if (shader_valid(newshader)) {
        shader_destroy(shader);
        shader = newshader;
      }
    }*/

    shader_paint(&shader, os_time());
    os_window_blit();
  }

  os_close_window();

  return 0;
}
