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

typedef struct cmd_t {
    char*  cmd;
    char** params;
    int    params_size;
} cmd_t;

// TODO
void exec_cmd(cmd_t* c) {
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
        // no params
        int err = execlp(ch, c->cmd, NULL);
        if (err == -1) {
            fprintf(stderr, "error: unrecognized command - %s\n", c->cmd);
            exit(0);
        }
    } else { // TODO check for '&'
        wait(&status);
        //printf("status: %d\n", status);
        //waitpid(pid, &status, WNOHANG);
    }
}

cmd_t* tokenize_str(char* fstr) {
    cmd_t* command = malloc(sizeof(cmd_t));
    char* c = strtok(fstr, " ");
    int count = 0;
    int pcount = 0;
    const int MAX_PSIZE = 16;
    char** p = malloc(MAX_PSIZE);
    while(c != NULL) {
        if (*c != '\n') {
            if (count == 0) {
                command->cmd = malloc(strlen(c) + 1);
                strcpy(command->cmd, c);
                count++;
            } else if (count != 0 && pcount < MAX_PSIZE) {
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
        if (strcmp(fstr, "quit") == 0) {
            run_flag = 0;
            break;
        }

        cmd_t* command = tokenize_str(fstr);
        if (command == NULL) {
            fprintf(stderr, "error: command is null\n");
            return 1;
        }
        /*printf(
            "cmd size: %lu\n cmd: %s\n params size: %d\n", 
            sizeof(command), 
            command->cmd, 
            command->params_size
        );*/
        exec_cmd(command);

        free(command->cmd);
        free(command->params);
        free(command);
        if (fstr != NULL) {
            free(fstr);
        }
    }
    return 0;
}
