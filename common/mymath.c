#include "mymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double vectorlen(struct vector v)
{
	return sqrt(dot(v,v));
}
struct vector normalize(struct vector v)
{
	return mulvector(1/vectorlen(v),v);	
}
struct vector cross(struct vector v1,struct vector v2)
{
	struct vector aux;
	aux.x=v1.y*v2.z-v1.z*v2.y;
	aux.y=v1.z*v2.x-v1.x*v2.z;
	aux.z=v1.x*v2.y-v1.y*v2.x;
	return aux;
}
double dot(struct vector v1,struct vector v2)
{
	return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}
struct vector sumvectors(struct vector v1,struct vector v2)
{
	struct vector aux;
	aux.x=v1.x+v2.x;
	aux.y=v1.y+v2.y;
	aux.z=v1.z+v2.z;
	return aux;	
}
struct vector difvectors(struct vector v1,struct vector v2)
{
	struct vector aux;
	aux.x=v1.x-v2.x;
	aux.y=v1.y-v2.y;
	aux.z=v1.z-v2.z;
	return aux;	
}
struct vector mulvector(double n,struct vector v)
{
	struct vector aux;
	aux.x=v.x*n;
	aux.y=v.y*n;
	aux.z=v.z*n;
	return aux;		
}

int inter2planes(struct plane p1, struct plane p2, struct line *l1)
{
    double s1, s2, a, b;
	double n1n2dot,n1normsqr,n2normsqr;
    struct vector d = cross(p1.normal,p2.normal);
    if (vectorlen(d) == 0) {
        return 0;
    }
	
    l1->dir = d;
    s1 = p1.d; // d from the plane equation
    s2 = p2.d;
    n1n2dot = dot(p1.normal, p2.normal);
    n1normsqr = dot(p1.normal, p1.normal);
    n2normsqr = dot(p2.normal, p2.normal);
    a = (s2 * n1n2dot - s1 * n2normsqr) / (n1n2dot*n1n2dot - n1normsqr * n2normsqr);
    b = (s1 * n1n2dot - s2 * n1normsqr) / (n1n2dot*n1n2dot - n1normsqr * n2normsqr);
	
    l1->point=sumvectors(mulvector(a,p1.normal),mulvector(b,p2.normal));
    //l1.point = a * p1.normal + b * p2.normal;
	
	
    l1->point=mulvector(-1.0,l1->point);
	
    return 1;
}

struct plane planepnormal(struct vector point,struct vector normal)
{
	struct plane aux;
	aux.normal=normal;
	aux.a=normal.x;
	aux.b=normal.y;
	aux.c=normal.z;
	aux.d=-dot(normal,point);
	return aux;
}
struct plane plane3points(struct vector p1,struct vector p2,struct vector p3)
{
	struct vector aux;
	aux=cross(difvectors(p3,p1),difvectors(p2,p1));
	return planepnormal(p1,aux);	
}


int interlineplane(struct line line1,struct plane plane1,struct vector *point)
{
	struct vector p1,p2;
	double u,d;
	p1=line1.point;
	p2=sumvectors(line1.point,line1.dir);
	d=plane1.a*(p1.x-p2.x)+plane1.b*(p1.y-p2.y)+plane1.c*(p1.z-p2.z);
	if(d!=0){
		u=(plane1.a*p1.x+plane1.b*p1.y+plane1.c*p1.z+plane1.d)/d;
		*point=sumvectors(p1,mulvector(u,difvectors(p2,p1)));
		return 1;
	}
	return 0;
}
int pointinplane(struct vector point,struct plane p)
{
	double res;
	res=p.a * point.x + p.b * point.y + p.c * point.z + p.d;
	if(res<1 && res>-1){
		return 0;	
	}else{
		if(res<0){		
			return -1;
		}else{
			return 1;			
		}
	}
	//printf("res:%lf\n",res);		
}

struct line line2points(struct vector p1,struct vector p2)
{
	struct line aux;
	aux.dir=difvectors(p1,p2);
	aux.point=p1;
	return aux;	
}


