#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "symboltable.h" //used to look up 
//global var needed 
int count = 0;
int params = 0;
int vaList[3];
int vaNum = 0;
FILE *x86;
int nextFreeReg = 0;
const int MaxNumOfLocalVar = 4;

//start of functions 
/*define if declaration was a fucntion or variable
Also sets up a proluge if a function*/
void declaration(int type, char *id){
    if(type == FUNC){
        fprintf(x86, "global %s\n", id);
        fprintf(x86, "%s:\n", id);
        fprintf(x86, ";prologue\n");
        fprintf(x86, "push ebp\n");
        fprintf(x86, "mov ebp, esp\n");
        fprintf(x86, "push ebx\n");
        fprintf(x86, "push esi\n");
        fprintf(x86, "push edi\n");
        //space for locals
        fprintf(x86, "sub esp, %i\n\n", 4*MaxNumOfLocalVar);
    }
    else if(type == VAR){
        if(findSym(id)->attr.array){
            fpos_t pos;
            fgetpos(x86, &pos);
            fseek(x86, 14, SEEK_SET);
            fprintf(x86, "%s times %i dd 0\n", id,findSym(id)->attr.arrSize);
            fsetpos(x86, &pos);
        }
        else if(CurrentScope == &globalSymTab)
            fprintf(x86, "%s: dd\n", id);
        else
            findSym(id)->attr.localVarStackOffset = CurrentScope->numOfLocalVar++;
    }
    else {printf("invalid declaration\n"); exit(0);}
}
/*Our basic math ops in nasm*/
void mathOp(int op, int reg1, int reg2){
    switch(op){
        case ADD:
            fprintf(x86, "add %s, %s\n",stringify(reg1),stringify(reg2));
            break;
        case SUB:
            fprintf(x86, "sub %s, %s\n",stringify(reg1),stringify(reg2));
            break;
        case MULT:
            fprintf(x86, "imul %s, %s\n",stringify(reg1),stringify(reg2));
            break;
        case DIV:
            fprintf(x86, "idiv %s, %s\n",stringify(reg1),stringify(reg2));
            break;
    }
}
/*Basic bool ops in nasm*/
void boolOp(int op, int reg1, int reg2){
    fprintf(x86, "cmp %s, %s\n",stringify(reg1),stringify(reg2));
    switch(op){
        case EQU:
            fprintf(x86, "je Equ%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndEqu%i\n", count);
            fprintf(x86, "Equ%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndEqu%i:\n", count);
            count++;
            break;
        case NEQU:
            fprintf(x86, "jne Nequ%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndNequ%i\n", count);
            fprintf(x86, "Nequ%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndNequ%i:\n", count);
            count++;
            break;
        case LESS:
            fprintf(x86, "jl Lt%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndLt%i\n", count);
            fprintf(x86, "Lt%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndLt%i:\n", count);
            count++;
            break;
        case GTR:
            fprintf(x86, "jg Gt%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndGt%i\n", count);
            fprintf(x86, "Gt%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndGt%i:\n", count);
            count++;
            break;
        case LTEQU:
            fprintf(x86, "jle Lte%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndLte%i\n", count);
            fprintf(x86, "Lte%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndLte%i:\n", count);
            count++;
            break;
        case GTEQU:
            fprintf(x86, "jge Gte%i\n", count);
            fprintf(x86, "mov %s, 0\n", stringify(reg1));
            fprintf(x86, "jmp EndGte%i\n", count);
            fprintf(x86, "Gte%i:\n", count);
            fprintf(x86, "mov %s, 1\n", stringify(reg1));
            fprintf(x86, "EndGte%i:\n", count);
            count++;
            break;
    }
}
/*loads or stores id into reg*/
void memOp(int op, char *id, int reg){
	struct symbolEntry *sym=findSym(id);


	if(op == LOAD){
        if(!sym->attr.initialized){
            printf("error: var %s not init\n",sym->id);
            exit(0);
        }
        
        if(findSym(id)->attr.array){
            fprintf(x86, "mov %s, %s\n", stringify(nextFreeReg), id);
            fprintf(x86, "mov %s, [%s+4*%s]\n", stringify(reg),
                    stringify(nextFreeReg),stringify(findSym(id)->attr.regContainingArrIndex));
        }
        else if(CurrentScope == &globalSymTab)
            fprintf(x86, "mov %s, [%s]\n", stringify(reg), id);
            
		else if(sym->attr.parameters==0)
            fprintf(x86, "mov %s, [ebp-%i]\n", stringify(reg), 4*(sym->attr.localVarStackOffset+3));

        else
            fprintf(x86, "mov %s, [ebp+%i]\n", stringify(reg), 4*(sym->attr.parameters+1));
    }
    else if(op == STORE){
            sym->attr.initialized = 1;

            if(findSym(id)->attr.array){
                fprintf(x86, "mov %s, %s\n", stringify(nextFreeReg), id);
                fprintf(x86, "mov [%s+4*%s], %s\n", stringify(nextFreeReg),
                        stringify(findSym(id)->attr.regContainingArrIndex),stringify(reg));
            }
            else if(CurrentScope == &globalSymTab)
                fprintf(x86, "mov [%s], %s\n", id, stringify(reg));
            else if(sym->attr.parameters==0)
                fprintf(x86, "mov [ebp-%i], %s\n", 4*(sym->attr.localVarStackOffset+3), stringify(reg));
            else
                fprintf(x86, "mov [ebp+%i], %s\n", 4*(sym->attr.parameters+1), stringify(reg));
    }
    else{printf("invalid input[memop]\n"); exit(0);}
}

