#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <utime.h>
#include "commands.h"
#include "utils.h"

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