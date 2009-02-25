#include "mymath.h"
#include <math.h>
#include <stdio.h>

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
