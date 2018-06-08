#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symboltable.h"



struct symTable globalSymTab={.outerScope=NULL, .numOfSym=0,.numOfLocalVar=0};
struct symTable *CurrentScope = &globalSymTab;

int ScopeLevel = 0; //Determines if Global or not
int ScopeCount = 0; //increments for each new scoper were in 
/*Set a baseline */
struct symType parsedsymType = {
  .type=0,
  .initialized=0,
  .references=0,
  .scope=0,
  .array=0,
  .arrSize=0,
  .parameters=0,
  .localVarStackOffset=0
};
/*make sure that var is of a decared type*/
struct symbolEntry *findSym(char *id){
  struct symTable *tablePointer = CurrentScope;
  while(tablePointer != NULL){
    for(int i=0; i < tablePointer->numOfSym; i++){
      if(strcmp(tablePointer->symTab[i].id, id) == 0){
        return &tablePointer->symTab[i];
      }
    }
    tablePointer = tablePointer-> outerScope;
  }
  printf("error - id: %s was never declared\n", id);
  exit(0);
}
/*inserts symbol into the table */
void insertSym(char *id, struct symType attr, int type){
  struct symbolEntry symbol = {.type = type,
                               .attr = attr
                              };
  strcpy(symbol.id, id);
  if(strcmp(id, "main")==0)
    symbol.attr.references=1;
    
  struct symTable *tablePointer = CurrentScope;
  while(tablePointer != NULL){
    for(int i=0; i < tablePointer->numOfSym; i++){
      if(strcmp(tablePointer->symTab[i].id, id) == 0){
        printf("error - id: %s was previously declared\n", id);
        exit(0);
      }
    }
    tablePointer = tablePointer-> outerScope;
  }
  CurrentScope->symTab[CurrentScope->numOfSym] = symbol;
  CurrentScope->numOfSym++;
}
/*For the global table*/
void insertGlobalSym(char *id, struct symType attr, int type){
  struct symbolEntry symbol = {.type = type,
                               .attr = attr
                              };
  strcpy(symbol.id, id);
  if(strcmp(id, "main")==0)
    symbol.attr.references=1;

  struct symTable *tablePointer = &globalSymTab;
  for(int i=0; i < tablePointer->numOfSym; i++){
      if(strcmp(tablePointer->symTab[i].id, id) == 0){
        printf("error - id: %s was previously declared\n", id);
        exit(0);
      }
  }
  tablePointer->symTab[tablePointer->numOfSym] = symbol;
  tablePointer->numOfSym++;
}
/*Start a scope, inc level and count then allocate space for entries in table
  and point the scope to the correct level*/
void startScope(){
  ScopeLevel++;
  ScopeCount++;
  struct symTable *newTable = (struct symTable*) malloc(sizeof(struct symTable));
  newTable->outerScope = CurrentScope;
  newTable->numOfSym = 0;
  CurrentScope = newTable;
}

void endScope(){
  for(int i=0; i < CurrentScope->numOfSym; i++){
    printSym(CurrentScope->symTab[i]);
    printf("\n");

    if(CurrentScope->symTab[i].attr.references == 0){
      printf("warning - id \"%s\" was not referenced\n\n",
      CurrentScope->symTab[i].id);
    }
  }
  printf("\n");

  if(ScopeLevel){
    struct symTable *temp = CurrentScope;
    CurrentScope = CurrentScope->outerScope;
    free(temp);
    temp = NULL;
    ScopeLevel--; //only dec this to determine global or not
  }
}

enum {VAR, FUNC};
void printSym(struct symbolEntry sym){//funtion used to simplify printing of the table
  printf("Symbol = %s, ", sym.id);
  if(sym.type == VAR)
    printf("Variable, ");
  else
    printf("Function Parameters:%i, ", sym.attr.parameters);
  
  if(sym.attr.type == 0)
    printf("INT, ");
  else
    printf("VOID, ");

  if(sym.attr.array == 1)
    printf("array, size:%i, ", sym.attr.arrSize);
  /*print out which scope unless the scope is global*/
  if(ScopeLevel==0)
    printf("Scoope:%i",ScopeLevel);
  else
    printf("Scope:%i",ScopeCount);
}
/*does what it says*/
void resetparsedsymType(){
  parsedsymType.type=0;
  parsedsymType.initialized=0;
  parsedsymType.references=0;
  parsedsymType.scope=0;
  parsedsymType.array=0;
  parsedsymType.arrSize=0;
  parsedsymType.parameters=0;
  parsedsymType.localVarStackOffset=0;
}
/*for bison*/
int inFunctionBody(){
  if (ScopeLevel == 1)
    return 1;
  return 0;
}