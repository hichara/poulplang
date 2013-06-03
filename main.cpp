#include "ast.h"
#include "codegen.h"
#include "log.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace std;

extern StatementBlock* programBlock;
extern int yyparse();

extern bool debugTokens;
extern bool debugAST;

extern bool parseFailed;

std::string line = "---------------------------";

void printAST(){
    cout << line << "\nAbstract Sintax Tree\n" << line << "\n";
}

int main( int argc, char** argv ){
    debugTokens = true;
    debugAST = false;
    Log::isDebugLevel = false;

    // lft-cc [ input-file ]
    if( argc > 1 ){
        freopen( argv[1], "r", stdin );
    }

    if( debugTokens ){
        cout << line << "\nTokens\n" << line << "\n";
    }

    yyparse();

    if( parseFailed ){
        return -1;
    }

    if( debugAST ){
        printAST();
    }

    CodeGenContext context;
    Log::Debug() << line << "\nCode Generator\n" << line << "\n";
    std::string asmCode = context.generateCode(*programBlock);

    ofstream fout("out.ll");
    fout << asmCode;
    fout.close();

    return 0;
}
