#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include "utils.h"
#include "commands.h"

unsigned long get_boot_time() {

    // boot time is in the file /proc/stat
    FILE *boot_time_file = fopen(BOOT_TIME_FILE, "r");
    if(!boot_time_file) {
        perror("Couldn't open the /proc/stat file.");
        return 0;
    }

    char line[256];
    unsigned long btime;

    // Looking for line that starts with "Uid"
    while(fgets(line, sizeof(line), boot_time_file)) {
        if(strncmp(line, "btime", 5) == 0) {
            sscanf(line, "btime %lu", &btime);
            break;
        }
    }
    fclose(boot_time_file);
    return btime;
}

uid_t get_process_uid(char *pid) {
    
    // Process UID is in the file /proc/pid/status
    char status_path[256];
    FILE *status_file;

    snprintf(status_path, sizeof(status_path), "%s/%s/%s", PROC_DIR, pid, STATUS_FILE);

    // Openning the file and verifying it is open.
    status_file = fopen(status_path, "r");

    if(!status_file) {
        perror("Couldn't open the status file.\n");
        exit(EXIT_FAILURE);
    }
    
    char line[256];
    uid_t uid = -1;

    // Looking for line that starts with "Uid:"
    while(fgets(line, sizeof(line), status_file) != NULL) {
        if(strncmp(line, "Uid", 3) == 0) {
            sscanf(line, "Uid:\t%u", &uid);
            break;
        }
    }

    fclose(status_file);
    return uid;
}

char *get_name_from_uid(uid_t uid) {
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}

