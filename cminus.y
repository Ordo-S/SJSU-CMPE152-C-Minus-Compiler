/* C-Minus BNF Grammar */
%{ 
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "asm.h"
  #include "symboltable.h"

  extern int yylineno;
  int yylex();
  void yyerror(const char *s);
//for symTable.h
  extern struct symType parsedsymType;
  extern struct symTable globalSymTable;
  extern struct symTable *CurrentScope;
//for asm.h
  extern int count;
  extern int params;
  extern int vaList[3];
  extern int vaNum;

  extern FILE *x86;
  int TextSection = 0; //if textsection = 0 then create it 
  //prevents multiple text sections to be created 
%}

%union {
   	int n;
  	char s[15];
}

%token ELSE
%token IF
%token INT
%token RETURN
%token VOID
%token WHILE

%token ID
%token NUM

%token LTE
%token GTE
%token EQUAL
%token NOTEQUAL

%type<n>    NUM call factor term expression type_specifier simple_expression additive_expression addop mulop relop iteration_stmt selection_stmt if_stmt

%type<s> ID var var_declaration


%%

program : declaration_list {endScope();}; //once the rule is met need to close the golobal scope
                                                                  

declaration_list : declaration_list declaration     
                 | declaration          
                 ;

declaration  : var_declaration 
	           | fun_declaration
             ;

var_declaration : type_specifier ID ';'	{
                                         insertSym($2, parsedsymType, VAR);
                                         strcpy($$, $2);
                                         resetparsedsymType();
                                        }
                | type_specifier ID '[' NUM ']' ';' {
                                                     parsedsymType.array = 1;
                                                     parsedsymType.arrSize = $4;
                                                     insertSym($2, parsedsymType, VAR);
                                                     strcpy($$, $2);
                                                     resetparsedsymType();
                                                    }
                ;

type_specifier : INT	  {$$ = 0; parsedsymType.type = 0;}
               | VOID   {$$ = 1; parsedsymType.type = 1;}
	             ;

fun_declaration : type_specifier ID '('     {
                                            if(!TextSection++) fprintf(x86, "\nSECTION .text\n");
                                            //decalre .text if has not been declared yet
                                             declaration(FUNC, $2);
                                             startScope();
                                            }
                  params ')'                {
                                             parsedsymType.type = $1;
                                             parsedsymType.parameters = params;
                                             insertGlobalSym($2, parsedsymType, FUNC);
											                       resetparsedsymType();
                                             params=0;
											                      }
				          compound_stmt  			      {
                                            endScope();
                                             if($1 == 1)
                                              	epi();      
                                            freeAllReg();
                                            }
                ;

params : param_list | VOID ;

param_list : param_list ',' param {params++; }
           | param 		            {params++; }
	         ;

param : type_specifier ID           {
                                     parsedsymType.parameters = params + 1;
                                     parsedsymType.initialized = 1;
                                     insertSym($2, parsedsymType, VAR);
                                     resetparsedsymType();
                                    }
      | type_specifier ID '[' ']'   {
                                     parsedsymType.parameters = params + 1;
                                     parsedsymType.array = 1;
                                     parsedsymType.initialized = 1;
                                     insertSym($2, parsedsymType, VAR);
                                     resetparsedsymType();
                                    }
      ;


compound_stmt : '{'                                     { //start scope b4 pararms read
                                                        if(!inFunctionBody())
                                                            startScope();
                                                        }
                 local_declarations statement_list '}'  {if(!inFunctionBody())
                                                            endScope();}
              ;

local_declarations : local_declarations var_declaration {declaration(VAR, $2);}
                   | /* empty */ ;

statement_list : statement_list statement
               | /* empty */ ;

statement : expression_stmt
          | compound_stmt
          | selection_stmt
          | iteration_stmt
          | return_stmt ;

expression_stmt : expression ';'
                | ';' 
				;

selection_stmt : if_stmt  statement        {fprintf(x86, "EndIf%i:\n", $1);}
               | if_stmt  statement ELSE   {fprintf(x86, "jmp EndIfElse%i\n", $1);
                                                 fprintf(x86, "EndIf%i:\n", $1);
                                                }
                 statement                      {fprintf(x86, "EndIfElse%i:\n", $1);}
               ;
                  /*
                  this was the oriinal code but bison gave a error so "rule useless in parser due to conflicts", this was 
                  due to bison only looks ahead one production rule.  But from searching people had a similar proble and the issues was 
                  "two productions for statement both have code blocks before the parser has read any tokens to know which production is going to apply."
                  hence a new production rule was added 
                  IF '(' expression ')'  {
                                        $<n>$ = count; count++;
                                        fprintf(x86, "cmp %s, 1\n", stringify($3));
                                        fprintf(x86, "jne EndIf%i\n", $<n>$);
                                        freeReg();
                                       } 
                 statement        {fprintf(x86, "EndIf%i:\n", $<n>1);}
               | IF  '(' expression ')'   {
                                           $<n>$ = count; count++;
                                           fprintf(x86, "cmp %s, 1\n", stringify($3));
                                           fprintf(x86, "jne EndIf%i\n", $<n>$);
                                           freeReg();
                                          }  
                statement ELSE	  {
                                   fprintf(x86, "jmp EndIfElse%i\n", $<n>1);
                              		 fprintf(x86, "EndIf%i:\n", $<n>1);
                      						}
				         statement					 	          {fprintf(x86, "EndIfElse%i:\n", $<n>1);}
               ;
               */
