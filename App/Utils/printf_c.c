#include <stdio.h>
#include <sys/stat.h>

extern void uartLoggerPutChar(char c);

int _write(int file, char* ptr, int len) {
  (void)file;
  for (int i = 0; i < len; i++)
    uartLoggerPutChar(ptr[i]);
  return len;
}

int _write_r(struct _reent* r, int file, const char* ptr, int len) {
  return _write(file, (char*)ptr, len);
}

int fputc(int ch, FILE* f) {
  uartLoggerPutChar((char)ch);
  return ch;
}
