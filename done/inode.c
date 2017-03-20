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
    int counter = 0;
    struct inode sector[INODES_PER_SECTOR];
	for(int s = u->s.s_inode_start ; s < u->s.s_isize + u->s.s_inode_start ; s++) {
		sector_read(u->f, s, sector);
		for(int i = 0 ; i < INODES_PER_SECTOR ; i++) {
			struct inode in = sector[i];
			if (in.i_mode & IALLOC) {
				counter++;
				printf("inode   %d (%s) len   %" PRIu32"\n", counter, (in.i_mode & IFDIR) ?
					SHORT_DIR_NAME : SHORT_FIL_NAME, inode_getsize(&in));
			}
		}
	}
	
	return 0;
}

void inode_print(const struct inode* in) {
	printf("**********FS SUPERBLOCK START**********\n");
	if(in == NULL) {
		printf("NULL ptr\n");
	} else {
		printf("i_mode: %" PRIu16 "\n", in.i_mode);
		printf("i_nlink: %" PRIu8 "\n", in.i_nlink);
		printf("i_uid: %" PRIu8 "\n", in.i_uid);
		printf("i_gid: %" PRIu8 "\n", in.gid);
		printf("i_size0: %" PRIu8 "\n", in.i_size0);
		printf("i_size1: %" PRIu16 "\n", in.i_size1);
		printf("size: %" PRIu32 "\n", inode_getsize(in));
	}
	printf("**********FS SUPERBLOCK END**********\n");
}
