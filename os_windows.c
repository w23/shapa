#include <stdio.h>
#include <stdlib.h>
#include "glutils.h"
#include <shellapi.h>
#include "common.h"

static int utf8_to_wchar(WCHAR *buffer, int buffer_length, const char *string, int length) {
  return MultiByteToWideChar(CP_UTF8, 0, string, length, buffer, buffer_length);
}

static int utf8_to_wchar_length(const char *string, int length) {
  return utf8_to_wchar(NULL, 0, string, length);
}

static int wchar_to_utf8(char *buffer, int buffer_length, const WCHAR *string, int length) {
  return WideCharToMultiByte(CP_UTF8, 0, string, length, buffer, buffer_length, 0, NULL);
}

static int wchar_to_utf8_length(const WCHAR *string, int length) {
  return wchar_to_utf8(NULL, 0, string, length);
}

void report_n_abort(const char *file, int line, const char *message) {
  char caption[MAX_PATH*2];
  WCHAR wcaption[MAX_PATH*2];
  WCHAR wmessage[256];
  _snprintf_s(caption, sizeof(caption), sizeof(caption), "%s:%d", file, line);
  utf8_to_wchar(wcaption, sizeof(wcaption) / sizeof(*wcaption), caption, -1);
  utf8_to_wchar(wmessage, sizeof(wmessage) / sizeof(*wmessage), message, -1);
  MessageBox(0, wmessage, wcaption, MB_ICONSTOP);
  ExitProcess(1);
}

static LARGE_INTEGER time_freq = {0};
static LARGE_INTEGER time_start = {0};

float os_time() {
  LARGE_INTEGER now;
  if (time_start.QuadPart == 0) {
    QueryPerformanceFrequency(&time_freq);
    QueryPerformanceCounter(&time_start);
  }
  QueryPerformanceCounter(&now);
  return (float)(now.QuadPart - time_start.QuadPart) / (float)time_freq.QuadPart;
}

