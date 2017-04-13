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
#include "shell.h"

/**
 * @brief run the different tests
 * @param u the filesystem (IN)
 * @return 0 if all were right or the number corresponding to the error
 */
int test(struct unix_filesystem *u) {
	char input[256];
	input[255] = '\0';
	char command[5][256];
	strcpy(input, "     ascj      ");
	tokenize_input(input, command, 5);
	/*for(int i = 0 ; i < 5 ; i++) {
		printf("'%s'\n", command[i]);
	}*/
    //printf("%d\n", direntv6_dirlookup(u, ROOT_INUMBER, "/hello/net/cgo_sockold.go"));
    return 0;
}
