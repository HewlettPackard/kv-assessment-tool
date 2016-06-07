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

#include <common_utils.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXCMDBUFFER 512
#define TEST_CLI_APP "cli-test"

int
run_banking_app(void)
{
	char command_buffer[MAXCMDBUFFER] = {0};
	int rcode;

	rcode = path_constructor(command_buffer, 
				 TEST_CLI_APP, 
				 BANKING_APP);
	if (rcode != 0)
		return -1;

	fprintf(stdout, "execing %s\n", command_buffer);	
	rcode = system(command_buffer);

	if (rcode == 0)
		fprintf(stdout, "executing: %s\n", command_buffer);
	else {
		fprintf(stdout, "an error occured executing: %s\n", command_buffer);
		return -1;
	}

	return 0;
}
