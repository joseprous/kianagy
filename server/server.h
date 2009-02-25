
struct client
{
	pthread_t thread;
	int socket;
	struct sockaddr_in address;
};

struct position
{
	float x;
	float y;
	float z;
};

struct player
{
	int id;
	struct position actual;
	int action;
	float angle;
	int impulse;
	char message[MESSAGE_MAX_LENGTH];
	char name[NAME_MAX_LENGTH];
	float jumpheight;
	int fly;
	int ontransport;
	int vitality;
	int colddowns[MAX_ATTACKS];
	int hurt;
	struct player *last_attacker;
	struct WorldObject *inobject;
	//struct position last;
};

struct node
{
	struct client *that;
	struct player *him;
	struct node *previous;
	struct node *next;
	
};

struct pathnode
{
	struct position actual;
	struct pathnode *next;
	int wait;
};

struct transporter
{
	int id;
	struct position actual;
	struct position vector;
	struct pathnode *targetlist;
	struct pathnode *nexttarget;
	float radius;
	int wait;
	struct transporter *next;
	struct WorldObject *object;
};

struct node root;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int s_socket,yes, sin_size;
pthread_t fb_thread,server_thread;
struct sockaddr_in s_address;
struct client *newclient;
pthread_attr_t tattr;
struct transporter *bus;
struct WorldObject *map;
struct Npc *npc_root;
struct Attack *attacks[MAX_ATTACKS];

struct player *CreatePlayer(int id,float x,float y,float z,char *name,struct WorldObject *object);
void DestroyPlayer(struct player *this);
struct transporter *CreateTransporter(int id,struct pathnode *targetlist, float radius, char *filename);
void LoadTransporters(char *filename);
struct pathnode *CreatePathNode(float x,float y,float z,struct pathnode *next,int wait);
void DestroyPathNodes(struct pathnode *root);
void DestroyTransporters(struct transporter *root);
float Distance2p(struct position *origin, struct position *target);
int InRange(struct position *origin, struct position *target, int radius);
struct position UnitVector(struct position *origin, struct position *destination);
int InObject(struct WorldObject *object, struct position *that, struct position *him);
struct position MapToObject(struct position *that, struct position *him);
struct position ObjectToMap(struct position *that, struct position *him);
void InTransport(struct transporter *this,struct player *him);
void OutTransport(struct transporter *that,struct player *him);
struct transporter *GetTransport(int id);
struct node *Register(struct client *this, struct player *me);
void UnRegister(struct node *mine);
void ShowRegister();
void Broadcast(char *message);
void GetMessage(char *buffer,struct player *me);
void ClientHandler(void *ptr);
int EqualsPos(struct position *origin, struct position *destination);
int PosModified(struct player *him);
void RealmHandler();
void ShutdownHandler();
void ServerHandler();
