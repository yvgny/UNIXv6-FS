/**
 * @file  inode.c
 * @brief accessing the UNIX v6 filesystem -- core of the first set of assignments.
 *
 * @author Théo Nikles et Sacha Kozma
 * @date 16 mars 2017
 */
#include <stdio.h>
#include <inttypes.h>
#include "error.h"
#include "unixv6fs.h"
#include "inode.h"
#include "sector.h"
#include "bmblock.h"

int inode_scan_print(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    struct inode sector[INODES_PER_SECTOR];
    int error = 0;
    for (int s = u->s.s_inode_start; s >= 0 && s < u->s.s_isize + u->s.s_inode_start; s++) {
        error = sector_read(u->f, (uint32_t)s, sector);
        if (error != 0) {
            return error;
        }
        for (size_t i = 0; i < INODES_PER_SECTOR; i++) {
            struct inode in = sector[i];
            if (in.i_mode & IALLOC) {
                printf("inode   %lu (%s) len   %" PRIu32"\n",
                       (((long unsigned)(s - u->s.s_inode_start) * INODES_PER_SECTOR) + i),
                       (in.i_mode & IFDIR) ? SHORT_DIR_NAME : SHORT_FIL_NAME,
                       inode_getsize(&in));
            }
        }
    }

    return 0;
}

void inode_print(const struct inode *in) {
    puts("**********FS INODE START**********");
    if (NULL == in) {
        puts("NULL ptr\n");
    } else {
        printf("i_mode: %" PRIu16 "\n", in->i_mode);
        printf("i_nlink: %" PRIu8 "\n", in->i_nlink);
        printf("i_uid: %" PRIu8 "\n", in->i_uid);
        printf("i_gid: %" PRIu8 "\n", in->i_gid);
        printf("i_size0: %" PRIu8 "\n", in->i_size0);
        printf("i_size1: %" PRIu16 "\n", in->i_size1);
        printf("size: %" PRIu32 "\n", inode_getsize(in));
    }
    puts("**********FS INODE END**********");
}

int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);
    M_REQUIRE_NON_NULL(u->f);

    if (u->s.s_isize * INODES_PER_SECTOR <= inr || inr < ROOT_INUMBER) {
        return ERR_INODE_OUTOF_RANGE;
    }

    uint32_t sec = (uint32_t)(u->s.s_inode_start + (inr / INODES_PER_SECTOR));

    struct inode sector[INODES_PER_SECTOR];
    int error = sector_read(u->f, sec, sector);
    M_RETURN_IF_NEGATIVE(error);
    *inode = sector[inr % INODES_PER_SECTOR];

    if (!(inode->i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }
    return 0;
}

int inode_write(struct unix_filesystem *u, uint16_t inr, const struct inode *inode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(inode);
    M_REQUIRE_NON_NULL(u->f);

    if (u->s.s_isize * INODES_PER_SECTOR <= inr) {
        return ERR_INODE_OUTOF_RANGE;
    }

    uint32_t sec = u->s.s_inode_start + (inr / INODES_PER_SECTOR);

    struct inode sector[INODES_PER_SECTOR];
    int error = sector_read(u->f, sec, sector);
    M_RETURN_IF_NEGATIVE(error);

    sector[inr % INODES_PER_SECTOR] = *inode;

    error = sector_write(u->f, sec, sector);
    M_RETURN_IF_NEGATIVE(error);

    return 0;
}


int inode_findsector(const struct unix_filesystem *u, const struct inode *i, int32_t file_sec_off) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(i);
    M_REQUIRE_NON_NULL(u->f);

    int32_t file_size = inode_getsize(i);
    if ((file_sec_off > file_size / SECTOR_SIZE && (file_size % SECTOR_SIZE != 0)) ||
        (file_sec_off >= file_size / SECTOR_SIZE && (file_size % SECTOR_SIZE == 0))) {
        return ERR_OFFSET_OUT_OF_RANGE;
    } else if (!(i->i_mode & IALLOC)) {
        return ERR_UNALLOCATED_INODE;
    }

    if (file_size > MAX_BIG_FILE_SIZE) {
        return ERR_FILE_TOO_LARGE;
    }

    if (file_size <= MAX_SMALL_FILE_SIZE) {
        return i->i_addr[file_sec_off];
    } else {
        int first_level = file_sec_off / ADDRESSES_PER_SECTOR;
        int second_level = file_sec_off % ADDRESSES_PER_SECTOR;
        uint16_t sector_list[ADDRESSES_PER_SECTOR];
        int error = sector_read(u->f, i->i_addr[first_level], sector_list);
        M_RETURN_IF_NEGATIVE(error);

        return sector_list[second_level];
    }

}

int inode_setsize(struct inode *inode, int new_size) {
	M_REQUIRE_NON_NULL(inode);
	if(new_size < 0) {
		return ERR_NOMEM;
	}
	
	inode->i_size0 = ((uint8_t)new_size >> 16) & 0xFF;
	inode->i_size1 = (uint16_t)(new_size & 0xFFFF);
	return 0;
}

int inode_alloc(struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);

    int inr = bm_find_next(u->ibm);
    if (inr < 0) {
        return ERR_NOMEM;
    }

    bm_set(u->ibm, (uint64_t)inr);

    return inr;
}


