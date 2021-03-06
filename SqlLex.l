%{
#include "tql_parser.h"
#include "SqlParser.h"
/*extra数据类型 %option extra-type 命令用不了*/
/*#define YY_EXTRA_TYPE int*/
%}

%option noyywrap
/*可重入的*/
%option reentrant
%x STRING

DIGIT   [0-9]
ID      [a-zA-Z0-9_]+
/*ID      [a-zA-Z][a-zA-Z0-9_]* */

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
REPLACE     {
                return TK_REPLACE;
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
\|          {
                return TK_BIN_OR;
            }
&           {
                return TK_BIN_AND;
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
!=          {
                return TK_UNEQ;
            }
\"          {
                BEGIN(STRING);
                return TK_QUOTE;
            }
<STRING>[^"]+ {
                return TK_STRING;
            }
<STRING>\"  {
                BEGIN(INITIAL);
                return TK_QUOTE;
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
