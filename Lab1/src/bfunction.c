#include "bfunction.h"

compfunc_status_t bFunction(int x, bool y, bool *result) {
    if(x == 0 || y == false)
        *result = false;
    else
        *result = true;
    return COMPFUNC_SUCCESS;
}