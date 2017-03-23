#include <stdio.h>
#include <openssl/sha.h>
#include "sha.h"
#include "unixv6fs.h"
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

static void sha_to_string(const unsigned char *SHA, char *sha_string) {
    if ((SHA == NULL) || (sha_string == NULL)) {
        return;
    }
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&sha_string[i * 2], "%02x", SHA[i]);
    }

    sha_string[2 * SHA256_DIGEST_LENGTH] = '\0';
}

void print_sha_inode(struct unix_filesystem *u, struct inode inode, int inr) {
    if (u == NULL || u->f == NULL || inode.i_mode != IALLOC) {
        return;
    }
    printf("SHA inode %d: ", inr);

    if (inode.i_mode & IFDIR) {
        printf("no SHA for directories.\n");
    } else {
        unsigned char content[inode_getsectorsize(&inode)];
        for (int i = 0; i < inode_getsectorsize(&inode); i++) {
            sector_read(u->f, inode_findsector(u, &inode, i), &content[i * SECTOR_SIZE]);
        }
        print_sha_from_content(content, inode_getsectorsize(&inode));
    }
}
