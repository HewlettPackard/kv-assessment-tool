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

#include "cli_utils.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

extern struct cmdObject co;

int
main(void)
{
	char *input;
	int ret_val = 1;

	if (!root_check())
		return 0;

	fprintf(stdout, "running as PID %d\n", getpid());

	do {
		show_prompt();
		input = read_cli_input();
		ret_val = process_cli_cmd(input);
	} while (ret_val != 0);

	return 0;
}
