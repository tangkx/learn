#include "minicrt.h"

extern "C" void *malloc(unsigned int);
extern "C" void free(void *);

typedef long unsigned int size_t;

void *operator new(size_t size) { return malloc(size); }

void operator delete(void *p) { free(p); }

void *operator new[](size_t size) { return malloc(size); }

void operator delete[](void *p) { free(p); }