if_stmt : IF  '(' expression ')'  {
                                      $$ = count; count++;
                                      fprintf(x86, "cmp %s, 1\n", stringify($3));
                                      fprintf(x86, "jne EndIf%i\n", $$);
                                      freeReg();
                                       }
       ;
iteration_stmt : WHILE    {
                            $<n>$ = count; count++; //$<n>$ used beacsue its a midrule 
                            fprintf(x86, "While%i:\n", $<n>1);
                           }

                '(' expression ')' {
                                     fprintf(x86, "cmp %s, 1\n", stringify($<n>3));
                                     fprintf(x86, "jne EndWhile%i\n", $<n>1);
                                     freeReg();
                                    }
                 statement          {
                                     fprintf(x86, "jmp While%i\n", $<n>1);
                                     fprintf(x86, "EndWhile%i:\n", $<n>1);
                                    }
                ;
/*If a similar production rule to if_stmt was made, then the code should have no issue running a nested
while loop, but when compiling bison can only look one ahead, this causes a usless rule to be formed when 
makeing a nestled while loop and causes a conflic in parser*/

return_stmt : RETURN ';'              {epi();}

            | RETURN expression ';'   {
                                        if($2 == EAX){
                                        epi();
									                     }
                                       else{
                                        fprintf(x86, "mov eax, %s\n", stringify($2));
                                        epi();
                                       }
                                       freeReg();
                                      }

            ;

expression : var '=' expression     {$$=9;
                                     memOp(STORE,$1,$3);
                                     freeReg();
                                    }
           | simple_expression      {$$ = $1;}
           ;

var : ID                    {findSym($1)->attr.references++; strcpy($$, $1);}

    | ID '[' expression ']' {
                             struct symbolEntry *tmp = findSym($1);
                             if(!tmp->attr.array)
                             printf("error - %s is not an array", tmp->id);
                             tmp->attr.references++;
                             tmp->attr.regContainingArrIndex = $3;
                             strcpy($$, $1);
                            }
    ;

simple_expression : additive_expression relop additive_expression  {
                                                                    $$=$1;
                                                                    boolOp($2,$1,$3);
                                                                    freeReg();
                                                                   }
                  | additive_expression                            {$$ = $1;
                                                                   }
                  ;

relop : LTE {$$=LTEQU;}| '<'{$$=LESS;} | '>' {$$=GTR;}| GTE{$$=GTEQU;} | EQUAL{$$=EQU;} | NOTEQUAL {$$=NEQU;};

additive_expression : additive_expression addop term    {
                                                         $$ = $1;
                                                         mathOp($2,$1,$3);
                                                         freeReg();
                                                        }
                    | term                              {$$ = $1;}
                    ;

addop : '+' {$$ = ADD;}
      | '-' {$$ = SUB;}
      ;

term : term mulop factor    {$$ = $1;
                             mathOp($2,$1,$3);
                             freeReg();
                            }
     | factor               {$$ = $1;}
     ;

mulop : '*' {$$ = MULT;}
      | '/' {$$ = DIV;}
      ;

factor : '(' expression ')' {$$ = $2;}
       | var                {
                             $$ = availReg();
                             memOp(LOAD,$1,$$);
                            }

       | call               {$$ = $1;}
       | NUM                {$$=availReg();
                             storeVar($$, $1);
                            }
       ;

call : ID '(' args ')'  {
                         call($1, vaList);
                         $$=availReg();
                         params=0;
                        }
     ;

args : arg_list | /* empty */ ;

arg_list : arg_list ',' expression {vaList[params++] = $3;}
         | expression              {vaList[params++] = $1;}
         ;

%%
int main(){	
	x86 = fopen ("output.asm", "w");
	if(!yyparse())
		printf("\nParsing complete\n");
	else
		printf("\nParsing failed\n");
	fclose(x86);
    	return 0;
}




void yyerror (char const *s)
{
  fprintf (stderr, "%s, line:%i\n", s, yylineno);
}


