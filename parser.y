%{
#include <string>
#include <stdio.h>
#include "ast.h"

ExpressionListNode* programBlock;

extern int yylex();
int yyerror( const char* err );
%}

%union {
    std::string* string;
    int token;
    double number;
    ASTNode* node;
    ExpressionListNode* exprlist;
    ExpressionNode* exprnode;
}

%token <string> T_IDENTIFIER 
%token <number> T_NUM_INTEGER T_NUM_DOUBLE
%token <token> T_EQUAL T_CMP_EQ T_CMP_NE T_CMP_LT T_CMP_LE
%token <token> T_CMP_GT T_CMP_GE T_LPAREN T_RPAREN T_LBRACE T_RBRACE
%token <token> T_SEMI T_PLUS T_MINUS T_DIV T_MUL

%type <exprlist> stmts;
%type <exprnode> term factor number arith_expr instruction
%type <token> program

%left T_PLUS T_MINUS
%left T_DIV T_MUL

%start program

%%

// Expression List Node
program : stmts                         { programBlock = $1; }
;

// Expression List Node
stmts   : stmts instruction T_SEMI      { $$ = $1->append($2); }
        | instruction T_SEMI            { $$ = new ExpressionListNode($1); }
;

// Expression Node
instruction : arith_expr                { $$ = $1; }
;

// Arithmetic Expression Node
arith_expr  : arith_expr T_PLUS term    { $$ = new BinaryArithOpNode(OP_ADD, $1, $3); }
            | arith_expr T_MINUS term   { $$ = new BinaryArithOpNode(OP_SUB, $1, $3); }
            | term                      { $$ = $1; }
;

// Arithmetic Expression Node
term    : term T_MUL factor             { $$ = new BinaryArithOpNode(OP_MUL, $1, $3); }
        | term T_DIV factor             { $$ = new BinaryArithOpNode(OP_DIV, $1, $3); } 
        | factor                        { $$ = $1; }
;

// Arithmetic Expression Node
factor  : number                        { $$ = $1; }
        | T_MINUS factor                { $$ = $2; }
        | T_LPAREN arith_expr T_RPAREN  { $$ = $2; }
;

// Literal Value Node
number  : T_NUM_INTEGER                 { $$ = new LiteralValueNode($1); }
        | T_NUM_DOUBLE                  { $$ = new LiteralValueNode($1); }
;

%%

int yyerror( const char* err )
{
    printf("ERROR: %s\n", err);
    return 0;
}