int pointinline(struct vector point,struct line l)
{
	double res;
	struct vector p1,p2;
	p1=difvectors(point,l.point);
	p2=l.dir;
	res=acos(dot(p1,p2)/(vectorlen(p1)*vectorlen(p2)));
	//printf("res:%lf\n",res);
	if((res<0.01 && res>-0.01)||(res<PI+0.01 && res>PI-0.01)){
		return 1;
	}else{
		return 0;
	}
	//printf("res:%f\n",res);		
}

double distpointlineseg(struct vector p,struct vector q1,struct vector q2)
{
	return vectorlen(cross(difvectors(q2,q1),difvectors(q1,p)))/vectorlen(difvectors(q2,q1));
}

struct vector segmidpoint(struct vector q1,struct vector q2)
{
	struct vector aux;
	aux.x=(q1.x+q2.x)/2;
	aux.y=(q1.y+q2.y)/2;
	aux.z=(q1.z+q2.z)/2;
	return aux;	
}

//d = abs(cross(Q2-Q1,P-Q1))/abs(Q2-Q1);

int comppoints(struct vector p1,struct vector p2){
	if((p1.x > p2.x-1 && p1.x < p2.x+1)&&(p1.y > p2.y-1 && p1.y < p2.y+1)&&(p1.z > p2.z-1 && p1.z < p2.z+1))
		return 1;
	return 0;
}

struct vector mulvecmatrix(struct vector v,struct matrix m)
{
	struct vector aux;
	aux.x=m.data[0][0]*v.x+m.data[0][1]*v.y+m.data[0][2]*v.z;		
	aux.y=m.data[1][0]*v.x+m.data[1][1]*v.y+m.data[1][2]*v.z;		
	aux.z=m.data[2][0]*v.x+m.data[2][1]*v.y+m.data[2][2]*v.z;
	return aux;	
}

