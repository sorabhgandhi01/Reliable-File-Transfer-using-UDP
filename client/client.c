#include "c_utility.h"

void print_error(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	if ((argc < 3) || (argc >3)) {
		printf("Client: Usage --> ./[%s] [IP Address] [Port Number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in send_addr, from_addr;
	ssize_t numRead;
	ssize_t length;
	int cfd;
	char ack_send[4] = "ACK";
	char cmd[10];
	char flname[20];
	char ack_recv[50];
	char cmd_send[50];
	//char msg_send[BUF_SIZE];

	memset(ack_send, 0, sizeof(ack_send));
	memset(ack_recv, 0, sizeof(ack_recv));
	memset(&send_addr, 0, sizeof(send_addr));
	memset(&from_addr, 0, sizeof(from_addr));

	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(atoi(argv[2]));
	send_addr.sin_addr.s_addr = inet_addr(argv[1]);

	if ((cfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		print_error("CLient: socket");

	for (;;) {

		memset(cmd_send, 0, sizeof(cmd_send));
		memset(ack_recv, 0, sizeof(ack_recv));
		memset(cmd, 0, sizeof(cmd));
		memset(flname, 0, sizeof(flname));

		printf("\n Menu \n Enter any of the following commands \n 1.) get [file_name] \n 2.) put [file_name] \n 3.) delete [file_name] \n 4.) ls \n 5.) exit \n");		
		scanf("%[^\n]%*c", cmd_send);

		sscanf(cmd_send, "%s %s", cmd, flname);

		if (sendto(cfd, cmd_send, sizeof(cmd_send), 0, (struct sockaddr *) &send_addr, sizeof(send_addr)) == -1)
			print_error("Client: send");


		if ((strcmp(cmd, "get")) && (flname != NULL) == 0) {


		}
		else if ((strcmp(cmd, "put")) && (flname != NULL) == 0) {


		}
		else if (strcmp(cmd, "delete") == 0) {

			memset(ack_recv, 0, sizeof(ack_recv));
			length = sizeof(from_addr);
                                                                                                                                 
			if((numRead = recvfrom(cfd, ack_recv, sizeof(ack_recv), 0,  (struct sockaddr *) &from_addr, (socklen_t *) &length)) < 0)
				print_error("recieve");

			printf("Client: Message recieved --> %s\n", ack_recv);
		}
		else if (strcmp(cmd, "ls") == 0) {

			char filename[200];
			memset(filename, 0, sizeof(filename));

			length = sizeof(from_addr);

			if((numRead = recvfrom(cfd, filename, sizeof(filename), 0,  (struct sockaddr *) &from_addr, (socklen_t *) &length)) < 0)
				print_error("recieve");

			printf("Number of bytes recieved = %ld\n", numRead);
			printf("\nThis is the List of files and directories --> \n%s \n", filename);
		}
		else if (strcmp(cmd, "exit") == 0) {

			exit(EXIT_SUCCESS);

		}
		else {
			printf("--------Invalid Command!----------\n");
		}

	}
		
	close(cfd);

	exit(EXIT_SUCCESS);
}
