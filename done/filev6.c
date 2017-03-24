/**
 * @file filev6.c
 * @brief Implement some methods to work with filev6
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 22 mars 2017
 *
 */
#include <stdio.h>
#include "inode.h"
#include "sector.h"
#include "filev6.h"
#include "error.h"

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6) {
    struct inode inode;
    int error = inode_read(u, inr, &inode);
    if (error) {
        return error;
    }
    fv6->u = u;
    fv6->i_number = inr;
    fv6->i_node = inode;
    fv6->offset = 0;

    return 0;
}

int filev6_readblock(struct filev6 *fv6, void *buf) {
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    if (fv6->offset >= inode_getsize(&(fv6->i_node))) {
        return 0;
    }
    
    int error = sector_read(fv6->u->f, inode_findsector(fv6->u, &(fv6->i_node), fv6->offset / SECTOR_SIZE), buf);

    if (error != 0) {
        return error;
    }
    int byteRead = 0;
    int remainingByte = inode_getsize(&(fv6->i_node)) - fv6->offset;
    if (remainingByte > SECTOR_SIZE) {
        byteRead = SECTOR_SIZE;
    } else {
        byteRead = remainingByte;
    }
    fv6->offset += byteRead;


    return byteRead;
}




