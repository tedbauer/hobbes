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

%%

[a-zA-Z][a-zA-Z0-9_]* { adjust(); yylval.sval = String(yytext); return ID; }
" "                   { adjust(); continue; }
\n                    { adjust(); EM_newline(); continue; }
","                   { adjust(); return COMMA; }
for                   { adjust(); return FOR; }
[0-9]+                { adjust(); yylval.ival = atoi(yytext); return INT; }
.                     { adjust(); EM_error(EM_tokPos, "illegal token"); }
