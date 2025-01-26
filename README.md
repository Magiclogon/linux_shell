
# Linux Shell

This **Linux Shell** is a custom implementation of a command-line interface shell built for Unix-like operating systems. It replicates the functionality of a standard Linux shell, allowing users to execute system commands, manage processes, and explore basic scripting capabilities.

This project is done as a personnal project in my free time.





## Features

Executing multiple commands listed in the documentation bellow.


## Documentation

List of commands available currently in this **Linux Shell**:

| Command | Options | Parameters | Description | Syntax |
|---------|-------|-----|-------------|--------|
|`exit`|`None` | `None` | Exits the shell. | `exit` |
|`help` | `None` | `None` | Lists the builtin commands | `help` |
|`cd`| `None` | `<path>` |Changes the current directory to the specified path. | `cd <path>` |
|`pwd`| `None` | `None` | Prints working directory. | `pwd`|
| `ld` | `-r ` : Recursively apply `ld` on subdirectories | `[directory1] ... [directoryN]` | Lists files and directories within a specified directory. | `ld [directory1] ... [directoryN]` |
| `cf` | `None` | `<path_to_file>` | Creates a file at the `<path_to_file>` path. | `cf <path_to_file>` |
| `rmvf` | `None` | `<file1> [file2] ... [fileN]` | Deletes all files listed as arguments. | `rmvf file1 [file2] ... [fileN]` |
| `rmvd` | `-r` : If the directory isn't empty, deletes all content within the directory then deletes it. | `<directory1> [directory2] ... [directoryN]` | Deletes directories. | `rmvd <directory1> [directory2] ... [directoryN]` |
| `mkd` | `-par` : Creates all parent directories if they don't exist. | `<directory1> [directory2] ... [directoryN]` | Creates directories. | `mkd <directory1> [directory2] ... [directoryN]` |
| `cpy` | `None` | `<source> <destination>` | Copies a file or a directory to a destination. | `cpy <source> <destination>` |
| `kill` | `None` | `<pid1> [pid2] ... [pidN]` | Kills processes based on their **pid**. | `kill <pid1> [pid2] ... [pidN]` |
| `lp` | `None` | `None` | List all running processes with some informations about them. | `lp` |

## Installation

To run this **Linux Shell**, follow the steps provided bellow : 

1. Clone Repository :
    ```bash
   git clone https://github.com/Magiclogon/linux_shell.git
    ```

2. If you want to compile it yourself (Optional) :
    ```bash
   gcc -c main.c utils.c commands.c
   gcc -o shell main.o utils.o commands.o
    ```

3. Run the shell :
    ```bash
   ./shell
   ```
And voilà you're ready to go, hope you enjoy it.

