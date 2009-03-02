#include "client.h"
#include "players.h"

extern GLuint transport_list;
extern struct player *pj;
extern float pj_ang;	

int n = 0; 
float interp = 0.0;
extern double curent_time;
extern double last_time;

struct player_model models[3];
struct md2_model_t weapons[20];

void load_weapons()
{
	if (!ReadMD2Model ("models/pknight/w_sshotgun.md2", &weapons[0])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_rlauncher.md2", &weapons[1])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/a_grenades.md2", &weapons[2])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_bfg.md2", &weapons[3])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_chaingun.md2", &weapons[4])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_glauncher.md2", &weapons[5])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_hyperblaster.md2", &weapons[6])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_railgun.md2", &weapons[7])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_shotgun.md2", &weapons[8])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_blaster.md2", &weapons[9])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_grapple.md2", &weapons[10])) exit (EXIT_FAILURE);
	if (!ReadMD2Model ("models/pknight/w_machinegun.md2", &weapons[11])) exit (EXIT_FAILURE);
}

void load_models()
{
	if (!ReadMD2Model ("models/Ogros.md2", &models[0].md2file)) exit (EXIT_FAILURE);
	models[0].fps=10;
	models[0].iniframes[MOVE]=40;
	models[0].endframes[MOVE]=45;
	models[0].iniframes[LMOVE]=40;
	models[0].endframes[LMOVE]=45;
	models[0].iniframes[RMOVE]=40;
	models[0].endframes[RMOVE]=45;
	models[0].iniframes[BMOVE]=40;
	models[0].endframes[BMOVE]=45;
	models[0].iniframes[STAND]=0;
	models[0].endframes[STAND]=39;	
	models[0].iniframes[ATTACK]=0;
	models[0].endframes[ATTACK]=39;	
	models[0].hasweapon=0;

	if (!ReadMD2Model ("models/pknight/tris.md2", &models[2].md2file)) exit (EXIT_FAILURE);
	//if (!ReadMD2Model ("models/pknight/w_sshotgun.md2", &models[2].weapon)) exit (EXIT_FAILURE);
	//if (!ReadMD2Model ("models/pknight/w_rlauncher.md2", &models[2].weapon)) exit (EXIT_FAILURE);
	models[2].fps=10;
	models[2].iniframes[MOVE]=40;
	models[2].endframes[MOVE]=45;
	models[2].iniframes[LMOVE]=40;
	models[2].endframes[LMOVE]=45;
	models[2].iniframes[RMOVE]=40;
	models[2].endframes[RMOVE]=45;
	models[2].iniframes[BMOVE]=40;
	models[2].endframes[BMOVE]=45;
	models[2].iniframes[STAND]=0;
	models[2].endframes[STAND]=39;	
	models[2].iniframes[ATTACK]=46;
	models[2].endframes[ATTACK]=53;	
	models[2].hasweapon=1;
		
	if (!ReadMD2Model ("models/Gargoyle.md2", &models[1].md2file)) exit (EXIT_FAILURE);
	models[1].fps=30;
	models[1].iniframes[FLY]=20;
	models[1].endframes[FLY]=34;
	models[1].hasweapon=0;
		
	load_weapons();	
}

void drawplayer(struct player *p)
{
	glPushMatrix();
		glTranslatef(p->x,p->y,p->z);
		glPushMatrix();
			glRotatef(p->dir,0,0,1);
			
			if(p->tipo==0){
				glTranslatef(0,0,8);
				glFrontFace(GL_CW);

				p->interp+=models[p->model].fps * (curent_time - last_time);
				Animate (models[p->model].iniframes[p->acc], models[p->model].endframes[p->acc], &p->n, &p->interp);
				RenderFrameItpWithGLCmds (p->n, p->interp, &models[p->model].md2file,p->dir);
				if(models[p->model].hasweapon){					
					RenderFrameItpWithGLCmds (p->n, p->interp, &weapons[p->weapon],p->dir);
				}	
			}else{
				glCallList(transport_list);	
			}
		glPopMatrix();				
		if(p->tipo==0){	
			glColor3f(0,0,0);
			glPushMatrix();
				glDisable(GL_FOG);
				glRasterPos3f( 0,0, 40 );
				glPrint( "(%d) %s",p->hp,p->name);
				if(SDL_GetTicks()<=p->txt_ini_time+TXT_MAXTIME){
					glRasterPos3f( 0,0,50);
					glPrint( "%s",p->txt);
				}	
				glEnable(GL_FOG);
			glPopMatrix();	
		}			
	glPopMatrix();		
}

void drawpj()
{
		glPushMatrix();
			glTranslatef(pj->x,pj->y,pj->z);
	
			glPushMatrix();
				glRotatef(-pj_ang,0,0,1);
				glTranslatef(0,0,8);
				glFrontFace(GL_CW);

				pj->interp+=models[pj->model].fps * (curent_time - last_time);
				Animate (models[pj->model].iniframes[pj->acc], models[pj->model].endframes[pj->acc], &pj->n, &pj->interp);
				RenderFrameItpWithGLCmds (pj->n, pj->interp, &models[pj->model].md2file,-pj_ang);	
				if(models[pj->model].hasweapon){					
					RenderFrameItpWithGLCmds (pj->n, pj->interp, &weapons[pj->weapon],-pj_ang);
				}							
			glPopMatrix();
			glColor3f(0,0,0);
			glPushMatrix();
				glRasterPos3f( 0,0, 40);
				glPrint( "(%d) %s",pj->hp,pj->name);
				if(SDL_GetTicks()<=pj->txt_ini_time+TXT_MAXTIME){
					glRasterPos3f( 0,0, 50 );
					glPrint( "%s",pj->txt);
				}
			glPopMatrix();
		glPopMatrix();		
}
