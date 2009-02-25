#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "map.h"
#include "collisionsys.h"

struct Cell ***CreateMatrix(struct Coordinate *size)
{
	int i,j;
	struct Cell ***new;

	new = malloc(sizeof(struct Cell**)*size->x);
	for (i=0;i<size->x;i++)
	{
		new[i] = malloc(sizeof(struct Cell*)*size->y);
		for (j=0;j<size->y;j++)
		{
			new[i][j] = malloc(sizeof(struct Cell)*size->z);
			memset(new[i][j],0,sizeof(struct Cell)*size->z);
		}
	}
	return new;
}

void DestroyMatrix(struct Cell ***matrix,struct Coordinate *size)
{
	int i,j;
	for (i=0;i<size->x;i++)
	{
		for (j=0;j<size->y;j++)
		{
			free(matrix[i][j]);
		}
		free(matrix[i]);
	}
	free(matrix);
}

void ShowMatrix(struct Cell ***matrix,struct Coordinate *size)
{
	int i,j,k;
	for (k=0;k<size->z;k++)
	{
		for (i=0;i<size->x;i++)
		{
			for (j=0;j<size->y;j++)
			{
				printf("(%d,%d)",matrix[i][j][k].state,matrix[i][j][k].type);
			}
			printf("\n");
		}
		printf("\n");
	}
	printf("ENDOFOBJECT\n");
}

struct WorldObject *CreateWorldObject(struct Coordinate real_size,struct Coordinate start)
{
	struct WorldObject *new;
	
	new = malloc(sizeof(struct WorldObject));
	new->start = start;
	new->size.x = real_size.x/SCALE_X;
	new->size.y = real_size.y/SCALE_Y;
	new->size.z = real_size.z/SCALE_Z;
	new->matrix = CreateMatrix(&new->size);
	return new;
}

void DestroyWorldObject(struct WorldObject *object)
{
	DestroyMatrix(object->matrix,&object->size);
	free(object);
}

void ObjectToFile(struct WorldObject *object, char *filename)
{
	int i,j;
	FILE *output;
	if ((output = fopen(filename,"wb"))==NULL){printf("Error while saving object to file.\n"); return;}

	fprintf(output,"%d %d %d\n",object->start.x,object->start.y,object->start.z);
	fprintf(output,"%d %d %d\n",object->size.x,object->size.y,object->size.z);
	for (i=0;i<object->size.x;i++)
	{
		for (j=0;j<object->size.y;j++)
		{
			fwrite(object->matrix[i][j], sizeof(struct Cell),object->size.z, output);
		}
	}
	fclose(output);
}

struct WorldObject *ObjectFromFile(char *filename)
{
	int i,j;
	FILE *input;
	struct WorldObject *new;
	struct Coordinate start,size;

	if ((input = fopen(filename,"rb"))==NULL){printf("Error while loading object %s.\n",filename);}
	
	fscanf(input,"%d %d %d\n",&start.x,&start.y,&start.z);
	fscanf(input,"%d %d %d\n",&size.x,&size.y,&size.z);

	new = malloc(sizeof(struct WorldObject));
	new->start = start;
	new->size = size;
	new->matrix = CreateMatrix(&size);

	for (i=0;i<new->size.x;i++)
	{
		for (j=0;j<new->size.y;j++)
		{
			fread(new->matrix[i][j], sizeof(struct Cell),new->size.z, input);
		}
	}
	fclose(input);
	return new;
}


/*
	Functions for matrix rendering (Planes and Volume).
*/
int Max3ui(int a,int b,int c)
{
	if (a>b && a>c) return a;
	if (b>c) return b;
	return c;
}

void MatrixToObject(struct Cell ***tmp,struct WorldObject *object)
{
	int i,j,k;
	for (i=0;i<object->size.x;i++)
	{
		for (j=0;j<object->size.y;j++)
		{
			for (k=0;k<object->size.z;k++)
			{
				if (tmp[i][j][k].state == USED){ object->matrix[i][j][k].state = USED;}
			}
		}
	}
}

