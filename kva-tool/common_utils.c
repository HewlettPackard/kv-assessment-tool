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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <common_utils.h>

int
path_constructor(char result_buf[], 
		 char *file_name,
		 PATH_OPS op)
{
	const char *abs_path = NULL;
	abs_path = getenv(PWD);

	if (abs_path == NULL) {
		fprintf(stderr, "unable to get PWD env variable\n");
		return -1;
	}

	switch(op) {
	case TEST_PHYSMAP:
		snprintf(result_buf, MAXPATHLEN, "%s%s%s", 
			 abs_path,
			 TEST_PHYSMAP_DIR, 
			 file_name);
		break;
	case BANKING_APP:
		snprintf(result_buf, MAXPATHLEN, "%s%s%s", 
			 abs_path,
			 TEST_CLI_DIR,
			 file_name);
		break;
	case MMSEARCH_MODULE:
		snprintf(result_buf, MAXPATHLEN, "%s%s%s", 
			 abs_path,
			 MMSEARCH_DIR,
			 file_name);
		break;
	default:
		fprintf(stdout, "Programmer error %s undefined path operation\n",
			__func__);
		return -1;
	}

#if defined(DEBUG)
	printf("path and file name %s: \n", result_buf);
#endif

	return 0;
}


