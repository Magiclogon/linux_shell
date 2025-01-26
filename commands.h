#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_ld(char **args);
int shell_cf(char **args);
int shell_rmvf(char **args);
int shell_rmvd(char **args);
int shell_mkd(char **args);
int shell_cpy(char **args);

// Processes
int shell_kill(char **args);
int shell_lp(char **args);

// Number or commands
int num_builtin_com(void);

// Array of commands
extern char *builtin_str[];

// Array of pointers to commands.
extern int (*builtin_func[]) (char **);

#endif
