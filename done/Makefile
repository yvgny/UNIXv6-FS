LDLIBS += -lcrypto -lc
CFLAGS += -std=c99 -g  -pedantic -Wall -Wextra -Wfloat-equal -Wshadow        \
-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
-Wconversion -Wunreachable-code -Wunused-parameter

TEST_DEPENDANCIES = test-core.o error.o mount.o sector.o inode.o filev6.o sha.o direntv6.o shell.o

all: test-file test-inodes test-dirent

sector.o: sector.c sector.h

test-core.o: test-core.c

error.o: error.c error.h

test-file.o : test-file.c

test-inodes.o : test-inodes.c

test-dirent.o : test-dirent.c

mount.o : mount.h mount.c unixv6fs.h

inode.o : inode.h inode.c

filev6.o : filev6.c filev6.h

direntv6.o : direntv6.c direntv6.h

sha.o : sha.h sha.c

shell.o : shell.h shell.c

test-file: test-file.o $(TEST_DEPENDANCIES)

test-inodes: test-inodes.o $(TEST_DEPENDANCIES)

test-dirent : test-dirent.o $(TEST_DEPENDANCIES)
