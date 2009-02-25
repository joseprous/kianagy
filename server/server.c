#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../common/map.h"
#include "../common/collisionsys.h"

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/tcp.h>

#include "defines.h"
#include "server.h"
#include "combatsys.h"
#include "npcsys.h"

struct player *CreatePlayer(int id,float x,float y,float z,char *name,struct WorldObject *object)
{
	struct player *new;
	
	new = malloc(sizeof(struct player));
	new->id = id;
	new->actual.x = x;
	new->actual.y = y;
	new->actual.z = z;
	new->action = STAND;
	new->angle = FALSE;
	new->impulse = FALSE;
	strcpy(new->message,"");
	strcpy(new->name,name);
	new->jumpheight = FALSE;
	new->fly = FALSE;
	new->ontransport = FALSE;
	new->vitality = START_VITALITY;
	memset(new->colddowns,0,sizeof(int)*MAX_ATTACKS);
	new->hurt = FALSE;
	new->last_attacker = NULL;
	new->inobject = object;
	
	return new;
}

void DestroyPlayer(struct player *this)
{
	free(this);
}

struct transporter *CreateTransporter(int id,struct pathnode *targetlist, float radius, char *filename)
{
	struct transporter *new;
	new = malloc(sizeof(struct transporter));
	new->id = id;
	new->actual.x = targetlist->actual.x;
	new->actual.y = targetlist->actual.y;
	new->actual.z = targetlist->actual.z;
	new->targetlist = targetlist;
	new->nexttarget = targetlist->next;
	new->radius = radius;
	new->wait = new->nexttarget->wait;
	new->vector = UnitVector(&new->actual,&new->nexttarget->actual);
	new->next = NULL;
	new->object = ObjectFromFile(filename);
	return new;
}

void LoadTransporters(char *filename)
{
	char buffer[BUFFSIZE];
	char header[BUFFSIZE];
	char objectfile[BUFFSIZE];
	FILE *input;

	struct pathnode *root,*node,*lastnode;
	struct transporter *tp,*lasttp;
	int firstpath,firsttp,wait,radius,id;
	float x,y,z;

	if ((input = fopen(filename,"r"))==NULL)
	{
		printf("Error while loading transports.\n");
		exit(ERROR);
	}
	
	tp = lasttp = NULL;
	root = node = lastnode = NULL;
	firsttp = firstpath = TRUE;
	while(!feof(input))
	{
		fgets(buffer,BUFFSIZE,input);
		sscanf(buffer,"%s ",header);
		if (strcmp("PATHNODE=",header)==0)
		{
			sscanf(buffer,"%s %f %f %f %d",header,&x,&y,&z,&wait);
			node = CreatePathNode(x,y,z,NULL,wait);
			if (firstpath==TRUE){root = node; firstpath = FALSE;}
			if (lastnode != NULL){lastnode->next = node;}
			lastnode = node;
			//printf("%s %f %f %f %d\n",header,x,y,z,wait);
		}
		else
		{
			lastnode->next = root;
			sscanf(buffer,"%s %d %d %s",header,&id,&radius,objectfile);
			tp = CreateTransporter(id,root,radius,objectfile);
			if (firsttp==TRUE){bus = tp;firsttp=FALSE;}
			if (lasttp != NULL){lasttp->next = tp;}
			lasttp = tp;
			root = node = lastnode = NULL;
			firstpath = TRUE;
			printf("*Successfully loaded transporter with %s.\n",objectfile);
		}
	}
	fclose(input);
}

struct pathnode *CreatePathNode(float x,float y,float z,struct pathnode *next,int wait)
{
	struct pathnode *new;
	new = malloc(sizeof(struct pathnode));
	new->actual.x = x;
	new->actual.y = y;
	new->actual.z = z;
	new->next = next;
	new->wait = wait;
	return new;
}

void DestroyPathNodes(struct pathnode *root)
{
	struct pathnode *aux,*next;
	aux = root;
	while(aux->next!=root)
	{
		next = aux->next;
		free(aux);
		aux=next;
	}
	free(aux);
}

void DestroyTransporters(struct transporter *root)
{
	struct transporter *aux,*next;
	aux = root;
	while(aux!=NULL)
	{
		next = aux->next;
		DestroyPathNodes(aux->targetlist);
		DestroyWorldObject(aux->object);
		free(aux);
		aux = next;
	}
}

