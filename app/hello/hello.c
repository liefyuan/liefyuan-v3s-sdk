#include "stdio.h"

int main(int argc, unsigned char* argv[])
{
	int i;
	printf("Total %d arguments\n", argc);
	
	for(i = 0; i < argc; i++)
	{
		printf("\nArgument argv[%d] = %s \n", i, argv[i]);
	}

	printf("Hello World!\r\n");

	return 0;
}
