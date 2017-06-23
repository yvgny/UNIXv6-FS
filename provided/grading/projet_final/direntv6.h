#pragma once

/**
 * @file direntv6.h
 * @brief accessing the UNIX v6 filesystem -- directory layer
 *
 * @date summer 2016
 */

#include <stdint.h>
#include "unixv6fs.h"
#include "filev6.h"
#include "mount.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXPATHLEN_UV6 1024

struct directory_reader {
    struct filev6 fv6;
    struct direntv6 dirs[DIRENTRIES_PER_SECTOR];
    uint16_t cur;
    uint16_t last;
};

/**
 * @brief opens a directory reader for the specified inode 'inr'
 * @param u the mounted filesystem
 * @param inr the inode -- which must point to an allocated directory
 * @param d the directory reader (OUT)
 * @return 0 on success; <0 on errror
 */
int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d);

/**
 * @brief return the next directory entry.
 * @param d the directory reader
 * @param name pointer to at least DIRENTMAX_LEN+1 bytes.  Filled in with the NULL-terminated string of the entry (OUT)
 * @param child_inr pointer to the inode number in the entry (OUT)
 * @return 1 on success;  0 if there are no more entries to read; <0 on error
 */
int direntv6_readdir(struct directory_reader *d, char *name, uint16_t *child_inr);

/**
 * @brief debugging routine; print a subtree (note: recursive)
 * @param u a mounted filesystem
 * @param inr the root of the subtree
 * @param prefix the prefix to the subtree
 * @return 0 on success; <0 on error
 */
int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix);

/**
 * @brief get the inode number for the given path
 * @param u a mounted filesystem
 * @param inr the root of the subtree
 * @param entry the pathname relative to the subtree
 * @return inr on success; <0 on error
 */
int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr, const char *entry);

/**
 * @brief create a new direntv6 with the given name and given mode
 * @param u a mounted filesystem
 * @param entry the path of the new entry
 * @param mode the mode of the new inode
 * @return inr on success; <0 on error
 */
int direntv6_create(struct unix_filesystem *u, const char *entry, uint16_t mode);

/**
 * @brief Tokenize a file path by separating the file name from the parent path
 * @param full_path (IN) the full path to be tokenized
 * @param parent_path (OUT) the base path (parent path) of the full path
 * @param filename  (OUT) the file name
 * @return < 0 in case of an error, 0 otherwise
 */
int tokenize_path(const char *const full_path, char *parent_path, char *filename);

#ifdef __cplusplus
}
#endif
