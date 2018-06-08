#ifndef ASM_H
#define ASM_H

int vaList[3]; //Variable Argument == VA
int vaNum;
int params;
int count;
FILE *x86;

void mathOp(int op, int reg1, int reg2);
enum {ADD, SUB, MULT, DIV} MATH_OPS;

void boolOp(int op, int reg1, int reg2);
enum {EQU, NEQU, LESS, GTR, LTEQU, GTEQU} BOOL_OPS;

void memOp(int op, char *id, int reg);
enum {LOAD, STORE} MEMORY_OPS;

void storeVar(int reg, int value);

void declaration(int type, char *id);
enum {VAR, FUNC} FUNC_DEC;

void call(char *id, int vaList[]);


void epi();

char *stringify(int reg);
enum {EAX, EBX, ECX, EDX, ESI, EDI} REG_NAME;

int availReg();
void freeReg();
void freeAllReg();

#endif