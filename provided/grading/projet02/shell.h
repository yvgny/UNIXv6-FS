/**
 * @file shell.h
 * @brief shell offers different methods to debug or find information on a fs
 *
 * @date 12.04.17
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "mount.h"
#include "sha.h"
#include "error.h"
#include "mount.h"
#include "direntv6.h"
#include "inode.h"

#define SUPPORTED_OPERATIONS 13
#define INPUT_MAX_LENGTH 256

typedef const char args_list[][INPUT_MAX_LENGTH];

typedef int (*shell_fct)(args_list);

struct shell_map {
    const char *name;    // nom de la commande
    shell_fct fct;       // fonction réalisant la commande
    const char *help;    // description de la commande
    size_t argc;         // nombre d'arguments de la commande
    const char *args;    // description des arguments de la commande
};

/*
 * enum for the different exceptions that may be thrown by the shell
 */

enum shell_error_codes {
    ERR_INVALID_COMMAND = 1,
    ERR_INVALID_ARGS,
    ERR_FS_UNMOUNTED,
    ERR_CAT_OPERATION,
    ERR_SHELL_IO,
    ERR_INTERRUPT_REQ,
    ERR_SHELL_LAST
};

/**
 * @brief method that displays the error, given an int
 * @param error, the error that was thrown
 */

void display_error(int error);

/**
 * @brief create the inode corresponding to the path given
 * @param i_node the inode that will be constructed (OUT)
 * @param path, the path to the inode we want to create (IN)
 * @return the number of the i_node on succes; <0 on exception
 */

int create_inode(struct inode* i_node,  const char* path);

/**
 * @brief unmount and free the filesystem given
 * @param u, the filesystem to unmount
 */

int umountv6_fs(void);

/**
 * @brief displays some helpful information for a user
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_help(args_list args);

/**
 * @brief this command allows to exit the shell
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_exit(args_list args);

/**
 * @brief this command allows to exit the shell
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_quit(args_list args);

/**
 * TODO
 */

int do_mkfs(args_list args);

/**
 * @brief mounts the filesystem
 * @param args, an array that contains the path to the filesystem we want to mount
 * @return 0 on succes; >0 on exception
 */

int do_mount(args_list args);

/**
 * TODO
 */

int do_mkdir(args_list args);

/**
 * @brief displays the full filesystem
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_lsall(args_list args);

/**
 * TODO
 */

int do_add(args_list args);

/**
 * @brief opens and displays a given file
 * @param args, an array that contains the path to the file we want to display
 * @return 0 on succes; >0 on exception
 */

int do_cat(args_list args);

/**
 * @brief displays some informations about a given inode
 * @param args, an array that contains the number of the inode that we want to display
 * @return 0 on succes; >0 on exception
 */

int do_istat(args_list args);

/**
 * @brief finds and print the number of the inode that we are looking for
 * @param args, an array that contains the path to the inode we want to find
 * @return 0 on succes; >0 on exception
 */

int do_inode(args_list args);

/**
 * @brief print the sha of a given file
 * @param args, an array that contains the path to the file
 * @return 0 on succes; >0 on exception
 */

int do_sha(args_list args);

/**
 * @brief prints the superblock of the filesystem
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_psb(args_list args);

/**
 * @brief Allows to tokenize the input of the user into an array of string.
 * The different tokens must separated by at least one white space.
 * @param input the input of the user in a string format
 * @param command the tokenized input
 * @param command_size the maximum number of token that should be read from the input
 * @return < 0 in case of an error, otherwise the number of token that have been scanned
 */
int tokenize_input(char *input, char (*command)[], size_t command_size);
