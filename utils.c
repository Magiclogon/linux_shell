#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"
#include "commands.h"

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
                printf("%s\n", direc_comp -> d_name);

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
    } 
    else if (pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("Error");
        }
        exit(EXIT_FAILURE);
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
    for(int i = 0; i < num_builtin_com(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}

void shell_loop(void) {
    char *line;
    char **args;
    int status;

    char pwd[512];

    do {
        getcwd(pwd, sizeof(pwd));
        printf("%s -> ", pwd);
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while(status);
}