/**
 * @file test-dirent.c
 * @brief Implement a test function for direntv6
 *
 * @date 30 mars 2017
 *
 */
#include "mount.h"
#include "direntv6.h"
#include "error.h"

/**
 * @brief run the different tests
 * @param u the filesystem (IN)
 * @return 0 if all were right or the number corresponding to the error
 */
int test(struct unix_filesystem *u) {
    M_REQUIRE_NON_NULL(u);
    return direntv6_print_tree(u, ROOT_INUMBER, "");
}
