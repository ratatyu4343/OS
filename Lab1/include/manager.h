#ifndef LAB1_MANAGER_H
#define LAB1_MANAGER_H

#include <fcntl.h>

int manager(char *pathFfifo, char *pathGfifo, pid_t *childIds);
void stopManagerSignal(int sig);

#endif //LAB1_MANAGER_H
