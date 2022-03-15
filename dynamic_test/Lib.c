#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Lib.h"

void foobar(int i)
{
    printf("Printing from Lib.so %d\n", i);
    sleep(-1);
}