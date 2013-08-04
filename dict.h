/*
 * dict.h
 *
 *  Created on: Jun 17, 2013
 *      Author: mustafa
 */

#ifndef DICT_H_
#define DICT_H_

#include "common.h"

#define RESIZE_FACTOR	8

unsigned int hash(unsigned char *str, int len);

typedef struct Entry {
	char * key;
	char * val;
	struct Entry *next;
}Entry;

typedef struct HashTable {
	Entry **table;
	int size;
	int count;
}HashTable;

typedef struct Dict {
	HashTable *pht;
	HashTable *ht;
}Dict;

int isResizing(Dict *dict);
int mustResize(Dict *dict);
int resize(Dict *dict);
Entry *getNext(HashTable *ht);
int moveEntries(Dict *dict, int count);
void addEntry(HashTable *ht, Entry *entry);
Entry *findEntry(Entry *slot, char *key);
Entry *removeEntry(HashTable *ht, char *key);

HashTable *allocTable(int size);
void deallocTable(HashTable *ht);

int init(Dict *dict, int size);
Entry *add(Dict *dict, char *key, char * val);
Entry *get(Dict *dict, char *key);
Entry *delete(Dict *dict, char *key);

#endif /* DICT_H_ */
