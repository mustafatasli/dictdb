
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include  <unistd.h>
#include <errno.h>

#include "common.h"
#include "server.h"

#define READ_BUFFER_SIZE 	100
#define QUERY_BUFFER_SIZE	100
#define MAX_CLIENT 	100

const char *SOCK_NAME = "/tmp/mysock";


int makeNonBlocking(int fd)
{
	int flags, r;
	flags = fcntl (fd, F_GETFL, 0);
	if (flags == -1){
	  puts("fcntl");
	  return -1;
	}

	flags |= O_NONBLOCK;
	r = fcntl(fd, F_SETFL, flags);
	if (r == -1){
	  puts("F_SETFL");
	  return -1;
	}

	return 0;
}

int createUnixSocket(){
	int sfd;
	struct sockaddr_un addr;

	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sfd == -1){
		puts("Socket Error\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_NAME, sizeof(addr.sun_path) - 1);

	if(remove(SOCK_NAME) == -1 && errno != ENOENT){
		puts("File Error\n");
		exit(-1);
	}

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1){
		puts("Bind Error\n");
		exit(-1);
	}

	return sfd;
}

int createTCPSocket(){
	int sfd;
	struct sockaddr_in addr;

	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd == -1){
		puts("Socket Error\n");
		exit(-1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(50005);

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1){
		puts("Bind Error\n");
		exit(-1);
	}

	return sfd;
}

Client *createClient(int fd)
{
	Client *c = (Client *)malloc(sizeof(Client));
	if (c == NULL)
		return NULL;
	c->fd = fd;
	c->buf = (char*)malloc(QUERY_BUFFER_SIZE + 1);
	c->numRead = 0;
	c->ready = 0;
	c->argc = 0;
	c->free = 0;
	c->pos = 0;
	c->len = 0;
	c->buf_size = QUERY_BUFFER_SIZE + 1;

	return c;
}

void resetClient(Client *c)
{
	c->ready = 0;
	c->numRead = 0;
}

void freeClient()
{

}

int acceptClient(Client **clients, int sfd, int epfd){
	int cfd;
	struct Client *c=NULL;
	struct epoll_event ev;
	for(;;){
		puts("Accept\n");
		cfd = accept(sfd, NULL, NULL);
		puts("Accepted\n");
		if(cfd == -1){
			break;
		}

		makeNonBlocking(cfd);
		c = createClient(cfd);
		if(c == NULL)
			return -1;
		clients[cfd] = c;

		ev.events = EPOLLIN;
		ev.data.fd = cfd;
		if(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1){
			puts("epoll_ctl Error\n");
			return -1;
		}
	}

	return 0;
}


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
	CommandArg *a;

	if(c->len){
		newline = strchr(c->buf, '\r');
		if(newline){
			next = newline + 1;
			if(*next != '\n')
				return -1;
			r = stringToInt(c->buf+1, newline-c->buf-1, &c->argc);
			if(r != 0)
				return -1;

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
			arg = (CommandArg*)malloc(sizeof(CommandArg));
			arg->len = len;
			arg->val = val;
			c->argv[0] = *arg;
			c->current = 1;

			for(i=c->current; i < c->argc; i++){
				a = &c->argv[c->current-1];
				next = a->val + a->len + 2;
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
				arg = (CommandArg*)malloc(sizeof(CommandArg));
				arg->len = len;
				arg->val = val;
				c->argv[i] = *arg;
				c->current = i+1;
			}
		}
		else
			return -1;
	}

	return 0;
}

int readClient(Client **clients, int cfd)
{
	ssize_t numRead = 0;
	Client *c = clients[cfd];
	if (c == NULL)
		return -1;
	if ((c->numRead + READ_BUFFER_SIZE) >= c->buf_size){
		printf("Buffer Max Client %d, %d\n", c->fd, c->numRead);
		c->buf = (char*)realloc(c->buf, c->numRead + QUERY_BUFFER_SIZE + 1);
		c->buf_size = c->numRead + QUERY_BUFFER_SIZE + 1;
	}

	numRead = read(cfd, c->buf+c->numRead, READ_BUFFER_SIZE);
	printf("numRead %d\n", numRead);

	if(numRead == -1){
		puts("numRead -1");
		if (errno == EAGAIN) {
			puts("EAGAIN");
			numRead = 0;
		}
		else
			return -1;
	}
	else if(numRead == 0){
		close(cfd);
	}

	if(numRead){
		c->len += numRead;
		c->buf[c->len] = '\0';
	}

	return 0;
}


void initServer()
{
	int fd, sfd, cfd, epfd;
	int lgfd;
	int i, num=0;

	struct Client *clients[MAX_CLIENT] = { NULL };

	struct Client *c=NULL;
	struct epoll_event *evlist;

	sfd = createTCPSocket();
	makeNonBlocking(sfd);

	if(listen(sfd, 10) == -1){
		puts("Listen Error\n");
		exit(-1);
	}

	epfd = epoll_create1(0);
	if(epfd == -1){
		puts("epoll Error\n");
		exit(-1);
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sfd;

	if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1){
		puts("epoll_ctl Error\n");
		exit(-1);
	}

	evlist = calloc(10, sizeof(struct epoll_event));

	for(;;){

		for(i=0; i < MAX_CLIENT; i++){
			c = clients[i];
			if (c == NULL)
				continue;

			if (c->ready){
				printf("Client %d ready, %d\n", c->fd, c->numRead);
				if (write(STDOUT_FILENO, c->buf, c->numRead) != c->numRead){
					puts("write Error\n");
					return;
				}
				resetClient(c);
			}

		}

		puts("epoll_wait");
		num = epoll_wait(epfd, evlist, 1, -1);
		if(num == -1){
			if(errno == EINTR){
				puts("EINTR");
				continue;
			}
		}

		for(i=0; i < num; i++){

			if(sfd == evlist[i].data.fd) {
				if(evlist[i].events & EPOLLIN){
					acceptClient(clients, sfd, epfd);
				}
				continue;
			}
			else if (evlist[i].events & EPOLLIN){
					puts("readClient");
					cfd = evlist[i].data.fd;
					readClient(clients, cfd);
				}

		}
	}
}
