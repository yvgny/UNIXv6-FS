CFLAGS += -std=c99

all: test-print-inode

sector.o: sector.c sector.h

test-core.o: test-core.c

error.o: error.c error.h

test-print-inode.o : test-print-inode.c

mount.o : mount.h mount.c unixv6fs.h

inode.o : inode.h inode.c

test-print-inode: test-core.o error.o test-print-inode.o mount.o sector.o inode.o
