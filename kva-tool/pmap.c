/* Copyright (c) 2002 Andrew Isaacson <adi@hexapodia.org>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int
print_name_pid(pid_t pid)
{
    char name[PATH_MAX];
    int c, i = 0;
    FILE *f;

    sprintf(name, "/proc/%ld/cmdline", (long) pid);
    f = fopen(name, "r");
    if(!f) {
	    fprintf(stderr, "%s: %s\n", name, strerror(errno));
	    return -1;
    }
    while((c = getc(f)) != EOF && c != 0)
	name[i++] = c;
    name[i] = '\0';
    printf("%s(%ld)\n", name, (long)pid);
    fclose(f);

    return 0;
}

int
print_maps(pid_t pid)
{
    char fname[PATH_MAX];
    unsigned long writable = 0, total = 0, shared = 0;
    FILE *f;

    sprintf(fname, "/proc/%ld/maps", (long)pid);
    f = fopen(fname, "r");

    if(!f) {
	    fprintf(stderr, "%s: %s\n", fname, strerror(errno));
	    return -1;
    }

    while(!feof(f)) {
	char buf[PATH_MAX+100], perm[5], dev[6], mapname[PATH_MAX];
	unsigned long begin, end, size, inode, foo;
	int n;

	if(fgets(buf, sizeof(buf), f) == 0)
	    break;
	mapname[0] = '\0';
	sscanf(buf, "%lx-%lx %4s %lx %5s %lu %s", &begin, &end, perm,
		&foo, dev, &inode, mapname);
	size = end - begin;
	total += size;
	/* the permission string looks like "rwxp", where each character can
	 * be either the letter, or a hyphen.  The final character is either
	 * p for private or s for shared.  We want to add up private writable
	 * mappings, to get a feel for how much private memory this process
	 * is taking.
	 *
	 * Also, we add up the shared mappings, to see how much this process
	 * is sharing with others.
	 */
	if(perm[3] == 'p') {
	    if(perm[1] == 'w')
		writable += size;
	} else if(perm[3] == 's')
	    shared += size;
	else {
		fprintf(stderr, "unable to parse permission string: '%s'\n", perm);
	}

	n = printf("%08lx (%ld KB)", begin, (end - begin)/1024);
	n += printf("%*s %s (%s %ld) ", 22-n, "", perm, dev, inode);
	printf("%*s %s\n", 44-n, "", mapname);
    }

    printf("mapped:   %ld KB writable/private: %ld KB shared: %ld KB\n",
	    total/1024, writable/1024, shared/1024);

    fclose(f);

    return 0;
}
