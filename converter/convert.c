#include <stdio.h>
#include <stdlib.h>
#include "../common/map.h"
#include "../common/collisionsys.h"

struct Coordinate size,start;
struct WorldObject *object;

void CreateGrid(struct map *m,int mode)
{
	int i;
	struct brushlist *bl;

	start.x = m->start.x;
	start.y = m->start.y;
	start.z = m->start.z;
	object = CreateWorldObject(size,start);	
	i=0;
	bl=m->entities->ent.brushes;
	while(bl)
	{
		BrushToGrid(bl->bsh,object,mode);
		bl=bl->next;
		printf("%d...\n",i++);
	}
	printf("Brushes = %d\n",i);
}

int main(int argc, char** argv)
{	
	if(argc!=7){
		printf("usage: test <input.map:file> <output.bcf:file> <x:integer> <y:integer> <z:integer> <mode:bin>\n");
		return 0;
	}
	
	size.x = atoi(argv[3]);
	size.y = atoi(argv[4]);
	size.z = atoi(argv[5]);
	if(!parsemap(argv[1]))return 0;
	printf("Map has been parsed, size = (%d,%d,%d) on %d mode.\n",size.x,size.y,size.z,atoi(argv[6]));
	CreateGrid(currentmap,atoi(argv[6]));
	ObjectToFile(object,argv[2]);
	//ShowMatrix(object->matrix,&object->size);
	DestroyWorldObject(object);
   return 0;
}
