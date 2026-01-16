#ifndef ABC_REGISTER_H
#define ABC_REGISTER_H

typedef struct Register Register;

typedef enum RegisterType 
{
    GPR, 
    FPR,
    ANY
}RegisterType;

typedef struct Register 
{
    char *name;
    int size;               // size of the register in bytes

    int is_pinned;          // its value cannot be changed because it hasnt been consumed, if we need a pinned register, we have to spill it
    int is_spilled;         // has it been pushed to the stack?

    Register *parentRegister; // direct parent al->ax->eax->rax
    Register *parent64Register;  // al->rax; 
    Register *subRegisters[3];   // eax, ax, al

    RegisterType type; //fpr,gpr

}Register;

#endif //ABC_REGISTER_H
