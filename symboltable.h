#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#define VAR_NAME_MAX 20
#define MAX_SYMBOLS 10

struct symType{
	int type;
  	int initialized;
  	int references;
	int scope;
	int array;
	int arrSize;
  	int parameters;
  	int localVarStackOffset;
  	int regContainingArrIndex;
};

struct symbolEntry{
	char id[VAR_NAME_MAX];
	int type;	/*variable or function*/
	struct symType attr;
};

struct symTable{
	struct symbolEntry symTab[MAX_SYMBOLS];
	struct symTable *outerScope;
  	int numOfSym;
  	int numOfLocalVar;
};

struct symType parsedsymType;

struct symTable globalSymTab;
struct symTable *CurrentScope;

struct symbolEntry *findSym(char *id);

void insertSym(char *id, struct symType attr, int type);
void insertGlobalSym(char *id, struct symType attr, int type);

void startScope();
void endScope();

void printSym(struct symbolEntry sym);

void resetparsedsymType();

int inFunctionBody();

#endif
