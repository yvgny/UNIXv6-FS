/**
 * @file  bmblock.c
 * @brief access the filesystem using fuse.
 *
 * @author Théo Nikles et Sacha Kozma
 * @date 28 avril 2017
 */
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "bmblock.h"
#include "error.h"

#define BM_MEMBER_SIZE 64


/**
 * Allows to free up the space used by the bitmap.
 * It should be used when a bitmap is no longer used
 *
 * @param bmblock The bitmap that should be freed up
 */
void bm_free(struct bmblock_array *bmblock);

/**
 * Print a number in a binary format, each block of eight bits being separated by a space
 *
 * @param bitmap_v The number to be displayed
 */
void print_binary(uint64_t bitmap_v);

struct bmblock_array *bm_alloc(uint64_t min, uint64_t max) {
    if (min > max) {
        return NULL;
    }
    struct bmblock_array *bmblock;
    size_t length = (size_t) ceil((max - min + 1) / (double) BM_MEMBER_SIZE);
    if (NULL == (bmblock = malloc(sizeof(struct bmblock_array) + (length - 1) * sizeof(uint64_t)))) {
        return bmblock;
    }
    memset(bmblock, 0, sizeof(*bmblock));
    bmblock->length = length;
    bmblock->min = min;
    bmblock->max = max;
    bmblock->cursor = 0;

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
    uint64_t mask = UINT64_C(1) << (relative_x % BM_MEMBER_SIZE);
    bmblock_array->bm[relative_x / BM_MEMBER_SIZE] |= mask;
}

void bm_clear(struct bmblock_array *bmblock_array, uint64_t x) {
    if (bmblock_array->min > x || bmblock_array->max < x) {
        return;
    }
    uint64_t relative_x = x - bmblock_array->min;
    uint64_t mask = (uint64_t) 1 << (relative_x % BM_MEMBER_SIZE);
    mask = ~mask;
    uint64_t relative_bitmap = relative_x / BM_MEMBER_SIZE;
    bmblock_array->bm[relative_bitmap] &= mask;
    bmblock_array->cursor = bmblock_array->cursor > relative_bitmap ? relative_bitmap : bmblock_array->cursor;
}

int bm_find_next(struct bmblock_array *bmblock_array) {
    for (uint64_t i = bmblock_array->cursor; i < bmblock_array->length; i++) {
        if (bmblock_array->bm[i] != UINT64_C(-1)) {
            for (uint64_t j = 0; j < BM_MEMBER_SIZE && bmblock_array->max - bmblock_array->min >= j + BM_MEMBER_SIZE * i; j++) {
                if (!((bmblock_array->bm[i] >> j ) & 1)) {
                    bmblock_array->cursor = i;
                    return bmblock_array->min + (uint64_t) (j + BM_MEMBER_SIZE * i);
                }
            }
        }
        bmblock_array->cursor++;
	}
    return ERR_BITMAP_FULL;
}

void bm_print(struct bmblock_array *bmblock_array) {
    puts("**********BitMap Block START**********");
    printf("length: %zu\n", bmblock_array->length);
    printf("min: %" PRIu64 "\n", bmblock_array->min);
    printf("max: %" PRIu64 "\n", bmblock_array->max);
    printf("cursor: %" PRIu64 "\n", bmblock_array->cursor);
    puts("content:");
    for (uint64_t i = 0; i < bmblock_array->length; i++) {
        printf("%d:", i);
        print_binary(bmblock_array->bm[i]);
    }
    puts("**********BitMap Block END************");
}

void print_binary(uint64_t bitmap_v) {
    for (int i = 0; i < BM_MEMBER_SIZE; i++) {
        if (i % 8 == 0) {
            printf(" ");
        }
        printf("%" PRIu64, (bitmap_v >> i) & 1);
    }
    puts("");
}
