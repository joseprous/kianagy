#include "octree.h"
#include "drawmap.h"
#include <stdlib.h>
#include <stdio.h>


void printaabb(struct aabb box)
{
  printf("min:");
  printvector(box.min);
  printf("max");
  printvector(box.max);
  printf("\n");
}

void printpoly(struct poly p)
{
  int i;
  printf("normal:");
  printvector(p.normal);
  printf("\nvert:");
  for(i=0;i<p.num;i++){
    printvector(p.vertexes[i]);
  }
  printf("\n");
}


void printbrush(struct brush *bsh)
{
  int i;
  printf("BRUSH:\n");
  for(i=0;i<bsh->num;i++){
    printpoly(bsh->polys[i]);
  }  
  printf("\n");
}

/*
  retorna un aabb que contiene a todo el mapa
*/
struct aabb getmaxbox(struct map *m)
{
  struct aabb aux;
  struct entitylist *pe;
  struct brushlist *pb;
  int i,j;
  struct vector gv;
  aux.min.x=0;
  aux.min.y=0;
  aux.min.z=0;
  aux.max.x=0;
  aux.max.y=0;
  aux.max.z=0;
  pe=m->entities;
  while(pe){
    pb=pe->ent.brushes;
    while(pb){
      for(i=0;i<pb->bsh->num;i++){
	for(j=0;j<pb->bsh->polys[i].num;j++){
	  gv=pb->bsh->polys[i].vertexes[j];
	  if(gv.x>aux.max.x)aux.max.x=gv.x;
	  if(gv.y>aux.max.y)aux.max.y=gv.y;
	  if(gv.z>aux.max.z)aux.max.z=gv.z;
	}
      }
      pb=pb->next;
    }
    pe=pe->next;
  }
  return aux;
}


/*
  retorna la lista de brushes que entran en box
*/
struct brushlist *getbrushes(struct map *m,struct aabb box)
{
  struct entitylist *pe;
  struct brushlist *pb,*bl=NULL,*aux=NULL;

  pe=m->entities;
  while(pe){
    pb=pe->ent.brushes;
    while(pb){
      if(!pb->used){
	if(brushinaabb(pb->bsh,box)){
	  aux=malloc(sizeof(struct brushlist));
	  aux->next=bl;
	  aux->bsh=pb->bsh;
	  aux->rbsh=pb->rbsh;
	  bl=aux;
	  pb->used=1;
	}
      }
      pb=pb->next;
    }
    pe=pe->next;
  }
  return bl;  
}

/*
  retorna la lista de brushes que tocan al box
*/
struct brushlist *getbrushespartial(struct map *m,struct aabb box)
{
  struct entitylist *pe;
  struct brushlist *pb,*bl=NULL,*aux=NULL;
  struct aabb bb;
  struct poly silh[3];
  int i;

  pe=m->entities;
  while(pe){
    pb=pe->ent.brushes;
    while(pb){
      bb=getaabb(pb->bsh);
      for(i=0;i<3;i++){
	silh[i]=getsilhouette(pb->bsh,i);
      }
      if(interaabbbrush(box,pb->bsh,bb,silh)){
	aux=malloc(sizeof(struct brushlist));
	aux->next=bl;
	aux->bsh=pb->bsh;
	aux->rbsh=pb->rbsh;
	bl=aux;
	pb->used=1;
      }
      pb=pb->next;
    }
    pe=pe->next;
  }
  return bl;  
}


void getboxes(struct aabb *boxes,struct aabb box)
{
  int i;
  struct vector mid;//punto medio de la caja
  mid.x=(box.min.x+box.max.x)/2;
  mid.y=(box.min.y+box.max.y)/2;
  mid.z=(box.min.z+box.max.z)/2;

  boxes[0].min.x=box.min.x;
  boxes[0].min.y=box.min.y;
  boxes[0].min.z=box.min.z;
  boxes[0].max.x=mid.x;
  boxes[0].max.y=mid.y;
  boxes[0].max.z=mid.z;

  boxes[1].min.x=mid.x;
  boxes[1].min.y=box.min.y;
  boxes[1].min.z=box.min.z;
  boxes[1].max.x=box.max.x;
  boxes[1].max.y=mid.y;
  boxes[1].max.z=mid.z;

  boxes[2].min.x=mid.x;
  boxes[2].min.y=box.min.y;
  boxes[2].min.z=mid.z;
  boxes[2].max.x=box.max.x;
  boxes[2].max.y=mid.y;
  boxes[2].max.z=box.max.z;

  boxes[3].min.x=box.min.x;
  boxes[3].min.y=box.min.y;
  boxes[3].min.z=mid.z;
  boxes[3].max.x=mid.x;
  boxes[3].max.y=mid.y;
  boxes[3].max.z=box.max.z;    

  for(i=0;i<4;i++){
    boxes[i+4].min.x=boxes[i].min.x;
    boxes[i+4].min.y=boxes[i].min.y+(mid.y-box.min.y);
    boxes[i+4].min.z=boxes[i].min.z;
    boxes[i+4].max.x=boxes[i].max.x;
    boxes[i+4].max.y=boxes[i].max.y+(mid.y-box.min.y);
    boxes[i+4].max.z=boxes[i].max.z;
  }  
  /*  for(i=0;i<8;i++){
    if(!checkaabb(boxes[i]))printf("ERROR: los puntos de un aabb estan mal!!\n");
    } */

}


