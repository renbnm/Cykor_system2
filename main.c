#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_ARGS 128
#define PATH_MAX 4096

void print_prompt() {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("[main:%s]$ ", cwd);
    fflush(stdout);
}

void parse_args(char *cmd, char **args) {
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int run(char *cmd, int bg) {
    char *args[MAX_ARGS];
    parse_args(cmd, args);
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    }

    if (strcmp(args[0], "pwd") == 0) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
            return 0;
        } else {
            perror("pwd");
            return 1;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(127);
    } else if (pid > 0) {
        if (!bg) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
                return WEXITSTATUS(status);
            else
                return 1;
        }
        return 0;
    } else {
        perror("fork");
        return 1;
    }
}

void 

int main() {
    char buffer[MAX_INPUT];

    while (1) {
        print_prompt();
        if (!fgets(buffer, MAX_INPUT, stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "") == 0) continue;
        if (strcmp(buffer, "exit") == 0) break;

        char *cmds[MAX_ARGS];
        char *ops[MAX_ARGS - 1];
        int count = 0;
        char *tmp = buffer;

        while (*tmp) {
            while (*tmp == ' ') tmp++;

            char *start = tmp;
            while (*tmp && strncmp(tmp, "&&", 2) != 0 && strncmp(tmp, "||", 2) != 0 && *tmp != ';')
                tmp++;

            int len = tmp - start;
            while (len > 0 && start[len - 1] == ' ') len--;

            cmds[count] = strndup(start, len);

            if (strncmp(tmp, "&&", 2) == 0) {
                ops[count] = "&&";
                tmp += 2;
            } else if (strncmp(tmp, "||", 2) == 0) {
                ops[count] = "||";
                tmp += 2;
            } else if (*tmp == ';') {
                ops[count] = ";";
                tmp++;
            } else {
                ops[count] = NULL;
            }

            count++;
        }

        int last_status = 0;

        for (int i = 0; i < count; i++) {
            char *command = cmds[i];
            while (*command == ' ') command++;

            int background_flag = 0;
            int len = strlen(command);
            if (len > 0 && command[len - 1] == '&') {
                background_flag = 1;
                command[--len] = '\0';
                while (len > 0 && command[len - 1] == ' ') command[--len] = '\0';
            }

            if (i == 0 ||
                strcmp(ops[i - 1], ";") == 0 ||
                (strcmp(ops[i - 1], "&&") == 0 && last_status == 0) ||
                (strcmp(ops[i - 1], "||") == 0 && last_status != 0)) {
                last_status = run(command, background_flag);
            }

            free(cmds[i]);
        }
    }

    return 0;
}