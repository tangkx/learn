#ifndef __MINI_CRT_H__
#define __MINI_CRT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

// malloc
int mini_crt_heap_init();
static int brk(void *end_data_segment);
void *malloc(unsigned size);
void free(void *ptr);

// file and IO
typedef int FILE;
#define EOF (-1)

#define stdin ((FILE *)0)
#define stdout ((FILE *)1)
#define stderr ((FILE *)2)

int mini_crt_io_init();
FILE *fopen(const char *filename, const char *mode);
int fread(void *buffer, int size, int count, FILE *stream);
int fwrite(const void *buffer, int size, int count, FILE *stream);
int fclose(FILE *fp);
int fseek(FILE *fp, int offset, int set);

// string
char *itoa(int n, char *str, int radix);
int strcmp(const char *src, const char *dst);
char *strcpy(char *dest, const char *src);
unsigned strlen(const char *str);

// printf
#define va_list char *
#define va_start(ap, arg) (ap = (va_list)&arg + sizeof(arg))
#define va_arg(ap, t) (*(t *)((ap += sizeof(t)) - sizeof(t)))
#define va_end(ap) (ap = (va_list)0)

int fputc(int c, FILE *stream);
int fputs(const char *str, FILE *stream);
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list arglist);

// internal
void do_global_ctors();
void mini_crt_call_exit_routine();

// atexit
typedef void (*atexit_func_t)(void);
int atexit(atexit_func_t func);
typedef void (*cxa_func_t)(void *);
int __cxa_atexit(cxa_func_t func, void *arg, void *);

#ifdef __cplusplus
}
#endif

#endif