/**
 * Program 1 - Part 2 : Basic Shell Implementation
 * @author Brandon Manke
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#include <sys/wait.h> 
#include <sys/types.h> 

#include <signal.h>

#define MAX_LINE 80

#define READ_END 0
#define WRITE_END 1

typedef struct cmd_t {
    char*  cmd;
    char** params;
    int    params_size;
    int    amp_index;
    int    pipe_index;
} cmd_t;

char** fmt_params(cmd_t* c) {
    char** new_params = NULL;
    int s;

    if (c->amp_index != -1 || c->pipe_index != -1) {
        new_params = malloc(c->params_size);
        s = c->params_size;
    } else {
        new_params = malloc(c->params_size + 1);
        s = c->params_size + 1;
    }

    for (int i = 0; i < s; i++) {
        //printf("params %d - %s\n", i, params[i]);
        if (i != c->amp_index && i != c->pipe_index) {
            new_params[i] = c->params[i];
        }
    }
    new_params[s - 1] = NULL;

    return new_params;
}

cmd_t** fmt_pipe_cmd(cmd_t* c) {
    printf(
        "c cmd: %s\n", c->cmd
    );
    if (c->pipe_index == -1) {
        fprintf(stderr, "error: pipe index is -1\n");
        return NULL; 
    }
    cmd_t** arr = malloc(2 * sizeof(cmd_t));
    cmd_t* c1 = malloc(sizeof(cmd_t));
    cmd_t* c2 = malloc(sizeof(cmd_t));

    c1->params = malloc(c->params_size);
    c2->params = malloc(c->params_size);

    c1->cmd = malloc(strlen(c->cmd) + 1);

    c1->cmd = c->cmd;
    int p2count = 0;
    int p1count = 0;
    for (int i = 0; i < c->params_size; i++) {
        if (i > c->pipe_index) {
            if (i == c->pipe_index + 1) {
                c2->cmd = c->params[i];
            } else {
                c2->params[i] = c->params[i];
                p2count++;
            }
        } else if (i < c->pipe_index) {
            c1->params[i] = c->params[i];
            p1count++;
        }
    }
    c1->params_size = p1count;
    c1->amp_index = -1;
    c1->pipe_index = -1;

    printf("c1 cmd in fmt: %s\n", c1->cmd);

    c2->params_size = p2count;
    c2->amp_index = -1;
    c2->pipe_index = -1;
    arr[0] = c1;
    arr[1] = c2;
    return arr;
}

void exec_pipe(cmd_t* c) {
    if (c == NULL) {
        fprintf(stderr, "error: c is null\n");
        return;
    }
    cmd_t** arr = fmt_pipe_cmd(c);
    if (arr == NULL) {
        fprintf(stderr, "error: arr is null\n");
        return;
    }
    cmd_t* c1 = arr[0];
    cmd_t* c2 = arr[1];
    
    printf("c1 cmd: %s\n c2 cmd: %s\n", c1->cmd, c2->cmd);

    int fd[2];
    if (pipe(fd) == -1) {
        fprintf(stderr, "error: pipe failed\n");
        fflush(stderr);
        fflush(stdout);
        return;
    }

    int status;
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "error: fork() -> -1\n");
        exit(1);
    }
    int max_len = MAX_LINE / 2 + 1;

    if (pid > 0) {
        close(fd[READ_END]);
        dup2(fd[WRITE_END], STDOUT_FILENO);
        
        char* wr_params[max_len];
        for (int i = 1; i < c1->params_size; i++) {
            wr_params[i] = c1->params[i];
        }
        wr_params[0] = c1->cmd;
        wr_params[c1->pipe_index] = NULL;
        int err = execvp(c1->cmd, wr_params);
        if (err == -1) {
            fprintf(stderr, "error: unrecognized command - %s\n", c1->cmd);
            fprintf(stderr, "with parameters: ");
            for (int i = 1; i < max_len && wr_params[i] != NULL; i++) {
                fprintf(stderr, "%s ", wr_params[i]);
                wr_params[i] = NULL;
            }
            fprintf(stderr, "\n");
            exit(0);
        }
    } else {
        waitpid(pid, &status, 0);
        close(fd[WRITE_END]);
        //dup2(fd[READ_END], stdin);
        char* buf;
        read(fd[READ_END], buf, max_len);
        printf("buf: %s\n", buf);
        char* ch = strtok(buf, " ");
        char* rd_params[MAX_LINE];
        int i = 0;
        for (i = 1; i < c2->params_size && c2->params[i - 1] != NULL; i++) {
            rd_params[i] = c2->params[i - 1];
        }
        i = rd_params[i] != NULL ? ++i : i;
        while (ch != NULL) {
            strcpy(rd_params[i], ch);
            ch = strtok(NULL, " ");
        }
        rd_params[0] = c2->cmd;
        rd_params[i >= MAX_LINE ? MAX_LINE - 1 : i] = NULL;
        int err = execvp(c2->cmd, rd_params);
        if (err == -1) {
            fprintf(stderr, "error: unrecognized command - %s\n", c2->cmd);
            fprintf(stderr, "with parameters: ");
            for (int i = 1; i < max_len && rd_params[i] != NULL; i++) {
                fprintf(stderr, "%s ", rd_params[i]);
                rd_params[i] = NULL;
            }
            fprintf(stderr, "\n");
            exit(0);
        }

    }
}

void exec_cmd(cmd_t* c, char* args[]) {
    if (c == NULL || c->cmd == NULL) {
        return;
    }

    int status;
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "error: fork() -> -1\n");
        // check errno?
        exit(1);
    } else if (pid == 0) { // exec cmd
        char* bin_path = "/bin/";
        char* ch = malloc((sizeof(bin_path) + sizeof(c->cmd)) / sizeof(char));
        strcat(ch, bin_path);
        strcat(ch, c->cmd);

        if (c->params == NULL || c->params_size == 0) {
            int err = execlp(ch, c->cmd, NULL);
            if (err == -1) {
                fprintf(stderr, "error: unrecognized command - %s\n", c->cmd);
                free(ch);
                fflush(stdout);
                exit(0);
            }
        } else if (c->pipe_index != -1) {
            printf("c %s\n", c->cmd);
            exec_pipe(c);
        } else {
            char** new_params = fmt_params(c); // rework?
            int i = 1;
            while (i < (MAX_LINE / 2) + 1 && new_params[i - 1] != NULL) {
                args[i] = new_params[i - 1];
                i++;
            }
            args[0] = c->cmd;
            args[i] = NULL;

            int err = execvp(c->cmd, args);
            if (err == -1) {
                fprintf(stderr, "error: unrecognized command - %s\n", c->cmd);
                fprintf(stderr, "with parameters: ");
                for (int j = 0; j < sizeof(new_params) && new_params[j] != NULL; j++) {
                    fprintf(stderr, "%s ", new_params[j]);
                    new_params[j] = NULL;
                }
                fprintf(stderr, "\n");
                free(ch);
                fflush(stdout);
                exit(0);
            }
        }
    } else { // TODO check for '&'
        if (c->amp_index == -1) {
            wait(&status);
        } else {
            printf("[1] Running: pid %d - %s ", pid, c->cmd);
            for (int i = 0; i < c->params_size && c->params[i] != NULL; i++) {
                printf("%s ", c->params[i]);
            }
            printf("\n");
        }
        //printf("status: %d\n", status);
        //waitpid(pid, &status, WNOHANG);
    }
    fflush(stdout);
}

cmd_t* tokenize_str(char* fstr) {
    cmd_t* command = malloc(sizeof(cmd_t));
    char* c = strtok(fstr, " ");
    int count = 0;
    int pcount = 0;
    const int MAX_PSIZE = 16;
    char** p = malloc(MAX_PSIZE);

    command->amp_index = -1;
    command->pipe_index = -1;

    while(c != NULL) {
        if (*c != '\n') {
            if (count == 0) {
                command->cmd = malloc(strlen(c) + 1);
                strcpy(command->cmd, c);
                count++;
            } else if (count != 0 && pcount < MAX_PSIZE) {
                if (strcmp(c, "&") == 0) {
                    command->amp_index = pcount;
                } else if (strcmp(c, "|") == 0) {
                    command->pipe_index = pcount;
                }
                p[pcount] = malloc(strlen(c) + 1);
                p[pcount] = c;
                pcount++;
            }
        }
        c = strtok(NULL, " ");

        if (pcount > MAX_PSIZE) { // impossible to reach?
            fprintf(stderr, "error: max param size reached (%d)\n", MAX_PSIZE);
            exit(1);
        }
    }
    command->params = p;
    command->params_size = pcount;
    return command;
}

char* fmt_str(char buf[512]) {
    char* nb = malloc(sizeof(char) * 512);
    char prev;
    int count = 0;
    for (int i = 0; i < 512; i++) {
        if (buf[i] == ' ' && prev == ' ') {
            continue;
        }
        nb[count] = buf[i];
        count++;
        prev = buf[i];
    }

    if (nb[0] == ' ') {
        for (int i = 0; i < (sizeof(nb) / sizeof(char)) - 1; i++) {
            if (nb[i] != 0) {
                nb[i] = nb[i+1];
            }
        }
    }

    if (nb[strlen(nb) - 2] == ' ') {
        //printf("0: %d, -1: %d, -2: %d\n", nb[strlen(nb)], nb[strlen(nb) - 1], nb[strlen(nb) - 2]);
        nb[strlen(nb) - 1] = 0; // this decreases len by one so we need to account for offset
        nb[strlen(nb)] = '\0';
        nb[strlen(nb) - 1] = '\n';
    }
    nb[strlen(nb)] = 0;
    nb[strlen(nb) - 1] = '\0';
    return nb;
}

// 1 - parse/read input
// 2 - fork()
// 3 - child process invokes execlp()
// 4 - check command for &, to determine if parent should wait()
//      (& -> no wait() , no & -> wait())
int main(void) {
    char* args[(MAX_LINE / 2) + 1];
    int run_flag = 1;

    while (run_flag) {
        printf("manke_ish> ");
        fflush(stdout); // flush

        char buf[512];
        // scanf("%s", buf); // blocking
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            fprintf(stderr, "%s\n", "error: fgets() -> NULL");
            return 1;
        }
        if (strlen(buf) > MAX_LINE) {
            fprintf(stderr, "error: max line length reached (%d)\n", MAX_LINE);
            continue;
        }
        if (buf[0] == '\n') {
            continue;
        }

        char* fstr = fmt_str(buf); // check NULL?
        //sscanf(fstr, "%s", fstr); ?
        if (fstr == NULL) {
            continue;
        }
        if (strcmp(fstr, "exit") == 0) {
            run_flag = 0;
            break;
        }

        cmd_t* command = tokenize_str(fstr);
        if (command == NULL) {
            fprintf(stderr, "error: command is null\n");
            return 1;
        }
        exec_cmd(command, args);
        printf("");

        free(command->cmd);
        free(command->params);
        free(command);
        command = NULL;
        if (fstr != NULL) {
            free(fstr);
        }
    }
    return 0;
}
