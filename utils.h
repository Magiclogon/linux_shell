#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 1024
#define TOK_BUFFER_SIZE 32
#define TOK_DELIMITER " \t\r\n\a"
#define PATH_MAX_SIZE 256

// For the lp command
#define BOOT_TIME_FILE "/proc/stat"
#define PROC_DIR "/proc"
#define STAT_FILE "stat"
#define STATM_FILE "statm"
#define CMDLINE_FILE "cmdline"
#define STATUS_FILE "status"


int is_param(char *params[], char *arg);

// shell_ld helpers
void list_directory(char *dir_path, int rec);

// shell_rmvd helpers
void empty_directory_rec(char *path);

// shell_cpy helpers
void cpy_file(char *source, char *destination);
void cpy_directory(char *source, char *destination);

// shell_lp helpers
unsigned long get_boot_time();
char *get_name_from_uid(uid_t uid);
uid_t get_process_uid(char *pid);

// Shell helpers
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_launch(char **args);
void shell_loop(void);

#endif
