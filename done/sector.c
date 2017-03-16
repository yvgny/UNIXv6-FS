/**
 * @file  sector.c
 * @brief block-level accessor function.
 *
 * @author Théo Nikles et Sacha Kozma
 * @date 16 mars 2017
 */

#include <stdint.h>
#include <stdio.h>
#include "unixv6fs.h"
#include "error.h"

int sector_read(FILE *f, uint32_t sector, void *data) {
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);
    int j;

    fseek(f, sector * SECTOR_SIZE, SEEK_SET);
    j = fread(data, SECTOR_SIZE, 1, f);

    if (j != 1) {
        return ERR_IO;
    }

    return 0;
}
