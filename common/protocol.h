#define BROADCAST_HEADER 0
#define MESSAGE_HEADER 1
#define CONTROL_HEADER 2

#define MESSAGE_SIZE 50
#define NAME_SIZE 12

#define STAND 1
#define LMOVE 2
#define BMOVE 3
#define RMOVE 4
#define MOVE 5
#define JUMP 6
#define FLY 7
#define UP 8
#define DOWN 9
#define NOFLY 10
#define ATTACK 11

#define MESSAGE 61
#define SETNAME 62
#define EXIT 63

struct broadcast
{
	unsigned char header:2;
	unsigned char type:2;
	unsigned char id:8;
	unsigned char model:7;
	unsigned char area_id:6;
	unsigned int pos_x:13;
	unsigned int pos_y:13;
	unsigned int pos_z:13;
	unsigned char vec_x:8;
	unsigned char vec_y:8;
	unsigned char vec_z:8;
	unsigned char speed:8;
	unsigned char vitality:7;
	unsigned char action:6;
};


struct chat
{
	unsigned char header:2;
	unsigned char id:8;
	unsigned char message[MESSAGE_SIZE];	
};

struct control
{
	unsigned char header:2;
	unsigned char id:8;
	unsigned char message[NAME_SIZE];
};

struct feedback
{
	unsigned char action:6;
	unsigned char vec_x:8;
	unsigned char vec_y:8;
	unsigned char vec_z:8;
	unsigned char message[MESSAGE_SIZE];
};
