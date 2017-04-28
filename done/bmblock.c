#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "bmblock.h"
#include "error.h"

#define BM_MEMBER_SIZE 64

void print_binary(uint64_t);

struct bmblock_array *bm_alloc(uint64_t min, uint64_t max) {
	if(min > max || min < 0) {
		return NULL;
	}
	struct bmblock_array *bmblock;
	size_t length = (size_t)ceil((max - min + 1) / (double)BM_MEMBER_SIZE);
	if (NULL == (bmblock = malloc(sizeof(struct bmblock_array) + (length - 1) * sizeof(uint64_t)))) {
		return bmblock;
	}
	memset(bmblock, 0, sizeof(*bmblock));
	bmblock->length = length;
	bmblock->min = bmblock->cursor = min;
	bmblock->max = max;
	
	return bmblock;
}

void bm_free(struct bmblock_array *bmblock) {
	free(bmblock);
}

int bm_get(struct bmblock_array *bmblock_array, uint64_t x) {
	if (bmblock_array->min > x || bmblock_array->max < x) {
		return ERR_BAD_PARAMETER;
	}
	uint64_t relative_x = x - bmblock_array->min;
	return (bmblock_array->bm[relative_x / BM_MEMBER_SIZE] >> relative_x % BM_MEMBER_SIZE) & 1;
}

void bm_set(struct bmblock_array *bmblock_array, uint64_t x) {
	if (bmblock_array->min > x || bmblock_array->max < x) {
		return;
	}
	uint64_t relative_x = x - bmblock_array->min;
	uint64_t mask = 1 << ((relative_x - bmblock_array->min) % BM_MEMBER_SIZE);
	bmblock_array->bm[relative_x / BM_MEMBER_SIZE] |= mask;
}

void bm_clear(struct bmblock_array *bmblock_array, uint64_t x) {
	if (bmblock_array->min > x || bmblock_array->max < x) {
		return;
	}
	uint64_t relative_x = x - bmblock_array->min;
	uint64_t mask = 1 << (relative_x % BM_MEMBER_SIZE);
	mask = !mask;
	bmblock_array->bm[relative_x / BM_MEMBER_SIZE] &= mask;
	bmblock_array->cursor = bmblock_array->cursor > relative_x ? relative_x : bmblock_array->cursor;
}

int bm_find_next(struct bmblock_array *bmblock_array) {
	for (int i = bmblock_array->cursor / BM_MEMBER_SIZE ; i < bmblock_array->max/ BM_MEMBER_SIZE ; i++) {
		if(bmblock_array->bm[i] != UINT64_C(-1)) {
			for(int j = bmblock_array->cursor % BM_MEMBER_SIZE ; j < BM_MEMBER_SIZE ; j++) {
				if (!((bmblock_array->bm[i] >> j) & 1)) {
					bmblock_array->cursor = j + BM_MEMBER_SIZE * i;
					return bmblock_array->cursor;
				}
			}
		}
		bmblock_array->cursor += BM_MEMBER_SIZE - (bmblock_array->cursor % BM_MEMBER_SIZE);
	}
	return ERR_BITMAP_FULL;
}

void bm_print(struct bmblock_array *bmblock_array) {
	puts("**********BitMap Block START**********");
	printf("length: %" PRIu64 "\n", bmblock_array->length);
	printf("min: %" PRIu64 "\n", bmblock_array->min);
	printf("max: %" PRIu64 "\n", bmblock_array->max);
	printf("cursor: %" PRIu64 "\n", bmblock_array->cursor);
	puts("content:");
	for(int i = 0 ; i < bmblock_array->length ; i++) {
		printf("%d:", i);
		print_binary(bmblock_array->bm[i]);
	}
	puts("**********BitMap Block END************");
}

void print_binary(uint64_t bitmap_v) {
	for(int i = 0 ; i < BM_MEMBER_SIZE ; i++) {
		if (i % 8 == 0) {
			printf(" ");
		}
		printf("%lu", (bitmap_v >> i) & 1);
	}
	puts("");
}
