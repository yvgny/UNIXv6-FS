/**
 * @file direntv6.c
 * @brief accessing the UNIX v6 filesystem -- directory layer
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 30 Mar 2017
 */

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "direntv6.h"
#include "unixv6fs.h"
#include "filev6.h"
#include "mount.h"
#include "error.h"

#define MAXPATHLEN_UV6 1024

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
    memset(d->dirs, 0, DIRENTRIES_PER_SECTOR);
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
    if (u->s.s_isize * INODES_PER_SECTOR <= inr) {
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
    int strLength = 0;
    while (!found && (result = direntv6_readdir(&d, name, &child_inr)) != 0) {
        if (result < 0 && result != ERR_INVALID_DIRECTORY_INODE) {
            return result;
        }

        strLength = NULL == nextEntry ? strlen(&entry[index]) : (int) (nextEntry - &entry[index]);

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
        if (!(IFDIR & i_node.i_mode)) {
            return child_inr;
        }
        return ERR_INODE_OUTOF_RANGE;
    } else {
        if (IFDIR & i_node.i_mode) {
            direntv6_dirlookup_core(u, child_inr, nextEntry, size - index - strLength);
        }
        return ERR_INODE_OUTOF_RANGE;
    }
}

int direntv6_dirlookup(const struct unix_filesystem *u, uint16_t inr, const char *entry) {
	return direntv6_dirlookup_core(u, inr, entry, strlen(entry));
}



