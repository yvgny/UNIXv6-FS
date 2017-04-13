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
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/mail//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "////hello/net/mail//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello//////net/mail//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/mailTest//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/ma//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/mail"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/mail//////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/mail//////"));
    printf("\n\n");
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/hosts.gotest"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/hosts.g"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/hosts.go/"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/hosts.go////"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "hello/net/hosts.go"));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, ""));
    printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello"));




    //printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/cgo_sockold.go"));
    return 0;
}
