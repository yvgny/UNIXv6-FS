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

int test(struct unix_filesystem *u) {
	struct filev6 *fs = memset(&fs, 255, sizeof(fs));
	filev6_open(u, 3, fs);
	struct inode in = {0};
	int error = inode_read(fs->u, 3, &in);
	if (error != 0) {
		printf("filev6_open failed for inode #3.\n");
		return error;
	}
	printf("Printing inode #3:\n");
	inode_print(&in);
	if(in.i_mode & IFDIR) {
		printf("which is a directory.\n");
	} else {
		printf("the first sector of data of which contains:\n");
		unsigned char sector[SECTOR_SIZE + 1];
		filev6_readblock(fs, sector);
	}
	
	
    return 0;
}
