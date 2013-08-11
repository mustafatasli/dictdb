/*
 ============================================================================
 Name        : DictDb.c
 Author      : mustafa tasli
 Version     : 0.1
 Description : Dictionary DB in C, Ansi-style
 ============================================================================
 */

#include "dict.h"
#include "server.h"
#include "test.h"


int main(void) {
	puts("Server Starting\n");
	test_stringToInt();
	test_parseQuery();
	//initServer();
	return EXIT_SUCCESS;
}
