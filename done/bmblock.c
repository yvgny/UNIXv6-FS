#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "bmblock.h"
#include "error.h"

#define BM_MEMBER_SIZE 64

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
	bmblock->min = min;
	bmblock->max = max;
	
	return bmblock;
}

int bm_get(struct bmblock_array *bmblock_array, uint64_t x) {
	if (bmblock_array->min > x || bmblock_array->max < x) {
		return ERR_BAD_PARAMETER;
	}
	return (bmblock_array->bm[x / BM_MEMBER_SIZE] >> x % BM_MEMBER_SIZE) & 1;
}

/**
 * @brief set to true (or 1) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to set
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_set(struct bmblock_array *bmblock_array, uint64_t x) {
	
}

/**
 * @brief set to false (or 0) the bit associated to the given value
 * @param bmblock_array the array containing the value we want to clear
 * @param x an integer corresponding to the number of the value we are looking for
 */
void bm_clear(struct bmblock_array *bmblock_array, uint64_t x);

/**
 * @brief return the next unused bit
 * @param bmblock_array the array we want to search for place
 * @return <0 on failure, the value of the next unused value otherwise
 */
int bm_find_next(struct bmblock_array *bmblock_array);

/**
 * @brief usefull to see (and debug) content of a bmblock_array
 * @param bmblock_array the array we want to see
 */
void bm_print(struct bmblock_array *bmblock_array);

#ifdef __cplusplus
}
#endif
