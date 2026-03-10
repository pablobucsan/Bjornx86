#ifndef ABC_PARAMETER_H
#define ABC_PARAMETER_H


#include "types.h"

/**
 * ===============================================================================
 * PARAMETER DEFINITION      
 * ===============================================================================   
 */

typedef enum PKind
{
    P_INMEDIATE,
    P_VARARGS,
}PKind;

typedef struct Parameter
{
    PKind ptype;
    Type *type;
    char *name;
}Parameter;

#endif //ABC_PARAMETER_H
