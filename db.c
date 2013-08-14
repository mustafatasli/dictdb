#include "db.h"

int parseQuery(Client *c)
{
	/*
	 * "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n"
	 */
	char *next=NULL;
	char *newline=NULL;
	char *val=NULL;
	char ch;
	int len=0, r=0, i=0;
	CommandArg *arg=NULL;

	if(c->len <= 0)
		return -1;

	if(c->current == 0){
		newline = strchr(c->buf, '\r');
		if(newline == NULL)
			return -1;
		next = newline + 1;
		if(*next != '\n')
			return -1;
		r = stringToInt(c->buf+1, newline-c->buf-1, &c->argc);
		if(r != 0)
			return -1;

		if(c->argv == NULL)
			c->argv = (CommandArg*)malloc(sizeof(CommandArg)*c->argc);

		next = newline+2;
		if(*next != '$')
			return -1;

		newline = strchr(next, '\r');
		if(newline == NULL)
			return -1;
		if(*(newline+1) != '\n')
			return -1;

		r = stringToInt(next+1, newline-next-1, &len);
		if(r != 0)
			return -1;

		val = newline+2;
		arg = &c->argv[0];
		arg->len = len;
		arg->val = val;
		c->current = 1;
	}

	for(i=c->current; i < c->argc; i++){
		arg = &c->argv[c->current-1];
		next = arg->val + arg->len + 2;
		//next = val+len+2;
		if(*next != '$')
			return -1;

		newline = strchr(next, '\r');
		if(newline == NULL)
			return -1;
		if(*(newline+1) != '\n')
			return -1;

		r = stringToInt(next+1, newline-next-1, &len);
		if(r != 0)
			return -1;

		val = newline+2;
		newline = strchr(val, '\r');
		if(newline == NULL)
			return -1;
		if(*(newline+1) != '\n')
			return -1;
		arg = &c->argv[i];
		arg->len = len;
		arg->val = val;
		c->current = i+1;
	}

	return 0;
}

int processQuery(Client *c)
{

}


