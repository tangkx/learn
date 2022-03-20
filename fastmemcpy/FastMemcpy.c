#include <stdio.h>
#include "FastMemcpy.h"
#include <string.h>

#define NUM 27

int main(void)
{
    char src[NUM] = "qwertyuiopasdfghjklzxcvbnm\0";
    char dst[NUM];

    memcpy_fast(dst, src, NUM);

    printf("dst is: %s, %ld\n", dst, strlen(dst));
}