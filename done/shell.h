#define SUPPORTED_OPERATIONS 13
#define INPUT_MAX_LENGTH 256

typedef int (*shell_fct)(const char*);

struct shell_map {
    const char* name;    // nom de la commande
    shell_fct fct;       // fonction réalisant la commande
    const char* help;    // description de la commande
    size_t argc;         // nombre d'arguments de la commande
    const char* args;    // description des arguments de la commande
};

int do_help(const char*);
int do_exit(const char*);
int do_quit(const char*);
int do_mkfs(const char*);
int do_mount(const char*);
int do_mkdir(const char*);
int do_lsall(const char*);
int do_add(const char*);
int do_cat(const char*);
int do_istat(const char*);
int do_inode(const char*);
int do_sha(const char*);
int do_psb(const char*);
int tokenize_input(char* input, char (*command)[256], size_t command_size);
