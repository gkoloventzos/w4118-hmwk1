#Author: Georgios Koloventzos
#Mail: gkoloven@cs.columbia.edu

CC = gcc -Wall
SRCS = shell.c
OBJS = $(SRCS:.c=.o)
PROG = w4118_sh

objs : $(SRCS)
	$(CC) -c $(SRCS)
exec : $(OBJS)
	$(CC) -o $(PROG) $(OBJS)
all : objs exec
clean : 
	rm -f $(OBJS) $(PROG)
