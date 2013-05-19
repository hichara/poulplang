#include "ast.h"
using namespace std;

const char* IdentChars = "  ";

const char* GetOperationName( int op ){
    static const char* OpNames[] = { "OP_ADD", "OP_SUB", "OP_DIV", "OP_MUL", "OP_MINUS" };
    return OpNames[op];
}

const std::string GetIdentation( int level, const std::string& chars  ){
    stringstream stream;
    for( int i = 0; i < level; ++i ){
        stream << chars;
    }
    return stream.str();
}
