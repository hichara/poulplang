%{
#include <string>
#include <stdio.h>
#include "ast.h"
#include "parser.hpp"

extern bool debugTokens;

// Helper export functions
inline int strToken( int token ){
    yylval.string = new std::string( yytext, yyleng );

    if( debugTokens ){
        printf("TOKEN\tIDENTIFIER\t%d\t%s\n", token, yytext);
    }

    return token;
}

inline int numToken( int token ){
    yylval.token = token;

    if( debugTokens ){
        printf("TOKEN\tKEYWORD\t\t%d\t%s\n", token, yytext);
    }

    return token;
}

inline double dblToken( int token ){
    sscanf( yytext, "%lf", &yylval.number );

    if( debugTokens ){
        printf("TOKEN\tNUMBER\t\t%d\t%s\n", token, yytext);
    }

    return token;
}

%}

%%
[ \t\n]                 ;

[a-zA-Z_][a-zA-Z0-9_]*  return strToken(T_IDENTIFIER);
[0-9]+\.[0-9]*          return dblToken(T_NUM_DOUBLE);
[0-9]+                  return dblToken(T_NUM_INTEGER);
"="                     return numToken(T_EQUAL);
"=="                    return numToken(T_CMP_EQ);
"!="                    return numToken(T_CMP_NE);
"<"                     return numToken(T_CMP_LT);
"<="                    return numToken(T_CMP_LE);
">"                     return numToken(T_CMP_GT);
">="                    return numToken(T_CMP_GE);
"("                     return numToken(T_LPAREN);
")"                     return numToken(T_RPAREN);
"{"                     return numToken(T_LBRACE);
"}"                     return numToken(T_RBRACE);
";"                     return numToken(T_SEMI);
"+"                     return numToken(T_PLUS);
"-"                     return numToken(T_MINUS);
"/"                     return numToken(T_DIV);
"*"                     return numToken(T_MUL);
","                     return numToken(T_COMMA);
.                       printf("Unkown token!\n"); yyterminate();
%%

int yywrap()
{
    return 1;
}
