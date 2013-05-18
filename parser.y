%{
#include <string>
#include <stdio.h>
extern int yylex();
int yyerror( const char* err );
%}

%union {
    std::string* string;
    int token;
}

%token <string> T_IDENTIFIER T_NUM_INTEGER T_NUM_DOUBLE
%token <token> T_EQUAL T_CMP_EQ T_CMP_NE T_CMP_LT T_CMP_LE
%token <token> T_CMP_GT T_CMP_GE T_LPAREN T_RPAREN T_LBRACE T_RBRACE
%token <token> T_SEMI T_PLUS T_MINUS T_DIV T_MUL

%type <token> program

%left T_PLUS T_MINUS
%left T_DIV T_MUL

%start program

%%

program : | T_IDENTIFIER program { printf("%s", $1->c_str()); }

%%

int yyerror( const char* err )
{
    printf("ERROR: %s\n", err);
    return 0;
}
