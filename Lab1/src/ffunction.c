#include <unistd.h>
#include "ffunction.h"
#include "stdlib.h"

compfunc_status_t fFunction(int x, int *result) {
    if(x < 0)
        while (true);
    if(x == 0) {
        return COMPFUNC_HARD_FAIL;
    }
    if(x == 1) {
        *result = 1;
        return COMPFUNC_SUCCESS;
    }
    if(x > 1) {
        if(x > 100) {
            usleep(5000);
            return COMPFUNC_SOFT_FAIL;
        }
        int r = rand()%2;
        if(r == 1) {
            *result = x*2;
            return COMPFUNC_SUCCESS;
        }
        return COMPFUNC_SOFT_FAIL;
    }
}