int is_param(char *params[], char *arg) {
    for(int i = 0; params[i] != NULL; i++) {
        if(strcmp(arg, params[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void list_directory(char *dir_path, int rec) {

    struct dirent *direc_comp;
    DIR *directory;

    if(dir_path != NULL) {
        directory = opendir(dir_path);
    } else {
        dir_path = ".";
        directory = opendir(dir_path);
    }

    if(directory == NULL) {
        printf("Directory not found.\n");
    } else {
        while((direc_comp = readdir(directory)) != NULL) {
            
            if(strcmp((direc_comp -> d_name), ".") != 0 && strcmp((direc_comp -> d_name), "..")) {

                // Color choosing.
                if((direc_comp -> d_type) == DT_DIR) {
                    printf("\033[36m%s\033[0m\n", direc_comp -> d_name);
                } else if ((direc_comp -> d_type) == DT_REG) {
                    printf("%s\n", direc_comp -> d_name);
                }
                
                // Recusrsion.
                if((direc_comp -> d_type) == DT_DIR && rec) {
                    char new_dirpath[PATH_MAX_SIZE];
                    strcpy(new_dirpath, dir_path);
                    strcat(new_dirpath, "/");
                    strcat(new_dirpath, direc_comp->d_name);
                    
                    list_directory(new_dirpath, rec);
                }
            }
        }
    }
    closedir(directory);
}

void empty_directory_rec(char *path) {
    DIR *directory;
    struct dirent *direc_comp;

    if(directory = opendir(path)) {

        while((direc_comp = readdir(directory)) != NULL) {
            if(strcmp(direc_comp -> d_name, ".") != 0 && strcmp(direc_comp -> d_name, "..") != 0) {
                
                if((direc_comp -> d_type) == DT_REG) {
                    char filepath[PATH_MAX_SIZE];
                    strcpy(filepath, path);
                    strcat(filepath, "/");
                    strcat(filepath, direc_comp->d_name);

                    remove(filepath);
                }
                else if((direc_comp -> d_type) == DT_DIR) {

                    char new_path[PATH_MAX_SIZE];
                    strcpy(new_path, path);
                    strcat(new_path, "/");
                    strcat(new_path, direc_comp -> d_name);

                    empty_directory_rec(new_path);
                }
            }
        }

        remove(path);
    }

    closedir(directory);
}

void cpy_file(char *src_path, char *des_path) {
    FILE *f_src, *f_des;

    struct stat s;

    if(stat(src_path, &s) == 0 && (s.st_mode & S_IFMT) == S_IFREG) {
        f_src = fopen(src_path, "r");

        if(f_src == NULL) {
            printf("Source not found or permission denied.\n");
            exit(EXIT_FAILURE);
        }

        f_des = fopen(des_path, "w");

        if(f_des == NULL) {
            printf("Can't create destination.\n");
            exit(EXIT_FAILURE);
        }

        char ch;
        while((ch = fgetc(f_src)) != EOF) {
            fputc(ch, f_des);
        }

        fclose(f_src);
        fclose(f_des);
    }
}

void cpy_directory(char *src_path, char *des_path) {
    DIR *directory;
    struct dirent *direc_comp;

    directory = opendir(src_path);
    if(directory == NULL) {
        printf("Directory not found.\n");
    }

    while((direc_comp = readdir(directory)) != NULL) {

        char new_path_src[PATH_MAX_SIZE];
        char new_path_des[PATH_MAX_SIZE];

        snprintf(new_path_src, PATH_MAX_SIZE, "%s/%s", src_path, direc_comp->d_name);
        snprintf(new_path_des, PATH_MAX_SIZE, "%s/%s", des_path, direc_comp->d_name);
        
        if((direc_comp -> d_type) == DT_DIR) {

            if(strcmp(direc_comp -> d_name, ".") == 0 || strcmp(direc_comp -> d_name, "..") == 0) {
                continue;
            }
            if(mkdir(new_path_des, S_IRWXU) != 0) {
                continue;
            }
            cpy_directory(new_path_src, new_path_des);
            
        }
        else if((direc_comp -> d_type) == DT_REG) {
            cpy_file(new_path_src, new_path_des);
        }
    }
    closedir(directory);
}

char *shell_read_line(void) {
    int bufsize = BUFFER_SIZE;
    int position = 0;
    char *buffer = (char*) malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "Buffer memory allocation error.");
        exit(EXIT_FAILURE);
    }

    while(1) {
        c = getchar();

        if(c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position++] = c;
        }

        if(position >= bufsize) {
            bufsize += BUFFER_SIZE;
            buffer = realloc(buffer, bufsize);

            if(!buffer) {
                fprintf(stderr, "Memory reallocation error.");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **shell_split_line(char* line) {

    int tok_bufsize = TOK_BUFFER_SIZE;
    int position = 0;
    char **tokens;
    char *current = line;
    char buffer[1024];
    int buf_index = 0;
    int in_quotes = 0;

    tokens = (char**) malloc(sizeof(char*) * tok_bufsize);
    if (!tokens) {
        fprintf(stderr, "Memory allocation for tokens failed.");
        exit(EXIT_FAILURE);
    }

    while (*current != '\0') {
        if (*current == '\"') {
            
            in_quotes = !in_quotes;
            current++; 
        } else if (!in_quotes && strchr(TOK_DELIMITER, *current) != NULL) {

            // Not quote and found delim, end curr token
            if (buf_index > 0) {
                buffer[buf_index] = '\0';
                tokens[position++] = strdup(buffer);
                buf_index = 0;

                if (position >= tok_bufsize) {
                    tok_bufsize += TOK_BUFFER_SIZE;
                    tokens = realloc(tokens, tok_bufsize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "Memory reallocation failed.");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            current++; // Jump the delim
        } else {
            // Not a delim so add it to the buffer.
            buffer[buf_index++] = *current++;
        }
    }

    // Add last token
    if (buf_index > 0) {
        buffer[buf_index] = '\0';
        tokens[position++] = strdup(buffer);
    }

    tokens[position] = NULL;
    return tokens;
}


int shell_launch(char **args) {

    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid < 0) {
        perror("Error forking");
        return 1;
    } 
    else if (pid == 0) {

        if(execvp(args[0], args) == -1) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
    }
    else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);

        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int shell_execute(char **args) {
    
    if(args[0] == NULL) {
        return 1;
    }

    // File descriptors and backing up STDIN AND STDOUT DESCRIPTORS
    int back_out_filed = dup(STDOUT_FILENO);
    int out_filed = -1;
    int in_filed = -1;


    for(int i = 0; args[i] != NULL; i++) {
        if(strcmp(args[i], ">") == 0) {
            out_filed = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(out_filed == -1) {
                perror("Couldn't open filepath.\n");
                return 1;
            }

            args[i] = NULL;
        }

        else if(strcmp(args[i], ">>") == 0) {
            out_filed = open(args[i + 1], O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
            if(out_filed == -1) {
                perror("Couldn't open filepath.\n");
                return 1;
            }
            args[i] = NULL;
        }
    }

    // Verifying if there's redirection.
    if (out_filed != -1) {
        dup2(out_filed, STDOUT_FILENO);
        close(out_filed);
    }

    for(int i = 0; i < num_builtin_com(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {

            int result = (*builtin_func[i])(args);
            // Load default FILENO
            dup2(back_out_filed, STDOUT_FILENO);
            
            close(back_out_filed);

            return result;
        }
    }

    // If it's not a builtin command.
    int result = shell_launch(args);

    // Load default FILENO
    dup2(back_out_filed, STDOUT_FILENO);
    
    close(back_out_filed);
    return result;

}

void shell_loop(void) {
    char *line;
    char **args;
    int status;

    char pwd[512];

    do {
        getcwd(pwd, sizeof(pwd));
        printf("\033[1;33m%s\033[0m -> ", pwd);
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while(status);
}