int LastVertex(struct Cell ***tmp,int size_x,int first,int j,int k)
{
	int last;
	last = first;
	for (++first;first<size_x;first++){if (tmp[first][j][k].state == USED){last = first;}}
	return last;
}

int FirstVertex(struct Cell ***tmp,int size_x,int j,int k)
{
	int i;
	for (i=0;i<size_x;i++){if (tmp[i][j][k].state == USED){return i;}}
	return size_x;
}

void DrawTilVertex(struct Cell ***tmp,int first,int j,int k,int last)
{
	for (;first<last;first++){tmp[first][j][k].state = USED;}
}

void DrawVolume(struct Cell ***tmp,struct WorldObject *object)
{
	int first,j,k,last;
	for(k=0;k<object->size.z;k++)
	{
		for(j=0;j<object->size.y;j++)
		{
			first = FirstVertex(tmp,object->size.x,j,k);
			if (first!=object->size.x)
			{
				last = LastVertex(tmp,object->size.x,first,j,k);
				if (last!=first) {DrawTilVertex(tmp,first+1,j,k,last);}
				
			}
		}		
	}
}

struct Coordinate UnitVectorUI(struct Coordinate *point1, struct Coordinate *point2)
{
	int r;
	struct Coordinate vector;
	
	vector.x = point2->x - point1->x;
	vector.y = point2->y - point1->y;
	vector.z = point2->z - point1->z;
	
	r = Max3ui(abs(vector.x),abs(vector.y),abs(vector.z));
	//printf("UnitVectorUI:vector: (%d,%d,%d), R: %d\n",vector.x,vector.y,vector.z,r);
	if (r!=0)
	{
		vector.x = vector.x/r;
		vector.y = vector.y/r;
		vector.z = vector.z/r;
	}
	return vector;
}

struct Coordinate ApplyVector(struct Coordinate *coord,struct Coordinate *vector)
{
	struct Coordinate new;
	
	new.x = coord->x + vector->x;
	new.y = coord->y + vector->y;
	new.z = coord->z + vector->z;
	
	return new;
}

struct Coordinate PosToCoor(float x,float y,float z)
{
	struct Coordinate new;

	new.x = x/SCALE_X;
	new.y = y/SCALE_Y;
	new.z = z/SCALE_Z;

	return new;
}

int EqualsCoor(struct Coordinate *point1, struct Coordinate *point2)
{
	if((point1->x!=point2->x)||(point1->y!=point2->y)||(point1->z!=point2->z)) {return FALSE;}
	return TRUE;
}

void PolyToGrid(struct Cell ***tmp,struct WorldObject *object,struct poly *poly)
{
	
	int vertex;
	struct Coordinate vector,vector2,aux,aux2,vertexgrid[POLY_MAX_VERTEX];

	if (poly->num<3) return;
	for(vertex=0;vertex<poly->num;vertex++)
	{
		vertexgrid[vertex] = PosToCoor(poly->vertexes[vertex].x,poly->vertexes[vertex].y,poly->vertexes[vertex].z);
		//printf("(%lf,%lf,%lf)\n",poly->vertexes[vertex].x,poly->vertexes[vertex].y,poly->vertexes[vertex].z);
		//printf("(%d,%d,%d)\n",vertexgrid[vertex].x,vertexgrid[vertex].y,vertexgrid[vertex].z);
	}	

	for(vertex=0;vertex<poly->num;vertex++)
	{
		aux = vertexgrid[vertex%poly->num];
		//printf("(%d,%d,%d)\n",aux.x,aux.y,aux.z);
		tmp[aux.x][aux.y][aux.z].state = USED;
		//printf("(%d,%d,%d)\n",aux.x,aux.y,aux.z);
		while(!EqualsCoor(&aux,&vertexgrid[(vertex+1)%poly->num]))
		{
			//printf("aux: (%d,%d,%d)\n",aux.x,aux.y,aux.z);
			aux2 = aux;
			while(!EqualsCoor(&aux2,&vertexgrid[(vertex+2)%poly->num]))
			{
				//printf("aux2: (%d,%d,%d)\n",aux2.x,aux2.y,aux2.z);	
				vector2 = UnitVectorUI(&aux2,&vertexgrid[(vertex+2)%poly->num]);
				//printf("vector2: (%d,%d,%d)\n",vector2.x,vector2.y,vector2.z);
				aux2 = ApplyVector(&aux2,&vector2);
				tmp[aux2.x][aux2.y][aux2.z].state = USED;
			}
			
			vector = UnitVectorUI(&aux,&vertexgrid[(vertex+1)%poly->num]);
			aux = ApplyVector(&aux,&vector);
			tmp[aux.x][aux.y][aux.z].state = USED;
		}
	}
	//printf("ENDOFPOLY\n");
}

