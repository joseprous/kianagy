#ifndef DRAWMAP_H
#define DRAWMAP_H
#include "octree.h"
#define WIREFRAME 1
#define SOLID 2
#define FLAT 3

void _drawmap(struct map *m);
void drawloctree(struct loctree *m);
void drawbrush(struct brush *b,int mode);

#endif
