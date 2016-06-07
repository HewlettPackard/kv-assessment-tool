#-----------------------------------------------#
# shared make rules
# author: dsr
# date: 24-11-07
#----------------------------------------------#
CC = /usr/bin/gcc

CFLAGS = -g -v -std=gnu99 -Wall -Werror -Wshadow 	\
	-Wextra -Wunreachable-code -Wredundant-decls 	\
	-Wpointer-arith -Wswitch-default -Wswitch-enum 	\
	-Winline -Wundef -Wformat -Wformat-security    	\
	-pedantic

INCS = -I. 
RM = /bin/rm

.PHONY: clean TAGS

.SUFFIXES: .o .c

.c.o :
	$(CC) $(CFLAGS) $(INCS) -c $<





