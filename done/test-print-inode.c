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
	struct inode in = {0};
	inode_read(u, 5, &in);
	inode_scan_print(u);
	inode_print(&in);
    printf("inode find sector = %i\n", inode_findsector(u, &in, 8));
    return 0;
}
