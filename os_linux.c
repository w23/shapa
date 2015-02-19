#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
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

struct filemon_t_ {
  int ifd;
  int watch;
  char filename[1];
};

filemon_t filemon_create(const char *filename) {
  const size_t size = sizeof(struct filemon_t_) + strlen(filename);
  struct filemon_t_ *f = malloc(size);
  CHECK(f != NULL, "malloc");
  strcpy(f->filename, filename);
  f->ifd = inotify_init1(IN_NONBLOCK);
  CHECK(f->ifd != -1, "inotify_init1");
  f->watch = inotify_add_watch(f->ifd, f->filename,
    IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
  return f;
}

int filemon_changed(filemon_t fin) {
  char buffer[sizeof(struct inotify_event) + NAME_MAX + 1];
  const struct inotify_event *e = (const struct inotify_event*)buffer;
  struct filemon_t_ *f = (struct filemon_t_*)(fin);
  int retval = 0;
  if (f->watch == -1) {
    f->watch = inotify_add_watch(f->ifd, f->filename,
      IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
    return (f->watch == -1) ? 0 : 1;
  }

  for (;;) {
    ssize_t rd = read(f->ifd, buffer, sizeof(buffer));
    if (rd == -1) {
      CHECK(errno == EAGAIN, "read inotify fd, errno != EAGAIN");
      break;
    }

    retval = 1;

    if (e->mask & (IN_IGNORED | IN_DELETE_SELF | IN_MOVE_SELF)) {
      if (e->mask & IN_MOVE_SELF) inotify_rm_watch(f->ifd, f->watch);
      f->watch = -1;
      retval = 0;
    }
  }

  return retval;
}

void filemon_close(filemon_t fin) {
  struct filemon_t_ *f = (struct filemon_t_*)(fin);
  if (f->watch != -1) inotify_rm_watch(f->ifd, f->watch);
  close(f->ifd);
  free(f);
}
