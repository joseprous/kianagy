#include "octree.h"
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
  int i,j;
  printf("BRUSH:\n");
  for(i=0;i<bsh->num;i++){
    printpoly(bsh->polys[i]);
  }  
  printf("\n");
}

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
/*
  retorna el p vertex descripto en *graphics gems 4*
*/
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

struct vector2d getpvertex2d(struct rect r,struct line2d l)
{
  struct vector2d aux;
  if(l.normal.x>=0 && l.normal.y>=0)return r.max;
  if(l.normal.x<=0 && l.normal.y<=0)return r.min;
  if(l.normal.x>=0 && l.normal.y<=0){
    aux.x=r.max.x;
    aux.y=r.min.y;
    return aux;
  }
  if(l.normal.x<=0 && l.normal.y>=0){
    aux.x=r.min.x;
    aux.y=r.max.y;
    return aux;
  }
}

struct vector2d getnvertex2d(struct rect r,struct line2d l)
{
  struct vector2d aux;
  if(l.normal.x<=0 && l.normal.y<=0)return r.max;
  if(l.normal.x>=0 && l.normal.y>=0)return r.min;
  if(l.normal.x<=0 && l.normal.y>=0){
    aux.x=r.max.x;
    aux.y=r.min.y;
    return aux;
  }
  if(l.normal.x>=0 && l.normal.y<=0){
    aux.x=r.min.x;
    aux.y=r.max.y;
    return aux;
  }
}

int pointinline2d(struct vector2d point,struct line2d l)
{
  double res;
  res=l.a * point.x + l.b * point.y + l.c;
  if(res<0.01 && res>0.01){
    return 0;	
  }else{
    if(res<0){		
      return -1;
    }else{
      return 1;			
    }
  }
}


/*
  0 si intersecta
  1 positive half plane
  -1 negative half plane
*/
int interrectline(struct rect r,struct line2d l)
{
  struct vector2d pv,nv;
  pv=getpvertex2d(r,l);
  nv=getnvertex2d(r,l);
  //  printf("pvert:<%lf %lf>\n",pv.x,pv.y);
  //printf("nvert:<%lf %lf>\n",nv.x,nv.y);

  if(pointinline2d(pv,l)==-1)return -1;
  if(pointinline2d(nv,l)==1)return 1;
  return 0;  
}

/*
  el poligono debe estar en el plano xy
  se asume que el poligono tiene al menos un vertice
*/
struct rect getbbox(struct poly p)
{
  struct rect aux;
  int i;
  aux.min.x=p.vertexes[0].x;
  aux.min.y=p.vertexes[0].y;
  aux.max.x=p.vertexes[0].x;
  aux.max.y=p.vertexes[0].y;
  for(i=1;i<p.num;i++){
    if(p.vertexes[i].x<aux.min.x)aux.min.x=p.vertexes[i].x;
    if(p.vertexes[i].y<aux.min.y)aux.min.y=p.vertexes[i].y;
    if(p.vertexes[i].x>aux.max.x)aux.max.x=p.vertexes[i].x;
    if(p.vertexes[i].y>aux.max.y)aux.max.y=p.vertexes[i].y;
  }
  return aux;
}
/*
  retorna 0 si no hay interseccion, 1 si hay
*/
int interrectrect(struct rect r1,struct rect r2)
{
  if (r1.max.x < r2.min.x || r1.min.x > r2.max.x) return 0;
  if (r1.max.y < r2.min.y || r1.min.y > r2.max.y) return 0;
  return 1;  
}

/*
  si no anda invertir la normal
*/
struct line2d line2points2d(struct vector p1,struct vector p2)
{
  struct line2d aux;
  struct vector n1,n2;
  n1.x=0;
  n1.y=0;
  n1.z=-1;

