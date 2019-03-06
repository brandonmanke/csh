#include "stdio.h"
#include "unistd.h"

#define MAX_LINE 80

int main(int argc, char* argv[]) {
    char* args[(MAX_LINE / 2) + 1];
    int run_flag = 1;
    
    while (run_flag) {
        printf("manke_ish> ");
        fflush(stdout); // flush

        // 1 - parse/read input
        // 2 - fork()
        // 3 - child process invokes execlp()
        // 4 - check command for &, to determine if parent should wait()
        //      (& -> no wait() , no & -> wait())
        break;
    }
    return 0;
}

