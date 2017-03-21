#include <stdio.h>
#include "inode.h"
#include "sector.h"
#include "filev6.h"

int filev6_open(const struct unix_filesystem *u, uint16_t inr, struct filev6 *fv6) {
	struct inode* inode;
	int error = inode_read(u, inr, inode);
	if (error != 0) return error;
	fv6->u = u;
	fv6->i_number = inr;
	fv6->i_node = *inode;
	fv6->offset = 0;
	
	return 0;
}

int filev6_readblock(struct filev6 *fv6, void *buf) {
	if(fv6->offset % SECTOR_SIZE != 0) {
		fv6->offset = 0;
		return inode_getsize(&(fv6->i_node));
	}
	int error = sector_read(fv6->u->f, inode_findsector(fv6->u, &(fv6->i_node), fv6->offset), buf);
	if (error != 0) return error;
	fv6->offset += SECTOR_SIZE;
	return 0;
}




