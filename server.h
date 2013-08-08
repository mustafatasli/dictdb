
typedef struct Client {
	int fd;
	int numRead;
	int ready;
	char *buf;
}Client;

void initServer();
