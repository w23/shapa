#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "common.h"

static struct timespec start = { 0, 0 };

float os_time() {
  struct timespec ts;
  CHECK(0 == clock_gettime(CLOCK_MONOTONIC, &ts), "clock_gettime(CLOCK_MONOTONIC)");

  if (start.tv_sec == 0 && start.tv_nsec == 0) start = ts;

  return
    (ts.tv_sec - start.tv_sec) +
    (ts.tv_nsec - start.tv_nsec) / 1e9;
}

void report_n_abort(const char *file, int line, const char *message) {
  fprintf(stderr, "error @ %s:%d : %s\n", file, line, message);
  exit(-1);
}

int file_read(struct file_t *file, const char *filename) {
  ssize_t rd;
  int fd = open(filename, 0);
  if (fd == -1) {
    fprintf(stderr, "error: cannot open file \"%s\"\n", filename);
    return 0;
  }

  rd = read(fd, file->text, sizeof(file->text));
  if (rd == -1) {
    fprintf(stderr, "error: cannot read file \"%s\"\n", filename);
    close(fd);
    return 0;
  }

  if (rd == sizeof(file->text) || rd < sizeof("void main(){gl_FragColor=vec4(0.);}")) {
    fprintf(stderr, "error: file \"%s\" has unreasonable size (read %d bytes)\n",
      filename, (int)rd);
    close(fd);
    return 0;
  }

  file->size = rd;
  close(fd);
  return 1;
}
