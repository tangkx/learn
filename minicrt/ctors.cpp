#include "minicrt.h"

void run_hooks();

extern "C" void do_global_ctors() { run_hooks(); }