/**
 * @file test-print-inode.c
 * @brief Implement a test function for inode-print
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 20 mars 2017
 *
 */
#include <stdio.h>
#include <string.h>
#include "mount.h"
#include "inode.h"
#include "filev6.h"
#include "sha.h"
#include "sector.h"

int inner_test(struct unix_filesystem *u, int inr);

int test(struct unix_filesystem *u) {
	printf("\n");
	inner_test(u, 3);
	printf("\n");
	inner_test(u, 5);
	int error;
    int counter;
    struct inode sector[INODES_PER_SECTOR];
    for (int s = u->s.s_inode_start; s < u->s.s_isize + u->s.s_inode_start; s++) {
        error = sector_read(u->f, s, sector);
        if (error != 0) {
            return error;
        }
        for (int i = 0; i < INODES_PER_SECTOR; i++) {
            struct inode in = sector[i];
            if (in.i_mode & IALLOC) {
                counter = ((s - u->s.s_inode_start) * INODES_PER_SECTOR) + i;
				print_sha_inode(u, in, counter);
            }
        }
    }
    return 0;
}

int inner_test(struct unix_filesystem *u, int inr) {
	struct filev6 fs;
	memset(&fs, 255, sizeof(fs));
	int error = filev6_open(u, inr, &fs);
	if (error) {
		printf("filev6_open failed for inode #%d.\n", inr);
		return error;
	}
	struct inode in = {0};
	error = inode_read(fs.u, inr, &in);
	if (error != 0) {
		return error;
	}
	printf("Printing inode #%d:\n", inr);
	inode_print(&in);
	if(in.i_mode & IFDIR) {
		printf("which is a directory.\n");
	} else {
		printf("the first sector of data of which contains:\n");
		unsigned char sector[SECTOR_SIZE + 1];
		filev6_readblock(&fs, sector);
		sector[SECTOR_SIZE] = '\0';
		printf("%s", sector);
	}
	return 0;
}
