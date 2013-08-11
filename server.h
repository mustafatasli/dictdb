
typedef struct CommandArg {
	int len;
	char *val;
}CommandArg;

typedef struct Client {
	int fd;
	int numRead;
	int ready;
	char *buf;

	int len;
	int pos;
	int free;
	int buf_size;

	//Query paramaters
	int argc;
	char *cmd;
	CommandArg *argv;
	int current;

}Client;




Client *createClient(int fd);

void initServer();

