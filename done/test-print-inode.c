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
	printf("\nListing inodes SHA:\n");
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
	struct filev6 fv6;
	memset(&fv6, 255, sizeof(fv6));
	int error = filev6_open(u, inr, &fv6);
	if (error) {
		printf("filev6_open failed for inode #%d.\n", inr);
		return error;
	}
	printf("Printing inode #%d:\n", inr);
	inode_print(&(fv6.i_node));
	if((fv6.i_node).i_mode & IFDIR) {
		printf("which is a directory.\n");
	} else {
		printf("the first sector of data of which contains:\n");
		unsigned char sector[SECTOR_SIZE + 1];
		int numberByteRead = filev6_readblock(&fv6, sector);
		if (numberByteRead <= 0) {
			return error;
		}
		sector[SECTOR_SIZE] = '\0';
		printf("%s\n----\n", sector);

	}
	return 0;
}
