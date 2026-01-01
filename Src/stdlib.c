/**
 * @file stdlib.c
 * @brief Stub implementations for newlib/nanolib system calls
 * 
 * These functions are required when linking with newlib-nano library.
 * The newlib C library expects these POSIX system call stubs to be
 * provided by the application for low-level I/O operations.
 * 
 * Since this embedded application doesn't use file I/O or process
 * management, these are minimal stub implementations that satisfy
 * the linker without providing actual functionality.
 * 
 * @note Unused parameters are explicitly marked with (void) casts
 *       to suppress -Wunused-parameter warnings.
 */

int _close(int file)
{
    (void)file;
    return -1;
}

int _getpid(void)
{
    return -1;
}

void _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    return;
}

int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    (void)ptr;
    return len;
}
