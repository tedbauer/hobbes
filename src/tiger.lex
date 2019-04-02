%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

string strAcc;

int yywrap()
{
	charPos = 1;
	return 1;
}

void adjust()
{
	EM_tokPos = charPos;
	charPos += yyleng;
}

// TODO: handle escape sequences in strings
//<STRING>\\                     { adjust(); BEGIN ESCAPE_SEQ; }
%}

%Start NORMAL COMMENT STRING ESCAPE_SEQ

%%

<NORMAL>\/\*                   { adjust(); BEGIN COMMENT; }
<COMMENT>.                     { adjust(); }
<COMMENT>\*\/                  { adjust(); BEGIN NORMAL; }

<NORMAL>\"                     { adjust(); strAcc = String(""); BEGIN STRING; }
<STRING>[^\"\\]                { adjust(); strAcc = StringAppend(strAcc, yytext); }
<STRING>\"                     { adjust(); yylval.sval = String(strAcc); BEGIN NORMAL; }

<NORMAL>"+"                    { adjust(); return PLUS; }
<NORMAL>"-"                    { adjust(); return MINUS; }
<NORMAL>"*"                    { adjust(); return TIMES; }
<NORMAL>"/"                    { adjust(); return DIVIDE; }

<NORMAL>":"                    { adjust(); return COLON; }
<NORMAL>";"                    { adjust(); return SEMICOLON; }

<NORMAL>"("                    { adjust(); return LPAREN; }
<NORMAL>")"                    { adjust(); return RPAREN; }
<NORMAL>"["                    { adjust(); return LBRACK; }
<NORMAL>"]"                    { adjust(); return RBRACK; }
<NORMAL>"{"                    { adjust(); return LBRACE; }
<NORMAL>"}"                    { adjust(); return RBRACE; }

<NORMAL>"&"                    { adjust(); return AND; }
<NORMAL>"|"                    { adjust(); return OR; }

<NORMAL>"="                    { adjust(); return EQ; }
<NORMAL>"<>"                   { adjust(); return NEQ; }
<NORMAL>">"                    { adjust(); return GT; }
<NORMAL>"<"                    { adjust(); return LT; }
<NORMAL>">="                   { adjust(); return GE; }
<NORMAL>"<="                   { adjust(); return LE; }

<NORMAL>"."                    { adjust(); return DOT; }
<NORMAL>","                    { adjust(); return COMMA; }

<NORMAL>":="                   { adjust(); return ASSIGN; }

<NORMAL>array                  { adjust(); return ARRAY; }
<NORMAL>for                    { adjust(); return FOR; }
<NORMAL>while                  { adjust(); return WHILE; }
<NORMAL>if                     { adjust(); return IF; }
<NORMAL>else                   { adjust(); return ELSE; }
<NORMAL>then                   { adjust(); return THEN; }
<NORMAL>to                     { adjust(); return TO; }
<NORMAL>do                     { adjust(); return DO; }
<NORMAL>let                    { adjust(); return LET; }
<NORMAL>in                     { adjust(); return IN; }
<NORMAL>end                    { adjust(); return END; }
<NORMAL>of                     { adjust(); return OF; }
<NORMAL>break                  { adjust(); return BREAK; }
<NORMAL>nil                    { adjust(); return NIL; }
<NORMAL>function               { adjust(); return FUNCTION; }
<NORMAL>type                   { adjust(); return TYPE; }
<NORMAL>var                    { adjust(); return VAR; }

<NORMAL>[a-zA-Z][a-zA-Z0-9_]*  { adjust(); yylval.sval = String(yytext); return ID; }
<NORMAL>[0-9]+                 { adjust(); yylval.ival = atoi(yytext); return INT; }

<NORMAL>[ \t]                  { adjust(); continue; }
<NORMAL>\r\n                     { adjust(); EM_newline(); continue; }
<NORMAL>.                      { adjust(); EM_error(EM_tokPos, "illegal token"); }

.                              { BEGIN NORMAL; yyless(0); }

