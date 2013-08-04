#include "dict.h"
#include "test.h"
#include "common.h"

void print_keys(HashTable *ht)
{
	Entry *e;
	int i = 0;
	if (ht == NULL)
		return;

	for(i=0; i < ht->size; i++){
		if(ht->table[i] != NULL){
			e = ht->table[i];
			while(e != NULL){
				puts(e->key);
				e = e->next;
			}
		}
	}

	puts("\n");
}

void test_resize()
{
	Dict d;
	int i = 0;
	int size = init(&d, 2);
	char *keys[] = {"test","test1","test2","test3","test4","test5",
			"test6","test7","test8","test9"};
	char *vals[] = {"value","value1","value2","value3","value4","value5",
				"value6","value7","value8","value9"};

	for(i=0; i < 10; i++){
		puts(keys[i]);
		add(&d, keys[i], vals[i]);
		printf("Size: %d, Count: %d, Ratio: %f\n", d.ht->size, d.ht->count, d.ht->count / d.ht->size);
		if(isResizing(&d)){
			puts("Resizing\n");
			print_keys(d.pht);
		}
		else{
			puts("Not Resizing\n");
			print_keys(d.ht);
		}
	}
}

void test(){
	Dict d;
	puts("!!!Init!!!");
	int size = init(&d, 8);
	if (size == -1){
		puts("init error\n");
		return;
	}

	Entry * e;

	char *key = "test";
	char *val = "xxxxxxxxxxxx";
	puts("add");
	e = add(&d, key, val);

	puts("List Keys");
	print_keys(d.ht);

	puts("Resize\n");
	test_resize();

	e = get(&d, key);
	printf("%s, %s", e->key, e->val);
	delete(&d, key);
	e = get(&d, key);
	if(e == NULL)
		puts("Success\n");
}
