#include <stdio.h>
#include "bmblock.h"
#include "mount.h"

int test(struct unix_filesystem* u) {
	bm_print(u->ibm);
	// bm_print(u->fbm);

	return 0;
}

