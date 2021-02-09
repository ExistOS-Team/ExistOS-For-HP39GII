#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#define TEST_SIZE_KB	(3212)


extern void sleep(int ms);

unsigned int U8RAND = 1;
unsigned char
get_u8_rand (unsigned int *seed)
{
  unsigned int next = *seed;
  int result;

  next *= 1103515245;
  next += 12345;
  result = (unsigned int) (next / 65536) % 2048;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  next *= 1103515245;
  next += 12345;
  result <<= 10;
  result ^= (unsigned int) (next / 65536) % 1024;

  *seed = next;

  return result & 0xFF;
}

int times = 0;

int main()
{
	
	
	
	unsigned char *mem;
	unsigned char *op_addr;
	
	re:
	
	U8RAND = 1;
	printf("Starting memtest.\n");
	printf("Test Size:%d KB\n",TEST_SIZE_KB);
	mem = malloc(TEST_SIZE_KB * 1024);
	if(mem == NULL){
		printf("malloc fault.\n");
		return 1;
	}
	printf("Malloc got addr:%08x\n",(unsigned int)mem);
	printf("Writing...\n");
	op_addr = mem;
	int ls = 0;
	for(int i=0;i<TEST_SIZE_KB * 1024; i++){
		
		if(((int)(((double)i/(TEST_SIZE_KB * 1024))*100)) != ls){
			if(ls % 10 == 0){
				printf("%d%%  ", ((int)(((double)i/(TEST_SIZE_KB * 1024))*100)) + 9 );
				printf("WR ADDR:%08x\n",(unsigned int)op_addr);
			}
			ls = ((int)(((double)i/(TEST_SIZE_KB * 1024))*100));
		}
		
		*op_addr = get_u8_rand(&U8RAND);
		op_addr++;
	}
	printf("WRITE DONE...\n");
	for(int i=0; i<3; i++){
		sleep(1000);
		printf(".");
		fflush(stdout);
	}
	printf("\nReading...\n");
	U8RAND = 1;
	op_addr = mem;
	unsigned char test_val;
	unsigned int fault_count = 0;
	ls = 0;
	for(int i=0;i<TEST_SIZE_KB * 1024; i++){
		test_val = get_u8_rand(&U8RAND);
		
		if(((int)(((double)i/(TEST_SIZE_KB * 1024))*100)) != ls){
			if(ls % 10 == 0){
				printf("%d%%  ", ((int)(((double)i/(TEST_SIZE_KB * 1024))*100)) + 9 );
				printf("Test Address : %08X, TEST VALUT:%02X, READ BACK:%02x\n",(unsigned int)op_addr,test_val,*op_addr);
			}
			ls = ((int)(((double)i/(TEST_SIZE_KB * 1024))*100));
		}
		
		if(*op_addr != test_val){
			printf("TEST FAULT AT: %08X, TEST VALUT:%02X, READ BACK:%02X\n",(unsigned int)op_addr,test_val,*op_addr);
			fault_count++;
		}
		if(fault_count > 10){
			printf("Too many faults!.\n");
			break;
		}
		op_addr++;
	}	
	
	printf("MemTest Finish.\n");
	
	times++;
	if(times == 2){
		return 1;
	}
	goto re;
	
	//return 1;
}