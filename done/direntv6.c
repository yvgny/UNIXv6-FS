/**
 * @file direntv6.c
 * @brief accessing the UNIX v6 filesystem -- directory layer
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 30 Mar 2017
 */

#include <stdint.h>
#include <string.h>
#include "direntv6.h"
#include "error.h"
#include "inode.h"

/**
 * @brief separate the filename from the parent path, given a full pathname
 * @param full_path (IN) the full pathname
 * @param parent_path (OUT) the parent path
 * @param filename (OUT) the filename inse the parent path
 * @return <0 in case on an error , 0 otherwise
 */

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d) {
    M_REQUIRE_NON_NULL(u);
    struct filev6 fv6;
    int error = filev6_open(u, inr, &fv6);
    if (error < 0) {
        return error;
    }
    if ((fv6.i_node.i_mode & IFMT) != IFDIR) {
        return ERR_INVALID_DIRECTORY_INODE;
    }
    d->fv6 = fv6;
    memset(d->dirs, 0, DIRENTRIES_PER_SECTOR * sizeof(struct direntv6));
    d->cur = 0;
    d->last = 0;
    return 0;
}

int direntv6_readdir(struct directory_reader *d, char *name, uint16_t *child_inr) {
    M_REQUIRE_NON_NULL(d);
    M_REQUIRE_NON_NULL(d->dirs);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(child_inr);
    if (d->cur >= d->last) {
        int byteRead = filev6_readblock(&d->fv6, d->dirs);
        if (byteRead <= 0) {
            return byteRead;
        }
        d->last += byteRead / sizeof(struct direntv6);
    }
    strncpy(name, d->dirs[d->cur % (SECTOR_SIZE / sizeof(struct direntv6))].d_name, DIRENT_MAXLEN);
    name[DIRENT_MAXLEN] = '\0';

    *child_inr = d->dirs[d->cur % (SECTOR_SIZE / sizeof(struct direntv6))].d_inumber;
    d->cur += 1;

    return 1;
}

int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(prefix);
    if (u->s.s_isize * INODES_PER_SECTOR <= inr && inr > 0) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct directory_reader d;
    char prefixCopy[MAXPATHLEN_UV6];
    memset(prefixCopy, 0, MAXPATHLEN_UV6);
    int error = direntv6_opendir(u, inr, &d);
    if (error == ERR_INVALID_DIRECTORY_INODE) {
        snprintf(prefixCopy, MAXPATHLEN_UV6, "%s", prefix);
        printf("%s %s\n", SHORT_FIL_NAME, prefixCopy);
        return error;
    } else if (error < 0) {
        return error;
    }
    snprintf(prefixCopy, MAXPATHLEN_UV6, "%s/", prefix);
    printf("%s %s\n", SHORT_DIR_NAME, prefixCopy);


    char name[DIRENT_MAXLEN + 1];
    uint16_t child_inr;
    while ((error = direntv6_readdir(&d, name, &child_inr)) != 0) {
        if (error < 0) {
            return error;
        }

        snprintf(prefixCopy, MAXPATHLEN_UV6, "%s/%s", prefix, name);
        error = direntv6_print_tree(u, child_inr, prefixCopy);
        if (error < 0) {
			return error;
		}
    }

    return 0;
}

int direntv6_dirlookup_core(const struct unix_filesystem *u, uint16_t inr, const char *entry, size_t size) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);
    if (u->s.s_isize * INODES_PER_SECTOR <= inr) {
        return ERR_INODE_OUTOF_RANGE;
    }

    size_t index = 0;
    while (index < size && entry[index] == '/') {
        index++;
    }
    if (index == size) {
        return inr;
    }
    char *nextEntry = strchr(&entry[index], '/');
    struct directory_reader d;
    int error = direntv6_opendir(u, inr, &d);
    if (error) {
        return error;
    }
    char name[DIRENT_MAXLEN + 1];
    uint16_t child_inr = 0;
    int found = 0, result = 0;
    size_t strLength = 0;
    while (!found && (result = direntv6_readdir(&d, name, &child_inr)) != 0) {
        if (result < 0 && result != ERR_INVALID_DIRECTORY_INODE) {
            return result;
        }

        strLength = NULL == nextEntry ? strlen(&entry[index]) : (nextEntry - &entry[index]);

        if (strLength == strlen(name) && strncmp(&entry[index], name, strLength) == 0) {
            found = 1;
        }
    }
    if (!found) {
        return ERR_INODE_OUTOF_RANGE;
    }

    struct inode i_node;
    error = inode_read(u, child_inr, &i_node);
    if (error) {
        return error;
    }
    if (NULL == nextEntry) {
        return child_inr;
    } else {
        if (IFDIR & i_node.i_mode) {
            return direntv6_dirlookup_core(u, child_inr, nextEntry, size - index - strLength);
        }
        return ERR_INODE_OUTOF_RANGE;
    }
}

int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr, const char *entry) {
    return direntv6_dirlookup_core(u, inr, entry, strlen(entry));
}

int direntv6_create(struct unix_filesystem *u, const char *entry, uint16_t mode) {
    M_REQUIRE_NON_NULL(u);
    M_REQUIRE_NON_NULL(entry);

    char parent_path[MAXPATHLEN_UV6];
    char filename[MAXPATHLEN_UV6];

    memset(parent_path, '\0', MAXPATHLEN_UV6);
    memset(filename, '\0', MAXPATHLEN_UV6);

    if (tokenize_path(entry, parent_path, filename) < 0) {
        return ERR_BAD_PARAMETER;
    }


    size_t filename_size = strlen(filename);
    if (filename_size > DIRENT_MAXLEN) {
        return ERR_FILENAME_TOO_LONG;
    }

    int parent_inr = direntv6_dirlookup(u, ROOT_INUMBER, parent_path);
    if (parent_inr < 0) {
        return parent_inr;
    }

    int inr = direntv6_dirlookup(u, parent_inr, filename);
    if (inr > 0) {
        return ERR_FILENAME_ALREADY_EXISTS;
    }

    inr = inode_alloc(u);
    if (inr < 0) {
        return inr;
    }

    struct inode i_node;
    memset(&i_node, 0, sizeof(struct inode));
    i_node.i_mode = mode;

    int error = inode_write(u, inr, &i_node);
    if (error < 0) {
        return error;
    }

    struct direntv6 d;
    d.d_inumber = inr;
    strncpy(d.d_name, filename, DIRENT_MAXLEN);

    struct filev6 fv6;
    error = filev6_open(u, parent_inr, &fv6);
    if (error < 0) {
        return error;
    }

    error = filev6_writebytes(u, &fv6, &d, sizeof(d));
    if (error < 0) {
        return error;
    }

    return 0;
}

int tokenize_path(const char *const full_path, const char *parent_path, const char *filename) {
    M_REQUIRE_NON_NULL(full_path);

    size_t full_path_length = strlen(full_path);

    while (full_path[--full_path_length] == '/');
    full_path_length++;

    char full_path_copy[full_path_length];
    strncpy(full_path_copy, full_path, full_path_length);

    full_path_copy[full_path_length] = '\0';
    char *filename_start = strrchr(full_path_copy, '/');
    if (NULL == filename_start) {
        strncpy(filename, full_path, strlen(full_path));
        return 0;
    }

    *filename_start = '\0';
    if (parent_path != NULL) {
        strncpy(parent_path, full_path_copy, MAXPATHLEN_UV6);
    }
    if (filename != NULL) {
        strncpy(filename, filename_start + 1, MAXPATHLEN_UV6);
    }


    return 0;
}
