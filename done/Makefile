CFLAGS += -std=c99

all: test-inodes

sector.o: sector.c sector.h

test-core.o: test-core.c

error.o: error.c error.h

test-inodes.o : test-inodes.c

mount.o : mount.h mount.c unixv6fs.h

test-inodes: test-core.o error.o test-inodes.o mount.o sector.o
