%{
#include <string>
#include <stdio.h>
#include "ast.h"

StatementBlock* programBlock;

extern int yylex();
int yyerror( const char* err );
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
%token <string> T_IDENTIFIER T_BUILTIN_TYPE
%token <number> T_NUM_INTEGER T_NUM_DOUBLE
%token <token> T_EQUAL T_CMP_EQ T_CMP_NE T_CMP_LT T_CMP_LE
%token <token> T_CMP_GT T_CMP_GE T_LPAREN T_RPAREN T_LBRACE T_RBRACE
%token <token> T_SEMI T_PLUS T_MINUS T_DIV T_MUL T_COMMA

/*
 *  Rules Declaration
 */
%type <ident>    ident;
%type <expr>     numeric expr;
%type <varVec>  func_decl_args;
%type <exprList> call_args;
%type <block>    program stmts block
%type <stmt>     stmt var_decl func_decl
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
;

block   : T_LBRACE stmts T_RBRACE       { $$ = $2; }
        | T_LBRACE T_RBRACE             { $$ = new StatementBlock(); }
;

ident   : T_IDENTIFIER                  { $$ = new Identifier( *$1 ); }
;

var_decl : ident ident                  { $$ = new VariableDeclaration( *$1, *$2 ); }
         | ident ident T_EQUAL expr     { $$ = new VariableDeclaration( *$1, *$2, $4 ); }
;

func_decl : ident ident T_LPAREN func_decl_args T_RPAREN block
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

expr : ident T_EQUAL expr               { $$ = new Assignment(*$<ident>1, $3); }
     | ident T_LPAREN call_args T_RPAREN
                                        { $$ = new MethodCall(*$1, *$3); }
     | ident                            { $<ident>$ = $1; }
     | numeric
     | expr comparison expr             { $$ = new BinaryOperation($2, *$1, *$3); }
     | T_LPAREN expr T_RPAREN           { $$ = $2; }
;

call_args : /*empty*/                   { $$ = new ExpressionList(); }
          | expr                        { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args T_COMMA expr      { $1->push_back($3); }
;

comparison : T_CMP_EQ | T_CMP_NE | T_CMP_LT | T_CMP_LE | T_CMP_GT | T_CMP_GE
           | T_PLUS | T_MINUS | T_MUL | T_DIV
;

%%


int yyerror( const char* err )
{
    printf("ERROR: %s\n", err);
    return 0;
}
