#define CHECK(cond, errmsg) if (!(cond)) report_n_abort(__FILE__, __LINE__, errmsg);

void report_n_abort(const char *file, int line, const char *message);

#define REASONABLE_SIZE 65536

float os_time();

struct file_t {
  int size;
  char text[REASONABLE_SIZE];
};
int file_read(struct file_t *file, const char *filename);

typedef void *filemon_t;
filemon_t filemon_create(const char *filename);
int filemon_changed(filemon_t);
void filemon_close(filemon_t);

void os_open_window();
int os_window_process();
void os_window_blit();
void os_close_window();

struct shader_t {
  int program;
  int fragment;
  int utime, uresolution;
};

const char *shader_get_last_error(int *last_error_size);
void shader_init();
void shader_resize(int new_width, int new_height);
void shader_paint(const struct shader_t *shader, float time);
int shader_create(struct shader_t *shader, const char *source, int size);
void shader_destroy(struct shader_t *shader);

