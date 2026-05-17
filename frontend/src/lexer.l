%{
#include <string.h>
#include <stdlib.h>
#include "lang.h"
#include "dsl.h"
#include "parser.hpp"

extern lang_ctx_t* ctx;

static int yycolumn = 1;
static int prev_yylineno = 1;

#define YY_USER_ACTION \
    yylloc.first_line = prev_yylineno; \
    yylloc.first_column = yycolumn; \
    for (int i = 0; yytext[i] != '\0'; i++) { \
        if (yytext[i] == '\n') { \
            yycolumn = 1; \
        } else { \
            yycolumn++; \
        } \
    } \
    yylloc.last_line = yylineno; \
    yylloc.last_column = yycolumn; \
    prev_yylineno = yylineno;
%}

%option noyywrap nounput noinput yylineno

digit       [0-9]
number      {digit}+
identifier  [a-zA-Z_][a-zA-Z_0-9]*
string      \"[^\"\n]*\"

whitespace  [ \t\r]
newline     [\n]

%x COMMENT

%%

{whitespace}+    { }
{newline}+       { }

"//".*           { }

"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>.       { }
<COMMENT>\n      { }

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
{string}         {
    size_t len = strlen(yytext);
    yylval.str = (char*) calloc(len - 1, sizeof(char));
    memcpy(yylval.str, yytext + 1, len - 2);
    yylval.str[len - 2] = '\0';
    return TK_STRING;
}
{identifier}     { 
    yylval.str = strdup(yytext); 
    return TK_IDENTIFIER; 
}

.                { return TK_ERROR; }

%%