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

/* 
   kmm (kernel module manager)
*/

#define __GNU_SOURCE
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <common_utils.h>
#include <kmm.h>

#define INSMOD "/sbin/insmod"

int finit_module(int fd, const char *param_values,
		 int flags);

int
ld_kernel_module(char *module_to_load, int *fd_in)
{
	long rcode = 0;
	int fd;

#if defined(DEBUG)	
	printf("loading module %s\n", module_to_load);
#endif
	fd = open(module_to_load, O_RDONLY);
	if (fd < 0) {
		fprintf(stdout, "unable to load %s\n", module_to_load);
		return -1;
	} else {
		fprintf(stdout, "loading: %s\n", module_to_load);
	}

	*fd_in = fd;
	
	rcode = syscall(SYS_finit_module, fd, "", 0);
	if (rcode != 0)
		return -1;
	
	return 0;
}


int
unload_kernel_module(char *module_name)
{
	long rcode;

	int flags = 0;
	rcode = syscall(SYS_delete_module, module_name, flags);
	if (rcode != 0) {
		fprintf(stdout, "failed to unload: %s\n", module_name);		
		return -1;
	} else {
		fprintf(stdout, "unloading: %s\n", module_name);
	}
	
	return 0;
}

