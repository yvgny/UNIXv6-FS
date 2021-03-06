/**
 * @file mount.c
 * @brief Used to mount the filesystem
 *
 * @date 16 mars 2017
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "mount.h"
#include "unixv6fs.h"
#include "sector.h"
#include "error.h"

int mountv6(const char *filename, struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(u);
    memset(u, 0, sizeof(*u));
    u->f = fopen(filename, "r");
    if (u->f == NULL) {
        return ERR_IO;
    }

    u->fbm = NULL;
    u->ibm = NULL;
    uint8_t bootblock[SECTOR_SIZE];
    int error = sector_read(u->f, BOOTBLOCK_SECTOR, bootblock);
    if (error) {
        return error;
    } else if (bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] != BOOTBLOCK_MAGIC_NUM) {
        return ERR_BADBOOTSECTOR;
    }
    error = sector_read(u->f, SUPERBLOCK_SECTOR, &u->s);
    if (error) {
        return error;
    }
    return 0;
}


void mountv6_print_superblock(const struct unix_filesystem *u) {
    puts("**********FS SUPERBLOCK START**********");
    if (u == NULL) {
        puts("NULL ptr");
    } else {
        printf("s_isize             : %" PRIu16 "\n", u->s.s_isize);
        printf("s_fsize             : %" PRIu16 "\n", u->s.s_fsize);
        printf("s_fbmsize           : %" PRIu16 "\n", u->s.s_fbmsize);
        printf("s_ibmsize           : %" PRIu16 "\n", u->s.s_ibmsize);
        printf("s_inode_start       : %" PRIu16 "\n", u->s.s_inode_start);
        printf("s_block_start       : %" PRIu16 "\n", u->s.s_block_start);
        printf("s_fbm_start         : %" PRIu16 "\n", u->s.s_fbm_start);
        printf("s_ibm_start         : %" PRIu16 "\n", u->s.s_ibm_start);
        printf("s_flock             : %" PRIu8  "\n", u->s.s_flock);
        printf("s_ilock             : %" PRIu8  "\n", u->s.s_ilock);
        printf("s_fmod              : %" PRIu8  "\n", u->s.s_fmod);
        printf("s_ronly             : %" PRIu8  "\n", u->s.s_ronly);
        printf("s_time              : [%" PRIu16 "] %" PRIu16 "\n", u->s.s_time[0], u->s.s_time[1]);
    }
    puts("**********FS SUPERBLOCK END**********");

}

int umountv6(struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    if (fclose(u->f)) {
        return ERR_IO;
    }
    return 0;
}






