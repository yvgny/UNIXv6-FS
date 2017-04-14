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


enum shell_error_codes {
    ERR_INVALID_COMMAND = 1,
    ERR_INVALID_ARGS,
    ERR_FS_UNMOUNTED,
    ERR_CAT_OPERATION,
    ERR_SHELL_IO,
    ERR_SHELL_LAST,
    ERR_INTERRUPT_REQ
};

void display_error(int error);

int do_help(const char (*)[]);

int do_exit(const char (*)[]);

int do_quit(const char (*)[]);

int do_mkfs(const char (*)[]);

int do_mount(const char (*)[]);

int do_mkdir(const char (*)[]);

int do_lsall(const char (*)[]);

int do_add(const char (*)[]);

int do_cat(const char (*)[]);

int do_istat(const char (*)[]);

int do_inode(const char (*)[]);

int do_sha(const char (*)[]);

int do_psb(const char (*)[]);

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