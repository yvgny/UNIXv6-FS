/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "error.h"
#include "mount.h"
#include "direntv6.h"
#include "inode.h"

struct unix_filesystem fs;

int print_error(int);

static int arg_parse(void *, const char *, int, struct fuse_args *);

static int fs_getattr(const char *path, struct stat *stbuf);

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi);

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi);

static struct fuse_operations available_ops = {
        .getattr    = fs_getattr,
        .readdir    = fs_readdir,
        .read        = fs_read,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int ret = fuse_opt_parse(&args, NULL, NULL, arg_parse);
    if (ret == 0) {
        ret = fuse_main(args.argc, args.argv, &available_ops, NULL);
        (void) umountv6(&fs);
    }
    return ret;
}

static int arg_parse(void *data, const char *filename, int key, struct fuse_args *outargs) {
    (void) data;
    (void) outargs;
    if (key == FUSE_OPT_KEY_NONOPT && fs.f == NULL && filename != NULL) {
        int error = mountv6(filename, &fs);
        if (error < 0) {
            umountv6(&fs);
            print_error(error);
            exit(1);
        }
        return 0;
    }
    return 1;
}

static int fs_getattr(const char *path, struct stat *stbuf) {
    int res = 0, inr = 0;
    memset(stbuf, 0, sizeof(struct stat));

    inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
    if (inr < 0) {
        return print_error(inr);
    }

    struct inode i_node;
    res = inode_read(&fs, inr, &i_node);
    if (res < 0) {
        return print_error(res);
    }

    stbuf->st_ino = inr;
    stbuf->st_mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    stbuf->st_mode = stbuf->st_mode | (i_node.i_mode & IFDIR ? S_IFDIR : S_IFREG);
    stbuf->st_uid = i_node.i_uid;
    stbuf->st_gid = i_node.i_gid;
    stbuf->st_gid = inode_getsize(&i_node);
    stbuf->st_blksize = SECTOR_SIZE;
    stbuf->st_blocks = inode_getsectorsize(&i_node);

    return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
	(void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
	int inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
	if (inr < 0) {
		return print_error(inr);
	}
	struct directory_reader d;
    int error = direntv6_opendir(&fs, inr, &d);
    if (error < 0) {
		return print_error(error);
	}

    char name[DIRENT_MAXLEN + 1];
    uint16_t child_inr;
    while ((error = direntv6_readdir(&d, name, &child_inr)) != 0) {
        if (error < 0) {
            return print_error(error);
        }
        filler(buf, name, NULL, 0);
    }

    return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    (void) fi;
    
	int inr = direntv6_dirlookup(&fs, ROOT_INUMBER, path);
	if (inr < 0) {
		print_error(inr);
		return 0;
	}
	struct directory_reader d;
	direntv6_opendir(&fs, inr, &d);
	if (filev6_lseek(&d.fv6, offset) < 0) {
		return 0;
	}
	
	int byteRead;
	int totalByte = 0;
    while ((byteRead = filev6_readblock(&d.fv6, &buf[totalByte])) != 0 && totalByte + byteRead < size) {
        if (byteRead < 0) {
			print_error(byteRead);
            return totalByte;
        }
        totalByte += byteRead;
    }

    return totalByte;
}

int print_error(int error) {
    fprintf(stderr, "ERROR FS: %s.\n", ERR_MESSAGES[error - ERR_FIRST]);
    return error;
}
