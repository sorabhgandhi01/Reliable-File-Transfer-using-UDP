default = server
objects = *.o

INC = -I../

server : server.o
	cc -Wall -Werror -o server server.o

server.o : server.c
	cc -Wall -Werror $(INC) -c server.c

clean :
	rm -f server $(objects) *.txt *.log
