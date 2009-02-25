
struct ActionNode
{
	int id;
	struct position actual;
	int action;
	int time;
	struct ActionNode *edges[MAX_ACTION_NODES];
	int edgesprob[MAX_ACTION_NODES];
};

struct Keyword
{
	char keyword[BUFFSIZE];
	char response[BUFFSIZE];
};

struct Npc
{
	struct player *him;
	struct ActionNode *actions[MAX_ACTION_NODES];
	struct Npc *next;
	struct node *that;
	struct ActionNode *current_action;
	int wait;
	int chatting;
	struct Keyword *keywords[MAX_KEYWORDS];
	int patience;
};

struct ActionNode *CreateActionNode(int id,float x,float y,float z,int action,int time)
{
	struct ActionNode *new;
	new = malloc(sizeof(struct ActionNode));

	new->id = id;
	new->actual.x = x;
	new->actual.y = y;
	new->actual.z = z;
	new->action = action;
	new->time = time;
	memset(new->edges,0,MAX_ACTION_NODES*sizeof(struct ActionNode*));
	memset(new->edgesprob,0,MAX_ACTION_NODES*sizeof(int));

	return new;
}

struct ActionNode *GoActionEdge(struct ActionNode *this,int prob)
{
	int i;
	for(i=0;i<MAX_ACTION_NODES;i++)
	{
		if (this->edges[i]!=NULL){if (this->edgesprob[i]>prob){break;}}
	}
	//printf("GO: %d\n",i);
	return this->edges[i];
}

void DestroyActionNode(struct ActionNode *this)
{
	free(this);
}

struct Keyword *CreateKeyword(char *buffer)
{
	int i,j;
	struct Keyword *new;
	new = malloc(sizeof(struct Keyword));

	i=0;
	while(buffer[i]!=' '){i++;}
	
	j=0;
	i++;
	while(buffer[i]!=' ')
	{
		new->keyword[j++] = buffer[i++];
	}
	new->keyword[j] = '\0';
	
	j=0;
	i++;
	while(buffer[i]!='\n')
	{
		new->response[j++] = buffer[i++];
	}
	new->keyword[j] = '\0';

	return new;
}

void DestroyKeyword(struct Keyword *this)
{
	free(this);
}

struct Npc *CreateNpc(int id, float x, float y,float z,char *name,int patience)
{
	struct Npc *new;
	new = malloc(sizeof(struct Npc));

	new->him = CreatePlayer(id,x,y,z,name,map);
	memset(new->actions,0,MAX_ACTION_NODES*sizeof(struct ActionNode*));
	new->next = NULL;
	new->that = NULL;
	new->current_action = NULL;
	new->wait = MILLON;
	new->chatting = FALSE;
	memset(new->keywords,0,MAX_KEYWORDS*sizeof(struct Keyword*));
	new->patience = patience;
	
	return new;
}

void DestroyNpc(struct Npc *this)
{
	int i;
	for (i=0;i<MAX_ACTION_NODES;i++){DestroyActionNode(this->actions[i]);}
	for (i=0;i<MAX_KEYWORDS;i++){DestroyKeyword(this->keywords[i]);}
	DestroyPlayer(this->him);
	free(this);
}

struct Npc *LoadNpcFromFile(char *filename)
{
	char buffer[BUFFSIZE];
	char header[BUFFSIZE];
	char string[BUFFSIZE];
	FILE *input;
	struct Npc *current_npc,*last_npc,*root;
	int id,action,time,prob,first_npc,current_keyword,patience;
	float x,y,z;
		
	input = fopen(filename,"r");
	
	current_keyword = 0;
	first_npc = TRUE;
	while(!feof(input))
	{
		fgets(buffer,BUFFSIZE,input);
		sscanf(buffer,"%s ",header);
		//printf("%s\n",header);
		if (strcmp(header,"EDGE=")==0)
		{
			sscanf(buffer,"%s %d %d %d",header,&id,&action,&prob); // Reutilizando varuables.
			//printf("%s %d %d %d ->",header,id,action,prob);
			last_npc->actions[id]->edges[action] = last_npc->actions[action];
			last_npc->actions[id]->edgesprob[action] = prob;
			//printf("%d %d %d\n",last_npc->actions[id],last_npc->actions[id]->edges[action],last_npc->actions[id]->edgesprob[action]);
		}
		else
		{
			if (strcmp(header,"ACTIONNODE=")==0)
			{
				sscanf(buffer,"%s %d %f %f %f %d %d",header,&id,&x,&y,&z,&action,&time);
				//printf("%s %d %f %f %f %d %d ->",header,id,x,y,z,action,time);
				last_npc->actions[id] = CreateActionNode(id,x,y,z,action,time);
				//printf("%d\n",last_npc->actions[id]);
			}
			else
			{
				if (strcmp(header,"KEYWORD=")==0)
				{
					last_npc->keywords[current_keyword++] = CreateKeyword(buffer);
					/*printf("KEYWORD= %s %s\n",last_npc->keywords[current_keyword]->keyword,last_npc->keywords[current_keyword]->response);*/
				}
				else
				{
					if(strcmp(header,"NPC=")==0)
					{
						sscanf(buffer,"%s %d %f %f %f %s %d",header,&id,&x,&y,&z,string,&patience);
						//printf("%s %d %f %f %f %s ->",header,id,x,y,z,string);

						current_npc = CreateNpc(id,x,y,z,string,patience);
						if (first_npc==TRUE)
						{
							root = current_npc;
							first_npc = FALSE;
						}
						else
						{
							last_npc->next = current_npc;
						}
						last_npc = current_npc;
						current_keyword=0;
						//printf("%d\n",last_npc);
					}
				}
			}
		}
	}
	fclose(input);
	printf("*Successfully loaded npc from %s.\n",filename);
	return root;
}

