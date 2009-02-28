#include "client.h"
#include "drawmap.h"
#include "octree.h"
#define WIREFRAME 1
#define SOLID 2
#define FLAT 3
extern struct vector lightAngle;
extern GLuint	shaderTexture[1];	
extern float shaderData[32][3];								

struct texlist{
	GLuint gltex;
	char *name;
	struct texlist *next;
};

extern struct texlist *textures;

float TmpShade;	
struct vector TmpVector, TmpNormal;

struct texlist *findtex(char *name);
GLuint loadtex(char *name)
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
	
	sprintf(aux,"textures/%s.jpg",name);
	
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
void drawpoly(struct poly p,int mode)
{
	int i,j;
	GLuint tex;
	
	if(mode==FLAT){
		if(!p.tex.name){
			TmpNormal=p.normal;
			
			TmpVector=TmpNormal;
			//RotateVector (TmpMatrix, TmpNormal, &TmpVector);
			
			TmpVector=normalize (TmpVector);
			TmpShade = dot (TmpVector, lightAngle);
			if (TmpShade < 0.0f) TmpShade = 0.0f;
			glBegin(GL_POLYGON);
				glTexCoord1f (TmpShade);
				glNormal3f(p.normal.x,p.normal.y,p.normal.z);
				for(i=0;i<p.num;i++){
					glVertex3f(p.vertexes[i].x,p.vertexes[i].y, p.vertexes[i].z);		
				}
			glEnd();	
		}else{
			struct vector centro,p1,p2,pointaux;
			double gmaxdist,dist;
			int gvert;
			double ang,x,y,lp2,gdot;
			float col;
			//int c;
			glDisable( GL_TEXTURE_1D );
			glEnable( GL_TEXTURE_2D ); 
			
			tex=loadtex(p.tex.name);
			
			 TmpNormal=p.normal;
			
			TmpNormal=TmpVector;
			// RotateVector (TmpMatrix, TmpNormal, &TmpVector);
			
			 TmpVector=normalize (TmpVector);
			 TmpShade = dot (TmpVector, lightAngle)*(32-1);
			 if (TmpShade < 0.0f) TmpShade = 0.0f;

			col=shaderData[(int)TmpShade][0];
			glColor3f(col,col,col);
			
			
			centro.x=0;
			centro.y=0;
			centro.z=0;
			for(j=0;j<p.num;j++){
				centro.x+=p.vertexes[j].x;
				centro.y+=p.vertexes[j].y;
				centro.z+=p.vertexes[j].z;
			}
			centro.x/=p.num;
			centro.y/=p.num;
			centro.z/=p.num;
			
			gmaxdist=vectorlen(difvectors(p.vertexes[0],centro));
			gvert=0;
			for(j=1;j<p.num;j++){
				dist=vectorlen(difvectors(p.vertexes[j],centro));
				if(dist>gmaxdist){
					gmaxdist=dist;
					gvert=j;
				}	
			}
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glBindTexture( GL_TEXTURE_2D, tex );
			glNormal3f(p.normal.x,p.normal.y,p.normal.z);
			//printf("gvert:%d\n",gvert);
			p1=difvectors(p.vertexes[gvert],centro);
			glBegin(GL_POLYGON);
				//printvector(p.vertexes[gvert]);printf("pos:0.5 0\n");
				glTexCoord2f(0, 0.5 );
				glVertex3f(p.vertexes[gvert].x,p.vertexes[gvert].y, p.vertexes[gvert].z);
				for(j=(gvert+1)%p.num;j!=gvert;j=(j+1)%p.num){
					p2=difvectors(p.vertexes[j],centro);
					lp2=vectorlen(p2);
					ang=acos(dot(p1,p2)/(gmaxdist*lp2));
					pointaux=cross(p1,p2);
					gdot=dot(pointaux,p.normal);
					if(gdot>-0.01 && gdot< 0.01){
						ang=PI;					
					}else{
						if(gdot<0){
							ang=2*PI-ang;					
						}
					}
					x=0.5+(lp2*sin(ang)*0.5)/gmaxdist;
					y=0.5-(lp2*cos(ang)*0.5)/gmaxdist;
					//printvector(p.vertexes[j]);printf("pos:%f %f\n",x,y);
					glTexCoord2f( y, x );					
					glVertex3f(p.vertexes[j].x,p.vertexes[j].y, p.vertexes[j].z);	
				}
				//printf("\n\n");
			glEnd();
			glDisable(GL_TEXTURE_2D );
			glEnable( GL_TEXTURE_1D );			
		}
	}else{
		glBegin(GL_POLYGON);
		glNormal3f(p.normal.x,p.normal.y,p.normal.z);
		for(i=0;i<p.num;i++){
			 glVertex3f(p.vertexes[i].x,p.vertexes[i].y, p.vertexes[i].z);		
		}
		glEnd();
	}

}

