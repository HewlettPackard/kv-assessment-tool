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

#ifndef PROC_INFO_H
#define PROC_INFO_H

#include <proc/readproc.h>

int
get_proc_info(char *input);

void
print_process_info(struct proc_t *proc);

void
display_process_list(void);

int
print_maps(pid_t pid);

int
print_name_pid(pid_t pid);

#endif /* PROC_INFO_H */
