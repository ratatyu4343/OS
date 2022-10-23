#include "manager.h"
#include "compfuncs.h"
#include "trialfuncs.h"
#include "unistd.h"
#include "errno.h"
#include "sys/wait.h"
#include "sys/stat.h"
#include "string.h"
#include "fcntl.h"
#include "signal.h"
#include "bfunction.h"
#include "stdlib.h"

//statuses and results of functions, need to be global for signal handler
static char *pathFfifoGlobal;
static char *pathGfifoGlobal;
static int  fStatus = (int)COMPFUNC_STATUS_MAX;
static int  gStatus = (int)COMPFUNC_STATUS_MAX;
static int  bStatus = (int)COMPFUNC_STATUS_MAX;
static int  bCount = 0, fCount = 0, gCount = 0;
static int  fResult;
static bool gResult;
static bool bResult;

//function for printing results
void printResults();

int manager(char *pathFfifo, char *pathGfifo, pid_t *childIds) {
    pathFfifoGlobal = pathFfifo;
    pathGfifoGlobal = pathGfifo;
    //Ctrl+C caching
    struct sigaction sa1;
    sa1.sa_handler = stopManagerSignal;
    sa1.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa1, NULL);
    //input x
    int x = 0;
    printf("Input x: ");
    scanf( "%d", &x);
    //create named pipes files f_fifo and g_fifo
    if(mkfifo(pathFfifo, 0777) == -1 && errno != EEXIST) {
        printf("Can`t make f-function FIFO file: %s", strerror(errno));
        return 2;
    }
    if(mkfifo(pathGfifo, 0777) == -1 && errno != EEXIST) {
        printf("Can`t make g-function FIFO file: %s", strerror(errno));
        return 2;
    }
    //creat files descriptors
    int fWriteDescriptor = open(pathFfifo, O_WRONLY);
    if(fWriteDescriptor == -1) {
        printf("Can`t open f_fifo for writing...");
        return 3;
    }
    int gWriteDescriptor = open(pathGfifo, O_WRONLY);
    if(gWriteDescriptor == -1) {
        printf("Can`t open g_fifo for writing...");
        return 3;
    }
    //puts x to functions
    if(write(fWriteDescriptor, &x, sizeof(x)) == -1) {
        printf("Can not write to function F...");
        return 4;
    }
    if(write(gWriteDescriptor, &x, sizeof(x)) == -1) {
        printf("Can not write to function G...");
        return 4;
    }
    close(fWriteDescriptor);
    close(gWriteDescriptor);
    int fReadDescriptor = open(pathFfifo, O_RDONLY | O_NONBLOCK);
    int gReadDescriptor = open(pathGfifo, O_RDONLY | O_NONBLOCK);
    //waiting for functions results
    int status;
    pid_t cId;
    while ((cId = wait(&status)) != -1) {
        if (cId == childIds[0]) {
            fStatus = status;
            fStatus = WEXITSTATUS(fStatus);
            //read results of functions
            if(fStatus == COMPFUNC_SUCCESS || fStatus == COMPFUNC_SOFT_FAIL) {
                if (fReadDescriptor == -1) {
                    printf("Can`t open f_fifo for reading...");
                    return 5;
                }
                if(fStatus == COMPFUNC_SOFT_FAIL) {
                    if(read(fReadDescriptor, &fCount, sizeof(int)) == -1) {
                        printf("Can not read count of iterations from f-function");
                        return 6;
                    }
                }
                if(read(fReadDescriptor, &fResult, sizeof(int)) == -1) {
                    printf("Can not read result from f-function");
                    return 7;
                }
                close(fReadDescriptor);
            }
        }
        else {
            gStatus = status;
            gStatus = WEXITSTATUS(gStatus);
            //read results of functions
            if(gStatus == COMPFUNC_SUCCESS || gStatus == COMPFUNC_SOFT_FAIL) {
                if(gReadDescriptor == -1) {
                    printf("Can`t open g_fifo for reading...");
                    return 5;
                }
                if(gStatus == COMPFUNC_SOFT_FAIL) {
                    if(read(gReadDescriptor, &gCount, sizeof(int)) == -1) {
                        printf("Can not read count of iterations from g-function");
                        return 6;
                    }
                }
                if(read(gReadDescriptor, &gResult, sizeof(int)) == -1) {
                    printf("Can not read result from g-function");
                    return 7;
                }
                close(gReadDescriptor);
            }
        }
    }
    //Binary operation calculating
    if(fStatus != COMPFUNC_HARD_FAIL && gStatus != COMPFUNC_HARD_FAIL) {
        do {
            bCount++;
            bStatus = (int) bFunction(fResult, gResult, &bResult);
        } while (bStatus == COMPFUNC_SOFT_FAIL);
        bStatus = (bCount == 1) ? bStatus : COMPFUNC_SOFT_FAIL;
    } else {
        bStatus = COMPFUNC_HARD_FAIL;
    }
    //Print results
    printResults();
    return 0;
}

