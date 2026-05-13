%{
#include <stdio.h>
#include <string.h>
#include "lang.h"
#include "dsl.h"
#include "parser.hpp"

extern lang_ctx_t* ctx;

/* Flex debug flag – set to 1 to see scanner trace */
extern int yy_flex_debug;

/* Column counter */
static int yycolumn = 1;
#define YY_USER_ACTION yylloc.first_line = yylineno; \
                       yylloc.last_line = yylineno; \
                       yylloc.first_column = yycolumn; \
                       yycolumn += yyleng; \
                       yylloc.last_column = yycolumn;

static void print_token(const char* name, int token, int line, int col) {
    fprintf(stderr, "[LEX] line %d, col %d: token %d (%s)\n", line, col, token, name);
}
%}

%option noyywrap nounput noinput yylineno
%option debug

digit       [0-9]
number      {digit}+
identifier  [a-zA-Z_][a-zA-Z_0-9]*

whitespace  [ \t\r]
newline     [\n]

%x COMMENT

%%

{whitespace}+    { yycolumn += yyleng; }
{newline}+       { yylineno += yyleng; yycolumn = 1; }

"//".*           { yycolumn += yyleng; }

"/*"             { BEGIN(COMMENT); yycolumn += yyleng; }
<COMMENT>"*/"    { BEGIN(INITIAL); yycolumn += yyleng; }
<COMMENT>.|\n    { if (*yytext == '\n') { yylineno++; yycolumn = 1; } else yycolumn++; }

"~"              { print_token("TK_TILDE", TK_TILDE, yylineno, yycolumn - yyleng); return TK_TILDE; }
":"              { print_token("TK_COLON", TK_COLON, yylineno, yycolumn - yyleng); return TK_COLON; }

"func"           { print_token("TK_FUNC", TK_FUNC, yylineno, yycolumn - yyleng); return TK_FUNC; }
"var"            { print_token("TK_VAR", TK_VAR, yylineno, yycolumn - yyleng); return TK_VAR; }
"if"             { print_token("TK_IF", TK_IF, yylineno, yycolumn - yyleng); return TK_IF; }
"else"           { print_token("TK_ELSE", TK_ELSE, yylineno, yycolumn - yyleng); return TK_ELSE; }
"while"          { print_token("TK_WHILE", TK_WHILE, yylineno, yycolumn - yyleng); return TK_WHILE; }
"return"         { print_token("TK_RETURN", TK_RETURN, yylineno, yycolumn - yyleng); return TK_RETURN; }
"print"          { print_token("TK_PRINT", TK_PRINT, yylineno, yycolumn - yyleng); return TK_PRINT; }
"scan"           { print_token("TK_SCAN", TK_SCAN, yylineno, yycolumn - yyleng); return TK_SCAN; }
"call"           { print_token("TK_CALL", TK_CALL, yylineno, yycolumn - yyleng); return TK_CALL; }
"sqrt"           { print_token("TK_SQRT", TK_SQRT, yylineno, yycolumn - yyleng); return TK_SQRT; }

"="              { print_token("TK_ASSIGN", TK_ASSIGN, yylineno, yycolumn - yyleng); return TK_ASSIGN; }
"+"              { print_token("TK_PLUS", TK_PLUS, yylineno, yycolumn - yyleng); return TK_PLUS; }
"-"              { print_token("TK_MINUS", TK_MINUS, yylineno, yycolumn - yyleng); return TK_MINUS; }
"*"              { print_token("TK_STAR", TK_STAR, yylineno, yycolumn - yyleng); return TK_STAR; }
"/"              { print_token("TK_SLASH", TK_SLASH, yylineno, yycolumn - yyleng); return TK_SLASH; }

"("              { print_token("TK_L_ROUND", TK_L_ROUND, yylineno, yycolumn - yyleng); return TK_L_ROUND; }
")"              { print_token("TK_R_ROUND", TK_R_ROUND, yylineno, yycolumn - yyleng); return TK_R_ROUND; }
"{"              { print_token("TK_L_CURLY", TK_L_CURLY, yylineno, yycolumn - yyleng); return TK_L_CURLY; }
"}"              { print_token("TK_R_CURLY", TK_R_CURLY, yylineno, yycolumn - yyleng); return TK_R_CURLY; }

{number}         { 
    print_token("TK_NUMBER", TK_NUMBER, yylineno, yycolumn - yyleng);
    yylval.num = atoi(yytext); 
    return TK_NUMBER; 
}
{identifier}     { 
    print_token("TK_IDENTIFIER", TK_IDENTIFIER, yylineno, yycolumn - yyleng);
    yylval.str = strdup(yytext); 
    return TK_IDENTIFIER; 
}

.                { 
    fprintf(stderr, "line %d, column %d: unknown character '%c'\n", 
            yylineno, yycolumn, yytext[0]); 
    return TK_ERROR; 
}

%%