float Distance2p(struct position *origin, struct position *target)
{
	float x,y,z;

	x = target->x - origin->x;
	y = target->y - origin->y;
	z = target->z - origin->z;

	return sqrtf((x*x)+(y*y)+(z*z));
}

int InRange(struct position *origin, struct position *target, int radius)
{
	if ( Distance2p(origin,target) <= radius) return TRUE;
	return FALSE;
}

struct position UnitVector(struct position *origin, struct position *destination)
{
	float r;
	struct position vector;
	
	vector.x = destination->x - origin->x;
	vector.y = destination->y - origin->y;
	vector.z = destination->z - origin->z;

	r = Distance2p(origin,destination);

	vector.x = vector.x/r;
	vector.y = vector.y/r;
	vector.z = vector.z/r;
	
	return vector;
}

int InObject(struct WorldObject *object, struct position *that, struct position *him)
{
	if ((him->x < that->x)||(him->x >= (that->x+((object->size.x)*(SCALE_X-1))))) return FALSE;
	if ((him->y < that->y)||(him->y >= (that->y+((object->size.y)*(SCALE_Y-1))))) return FALSE;
	if ((him->z < that->z)||(him->z >= (that->z+((object->size.z)*(SCALE_Z-1))))) return FALSE;
	return TRUE;
}

struct position MapToObject(struct position *that, struct position *him)
{
	struct position new;

	new.x = him->x - that->x;
	new.y = him->y - that->y;
	new.z = him->z - that->z;

	return new;
}

struct position ObjectToMap(struct position *that, struct position *him)
{
	struct position new;

	new.x = that->x + him->x;
	new.y = that->y + him->y;
	new.z = that->z + him->z;
	
	return new;
}

void InTransport(struct transporter *this,struct player *him)
{
	struct position faux;
	//struct Coordinate iaux;
	//printf("ZERO POS: X: %f, Y: %f, Z: %f\n",him->actual.x,him->actual.y,him->actual.z);
	if (InObject(this->object,&this->actual,&him->actual))
	{
		//printf("INOBJECT %d\n",this->object);
		faux = MapToObject(&this->actual,&him->actual);
		//iaux = PosToCoor(faux.x,faux.y,faux.z);
		//printf("ZERO COOR: X: %d, Y: %d, Z: %d\n",iaux.x,iaux.y,iaux.z);
		him->impulse = FALSE;
		/*if (BlockAvailable(&iaux,this->object)==FALSE)
		{
			him->actual = him->last;
			return;
		}*/
		him->actual = faux;
		him->ontransport = this->id;
		him->inobject = this->object;
	}
}

void OutTransport(struct transporter *that,struct player *him)
{
	him->actual = ObjectToMap(&that->actual,&him->actual);
	him->inobject = map;
	him->ontransport = FALSE;
	him->impulse = FALSE;
}

struct transporter *GetTransport(int id)
{
	struct transporter *aux;
	aux = bus;
	while(aux!=NULL)
	{
		if (aux->id == id) return aux;
		aux = aux->next;
	}
	return NULL;
}

struct position TransportPos(struct transporter *this)
{
	struct position new;	
	new.x = 0;
	new.y = 0;
	new.z = 0;
	if (this != NULL){return this->actual;}
	return new;
}

struct node *Register(struct client *this, struct player *me)
{
	struct node *new;
	new = malloc(sizeof(struct node));
	new->that = this;
	new->him = me;
	pthread_mutex_lock(&mutex);
		new->previous = &root;
		new->next = root.next;
		if (root.next!=NULL){root.next->previous = new;}
		root.next = new;
	pthread_mutex_unlock(&mutex);
	return new;
}

void UnRegister(struct node *mine)
{
	pthread_mutex_lock(&mutex);
		mine->previous->next = mine->next;
		if (mine->next!=NULL){mine->next->previous = mine->previous;}
	pthread_mutex_unlock(&mutex);
}

void ShowRegister()
{
	struct node *aux;
	aux = root.next;
	pthread_mutex_lock(&mutex);
		while(aux!=NULL)
		{
			printf("[%id] ->",aux->him->id);
			aux = aux->next;
		}
	pthread_mutex_unlock(&mutex);
	printf(" NULL \n");
}

