#include "dict.h"
#include "server.h"
#include "test.h"
#include "common.h"
#include <assert.h>


void test_parseQuery(){
	int r=0;
	Client *c = createClient(-1);
	c->buf = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n";
	c->len = strlen(c->buf);
	r = parseQuery(c);
	assert(r == 0);
	assert(c->argc == 3);
	printf("Result %d, argc %d\r", r, c->argc);
	assert(strncmp(c->argv[0].val, "SET", c->argv[0].len) == 0);
	assert(strncmp(c->argv[1].val, "mykey", c->argv[1].len) == 0);
	assert(strncmp(c->argv[2].val, "myvalue", c->argv[2].len) == 0);
}

void test_stringToInt()
{
	char *nums[] = { "*3", "*34", "*012", "*000"};
    int vals[] = {3, 34, 12, 0,};
	int n=-1, r=-1, i=0;

	for(i=0; i < 4; i++){
		r = stringToInt(nums[i]+1, strlen(nums[i])-1, &n);
		assert(r == 0);
		assert(n == vals[i]);
		printf("Result %d, num %d\r", r, n);
	}

	char *wrong = "*1c98";
	r = stringToInt(wrong+1, strlen(wrong)-1, &n);
	assert(r == -1);

}

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
