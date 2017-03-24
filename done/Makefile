LDLIBS += -lcrypto
CDFLAGS += -std=c99

all: test-file

sector.o: sector.c sector.h

test-core.o: test-core.c

error.o: error.c error.h

test-file.o : test-file.c

mount.o : mount.h mount.c unixv6fs.h

inode.o : inode.h inode.c

filev6.o : filev6.c filev6.h

sha.o : sha.h sha.c

test-file: test-core.o error.o test-file.o mount.o sector.o inode.o filev6.o sha.o
