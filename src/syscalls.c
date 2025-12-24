#include <sys/stat.h>
/* These stubs are here to prevent compiler warnings:
 *
 * warning: _close is not implemented and will always fail
 * warning: _fstat is not implemented and will always fail
 * warning: _getpid is not implemented and will always fail
 * warning: _isatty is not implemented and will always fail
 * warning: _kill is not implemented and will always fail
 * warning: _lseek is not implemented and will always fail
 * warning: _read is not implemented and will always fail
 * warning: _write is not implemented and will always fail
 */

int __attribute__((weak)) _close(int fildes) {
   (void)fildes;
   return 0;
}

int __attribute__((weak)) _fstat(int fildes, struct stat *buf) {
   (void)fildes;
   (void)buf;
   return 0;
}

pid_t __attribute__((weak)) _getpid(void) {
   return 0;
}

int __attribute__((weak)) _isatty(int fildes) {
   (void)fildes;
   return 0;
}
int __attribute__((weak)) _kill(pid_t pid, int sig) {
   (void)pid;
   (void)sig;
   return 0;
}

off_t __attribute__((weak)) _lseek(int fildes, off_t offset, int whence) {
   (void)fildes;
   (void)offset;
   (void)whence;
   return 0;
}

ssize_t __attribute__((weak)) _read(int fildes, void *buf, size_t nbyte) {
   (void)fildes;
   (void)buf;
   (void)nbyte;
   return 0;
}

ssize_t __attribute__((weak)) _write(int fildes, const void *buf, size_t nbyte) {
   (void)fildes;
   (void)buf;
   (void)nbyte;
   return 0;
}
