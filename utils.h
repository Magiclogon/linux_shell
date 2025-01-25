#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 1024
#define TOK_BUFFER_SIZE 32
#define TOK_DELIMITER " \t\r\n\a"
#define PATH_MAX_SIZE 256

int is_param(char *params[], char *arg);
void list_directory(char *dir_path, int rec);
void empty_directory_rec(char *path);
void cpy_file(char *source, char *destination);
void cpy_directory(char *source, char *destination);
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_launch(char **args);
void shell_loop(void);

#endif
