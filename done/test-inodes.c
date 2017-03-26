/**
 * @file test-inodes.c
 * @brief Implement a test function for the inodes
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 16 mars 2017
 *
 */
#include "mount.h"
#include "inode.h"

int test(struct unix_filesystem *u) {
    inode_scan_print(u);
    return 0;
}
