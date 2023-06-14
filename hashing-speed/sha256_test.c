/*************************** HEADER FILES ***************************/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include "sha256.h"

/*********************** FUNCTION DEFINITIONS ***********************/
BYTE buf1[SHA256_BLOCK_SIZE], buf2[SHA256_BLOCK_SIZE], buf3[SHA256_BLOCK_SIZE];
BYTE text0[SHA256_BLOCK_SIZE];
BYTE text2[SHA256_BLOCK_SIZE] = {53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57};
BYTE text3[] = {"aaaaaaaaaa"};

clock_t start, end;
double cpu_time_used;

int sha256_test()
{
	BYTE hash1[SHA256_BLOCK_SIZE] = {0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
	                                 0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad};
	BYTE hash2[SHA256_BLOCK_SIZE] = {0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
	                                 0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1};
	BYTE hash3[SHA256_BLOCK_SIZE] = {0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
	                                 0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0};
	SHA256_CTX ctx;
	int idx;
	int pass = 1;

	// Initialize text 0 with 0
	memset(text0, 0x00, SHA256_BLOCK_SIZE);

	sha256_init(&ctx);
	sha256_update(&ctx, text0, SHA256_BLOCK_SIZE);
	sha256_final(&ctx, buf1);
	pass = pass && !memcmp(hash1, buf1, SHA256_BLOCK_SIZE);

	start = clock();
	sha256_init(&ctx);
	sha256_update(&ctx, text2, strlen(text2));
	sha256_final(&ctx, buf2);
	end = clock();
	pass = pass && !memcmp(hash2, buf2, SHA256_BLOCK_SIZE);

	sha256_init(&ctx);
	for (idx = 0; idx < 100000; ++idx)
	   sha256_update(&ctx, text3, strlen(text3));
	sha256_final(&ctx, buf3);
	pass = pass && !memcmp(hash3, buf3, SHA256_BLOCK_SIZE);

	return(pass);
}

int main()
{
	printf("SHA-256 tests: %s\n", sha256_test() ? "SUCCEEDED" : "FAILED");

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("text0: = ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", text0[i]);
	}
	printf("\n");

	printf("SHA-256(text0) = ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf1[i]);
	}
	printf("\n");

	printf("text2: = ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", text2[i]);
	}
	printf("\n");

	printf("SHA-256(text2) = ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf2[i]);
	}
	printf("\n");

	printf("time spent (text2): %f\n", cpu_time_used);

	printf("text3: = ");
	for (int i = 0; i < strlen(text3); i++)
	{
		printf("%02x", text3[i]);
	}
	printf("\n");

	printf("SHA-256(text3) = ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf3[i]);
	}

	return(0);
}
