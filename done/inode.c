/**
 * @file  inode.c
 * @brief accessing the UNIX v6 filesystem -- core of the first set of assignments.
 *
 * @author Théo Nikles et Sacha Kozma
 * @date 16 mars 2017
 */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "error.h"
#include "unixv6fs.h"
#include "inode.h"
#include "sector.h"

int inode_scan_print(const struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    int counter = 0;
    struct inode sector[INODES_PER_SECTOR];
    int error = 0;
    for (int s = u->s.s_inode_start; s < u->s.s_isize + u->s.s_inode_start; s++) {
        error = sector_read(u->f, s, sector);
        if (error != 0) {
            return error;
        }
        for (int i = 0; i < INODES_PER_SECTOR; i++) {
            struct inode in = sector[i];
            if (in.i_mode & IALLOC) {
                counter = ((s - u->s.s_inode_start) * INODES_PER_SECTOR) + i;
                printf("inode   %d (%s) len   %" PRIu32"\n", counter, (in.i_mode & IFDIR) ?
                       SHORT_DIR_NAME : SHORT_FIL_NAME, inode_getsize(&in));
            }
        }
    }

    return 0;
}

void inode_print(const struct inode* in) {
	printf("**********FS INODE START**********\n");
	if(in == NULL) {
		printf("NULL ptr\n");
	} else {
		printf("i_mode: %" PRIu16 "\n", in->i_mode);
		printf("i_nlink: %" PRIu8 "\n", in->i_nlink);
		printf("i_uid: %" PRIu8 "\n", in->i_uid);
		printf("i_gid: %" PRIu8 "\n", in->i_gid);
		printf("i_size0: %" PRIu8 "\n", in->i_size0);
		printf("i_size1: %" PRIu16 "\n", in->i_size1);
		printf("size: %" PRIu32 "\n", inode_getsize(in));
	}
	printf("**********FS INODE END**********\n");
} 	

int inode_read(const struct unix_filesystem *u, uint16_t inr, struct inode *inode) {
	M_REQUIRE_NON_NULL(u);
	M_REQUIRE_NON_NULL(inode);

	if(u->s.s_isize < inr || u->s.s_inode_start > inr) {
		return ERR_INODE_OUTOF_RANGE;
	}

	int sector = u->s.s_inode_start + (inr / INODES_PER_SECTOR);
	printf("%" PRIu16 "\n", sector);

	struct inode in[INODES_PER_SECTOR];
	sector_read(u->f, sector, in);
	*inode = in[inr - ((inr / INODES_PER_SECTOR) * INODES_PER_SECTOR)];

	if(inode->i_mode != IALLOC) {
		return ERR_UNALLOCATED_INODE;
	}
	return 0;
}