//Ctrl+C caching
void stopManagerSignal(int sig) {
    signal(sig, SIG_IGN);
    if(fStatus == COMPFUNC_STATUS_MAX) {
        size_t len = strlen(pathFfifoGlobal);
        char *pathFlobalNew = malloc(len + 2);
        strcpy(pathFlobalNew, pathFfifoGlobal);
        pathFlobalNew[len] = '2';
        pathFlobalNew[len + 1] = '\0';
        mkfifo(pathFlobalNew, 0777);
        int rd = open(pathFlobalNew, O_RDONLY);
        read(rd, &fStatus, sizeof(compfunc_status_t));
        if(fStatus == COMPFUNC_SOFT_FAIL) {
            read(rd, &fCount, sizeof(int));
            fStatus = COMPFUNC_STATUS_MAX;
        }
        close(rd);
    }
    if(gStatus == COMPFUNC_STATUS_MAX) {
        size_t len = strlen(pathFfifoGlobal);
        char *pathGlobalNew = malloc(len + 2);
        strcpy(pathGlobalNew, pathGfifoGlobal);
        pathGlobalNew[len] = '2';
        pathGlobalNew[len + 1] = '\0';
        mkfifo(pathGlobalNew, 0777);
        int rd = open(pathGlobalNew, O_RDONLY);
        read(rd, &gStatus, sizeof(compfunc_status_t));
        if(gStatus == COMPFUNC_SOFT_FAIL) {
            read(rd, &gCount, sizeof(int));
            gStatus = COMPFUNC_STATUS_MAX;
        }
        close(rd);
    }
    printResults();
    _exit(0);
}

//print results
void printResults() {
    char fResultStr[50], gResultStr[50];
    sprintf(fResultStr, "%d", fResult);
    sprintf(gResultStr, "%s", gResult ? "true" : "false");
    printf("\nResults:\nf:\t\t");
    switch (fStatus) {
        case COMPFUNC_SUCCESS:
            printf("%s", fResultStr);
            break;
        case COMPFUNC_SOFT_FAIL:
            printf("%s, SOFT_FAIL, count of iterations: %d", fResultStr, fCount);
            break;
        case COMPFUNC_STATUS_MAX:
            if(fCount > 1) {
                printf("%s, SOFT_FAIL, count of iterations: %d...", symbolic_status(fStatus), fCount);
            } else {
                printf("%s", symbolic_status(fStatus));
            }
            break;
        default:
            printf("%s", symbolic_status(fStatus));
            break;
    }
    printf("\ng:\t\t");
    switch (gStatus) {
        case COMPFUNC_SUCCESS:
            printf("%s\n", gResultStr);
            break;
        case COMPFUNC_SOFT_FAIL:
            printf("%s, SOFT_FAIL, count of iterations: %d\n", gResultStr, gCount);
            break;
        case COMPFUNC_STATUS_MAX:
            if(gCount > 1) {
                printf("%s, SOFT_FAIL, count of iterations: %d...\n", symbolic_status(gStatus), gCount);
            } else {
                printf("%s\n", symbolic_status(gStatus));
            }
            break;
        default:
            printf("%s\n", symbolic_status(gStatus));
            break;
    }
    printf("f x g:\t\t");
    switch (bStatus) {
        case COMPFUNC_SUCCESS:
            printf("%s\n", bResult ? "true" : "false");
            break;
        case COMPFUNC_SOFT_FAIL:
            printf("%s, SOFT_FAIL, count of iterations: %d\n", bResult ? "true" : "false", bCount);
            break;
        case COMPFUNC_STATUS_MAX:
            if(bCount > 1) {
                printf("%s, SOFT_FAIL, count of iterations: %d...\n", symbolic_status(bStatus), bCount);
            } else {
                printf("%s\n", symbolic_status(bStatus));
            }
            break;
        default:
            printf("%s\n", symbolic_status(bStatus));
            break;
    }
    fflush(stdout);
}