void Broadcast(char *message)
{
	struct node *aux;
	pthread_mutex_lock(&mutex);
		aux = root.next;
		while(aux!=NULL)
		{
			if (aux->that!=NULL)
			{
				if (send(aux->that->socket,message,strlen(message),0) == ERROR)
				{
					perror("send");
				}
			}
			aux = aux->next;
		}
	pthread_mutex_unlock(&mutex);
}

void GetMessage(char *buffer,struct player *me)
{
	int i = 0;
	int j = 0;
	char aux[BUFFSIZE];

	while(buffer[i]!=32){aux[i] = buffer[i++];}
	aux[i] = '\0';
	me->action = atoi(aux);
	i++;
	while(buffer[i]!=32){aux[j++] = buffer[i++];}
	aux[j] = '\0';
	me->angle = atof(aux);
	if ( me->action == MESSAGE || me->action == SETNAME || me->action == ATTACK)
	{
		j=0;
		while(buffer[++i]!='\0' && j < MESSAGE_MAX_LENGTH){me->message[j++] = buffer[i];}
		me->message[j] = '\0';
	}
}

void ClientHandler(void *ptr)
{
	struct node *mine;
	int action;

	struct client *this;
	this = (struct client*)ptr;

	int numbytes;
	char buffer[BUFFSIZE];

	struct player *me;
	/*me = malloc(sizeof(struct player));
	
	me->id = (int)pthread_self();
	me->actual.x = map->start.x;
	me->actual.y = map->start.y;
	me->actual.z = map->start.z;
	me->action = STAND;
	me->angle = 0;
	me->impulse = FALSE;
	strcpy(me->message,"");
	strcpy(me->name,"");
	me->jumpheight = 0;
	me->fly = FALSE;
	me->ontransport = FALSE;
	me->inobject = map;
	//me->last = me->actual;*/

	me = CreatePlayer((int)pthread_self(),map->start.x,map->start.y,map->start.z,"",map);

	mine = Register(this,me);

	sprintf(buffer,"wellcome:%d\n",me->id);
	if (send(this->socket,buffer,strlen(buffer),0) == ERROR)
	{
		perror("send");
		me->action = EXIT;
	}

	while(me->action-EXIT)
	{
		if ((numbytes = recv(this->socket,buffer,BUFFSIZE-1,0))==ERROR)
		{
			perror("recv");
			break;
		}
		if (numbytes < 4){me->action = EXIT; continue;}
		buffer[numbytes-2] = '\0';
		//printf("%d sent %s from %s, %d bytes\n",me->id,buffer,inet_ntoa(this->address.sin_addr),numbytes);
		GetMessage(buffer,me);
		usleep(BREATH);
	}
	close(this->socket);
	printf("Player %d has been disconected\n",me->id);
	free(this);
	free(me);
	UnRegister(mine);
	free(mine);
	pthread_exit(0);
}

int EqualsPos(struct position *origin, struct position *destination)
{
	if (origin->x != destination->x ) return FALSE;
	if (origin->y != destination->y ) return FALSE;
	if (origin->z != destination->z ) return FALSE;
	return TRUE;
}

int PosModified(struct player *him)
{
	/*struct Coordinate actual;
	actual = PosToCoor(him->actual.x,him->actual.y,him->actual.z);
	if (him->inobject!=map && !InMatrix(&actual,him->inobject))
	{
		OutTransport(GetTransport(him->ontransport),him);
	}*/
	if (him->inobject!=map)
	{
		struct transporter *this = GetTransport(him->ontransport);
		struct position actual = ObjectToMap(&this->actual,&him->actual);
		if(!InObject(him->inobject,&this->actual,&actual))
		{
			//printf("IS OUT!!!\n");
			OutTransport(this,him);
			return TRUE;
		}
	}
	return FALSE;
}

