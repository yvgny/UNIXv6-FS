/**
 * @file test-print-inode.c
 * @brief Implement a test function for inode-print
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 20 mars 2017
 *
 */
 
#include "mount.h"
#include "inode.h"

int test(struct unix_filesystem *u) {
	struct inode* in;
	inode_read(u, 3, in);
	inode_print(in);
    return 0;
}
