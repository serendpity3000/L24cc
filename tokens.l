%{
#include <string>
#include "node.h"
#include "parser.hpp"

#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
%}

%option noyywrap

%%

[ \t\n]					        ;
"main"                          return TOKEN(TMAIN);
"extern"                        return TOKEN(TEXTERN);
"return"				        return TOKEN(TRETURN);
"if"                            return TOKEN(TIF);
"then"                          return TOKEN(TTHEN);
"else"                          return TOKEN(TELSE);
"end"                           return TOKEN(TEND);    
"while"                         return TOKEN(TWHILE);
"for"                           return TOKEN(TFOR);
"scan"                          return TOKEN(TSCAN);
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\.[0-9]* 			    SAVE_TOKEN; return TDOUBLE;
[0-9]+					        SAVE_TOKEN; return TINTEGER;

"++"                            return TOKEN(TINCRE);
"--"                            return TOKEN(TDECRE);
"="						          return TOKEN(TEQUAL);
"=="				          	return TOKEN(TCEQ);
"!="			          		return TOKEN(TCNE);
"<"				          		return TOKEN(TCLT);
"<="	          				return TOKEN(TCLE);
">"				          		return TOKEN(TCGT);
">="					          return TOKEN(TCGE);

"("	          					return TOKEN(TLPAREN);
")"					          	return TOKEN(TRPAREN);
"{"         						return TOKEN(TLBRACE);
"}"					          	return TOKEN(TRBRACE);

"."         						return TOKEN(TDOT);
","				          		return TOKEN(TCOMMA);
";"                             return TOKEN(TSEMIC);        

"+"				          		return TOKEN(TPLUS);
"-"		          				return TOKEN(TMINUS);
"*"		          				return TOKEN(TMUL);
"/"				          		return TOKEN(TDIV);

"||"                            return TOKEN(TOR);
"&&"                            return TOKEN(TAND);

.                       printf("Unknown token!\n"); yyterminate();

%%
