uni-academic-c
==============

Implementation of a C like language using flex, bison and llvm

llvm
----
* llvm-as -f <outfile>
    LLVM Assembler. Reads a file containing human-readable LLV assembly language and translates it to LLVM bitcode (.bc)

* llc -o <outfile> <bcfile>
    LLVM static compiler. Reads a file containing LLVM bitcode (.bc) and compiles it into assembly language for a specified architecture (.s). The assembly language output can then be passed through a native assembler and linker to generate a native executable

running-instructions
--------------------
* make clean

* make
    Builds the lexer, parser and codegen. 
    Result: 'lft-cc' executable
    
* make run
    Runs the compiler on the dummy source code.
    Result: 'out.ll' llvm assembly code
    
* make llvm-as
    Runs the llvm assembler on the out.ll file. Run xxd to view binary code.    
    Result: `out.bc` llvm bitcode
    
* make llc
    Runs the LLVM compiler on the out.bc file.
    Result: `out.s` native assembly code

* make native-compiler
    Runs the gcc compiler on the out.s assembly file
    Result: `out` executable