void RealmHandler()
{
	struct node *aux,*aux2;
	char state[BUFFSIZE],buffer[BUFFSIZE];
	struct position fnext;
	struct Coordinate actual, next;
	float ground,height;
	int collision,lastaction,atk;
	struct transporter *taux;
	struct Npc *npc_aux;

	while(TRUE)
	{
		usleep(TICKS);
		strcpy(state,"<0>");

		NpcHandler(npc_root);

		taux = bus;
		while(taux!=NULL)
		{
			if (taux->wait <= 0)
			{
	
				if (InRange(&taux->actual,&taux->nexttarget->actual,NODERADIUS)==TRUE)
				{
					//taux->actual = taux->nexttarget->actual;
					taux->vector = UnitVector(&taux->actual,&taux->nexttarget->next->actual);
					taux->wait = taux->nexttarget->wait;
					taux->nexttarget = taux->nexttarget->next;
					//printf("CHECKPOINT X: %f, Y: %f, Z: %f\n",taux->vector.x,taux->vector.y,taux->vector.z);
				}
				else
				{
					taux->actual.x += taux->vector.x*TRANSSPEED;
					taux->actual.y += taux->vector.y*TRANSSPEED;
					taux->actual.z += taux->vector.z*TRANSSPEED;
				}
			}
			else
			{
				taux->wait-=TICKS;
			}

			sprintf(buffer,"1,%d,transporter,0,%f,%f,%f,8,0,;",taux->id,taux->actual.x,taux->actual.y,taux->actual.z);
			strcat(state,buffer);

			taux = taux->next;
		}


		pthread_mutex_lock(&mutex);
		aux = root.next;
		while(aux!=NULL)
		{
			if (aux->him->ontransport==FALSE)
			{
				taux = bus;
				while(taux!=NULL)
				{
					InTransport(taux,aux->him);
					taux = taux->next;
				}
			}

			if (aux->him->action == MESSAGE)
			{
				npc_aux = npc_root;
				while(npc_aux!=NULL)
				{
					if (npc_aux->him!=aux->him){NpcDoChat(npc_aux,aux->him);}
					npc_aux = npc_aux->next;
				}
			}

			if (aux->him->action == ATTACK && aux->him->colddowns[atoi(aux->him->message)]==FALSE)
			{
				aux2 = root.next;
				while(aux2!=NULL)
				{
					//printf("Entro aca..\n");
					if (aux!=aux2){DoAttack(aux->him,aux2->him,attacks[atoi(aux->him->message)]);}
					aux2 = aux2->next;
				}
				strcpy(aux->him->message,"");
				//printf("Salio de aca.\n");
			}

			for(atk=0;atk<MAX_ATTACKS;atk++)
			{
				aux->him->colddowns[atk] -= TICKS;
				if (aux->him->colddowns[atk]<0){aux->him->colddowns[atk]=FALSE;}
			}

			//aux->him->last = aux->him->actual;
			actual = PosToCoor(aux->him->actual.x,aux->him->actual.y,aux->him->actual.z);
			ground = (GetGround(&actual,aux->him->inobject))*SCALE_Z;
			height = GetHeight(&actual,aux->him->inobject)*SCALE_Z;
			lastaction = aux->him->action;
			//printf("FIRST: X: %f, Y: %f, Z: %f, G: %f, H: %f\n",aux->him->actual.x,aux->him->actual.y,aux->him->actual.z,ground,height);

			if (aux->him->action >= LMOVE && (ground == aux->him->actual.z || aux->him->impulse == TRUE || aux->him->fly == TRUE))
			{
				fnext.x = (aux->him->actual.x+cos(((aux->him->angle+aux->him->action)/180)*PI)*(SPEED+(aux->him->fly*FLYSPEED)));
				fnext.y = (aux->him->actual.y+sin(((aux->him->angle+aux->him->action)/180)*PI)*(SPEED+(aux->him->fly*FLYSPEED)));
				fnext.z = aux->him->actual.z;

				next = PosToCoor(fnext.x,fnext.y,fnext.z);

				collision = Collision(&actual,&next,aux->him->inobject);
				if (collision != FALSE && !(aux->him->inobject==map && collision==OUTOFMAP))
				{
					aux->him->actual = fnext;
					if (collision!=TRUE && collision!=OUTOFMAP){ground = aux->him->actual.z = (collision*SPECIAL)*SCALE_Z;}
					actual = next;

					if (PosModified(aux->him))
					{
						actual = PosToCoor(aux->him->actual.x,aux->him->actual.y,aux->him->actual.z);
						ground = GetGround(&actual,aux->him->inobject)*SCALE_Z;
						height = GetHeight(&actual,aux->him->inobject)*SCALE_Z;
					}
				}
			}	
			//printf("SECOND: X: %f, Y: %f, Z: %f, G: %f, H: %f\n",aux->him->actual.x,aux->him->actual.y,aux->him->actual.z,ground,height);

			if (aux->him->action == FLY){aux->him->fly = TRUE;}

			if (aux->him->action == NOFLY){aux->him->fly = FALSE;}

			if (aux->him->fly == TRUE)
			{
				lastaction = FLY;
				if (aux->him->action == UP)
				{
					if ((aux->him->actual.z+PLAYER_SIZE_Z+FLYSPEED < height) || (aux->him->inobject != map && height == (aux->him->inobject->size.z-1)*SCALE_Z))
					{
						aux->him->actual.z+=FLYSPEED;
						if (PosModified(aux->him))
						{
							actual = PosToCoor(aux->him->actual.x,aux->him->actual.y,aux->him->actual.z);
							ground = GetGround(&actual,aux->him->inobject)*SCALE_Z;
							height = GetHeight(&actual,aux->him->inobject)*SCALE_Z;
						}
					}
				}

				if (aux->him->action == DOWN)
				{
					if ((aux->him->actual.z-FLYSPEED > ground) || (aux->him->inobject != map && ground == FALSE))
					{
						aux->him->actual.z-=FLYSPEED;
						if (PosModified(aux->him))
						{
							actual = PosToCoor(aux->him->actual.x,aux->him->actual.y,aux->him->actual.z);
							ground = GetGround(&actual,aux->him->inobject)*SCALE_Z;
							height = GetHeight(&actual,aux->him->inobject)*SCALE_Z;
						}
					}
				}
			}

			if (aux->him->impulse == TRUE)
			{
				if (aux->him->actual.z > aux->him->jumpheight || aux->him->actual.z+PLAYER_SIZE_Z > height)
				{
					aux->him->impulse = FALSE;
				}
				else
				{
					aux->him->actual.z = aux->him->actual.z + IMPULSE;
				}
			}

			if (aux->him->action == JUMP && aux->him->actual.z==ground && aux->him->fly==FALSE)
			{
				aux->him->impulse = TRUE;
				aux->him->jumpheight = aux->him->actual.z + JUMPHEIGH;
			}

			if (aux->him->action == SETNAME)
			{
				strcpy(aux->him->name,aux->him->message); 
				strcpy(aux->him->message,"");
				aux->him->action = STAND;
			}
			
			//printf("OTP: %d\n",aux->him->ontransport);
			if ((aux->him->actual.z > ground || ground == FALSE) && aux->him->fly==FALSE)
			{
				//printf("1. %f > %f?\n",aux->him->actual.z,ground);
				aux->him->actual.z = aux->him->actual.z - GRAVITY;
				if (PosModified(aux->him))
				{
					actual = PosToCoor(aux->him->actual.x,aux->him->actual.y,aux->him->actual.z);
					ground = GetGround(&actual,aux->him->inobject)*SCALE_Z;
					height = GetHeight(&actual,aux->him->inobject)*SCALE_Z;
				}
				//printf("2. %f %d\n",aux->him->actual.z,ground);

				if (aux->him->actual.z < ground && ground != FALSE)
				{
					aux->him->action = STAND;
					aux->him->actual.z = ground;
				}
			}

			if (aux->him->inobject!=map) {fnext = ObjectToMap(&GetTransport(aux->him->ontransport)->actual,&aux->him->actual);}
			else {fnext = aux->him->actual;}

			sprintf(buffer,"0,%d,%s,%d,%f,%f,%f,%d,%f,%s;",aux->him->id,aux->him->name,aux->him->vitality,fnext.x,fnext.y,fnext.z,lastaction,aux->him->angle,aux->him->message);
			strcat(state,buffer);

			if (aux->him->action == MESSAGE)
			{
				if (strcmp(aux->him->message,".unstuck")==0)
				{
					aux->him->actual.x = map->start.x;
					aux->him->actual.y = map->start.y;
					aux->him->actual.z = map->start.z;
					aux->him->inobject = map;
					aux->him->ontransport = FALSE;
				}
				strcpy(aux->him->message,"");
				aux->him->action = STAND;
			}
			//printf("THIRD: X: %f, Y: %f, Z: %f, G: %f, H: %f\n",aux->him->actual.x,aux->him->actual.y,aux->him->actual.z,ground,height);
			aux = aux->next;
		}
		pthread_mutex_unlock(&mutex);
		strcat(state,"\n");
		Broadcast(state);
	}
	pthread_exit(0);
}

