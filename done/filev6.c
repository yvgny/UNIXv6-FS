/**
 * @file filev6.c
 * @brief Implement some methods to work with filev6
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 22 mars 2017
 *
 */
#include <stdio.h>
#include <inttypes.h>
#include "inode.h"
#include "sector.h"
#include "filev6.h"
#include "error.h"
#include "bmblock.h"
#include "string.h"

int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6, const char *buf, int len);

int to_indirect_sectors(struct unix_filesystem *u, struct filev6 *fv6);

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(u->f);
    M_REQUIRE_NON_NULL(fv6);

    if (inr >= u->s.s_isize * INODES_PER_SECTOR) {
        return ERR_BAD_PARAMETER;
    }

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
    M_REQUIRE_NON_NULL(fv6->u);
    M_REQUIRE_NON_NULL(fv6->u->f);

    if (fv6->offset >= inode_getsize(&(fv6->i_node)) || fv6->offset < 0) {
        return 0;
    }

    int sector = inode_findsector(fv6->u, &(fv6->i_node), fv6->offset / SECTOR_SIZE);
    if (sector < 0) {
        return sector;
    }
    int error = sector_read(fv6->u->f, sector, buf);

    if (error) {
        return error;
    }
    int byteRead = 0;
    int remainingByte = inode_getsize(&(fv6->i_node)) - fv6->offset;
    byteRead = remainingByte > SECTOR_SIZE ? SECTOR_SIZE : remainingByte;
    fv6->offset += byteRead;


    return byteRead;
}

int filev6_lseek(struct filev6 *fv6, int32_t offset) {
    M_REQUIRE_NON_NULL(fv6);
    if (offset < 0 || offset >= inode_getsize(&fv6->i_node)) {
        return ERR_OFFSET_OUT_OF_RANGE;
    }

    fv6->offset = offset;

    return 0;
}

int filev6_create(struct unix_filesystem *u, uint16_t mode, struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);


    memset(&fv6->i_node, 0, sizeof(struct inode));

    fv6->i_node.i_mode = mode;
    fv6->offset = 0;
    fv6->u = u;

    int error = inode_write(u, fv6->i_number, &fv6->i_node);
    if (error < 0) {
        return error;
    }

    //TODO superflu ?
    bm_set(u->ibm, fv6->i_number);

    return 0;
}

int filev6_writebytes(struct unix_filesystem *u, struct filev6 *fv6, void *buf, int len) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);

    fv6->offset = 0;
    int byte_read = 0;
    const char *byte_buf = buf;
    while (len > 0) {
        byte_read = filev6_writesector(u, fv6, &byte_buf[fv6->offset], len);
        if (byte_read < 0) {
            return byte_read;
        }
        fv6->offset += byte_read;
        len -= byte_read;
    }

    int error = inode_write(u, fv6->i_number, &fv6->i_node);
    if (error < 0) {
        return error;
    }

    return 0;
}

int filev6_writesector(struct unix_filesystem *u, struct filev6 *fv6, const char *buf, int len) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);
    int32_t i_size = inode_getsize(&fv6->i_node);
    int error = 0;
    if (0 >= len) {
        return 0;
    } else if (i_size + len >= MAX_BIG_FILE_SIZE) {
        return ERR_FILE_TOO_LARGE;
    } else if (i_size == ADDR_SMALL_LENGTH * SECTOR_SIZE) {
        error = to_indirect_sectors(u, fv6);
        if (error < 0) {
            return error;
        }
    }

    int32_t index = i_size / SECTOR_SIZE;
    uint16_t last_addr = fv6->i_node.i_addr[index];
    char byte[SECTOR_SIZE];
    memset(byte, 0, SECTOR_SIZE);
    int byte_written;
    int next;
    if (i_size % SECTOR_SIZE == 0) {
        next = bm_find_next(u->fbm);
        if (next < 0) {
            return next;
        }
        bm_set(u->fbm, next);
        memcpy(byte, buf, len > SECTOR_SIZE ? SECTOR_SIZE : len);
        byte_written = len > SECTOR_SIZE ? SECTOR_SIZE : len;
        fv6->i_node.i_addr[index] = next;
    } else {
        sector_read(u->f, last_addr, byte);
        int remaining_byte = SECTOR_SIZE - (i_size % SECTOR_SIZE);
        remaining_byte = remaining_byte < len ? remaining_byte : len;
        memcpy(&byte[i_size % SECTOR_SIZE], buf, remaining_byte);
        byte_written = remaining_byte;
        next = last_addr;
    }

    error = sector_write(u->f, next, byte);
    if (error < 0) {
        return error;
    }
    // TODO : Vérifier que index < 7
    return (error = inode_setsize(&fv6->i_node, i_size + byte_written)) ? error : byte_written;
}

int to_indirect_sectors(struct unix_filesystem *u, struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);

    // each cell take two bytes thus we divide the size by two
    uint16_t buf[SECTOR_SIZE / 2];
    memset(buf, 0, SECTOR_SIZE);

    for (int i = 0; i < ADDR_SMALL_LENGTH; ++i) {
        buf[i] = fv6->i_node.i_addr[i];
    }

    int next = bm_find_next(u->fbm);
    if (next < 0) {
        return next;
    }
    bm_set(u->fbm, next);

    int error = sector_write(u->f, next, buf);
    if (error < 0) {
        return error;
    }

    memset(fv6->i_node.i_addr, 0, sizeof(fv6->i_node.i_addr));
    fv6->i_node.i_addr[0] = next;

    error = inode_write(u, fv6->i_number, &fv6->i_node);
    if (error < 0) {
        return error;
    }

    return 0;
}

int write_big_file (struct unix_filesystem *u, struct filev6 *fv6, const char *buf, int len) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);
    M_REQUIRE_NON_NULL(buf);


}





