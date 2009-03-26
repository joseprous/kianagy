#ifndef OCTREE_H
#define OCTREE_H

#include <GL/gl.h>
#include "../common/map.h"

//loose octree
struct loctree{
  GLuint gllistf,gllistw;
  int draw;
  struct aabb box;
  struct brushlist *brushes;
  struct loctree *hijos[8];
};

//octree donde los brushes pueden aparecer en varios nodos
struct roctree{
  int draw;
  struct aabb box;
  struct brushlist *brushes;
  struct roctree *hijos[8];
};

struct loctree *loadloctree(struct map *m,float size,int maxdepth);
struct roctree *loadroctree(struct map *m,int maxdepth);

struct poly getsilhouette(struct brush *bsh,int axis);

struct aabb getaabb(struct brush *bsh);

void loctreestats(struct loctree *m);

int interaabbbrush(struct aabb box,struct brush *bsh,struct aabb bb,struct poly *silh);
void loctreegenlists(struct loctree *m,int maxdepth);

#endif