void ShutdownHandler()
{
	struct node *aux;
	struct node *hlp;
	ShowRegister();
	UnRegisterNpcs(npc_root);
	printf("Server is going to shutdown\n");
	pthread_mutex_lock(&mutex);
		pthread_cancel(server_thread);
		printf("*ServerHandler is terminated.\n");
		pthread_cancel(fb_thread);
		printf("*Realm thread is terminated\n");
		aux = root.next;
		while(aux!=NULL)
		{
			pthread_cancel(aux->that->thread);
			close(aux->that->socket);
			free(aux->that);
			free(aux->him);
			hlp = aux;
			aux = hlp->next;
			free(hlp);
		}
		printf("*All Player threads and conexions has been terminated\n");
		close(s_socket);
		printf("*Socket has been closed\n");
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	DestroyWorldObject(map);
	DestroyTransporters(bus);
	printf("Server is down\n");
	exit(0);
}

void ServerHandler()
{
	sin_size = sizeof(struct sockaddr_in);
	printf("Server is up, for terminating press ctrl-c\n");
	while(TRUE)
	{
		newclient = malloc(sizeof(struct client));
		if ((newclient->socket = accept(s_socket, (struct sockaddr *)&newclient->address, &sin_size)) == ERROR)
		{
			perror("accept");
			free(newclient);
			continue;
		}
		printf("New player from %s.\n",inet_ntoa(newclient->address.sin_addr));
		pthread_create(&newclient->thread,&tattr,(void*)&ClientHandler,(void*)newclient);
	}
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	if (argc!=5)
	{
		printf("Usage: %s <mapfile.bcf> <transporters.tg> <npcs.ng> <attacks.list>\n",argv[0]);
		exit(ERROR);
	}

	root.that = NULL;
	root.him = NULL;
	root.previous = NULL;
	root.next = NULL;

	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);

	if ((s_socket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{
		perror("socket");
		exit(ERROR);
	}

	yes = TRUE;
	if (setsockopt(s_socket, SOL_SOCKET, SO_REUSEADDR,&yes,sizeof(int)) == ERROR)
	{
		perror("setsockopt");
		exit(ERROR);
	}
	
	yes = TRUE;
	if (setsockopt(s_socket, IPPROTO_TCP, TCP_NODELAY,&yes,sizeof(int)) == ERROR)
	{
		perror("setsockopt");
		exit(ERROR);
	}

	s_address.sin_family = AF_INET;
	s_address.sin_port = htons(S_PORT);
	s_address.sin_addr.s_addr = INADDR_ANY;
	memset(&(s_address.sin_zero), '\0', 8);

	if (bind(s_socket, (struct sockaddr *)&s_address, sizeof(struct sockaddr)) == ERROR)
	{
		perror("bind");
		exit(ERROR);
	}

	if (listen(s_socket,BACKLOG) == ERROR)
	{
		perror("listen");
		exit(ERROR);
	}	
	
	map = ObjectFromFile(argv[1]);
	printf("*Successfully loaded binary collision file %s for worldmap.\n",argv[1]);
	LoadTransporters(argv[2]);
	npc_root = LoadNpcFromFile(argv[3]);
	//ShowNpcs(npc_root);
	RegisterNpcs(npc_root);
	LoadAttacksFromFile(attacks,argv[4]);
	ShowAttacks(attacks);
	pthread_create(&fb_thread,&tattr,(void*)&RealmHandler,NULL);
	pthread_create(&server_thread,NULL,(void*)&ServerHandler,NULL);
	signal(SIGINT, ShutdownHandler);
	pthread_join(server_thread,0);
	return 0;
}
