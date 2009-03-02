

#define BREATH 25
#define MAXBUFLEN 100

#define PORT 7777


void send_msg(int code);
void send_msg2(int code,char *txt);
void send_txt();
void send_name();
int network(void *data);
void ini_network();
void end_network();
