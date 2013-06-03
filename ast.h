#ifndef __AST_H__
#define __AST_H__

#include <sstream>
#include <string>
#include <vector>
#include <llvm/Value.h>

enum {
    OP_ADD,
    OP_SUB,
    OP_DIV,
    OP_MUL,
    OP_MINUS
};

typedef std::string String;

class CodeGenContext;

class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement*> StatementList;
typedef std::vector<Expression*> ExpressionList;
typedef std::vector<VariableDeclaration*> VariableList;


class Node{
public:
    //virtual ~Node();
    virtual String str( int ident = 0 ) { return "Node"; }
    virtual llvm::Value* codeGen(CodeGenContext& context) { return 0; }
};

class Expression : public Node {
public:
    String str( int ident = 0 );
};

class Statement : public Node {
};

class Identifier : public Expression {
public:
    const String& name;

    Identifier( const String& name ) : name(name) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Integer : public Expression {
public:
    int value;
    Integer( int value ) : value(value){}

    //String str( int ident = 0 );

    llvm::Value* codeGen(CodeGenContext& context);
};

class Double : public Expression {
public:
    double value;
    Double( double value ) : value(value){}

    //String str( int ident = 0 );

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BinaryOperation : public Expression {
public:
    int op;
    Expression& rhs;
    Expression& lhs;

    BinaryOperation( int op, Expression& lhs, Expression& rhs ) :
        op(op), lhs(lhs), rhs(rhs) {}

    String str( int ident = 0 );

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class StatementBlock: public Expression {
public:
    StatementList statements;
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class MethodCall : public Expression {
public:
    const Identifier& methodName;
    ExpressionList arguments;

    MethodCall( const Identifier& name, ExpressionList& args ) :
        methodName(name), arguments(args) { }

    MethodCall( const Identifier& name ) :
        methodName(name) { }

    //virtual String str( int ident );

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Assignment : public Expression {
public:
    const Identifier& lhs;
    Expression* rhs;

    Assignment( const Identifier& lhs, Expression* rhs ) :
        lhs(lhs), rhs(rhs) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};


class FunctionDeclaration : public Statement {
public:
    const Identifier& functionType;
    const Identifier& functionName;
    VariableList arguments;
    StatementBlock block;

    FunctionDeclaration( const Identifier& type, const Identifier& name, VariableList args, StatementBlock& block ) :
        functionType(type), functionName(name), arguments(args), block(block) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ExpressionStatement : public Statement {
public:
    Expression& expression;

    ExpressionStatement( Expression& expression ) :
        expression( expression ) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class VariableDeclaration : public Statement {
public:
    const Identifier& type;
    const Identifier& name;
    Expression* assignmentExpression;

    VariableDeclaration( const Identifier& type, const Identifier& name ) :
        type(type), name(name) { }

    VariableDeclaration( const Identifier& type, const Identifier& name, Expression* value ) :
        type(type), name(name), assignmentExpression(value) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

#endif
