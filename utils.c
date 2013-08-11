#include "common.h"
#include "utils.h"

int stringToInt(char *ch, int len, int *n)
{
	char *p=NULL;
	int val = 0;
	int i = 0;

	if(len < 1)
		return -1;
	p = ch;

	for(i=0; i < len; i++, p++){
		if(*p >= '0' && *p <= '9'){
			val *= 10;
			val += *p - '0';
		}
		else
			return -1;
	}

	*n = val;

	return 0;
}

