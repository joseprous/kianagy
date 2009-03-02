#include "client.h"

#include <getopt.h>
#include <time.h>
#include "font.h"
#include "network.h"
#include "players.h"
#include "drawmap.h"
#include "events.h"

#include "octree.h"

float WIDTH=500.0;
float HEIGHT=500.0;

int numpl;
int fullscreen=0;
SDL_mutex *mutex;

struct bullet
{
	struct vector p0;	
	struct vector pos;
	struct vector vel;
	GLuint tex;
	int t0,tf;
	int active;
	int ang;
};

struct bullet bala;

typedef struct tagMATRIX // A Structure To Hold An OpenGL Matrix ( NEW )
{
	float Data[16];// We Use [16] Due To OpenGL's Matrix Format ( NEW )
}
MATRIX;

struct player *players;

struct player *pj;

struct loctree *tree;

double curent_time = 0;
double last_time = 0;

double fov, ratio, nearDist, farDist;

float cam_x,cam_z,cam_y,cam_ang,cam_dist;
float pj_ang;
char pj_txtbuf[MESSAGE_MAX_LENGTH+1]="";
int escribiendo=0;

char myname[NAME_MAX_LENGTH+1];
char smap[256]="";
char stmap[256]="";
char sserver[256]="";	

extern Uint32 lastmsg;

GLuint cube_list,redcube_list,bluecube_list,piso_list,map_list,transport_list;

struct vector lightAngle;
GLuint	shaderTexture[1];	
float shaderData[32][3];								

//float TmpShade;// Temporary Shader Value ( NEW )

struct texlist{
	GLuint gltex;
	char *name;
	struct texlist *next;
};

struct texlist *textures=0;

struct texlist *findtex(char *name)
{
	struct texlist *aux;
	aux=textures;
	//printf("0 %s\n",name);
	while(aux){
		//printf("1 %s %s\n",aux->name,name);
		if(!strcmp(aux->name,name)){
			//printf("2 %s %s\n",aux->name,name);
			return aux;	
		}
		aux=aux->next;				
	}
	return 0;		
}
GLuint loadtex2(char *name)
{
	SDL_Surface *surface;	// This surface will tell us the details of the image
	GLenum texture_format;
	GLint  nOfColors;
	char aux[255];
	struct texlist *gtex;
	//printf("entro %s\n",name);
	gtex=findtex(name);
	//printf("1) %s %p\n",name,gtex);
	if(gtex){
		//printf("salio %s %d\n",gtex->name,gtex->gltex);
		return gtex->gltex;
	}
	
	gtex=malloc(sizeof(struct texlist));
	gtex->name=name;
	gtex->next=textures;
	textures=gtex;
	
	sprintf(aux,"textures/%s",name);
	
	if ( (surface = IMG_Load(aux)) ) { 
	
		// Check that the image's width is a power of 2
		if ( (surface->w & (surface->w - 1)) != 0 ) {
			printf("warning: image.bmp's width is not a power of 2\n");
		}
		
		// Also check if the height is a power of 2
		if ( (surface->h & (surface->h - 1)) != 0 ) {
			printf("warning: image.bmp's height is not a power of 2\n");
		}
		
		// get the number of channels in the SDL surface
		nOfColors = surface->format->BytesPerPixel;
		if (nOfColors == 4)    {
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGBA;
			else
				texture_format = GL_BGRA;
		} else 
			if (nOfColors == 3){
				if (surface->format->Rmask == 0x000000ff)
					texture_format = GL_RGB;
				else
					texture_format = GL_BGR;
			} else {
				printf("warning: the image is not truecolor..  this will probably break\n");
				// this error should not go unhandled
			}
		
		// Have OpenGL generate a texture object handle for us
		glGenTextures( 1, &gtex->gltex );
		
		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, gtex->gltex );
		
		// Set the texture's stretching properties
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
		texture_format, GL_UNSIGNED_BYTE, surface->pixels );
	} else {
		printf("SDL could not load %s: %s\n",aux, SDL_GetError());
		SDL_Quit();
		exit(0);
		return 1;
	}    
 
	// Free the SDL_Surface only if it was successfully created
	if ( surface ) { 
		SDL_FreeSurface( surface );
	}
	//printf("salio %s %d\n",gtex->name,gtex->gltex);
	return gtex->gltex;	
}