void storeVar(int reg, int value){
    fprintf(x86, "mov %s, %i\n",stringify(reg), value);

}
/*Nasm Call or func*/
void call(char *id, int vaList[]){
    struct symbolEntry *tmp =findSym(id);
    if(tmp->attr.parameters!= params){
        printf("error: incorrect number of parameters\n");
        exit(0);
    }
    tmp->attr.references++;

    fprintf(x86, "\n;precall\n");
    for(int i=0; i < nextFreeReg - params; i++)
        fprintf(x86, "push %s\n", stringify(i));
        
    fprintf(x86, ";add parameters to stack\n");
    for(int i=params-1; i >= 0; i--)
        fprintf(x86, "push %s\n", stringify(vaList[i]));
    fprintf(x86, "call %s\n", id);
    
    fprintf(x86, "\n;postcall\n");
    if(nextFreeReg-params !=EAX)
        /*registers holding parameters can be freed*/
        fprintf(x86, "mov %s, eax\n", stringify(nextFreeReg-params));
        
    fprintf(x86, "\n;clear stack\n");
        fprintf(x86, "add esp, %i\n", 4*params);

    fprintf(x86, "\n;restore registers\n");
    for(int i= nextFreeReg - params-1 ; i >=0 ; i--)
        fprintf(x86, "pop %s\n", stringify(i));
        
    fprintf(x86, "\n");

    nextFreeReg -= params;
}
//basic nasm epilogue
void epi(){
    fprintf(x86, "\n;epilogue\n");
    fprintf(x86, "pop edi\n");
    fprintf(x86, "pop esi\n");
    fprintf(x86, "pop ebx\n");
    fprintf(x86, "mov esp,ebp\n");
    fprintf(x86, "pop ebp\n");
    fprintf(x86, "ret\n\n");

}
//stringify our reg
char *stringify(int reg){
    switch(reg){
        case EAX:
            return "eax";
        case EBX:
            return "ebx";
        case ECX:
            return "ecx";
        case EDX:
            return "edx";
        case ESI:
            return "esi";
        case EDI:
            return "edi";
	default:
	   printf("error: out of availible registers reg=%i\n", reg);
	   exit(0);
    }
}

//reg managment
int availReg(){
    int tmp = nextFreeReg;
    nextFreeReg++;
    return tmp;
}
void freeReg(){
    nextFreeReg--;
}
void freeAllReg(){
    nextFreeReg=0;
}