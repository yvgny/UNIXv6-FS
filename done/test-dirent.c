/**
 * @file test-dirent.c
 * @brief Implement a test function for direntv6
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 30 mars 2017
 *
 */
#include <stdio.h>
#include <string.h>
#include "mount.h"
#include "inode.h"
#include "filev6.h"
#include "sha.h"
#include "sector.h"
#include "direntv6.h"
#include "error.h"

/**
 * @brief run the different tests
 * @param u the filesystem (IN)
 * @return 0 if all were right or the number corresponding to the error
 */
int test(struct unix_filesystem *u) {
	direntv6_print_tree(u, 1,"/");
}
