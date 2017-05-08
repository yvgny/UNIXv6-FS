/**
 * @file shell.h
 * @brief shell offers different methods to debug or find information on a fs
 *
 * @author Sacha Kozma & Théo Nikles
 * @date 12.04.17
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "shell.h"
#include "mount.h"
#include "sha.h"
#include "error.h"
#include "mount.h"
#include "direntv6.h"
#include "inode.h"

/*
 * An array that contains the messages corresponding to the different errors
 */

const char *const ERR_SHELL_MESSAGES[] = {
        "invalid command",
        "wrong number of arguments",
        "mount the FS before the operation",
        "cat on directory is not defined",
        "IO error: No such file or directory"
};

/*
 * An array that contains all the supported operations
 */

struct shell_map shell_cmds[SUPPORTED_OPERATIONS] = {
        {"help",  do_help,  "display this help",                                                            0, ""},
        {"exit",  do_exit,  "exit shell",                                                                   0, ""},
        {"quit",  do_quit,  "exit shell",                                                                   0, ""},
        {"mkfs",  do_mkfs,  "create a new filesystem",                                                      3, "<diskname> <#inodes> <#blocks>"},
        {"mount", do_mount, "mount the provided filesystem",                                                1, "<diskname>"},
        {"mkdir", do_mkdir, "create a new directory",                                                       1, "<dirname>"},
        {"lsall", do_lsall, "list all directories and files contained in the currently mounted filesystem", 0, ""},
        {"add",   do_add,   "add a new file",                                                               2, "<src-fullpath> <dst>"},
        {"cat",   do_cat,   "display the content of a file",                                                1, "<pathname>"},
        {"istat", do_istat, "display information about the provided inode",                                 1, "<inode_nr>"},
        {"inode", do_inode, "display the inode number of a file",                                           1, "<pathname>"},
        {"sha",   do_sha,   "display the SHA of a file",                                                    1, "<pathname>"},
        {"psb",   do_psb,   "Print SuperBlock of the currently mounted filesystem",                         0, ""}};

/*
 * The filesystem that we will work with in the shell
 */

struct unix_filesystem *u = NULL;

int main(void) {
    size_t max_argc = 0;
    for (int i = 0; i < SUPPORTED_OPERATIONS; ++i) {
        max_argc = shell_cmds[i].argc > max_argc ? shell_cmds[i].argc : max_argc;
    }

    char command[max_argc][INPUT_MAX_LENGTH];
    char input[INPUT_MAX_LENGTH];
    int error = 0;
    int number_args = 0;
    int compare = 0;
    int found = 0;


    while (!feof(stdin) && !ferror(stdin)) {
        found = 0;
        printf("shell$ ");
        if (fgets(input, INPUT_MAX_LENGTH, stdin) == NULL) {
            return ERR_IO;
        }
        input[strlen(input) - 1] = '\0';
        number_args = tokenize_input(input, command, max_argc);
        if (number_args < 0) {
            display_error(number_args);
            continue;
        }
        for (int i = 0; i < SUPPORTED_OPERATIONS && !found; ++i) {
            compare = strcmp(shell_cmds[i].name, command[0]);
            if (compare == 0) {
                found = 1;
                if (number_args - 1 != (int) shell_cmds[i].argc) {
                    display_error(ERR_INVALID_ARGS);
                    continue;
                } else {
                    error = shell_cmds[i].fct(&command[1]);
                    if (error == ERR_INTERRUPT_REQ) {
                        return 0;
                    }
                    display_error(error);
                    continue;
                }
            }
        }
        if (!found) {
            display_error(ERR_INVALID_COMMAND);
        }
    }
    umountv6_fs();

    return 0;
}

void display_error(int error) {
    if (error > 0) {
        fprintf(stderr, "ERROR SHELL: %s.\n", ERR_SHELL_MESSAGES[error - 1]);
    } else if (error < 0) {
        fprintf(stderr, "ERROR FS: %s.\n", ERR_MESSAGES[error - ERR_FIRST]);
    }
}

