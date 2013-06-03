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

String Expression::str( int ident ){
    return "Expression Node";
}

String BinaryOperation::str( int ident ){
    std::string il1 = GetIdentation( ident, IdentChars );
    std::string il2 = GetIdentation( ident + 1, IdentChars );

    std::stringstream stream;
    stream  << il1 << "BinaryArithmetic" << "\n"
            << il2 << GetOperationName(op) << "\n"
            << lhs.str( ident + 1 ) << "\n"
            << rhs.str( ident + 1 );
    return stream.str();
}
