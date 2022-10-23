#include "gfunction.h"
#include "stdlib.h"

compfunc_status_t gFunction(int x, bool *result) {
    if(x < 0){
        while(true);
    }
    if(x == 0) {
        return COMPFUNC_HARD_FAIL;
    }
    if(x == 1) {
        *result = true;
        return COMPFUNC_SUCCESS;
    }
    if(x > 1) {
        int r = rand()%10;
        if(r == 1) {
            *result = false;
            return COMPFUNC_SUCCESS;
        }
        return COMPFUNC_SOFT_FAIL;
    }
}