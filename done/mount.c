/**
 * @file mount.c
 * @brief Used to mount the filesystem
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 16 mars 2017
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "mount.h"
#include "bmblock.h"
#include "sector.h"
#include "error.h"
#include "inode.h"

void fill_ibm(struct unix_filesystem *u) {
    struct inode sector[INODES_PER_SECTOR];
    int error = 0;

    for (uint32_t s = u->s.s_inode_start; s < u->s.s_isize + u->s.s_inode_start; s++) {
        error = sector_read(u->f, s, sector);
        for (size_t i = 0; i < INODES_PER_SECTOR && error == 0; i++) {
            struct inode in = sector[i];
            if (in.i_mode & IALLOC) {
                bm_set(u->ibm, (s - u->s.s_inode_start) * INODES_PER_SECTOR + i);
            }
        }
    }
}

void fill_fbm(struct unix_filesystem *u) {
    struct inode i_node;
    int sector = 0;
    int offset = 0;

    // we use a offset of -1 in the loop to correct the "bug" that have been explained
    // by Mr.Chappelier on the Moodle
    for (uint64_t i = u->ibm->min - 1; i < u->ibm->max; i++) {
        if (bm_get(u->ibm, i)) {
            int error = inode_read(u, (uint16_t) i, &i_node);
            if (error < 0) {
                return;
            }
            if (inode_getsize(&i_node) > MAX_SMALL_FILE_SIZE) {
                for (int j = 0; j < ADDR_SMALL_LENGTH; ++j) {
                    bm_set(u->fbm, i_node.i_addr[j]);
                }
            }
            while ((sector = inode_findsector(u, &i_node, offset++)) > 0) {
                if (sector < 0) {
                    return;
                }
                bm_set(u->fbm, (uint64_t) sector);
            }
            offset = 0;
        }
    }
}

int mountv6(const char *filename, struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(filename);
    M_REQUIRE_NON_NULL(u);
    memset(u, 0, sizeof(*u));
    u->f = fopen(filename, "rb+");
    if (u->f == NULL) {
        return ERR_IO;
    }

    uint8_t bootblock[SECTOR_SIZE];
    int error = sector_read(u->f, BOOTBLOCK_SECTOR, bootblock);
    M_RETURN_IF_NEGATIVE(error);
    if (bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] != BOOTBLOCK_MAGIC_NUM) {
        return ERR_BADBOOTSECTOR;
    }
    error = sector_read(u->f, SUPERBLOCK_SECTOR, &u->s);
    M_RETURN_IF_NEGATIVE(error);

    uint16_t number_inode = (uint16_t) (u->s.s_isize * INODES_PER_SECTOR);

    u->fbm = bm_alloc(u->s.s_block_start + UINT64_C(1), u->s.s_fsize);
    u->ibm = bm_alloc(ROOT_INUMBER + 1, number_inode);

    fill_ibm(u);
    fill_fbm(u);

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
    bm_free(u->fbm);
    bm_free(u->ibm);
    return 0;
}

int mountv6_mkfs(const char *filename, uint16_t num_blocks, uint16_t num_inodes) {
    M_REQUIRE_NON_NULL(filename);

    struct superblock sb;
    memset(&sb, 0, sizeof(sb));

    sb.s_isize = num_inodes / INODES_PER_SECTOR;
    sb.s_isize += num_inodes % INODES_PER_SECTOR != 0 ? 1 : 0;

    if (num_blocks < sb.s_isize + num_inodes) {
        return ERR_NOT_ENOUGH_BLOCS;
    }
    sb.s_fsize = num_blocks;
    sb.s_inode_start = SUPERBLOCK_SECTOR + 1;
    sb.s_block_start = (uint16_t) (sb.s_inode_start + sb.s_isize);

    FILE *f = fopen(filename, "wb+");
    if (NULL == f) {
        return ERR_IO;
    }

    uint8_t bootblock[SECTOR_SIZE];
    memset(&bootblock, 0, sizeof(bootblock));

    bootblock[BOOTBLOCK_MAGIC_NUM_OFFSET] = BOOTBLOCK_MAGIC_NUM;

    int error = sector_write(f, BOOTBLOCK_SECTOR, bootblock);
    if (error < 0) {
        fclose(f);
        return error;
    }

    error = sector_write(f, SUPERBLOCK_SECTOR, &sb);
    if (error < 0) {
        fclose(f);
        return error;
    }

    struct inode sector[INODES_PER_SECTOR];
    memset(&sector, 0, sizeof(sector));

    sector[1].i_mode = IALLOC | IFDIR;
    sector[1].i_addr[0] = sb.s_block_start + 1;

    error = sector_write(f, sb.s_inode_start, sector);
    if (error < 0) {
        fclose(f);
        return error;
    }

    memset(&sector, 0, sizeof(sector));
    for (int i = sb.s_inode_start + 1; i >= 0 && i < sb.s_block_start - 1; ++i) {
        error = sector_write(f, (uint32_t) i, sector);
        if (error < 0) {
            fclose(f);
            return error;
        }
    }

    fclose(f);

    return 0;
}
