#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

#define MAX_LINE 80

typedef struct cmd_t {
    char* cmd;
    char** params;
    int params_size;
} cmd_t;

// TODO
void parse_token(cmd_t* c) {
    printf("cmd: %s\n", c->cmd);
    if (strcmp(c->cmd, "ls") == 0) {
        printf("ls command\n");
    } else {
        fprintf(stderr, "unrecognized command: %s", c->cmd);
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
        if (*c != '\n' && count == 0) {
            command->cmd = c;
            count++;
        }
        c = strtok(NULL, " ");
        if (count != 0 && pcount < MAX_PSIZE) {
            command->params[pcount] = c;
            pcount++;
        }

        if (pcount > MAX_PSIZE) {
            fprintf(stderr, "max param size reached (%d)\n", MAX_PSIZE);
            exit(1);
        }
    }
    command->params = p;
    command->params_size = pcount;
    return command;
    //parse_token(c);
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
    printf("nb:%s\n", nb);
    return nb;
}

int main(void) {
    char* args[(MAX_LINE / 2) + 1];
    int run_flag = 1;

    while (run_flag) {
        printf("manke_ish> ");
        fflush(stdout); // flush

        char buf[512];
        // scanf("%s", buf); // blocking
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            fprintf(stderr, "%s\n", "error in fgets");
            return 1;
        }

        char* fstr = fmt_str(buf); // check NULL?
        if (strcmp(fstr, "quit") == 0) {
            break;
        }

        /*cmd_t* command = tokenize_str(fstr);
        if (command == NULL) {
            fprintf(stderr, "command is null");
            return 1;
        }
        printf(
            "cmd size: %lu\n cmd: %s\n params size: %d\n", 
            sizeof(command), 
            command->cmd, 
            command->params_size
        );*/


        // 1 - parse/read input
        // 2 - fork()
        // 3 - child process invokes execlp()
        // 4 - check command for &, to determine if parent should wait()
        //      (& -> no wait() , no & -> wait())
        //break;
    }
    return 0;
}