void drawbrush(struct brush *b,int mode)
{
	int i,j;
	for(i=0;i<b->num;i++){
		if(!(b->polys[i].tex.name && !strcmp(b->polys[i].tex.name,"common/caulk"))) 
			drawpoly(b->polys[i],mode);
	}
}

void drawmap(struct map *m,int mode)
{
	struct entitylist *maux;
	struct brushlist *bl;	
	maux=m->entities;
	if(mode==WIREFRAME){
		glLineWidth(3);
		glColor3f(0,0,0);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	if(mode==FLAT){
		glEnable (GL_TEXTURE_1D);									
		glBindTexture (GL_TEXTURE_1D, shaderTexture[0]);			
		glColor3f (0.5f, 0.5f, 0.5f);	
	}
	while(maux){
		bl=maux->ent.brushes;
		while(bl){
			drawbrush(bl->bsh,mode);
			bl=bl->next;	
		}
		maux=maux->next;	
	}
	if(mode==WIREFRAME){
		glLineWidth(1);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
	if(mode==FLAT){	
		glDisable (GL_TEXTURE_1D);
	}
}

void _drawmap(struct map *m)
{
	glPushMatrix();		
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		drawmap(m,FLAT);
		glCullFace(GL_FRONT);
		glEnable (GL_BLEND);									
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		
		glDepthFunc (GL_LEQUAL);	
		drawmap(m,WIREFRAME);
		glDisable (GL_BLEND);	
		glDepthFunc (GL_LESS);
		glCullFace(GL_BACK);	
	glPopMatrix();		
}

void __drawloctree(struct loctree *m,int mode,struct brush *vf)
{
  int i;
  struct brushlist *bl;	
  if(!m||!interaabbbrush(m->box,vf))return; 
  bl=m->brushes;
  while(bl){
    drawbrush(bl->bsh,mode);
    bl=bl->next;
  }
  for(i=0;i<8;i++){
    __drawloctree(m->hijos[i],mode,vf);
  }  
}

void freebrush(struct brush *bsh)
{
  int i,j;
  for(i=0;i<bsh->num;i++){
    if(bsh->polys[i].vertexes){
      free(bsh->polys[i].vertexes);
      bsh->polys[i].vertexes=NULL;
    }
  }
  free(bsh->polys);
  bsh->polys=NULL;
}

int banvf=-1;
struct brush gvf;

extern double farDist,fov, ratio, nearDist;
void _drawloctree(struct loctree *m,int mode)
{
  struct brush vf; //view frustum
  //farDist=1000;
  vf=getviewfrustum();	
  if(mode==WIREFRAME){
    glLineWidth(3);
    glColor3f(0,0,0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  if(mode==FLAT){
    glEnable (GL_TEXTURE_1D);									
    glBindTexture (GL_TEXTURE_1D, shaderTexture[0]);			
    glColor3f (0.5f, 0.5f, 0.5f);	
  }
  __drawloctree(m,mode,&vf);
  if(mode==WIREFRAME){
    glLineWidth(1);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  }
  if(mode==FLAT){	
    glDisable (GL_TEXTURE_1D);
  }
  if(banvf){
    freebrush(&vf);
  }else{
    gvf=vf;
    banvf=1;
  }
}

/*void drawloctree(struct loctree *m)
{
	glPushMatrix();		
	        glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		_drawloctree(m,FLAT);
		//drawmap(m,FLAT);
		glCullFace(GL_FRONT);
		glEnable (GL_BLEND);									
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		
		glDepthFunc (GL_LEQUAL);	
		_drawloctree(m,WIREFRAME);
		//drawmap(m,WIREFRAME);
		glDisable (GL_BLEND);	
		glDepthFunc (GL_LESS);
		glCullFace(GL_BACK);	
	glPopMatrix();		  
	}*/


void drawloctree(struct loctree *m)
{ 

  glPushMatrix();		
    if(banvf==1){
      int i,j;
      glFrontFace(GL_CW);
      glCullFace(GL_BACK);
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      //glDisable(GL_CULL_FACE);
      glColor3f (1, 0, 0);
      for(i=0;i<gvf.num;i++){
	glBegin(GL_POLYGON);
	for(j=0;j<gvf.polys[i].num;j++){
	  glVertex3f(gvf.polys[i].vertexes[j].x,gvf.polys[i].vertexes[j].y, gvf.polys[i].vertexes[j].z);		
	}
	glEnd();	
      }
      //glEnable(GL_CULL_FACE);
    }
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    _drawloctree(m,FLAT);
    glCullFace(GL_FRONT);
    glEnable (GL_BLEND);									
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);		
    glDepthFunc (GL_LEQUAL);	
    _drawloctree(m,WIREFRAME);
    glDisable (GL_BLEND);	
    glDepthFunc (GL_LESS);
    glCullFace(GL_BACK);	
  glPopMatrix();		  
}