int file_read(struct file_t *file, const char *filename){
  WCHAR wfilename[MAX_PATH+1];
  utf8_to_wchar(wfilename, sizeof(wfilename) / sizeof(*wfilename), filename, -1);
  HANDLE h = CreateFile(wfilename, GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (h == INVALID_HANDLE_VALUE) return 0;
  DWORD read;
  BOOL rr = ReadFile(h, file->text, REASONABLE_SIZE - 1, &read, NULL);
  CloseHandle(h);
  if (rr == FALSE || read < 1) return 0;
  file->size = read;
  return 1;
}

struct filemon_t_ {
  LONG sentinel;
  HANDLE dir;
  HANDLE thread;
  DWORD filename_size;
  WCHAR *filename;
  WCHAR dirname[1];
};

static DWORD WINAPI filemon_thread_main(struct filemon_t_ *this) {
  DWORD buffer[16384];
  for (;;) {
    DWORD returned = 0;
    BOOL result = ReadDirectoryChangesW(this->dir, &buffer, sizeof(buffer), FALSE,
      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
      &returned, NULL, NULL);
    CHECK(result != 0, "ReadDirectoryChangesW");
    const char *ptr = (const char*)buffer;
    while (returned >= sizeof(FILE_NOTIFY_INFORMATION)) {
      PFILE_NOTIFY_INFORMATION notify = (PFILE_NOTIFY_INFORMATION)ptr;
      if ((notify->FileNameLength == this->filename_size)
        && (0 == memcmp(notify->FileName, this->filename, notify->FileNameLength)))
        InterlockedIncrement(&this->sentinel);
      ptr += notify->NextEntryOffset;
      returned -= notify->NextEntryOffset;
      if (notify->NextEntryOffset == 0)
        break;
    }
  }
  return 0;
}

filemon_t filemon_create(const char *filename) {
  WCHAR wfilename[MAX_PATH+1];
  WCHAR buffer[MAX_PATH+1];
  WCHAR *filepart;
  utf8_to_wchar(wfilename, sizeof(wfilename) / sizeof(*wfilename), filename, -1);
  DWORD length = GetFullPathName(wfilename, sizeof(buffer) / sizeof(*buffer), buffer, &filepart);
  if (length == 0 || length >= sizeof(buffer))
    return NULL;

  int filename_length = filepart - buffer;
  WCHAR filetmp = filepart[0];
  filepart[0] = 0;
  HANDLE dir = CreateFile(buffer,
    GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (dir == INVALID_HANDLE_VALUE)
    return NULL;
  filepart[0] = filetmp;

  struct filemon_t_ *this = (struct filemon_t_*)malloc(
    sizeof(*this) + length * sizeof(WCHAR));
  this->sentinel = 1;
  this->dir = dir;
  memcpy(this->dirname, buffer, sizeof(WCHAR) * (length + 1));
  this->filename_size = sizeof(WCHAR) * (length - filename_length);
  this->filename = this->dirname + filename_length;

  this->thread = CreateThread(NULL, 0, filemon_thread_main, this, 0, NULL);
  CHECK(this->thread != NULL, "CreateThread");
  return this;
}

int filemon_changed(filemon_t mon) {
  struct filemon_t_ *this = (struct filemon_t_*)mon;
  return 0 != InterlockedExchange(&this->sentinel, 0);
}

void filemon_close(filemon_t mon) {
  struct filemon_t_ *this = (struct filemon_t_*)mon;
  CloseHandle(this->thread);
  //CloseHandle(this->dir);
  //free(this);
}

#define GLFUNCLIST_DO(T,N) T N = NULL;
GLFUNCLIST
WGLFUNCLSIT
#undef GLFUNCLIST_DO

static PROC check_get_proc_address(const char *name, const char *error) {
  PROC ret = wglGetProcAddress(name);
  CHECK(NULL != ret, error);
  return ret;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_SIZE:
    shader_resize(lparam & 0xffff, lparam >> 16);
    break;

  case WM_KEYUP:
    if (wparam == VK_ESCAPE)
      PostQuitMessage(0);
    break;

  case WM_CLOSE:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nCmdShow;
  int argc;
  LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);

  int total_length = 0;
  for (int i = 0; i < argc; ++i)
    total_length += wchar_to_utf8_length(wargv[i], -1);

  char **argv = malloc(sizeof(char*) * (argc + 1));
  char *argvbuf = malloc(total_length);

  int offset = 0;
  for (int i = 0; i < argc; ++i) {
    argv[i] = argvbuf + offset;
    wchar_to_utf8(argv[i], total_length - offset, wargv[i], -1);
  }

  PIXELFORMATDESCRIPTOR pfd;
  WNDCLASSEX wndclass;
  int format, count;
  HWND hwnd;
  HDC hdc;
  HGLRC hglrc;
  UINT attribs[] = {
    WGL_SUPPORT_OPENGL_ARB, 1,
    WGL_DRAW_TO_WINDOW_ARB, 1,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_COLOR_BITS_ARB, 24,
    WGL_DOUBLE_BUFFER_ARB, 1,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    0
  };
  int ctxattribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
    WGL_CONTEXT_MINOR_VERSION_ARB, 1,
    0
  };
  memset(&wndclass, 0, sizeof(wndclass));
  wndclass.cbSize = sizeof(wndclass);
  wndclass.lpszClassName = TEXT("shapa");
  wndclass.lpfnWndProc = window_proc;
  wndclass.hInstance = hInstance;
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
  wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  CHECK(RegisterClassEx(&wndclass), "RegisterClass");

  hwnd = CreateWindow(
    TEXT("shapa"), TEXT("shapa"),
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    0, 0, 1280, 720, NULL, NULL, hInstance, NULL);
  CHECK(0 != hwnd, "CreateWindow failed");
  hdc = GetDC(hwnd);
  CHECK(0 != hdc, "GetDC failed");

  SetPixelFormat(hdc, 1, &pfd);
  hglrc = wglCreateContext(hdc);
  CHECK(0 != hglrc, "wglCreateContext failed");
  wglMakeCurrent(hdc, hglrc);

#define GLFUNCLIST_DO(T,N) N = (T)check_get_proc_address(#N, #N " == NULL!");
GLFUNCLIST
WGLFUNCLSIT
#undef GLFUNCLIST_DO

  wglMakeCurrent(hdc, NULL);
  wglDeleteContext(hglrc);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);

  hwnd = CreateWindow(
    TEXT("shapa"), TEXT("shapa"),
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
    0, 0, 1280, 720, NULL, NULL, hInstance, NULL);
  CHECK(0 != hwnd, "CreateWindow failed");
  hdc = GetDC(hwnd);
  CHECK(0 != hdc, "GetDC failed");

  wglChoosePixelFormatARB(hdc, attribs, NULL, 1, &format, &count);
  CHECK(-1 != format, "wglChoosePixelFormatARB");

  SetPixelFormat(hdc, format, &pfd);
  hglrc = wglCreateContextAttribsARB(hdc, NULL, ctxattribs);
  CHECK(0 != hglrc, "wglCreateContextAttribsARB failed");
  wglMakeCurrent(hdc, hglrc);

  shapa_init(argc, argv);

  ShowWindow(hwnd, SW_SHOW);
  SetForegroundWindow(hwnd);
  SetFocus(hwnd);

  for (;;) {
    MSG msg;
    while (0 != PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) {
      if (0 == GetMessage(&msg, NULL, 0, 0)) goto exit;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    shapa_process(os_time());
    SwapBuffers(hdc);
  }

exit:
  shapa_deinit();
  wglMakeCurrent(hdc, NULL);
  wglDeleteContext(hglrc);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);
  return 0;
}