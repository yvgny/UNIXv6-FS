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

int counter = 0;

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d) {
	M_REQUIRE_NON_NULL(u);
	struct filev6 fv6;
	int error = filev6_open(u, inr, &fv6);
	if(error < 0) {
		return error;
	}
	if((fv6.i_node.i_mode & IFMT) != IFDIR) {
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
	if (d->cur >=  d->last) {
		int byteRead = filev6_readblock(&d->fv6, d->dirs);
		if (byteRead <= 0) {
			return byteRead;
		}
		d->last += byteRead / sizeof(struct direntv6);
		//printf("byteRead : %d\n", byteRead);
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
	
	struct directory_reader d;
	int error = direntv6_opendir(u, inr, &d);
	if (error < 0) {
		printf("%s %s\n", SHORT_FIL_NAME, prefix);
		return error;
	}
	
	int index = strlen(prefix);
	char prefixCopy[index + 1];
	memset(prefixCopy, 0, index + 1);
	strncpy(prefixCopy, prefix, index + 1);
	strncat(prefixCopy, "/", MAXPATHLEN_UV6 - index);
	printf("%s %s\n", SHORT_DIR_NAME, prefixCopy);
	
	
	char name[DIRENT_MAXLEN + 1];
	uint16_t child_inr;
	while((error = direntv6_readdir(&d, name, &child_inr)) != 0) {
		if(error < 0) {
			return error;
		}
		strncat(prefixCopy, name, MAXPATHLEN_UV6 - index + 1);
		int returnValue = direntv6_print_tree(u, child_inr, prefixCopy);
		if(returnValue < 0) {
			memset(prefixCopy, 0, index + 1);
			strncpy(prefixCopy, prefix, index);
			strncat(prefixCopy, "/", MAXPATHLEN_UV6 - index);
		}
	}
	
	return 0;
}
