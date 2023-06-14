/*************************** HEADER FILES ***************************/
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include "md5.h"

/*********************** FUNCTION DEFINITIONS ***********************/
BYTE text0[32];
BYTE text2[32] = {53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57,51,67,49,53,51,65,57};
BYTE text3_1[32] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcde"};
BYTE text3_2[32] = {"fghijklmnopqrstuvwxyz0123456789"};

BYTE buf1[16], buf2[16], buf3[16];

clock_t start, end;
double cpu_time_used;

int md5_test()
{
	BYTE hash1[MD5_BLOCK_SIZE] = {0xd4,0x1d,0x8c,0xd9,0x8f,0x00,0xb2,0x04,0xe9,0x80,0x09,0x98,0xec,0xf8,0x42,0x7e};
	BYTE hash2[MD5_BLOCK_SIZE] = {0x90,0x01,0x50,0x98,0x3c,0xd2,0x4f,0xb0,0xd6,0x96,0x3f,0x7d,0x28,0xe1,0x7f,0x72};
	BYTE hash3[MD5_BLOCK_SIZE] = {0xd1,0x74,0xab,0x98,0xd2,0x77,0xd9,0xf5,0xa5,0x61,0x1c,0x2c,0x9f,0x41,0x9d,0x9f};

	MD5_CTX ctx;
	int pass = 1;

	// Initialize text 0 with 0
	memset(text0, 0x00, 32);

	md5_init(&ctx);
	md5_update(&ctx, text0, 32);
	md5_final(&ctx, buf1);
	pass = pass && !memcmp(hash1, buf1, MD5_BLOCK_SIZE);

	// Note the MD5 object can be reused.
	start = clock();
	md5_init(&ctx);
	md5_update(&ctx, text2, strlen(text2));
	md5_final(&ctx, buf2);
	end = clock();
	pass = pass && !memcmp(hash2, buf2, MD5_BLOCK_SIZE);

	// Note the data is being added in two chunks.
	md5_init(&ctx);
	md5_update(&ctx, text3_1, strlen(text3_1));
	md5_update(&ctx, text3_2, strlen(text3_2));
	md5_final(&ctx, buf3);
	pass = pass && !memcmp(hash3, buf3, MD5_BLOCK_SIZE);

	return(pass);
}

int main()
{
	printf("MD5 tests: %s\n", md5_test() ? "SUCCEEDED" : "FAILED");

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("text0: = ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", text0[i]);
	}
	printf("\n");

	printf("MD5(text0) = ");
	for (int i = 0; i < MD5_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf1[i]);
	}
	printf("\n");

	printf("text2: = ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", text2[i]);
	}
	printf("\n");

	printf("MD5(text2) = ");
	for (int i = 0; i < MD5_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf2[i]);
	}
	printf("\n");

	printf("time spent (text2): %f\n", cpu_time_used);

	printf("text3_1: = ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", text3_1[i]);
	}
	printf("\n");

	printf("text3_2: = ");
	for (int i = 0; i < 32; i++)
	{
		printf("%02x", text3_2[i]);
	}
	printf("\n");

	printf("MD5(text3) = ");
	for (int i = 0; i < MD5_BLOCK_SIZE; i++)
	{
		printf("0x%02x ", buf3[i]);
	}

	return(0);
}
