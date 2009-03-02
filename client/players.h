#include "md2.h"
struct player{
	int tipo;
	int id;
	int model;
	int weapon;
	int hp;
	float x,z,y;
	float dir;
	float vel;
	int acc;
	char txt[MESSAGE_MAX_LENGTH+1];
	char name[NAME_MAX_LENGTH+1];
	int txt_ini_time;
	int alive;
	int n;
	float interp;	
	struct player *next;
};

#define NMOVES 361

struct player_model{
	struct md2_model_t md2file;
	int hasweapon;
	//struct md2_model_t weapon;
	int fps;
	int iniframes[NMOVES];
	int endframes[NMOVES];
};

void drawpj();
void drawplayer(struct player *p);
void load_models();
