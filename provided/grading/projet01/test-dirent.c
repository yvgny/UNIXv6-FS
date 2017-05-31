/**
 * @file test-dirent.c
 * @brief Implement a test function for direntv6
 *
 * @date 30 mars 2017
 *
 */
// correcteur : [m] beaucoup d'include on dirait ...
#include <stdio.h>
#include <string.h>
#include "mount.h"
#include "inode.h"
#include "filev6.h"
#include "sha.h"
#include "sector.h"
#include "direntv6.h"
#include "error.h"
#include "shell.h"

/**
 * @brief run the different tests
 * @param u the filesystem (IN)
 * @return 0 if all were right or the number corresponding to the error
 */
int test(struct unix_filesystem *u) {
    return direntv6_print_tree(u, ROOT_INUMBER, "");
}
