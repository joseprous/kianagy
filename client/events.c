#include "client.h"
#include "players.h"
#include "network.h"

extern float pj_ang;
extern char pj_txtbuf[MESSAGE_MAX_LENGTH+1];
extern int escribiendo;
extern struct player *pj;
extern float cam_ang,cam_dist;	
extern Uint32 lastmsg;

int fly;

void rotcam(float angulo,float z);

void salir()
{
	send_msg(EXIT);
	//end_network();	
	exit (0);
}

int otherkeys()
{
	Uint8 * keystate;
	keystate = SDL_GetKeyState(NULL);
	if(keystate[SDLK_d]){
		send_msg(RMOVE);
		return 1;
	}
	if(keystate[SDLK_a]){
		send_msg(LMOVE);
		return 1;
	}
	if(keystate[SDLK_w]){
		send_msg(MOVE);
		return 1;
	}
	if(keystate[SDLK_s]){
		send_msg(BMOVE);
		return 1;
	}
	if(keystate[SDLK_q]){
		if(fly){
			send_msg(UP);
			return 1;
		}
	}
	if(keystate[SDLK_e]){
		if(fly){
			send_msg(DOWN);
			return 1;
		}
	}					
	if(keystate[SDLK_SPACE]){
		send_msg(JUMP);
		return 1;
	}
	return 0;	
}

SDL_Event event;
int mover=0;

extern int banvf;

int handle_events()
{
	while (SDL_PollEvent(&event)){	
		SDL_keysym keysym;	
		switch (event.type){
			case SDL_KEYDOWN:
				keysym = event.key.keysym;
				if(escribiendo){
					int len;
					switch (keysym.sym) {
						case SDLK_RETURN:
							escribiendo=0;
							send_txt();
							pj_txtbuf[0]=0;
							SDL_EnableUNICODE(0);							
							break;
						case SDLK_BACKSPACE:
							len=strlen(pj_txtbuf);
							pj_txtbuf[len-1]=0;
							break;
						case SDLK_ESCAPE:
							escribiendo=0;
							pj_txtbuf[0]=0;
							SDL_EnableUNICODE(0);
							break;								
						default:
							if ( (keysym.unicode & 0xFF80) == 0 ) {
								char ch;
								ch = keysym.unicode & 0x7F;
								if(ch){
									len=strlen(pj_txtbuf);
									if(ch==';'){
										pj_txtbuf[len]=',';
									}else{
										pj_txtbuf[len]=ch;
									}
									pj_txtbuf[len+1]=0;
								}
								//printf("ascii %c %d\n",ch,ch);
							}else {
								//printf("An International Character.\n");
							}
							//strcat(pj_txtbuf,SDL_GetKeyName (keysym.sym));
							break;							
					}
				}else{
					switch (keysym.sym) {
					        case SDLK_c:
						  banvf=0;
						  break;
						case SDLK_d:
							send_msg(RMOVE);
							break;
						case SDLK_a:  
							send_msg(LMOVE);
							break;
						case SDLK_w:    
							send_msg(MOVE);
							break;
						case SDLK_s:    
							send_msg(BMOVE);
							break;
						case SDLK_SPACE:
							send_msg(JUMP);								
							break;							
						case SDLK_LEFT:
							rotcam(10,0);
							break;
						case SDLK_RIGHT:
							rotcam(-10,0);
							break;
						case SDLK_UP:
							break;
						case SDLK_DOWN:
							break;
						case SDLK_t:
							escribiendo=1;
							pj_txtbuf[0]=0;
							SDL_EnableUNICODE(1);
							break;
						case SDLK_q:
							if(fly)	send_msg(UP);
							//up
							break;
						case SDLK_e:
							if(fly)	send_msg(DOWN);
							//down
							break;
						case SDLK_1:
							send_msg2(ATTACK,"0");
							break;
						case SDLK_2:
							send_msg2(ATTACK,"1");
							break;							
						case SDLK_f:
							if(fly){
								send_msg(NOFLY);
								fly=0;
							}else{
								send_msg(FLY);									
								fly=1;
							}
							//fly/nofly
							break;	
						case SDLK_ESCAPE:
							salir();
							break;									
					}
				}				
				break;
			case SDL_KEYUP:
				if(!otherkeys()){
					pj->vel=0;
					send_msg(STAND);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(event.button.button ==  1){
					mover=1;						
				}
				if(event.button.button ==  3){
					mover=2;
					cam_ang=pj_ang;	
					rotcam(0,0);						
				}					
				break;					
			case SDL_MOUSEBUTTONUP:
				if(event.button.button ==  1 || event.button.button ==  3){
					mover=0;
					if(!otherkeys())send_msg(STAND);					
				}
			
				if(event.button.button == SDL_BUTTON_WHEELDOWN){
					cam_dist+=10;						
					rotcam(0,0);						
				}
				if(event.button.button == SDL_BUTTON_WHEELUP){
					cam_dist-=10;
					if(cam_dist<10)cam_dist=10;						
					rotcam(0,0);						
				}
				break;				
				
			case SDL_MOUSEMOTION:
				if(mover==2){
					rotcam(event.motion.xrel,event.motion.yrel);
					pj_ang=cam_ang;
					if(lastmsg+BREATH<SDL_GetTicks()){
						otherkeys();
					}						
				}
				if(mover==1){
					rotcam(event.motion.xrel,event.motion.yrel);
				}					
				break;					
			case SDL_QUIT:
				salir();
				break;
		}
	}	
	return 0;	
}