int create_inode(struct inode *i_node, const char *path) {
    int inr = direntv6_dirlookup(u, ROOT_INUMBER, path);
    if (inr < 0) {
        return inr;
    }
    int error = inode_read(u, inr, i_node);
    if (error) {
        return error;
    }
    return inr;
}

int umountv6_fs(void) {
    if (u != NULL) {
        umountv6(u);
        free(u);
        u = NULL;
    }

    return 0;
}

int do_help(args_list args) {
    for (int i = 0; i < SUPPORTED_OPERATIONS; i++) {
        printf("- %s: %s.\n", shell_cmds[i].name, shell_cmds[i].help);
    }
    return 0;
}

int do_exit(args_list args) {
    umountv6_fs();
    return ERR_INTERRUPT_REQ;
}

int do_quit(args_list args) {
    return do_exit(args);
}

int do_mkfs(args_list args) {
    return 0;
}

int do_mount(args_list args) {
    umountv6_fs();
    printf("%s\n", args[0]);
    u = malloc(sizeof(struct unix_filesystem));
    int error = mountv6(args[0], u);
    if (error < 0) {
        umountv6_fs();
        return error;
    }
    return 0;
}

int do_mkdir(args_list args) {
    return 0;
}

int do_lsall(args_list args) {
    if (u == NULL) {
        return ERR_FS_UNMOUNTED;
    }
    return direntv6_print_tree(u, ROOT_INUMBER, "");
}

int do_add(args_list args) {
    return 0;
}

int do_cat(args_list args) {
    if (u == NULL) {
        return ERR_FS_UNMOUNTED;
    }
    struct inode i_node;
    int error = create_inode(&i_node, args[0]);

    if (error < 0) {
        return error;
    } else if (i_node.i_mode & IFDIR) {
        return ERR_CAT_OPERATION;
    }

    struct filev6 file;
    error = filev6_open(u, error, &file);
    if (error < 0) {
        return error;
    }
    char buffer[SECTOR_SIZE + 1];
    buffer[SECTOR_SIZE] = '\0';
    while ((error = filev6_readblock(&file, buffer)) != 0) {
        if (error < 0) {
            return error;
        }
        printf("%s", buffer);
    }

    return 0;
}

int do_istat(args_list args) {
    if (NULL == u) {
        return ERR_FS_UNMOUNTED;
    }
    struct inode i_node;
    int inr;
    int error = sscanf(args[0], "%d", &inr);
    if (error != 1 || inr < 0) {
        return ERR_INODE_OUTOF_RANGE;
    }
    error = inode_read(u, inr, &i_node);
    if (error) {
        return error;
    }
    inode_print(&i_node);
    return 0;
}

int do_inode(args_list args) {
    if (u == NULL) {
        return ERR_FS_UNMOUNTED;
    }
    struct inode i_node;
    int inr = create_inode(&i_node, args[0]);
    if (inr < 0) {
        return inr;
    }
    printf("inode: %d\n", inr);
    return 0;
}

int do_sha(args_list args) {
    if (u == NULL) {
        return ERR_FS_UNMOUNTED;
    }
    struct inode i_node;
    int inr = create_inode(&i_node, args[0]);
    if (inr < 0) {
        return inr;
    }
    print_sha_inode(u, i_node, inr);
    return 0;
}

int do_psb(args_list args) {
    if (u == NULL) {
        return ERR_FS_UNMOUNTED;
    }
    mountv6_print_superblock(u);
    return 0;
}

int tokenize_input(char *input, char (*command)[INPUT_MAX_LENGTH], size_t command_size) {
    M_REQUIRE_NON_NULL(input);
    M_REQUIRE_NON_NULL(command);

    int index = 0;
    const char *maxInput = input + strlen(input);
    char space = ' ';
    char *c = strtok(input, &space);

    do {
        while (input < maxInput && isspace(*input)) {
            input++;
        }
        c = strtok(NULL, &space);
        strcpy(command[index++], input);
        input = c;
    } while (input != NULL && index < command_size);

    return index;
}




