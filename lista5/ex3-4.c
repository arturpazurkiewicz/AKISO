
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <wait.h>
#include <zconf.h>
#include <stdbool.h>

struct job {
    char *command;
    int *pids;
    int mode;
};

int *jobTableSize = 0;

struct job jobTable[100];

void sighandler(int signum) {
    wait(NULL);
    //printf("Child died: %d\n",signum);
    if (signum == SIGINT) {

    }
}

//void printJobs() {
//    int i;
//    printf("start job\n");
//    printf("%d   %s\n", jobTable[0].pids[0], jobTable[0].command[0]);
//}

int getLine(char *words[]) {
    char input[PATH_MAX];
    char *word;
    char path[PATH_MAX];
    getcwd(path, sizeof(path));
    printf("%s$ ", path);
    if (feof(stdin)) {
        printf("exit\n");
        _exit(0);
    }
    fgets(input, PATH_MAX, stdin);
    if (strlen(input) > 0 && input[strlen(input) - 1] == '\n') {
        input[strlen(input) - 1] = '\0';
    }
    int i = 0;
    word = strtok(input, " ");
    while (word != NULL) {
        words[i] = calloc(strlen(word) + 1, sizeof(char));
        strcpy(words[i], word);
        i++;
        word = strtok(NULL, " ");
    }
    return i;
}

// sets fds to files we specify
// modes:
// 0 - read
// 1 - write
// -1 - none
// 2 - all
void addRedirects(int size, char *commands[][size], int index, int mode) {
    int commandLength = 0;
    while (commands[index][commandLength] != NULL) commandLength++;

    for (int j = 0; j < commandLength; j++) {
        if (commands[index][j][0] == '>' && (mode == 1 || mode == 2)) {
            char path[PATH_MAX];
            for (int k = 1; k < strlen(commands[index][j]); k++) {
                path[k - 1] = commands[index][j][k];
            }
            int file;
            if ((file = open(path, O_RDWR | O_CREAT | O_APPEND, 0777)) == -1) {
                perror("> error");
                _exit(1);
            }
            if (dup2(file, 1) == -1) {
                perror("dup2 error when tryging to redirect to file");
            }
            commands[index][j] = NULL;
        } else if (commands[index][j][0] == '2' && commands[index][j][1] == '>') {
            char path[PATH_MAX];
            for (int k = 2; k < strlen(commands[index][j]); k++) {
                path[k - 2] = commands[index][j][k];
            }
            int file;
            if ((file = open(path, O_RDWR | O_CREAT | O_APPEND, 0777)) == -1) {
                perror("2> error");
                _exit(1);
            }
            if (dup2(file, 2) == -1) {
                perror("dup2 error when tryging to redirect to file");
            }
            commands[index][j] = NULL;
        } else if (commands[index][j][0] == '<' && (mode == 0 || mode == 2)) {
            char path[PATH_MAX];
            for (int k = 1; k < strlen(commands[index][j]); k++) {
                path[k - 1] = commands[index][j][k];
            }
            int file;
            if ((file = open(path, O_RDWR | O_CREAT | O_APPEND, 0777)) == -1) {
                perror("> error");
                _exit(1);
            }
            if (dup2(file, 0) == -1) {
                perror("dup2 error when tryging to redirect to file");
            }
            commands[index][j] = NULL;
        }
    }
}

void pipeEngine(char **pipedCommands, int size, bool processWait) {
    int foundIndex = size;
    int commandsCount = 0;
    char* commands[size][size];
    for (int i = size - 1; i > -1; i--) {
        if (pipedCommands[i][0] == '|' || i == 0) {
            int k = 0;
            int j = i + 1;
            if (i == 0) {
                j = 0;
            }
            for (; j < foundIndex; j++) {
                // commands is allocated and written to and yet we don't ever free it, because the process dies eventually, so, whatever
                commands[commandsCount][k] = calloc(strlen(pipedCommands[j]) + 1, sizeof(char));
                strcpy(commands[commandsCount][k], pipedCommands[j]);
                k++;
            }
            commands[commandsCount][k] = NULL;
            commandsCount++;
            foundIndex = i;
        }
    }
    int fd[commandsCount - 1][2];
    for (int i = 0; i < commandsCount - 1; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe error");
        }
    }
    int pids[commandsCount];
    int status, wpid;
    for (int i = 0; i < commandsCount; i++) {
        if ((pids[i] = fork()) == 0) {
            if (commandsCount > 1) {
                if (i == 0) {
                    // case for the LAST command in pipes
                    if (dup2(fd[i][0], 0) == -1) {
                        perror("dup2 error on i=0");
                    }
                    addRedirects(size, commands, i, 1);
                } else if (i != commandsCount - 1) {
                    // case for all the MIDDLE commands in pipes
                    if (dup2(fd[i-1][1], 1) == -1) {
                        perror("dup2 error");
                    }
                    if (dup2(fd[i][0], 0) == -1) {
                        perror("dup2 error");
                    }
                    addRedirects(size, commands, i, -1);
                } else {
                    if (dup2(fd[i-1][1], 1) == -1) {
                        perror("dup2 error on i=commands-1");
                    }
                    addRedirects(size, commands, i, 0);
                }
            } else {
                addRedirects(size, commands, 0, 2);
            }
            for (int j = 0; j < commandsCount - 1; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
            }
            execvp(commands[i][0], commands[i]);
            perror("execvp error");
            _exit(1);
        }
    }
    for (int j = 0; j < commandsCount - 1; j++) {
        close(fd[j][0]);
        close(fd[j][1]);
    }
    if(processWait) {
        for(int i = commandsCount - 1; i > -1; i--) {
            if(pids[i] > 0) {
                int status;
                waitpid(pids[i], &status, 0);
            } else {
            }
        }
    }
    exit(0);
}

int main() {
    printf("lsh Artur Pazurkiewicz\n");
    char *words[PATH_MAX];
    signal(SIGINT, sighandler);
    bool processWait;
    while (1) {
        int i = getLine(words);
        if (i == 0) {
            continue;
        }
        int status, wpid;
        if (strcmp(words[0], "exit") == 0) {
            _exit(0);
        } else if (strcmp(words[0], "cd") == 0) {
            chdir(words[1]);
            continue;
        } else {
            if (strcmp(words[i - 1], "&") == 0) {
                processWait = false;
                i--;
            } else {
                processWait = true;
            }

            int child = fork();
            signal(SIGINT, sighandler);
            if (child == 0) {
                pipeEngine(words, i, processWait);
            } else {
                signal(SIGCHLD, sighandler);
                signal(SIGINT, sighandler);
                if (processWait) {
                    wpid = waitpid(child, &status, 0);
                }
            }
            while (0 < --i) free(words[i]);
        }
    }
}