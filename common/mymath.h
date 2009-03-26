#ifndef MYMATH_H
#define MYMATH_H
#define PI 3.14159265358979323846

//#include "map.h"

#define XY 0
#define YZ 1
#define XZ 2


struct matrix{
	double data[3][3];
};

struct vector{
	double x,y,z;
};

struct vector2d{
  double x,y;
};

struct rect{
  struct vector2d min, max;
};

struct line{
  struct vector dir;
  struct vector point;
};
//ax+by+cz+d=0
struct plane{
	float a,b,c,d;
	struct vector normal;
};

//ax + by + c = 0
struct line2d{
  struct vector normal;
  double a,b,c;
};


//Axis-aligned Bounding Box
struct aabb{
  struct vector min,max;
};


struct texture{
	char *name;
};

struct points{
	struct vector p[3];
};

struct poly{
  int num;
  struct vector *vertexes;
  struct vector center;
  struct vector normal;
  struct texture tex;
};

struct brush{
  unsigned long int draw;
  int num;
  struct poly *polys;
};


struct vector normalize(struct vector v);
double vectorlen(struct vector v);
struct vector cross(struct vector v1,struct vector v2);
double dot(struct vector v1,struct vector v2);
struct vector sumvectors(struct vector v1,struct vector v2);
struct vector difvectors(struct vector v1,struct vector v2);	
struct vector mulvector(double n,struct vector v);
int inter2planes(struct plane p1, struct plane p2, struct line *l1);
struct plane planepnormal(struct vector point,struct vector normal);	
struct plane plane3points(struct vector p1,struct vector p2,struct vector p3);
int interlineplane(struct line line1,struct plane plane1,struct vector *point);
int pointinplane(struct vector point,struct plane p);
struct line line2points(struct vector p1,struct vector p2);
double distpointlineseg(struct vector p,struct vector q1,struct vector q2);
struct vector segmidpoint(struct vector q1,struct vector q2);
int comppoints(struct vector p1,struct vector p2);
struct vector mulvecmatrix(struct vector v,struct matrix m);
struct vector rotatevecz(struct vector v,float ang);

struct aabb getaabb(struct brush *bsh);
int pointinaabb(struct vector v,struct aabb box);
int brushinaabb(struct brush *bsh,struct aabb box);
int interaabbplane(struct aabb box,struct plane p);
int pointinline2d(struct vector2d point,struct line2d l);
int interrectline(struct rect r,struct line2d l);
struct rect getbbox(struct poly p);
int interrectrect(struct rect r1,struct rect r2);
struct line2d line2points2d(struct vector p1,struct vector p2);
int interrectpoly(struct rect r,struct poly p);
int interaabbaabb(struct aabb box1,struct aabb box2);
int compvectors(struct vector v1,struct vector v2,double prec);
int signo(double a,double b);
struct poly getsilhouette(struct brush *bsh,int axis);
struct rect getrect(struct aabb box,int axis);
int interaabbbrush(struct aabb box,struct brush *bsh,struct aabb bb,struct poly *silh);
int polysadj(struct poly p1,struct poly p2,struct vector *v1,struct vector *v2);
void ordervertexes(struct poly *p);


#endif
