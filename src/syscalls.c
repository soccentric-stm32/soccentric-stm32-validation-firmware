/*
 * syscalls.c - newlib syscall retargeting
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  arm-none-eabi-gcc + newlib-nano
 * SRS:     GEN-001 build support; stdout/stderr go to the VCP (GEN-003).
 *
 * No dynamic allocation: _sbrk fails (FreeRTOS heap_4 is static, section 5).
 */

#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "uart3.h"

int _write(int fd, const void *buf, size_t len)
{
    if ((fd == 1) || (fd == 2)) {
        hwval_uart3_write(buf, len);
        return (int)len;
    }
    errno = EBADF;
    return -1;
}

int _read(int fd, void *buf, size_t len)
{
    (void)buf;
    (void)len;
    (void)fd;
    errno = EBADF;
    return -1;
}

void *_sbrk(ptrdiff_t incr)
{
    (void)incr;
    errno = ENOMEM;
    return (void *)-1;
}

int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

off_t _lseek(int fd, off_t off, int whence)
{
    (void)fd;
    (void)off;
    (void)whence;
    errno = EBADF;
    return -1;
}

void _exit(int code)
{
    (void)code;
    for (;;) {
        ;
    }
}

int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _getpid(void)
{
    return 1;
}