#include "minicrt.h"

typedef void (*ctor_func)(void);

ctor_func crt_end[1] __attribute__((section(".ctors"))) = {(ctor_func)-1};