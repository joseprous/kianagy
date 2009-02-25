
struct Attack
{
	int id;
	int radius;
	int angle;
	int damage;
	int colddown;
	int cost;
	char name[BUFFSIZE];	
};

struct Attack *CreateAttack(int id,int radius, int angle, int damage,int colddown,int cost,char *name)
{
	struct Attack *new;
	
	new = malloc(sizeof(struct Attack));

	new->id = id;
	new->radius = radius;
	new->angle = angle;
	new->damage = damage;
	new->colddown = colddown;
	new->cost = cost;
	strcpy(new->name,name);
	
	return new; 
}

void DestroyAttack(struct Attack *this)
{
	free(this);
}

void LoadAttacksFromFile(struct Attack *list[], char *filename)
{
	char header[BUFFSIZE];
	char name[BUFFSIZE];
	int radius,angle,damage,colddown,cost,id;
	FILE *input;
	if ((input = fopen(filename,"r"))==NULL){printf("Error while loading attacks list from %d.\n",filename);exit(ERROR);}
	while(!feof(input))
	{
		fscanf(input,"%s %d %d %d %d %d %d %s",header,&id,&radius,&angle,&damage,&colddown,&cost,name);
		if (strcmp(header,"ATTACK=")==0){list[id] = CreateAttack(id,radius,angle,damage,colddown,cost,name);}
	}
	fclose(input);
}

void DestroyAttacksList(struct Attack *list[])
{
	int i;
	for(i=0;i<MAX_ATTACKS;i++)
	{
		DestroyAttack(list[i]);
	}
}

void ShowAttacks(struct Attack *list[])
{
	int i;
	for(i=0;i<MAX_ATTACKS;i++)
	{
		if(list[i]!=NULL){printf("ATTACK= %d %d %d %d %d %d %s\n",list[i]->id,list[i]->radius,list[i]->angle,list[i]->damage,list[i]->colddown,list[i]->cost,list[i]->name);}
	}
}

void DoAttack(struct player *attacker, struct player *victim, struct Attack *attack)
{
	if (InRange(&attacker->actual,&victim->actual,attack->radius))
	{
		printf("%s got attacked by %s.\n",attacker->name,victim->name);
		victim->hurt = TRUE;
		victim->last_attacker = attacker;
		victim->vitality -= attack->damage;
		attacker->vitality -= attack->cost;
		attacker->colddowns[attack->id] = attack->colddown;
	}
}