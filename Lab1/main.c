#include <stdio.h>
#include <unistd.h>
#include "manager.h"
#include "fgfunctions.h"

int main() {
    //id of process, if id == 0 -> it`s child process
    int processId[2];
    int identificatorID;
    for (int i = 1; i < 3; i++) {
        //creat child process
        processId[i - 1] = fork();
        //check error
        if (processId[i - 1] == -1) {
            printf("Can`t make child process %d", i);
            return 1;
        }
        //set ids for childes
        if (processId[i - 1] == 0) {
            identificatorID = i;
            break;

        }
    }
    //set id for manager
    if (identificatorID != 1 && identificatorID != 2) {
        identificatorID = 0;
    }
    //ids: manager = 0, child1 = 1, child2 = 2
    switch (identificatorID) {
        //manager
        case 0: {
            return manager("/tmp/f_fifo", "/tmp/g_fifo", processId);
        }
            //f-function
        case 1: {
            return (int) fgfunction("/tmp/f_fifo", 'f');
        }
            //g-function
        case 2: {
            return (int) fgfunction("/tmp/g_fifo", 'g');
        }
    }
}
