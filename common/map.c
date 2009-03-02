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
	int i;
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
	struct vector pointaux;
	int maxvert;
	int i,j,k,c;

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

	for(i=0;i<b->num;i++){
	  ordervertexes(&(aux->polys[i]));
	}
	
	//printf("7\n");	
	
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
