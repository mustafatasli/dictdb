/*
 * dict.c
 *
 *  Created on: Jun 23, 2013
 *      Author: mustafa
 */

#include "dict.h"


// dbj2 hash
unsigned int hash(unsigned char *str, int len)
{
	unsigned int hash = 5381;
	int c;

	while(len--){
		c = *str++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

HashTable *allocTable(int size)
{
	HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
	if (ht == NULL)
		return NULL;

	Entry **table = (Entry **)malloc(sizeof(Entry*)*size);
	if(table == NULL)
		return NULL;

	memset(table, NULL, sizeof(Entry*)*size);
	ht->table = table;
	return ht;
}

void deallocTable(HashTable *ht)
{
	int i = 0;
	for(i=0; i < ht->size; i++){
		ht->table[i] = NULL;
	}

	free(ht->table);
	free(ht);
}

int init(Dict *dict, int size)
{
	HashTable *hashTable = allocTable(size);

	if(hashTable == NULL)
		return -1;

	hashTable->size = size;
	hashTable->count = 0;

	dict->pht = NULL;
	dict->ht = hashTable;

	return size;
}

int isResizing(Dict *dict)
{
	if (dict->pht == NULL)
		return 0;
	if (dict->pht->count == 0)
		return 0;
	return 1;
}

int mustResize(Dict *dict)
{
	HashTable *ht = dict->ht;
	if ( (ht->count / ht->size) >= 2.0 )
		return 1;
	return 0;
}

int resize(Dict *dict)
{
	if(isResizing(dict))
		puts("Something wrong!!!\n");

	HashTable *ht = dict->ht;
	int size = ht->size * 2;
	HashTable *hashTable = allocTable(size);

	if(hashTable == NULL)
		return -1;

	hashTable->size = size;
	hashTable->count = 0;

	dict->pht = dict->ht;
	dict->ht = hashTable;

	return 0;
}

Entry *getNext(HashTable *ht)
{
	Entry *e = NULL;
	int i = 0;
	for(i=0; i < ht->size; i++){
		if(ht->table[i] != NULL){
			e = ht->table[i];
			ht->table[i] = e->next;
			ht->count--;
			return e;
		}
	}

	return NULL;
}

int moveEntries(Dict *dict, int count)
{
	Entry *e=NULL;
	int n=0;

	for(n=0; n < count; n++){
		e = getNext(dict->pht);
		if(e==NULL)
			break;
		else
			addEntry(dict->ht, e);
	}

	return n;
}

void addEntry(HashTable *ht, Entry *entry)
{
	char *key = entry->key;
	int h = hash(key, strlen(key)) % ht->size;
	Entry *current = ht->table[h];
	entry->next = current;
	ht->table[h] = entry;
	ht->count++;
}

Entry* add(Dict *dict, char *key, char *val)
{
	Entry *entry = get(dict, key);

	if(isResizing(dict))
		moveEntries(dict, 2);
	else if (mustResize(dict))
		resize(dict);

	if (entry == NULL) {
		entry = (Entry*)malloc(sizeof(Entry));
		if(entry == NULL)
			return NULL;
		entry->key = key;
		entry->val = val;
		addEntry(dict->ht, entry);
	}
	else {
		entry->val = val;
	}

	return entry;
}


Entry *findEntry(Entry *slot, char *key)
{
	Entry *e = NULL;
	for(e=slot; e != NULL; e=e->next){
		if(strcmp(key, e->key) == 0){
			break;
		}
	}

	return e;
}

Entry* get(Dict *dict, char *key)
{
	HashTable *ht = NULL;
	Entry *slot = NULL;
	Entry *entry = NULL;
	int h = 0;

	if (isResizing(dict)) {
		puts("Resizing\n");
		ht = dict->pht;
		hash(key, strlen(key)) % ht->size;
		entry = findEntry(ht->table[h], key);
	}

	if (entry != NULL)
		return entry;

	ht = dict->ht;
	h = hash(key, strlen(key)) % ht->size;
	entry = findEntry(ht->table[h], key);

	return entry;
}

Entry *removeEntry(HashTable *ht, char *key)
{
	Entry *prev = NULL;
	int h = hash(key, strlen(key)) % ht->size;
	Entry *entry = ht->table[h];

	if(entry != NULL){
		while(entry){
			if(strcmp(key, entry->key) == 0){
				if(prev == NULL)
					ht->table[h] = entry->next;
				else
					prev->next = entry->next;
				return entry;
			}
			prev = entry;
			entry = entry->next;
		}
	}

	return NULL;
}

Entry* delete(Dict *dict, char *key)
{
	Entry *entry = NULL;

	if (isResizing(dict)) {
		entry = removeEntry(dict->pht, key);
	}

	if (entry){
		dict->pht->count--;
		return entry;
	}

	entry = removeEntry(dict->ht, key);

	if(entry){
		dict->ht->count--;
	}

	return entry;
}
