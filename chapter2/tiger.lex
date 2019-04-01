%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

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

%}

%Start INITIAL COMMENT

%%

<INITIAL>if                    { adjust(); return IF; }
<INITIAL>[a-zA-Z][a-zA-Z0-9_]* { adjust(); yylval.sval = String(yytext); return ID; }
<INITIAL>"(*"                  { adjust(); BEGIN COMMENT; }
<INITIAL>.                     { adjust(); EM_error("illegal character"); }
<COMMENT>"*)"                  { adjust(); BEGIN COMMENT; }
<COMMENT>.                     { adjust(); }
.                              { BEGIN INITIAL; yyless(1); }
" "                            { adjust(); continue; }
\n                             { adjust(); EM_newline(); continue; }
","                            { adjust(); return COMMA; }
for                            { adjust(); return FOR; }
[0-9]+                         { adjust(); yylval.ival = atoi(yytext); return INT; }
.                              { adjust(); EM_error(EM_tokPos, "illegal token"); }
