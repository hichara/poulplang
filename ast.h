#ifndef __AST_H__
#define __AST_H__

#include <sstream>
#include <string>
#include <vector>

enum {
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_MINUS
};

const char* GetOperationName( int op );
const std::string GetIdentation( int level, const std::string& chars );
extern const char* IdentChars;

class ASTNode{
public:
    virtual std::string str( int ident = 0 ) = 0;
};

class ExpressionNode : public ASTNode {
public:
    ExpressionNode(){}

    std::string str( int ident = 0 ){
        return GetIdentation( ident, IdentChars ) + "Expression Node";
    }
};

class VariableNode : public ExpressionNode {
public:
    const std::string& name;

    VariableNode( const std::string& varName ) :
        name( varName ){
    }

    std::string str( int ident = 0 ){
        return GetIdentation( ident, IdentChars ) + "Variable Node " + name;
    }
};

class LiteralValueNode : public ExpressionNode {
public:
    double value;
    LiteralValueNode( double value ){
        this->value = value;
    }

    std::string str( int ident = 0 ){
        std::stringstream stream;
        stream  << GetIdentation( ident, IdentChars )
                << "Literal Value " << value;
        return stream.str();
    }
};

class BinaryArithOpNode : public ExpressionNode {
public:
    int op;
    ExpressionNode *rhs, *lhs;

    BinaryArithOpNode( int op, 
        ExpressionNode* lhs,
        ExpressionNode* rhs ) :
        op(op), lhs(lhs), rhs(rhs){
    }

    std::string str( int ident = 0 ){
        std::string il1 = GetIdentation( ident, IdentChars );
        std::string il2 = GetIdentation( ident + 1, IdentChars );

        std::stringstream stream;
        stream  << il1 << "BinaryArithmetic" << "\n"
                << il2 << GetOperationName(op) << "\n"
                << lhs->str( ident + 1 ) << "\n"
                << rhs->str( ident + 1 );
        return stream.str();
    }
};

class BinaryLogicOperationNode : public ASTNode {
public:
    std::string str( int ident = 0 ){
        return GetIdentation( ident, IdentChars ) + "Binary Logic Operation Node";
    }
};

class FunctionDefinitionNode : public ASTNode {
public:
    std::string str( int ident = 0 ){
        return GetIdentation( ident, IdentChars ) + "Function Definition Node";
    }
};

class StatementListNode : public ASTNode {
public:
    std::string str( int ident = 0 ){
        return GetIdentation( ident, IdentChars ) + "Statement List Node";
    }
};

class ExpressionListNode : public ASTNode {
public:
    std::vector<ASTNode*> nodes;
    ExpressionListNode( ASTNode* elem ){
        nodes.push_back( elem );
    }
    
    ExpressionListNode* append( ASTNode* elem ){
        nodes.push_back(elem);
        return this;
    }

    std::string str( int ident = 0 ){
        std::stringstream stream;
        std::string il1 = GetIdentation( ident, IdentChars );
        std::string il2 = GetIdentation( ident, IdentChars );

        stream << il1 << "ExpressionListNode" << "\n";

        for( int i = 0; i < nodes.size()-1; ++i ){
            stream << nodes[i]->str( ident + 1 ) << "\n";
        }

        stream << nodes[nodes.size()-1]->str( ident + 1 ) << "\n";
        return stream.str();
    }
};

#endif
