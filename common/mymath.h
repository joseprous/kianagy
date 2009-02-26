#ifndef MYMATH_H
#define MYMATH_H
#define PI 3.14159265358979323846
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
#endif
