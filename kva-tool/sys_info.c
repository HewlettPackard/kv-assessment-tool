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

#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <stdio.h>

void
system_info(void)
{
	struct utsname _uname;
	struct sysinfo si;
	int rc;

	rc = uname(&_uname);

	if (rc != 0) {
		printf("Error accessing uname information\n");
		return;
	}

	printf("processor: %s\n", _uname.machine);
	printf("release: %s\n", _uname.release);
	printf("version: %s\n", _uname.version);

	rc = sysinfo(&si);

	if (rc != 0) {
		printf("Error accessing system information\n");
		return;
	}

	printf("total ram: %ld\n", si.totalram);
	printf("free ram: %ld\n", si.freeram);
	printf("total swap: %ld\n", si.totalswap);
	printf("free swap: %ld\n", si.freeswap);
	printf("num current processes: %d\n", si.procs);
	printf("total high memory size: %ld\n", si.totalhigh);
	printf("free high memory: %ld\n", si.freehigh);
	printf("memory units size (bytes): %d\n", si.mem_unit);

	return;
}
