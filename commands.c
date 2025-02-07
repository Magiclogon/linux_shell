#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <utime.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include "commands.h"
#include "utils.h"

char *builtin_str[] =  {"cd", "help", "exit", "pwd", "ld", "cf", "rmvf", "rmvd", "mkd", "cpy", "kill", "lp", "sc", "schead"};

int (*builtin_func[]) (char**) = { &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_ld, &shell_cf, &shell_rmvf, &shell_rmvd, &shell_mkd, &shell_cpy, &shell_kill, &shell_lp, &shell_sc, &shell_schead}; 

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

    int i;
    for(i = 1; args[i] != NULL; i++) {
        stat(args[i], &s);
        if((s.st_mode & S_IFMT) == S_IFREG) {
            if(remove(args[i]) == 0) {
                printf("File removed.\n");
            } else {
                printf("Error occured.\n");
            }
        } 
        else if((s.st_mode & S_IFMT) == S_IFDIR) {
            printf("A directory was provided and not a file: %s\n", args[i]);
        }
        else {
            printf("Error: Not found: %s\n", args[i]);
        }
    }

    if(i == 1) {
        printf("No argument was given.\n");
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

int shell_kill(char **args) {
     
    int i;
    for(i = 1; args[i] != NULL; i++) {

        char *endptr;
        errno = 0;

        long pid_value = strtol(args[i], &endptr, 10);

        if((errno != 0 || *endptr != '\0' || pid_value < 0)) {
            printf("Invalid pid: %s\n", args[i]);
            continue;
        }

        pid_t pid = (pid_t) pid_value;

        if(kill(pid, SIGKILL) == 0) {
            printf("Process with pid: %s was killed.\n", args[i]);
        }
        else {
            printf("Couldn't kill process with pid: %s.\n", args[i]);
        }
    }

    if(i == 1) {
        printf("No argument was given.\n");
    }
}

int shell_lp(char **args) {

    struct dirent *direc_comp;
    DIR *proc_dir = opendir(PROC_DIR);

    if(!proc_dir) {
        printf("Couldn't open the /proc file.");
        exit(EXIT_FAILURE);
    }

    printf("%-32s %-5s %-10s %-10s %s\n", "USER", "PID", "MEM (kB)", "START TIME", "COMMAND");

    while((direc_comp = readdir(proc_dir)) != NULL) {
        if(!isxdigit(direc_comp -> d_name[0])) {
            continue;
        }

        char stat_path[256], cmdline_path[256], statm_path[256];

        // Getting the path ready.
        snprintf(stat_path, sizeof(stat_path), "%s/%s/%s", PROC_DIR, direc_comp->d_name, STAT_FILE);
        snprintf(cmdline_path, sizeof(cmdline_path), "%s/%s/%s", PROC_DIR, direc_comp->d_name, CMDLINE_FILE);
        snprintf(statm_path, sizeof(statm_path), "%s/%s/%s", PROC_DIR, direc_comp->d_name, STATM_FILE);

        // Openning the files and verifying that they're open.
        FILE *stat_file = fopen(stat_path, "r");
        FILE *cmdline_file = fopen(cmdline_path, "r");
        FILE *statm_file = fopen(statm_path, "r");

        if (!stat_file || !cmdline_file) {
            if (stat_file) fclose(stat_file);
            if (cmdline_file) fclose(cmdline_file);
            if (statm_file) fclose(statm_file);
            continue;
        }

        // Fetching pid, command, state and starting time.
        int pid;
        char comm[256];
        char state;
        unsigned long start_time;

        fscanf(stat_file, "%d %255s %c %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu", &pid, comm, &state, &start_time);
        fclose(stat_file);

        // Fetching username.
        uid_t uid = get_process_uid(direc_comp->d_name);
        char *user = get_name_from_uid(uid);

        // Fetching mem usage.
        long mem_usage;
        fscanf(statm_file, "%ld", &mem_usage);
        fclose(statm_file);

        mem_usage *= 4; // Converting to kB

        // Fetching command from cmdline file.
        char command[256];
        if(fgets(command, sizeof(command), cmdline_file) == NULL) {
            strncpy(command, comm, sizeof(command));
        }
        fclose(cmdline_file);

        // Get the process starting time.
        unsigned long boot_time = get_boot_time();

        time_t process_start_time = boot_time + (start_time / sysconf(_SC_CLK_TCK));
        char start_time_str[32];
        strftime(start_time_str, sizeof(start_time_str), "%H:%M:%S", localtime(&process_start_time));

        // Printing the line
        printf("%-32s %-5d %-10ld %-10s %s\n", user, pid, mem_usage, start_time_str,command);
    }

    closedir(proc_dir);
    return 1;
}

int shell_sc(char **args) {
    
    int i;
    for(i = 1; args[i] != NULL; i++) {
        FILE *f = fopen(args[i], "r");
        if(!f) {
            perror("Couldn't open specified file.\n");
            continue;
        }

        printf("\nShowing file: %s\n", args[i]);
        
        char c;
        while((c = fgetc(f)) != EOF) {
            fputc(c, stdout);
        }
    }

    if(i == 1) {
        printf("No argument was given.\n");
    }

    return 1;
}

int shell_schead(char **args) {

    char *params[] = {"-n", NULL};

    int lines_to_read = 10;
    int j, n;
    int i;

    for(j = 1; args[j] != NULL; j++) {
        if(strcmp(args[j], "-n") == 0) {
            n = 1;
            break;
        }
    }

    if(n == 1) {
        if((args[j + 1] != NULL && atoi(args[j + 1]) == 0) || args[j + 1] == NULL) {
            printf("Argument for option -n is required.\n");
            return 1;
        }
        else if((args[j + 1] != NULL && atoi(args[j + 1]) != 0)) {
            lines_to_read = atoi(args[j + 1]);
        }
    }

    for(i = 1; args[i] != NULL; i++) {

        // Skip the -n and it's argument.
        if(is_param(params, args[i]) || i == j + 1) {
            continue;
        }
        
        FILE *f = fopen(args[i], "r");

        if(!f) {
            perror("Couldn't open file to read.\n");
            continue;
        }

        char buffer[1024];
        int line_count = 0;

        while(line_count < lines_to_read) {
            char *line = NULL;
            size_t line_length = 0;
            size_t buffer_length = 0;

            do {
                if(fgets(buffer, sizeof(buffer), f) == NULL) {
                    break;
                }

                buffer_length = strlen(buffer);
                
                char *new_line = realloc(line, line_length + buffer_length + 1);
                if(new_line == NULL) {
                    fprintf(stderr, "Couldn't reallocate memory.\n");
                    free(line);
                    fclose(f);
                    return 1;
                }
                line = new_line;

                memcpy(line + line_length, buffer, buffer_length);
                line_length += buffer_length;
                line[line_length] = '\0';

            } while(buffer[buffer_length - 1] != '\n' && !feof(f));

            if(line != NULL) {
                fputs(line, stdout);
                free(line);
                line_count++;
            }

            if(feof(f)) {
                break;
            }
        }
        fclose(f);
    }

    if((i == 1) || (n == 1 && i == 2)) {
        printf("No argument was given.\n");
    }

    return 1;
}