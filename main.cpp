#include <iostream>
#include <stdio.h>
#include  <sstream>
#include "ast.h"

using namespace std;

extern ExpressionListNode* programBlock; 
extern int yyparse();

extern bool debugTokens;
extern bool debugAST;

std::string line = "---------------------------";

void printAST(){
    cout << line << "\nAbstract Sintax Tree\n" << line << "\n";
    cout << programBlock->str() << endl;
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

    return 0;
}
