#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define TOK_BUFFER_SIZE 32
#define TOK_DELIMITER " \t\r\n\a"
#define PATH_MAX_SIZE 256

int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_pwd(char **args);
int shell_ld(char **args);
int shell_cf(char **args);
int shell_rmvf(char **args);
int shell_rmvd(char **args);
int shell_mkd(char **args);
int shell_cpy(char **args);

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

char *builtin_str[] =  {"cd", "help", "exit", "pwd", "ld", "cf", "rmvf", "rmvd", "mkd", "cpy"};

int (*builtin_func[]) (char**) = { &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_ld, &shell_cf, &shell_rmvf, &shell_rmvd, &shell_mkd, &shell_cpy}; 

int num_builtin_com() {
    return sizeof(builtin_str) / sizeof(char*);
}

int shell_cd(char** args) {
    if(args[1] == NULL) {
        fprintf(stderr, "No such directory found.");
    } else {
        if(chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_exit(char** args) {
    return 0;
}

int shell_help(char **args) {
    printf("\n--------------------------------------------\n");
    printf("Shell BY W. HOUSNI for his personnal project\n");
    printf("You'll see the names of the available builtin commands in this shell.\n");

    for(int i = 0; i < num_builtin_com(); i++) {
        printf("%s\n", builtin_str[i]);
    }
    return 1;
}

int shell_pwd(char **args) {
    char pwd[512];

    if(getcwd(pwd, sizeof(pwd)) == NULL) {
        perror("Failed to get current directory");
    } else {
        printf("Current directory: %s\n", pwd);
    }
    return 1;
}

int shell_ld(char **args) {
    
    struct dirent *direc_comp;
    char dir_path[PATH_MAX_SIZE];
    int rec = 0;
    int i;
    
    strcpy(dir_path, ".");

    for(i = 1; args[i] != NULL; i++) {
        if(strcmp(args[i], "-r") == 0) {
            rec = 1;
        } else {
            strcpy(dir_path, args[i]);
            printf("Directory %s:\n", args[i]);
            list_directory(dir_path, rec);
        }
    }

    if(i == 1 || (i == 2 && rec == 1)) {
        list_directory(dir_path, rec);
    }
    
    return 1;
}

int shell_cf(char **args) {
    if(args[1] != NULL) {

        // File doesn't exist.
        if(!fopen(args[1], "r")) {
            if(fopen(args[1], "w")) {
                printf("File created\n");
            } else {
                printf("Directory not found\n");
            }
        }

        // File exists
        else {
            utime(args[1], NULL);
        }

    } else {
        printf("No filename was given.\n");
    }
    return 1;
}

int shell_rmvf(char **args) {
    
    struct stat s;

    if(args[1] != NULL) {
        stat(args[1], &s);
        if((s.st_mode & S_IFMT) == S_IFREG) {
            if(remove(args[1]) == 0) {
                printf("File removed.\n");
            } else {
                printf("Error occured.\n");
            }
        } 
        else if((s.st_mode & S_IFMT) == S_IFDIR) {
            printf("A directory was provided and not a file.\n");
        }
        else {
            printf("Error: Not found\n");
        }
    }
    else {
        printf("Missing argument.\n");
    }

    return 1;
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

int shell_rmvd(char **args) {

    char *params[] = {"-r", NULL};
    int r = 0;

    for(int i = 1; args[i] != NULL; i++) {
        if(strcmp(args[i], "-r") == 0) {
            r = 1;
        }
    }

    struct stat s;

    int i;
    for(i = 1; args[i] != NULL; i++) {
        stat(args[i], &s);

        if((s.st_mode & S_IFMT) == S_IFDIR) {
            if(remove(args[i]) == 0) {
                printf("Directory removed.\n");
            } else if(!r) {
                printf("Directory not empty.\n");
            }
            else if(r) {
                empty_directory_rec(args[i]);
                printf("Directory removed.\n");
            }
        }
        else if((s.st_mode & S_IFMT) == S_IFREG) {
            printf("A file was provided and not a directory.\n");
        }
        else {
            printf("Error: Not found.\n");
        }
    }

    if(i == 1 || (i == 2 && r == 1)){
        printf("No files were given.\n");
    }
    
    return 1;
}

int shell_mkd(char **args) {

    char *params[] = {"-par", NULL};

    struct stat s;
    int par = 0;

    for(int i = 1; args[i] != NULL; i++) {
        if(strcmp(args[i], "-par") == 0) {
            par = 1;
        }
    }

    int i;

    // Iterating over other args and creating folders
    for(i = 1; args[i] != NULL; i++) {
        if(is_param(params, args[i]) == 0) {
            if(stat(args[i], &s) == -1) {

                if(mkdir(args[i], S_IRWXU) == 0) {
                    printf("Directory created.\n");
                }
                else if(!par) {
                    printf("Can't create directory.\n");
                }

                // Create all previous parents <<par arguments>>
                else if(par) {
                    // Count '/' in args[1]
                    int j, c;
                    for (j=0, c=1; args[i][j]; j++) {
                        c += (args[i][j] == '/');
                    }

                    char *dirs[c];
                    char *dir;
                    int position = 0;

                    // Allocating memory for dirs
                    for(int j = 0; j <= c; j++) {
                        dirs[j] = (char*) malloc(256 * sizeof(char));
                    }
                    
                    // Absolute path?
                    if(args[i][0] == '/') {
                        position = 1;
                        strcpy(dirs[0], "/");
                    }

                    // Spliting subfolders
                    dir = strtok(args[i], "/");
                    while(dir != NULL) {
                        dirs[position++] = strdup(dir);
                        dir = strtok(NULL, "/");
                    }

                    char path[PATH_MAX_SIZE];
                    strcpy(path, dirs[0]);

                    // Creating the directories.
                    for(int j = 1; j <= c; j++) {
                        if(mkdir(path, S_IRWXU) == -1) {
                            printf("Error creating directory.\n");
                            break;
                        }
                        strcat(path, "/");
                        strcat(path, dirs[j]);
                        
                    }
                    dirs[position] = NULL;

                }
            }
            else {
                printf("Directory already exists.\n");
            }
        }
    }
    
    if(i == 1 || (i == 2 && par == 1)) {
        printf("No Path was given.");
    }

    return 1;
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

int shell_cpy(char **args) {
    
    FILE *f_src, *f_des;

    struct stat s;
    
    if(args[1] != NULL && args[2] != NULL) {

        if(stat(args[1], &s) == 0 && (s.st_mode & S_IFMT) == S_IFREG) {
            cpy_file(args[1], args[2]);
        }
        else if(stat(args[1], &s) == 0 && (s.st_mode & S_IFMT) == S_IFDIR) {
            if(mkdir(args[2], S_IRWXU) != 0) {
                exit(EXIT_FAILURE);
            }
            cpy_directory(args[1], args[2]);
        }
    }
    else {
        printf("Argument missing.\n");
    }
    return 1;

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

int main() {

    shell_loop();

    return 0;
}