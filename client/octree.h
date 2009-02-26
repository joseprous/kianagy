#ifndef OCTREE_H
#define OCTREE_H
#include "../common/map.h"

#define XY 0
#define YZ 1
#define XZ 2

//Axis-aligned Bounding Box
struct aabb{
  struct vector min,max;
};

//loose octree
struct loctree{
  struct aabb box;
  struct brushlist *brushes;
  struct loctree *hijos[8];
};

struct loctree *loadloctree(struct map *m,float size,int maxdepth);

void loctreestats(struct loctree *m);

#endif
