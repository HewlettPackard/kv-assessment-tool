/*
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP This
 * program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * impl ied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details. You
 * should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses.
*/

#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define NUM_MEM_INCREMENTS 32
#define PAGE_ALLOC_BYTES 4096
#define NUM_ITERATIONS 1

#define CARD_INFO "3333 444444 55555 1234 0910 Frodo Baggins"

struct memtest ta;

struct memtest {
	u_int8_t *array[NUM_MEM_INCREMENTS];
};

static void
signal_handler(int sig)
{
	u_int8_t iter;
	
	if (sig == SIGKILL) {
		for (iter = 0; iter < NUM_ITERATIONS; iter++)
		{
			u_int8_t *ptr = ta.array[iter];
			munlock(ptr, PAGE_ALLOC_BYTES);
			free(ptr);
		}
	}

	exit(EXIT_SUCCESS);
}

static int
page_malloc_test(void)
{
	int array_index;
	int rcode = 0;
	size_t slen = 0;
	u_int8_t *pgptr = NULL;
	
	for (array_index = 0; array_index < NUM_ITERATIONS; array_index++)
	{
		ta.array[array_index] = malloc(PAGE_ALLOC_BYTES);

		if (ta.array[array_index] == NULL) {
			rcode = -1;
			break;
		}

		pgptr = ta.array[array_index];
		mlock(pgptr, (size_t)PAGE_ALLOC_BYTES);
		
		memset(pgptr, 0, PAGE_ALLOC_BYTES);
		slen = strlen(CARD_INFO) + 1;
		memcpy(pgptr, CARD_INFO, slen);
		fprintf(stdout, "malloc operation %u ptr val: %p\n", 
			array_index,
			ta.array[array_index]);
	} 

	return rcode;
}

int
main(void)
{
	int rcode = 0;

	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		printf("signal error %d \n", errno);
		return 1;
	}

	fprintf(stdout, "running as pid: %d\n", getpid());

	rcode = page_malloc_test();

	do {

	} while(1);

	return rcode;
}

