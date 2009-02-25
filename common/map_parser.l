%{
#include "y.tab.h"

extern int lineno;

double addfnumber(char *s){
	double aux;
	sscanf(s,"%lf",&aux);
	return aux;
}
char *addstringl(char *s){
	char *aux;
	aux=malloc(sizeof(char)*(strlen(s)+1));
	strcpy(aux,s+1);
	aux[strlen(s)-2]=0;
	return aux;
}
char *addtexture(char *s){
	char *aux;
	aux=malloc(sizeof(char)*(strlen(s)+1));
	strcpy(aux,s);
	return aux;
}

%}

delim [ \t]
ws {delim}+
letter [A-Za-z]
digit [0-9]
id ({letter}|{digit}|_|\-)*

number \-?{digit}+
fnumber \-?{digit}+\.{digit}+

/* number {digit}+(\.{digit}+)?(E[+-]?{digit}+)? */

%%

{ws} {}
\n   {lineno++;}
\/\/.* {}

\( {return *yytext;}
\) {return *yytext;}
\{ {return *yytext;}
\} {return *yytext;}

{number}   {yylval.fnum = addfnumber(yytext);return FNUMBER;}
{fnumber}   {yylval.fnum=addfnumber(yytext);return FNUMBER;}
\"(\\.|[^\\\"])*\"  {yylval.strl=addstringl(yytext);return STRINGL;}

NULL {yylval.strl=0;return TEXTURE;}

{id} {yylval.strl=addtexture(yytext);/*addstringl(yytext);*/return TEXTURE;}

{id}(\/?{id})* {yylval.strl=addtexture(yytext);/*addstringl(yytext);*/return TEXTURE;}

. {}

%%
