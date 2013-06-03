%{
#include <string>
#include <stdio.h>
#include "ast.h"

StatementBlock* programBlock;

extern int yylex();
extern int lineNumber;
extern char* yytext;
int yyerror( const char* err );

bool parseFailed = false;

%}

%union {
    int                     token;
    double                  number;
    String*                 string;
    Node*                   node;
    Expression*             expr;
    Statement*              stmt;
    StatementBlock*         block;
    std::vector<VariableDeclaration*>    *varVec;
    VariableDeclaration   *varDecl;
    Identifier*             ident;
    VariableList*           varList;
    ExpressionList*         exprList;
}

/*
 *  Token Declaration
 */
%token <string> T_IDENTIFIER T_BUILTIN_TYPE T_STR T_IF T_ELSE
%token <number> T_NUM_INTEGER T_NUM_DOUBLE
%token <token> T_EQUAL T_CMP_EQ T_CMP_NE T_CMP_LT T_CMP_LE T_PRINTF T_RETURN
%token <token> T_CMP_GT T_CMP_GE T_LPAREN T_RPAREN T_LBRACE T_RBRACE
%token <token> T_SEMI T_PLUS T_MINUS T_DIV T_MUL T_COMMA

/*
 *  Rules Declaration
 */
%type <ident>    identifier
%type <expr>     numeric expr factor term arith_expr printf logic_expr fun_call
%type <varVec>   func_decl_args
%type <exprList> call_args
%type <block>    program stmts block
%type <stmt>     stmt var_decl func_decl return_stmt branch_stmt branch_stmt2
%type <token>    comparison

%left T_PLUS T_MINUS
%left T_DIV T_MUL

%start program

%%

program : stmts                         { programBlock = $1; }
;

stmts   : stmt T_SEMI                   { $$ = new StatementBlock(); $$->statements.push_back($<stmt>1); }
        | stmts stmt T_SEMI             { $1->statements.push_back( $<stmt>2 ); }
;

stmt    : var_decl | func_decl | expr   { $$ = new ExpressionStatement( *$1 ); }
        | return_stmt                   { $$ = $1; }
        | branch_stmt                   { $$ = $1; }
        | branch_stmt2                  { $$ = $1; }
;

return_stmt : T_RETURN expr
                                        { $$ = new ReturnStatement( $2 ); }
;

branch_stmt : T_IF T_LPAREN expr T_RPAREN block T_ELSE block
                                        { $$ = new BranchStatement( $3, *$5, *$7 ); }

branch_stmt2 : T_IF T_LPAREN expr T_RPAREN block
                                        { $$ = new BranchStatement( $3, *$5 ); }
;

block   : T_LBRACE stmts T_RBRACE       { $$ = $2; }
        | T_LBRACE T_RBRACE             { $$ = new StatementBlock(); }
;

identifier: T_IDENTIFIER                { $$ = new Identifier( *$1 ); }
;

var_decl : identifier identifier        { $$ = new VariableDeclaration( *$1, *$2 ); }
         | identifier identifier T_EQUAL expr
                                        { $$ = new VariableDeclaration( *$1, *$2, $4 ); }
;

func_decl : identifier identifier T_LPAREN func_decl_args T_RPAREN block
                                        { $$ = new FunctionDeclaration( *$1, *$2, *$4, *$6 ); }
;

func_decl_args  : /* empty */           { $$ = new VariableList(); }
                | var_decl              { $$ = new VariableList(); $$->push_back($<varDecl>1); }
                | func_decl_args T_COMMA var_decl
                                        { $1->push_back($<varDecl>3); }
;

numeric : T_NUM_INTEGER                 { $$ = new Integer( $1 ); }
        | T_NUM_DOUBLE                  { $$ = new Double( $1 ); }
;

expr : identifier T_EQUAL expr          { $$ = new Assignment(*$<ident>1, $3); }
     | fun_call                         { $$ = $1; }
     | identifier                       { $<ident>$ = $1; }
     | printf                           { $$ = $1; } 
     | numeric
     | arith_expr                       { $$ = $1; }
     | logic_expr                       { $$ = $1; }
     | T_LPAREN expr T_RPAREN           { $$ = $2; }
;

fun_call : identifier T_LPAREN call_args T_RPAREN
                                        { $$ = new MethodCall(*$1, *$3); }
;

call_args : /*empty*/                   { $$ = new ExpressionList(); }
          | expr                        { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args T_COMMA expr      { $1->push_back($3); }
;

arith_expr  : arith_expr T_PLUS term    { $$ = new BinaryOperation($2, *$1, *$3); }
            | arith_expr T_MINUS term   { $$ = new BinaryOperation($2, *$1, *$3); }
            | term                      { $$ = $1; }
;

term    : term T_MUL factor             { $$ = new BinaryOperation($2, *$1, *$3); }
        | term T_DIV factor             { $$ = new BinaryOperation($2, *$1, *$3); }
        | factor                        { $$ = $1; }
;

factor  : numeric                       { $$ = $1; }
        | identifier                    { $$ = $1; }
        | fun_call                      { $$ = $1; }
        | T_MINUS factor                { $$ = new BinaryOperation(T_MINUS, *(new Integer(0)), *$2); }
        | T_LPAREN arith_expr T_RPAREN  { $$ = $2; }
;

logic_expr : expr comparison expr       { $$ = new BinaryOperation( $2, *$1, *$3 ); }
;

comparison : T_CMP_EQ | T_CMP_NE | T_CMP_LT | T_CMP_LE | T_CMP_GT | T_CMP_GE
;

printf : T_PRINTF T_LPAREN T_STR T_COMMA call_args T_RPAREN
                                        { $$ = new PrintfMethodCall( *$3, *$5 ); }
;
%%


int yyerror( const char* err )
{
    printf("ERROR at line %d, unexpected \'%s\'\n", lineNumber, yytext );
    parseFailed = true;
    return 0;
}
