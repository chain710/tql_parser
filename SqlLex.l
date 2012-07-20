%{
#include "tql_parser.h"
#include "SqlParser.h"
/*extra数据类型 %option extra-type 命令用不了*/
/*#define YY_EXTRA_TYPE int*/
%}

%option noyywrap
/*可重入的*/
%option reentrant

DIGIT   [0-9]
ID      [a-zA-Z][a-zA-Z0-9_]*
ARRAY_E {ID}\[{DIGIT}+\]

%%
{DIGIT}+    {
                return TK_INTEGER;
            }
SELECT      {
                return TK_SELECT;
            }

INSERT      {
                return TK_INSERT;
            }

DELETE      {
                return TK_DELETE;
            }

UPDATE      {
                return TK_UPDATE;
            }

SET         {
                return TK_SET;
            }

WHERE       {
                return TK_WHERE;
            }

AND         {
                return TK_AND;
            }

OR          {
                return TK_OR;
            }

FROM        {
                return TK_FROM;
            }
KEY         {
                return TK_KEY;
            }
(((({ARRAY_E}|{ID})\.)+({ARRAY_E}|{ID}))|{ARRAY_E}) {
                return TK_FIELD;
            }
{ID}        {
                return TK_ID;
            }
\+          {
                return TK_PLUS;
            }
-           {
                return TK_MINUS;
            }
\*          {
                return TK_MULTI;
            }
\/          {
                return TK_DIV;
            }
\(          {
                return TK_LB;
            }

\)          {
                return TK_RB;
            }

\>          {
                return TK_GT;
            }

\<          {
                return TK_LT;
            }

\>=         {
                return TK_GE;
            }

\<=         {
                return TK_LE;
            }
,           {
                return TK_COMMA;
            }
\=          {
                return TK_EQ;
            }

\n          {
                /* ignore end of line */;
            }

[ \t]+      {
                /* ignore blank */;
            }

.           {
                return -1;
            }
%%

#ifndef _USER_DEFINE_YYERROR_

void yyerror(const char* str)
{
    fprintf(stderr,"yyerror: %s\n",str);
}

#endif
