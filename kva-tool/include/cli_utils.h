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

#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

struct cmdObject {
	unsigned long startAddress;
	unsigned long endAddress;
	unsigned long num_bytes;
	pid_t pid_num;
};

void
show_prompt(void);

char *
read_cli_input(void);

int
root_check(void);

char *
next_token(char **stringp,
	   const char *delim);

int
check_addr(char *addr);

int
process_cli_cmd(char *cmd);

#endif /* CLI_UTILS_H */
