#include "octree.h"
#include <stdlib.h>
#include <stdio.h>


/*
  retorna un aabb que contine al brush
*/
struct aabb getaabb(struct brush *bsh)
{
  int i,j;
  struct aabb aux;
  struct vector gv;
  aux.min=bsh->polys[0].vertexes[0];
  aux.max=bsh->polys[0].vertexes[0];

  for(i=0;i<bsh->num;i++){
    for(j=0;j<bsh->polys[i].num;j++){
      gv=bsh->polys[i].vertexes[j];
      if(gv.x>aux.max.x)aux.max.x=gv.x;
      if(gv.y>aux.max.y)aux.max.y=gv.y;
      if(gv.z>aux.max.z)aux.max.z=gv.z;
      if(gv.x<aux.min.x)aux.min.x=gv.x;
      if(gv.y<aux.min.y)aux.min.y=gv.y;
      if(gv.z<aux.min.z)aux.min.z=gv.z;
    }
  }
  return aux;
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

// region R = { (x, y, z) | min.x<=x<=max.x, min.y<=y<=max.y, min.z<=z<=max.z }
int pointinaabb(struct vector v,struct aabb box)
{
  if(v.x<box.min.x||v.x>box.max.x)return 0;
  if(v.y<box.min.y||v.y>box.max.y)return 0;
  if(v.z<box.min.z||v.z>box.max.z)return 0;
  return 1;
}

int brushinaabb(struct brush *bsh,struct aabb box){
  int i,j;
  for(i=0;i<bsh->num;i++){
    for(j=0;j<bsh->polys[i].num;j++){
      if(!pointinaabb(bsh->polys[i].vertexes[j],box)){
	return 0;
      }
    }
  }
  return 1;
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

int checkaabb(struct aabb box)
{
  if(box.min.x>box.max.x)return 0;
  if(box.min.y>box.max.y)return 0;
  if(box.min.z>box.max.z)return 0;
  return 1;
}


struct loctree *_loadloctree(struct map *m,struct aabb box,float size,int depth)
{
  struct loctree *aux;
  struct vector mid;//punto medio de la caja
  struct aabb boxes[8],box2;
  double sizex,sizey,sizez,dx,dy,dz;
  int i;
  if(depth<0){
    return NULL;
  }

  aux=malloc(sizeof(struct loctree));
  //aux->box=box;

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

  for(i=0;i<8;i++){
    if(!checkaabb(boxes[i]))printf("ERROR: los puntos de un aabb estan mal!!\n");
  } 
  
  for(i=0;i<8;i++){
    aux->hijos[i]=_loadloctree(m,boxes[i],size,depth-1);
  }


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

struct vector getpvertex(struct aabb box,struct plane p)
{
  struct vector aux;
  if(p.normal.x>=0 && p.normal.y>=0 && p.normal.z>=0)return box.max;
  if(p.normal.x<=0 && p.normal.y<=0 && p.normal.z<=0)return box.min;
  if(p.normal.x>=0 && p.normal.y>=0 && p.normal.z<=0){
    aux.x=box.max.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
  }
  if(p.normal.x>=0 && p.normal.y<=0 && p.normal.z>=0){
    aux.x=box.max.x;
    aux.y=box.min.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x<=0 && p.normal.y>=0 && p.normal.z>=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x>=0 && p.normal.y<=0 && p.normal.z<=0){
    aux.x=box.max.x;
    aux.y=box.min.y;
    aux.z=box.min.z;
    return aux;
  }
  if(p.normal.x<=0 && p.normal.y<=0 && p.normal.z>=0){
    aux.x=box.min.x;
    aux.y=box.min.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x<=0 && p.normal.y>=0 && p.normal.z<=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
  }
}

struct vector getnvertex(struct aabb box,struct plane p)
{
  struct vector aux;
  if(p.normal.x<=0 && p.normal.y<=0 && p.normal.z<=0)return box.max;
  if(p.normal.x>=0 && p.normal.y>=0 && p.normal.z>=0)return box.min;
  if(p.normal.x<=0 && p.normal.y<=0 && p.normal.z>=0){
    aux.x=box.max.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
  }
  if(p.normal.x<=0 && p.normal.y>=0 && p.normal.z<=0){
    aux.x=box.max.x;
    aux.y=box.min.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x>=0 && p.normal.y<=0 && p.normal.z<=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x<=0 && p.normal.y>=0 && p.normal.z>=0){
    aux.x=box.max.x;
    aux.y=box.min.y;
    aux.z=box.min.z;
    return aux;
  }
  if(p.normal.x>=0 && p.normal.y>=0 && p.normal.z<=0){
    aux.x=box.min.x;
    aux.y=box.min.y;
    aux.z=box.max.z;
    return aux;
  }
  if(p.normal.x>=0 && p.normal.y<=0 && p.normal.z>=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
  }
}

/*
  0 si intersecta
  1 positive half space
  -1 negative half space
*/
int interaabbplane(struct aabb box,struct plane p)
{
  struct vector pv,nv;//p vertex, n vertex *graphics gems 4*
  pv=getpvertex(box,p);
  nv=getnvertex(box,p);
  if(pointinplane(pv,p)==-1)return -1;
  if(pointinplane(nv,p)==1)return 1;
  return 0;
}

/*
  ###TODO###
  0 si intersecta
  1 positive half plane
  -1 negative half plane
*/
int interrectline(struct rect r,struct line l)
{

}

/*
  ###TODO###
*/
struct rect getbbox(struct poly p)
{
  
}
/*
  ###TODO###
*/
int interrectrect(struct rect r1,struct rect r2)
{
  
}

/*
  se asume que estan en el plano x y
  retorna 0 si no hay interseccion, 1 si hay
*/
int interrectpoly(struct rect r,struct poly p)
{
  int i;
  struct rect bb;//bounding box del poly
  bb=getbbox(p);
  if(!interrectrect(r,bb))return 0;
  for(i=0;i<p.num-1;i++){
    //revisar line2points
    if(interrectline(r,line2points(p.vertexes[i],p.vertexes[i+1]))==1)return 0;
  }
  if(interrectline(r,line2points(p.vertexes[p.num-1],p.vertexes[0]))==1)return 0; 
  return 1;
}

/*
  0 no intersecta
  1 intersecta
*/
int interaabbaabb(struct aabb box1,struct aabb box2)
{
  if (box1.max.x < box2.min.x || box1.min.x > box2.max.x) return 0;
  if (box1.max.y < box2.min.y || box1.min.y > box2.max.y) return 0;
  if (box1.max.z < box2.min.z || box1.min.z > box2.max.z) return 0;
  return 1;
}

/*
  ###TODO###
*/
struct poly getsilhouette(struct brush *bsh,int axis)
{
  
}
/*
  ###TODO###
*/
struct rect getrect(struct aabb box,int axis)
{
  
}


/*
  0 si no hay interseccion
*/
int interaabbbrush(struct aabb box,struct brush *bsh)
{
  int i;
  struct plane p;
  struct poly lp[3];
  if(!interaabbaabb(box,getaabb(bsh)))return 0;
  for(i=0;i<bsh->num;i++){
    p=planepnormal(bsh->polys[i].vertexes[0],bsh->polys[i].normal);
    if(interaabbplane(box,p)==1)return 0;
  }
  
  for(i=0;i<3;i++){
    lp[i]=getsilhouette(bsh,i);
  }
  for(i=0;i<3;i++){
    if(interrectpoly(getrect(box,i),lp[i])==0)return 0;
  }
  return 1;
}