  if(p1.x==p2.x){
    aux.normal.x=1;
    aux.normal.y=0;
    aux.normal.z=0;
    n2=cross(difvectors(p2,p1),n1);
    if(((aux.normal.x<-0.001||aux.normal.x>0.001) && (n2.x<-0.001||n2.x>0.001) 
	&& signo(aux.normal.x,n2.x))||signo(aux.normal.y,n2.y)){
      aux.a=1;
      aux.b=0;
      aux.c=-p1.x;
    }else{
      aux.a=-1;
      aux.b=0;
      aux.c=p1.x;
    }    
  }else{
    aux.normal.x=((p2.y-p1.y)/(p2.x-p1.x));
    aux.normal.y=(-1);
    aux.normal.z=0;
    n2=cross(difvectors(p2,p1),n1);
    if(((aux.normal.x<-0.001||aux.normal.x>0.001) && (n2.x<-0.001||n2.x>0.001) 
	&& signo(aux.normal.x,n2.x))||signo(aux.normal.y,n2.y)){
      aux.a=aux.normal.x;
      aux.b=aux.normal.y;
      aux.c=p1.y-aux.a*p1.x;
    }else{
      aux.a=aux.normal.x*(-1);
      aux.b=aux.normal.y*(-1);
      aux.c=(p1.y-aux.a*p1.x)*(-1);    
    }
  }
  
  /*  aux.a=((p2.y-p1.y)/(p2.x-p1.x))*(-1);
  aux.b=(-1)*(-1);
  aux.c=(p1.y-aux.a*p1.x)*(-1);
  aux.normal.x=aux.a*(-1);
  aux.normal.y=aux.b*(-1);*/
  return aux;
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
  //printf("entro\n");
  if(!interrectrect(r,bb))return 0;
  //printf("1\n");
  //printf("rect: <%lf %lf> <%lf %lf>\n",r.min.x,r.min.y,r.max.x,r.max.y);
  //printpoly(p);
  for(i=0;i<p.num-1;i++){
    if(interrectline(r,line2points2d(p.vertexes[i],p.vertexes[i+1]))==1){
      //printf("\n");
      //printvector(p.vertexes[i]);
      //printf("\n");
      //printvector(p.vertexes[i+1]);
      //exit(0);
      return 0;
    }
  }
  //printf("2\n");
  if(interrectline(r,line2points2d(p.vertexes[p.num-1],p.vertexes[0]))==1)return 0;
  //printf("#######\n");
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

int compvectors(struct vector v1,struct vector v2,double prec)
{
  //  prec=0.005;
	if((v1.x > v2.x-prec && v1.x < v2.x+prec)&&
	   (v1.y > v2.y-prec && v1.y < v2.y+prec)&&
	   (v1.z > v2.z-prec && v1.z < v2.z+prec))
		return 1;
	return 0;

	//  return (v1.x==v2.x && v1.y==v2.y && v1.z==v2.z);
}

/*
  retorna 1 si los poligonos son adyacentes
  y devuelve los valores de los vertices de la arista en comun en v1 y v2
  retorna 0 sino
*/
int polysadj(struct poly p1,struct poly p2,struct vector *v1,struct vector *v2)
{
  int i,j;
  for(i=0;i<p1.num;i++){
    for(j=0;j<p2.num;j++){
      if(compvectors(p1.vertexes[i],p2.vertexes[j],2)){
	if(i+1<p1.num){
	  if(j+1<p2.num){
	    if(compvectors(p1.vertexes[i+1],p2.vertexes[j+1],2)){
	      *v1=p1.vertexes[i];
	      *v2=p1.vertexes[i+1];
	      return 1;
	    }
	  }else{
	    if(compvectors(p1.vertexes[i+1],p2.vertexes[0],2)){
	      *v1=p1.vertexes[i];
	      *v2=p1.vertexes[i+1];
	      return 1;
	    }	    
	  }
	}else{
	  //no se si hace falta el else
	  if(j+1<p2.num){
	    if(compvectors(p1.vertexes[0],p2.vertexes[j+1],2)){
	      *v1=p1.vertexes[i];
	      *v2=p1.vertexes[0];
	      return 1;
	    }
	  }else{
	    if(compvectors(p1.vertexes[0],p2.vertexes[0],2)){
	      *v1=p1.vertexes[i];
	      *v2=p1.vertexes[0];
	      return 1;
	    }	    
	  }	  
	}
      }
    }
  } 
  return 0;
}

void addvertex(struct poly *p,struct vector v,int axis)
{
  struct vector aux;
  int i;
  switch(axis){
  case XY:
    aux.x=v.x;
    aux.y=v.y;
    break;
  case YZ:
    aux.x=v.y;
    aux.y=v.z;
    break;
  case XZ:
    aux.x=v.x;
    aux.y=v.z;
  }
  aux.z=0;
  for(i=0;i<p->num;i++){
    if(compvectors(aux,p->vertexes[i],2))return;
  }
  p->num++;
  p->vertexes=realloc(p->vertexes,sizeof(struct vector)*p->num);
  p->vertexes[p->num-1]=aux;
}


int signo(double a,double b)
{
  if((a<0 && b<0) || (a>0 && b>0))return 1;
  return 0;
}

struct poly getsilhouette(struct brush *bsh,int axis)
{
  struct poly aux;
  int i,j,c;
  struct vector v1,v2;
  aux.num=0;
  aux.vertexes=NULL;

