#include <iostream>
#include <stdio.h>
#include  <sstream>
#include "ast.h"
#include "codegen.h"

#include <fstream>

using namespace std;

extern StatementBlock* programBlock;
extern int yyparse();

extern bool debugTokens;
extern bool debugAST;

std::string line = "---------------------------";

void printAST(){
    cout << line << "\nAbstract Sintax Tree\n" << line << "\n";
    //cout << programBlock->str() << endl;
}

int main( int argc, char** argv ){
    debugTokens = true;
    debugAST = true;

    // lft-cc [ input-file [ output-file ] ]
    if( argc > 1 ){
        freopen( argv[1], "r", stdin );
    }

    if( debugTokens ){
        cout << line << "\nTokens\n" << line << "\n";
    }

    yyparse();

    if( debugAST ){
        printAST();
    }

    CodeGenContext context;
    cout << line << "\nCode Generator\n" << line << "\n";
    std::string asmCode = context.generateCode(*programBlock);

    ofstream fout("out.ll");
    fout << asmCode;
    fout.close();

    //context.runCode();

    return 0;
}