void BrushToGrid(struct brush *brush, struct WorldObject *object,int mode)
{
	int poly;
	struct Cell ***tmp;

	tmp = CreateMatrix(&object->size);
	
	for (poly=0;poly<brush->num;poly++)
	{
		PolyToGrid(tmp,object,&brush->polys[poly]);
	}
	if(mode){DrawVolume(tmp,object);}
	MatrixToObject(tmp,object);
	DestroyMatrix(tmp,&object->size);
	//printf("ENDOFBRUSH\n");
}

/*
	Functions for collision system to apply.
*/

int InMatrix(struct Coordinate *actual, struct WorldObject *object)
{
	if ((actual->x>=object->size.x)||(actual->y>=object->size.y)||(actual->z>=object->size.z)){return FALSE;}
	if ((actual->x < 0)||(actual->y < 0)||(actual->z < 0)){return FALSE;}
	return TRUE;
}

float GetGround(struct Coordinate *actual,struct WorldObject *object)
{
	int i,j,k;
	k = actual->z-1;
	while(k >= GROUND)
	{
		for (i=((PLAYER_SIZE_X/SCALE_X)-1);i>=0;i--)
		{
			for (j=((PLAYER_SIZE_Y/SCALE_Y)-1);j>=0;j--)
			{
				if (object->matrix[actual->x-i][actual->y-j][k].state == USED) return k+1;
			}
		}
		k--;
	}
	return GROUND;
}

float GetHeight(struct Coordinate *actual,struct WorldObject *object)
{
	int i,j,k;
	k = actual->z;
	while(k <= object->size.z-1)
	{
		for (i=((PLAYER_SIZE_X/SCALE_X)-1);i>=0;i--)
		{
			for (j=((PLAYER_SIZE_Y/SCALE_Y)-1);j>=0;j--)
			{
				if (object->matrix[actual->x-i][actual->y-j][k].state == USED) return k;
			}
		}
		k++;
	}
	return object->size.z-1;
}

int BlockAvailable(struct Coordinate *actual,struct WorldObject *object)
{
	int i,j,k,special;
	special = TRUE;
	for(k=(PLAYER_SIZE_Z/SCALE_Z);k>=0;k--)
	{
		for (i=((PLAYER_SIZE_X/SCALE_X)-1);i>=0;i--)
		{
			for (j=((PLAYER_SIZE_Y/SCALE_Y)-1);j>=0;j--)
			{
				if (object->matrix[actual->x-i][actual->y-j][actual->z+k].state == USED)
				{
					if (k==(PLAYER_SIZE_Z/SCALE_Z)){special=FALSE; continue;}
					if (k==0 && special==TRUE){return SPECIAL;}
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

int Collision(struct Coordinate *origin, struct Coordinate *destination, struct WorldObject *object)
{
	int r,special;
	struct Coordinate vector,aux;

	if (!InMatrix(destination,object)){return OUTOFMAP;}
	special = FALSE;
	aux = *origin;

	
	while (!((aux.x==destination->x)&&(aux.y==destination->y)))
	{	
			vector = UnitVectorUI(&aux,destination);
			//printf("here?\n");

			aux.x = (aux.x + vector.x*(PLAYER_SIZE_X/SCALE_X));
			aux.y = (aux.y + vector.y*(PLAYER_SIZE_Y/SCALE_Y));

			switch (BlockAvailable(&aux,object))
			{
				case FALSE: {return FALSE;}
				case SPECIAL: {aux.z++;special = TRUE; break;}
			}
	}
	if (special==TRUE) return (aux.z*(SPECIAL));
	return TRUE;
}
