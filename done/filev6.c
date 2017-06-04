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

int big_file_add_sector(struct unix_filesystem *u, struct filev6 *fv6, int32_t i_size, int32_t last_used_addr_index);

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
    int error = sector_read(fv6->u->f, (uint32_t)sector, buf);

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
    } else if (i_size + len > MAX_BIG_FILE_SIZE) {
        return ERR_FILE_TOO_LARGE;
    } else if (i_size == ADDR_SMALL_LENGTH * SECTOR_SIZE) {
        error = to_indirect_sectors(u, fv6);
        if (error < 0) {
            return error;
        }
    }
    int is_big_file = i_size >= MAX_SMALL_FILE_SIZE ? 1 : 0;
    int32_t index = i_size / SECTOR_SIZE;
    uint16_t last_addr = fv6->i_node.i_addr[index];

    if (is_big_file) {
        index = i_size / (SECTOR_SIZE * ADDRESSES_PER_SECTOR);
        uint16_t sector[ADDRESSES_PER_SECTOR];
        memset(sector, 0, sizeof(sector));

        error = sector_read(u->f, fv6->i_node.i_addr[index], sector);
        if (error < 0) {
            return error;
        }
    }

    char byte[SECTOR_SIZE];
    memset(byte, 0, SECTOR_SIZE);
    int byte_written;
    int next;
    if (i_size % SECTOR_SIZE == 0) {
        if (is_big_file) {
            next = big_file_add_sector(u, fv6, i_size, index);
            if (next < 0) {
                return next;
            }
        } else {
            next = bm_find_next(u->fbm);
            if (next < 0) {
                return next;
            }
            bm_set(u->fbm, (uint64_t)next);
            fv6->i_node.i_addr[index] = (uint16_t)next;
        }
        byte_written = len > SECTOR_SIZE ? SECTOR_SIZE : len;
        memcpy(byte, buf, (size_t)len > SECTOR_SIZE ? SECTOR_SIZE : (size_t)len);

    } else {
        sector_read(u->f, last_addr, byte);
        int remaining_byte = SECTOR_SIZE - (i_size % SECTOR_SIZE);
        remaining_byte = remaining_byte < len ? remaining_byte : len;
        memcpy(&byte[i_size % SECTOR_SIZE], buf, (size_t)remaining_byte);
        byte_written = remaining_byte;
        next = last_addr;
    }
    error = sector_write(u->f, (uint32_t)next, byte);
    if (error < 0) {
        return error;
    }

    return (error = inode_setsize(&fv6->i_node, i_size + byte_written)) ? error : byte_written;
}

int to_indirect_sectors(struct unix_filesystem *u, struct filev6 *fv6) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);

    // each cell take two bytes thus we divide the size by two
    uint16_t buf[ADDRESSES_PER_SECTOR];
    memset(buf, 0, SECTOR_SIZE);

    for (int i = 0; i < ADDR_SMALL_LENGTH; ++i) {
        buf[i] = fv6->i_node.i_addr[i];
    }

    int next = bm_find_next(u->fbm);
    if (next < 0) {
        return next;
    }

    int error = sector_write(u->f, (uint32_t)next, buf);
    if (error < 0) {
        return error;
    }

    memset(fv6->i_node.i_addr, 0, sizeof(fv6->i_node.i_addr));
    fv6->i_node.i_addr[0] = (uint16_t)next;

    error = inode_write(u, fv6->i_number, &fv6->i_node);
    if (error < 0) {
        return error;
    }
    bm_set(u->fbm, (uint64_t)next);

    return 0;
}

int big_file_add_sector(struct unix_filesystem *u, struct filev6 *fv6, int32_t i_size, int32_t last_used_addr_index) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(fv6);

    int error = 0;

    int sector_addr = bm_find_next(u->fbm);
    if (sector_addr < 0) {
        return sector_addr;
    }
    bm_set(u->fbm, (uint64_t)sector_addr);

    uint16_t buf[ADDRESSES_PER_SECTOR];
    memset(buf, 0, sizeof(buf));
    int sector_offset = i_size / SECTOR_SIZE % ADDRESSES_PER_SECTOR;

    if (i_size % (ADDRESSES_PER_SECTOR * SECTOR_SIZE) == 0) {

        int data_sector = bm_find_next(u->fbm);
        if (data_sector < 0) {
            bm_clear(u->fbm, (uint64_t)sector_addr);
            return data_sector;
        }
        buf[0] = (uint16_t)data_sector;

        fv6->i_node.i_addr[last_used_addr_index] = (uint16_t)sector_addr;
        error = inode_write(u, fv6->i_number, &fv6->i_node);
        if (error < 0) {
            bm_clear(u->fbm, (uint64_t)sector_addr);
            return error;
        }

        error = sector_write(u->f, (uint32_t)sector_addr, buf);
        if (error < 0) {
            bm_clear(u->fbm, (uint64_t)sector_addr);
            return error;
        }
        bm_set(u->fbm, (uint64_t)data_sector);

        return data_sector;
    } else {
        error = sector_read(u->f, fv6->i_node.i_addr[last_used_addr_index], buf);
        if (error < 0) {
            return error;
        }
        buf[sector_offset] = (uint16_t)sector_addr;
        error = sector_write(u->f, fv6->i_node.i_addr[last_used_addr_index], buf);
        if (error < 0) {
            return error;
        }

        return sector_addr;
    }
}
