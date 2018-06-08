# SJSU-CMPE152-C-Minus-Compiler
This is a final submission of the 152 Compiler project
There are a few known issues with this code
 - The compiler **can not** handle nested while loops (but it is a easy fix, look how for loops are done and production rules with yacc)
 - It is a single pass compiler with no optimization so the asm is correct it is just not optimal 
 - You can probably figure out a better way to store the symbol table then I did
 
To build the project run the following commands

```
yacc -d cminus.y
lex cminus.l
cc lex.yy.c y.tab.c asm.c symboltable.c
./a.out << 'your cminus file'

```

This code only works in a linux enviroment, and if you work only in x64 you will need multilib installed
To generage the asm and exe run
```
nasm -f elf -g -F stabs output.asm -l output.lst
gcc –m32 output.o –o output
````

The exe thats generated will run, but its up to you to tell if its working, as our gramar does not have any console printing.  Some other versions of this people had used printf in the x86 to debug, but I did not find it nessascary to do.  

## Happy Coding!
