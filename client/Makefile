all : client
objects = *.o

client : client.o
	cc -Wall -Werror -o client client.o

client.o : client.c
	cc -Wall -Werror $(INC) -c client.c

clean :
	rm -f client $(objects) *.txt *.log
