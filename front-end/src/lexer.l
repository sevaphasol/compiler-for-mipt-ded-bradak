%{
#include <string.h>
#include <stdlib.h>
#include "lang.h"
#include "dsl.h"
#include "parser.hpp"

extern lang_ctx_t* ctx;

static int yycolumn = 1;
#define YY_USER_ACTION yylloc.first_line = yylineno; \
                       yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; \
                       yycolumn += yyleng; \
                       yylloc.last_column = yycolumn;
%}

%option noyywrap nounput noinput yylineno

digit       [0-9]
number      {digit}+
identifier  [a-zA-Z_][a-zA-Z_0-9]*

whitespace  [ \t\r]
newline     [\n]

%x COMMENT

%%

{whitespace}+    { yycolumn += yyleng; }
{newline}+       { yylineno += yyleng; yycolumn = 1; }

"//".* { yycolumn += yyleng; }

"/*"             { BEGIN(COMMENT); yycolumn += yyleng; }
<COMMENT>"*/"    { BEGIN(INITIAL); yycolumn += yyleng; }
<COMMENT>.|\n    { if (*yytext == '\n') { yylineno++; yycolumn = 1; } else yycolumn++; }

"~"              { return TK_TILDE; }
":"              { return TK_COLON; }

"func"           { return TK_FUNC; }
"var"            { return TK_VAR; }
"if"             { return TK_IF; }
"else"           { return TK_ELSE; }
"while"          { return TK_WHILE; }
"return"         { return TK_RETURN; }
"call"           { return TK_CALL; }
"sqrt"           { return TK_SQRT; }
"fdecl"          { return TK_FDECL; }
"="              { return TK_ASSIGN; }
"+"              { return TK_PLUS; }
"-"              { return TK_MINUS; }
"*"              { return TK_STAR; }
"/"              { return TK_SLASH; }

"("              { return TK_L_ROUND; }
")"              { return TK_R_ROUND; }
"{"              { return TK_L_CURLY; }
"}"              { return TK_R_CURLY; }

{number}         { 
    yylval.num = atoi(yytext); 
    return TK_NUMBER; 
}
{identifier}     { 
    yylval.str = strdup(yytext); 
    return TK_IDENTIFIER; 
}

.                { return TK_ERROR; }

%%