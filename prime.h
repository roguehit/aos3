#ifndef PRIME_H_XMLRPC
#define PRIME_H_XMLRPC

#include <openssl/bn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "queue.h"

	void status() {}
	void print_prime(char *prime)
	{
		int i;
		for(i = 0; i < strlen(prime) && prime[i] == '0'; i++);
		for(; i < strlen(prime); i++)
			printf("%c", prime[i]);
		printf("\n");
	}
Task gen_prime(Task task)
{
	srand(time(0));
	char *prime;
	BIGNUM *num_tmp;
	char *p; //power or exponent
	char *m; //modulus or prime number
	long int num_bits = 512;
	long int num_bits1 = 32;
	char *temp1,*temp2;
	
	   num_tmp = BN_new();
	  // printf("Generating Exponent\n");	
	   BN_generate_prime(num_tmp, num_bits1 ,1,NULL,NULL,status,NULL);
	   temp1 =  (char *)malloc(BN_num_bytes(num_tmp));
	   temp1 = BN_bn2dec(num_tmp);
	   if(strcpy(task.p,temp1)==NULL){
	   fprintf(stderr,"Copy of exponent Failed\n");
	   exit(0);
	   }
	   //printf("Sizeof temp1 is %lu\n",sizeof(task.p));

	   //print_prime(task.p);    
	   //printf("Generating Prime\n");
	   BN_generate_prime(num_tmp,num_bits,1,NULL,NULL,status,NULL);
	   temp2 = (char *)malloc(BN_num_bytes(num_tmp));
	   temp2 = BN_bn2dec(num_tmp);
	//   printf("Prime Created\n");
	   
	   if( strcpy(task.m,temp2)==NULL){
	   fprintf(stderr,"Copy of exponent Failed\n");
	   exit(0);
	   }
	sprintf(task._clientid,"%d",getpid());	
	return task;
}

#endif
