%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "map.h"

struct vector start={0,0,0};

struct planelist{
	struct points pts;
	struct texture tex;
	struct planelist *next;
};

int listsize(struct planelist *ls)
{
	if(!ls)return 0;
	return listsize(ls->next)+1;
}

struct planelist *addplane(struct vector *p0,struct vector *p1,struct vector *p2,char *texname,struct planelist *next)
{
	struct planelist *aux;
	aux=malloc(sizeof(struct planelist));
	aux->pts.p[0]=*p0;
	aux->pts.p[1]=*p1;
	aux->pts.p[2]=*p2;
	aux->tex.name=texname;
	aux->next=next;
	return aux;
}

struct headerlist *addheader(char *str1,char *str2,struct headerlist *next)
{
	struct headerlist *aux;
	aux=malloc(sizeof(struct headerlist));
	aux->str1=str1;
	aux->str2=str2;
	aux->next=next;
	return aux;
}

struct brushlist *addbrush(struct planelist *pl,struct brushlist *next)
{
	struct brushlist *aux;
	struct planelist *pa1,*pa2;
	int i;
	int len=listsize(pl);
	//printf("ini addbrush\n");
	aux=malloc(sizeof(struct brushlist));
	aux->rbsh=malloc(sizeof(struct rawbrush));
	aux->rbsh->num=len;
	aux->rbsh->planes=malloc(sizeof(struct points)*len);
	aux->rbsh->textures=malloc(sizeof(char *)*len);
	pa1=pl;
	for(i=0;i<len;i++){
		aux->rbsh->planes[i]=pa1->pts;
		aux->rbsh->textures[i]=pa1->tex;
		pa2=pa1->next;
		free(pa1);
		pa1=pa2;
	}
	//printf("1\n");
	aux->bsh=loadbrush(aux->rbsh);
	aux->used=0;
	//printf("2\n");
	aux->next=next;
	//printf("Fin addbrush\n");
	return aux;
}

struct entity *addentity(struct headerlist *hl,struct brushlist *bl)
{
	struct entity *aux;
	struct headerlist *ph;
	int ban=0;
	struct vector gp={0,0,0};
	ph=hl;
	while(ph){
		if(!strcmp(ph->str1,"classname") && !strcmp(ph->str2,"info_player_start")){
			ban=1;
		}
		if(!strcmp(ph->str1,"origin")){
			sscanf(ph->str2,"%lf %lf %lf",&gp.x,&gp.y,&gp.z);
		}
		ph=ph->next;
	}
	if(ban){
		start=gp;
	}
	aux=malloc(sizeof(struct entity));
	aux->header=hl;
	aux->brushes=bl;
	return aux;
}


struct map *currentmap;

int lineno=1;
int cerrores=0;

struct listaerr{
	char *str;
	int line;
	struct listaerr *next;
};
struct listaerr *errores=0;

int imprimirerrores(struct listaerr *n)
{
	if(!n)return 0;
	imprimirerrores(n->next);
	printf("line %d: error: %s\n",n->line,n->str);
	return 0;
}

void yyerror(const char *str)
{
	struct listaerr *aux;
	aux=malloc(sizeof(struct listaerr));
	aux->str=malloc(sizeof(char)*(strlen(str)+1));
	strcpy(aux->str,str);
	aux->line=lineno;
	aux->next=errores;
	errores=aux;
	cerrores++;
}


%}
%error-verbose

%union{
	int num;
	double fnum;
	char *strl;
	struct brushlist *bl;
	struct planelist *pl;
	struct headerlist *hl;
	struct entitylist *el;
	struct vector *pt;
	struct entity *ent;
	struct map *m;
}

%token <num> NUMBER
%token <num> TNULL  
%token <fnum> FNUMBER
%token <strl> STRINGL TEXTURE

%type <m> map
%type <el> entlist
%type <bl> brushes 
%type <pl>vertexes
%type <pt> point
%type <num> numlist 
%type <hl> header
%type <ent> entity

%%

map: entlist {currentmap=malloc(sizeof(struct map));currentmap->start=start;currentmap->entities=$1;}
   ;

entlist: entity entlist {$$=malloc(sizeof(struct map));$$->ent=*($1);$$->next=$2;}
   |                    {$$=0;}
   ;

entity: '{' header brushes '}' {$$=addentity($2,$3);}
      ;

header: STRINGL STRINGL header {$$=addheader($1,$2,$3);}
      |                        {$$=0;}
      ;

brushes: '{' vertexes '}' brushes {$$=addbrush($2,$4);}
       |                          {$$=0;}
       ;

vertexes: point point point TEXTURE numlist vertexes {$$=addplane($1,$2,$3,$4,$6);}
        |                                            {$$=0;}
	;

point: '(' FNUMBER FNUMBER FNUMBER ')' {$$=malloc(sizeof(struct vector));$$->x=$2;$$->y=$3;$$->z=$4;}
     ;

numlist: NUMBER numlist {$$=0;}
       | FNUMBER numlist {$$=0;}
       |                 {$$=0;}
       ;

%%

int yywrap()
{
	return 1;
} 

int parsemap(char *file)
{	
	int fd_in,gfd;
	fd_in=open(file, O_RDONLY);
	dup2( STDIN_FILENO , gfd);
	dup2( fd_in, STDIN_FILENO );

	yyparse();

	dup2( gfd, STDIN_FILENO );
	close(fd_in);

	if(!cerrores){
		return 1;
	}else{
		printf("%d errores\n",cerrores);
		imprimirerrores(errores);
		return 0;
	}

}

/*int main(int argc,char **argv) {
	int fd_in,fd_out;

	yyparse();

	if(!cerrores){
	}else{
		printf("%d errores\n",cerrores);
		imprimirerrores(errores);
	}
	return 0;
}
*/