void ini_celshade()
{
	int i;														
	char line[255];												
	FILE *in	= NULL;	
	glDepthFunc (GL_LESS);
	
	in = fopen ("Data/Shader.txt", "r");
	if (in){
		for (i = 0; i < 32; i++){
			if (feof (in)) break;
			
			fgets (line, 255, in);								
			
			shaderData[i][0] = shaderData[i][1] = shaderData[i][2] = (float)atof(line);
		}
		
		fclose (in);										
	}else{
		printf("no se pudo abrir el archivo Data/Shader.txt\n");
		exit(0);
	}
	
	glGenTextures (1, &shaderTexture[0]);// Get A Free Texture ID ( NEW )
	
	glBindTexture (GL_TEXTURE_1D, shaderTexture[0]);// Bind This Texture. From Now On It Will Be 1D ( NEW )
	
	// For Crying Out Loud Don't Let OpenGL Use Bi/Trilinear Filtering! ( NEW )
	glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
	glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexImage1D (GL_TEXTURE_1D, 0, GL_RGB, 32, 0, GL_RGB , GL_FLOAT, shaderData);	// Upload ( NEW )
	
	lightAngle.x = 0.0f;// Set The X Direction ( NEW )
	lightAngle.y = 1.0f;// Set The Y Direction ( NEW )
	lightAngle.z = 1.0f;// Set The Z Direction ( NEW )
	
	lightAngle=normalize (lightAngle);	
}

void ini_fog()
{
	GLfloat fogColor[4]={1,1,1,1};
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE,GL_EXP);
	glFogfv(GL_FOG_COLOR,fogColor);
	glFogf(GL_FOG_DENSITY,0.0005);
	glFogf(GL_FOG_START,1);
	glFogf(GL_FOG_END,10000);
	glClearColor(1,1,1,1);
}

