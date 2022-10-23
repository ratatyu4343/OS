#include <string.h>
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>
#include "../include/fgfunctions.h"
#include "compfuncs.h"
#include "unistd.h"
#include "fcntl.h"
#include "ffunction.h"
#include "gfunction.h"
#include "signal.h"
#include "time.h"

//status of function
static int STATUS = COMPFUNC_STATUS_MAX;
//count of iterations for soft_fail
static int count = 0;
//path fifo
static char *pathGlobal;

compfunc_status_t fgfunction(char *path, char functionFlag) {
    pathGlobal = path;
    //Ctrl+C catching
    struct sigaction sa2;
    sa2.sa_handler = exitSignal;
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa2, NULL);
    //creat file descriptors
    int ReadDescriptor = open(path, O_RDONLY);
    if (ReadDescriptor == -1) {
        printf("Can not open %c-fifo for reading...", functionFlag);
        return 3;
    }
    //argument
    int x;
    //calculated result
    int result;
    //read x from manager
    if (read(ReadDescriptor, &x, sizeof(x)) == -1) {
        printf("Can not read x for %c-function...", functionFlag);
        return 4;
    }
    close(ReadDescriptor);
    //calculating function and returning status
    switch (functionFlag) {
        case 'f':
            srand(time(NULL));
            do{
                STATUS = fFunction(x, &result);
                count++;
            } while (STATUS == COMPFUNC_SOFT_FAIL);
            break;
        case 'g':
            srand(time(NULL)+1);
            bool result2;
            do {
                STATUS = gFunction(x, &result2);
                count++;
            } while (STATUS == COMPFUNC_SOFT_FAIL);
            result = (int)result2;
            break;
        default:
            return COMPFUNC_STATUS_MAX;
    }
    STATUS = (count == 1) ? STATUS : COMPFUNC_SOFT_FAIL;
    //writing result and returning status
    int WriteDescriptor = open(path, O_WRONLY);
    if(WriteDescriptor == -1) {
        printf("Can not open %c-fifo to write result...", functionFlag);
        return 5;
    }
    //if it's soft fail write count of iterations
    if(count > 1) {
        if(write(WriteDescriptor, &count, sizeof(int)) == -1) {
            printf("Can not write count of iterations of %c-function...", functionFlag);
            return 6;
        }
    }
    if(write(WriteDescriptor, &result, sizeof(int)) == -1) {
        printf("Can not write result of %c-function...", functionFlag);
        return 6;
    }
    close(WriteDescriptor);
    return STATUS;
}

void exitSignal(int sig) {
    signal(sig, SIG_IGN);
    size_t len = strlen(pathGlobal);
    char *pathGlobalNew = malloc(len + 2);
    strcpy(pathGlobalNew, pathGlobal);
    pathGlobalNew[len] = '2';
    pathGlobalNew[len + 1] = '\0';
    mkfifo(pathGlobalNew, 0777);
    int wd = open(pathGlobalNew, O_WRONLY);
    write(wd, &STATUS, sizeof(STATUS));
    if(STATUS == COMPFUNC_SOFT_FAIL)
        write(wd, &count, sizeof(count));
    close(wd);
    kill(getpid(), SIGKILL);
}