/**
 * @file shell.h
 * @brief shell offers different methods to debug or find information on a fs
 *
 * @author Sacha Kozma & Théo Nikles
 * @date 12.04.17
 */

#define SUPPORTED_OPERATIONS 13
#define INPUT_MAX_LENGTH 256

typedef int (*shell_fct)(const char (*)[]);

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
    ERR_SHELL_LAST,
    ERR_INTERRUPT_REQ
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

void umountv6_fs(struct unix_filesystem*);

/**
 * @brief displays some helpful information for a user
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_help(const char (*args)[]);

/**
 * @brief this command allows to exit the shell
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_exit(const char (*args)[]);

/**
 * @brief this command allows to exit the shell
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_quit(const char (*args)[]);

/**
 * TODO
 */

int do_mkfs(const char (*args)[]);

/**
 * @brief mounts the filesystem
 * @param args, an array that contains the path to the filesystem we want to mount
 * @return 0 on succes; >0 on exception
 */

int do_mount(const char (*args)[]);

/**
 * TODO
 */

int do_mkdir(const char (*args)[]);

/**
 * @brief displays the full filesystem
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_lsall(const char (*args)[]);

/**
 * TODO
 */

int do_add(const char (*args)[]);

/**
 * @brief opens and displays a given file
 * @param args, an array that contains the path to the file we want to display
 * @return 0 on succes; >0 on exception
 */

int do_cat(const char (*args)[]);

/**
 * @brief displays some informations about a given inode
 * @param args, an array that contains the number of the inode that we want to display
 * @return 0 on succes; >0 on exception
 */

int do_istat(const char (*args)[]);

/**
 * @brief finds and print the number of the inode that we are looking for
 * @param args, an array that contains the path to the inode we want to find
 * @return 0 on succes; >0 on exception
 */

int do_inode(const char (*args)[]);

/**
 * @brief print the sha of a given file
 * @param args, an array that contains the path to the file
 * @return 0 on succes; >0 on exception
 */

int do_sha(const char (*args)[]);

/**
 * @brief prints the superblock of the filesystem
 * @param args, an empty array
 * @return 0 on succes; >0 on exception
 */

int do_psb(const char (*args)[]);

/**
 * Allows to tokenize the input of the user into an array of string.
 * The different tokens must separated by at least one white space.
 *
 * @param input the input of the user in a string format
 * @param command the tokenized input
 * @param command_size the maximum number of token that should be read from the input
 * @return < 0 in case of an error, otherwise the number of token that have been scanned
 */
int tokenize_input(char *input, char (*command)[], size_t command_size);
