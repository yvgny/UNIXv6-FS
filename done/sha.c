/**
 * @file sha.c
 * @brief Implement some useful fonctions to calculate and print some sha
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 22 mars 2017
 *
 */
#include <stdio.h>
#include <openssl/sha.h>
#include "sha.h"
#include "unixv6fs.h"
#include "filev6.h"
#include "inode.h"
#include "sector.h"


void print_sha(unsigned char sha[]) {
    if (sha == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        printf("%02x", sha[i]);
    printf("\n");
}

void print_sha_from_content(const unsigned char *content, size_t length) {
    unsigned char sha[SHA256_DIGEST_LENGTH];
    print_sha(SHA256(content, length, sha));
}

void print_sha_inode(struct unix_filesystem *u, struct inode inode, int inr) {
    if (u == NULL || u->f == NULL || /*inode == NULL ||*/ !(inode.i_mode & IALLOC)) {
        return;
    }
    printf("SHA inode %d: ", inr);

    if (inode.i_mode & IFDIR) {
        printf("no SHA for directories.\n");
    } else {
		struct filev6 fv6 = { .u = u, .i_number = inr, .i_node = inode, .offset = 0 };
        unsigned char content[inode_getsectorsize(&inode)];
        //How ot test whether readblock returns an error ?
        while(filev6_readblock(&fv6, &content[fv6.offset]) > 0);
        print_sha_from_content(content, inode_getsize(&inode));
    }
}
