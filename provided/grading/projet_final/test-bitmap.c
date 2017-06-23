#include <stdio.h>
#include "bmblock.h"
#include "error.h"

int test(void) {
	struct bmblock_array *bmblock = bm_alloc(4, 131);
	bm_print(bmblock);
	int index = bm_find_next(bmblock);
	M_RETURN_IF_NEGATIVE(index);

	printf("find_next() = %d\n", index);
	bm_set(bmblock, 4);
	bm_set(bmblock, 5);
	bm_set(bmblock, 6);
	bm_print(bmblock);
	index = bm_find_next(bmblock);
	M_RETURN_IF_NEGATIVE(index);

	printf("find_next() = %d\n", index);
	for(int i = 4 ; i <= 130 ; i+= 3) {
		bm_set(bmblock, i);
	}
	bm_print(bmblock);
	index = bm_find_next(bmblock);
	M_RETURN_IF_NEGATIVE(index);

	printf("find_next() = %d\n", index);
	for(int i = 5 ; i <= 130 ; i+= 5) {
		bm_clear(bmblock, i);
	}
	bm_print(bmblock);
	index = bm_find_next(bmblock);
	M_RETURN_IF_NEGATIVE(index);

	printf("find_next() = %d\n", index);
	return 0;
}
