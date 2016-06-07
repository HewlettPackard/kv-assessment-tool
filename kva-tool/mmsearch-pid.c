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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MMSEARCH_PID_FILE "/sys/kernel/mmsearch_pid/mmsearch_pid" 
#define BUFSZ 16

int
mmsearch_pid(char *pid)
{
	size_t slen = 0;
	size_t numbytes_written = 0;
	char buf[BUFSZ] = {0};

	FILE *fd = fopen(MMSEARCH_PID_FILE, "w");
	if (fd == NULL) {
		fprintf(stdout, "error (%s) opening %s\n", strerror(errno),
			MMSEARCH_PID_FILE);
		return -1;
	}

	slen = strlen(pid);
	memcpy(buf, pid, slen-1);
	numbytes_written = fwrite(pid, sizeof(char), slen, fd);
	
	if (numbytes_written == 0) {
		fclose(fd);
		return -1;
	}

	fclose(fd);
	return 0;
}
