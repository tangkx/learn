#include <stdio.h>
#include <stdint.h>
#include <limits.h>

int main(void)
{
    uint32_t a = (12 << 24) | (34 << 16) | (56 << 8) | 78;
    unsigned char* dd = ((unsigned char*)&a) + 4;
    unsigned char* ss = ((unsigned char*)&a);


    printf("a is : %d\n", dd[-1]);
    printf("a is : %d\n", dd[-2]);
    printf("a is : %d\n", dd[-3]);
    printf("a is : %d\n", dd[-4]);

    printf("a is : %d\n", ss[0]);
    printf("a is : %d\n", ss[1]);
    printf("a is : %d\n", ss[2]);
    printf("a is : %d\n", ss[3]);
    
    return 0;
}