struct loctree *_loadloctree(struct map *m,struct aabb box,float size,int depth)
{
  struct loctree *aux;
  struct aabb boxes[8];
  double sizex,sizey,sizez,dx,dy,dz;
  int i;
  if(depth<0){
    return NULL;
  }

  aux=malloc(sizeof(struct loctree));
  //aux->box=box;
  
  aux->gllistf=0;
  aux->gllistw=0;

  getboxes(boxes,box);
  
  for(i=0;i<8;i++){
    aux->hijos[i]=_loadloctree(m,boxes[i],size,depth-1);
  }
  
  /*ajusta el tamanho de la caja de acuerdo a size*/
  sizex=box.max.x-box.min.x;
  sizey=box.max.y-box.min.y;
  sizez=box.max.z-box.min.z;
  dx=sizex*size;
  dy=sizey*size;
  dz=sizez*size;
  aux->box.min.x=box.min.x-dx;
  aux->box.min.y=box.min.y-dy;
  aux->box.min.z=box.min.z-dz;
  aux->box.max.x=box.max.x+dx;
  aux->box.max.y=box.max.y+dy;
  aux->box.max.z=box.max.z+dz;    
  
  aux->brushes=getbrushes(m,aux->box);
  
  /*si no hay brushes en el nodo actual
    ni en los hijos liberar memoria y retornar NULL
   */
  if(aux->brushes==NULL){
    int b=0;
    for(i=0;i<8;i++){
      if(aux->hijos[i])b=1;
    }
    if(!b){
      free(aux);
      return NULL;
    }
  }

  return aux;
}

/*
  carga con loose octree con areas size veces mas grandes
  maxdepth maxima profundidad del arbol
*/
struct loctree *loadloctree(struct map *m,float size,int maxdepth)
{
  return _loadloctree(m,getmaxbox(m),size,maxdepth);
}


struct roctree *_loadroctree(struct map *m,struct aabb box,int depth)
{
  struct roctree *aux;
  struct vector mid;//punto medio de la caja
  struct aabb boxes[8];
  double sizex,sizey,sizez,dx,dy,dz;
  int i;
  int b=0;

  aux=malloc(sizeof(struct roctree));
  aux->box=box;
  
  if(depth==0){
    aux->brushes=getbrushespartial(m,aux->box);
    if(aux->brushes==NULL){
      free(aux);
      return NULL;      
    }
    for(i=0;i<8;i++){
      aux->hijos[i]=NULL;
    }    
    return aux;
  }
  aux->brushes=NULL;

  getboxes(boxes,box);
  
  for(i=0;i<8;i++){
    if(aux->hijos[i]=_loadroctree(m,boxes[i],depth-1)){
      b=1;
    }
  }

  if(!b){
    free(aux);
    return NULL;
  }

  return aux;
}



struct roctree *loadroctree(struct map *m,int maxdepth)
{
  return _loadroctree(m,getmaxbox(m),maxdepth);
}

void _loctreestats(struct loctree *m, int level)
{
  int i,c;
  struct brushlist *bl;
  if(m==NULL)return;
  c=0;
  bl=m->brushes;
  while(bl){
    c++;
    bl=bl->next;
  }
  for(i=0;i<level;i++)printf("-");
  printf("num brushes:%d\n",c);
  for(i=0;i<8;i++){
    _loctreestats(m->hijos[i],level+1);
  }
}

/*
  imprime un arbol con la cantidad de brushes por nodo
*/
void loctreestats(struct loctree *m)
{
  _loctreestats(m,0);
}

void _roctreestats(struct roctree *m, int level)
{
  int i,c;
  struct brushlist *bl;
  if(m==NULL)return;
  c=0;
  bl=m->brushes;
  while(bl){
    c++;
    bl=bl->next;
  }
  for(i=0;i<level;i++)printf("-");
  printf("num brushes:%d\n",c);
  for(i=0;i<8;i++){
    _roctreestats(m->hijos[i],level+1);
  }
}

/*
  imprime un arbol con la cantidad de brushes por nodo
*/
void roctreestats(struct roctree *m)
{
  _roctreestats(m,0);
}





/*
  genera listas de opengl para los bruhes de los nodos
  hasta mientras la profundidad sea menor a maxdepth
*/
void loctreegenlists(struct loctree *m,int maxdepth)
{
  struct brushlist *bl;
  int i;
  if(maxdepth<=0)return;
  m->gllistf=glGenLists(1);
  glNewList(m->gllistf, GL_COMPILE);
     bl=m->brushes;
     while(bl){
       drawbrush(bl->bsh,FLAT);
       bl=bl->next;
     }
  glEndList();
  m->gllistw=glGenLists(1);
  glNewList(m->gllistw, GL_COMPILE);
     bl=m->brushes;
     while(bl){
       drawbrush(bl->bsh,WIREFRAME);
       bl=bl->next;
     }
  glEndList();
  for(i=0;i<8;i++){
    loctreegenlists(m->hijos[i],maxdepth-1);
  }  
}