void init(void)
{
	glViewport(0, 0, (GLsizei) WIDTH, (GLsizei) HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	fov=60;
	ratio=WIDTH/HEIGHT;
	nearDist=1.5;
	farDist=3000;

	gluPerspective(fov, ratio, nearDist, farDist);
	//gluPerspective(60,WIDTH/HEIGHT,1.5,10000);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(1, 1, 1, 0.0);
	//glClearColor(0.0, 0.0, 0.5, 0.0);
	//glShadeModel(GL_SMOOTH);	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	
	//glEnable(GL_BLEND);
	//glEnable(GL_LINE_SMOOTH);
	
	cam_x=-100;
	cam_y=0;
	cam_z=25;//2.5;
	players=malloc(sizeof(struct player));
	pj=players;
	pj->tipo=0;
	pj->x=-5;
	pj->y=0;
	pj->z=0;
	pj->dir=0;
	pj->vel=0;
	pj->next=0;
	pj->txt[0]=0;
	pj->name[0]=0;
	pj->txt_ini_time=0;
	pj->alive=1;
	pj->interp=0;
	pj->n=0;
	pj->weapon=rand()%12;
	pj_ang=0;
	//ini_light();
	cam_dist=100;
	cam_ang=0;
	
	buildFont();
	
	ini_celshade();
	
	ini_fog();
	
	load_models();
		
}



void billboardCheatSphericalBegin() 
{
	
	float modelview[16];
	int i,j;

	// save the current modelview matrix
	glPushMatrix();

	// get the current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

	// undo all rotations
	// beware all scaling is lost as well 
	for( i=0; i<3; i++ ) 
	    for( j=0; j<3; j++ ) {
		if ( i==j )
		    modelview[i*4+j] = 1.0;
		else
		    modelview[i*4+j] = 0.0;
	    }

	// set the modelview with no rotations
	glLoadMatrixf(modelview);
}

void billboardEnd() 
{
	// restore the previously 
	// stored modelview matrix
	glPopMatrix();
}

void createbullet()
{
	bala.p0.x=pj->x;
	bala.p0.y=pj->y;
	bala.p0.z=pj->z+25;

/*	cam_ang = (int)(cam_ang + angulo+360)%360;
	cam_x=-cos(PI*(-cam_ang)/180)*(cam_dist);
	cam_y=-sin(PI*(-cam_ang)/180)*(cam_dist);	
	cam_z+=z;
	*/

	bala.vel.x=cos(PI*(-pj_ang)/180);
	bala.vel.y=sin(PI*(-pj_ang)/180);
	bala.vel.z=0;
	
	
	bala.active=1;
	bala.t0=SDL_GetTicks();
	bala.tf=bala.t0+5000;
	bala.tex=loadtex2("bala.png");
	bala.ang=0;
}

void drawbullet(struct bullet *b)
{
	int t;
	struct vector v0t;
	t=SDL_GetTicks();
	if(t>b->tf){
		b->active=0;
		//return;
	}
	v0t=mulvector((t-b->t0),b->vel);	
	b->pos=sumvectors(b->p0,v0t);
	
	b->ang=(b->ang+50)%360;
	
	glPushMatrix();
		glTranslatef(b->pos.x,b->pos.y,b->pos.z);
		//glTranslatef(pj->x+25,pj->y,pj->z+50);
		glDisable( GL_TEXTURE_1D );
		glEnable( GL_TEXTURE_2D ); 
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		//balatex=loadtex2("bala.png");
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
		glBindTexture( GL_TEXTURE_2D, b->tex );	
		billboardCheatSphericalBegin();
			glRotatef(90,0,1,0);
			glRotatef(b->ang,1,0,0);
			glColor3f(0,0,0);
			glFrontFace(GL_CCW);
			glBegin(GL_POLYGON);
				glTexCoord2f(0,0);
				glVertex3f(0,-16,-16);
				glTexCoord2f(0,1);
				glVertex3f(0,-16,16);
				glTexCoord2f(1,1);
				glVertex3f(0,16,16);
				glTexCoord2f(1,0);
				glVertex3f(0,16,-16);		
			glEnd();
		billboardEnd();	
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDisable( GL_TEXTURE_2D );
		glEnable( GL_TEXTURE_1D ); 			
	glPopMatrix();		
}

struct brush getviewfrustum()
{
  struct brush aux;
  int i;
  struct vector vr,vp,vl,vu,vc,p1,p2;
  double hnear,wnear,hfar,wfar;
  //gluPerspective(fov, ratio, nearDist, farDist);
  //gluLookAt(px,py,pz, lx,ly,lz, ux,uy,uz)

  //gluLookAt(pj->x+cam_x, pj->y+cam_y, pj->z+cam_z, pj->x, pj->y, pj->z, 0.0, 0.0, 1.0);

  vp.x=pj->x+cam_x;
  vp.y=pj->y+cam_y;
  vp.z=pj->z+cam_z;
  vl.x=pj->x;
  vl.y=pj->y;
  vl.z=pj->z;
  vu.x=0;
  vu.y=0;
  vu.z=1;
  
  vr=normalize(difvectors(vl,vp));

  /*  px=pj->x+cam_x;
  py=pj->y+cam_y;
  pz=pj->z+cam_z;
  */
  
  hnear = 2 * tan((fov*(PI/180)) / 2) * nearDist;
  wnear = hnear * ratio;
  hfar = 2 * tan((fov*(PI/180)) / 2) * farDist;
  wfar = hfar * ratio;

  
  /*  printf("vp:");printvector(vp);
  printf("\nvl:");printvector(vl);
  printf("\nvu:");printvector(vu);
  printf("\nvp:");printvector(vr);
  printf("hnear:%lf\nwnear:%lf\nhfar:%lf\nwfar:%lf\n",hnear,wnear,hfar,wfar);
  */

  aux.num=6;
  aux.polys=malloc(sizeof(struct poly)*aux.num);
  for(i=0;i<aux.num;i++){
    aux.polys[i].num=4;
    aux.polys[i].vertexes=malloc(sizeof(struct vector)*4);
    aux.polys[i].tex.name=0;
  }
  //near plane
  //(hn/2)*vu+(wn/2)*vu+vp+nearDist*vr;
  vc=sumvectors(vp,mulvector(nearDist,vr));
  p1=mulvector(wnear/2,normalize(cross(vu,vr)));
  p2=mulvector(hnear/2,normalize(cross(p1,vr)));
  aux.polys[0].vertexes[0]=sumvectors(vc,sumvectors(p1,p2));
  aux.polys[0].vertexes[1]=sumvectors(vc,sumvectors(mulvector(-1,p1),p2));
  aux.polys[0].vertexes[2]=sumvectors(vc,sumvectors(mulvector(-1,p1),mulvector(-1,p2)));
  aux.polys[0].vertexes[3]=sumvectors(vc,sumvectors(p1,mulvector(-1,p2)));
  aux.polys[0].normal=mulvector(-1,vr);

  
  //far plane
  vc=sumvectors(vp,mulvector(farDist,vr));
  p1=mulvector(wfar/2,normalize(cross(vu,vr)));
  p2=mulvector(hfar/2,normalize(cross(p1,vr)));
  aux.polys[1].vertexes[0]=sumvectors(vc,sumvectors(p1,p2));
  aux.polys[1].vertexes[1]=sumvectors(vc,sumvectors(mulvector(-1,p1),p2));
  aux.polys[1].vertexes[2]=sumvectors(vc,sumvectors(mulvector(-1,p1),mulvector(-1,p2)));
  aux.polys[1].vertexes[3]=sumvectors(vc,sumvectors(p1,mulvector(-1,p2)));
  aux.polys[1].normal=vr;
  

  /*  printpoly(aux.polys[1]);

  printf("vp:");printvector(vp);
  printf("\nvc:");printvector(vc);
  printf("\nvr:");printvector(vr);
  printf("\nvl:");printvector(vl);
  printf("\nfov:%lf\nratio:%lf\nnearDist:%lf\nfarDist:%lf\n",fov, ratio, nearDist, farDist);
  printf("\nhnear:%lf\nwnear%lf\nhfar:%lf\nwfar:%lf\n",hnear,wnear,hfar,wfar);
  printf("\n\n");
  */
  
  aux.polys[2].vertexes[0]=aux.polys[0].vertexes[0];
  aux.polys[2].vertexes[1]=aux.polys[1].vertexes[0];
  aux.polys[2].vertexes[2]=aux.polys[1].vertexes[3];
  aux.polys[2].vertexes[3]=aux.polys[0].vertexes[3];
  aux.polys[2].normal=cross(difvectors(aux.polys[2].vertexes[3],aux.polys[2].vertexes[0]),
			    difvectors(aux.polys[2].vertexes[1],aux.polys[2].vertexes[0]));

  aux.polys[3].vertexes[0]=aux.polys[0].vertexes[1];
  aux.polys[3].vertexes[1]=aux.polys[1].vertexes[1];
  aux.polys[3].vertexes[2]=aux.polys[1].vertexes[0];
  aux.polys[3].vertexes[3]=aux.polys[0].vertexes[0];
  aux.polys[3].normal=cross(difvectors(aux.polys[3].vertexes[3],aux.polys[3].vertexes[0]),
			    difvectors(aux.polys[3].vertexes[1],aux.polys[3].vertexes[0]));

  aux.polys[4].vertexes[0]=aux.polys[0].vertexes[2];
  aux.polys[4].vertexes[1]=aux.polys[1].vertexes[2];
  aux.polys[4].vertexes[2]=aux.polys[1].vertexes[1];
  aux.polys[4].vertexes[3]=aux.polys[0].vertexes[1];
  aux.polys[4].normal=cross(difvectors(aux.polys[4].vertexes[3],aux.polys[4].vertexes[0]),
			    difvectors(aux.polys[4].vertexes[1],aux.polys[4].vertexes[0]));

  aux.polys[5].vertexes[0]=aux.polys[0].vertexes[3];
  aux.polys[5].vertexes[1]=aux.polys[1].vertexes[3];
  aux.polys[5].vertexes[2]=aux.polys[1].vertexes[2];
  aux.polys[5].vertexes[3]=aux.polys[0].vertexes[2];
  aux.polys[5].normal=cross(difvectors(aux.polys[5].vertexes[3],aux.polys[5].vertexes[0]),
			    difvectors(aux.polys[5].vertexes[1],aux.polys[5].vertexes[0]));
  
  //printbrush(&aux);
  //exit(0);
  return aux;
}


void display(void)
{

	struct player *aux;
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	last_time = curent_time;
	curent_time = SDL_GetTicks() / 1000.0;
		
//	SDL_mutexP (mutex);
	
		gluLookAt(pj->x+cam_x, pj->y+cam_y, pj->z+cam_z, pj->x, pj->y, pj->z, 0.0, 0.0, 1.0);
	
		drawpj();
	
		aux=players->next;
		while(aux!=NULL){
			drawplayer(aux);
			aux=aux->next;		
		}
				
		//		glCallList(map_list);
		drawloctree (tree);

		
	//	if(bala.active)drawbullet(&bala);
		
		if(escribiendo){
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity();
				gluOrtho2D(	-10,10,-10,10);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glDepthMask(GL_FALSE);
					glColor3f(0,0,0);
					glFrontFace(GL_CW);
					glBegin(GL_POLYGON);
						 glVertex2f(-10.0,  -10.0);
						 glVertex2f(-10.0,  -9.0);
						 glVertex2f(10.0,  -9.0);
						 glVertex2f(10.0,  -10.0);
					glEnd();
					glFrontFace(GL_CCW);				
					glColor3f(1,1,1);
					glRasterPos2i(-10, -10);
					glPrint( "# %s",pj_txtbuf);
				glDepthMask(GL_TRUE);
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}
	
	
	SDL_GL_SwapBuffers ();
//	SDL_mutexV (mutex);
}

void rotcam(float angulo,float z)
{

	cam_ang = (int)(cam_ang + angulo+360)%360;
	cam_x=-cos(PI*(-cam_ang)/180)*(cam_dist);
	cam_y=-sin(PI*(-cam_ang)/180)*(cam_dist);	
	cam_z+=z;

	//printf("ang:%f cx:%f cz:%f pjx:%f pjz:%f\n",pj_ang,cam_x,cam_z,pj->x,pj->z);
}

void init_sdl()
{
	if (SDL_Init (SDL_INIT_VIDEO) != 0){
		printf ("Error: %s\n", SDL_GetError ());
		return;
	}
	atexit (SDL_Quit);		
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);	
	SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 6);
	SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 5);	
	if (SDL_SetVideoMode (WIDTH, HEIGHT, 16, SDL_OPENGL|fullscreen /*|SDL_FULLSCREEN*/) == NULL){
		printf ("Error: %s\n", SDL_GetError ());
		return;
	}	
	mutex = SDL_CreateMutex ();
}


