#include <stdio.h>
#include <elf.h>

int main(int argc, char* argv[])
{
    int* p = (int*)argv;
    int i;
    Elf64_auxv_t* aux;

    printf("Argument count: %d\n", *(p-1));

    for(i = 0; i < *(p-1); ++i)
    {
        printf("Argument %d : %s\n", i, *(p+i));
    }

    p += i;
    p++;
    printf("Enviroment:\n");
    while(*p)
    {
        printf("%s\n", *p);
        p++;
    }

    p++;

    printf("Auxiliary Vectors: \n");
    aux = (Elf64_auxv_t*)p;
    while(aux->a_type != AT_NULL)
    {
        printf("Type: %02ld Value: %lx\n", aux->a_type, aux->a_un.a_val);
        aux++;
    }

    return 0;
}