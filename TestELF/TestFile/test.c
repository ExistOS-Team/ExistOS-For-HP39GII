#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


extern void sleep(int ms);


int main()
{
	
	FILE *fil;
	fil = fopen("/t233","wb+");
	if(fil != NULL){
		printf("file opened.\n");
	}
	fprintf(fil,"Hello World!\n");
	fclose(fil);
	printf("fclosed\n");

	return 0;
}