void uso() 
{ 
	printf ("usage: ./client opciones\n"); 
	printf ("-n, --name nombre    \n");
	printf ("-f, --fullscreen    \n");	
	printf ("-s, --server host      \n");
	printf ("-m, --map ruta      \n");	
	printf ("-t, --transport ruta      \n");
	printf ("-h, --height alto      \n");
	printf ("-w, --width ancho      \n");
	exit (0); 
}

int main(int argc, char** argv)
{


	const char* const short_options =  "fh:w:s:n:m:t:"; 
	const struct option long_options[] = { 
		{"width" , 1, NULL,  'w'},
		{"height" , 1, NULL,  'h'},
		{"fullscreen" , 0, NULL,  'f'}, 		
		{"server" , 1, NULL,  's'},
		{"name" , 1, NULL,  'n'},		
		{"map" , 1, NULL,  'm'},
		{"transport" , 1, NULL,  't'},		
		{ NULL, 0, NULL, 0 }
	};

	int opcion; 
	
	Uint32 gticks2;
	int fps=0;
	
	srand(time(0));

	
	do { 
		opcion = getopt_long (argc, argv, short_options, long_options, NULL); 
		switch (opcion) { 
		case 'f':
			fullscreen=SDL_FULLSCREEN;
			break;
		case 's' : 
			strcpy(sserver,optarg); 
			break;
		case 'h' : 
			sscanf(optarg,"%f",&HEIGHT); 
			break;
		case 'w' : 
			sscanf(optarg,"%f",&WIDTH); 
			break;			 
		case 'm' : 
			strcpy(smap,optarg); 
			break;
		case 'n' :
			strcpy(myname,optarg); 			
			break;
		case 't' :
			strcpy(stmap,optarg); 			
			break;		
		case '?' : /*opcion invalida*/
			uso (stderr, 1); 
		case -1: /* no hay mas opciones */ 
			break; 
		default:
			abort (); 
		} 
	} while (opcion != -1);
	
	
	if(!strcmp(sserver,""))strcpy(sserver,"localhost");
	if(!strcmp(smap,"")){
		printf("debe especificar un mapa\n");
		uso();
	}
	if(!strcmp(stmap,"")){
		printf("debe especificar un transporte\n");
		uso();
	}
	if(!strcmp(myname,"")){
		strcpy(myname,"nombre");
	}
	
	init_sdl();
	
	init();
	
	ini_network();

	if(!parsemap(smap))return 0;
	
	printf("cargando loose octree...\n");
	tree=loadloctree(currentmap,0.25,5);
	printf("loose octree cargado\n");

	loctreegenlists(tree,2);
	
	loctreestats(tree);

	/*	map_list=glGenLists(1);
	glNewList(map_list, GL_COMPILE);
	   //_drawmap(currentmap);	
	   drawloctree (tree);
	glEndList();
	*/
	
	if(!parsemap(stmap))return 0;

	transport_list=glGenLists(1);
	glNewList(transport_list, GL_COMPILE);
		_drawmap(currentmap);	
	glEndList();	
	
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	display();
	
	
	gticks2=SDL_GetTicks();
	while (1)
	{
		handle_events();
		display();
		fps++;
		if(SDL_GetTicks()>gticks2+1000){
			printf("fps:%d\n",fps);
			fps=0;
			gticks2=SDL_GetTicks();
		}				
	}
	SDL_Delay (5000);
	return 0;
}