  for(i=0;i<bsh->num;i++){
    for(j=i;j<bsh->num;j++){
      if((axis==XY && signo(bsh->polys[i].normal.z,bsh->polys[j].normal.z))||
	 (axis==YZ && signo(bsh->polys[i].normal.x,bsh->polys[j].normal.x))||
	 (axis==XZ && signo(bsh->polys[i].normal.y,bsh->polys[j].normal.y))){
	if(polysadj(bsh->polys[i],bsh->polys[j],&v1,&v2)){
	  addvertex(&aux,v1,axis);
	  addvertex(&aux,v2,axis);
	}
      }
    }
  }
  aux.normal.x=0;
  aux.normal.y=0;
  aux.normal.z=-1;
  ordervertexes(&aux);
  return aux;
}
/*
  devuelve la proyeccion de un aabb en un eje
*/
struct rect getrect(struct aabb box,int axis)
{
  struct rect aux;
  if(axis==XY){
    aux.min.x=box.min.x;
    aux.min.y=box.min.y;
    aux.max.x=box.max.x;
    aux.max.y=box.max.y;
    return aux;
  }
  if(axis==YZ){
    aux.min.x=box.min.y;
    aux.min.y=box.min.z;
    aux.max.x=box.max.y;
    aux.max.y=box.max.z;
    return aux;
  }
  aux.min.x=box.min.x;
  aux.min.y=box.min.z;
  aux.max.x=box.max.x;
  aux.max.y=box.max.z;
  return aux;
}


/*
  0 si no hay interseccion
  1 si hay
*/
int interaabbbrush(struct aabb box,struct brush *bsh)
{
  int i;
  struct plane p;
  struct poly lp[3];
  struct aabb bb;
  struct rect r;

  //  return 1;//************************************

  bb=getaabb(bsh);
  if(!interaabbaabb(box,bb))return 0;
  //printf("1\n");
  //printaabb(box);
  //printaabb(bb);
  //printbrush(bsh);
  //exit(0);
  
  //return 1;//***************************************88

  //printf("2\n");

  for(i=0;i<bsh->num;i++){
    p=planepnormal(bsh->polys[i].vertexes[0],bsh->polys[i].normal);
    if(interaabbplane(box,p)==1)return 0;
  }


  //  return 1;//***************************************************************************

  for(i=0;i<3;i++){
    lp[i]=getsilhouette(bsh,i);
  }
  for(i=0;i<3;i++){
    r=getrect(box,i);
    if(interrectpoly(r,lp[i])==0){
      //printf("rect: <%lf %lf> <%lf %lf>\n",r.min.x,r.min.y,r.max.x,r.max.y);
      //printf("poly:"); printpoly(lp[i]);
      //printf("\n");
      //exit(0);
      for(i=0;i<3;i++)free(lp[i].vertexes);
      return 0;
    }
  }
  //printf("3\n");

  for(i=0;i<3;i++)free(lp[i].vertexes);
  return 1;
}
