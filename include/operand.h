
#ifndef ABC_OPERAND_H
#define ABC_OPERAND_H

#include "register.h"

typedef struct Operand 
{   
    Register *linkedToRegister;
    int is_unsigned;
    int displacement;
}Operand;


#endif //ABC_OPERAND_H