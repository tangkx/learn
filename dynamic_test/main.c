#include <stdio.h>
#include <setjmp.h>

__thread int number;
jmp_buf b;
void f()
{
    longjmp(b, 0);
}

int main(void)
{
    if(setjmp(b))
        printf("World!\n");
    else
    {
        printf("Hello ");
        f();
    }

    return 0;
}