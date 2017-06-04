/**
 * @file  sector.c
 * @brief block-level accessor function.
 *
 * @author Théo Nikles et Sacha Kozma
 * @date 16 mars 2017
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include "unixv6fs.h"

int sector_read(FILE *f, uint32_t sector, void *data) {
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);
    memset(data, 0, SECTOR_SIZE);

    int error = fseek(f, sector * SECTOR_SIZE, SEEK_SET);
    if (error < 0) {
        return ERR_IO;
    }
    size_t size = fread(data, SECTOR_SIZE, 1, f);
    if (size != 1) {
        return ERR_IO;
    }

    return 0;
}

int sector_write(FILE *f, uint32_t sector, void *data) {
    M_REQUIRE_NON_NULL(f);
    M_REQUIRE_NON_NULL(data);

    int error = fseek(f, sector * SECTOR_SIZE, SEEK_SET);
    if (error < 0) {
        return ERR_IO;
    }

    size_t size = fwrite(data, SECTOR_SIZE, 1, f);
    fflush(f);

    if (size != 1) {
        return ERR_IO;
    }

    return 0;
}
