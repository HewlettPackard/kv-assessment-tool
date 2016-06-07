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

#if !defined(COMMON_UTILS_H)
#define COMMON_UTILS_H

#define MAXPATHLEN 256
#define MAXCMDBUFFER 512
#define PWD "PWD"
#define TEST_PHYSMAP_DIR "/physmap-test/"
#define TEST_CLI_DIR "/cli-test/"
#define MMSEARCH_DIR "/mmsearch/"

enum path_ops {
	TEST_PHYSMAP = 0,
	BANKING_APP,
	MMSEARCH_MODULE,
};

typedef enum path_ops PATH_OPS;

int
path_constructor(char result_buf[], 
		 char *module_name,
		 PATH_OPS op);

#endif
