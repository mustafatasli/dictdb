
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

#define BUF_SIZE 	100

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
	addr.sin_port = htons(50001);

	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1){
		puts("Bind Error\n");
		exit(-1);
	}

	return sfd;
}

int acceptClient(int sfd, int epfd){
	int cfd;
	struct epoll_event ev;
	for(;;){
		puts("Accept\n");
		cfd = accept(sfd, NULL, NULL);
		puts("Accepted\n");
		if(cfd == -1){
			break;
		}

		makeNonBlocking(cfd);

		ev.events = EPOLLIN;
		ev.data.fd = cfd;
		if(epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev) == -1){
			puts("epoll_ctl Error\n");
			return -1;
		}
	}

	return 0;

}

void initServer()
{
	int fd, sfd, cfd, epfd;
	int lgfd;
	int i, num=0;
	int c = 0;

	char buf[BUF_SIZE];
	ssize_t numRead;
	struct epoll_event *evlist;

	lgfd = open("/tmp/db.log", O_WRONLY|O_CREAT|O_TRUNC);
	if(lgfd == -1){
		puts("log file error");
		exit(-1);
	}

	char *msg = "DictDB Log\n";
	write(lgfd, msg, strlen(msg));

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
	ev.events = EPOLLIN | EINTR;
	ev.data.fd = sfd;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1){
		puts("epoll_ctl Error\n");
		exit(-1);
	}

	evlist = calloc(10, sizeof(struct epoll_event));

	for(;;){
		num = epoll_wait(epfd, evlist, 1, -1);
		if(num == -1){
			if(errno == EINTR)
				continue;
		}


		for(i=0; i < num; i++){

			if(sfd == evlist[i].data.fd){
				if(evlist[i].events & EPOLLIN){
					acceptClient(sfd, epfd);
				}
				continue;
			} else if (evlist[i].events & EPOLLIN){
					cfd = evlist[i].data.fd;
					while( (numRead = read(cfd, buf, BUF_SIZE)) >= 0)
					{
						if (write(STDOUT_FILENO, buf, numRead) != numRead){
							puts("write Error\n");
							exit(-1);
						}
					}
				}

		}

	}
}
