#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <arpa/inet.h>
#include  <netdb.h>
#include  <errno.h>
#include "client.h"
#include "players.h"
#include "network.h"

Uint32 lastmsg;
int sockfd;

extern SDL_mutex *mutex;
extern float pj_ang;
extern char pj_txtbuf[MESSAGE_MAX_LENGTH+1];
extern struct player *pj;
extern char myname[NAME_MAX_LENGTH+1];
extern struct player *players;
extern char sserver[256];

void send_msg(int code)
{
	char msg[20];
	Uint32 t=SDL_GetTicks();
	if(lastmsg+BREATH>=t){
		SDL_Delay(t-lastmsg+BREATH);
	}
	sprintf(msg,"%d %f ",code,360-pj_ang);
	//printf("msg:%s\n",msg);
	if (send(sockfd, msg, strlen(msg), 0) == -1) perror("send");
	lastmsg=SDL_GetTicks();
}
void send_msg2(int code,char *txt)
{
	char msg[20];
	Uint32 t=SDL_GetTicks();
	if(lastmsg+BREATH>=t){
		SDL_Delay(t-lastmsg+BREATH);
	}
	sprintf(msg,"%d %f %s..",code,360-pj_ang,txt);
	//printf("msg:%s\n",msg);
	if (send(sockfd, msg, strlen(msg), 0) == -1) perror("send");
	lastmsg=SDL_GetTicks();
}
void send_txt()
{
	char msg[MESSAGE_MAX_LENGTH];
	sprintf(msg,"%d %f %s..",MESSAGE,pj_ang,pj_txtbuf);
	printf("msg:%s\n",msg);
	if (send(sockfd, msg, strlen(msg), 0) == -1) perror("send");		
}

void send_name()
{
	char msg[200];

	//printf("name:");
	//scanf("%s",buf);
	sprintf(msg,"%d 0 %s..",SETNAME,myname);
	if (send(sockfd, msg, strlen(msg), 0) == -1) perror("send");	
}
int count(struct player *p)
{
	if(p==0)return 0;
	return 1+count(p->next);
}
int buscar_player(int tipo,int id,char *name,int hp,float x,float y,float z,int acc,float ang,char *txt)
{
	struct player *aux;
	aux=players;
	while(aux!=NULL){
		if(aux->id==id){
			SDL_mutexP (mutex);
				//printf("nombre:%s tipo:%d\n",name,tipo);
				aux->tipo=tipo;
				strcpy(aux->name,name);
				aux->hp=hp;	
				aux->x=x;
				aux->y=y;
				aux->z=z;			
				aux->acc=acc;
				if(acc==FLY){
					aux->model=1;	
				}else{
					aux->model=2;
				}
				aux->dir=ang;
				aux->vel=0;
				if(txt[0]){
					strcpy(aux->txt,txt);
					aux->txt_ini_time=SDL_GetTicks();
				}
				aux->alive=1;				
			SDL_mutexV (mutex);
			return 1;
		}
		aux=aux->next;
	}	
	return 0;	
}
int network(void *data)
{
	char buf[1024],buf2[1024],txt[MESSAGE_MAX_LENGTH],gname[NAME_MAX_LENGTH];
	char curbuf[1024]="";
	struct player *aux,*aux2;
	char *pbuf;
	int i,j,k,c,i2;
	static int c1=0;
	int numbytes;
	int gid,gacc,gtipo=3;
	float gx,gy,gz,gang;
	int ghp;
	while(1){
		if ((numbytes=recv(sockfd, buf, 1024-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buf[numbytes] = '\0';
		
		if(!numbytes)continue;
		//printf("buf:%s\n\n",buf);
		
		for(i2=0;i2<numbytes;i2++){
			if(buf[i2]!='\n'){
				curbuf[c1]=buf[i2];
				c1++;	
			}else{
				curbuf[c1]=0;
				//printf("curbuf:%s\n",curbuf);
				if(curbuf[0]=='<' && curbuf[1]=='0' && curbuf[2]=='>' ){
					j=0;
					//SDL_mutexP (mutex);
					for(i=3;i<c1;i++){
						if(curbuf[i]!=';'){
							buf2[j]=curbuf[i];
							j++;
						}else{
							buf2[j]=0;
							pbuf=buf2;
		
							sscanf(pbuf,"%d,",&gtipo);
							while(pbuf[0]!=',')pbuf++;
							pbuf++;				
							
							sscanf(pbuf,"%d,",&gid);
							while(pbuf[0]!=',')pbuf++;
							pbuf++;
		
							k=0;
							while(pbuf[0]!=','){
								gname[k]=pbuf[0];
								pbuf++;
								k++;
							}
							gname[k]=0;
							pbuf++;					
							sscanf(pbuf,"%d,%f,%f,%f,%d,%f",&ghp,&gx,&gy,&gz,&gacc,&gang);					
							c=0;
							for(k=0;k<6;k++){
								while(pbuf[c]!=',')c++;
								c++;
							}
							strcpy(txt,pbuf+c);
							//SDL_mutexP (mutex);
							if(!buscar_player(gtipo,gid,gname,ghp,gx,gy,gz,gacc,gang,txt)){
								aux=players;
								while(aux->next!=NULL){
									aux=aux->next;
								}
								SDL_mutexP (mutex);
									aux->next=malloc(sizeof(struct player));
									aux->next->tipo=gtipo;
									aux->next->id=gid;
									aux->next->hp=ghp;
									aux->next->x=gx;
									aux->next->y=gy;
									aux->next->z=gz;
									aux->next->acc=gacc;	
									if(gacc==FLY){
										aux->next->model=1;	
									}else{
										aux->next->model=2;
									}
									aux->next->weapon=rand()%12;																								
									if(txt[0]){
										strcpy(aux->txt,txt);
										aux->txt_ini_time=SDL_GetTicks();
									}else{	
										aux->next->txt[0]=0;
										aux->next->txt_ini_time=0;
									}
									strcpy(aux->name,gname);
									aux->next->dir=gang;
									aux->next->vel=0;
									aux->next->next=0;
									aux->next->alive=1;
									aux->interp=0;
								SDL_mutexV (mutex);
							}
							//SDL_mutexV (mutex);
							j=0;	
						}	
					}
					
					aux=players;
					while(aux->next!=NULL){
						if(!aux->next->alive){
							aux2=aux->next->next;
							free(aux->next);
							aux->next=aux2;
						}else{
							aux->next->alive=0;
							aux=aux->next;
						}
					}			
					//SDL_mutexV (mutex);
				}else{
					printf("Error %s\n",curbuf);
				}
				c1=0;
			}			
		}
		
		
	}

}

void ini_network()
{
	SDL_Thread *thread_network;
	int numbytes;
	char buf[256];
	struct hostent *he;
	struct sockaddr_in their_addr; // connectorÃ¢ÂÂs address information
	
	//printf("servidor:");
	//scanf("%s",buf);	
	
	if ((he=gethostbyname(sserver)) == NULL) {  // get the host info
		perror("gethostbyname");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	their_addr.sin_family = AF_INET;    // host byte order
	their_addr.sin_port = htons(PORT); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	memset(&(their_addr.sin_zero), '\0', 8); // zero the rest of the struct
	if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
	
	if ((numbytes=recv(sockfd, buf, 256-1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	buf[numbytes] = '\0';
	printf("Received: %s",buf);
	
	sscanf(buf,"wellcome:%d",&(pj->id));
	
	send_name();
	
	thread_network = SDL_CreateThread (network, NULL);		
}
void end_network()
{
	close(sockfd);	
}
