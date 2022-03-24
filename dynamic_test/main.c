#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _list {
	int node;
	struct _list *next;
} list;

int
main(void) {
	// int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	// int *p0 = &arr[0];
	// int *p1 = &arr[1];
	// int *p9 = &arr[9];

	// printf("p1 - p0 : %ld \n", ((char *)p1 - (char *)p0));
	return 0;
}