void ShowNpcs(struct Npc *root)
{
	int i,j;
	struct Npc *aux;
	aux = root;
	while(aux!=NULL)
	{
		printf("NPC= %d %s %d\n",aux->him->id,aux->him->name,aux->patience);
		for(i=0;i<MAX_ACTION_NODES;i++)
		{
			if (aux->actions[i]==NULL){continue;}
			printf(" ACTIONNODE= %d %d %d\n",aux->actions[i]->id,aux->actions[i]->action,aux->actions[i]->time);
			for(j=0;j<MAX_ACTION_NODES;j++)
			{
				if (aux->actions[i]->edges[j]==NULL){continue;}
				printf("  EDGE= %d %d\n",aux->actions[i]->edges[j]->id,aux->actions[i]->edgesprob[j]);
			}
		}
		for(i=0;i<MAX_KEYWORDS;i++)
		{
			if (aux->keywords[i]==NULL){break;}
			printf("KEYWORD= [%s] [%s]\n",aux->keywords[i]->keyword,aux->keywords[i]->response);
		}
		aux = aux->next;
	}	
}

void RegisterNpcs(struct Npc *root)
{
	struct Npc *aux;
	aux = root;
	while(aux!=NULL)
	{
		aux->current_action = aux->actions[0]; 
		aux->that = Register(NULL,aux->him);
		aux = aux->next;
	}
}

void UnRegisterNpcs(struct Npc *root)
{
	struct Npc *aux,*next;
	aux = root;
	while(aux!=NULL)
	{
		next = aux->next;
		UnRegister(aux->that);
		DestroyNpc(aux);
		aux = next;
	}
}

int NpcGetResponse(struct Npc *this,char *keyword,char *response)
{
	int i;
	for(i=0;i<MAX_KEYWORDS;i++)
	{
		if (this->keywords[i]==NULL){break;}
		if (strcmp(this->keywords[i]->keyword,keyword)==0) {strcpy(response,this->keywords[i]->response); return TRUE;}
	}
	strcpy(response,this->keywords[0]->response);
	return FALSE;
}


void NpcDoChat(struct Npc *this, struct player *him)
{
	char response[BUFFSIZE];
	if (InRange(&this->him->actual,&him->actual,CHAT_AREA))
	{
			NpcGetResponse(this,him->message,response);
			this->him->action = MESSAGE;
			strcpy(this->him->message,response);
			this->wait += this->patience;
	}
}

void NpcFollowThis(struct Npc *npc, struct position *actual)
{
	struct position vector;
	vector = UnitVector(&npc->him->actual,actual);
	npc->him->angle = (atan2f(vector.y,vector.x)*180)/PI;
}

void NpcGotAttacked(struct Npc *npc, struct player *player)
{
	NpcFollowThis(npc,&player->actual);
	npc->wait += 10000000;
}

void NpcHandler(struct Npc *root)
{
	struct position vector;
	struct Npc *aux;
	aux = root;
	while(aux!=NULL)
	{
		if (aux->wait<=0)
		{
			if (aux->him->hurt==TRUE)
			{
				NpcGotAttacked(aux,aux->him->last_attacker);
			}
			else
			{
				if(InRange(&aux->him->actual,&aux->current_action->actual,ACTION_AREA))
				{
					aux->wait = aux->current_action->time;
					aux->him->action = aux->current_action->action;
					aux->current_action = GoActionEdge(aux->current_action,rand()%PROB_SCALE);
					NpcFollowThis(aux,&aux->current_action->actual);
					//printf("ANGLE: %f\n",aux->him->angle);
				}
				else
				{
					aux->him->action = MOVE;
				}
			}
		}
		else
		{
			aux->wait-=TICKS;
			if (aux->him->hurt==TRUE)
			{
				if (aux->wait<=0)
				{
					aux->him->hurt = FALSE;
					aux->him->last_attacker = NULL;
					NpcFollowThis(aux,&aux->current_action->actual);
				}
				else
				{
					NpcFollowThis(aux,&aux->him->last_attacker->actual);
					if (InRange(&aux->him->actual,&aux->him->last_attacker->actual,attacks[0]->radius))
					{
						aux->him->action = ATTACK;
					}
					else
					{
						aux->him->action = MOVE;
					}
				}
			}
		}
		aux = aux->next;
	}
}

