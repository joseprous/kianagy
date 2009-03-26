#ifndef MAP_H
#define MAP_H
#include "mymath.h"

struct rawbrush{
	int num;
	struct points *planes;
	struct texture *textures;
};

struct brushlist{
        int used;//para crear los arboles
	struct rawbrush *rbsh;
	struct brush	*bsh;
	struct brushlist *next;
};

struct headerlist{
	char *str1,*str2;
	struct headerlist *next;
};

struct entity{
	struct headerlist *header;
	struct brushlist *brushes;
};

struct entitylist{
	struct entity ent;
	struct entitylist *next;	
};

struct map{
	struct vector start;
	struct entitylist *entities;	
};


struct brush *loadbrush(struct rawbrush *b);
	
int parsemap(char *file);
void ordervertexes(struct poly *p);


extern struct map *currentmap;
#endif
