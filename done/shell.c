#include <stdio.h>
#include "shell.h"


struct shell_map shell_cmds[SUPPORTED_OPERATIONS] = {
	{ "help", do_help, "display this help", 0, ""},
	{ "exit", do_exit, "exit shell", 0, ""},
	{ "quit", do_quit, "exit shell", 0, ""},
	{ "mkfs", do_mkfs, "create a new filesystem", 3, "<diskname> <#inodes> <#blocks>"},
	{ "mount", do_mount, "mount the provided filesystem", 1, "<diskname>"},
	{ "mkdir", do_mkdir, "create a new directory", 1, "<dirname>"},
	{ "lsall", do_lsall, "list all directories and files contained in the currently mounted filesystem", 0, ""},
	{ "add", do_add, "add a new file", 2, "<src-fullpath> <dst>"},
	{ "cat", do_cat, "display the content of a file", 1, "<pathname>"},
	{ "istat", do_istat, "display information about the provided inode", 1, "<inode_nr>"},
	{ "inode", do_inode, "display the inode number of a file", 1, "<pathname>"},
	{ "sha", do_sha, "display the SHA of a file", 1, "<pathname>"},
	{ "psb", do_psb, "Print SuperBlock of the currently mounted filesystem", 0, ""}};

int do_help(const char* args) {
	
	return 0;
}

int do_exit(const char* args) {
	return 0;
}

int do_quit(const char* args) {
	return 0;
}

int do_mkfs(const char* args) {
	printf("%s", args);
	return 0;
}

int do_mount(const char* args) {
	return 0;
}

int do_mkdir(const char* args) {
	printf("%s", args);
	return 0;
}

int do_lsall(const char* args) {
	return 0;
}

int do_add(const char* args) {
	printf("%s", args);
	return 0;
}

int do_cat(const char* args) {
	return 0;
}

int do_istat(const char* args) {
	return 0;
}

int do_inode(const char* args) {
	return 0;
}

int do_sha(const char* args) {
	return 0;
}

int do_psb(const char* args) {
	return 0;
}
