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

    int error;

    fseek(f, sector * SECTOR_SIZE, SEEK_SET);
    error = fread(data, SECTOR_SIZE, 1, f);

    //TODO renvoyer une erreur si sector < 0 ?

    if (error != 1 && !feof(f)) {
        return ERR_IO;
    }

    return 0;
}