struct vector rotatevecz(struct vector v,float ang)
{
	struct vector aux;
	struct matrix m;
	m.data[0][0]=cos(ang);
	m.data[0][1]=-sin(ang);
	m.data[0][2]=0;
	m.data[1][0]=sin(ang);
	m.data[1][1]=cos(ang);
	m.data[1][2]=0;
	m.data[2][0]=0;
	m.data[2][1]=0;
	m.data[2][2]=1;
	aux=mulvecmatrix(v,m);
	return aux;
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
  1 si el punto esta dentro del aabb
  0 en caso contrario
*/
int pointinaabb(struct vector v,struct aabb box)
{
  if(v.x<box.min.x||v.x>box.max.x)return 0;
  if(v.y<box.min.y||v.y>box.max.y)return 0;
  if(v.z<box.min.z||v.z>box.max.z)return 0;
  return 1;
}
/*
  1 si el brush esta dentro del aabb
  0 en caso contrario
*/
int brushinaabb(struct brush *bsh,struct aabb box)
{
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
  verifica si se cumplen las propiedades de los
  2 puntos del aabb
*/
int checkaabb(struct aabb box)
{
  if(box.min.x>box.max.x)return 0;
  if(box.min.y>box.max.y)return 0;
  if(box.min.z>box.max.z)return 0;
  return 1;
}
/*
  retorna el p vertex descripto en *graphics gems 4*
*/
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
  //if(p.normal.x<=0 && p.normal.y>=0 && p.normal.z<=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
    //}
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
  //  if(p.normal.x>=0 && p.normal.y<=0 && p.normal.z>=0){
    aux.x=box.min.x;
    aux.y=box.max.y;
    aux.z=box.min.z;
    return aux;
    //}
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
  //if(l.normal.x<=0 && l.normal.y>=0){
    aux.x=r.min.x;
    aux.y=r.max.y;
    return aux;
    //}
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
  //if(l.normal.x>=0 && l.normal.y<=0){
    aux.x=r.min.x;
    aux.y=r.max.y;
    return aux;
    //}
}
/*
  0 si intersecta
  1 positive half plane
  -1 negative half plane
*/
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
      aux.c=(p1.y-aux.normal.x*p1.x)*(-1);    
    }
  }
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
  if(!interrectrect(r,bb))return 0;
  for(i=0;i<p.num-1;i++){
    if(interrectline(r,line2points2d(p.vertexes[i],p.vertexes[i+1]))==1){
      /*printf("rect: <%lf %lf> <%lf %lf>\n",r.min.x,r.min.y,r.max.x,r.max.y);
      printpoly(p);
      printf("\n");
      printvector(p.vertexes[i]);
      printf("\n");
      printvector(p.vertexes[i+1]);
      exit(0);*/
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
  if((v1.x > v2.x-prec && v1.x < v2.x+prec)&&
     (v1.y > v2.y-prec && v1.y < v2.y+prec)&&
     (v1.z > v2.z-prec && v1.z < v2.z+prec))
    return 1;
  return 0;
}
/*
  retorna 1 si los numeros tienen el mismo signo
*/
int signo(double a,double b)
{
  if((a<0 && b<0) || (a>0 && b>0))return 1;
  return 0;
}
/*
  usado en getsilhouette
  agrega un vertice al poligono p en el eje axis
*/
void _addvertex(struct poly *p,struct vector v,int axis)
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

void ordervertexes(struct poly *p)
{
  struct vector centro,p1,p2,pointaux;
  int i,ban;
  double *angs,gdot,ga;
  
  angs=malloc(sizeof(double)*p->num);
  centro.x=0;
  centro.y=0;
  centro.z=0;
  for(i=0;i<p->num;i++){
    centro.x+=p->vertexes[i].x;
    centro.y+=p->vertexes[i].y;
    centro.z+=p->vertexes[i].z;
  }
  centro.x/=p->num;
  centro.y/=p->num;
  centro.z/=p->num;

  p->center=centro;
		
  p1=difvectors(p->vertexes[0],centro);
  for(i=1;i<p->num;i++){			
    p2=difvectors(p->vertexes[i],centro);
    angs[i]=acos(dot(p1,p2)/(vectorlen(p1)*vectorlen(p2)));
    pointaux=cross(p1,p2);
    gdot=dot(pointaux,p->normal);
			
    if(gdot>-0.01 && gdot< 0.01){
      angs[i]=PI;					
    }else{
      if(gdot<0){
	angs[i]=2*PI-angs[i];					
      }
    }
  }
  do{
    ban=0;
    for(i=1;i<p->num-1;i++){
      if(angs[i]>angs[i+1]){
	ga=angs[i];
	angs[i]=angs[i+1];
	angs[i+1]=ga;
	pointaux=p->vertexes[i];
	p->vertexes[i]=p->vertexes[i+1];
	p->vertexes[i+1]=pointaux;
	ban=1;
      }	
    }
  }while(ban);
  free(angs);			  
}

/*
  retorna un poligono con la silueta del brush
  proyectada en el eje axis
*/
struct poly getsilhouette(struct brush *bsh,int axis)
{
  struct poly aux;
  int i,j;
  struct vector v1,v2;
  aux.num=0;
  aux.vertexes=NULL;

  for(i=0;i<bsh->num;i++){
    for(j=i;j<bsh->num;j++){
      if((axis==XY && signo(bsh->polys[i].normal.z,bsh->polys[j].normal.z))||
	 (axis==YZ && signo(bsh->polys[i].normal.x,bsh->polys[j].normal.x))||
	 (axis==XZ && signo(bsh->polys[i].normal.y,bsh->polys[j].normal.y))){
	if(polysadj(bsh->polys[i],bsh->polys[j],&v1,&v2)){
	  _addvertex(&aux,v1,axis);
	  _addvertex(&aux,v2,axis);
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
int interaabbbrush(struct aabb box,struct brush *bsh,struct aabb bb,struct poly *silh)
{
  int i;
  struct plane p;
  struct rect r;
  if(!interaabbaabb(box,bb))return 0;

  for(i=0;i<bsh->num;i++){
    p=planepnormal(bsh->polys[i].vertexes[0],bsh->polys[i].normal);//posible precalcular
    if(interaabbplane(box,p)==1)return 0;
  }
  for(i=0;i<3;i++){
    r=getrect(box,i);
    if(interrectpoly(r,silh[i])==0){
      return 0;
    }
  }
  return 1;
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
