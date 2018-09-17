#include "c_utility.h"

int ls(FILE *f) 
{ 
	struct dirent **dirent; int n = 0;

	if ((n = scandir(".", &dirent, NULL, alphasort)) < 0) { 
		perror("Scanerror"); 
		return -1; 
	}
        
	while (n--) {
		//printf("%s		%d\n", dirent[n]->d_name, dirent[n]->d_type);

		fprintf(f, "%s\n", dirent[n]->d_name);

		free(dirent[n]); 
	}
	
	free(dirent); 
	return 0; 
}                                             

static void print_msg(const char *msg, ...)
{
	va_list va_args;
	va_start(va_args, msg);
	vprintf(msg, va_args);
	va_end(va_args);
	fflush(stdout);
}

static void print_error(const char *msg, ...)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if ((argc < 2) || (argc > 2)) {
		print_msg("Usage --> ./[%s] [Port Number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sv_addr, cl_addr;
	int sfd;
	ssize_t numRead;
	ssize_t length;
	char ack_send[4] = "ACK";
	//char ack_recv[4];
	char cmd_recv[10];
	char flname_recv[20];
	char msg_recv[BUF_SIZE];

	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(atoi(argv[1]));
	sv_addr.sin_addr.s_addr = INADDR_ANY;

	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		print_error("Server: socket");

	if (bind(sfd, (struct sockaddr *) &sv_addr, sizeof(sv_addr)) == -1)
		print_error("Server: bind");

	for(;;) {
		print_msg("Server: Waiting for client to connect\n");

		memset(msg_recv, 0, sizeof(msg_recv));
		memset(cmd_recv, 0, sizeof(cmd_recv));
		memset(flname_recv, 0, sizeof(flname_recv));

		length = sizeof(cl_addr);

		if((numRead = recvfrom(sfd, msg_recv, BUF_SIZE, 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length)) == -1)
			print_error("Server: recieve");

		//print_msg("Server: Recieved %ld bytes from %s\n", numRead, cl_addr.sin_addr.s_addr);
		print_msg("Server: The recieved message ---> %s\n", msg_recv);

		sscanf(msg_recv, "%s %s", cmd_recv, flname_recv);

		if (strcmp(cmd_recv, "get") == 0) {
			print_msg("Server: Get called with file name --> %s\n", flname_recv);

			if (sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");

		}
		else if (strcmp(cmd_recv, "put") == 0) {
			print_msg("Server: Put called with file name --> %s\n", flname_recv);

			if (sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");
		}
		else if (strcmp(cmd_recv, "delete") == 0) {

			if (flname_recv == NULL)
				print_msg("No file given\n");

			if(access(flname_recv, F_OK) == -1)
				sendto(sfd, "Invalid Filename", 16, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));			
#if 0
			if(access(flname_recv, R_OK) == -1)
				sendto(sfd, "File does not have read permission", 34, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
#endif
			else {
				printf("Filename is %s\n", flname_recv);

				remove(flname_recv);
				sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
			}
		}
		else if (strcmp(cmd_recv, "ls") == 0) {
			
			char file_entry[200];
			memset(file_entry, 0, sizeof(file_entry));
			FILE *list;

			list = fopen("a.log", "wb");

			if (ls(list) == -1)
				print_error("ls");

			fclose(list);
			
			list = fopen("a.log", "rb");	
			int filesize = fread(file_entry, 1, 199, list);

			print_msg("Filesize = %d	%ld\n", filesize, strlen(file_entry));
			
			if (sendto(sfd, file_entry, filesize, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");

			remove("a.log");
			fclose(list);
		}
		else if (strcmp(cmd_recv, "exit") == 0) {
			if (sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");

			close(sfd);
			exit(EXIT_SUCCESS);
		}
		else {
			print_msg("Server: Unkown command. Please try again\n");

			if (sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");
		}
		//
	}
	
	close(sfd);
	exit(EXIT_SUCCESS);
}
