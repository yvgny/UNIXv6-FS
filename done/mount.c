/**
 * @file mount.c
 * @brief Used to mount the filesystem
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 16 mars 2017
 */
#include <stdio.h>
#include <string.h>
#include "mount.h"
#include "unixv6fs.h"
#include "sector.h"
#include "error.h"

int mountv6(const char *filename, struct unix_filesystem *u) {
	M_REQUIRE_NON_NULL(filename);
	M_REQUIRE_NON_NULL(u->f);
	memset(u, 0, sizeof(*u));
	u->fbm = NULL;
	u->ibm = NULL;
	uint8_t bootblock[SECTOR_SIZE];
	int error = sector_read(u->f, BOOTBLOCK_SECTOR, bootblock);
	if(error)
		return error;
	else if (bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] != BOOTBLOCK_MAGIC_NUM)
		return ERR_BADBOOTSECTOR;
	void* superblock[SECTOR_SIZE];
	sector_read(u->f, SUPERBLOCK_SECTOR, superblock);
	return 0;
}

void mountv6_print_superblock(const struct unix_filesystem *u) {
	
}

int umountv6(struct unix_filesystem *u) {
	return 0;
}
