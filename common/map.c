#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*double fac(int n){
	int i;
	double res=1;
	for(i=2;i<=n;i++)res*=i;	
	return res;
	//if(n==0)return 1;
	//return n*fac(n-1);		
}
double comb(int n,int k)
{
	return (fac(n)/(fac(n-k)*fac(k)));	
	}*/

unsigned int comb(unsigned int m, unsigned int n) {
  unsigned r = 1;
  unsigned d;
  if (n > m) return 0;
  for (d=1; d <= n; d++) {
    r *= m--;
    r /= d;
  }
  return r;
}


void printvector(struct vector v)
{
	printf("<%lf %lf %lf> ",v.x,v.y,v.z);
}

int pointinpoly(struct vector point,struct poly p)
{
	int i,j;
	for(i=0;i<p.num;i++){
		if(comppoints(point,p.vertexes[i]))return 1;	
	}
	return 0;	
}

struct brush *loadbrush(struct rawbrush *b)	
{
	struct brush *aux;
	struct plane *planes;
	struct line lineaux;
	struct vector pointaux,pointaux2,p1,p2,centro;
	int maxvert;
	int i,j,k,c;
	double res,gdot;
	double *angs;
	int ban,gg;	
	double ga;
	int gp1,gp2;
	static int cent=0;
	
	cent++;
	
	//printf("ini loadbrush %d\n",cent);
	aux=malloc(sizeof(struct brush));
	//printf("1\n");
	
	aux->num=b->num;
	aux->polys=malloc(sizeof(struct poly)*b->num);
	//printf("2\n");
	maxvert=(int)comb(b->num-1,2)+1;
	//printf("3\n");
	/**********Ini crear planos ***********/
	planes=malloc(sizeof(struct plane)*b->num);
	for(i=0;i<b->num;i++){
		planes[i]=plane3points(b->planes[i].p[0],b->planes[i].p[1],b->planes[i].p[2]);
		//pointinplane(b->planes[i].p[0],planes[i]);
		//pointinplane(b->planes[i].p[1],planes[i]);
		//pointinplane(b->planes[i].p[2],planes[i]);		
		aux->polys[i].num=0;
		aux->polys[i].vertexes=malloc(sizeof(struct vector)*maxvert);
		aux->polys[i].normal=planes[i].normal;
		aux->polys[i].tex=b->textures[i];		
	}
	/**********Fin crear planos ***********/
//printf("4\n");
	/**********Ini crear vertices ***********/	
	for(i=0;i<b->num;i++){
		for(j=i+1;j<b->num;j++){
			if(inter2planes(planes[i],planes[j],&lineaux)){
				//pointinplane(lineaux.point,planes[i]);
				//pointinplane(lineaux.point,planes[j]);
				for(k=0;k<b->num;k++){
					if(k!=i && k!=j){
						if(interlineplane(lineaux,planes[k],&pointaux)){
							//pointinline(pointaux,lineaux);
							aux->polys[k].vertexes[aux->polys[k].num]=pointaux;
							aux->polys[k].num++;						
						}
					}	
				}
			}
		}	
	}
	/**********Fin crear vertices ***********/
//printf("5\n");
	
	/**********Ini recortar vertices ***********/
	
	for(i=0;i<b->num;i++){
		for(j=0;j<aux->polys[i].num;j++){
			for(k=j+1;k<aux->polys[i].num;k++){
				if(comppoints(aux->polys[i].vertexes[j],aux->polys[i].vertexes[k])){
					for(c=j;c<aux->polys[i].num-1;c++){
						aux->polys[i].vertexes[c]=aux->polys[i].vertexes[c+1];
					}
					aux->polys[i].num--;
					j--;
					break;					
				}
			}			
		}
	}
	
	for(i=0;i<b->num;i++){
		for(j=0;j<aux->polys[i].num;j++){
			for(k=0;k<b->num;k++){
				if(k!=i && pointinplane(aux->polys[i].vertexes[j],planes[k])==1){
					for(c=j;c<aux->polys[i].num-1;c++){
						aux->polys[i].vertexes[c]=aux->polys[i].vertexes[c+1];
					}
					aux->polys[i].num--;
					j--;
					break;
				}
			}
		}
	}
//printf("6\n");	
	/**********Fin recortar vertices ***********/
	
/*	printf("sin ordenar:\n");
	for(i=0;i<b->num;i++){
		for(j=0;j<aux->polys[i].num;j++){
			printvector(aux->polys[i].vertexes[j]);
		}
		printf("\nnormal:");
		printvector(aux->polys[i].normal);
		printf("\n\n");
	}*/

	/**********Ini ordenar ***********/
	angs=malloc(sizeof(double)*maxvert);
	for(i=0;i<b->num;i++){
		centro.x=0;
		centro.y=0;
		centro.z=0;
		for(j=0;j<aux->polys[i].num;j++){
			centro.x+=aux->polys[i].vertexes[j].x;
			centro.y+=aux->polys[i].vertexes[j].y;
			centro.z+=aux->polys[i].vertexes[j].z;
		}
		centro.x/=aux->polys[i].num;
		centro.y/=aux->polys[i].num;
		centro.z/=aux->polys[i].num;
		
		p1=difvectors(aux->polys[i].vertexes[0],centro);
		for(j=1;j<aux->polys[i].num;j++){			
			p2=difvectors(aux->polys[i].vertexes[j],centro);
			angs[j]=acos(dot(p1,p2)/(vectorlen(p1)*vectorlen(p2)));
			pointaux=cross(p1,p2);
			gdot=dot(pointaux,aux->polys[i].normal);
			
			if(gdot>-0.01 && gdot< 0.01){
				angs[j]=PI;					
			}else{
				if(gdot<0){
					angs[j]=2*PI-angs[j];					
				}
			}
		}
		do{
			ban=0;
			for(j=1;j<aux->polys[i].num-1;j++){
				if(angs[j]>angs[j+1]){
					ga=angs[j];
					angs[j]=angs[j+1];
					angs[j+1]=ga;
					pointaux=aux->polys[i].vertexes[j];
					aux->polys[i].vertexes[j]=aux->polys[i].vertexes[j+1];
					aux->polys[i].vertexes[j+1]=pointaux;
					ban=1;
				}	
			}
		}while(ban);	
	}
	
	//printf("7\n");	
	
	free(angs);		
	/**********Fin ordenar ***********/

	//printf("8\n");	
	/*printf("ordenado:\n");
	for(i=0;i<b->num;i++){
		for(j=0;j<aux->polys[i].num;j++){
			printvector(aux->polys[i].vertexes[j]);
			printf("\n");
		}
		printf("\nnormal:");
		printvector(aux->polys[i].normal);
		printf("\n\n");
	}*/
	
	free(planes);
	//printf("fin loadbrush\n");
	return aux;
}
