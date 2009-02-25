#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include "../common/map.h"
//#include "../common/protocol.h"

#define MESSAGE_MAX_LENGTH 256
#define NAME_MAX_LENGTH 16
#define TXT_MAXTIME 10000



#define LMOVE 90
#define BMOVE 180
#define RMOVE 270
#define MOVE 360
#define STAND 1
#define EXIT 3
#define JUMP 5
#define FLY 8
#define UP  9
#define DOWN 10
#define NOFLY 11
#define ATTACK 12

#define BREATH 25

#define MESSAGE 6
#define SETNAME 7
