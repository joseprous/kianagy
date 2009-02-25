#define FREE 0
#define USED 1

#define SOLID 0
#define LIQUID 1

#define OUTOFMAP 2
#define TRUE 1
#define FALSE 0
#define SPECIAL -1

#define POLY_MAX_VERTEX 512

#define GROUND 0

#define SCALE_X 16
#define SCALE_Y 16
#define SCALE_Z 16

#define PLAYER_SIZE_X 16
#define PLAYER_SIZE_Y 16
#define PLAYER_SIZE_Z 64

struct Cell
{
	unsigned char state:1;
	unsigned char type:1;
};

struct Coordinate
{
	int x;
	int y;
	int z;
};

struct WorldObject
{
	struct Coordinate start;
	struct Coordinate size;
	struct Cell ***matrix;
};

struct Cell ***CreateMatrix(struct Coordinate *size);
void DestroyMatrix(struct Cell ***matrix,struct Coordinate *size);
void ShowMatrix(struct Cell ***matrix,struct Coordinate *size);
struct WorldObject *CreateWorldObject(struct Coordinate real_size,struct Coordinate start);
void DestroyWorldObject(struct WorldObject *object);
void ObjectToFile(struct WorldObject *object, char *filename);
struct WorldObject *ObjectFromFile(char *filename);
int Max3ui(int a,int b,int c);
void MatrixToObject(struct Cell ***tmp,struct WorldObject *object);
int LastVertex(struct Cell ***tmp,int size_x,int first,int j,int k);
int FirstVertex(struct Cell ***tmp,int size_x,int j,int k);
void DrawTilVertex(struct Cell ***tmp,int first,int j,int k,int last);
void DrawVolume(struct Cell ***tmp,struct WorldObject *object);
struct Coordinate UnitVectorUI(struct Coordinate *point1, struct Coordinate *point2);
struct Coordinate ApplyVector(struct Coordinate *coord,struct Coordinate *vector);
struct Coordinate PosToCoor(float x,float y,float z);
int EqualsCoor(struct Coordinate *point1, struct Coordinate *point2);
void PolyToGrid(struct Cell ***tmp,struct WorldObject *object,struct poly *poly);
void BrushToGrid(struct brush *brush, struct WorldObject *object,int mode);
int InMatrix(struct Coordinate *actual, struct WorldObject *object);
float GetGround(struct Coordinate *actual,struct WorldObject *object);
float GetHeight(struct Coordinate *actual,struct WorldObject *object);
int BlockAvailable(struct Coordinate *actual,struct WorldObject *object);
int Collision(struct Coordinate *origin, struct Coordinate *destination, struct WorldObject *object);