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

#include <cli_utils.h>
#include <kmm.h>
#include <proc_info.h>
#include <sys_info.h>
#include <run_apps.h>
#include <mmsearch_pid.h>

#include <common_utils.h>
#include <errno.h>

#define CMD_BUFFER_SZ 64

struct cmdObject co;

static struct module_info mmsearch_module_info;

void
show_prompt(void)
{
	printf("kva-tool> ");
	return;
}

int
root_check(void)
{
	uid_t uid;
	uid_t euid;

	if (((uid = getuid()) != 0) ||
	    ((euid = geteuid()) != 0)) {
		printf("kvtool must be run as root\n");
		return 0;
	}

	return 1;
}

char *
read_cli_input(void)
{
	char *input = malloc(CMD_BUFFER_SZ);
	fgets(input, CMD_BUFFER_SZ, stdin);

	return input;
}

char *
next_token(char **stringp,
	   const char *delim)
{
	char *res;

	do {
		res = strsep(stringp, delim);
		if (res == NULL)
			break;
	} while (*res == '\0');

	return res;
}

static pid_t 
str_to_pid(char *input)
{
	pid_t pidNum = 0;
	char *ptr;

	if (input != NULL) {
		ptr = next_token(&input, " \t\r\n");
		if (ptr != NULL) {
			pidNum = (pid_t)atoi(ptr);
		} else {
			printf("Must provide process identifier\n");
		}
	}

	return pidNum;
}

static void
print_help(void)
{
	printf(
	       "\tlkm-mmsearch\n"
	       "\t\tdescr: loads the mmsearch kernel module\n\n"
	       "\trkm-mmsearch\n"
	       "\t\tdescr: unloads the mmsearch kernel module\n\n"
	       "\tmmsearch-pid <proc_id>\n"
	       "\t\tdescr: searches a given process' memory space\n"
	       "\t\t       depends on mmsearch kernel module\n\n"
	       "\tpagemapinfo <proc_id>\n"
	       "\t\tdescr: outputs contents virtual memory addresses\n\n"
	       "\tprocinfo <proc_id>\n"
	       "\t\tdescr: displays detailed process information\n\n"
	       "\tproclist\n"
	       "\t\tdescr: displays the set of running processes\n" 
	       "\t\t       (akin to ps)\n\n"
	       "\tsysinfo\n"
	       "\t\tdescr: critical system information for hackers\n\n"
	       "\tquit\n"
	       "\t\tdescr: to exit\n\n"
	       );

	return;
}

int
process_cli_cmd(char *input)
{
	char *ptr;
	int ret_val;
	int fd;
	ptr = input;

	if (ptr != NULL) {
		ptr = next_token(&input, " \t\r\n");

		if (ptr == NULL)
			goto done;

		if (strcasecmp(ptr, "quit") == 0)
			return 0;

		if (strcasecmp(ptr, "sysinfo") == 0) {
			system_info();
			goto done;
		}

		if (strcasecmp(ptr, "proclist") == 0) {
			display_process_list();
			goto done;
		}

		if (strcasecmp(ptr, "procinfo") == 0) {
			if ((ret_val = get_proc_info(input)) == ENOMEM)
				printf("Failed to get memory\n");
			goto done;
		}

		if (strcasecmp(ptr, "pagemapinfo") == 0) {
			pid_t pidNum = str_to_pid(input);

			if (pidNum != 0) {
				if ((ret_val = print_name_pid(pidNum)) == -1)
					printf("Failed to get page map name\n");
				if ((ret_val = print_maps(pidNum)) == -1)
					printf("Failed to get page map info\n");
			}
			goto done;
		}

		if (strcasecmp(ptr, "rkm-mmsearch") == 0) {
			ret_val = unload_kernel_module("mmsearch");
			if (ret_val == 0) {
				memset(&mmsearch_module_info, 0, sizeof(struct module_info));
			}
			goto done;
		}

		if (strcasecmp(ptr, "lkm-mmsearch") == 0) {
			char module_to_load[MAXPATHLEN] = {0};
			ret_val = path_constructor(module_to_load,
						   "mmsearch.ko",
						   MMSEARCH_MODULE);
			if (ret_val < 0) {
				printf("Failed to get module path\n");
			} else {
				ret_val = ld_kernel_module(module_to_load, &fd);
				if (ret_val == 0) {
					memcpy(&mmsearch_module_info.module_name,
					       module_to_load,
					       MAX_MODULE_NAME);
					mmsearch_module_info.installed = 1;
					mmsearch_module_info.module_fd = fd;
				}
			}
			goto done;
		}

		if (strcasecmp(ptr, "mmsearch-pid") == 0) {
			if ((ret_val = mmsearch_pid(input) != 0))
				printf("File write failure\n");
			goto done;
		}

		if (strcasecmp(ptr, "help") == 0) {
			print_help();
			goto done;
		}

		printf("command not in list...\n");
	}

done:
	return 1;
}

