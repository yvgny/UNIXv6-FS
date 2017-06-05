/**
 * @file sha.c
 * @brief Implement some useful fonctions to calculate and print some sha
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 22 mars 2017
 *
 */
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include "sha.h"
#include "filev6.h"
#include "inode.h"


void print_sha(unsigned char sha[]) {
    if (sha == NULL) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", sha[i]);
	}
    puts("");
}

void print_sha_from_content(const unsigned char *content, size_t length) {
    if (length > CONTENT_MAX_SIZE) {
        fprintf(stderr, "Cannot print SHA : content is too big (bigger than %d).\n", CONTENT_MAX_SIZE);
    }
    unsigned char sha[SHA256_DIGEST_LENGTH];
    memset(sha, 0, SHA256_DIGEST_LENGTH);
    if (SHA256(content, length, sha) == NULL) {
		return;
	}
    print_sha(sha);
}

void print_sha_inode(struct unix_filesystem *u, struct inode inode, int inr) {
    if (u == NULL || u->f == NULL || !(inode.i_mode & IALLOC) || u->s.s_isize * INODES_PER_SECTOR <= inr || inr == 0) {
        return;
    }
    printf("SHA inode %d: ", inr);

    if (inode.i_mode & IFDIR) {
        puts("no SHA for directories.");
    } else {
		struct filev6 fv6 = { .u = u, .i_number = (uint16_t)inr, .i_node = inode, .offset = 0 };
        unsigned char content[inode_getsectorsize(&inode) + 1];
        content[inode_getsectorsize(&inode)] = '\0';
		memset(content, 0, (size_t)inode_getsectorsize(&inode));
        int error = 0;
        while((error = filev6_readblock(&fv6, &content[fv6.offset])) > 0) {
            if (error < 0) {
                return;
            }
        };
        print_sha_from_content(content, (size_t)inode_getsize(&inode));
    }
}
