/**
 * @file direntv6.c
 * @brief accessing the UNIX v6 filesystem -- directory layer
 *
 * @author Théo Nikles & Sacha Kozma
 * @date 30 Mar 2017
 */

#include <stdint.h>
#include "unixv6fs.h"
#include "filev6.h"
#include "mount.h"
#include "error.h"

int direntv6_opendir(const struct unix_filesystem *u, uint16_t inr, struct directory_reader *d) {


}

int direntv6_readdir(struct directory_reader *d, char *name, uint16_t *child_inr) {


}

int direntv6_print_tree(const struct unix_filesystem *u, uint16_t inr, const char *prefix) {


}