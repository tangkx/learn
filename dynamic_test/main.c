#include <stdio.h>
#include <stdint.h>
#include <limits.h>

int main(void)
{
    size_t dst = 231;
    // size_t padding = (16 - (((size_t)dst) & 15)) & 15;
    size_t padding = (16 - (((size_t)dst) & 15)) & 15;
    printf("padding is : %ld\n", padding);
    return 0;
}