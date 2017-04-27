LDLIBS += -lcrypto -lc -lm
CFLAGS += -std=c99 -g  #-pedantic -Wall -Wextra -Wfloat-equal -Wshadow        \
#-Wpointer-arith -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings \
#-Wconversion -Wunreachable-code -Wunused-parameter

DEPENDANCIES = error.o mount.o sector.o inode.o filev6.o sha.o direntv6.o

all: test-file test-inodes test-dirent test-bitmap shell

sector.o: sector.c sector.h

test-core.o: test-core.c

error.o: error.c error.h

test-file.o : test-file.c

test-inodes.o : test-inodes.c

test-dirent.o : test-dirent.c

test-bitmap.o : test-bitmap.c

mount.o : mount.h mount.c unixv6fs.h

inode.o : inode.h inode.c

filev6.o : filev6.c filev6.h

direntv6.o : direntv6.c direntv6.h

sha.o : sha.h sha.c

shell.o : shell.h shell.c

bmblock.o : bmblock.h bmblock.c

test-file: test-file.o test-core.o $(DEPENDANCIES)

test-inodes: test-inodes.o test-core.o $(DEPENDANCIES)

test-dirent : test-dirent.o test-core.o $(DEPENDANCIES)

test-bitmap : test-bitmap.o test-core.o bmblock.o $(DEPENDANCIES)

shell: shell.o $(DEPENDANCIES)
