#include "c_utility.h"

struct frame_t {
	long int ID;
	long int length;
	char data[BUF_SIZE];
};

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
	struct stat st;
	struct frame_t frame;

	ssize_t numRead;
	ssize_t length;
	off_t f_size = 0;
	int cfd;
	long int ack_num = 0;
	char ack_send[4] = "ACK";
	char cmd[10];
	char flname[20];
	char ack_recv[50];
	char cmd_send[50];
	//char msg_send[BUF_SIZE];

	FILE *fptr;

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


		if ((strcmp(cmd, "get") == 0) && (flname[0] != '\0' )) {

			long int total_pckt = 0, bytes_rec = 0, i = 0;

			recvfrom(cfd, &(total_pckt), sizeof(total_pckt), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);
			sendto(cfd, &(total_pckt), sizeof(total_pckt), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));

			if (total_pckt > 0) {
				fptr = fopen(flname, "wb");

				for (i = 1; i <= total_pckt; i++)
				{
					memset(&frame, 0, sizeof(frame));

					recvfrom(cfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);
					sendto(cfd, &(frame.ID), sizeof(frame.ID), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));

					if ((frame.ID < i) || (frame.ID > i))
						i--;
					else {
						fwrite(frame.data, 1, frame.length, fptr);
						printf("frame.ID ---> %ld	frame.length ---> %ld\n", frame.ID, frame.length);
						bytes_rec += frame.length;
					}

					if (i == total_pckt) {
						printf("File recieved\n");
					}
				}
				printf("Total bytes recieved ---> %ld\n", bytes_rec);
				fclose(fptr);
			}
			else {
				printf("File is empty\n");
			}
		}
		else if ((strcmp(cmd, "put") == 0) && (flname[0] != '\0')) {
			
			if (access(flname, F_OK) == 0) {
				int total_pckt = 0, repeat_send = 0;
				long int i = 0;

				stat(flname, &st);
				f_size = st.st_size;

				fptr = fopen(flname, "rb");

				if ((f_size % BUF_SIZE) != 0)
					total_pckt = (f_size / BUF_SIZE) + 1;
				else
					total_pckt = (f_size / BUF_SIZE);

				printf("Total number of packets ---> %d\n", total_pckt);

				sendto(cfd, &(total_pckt), sizeof(total_pckt), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
				if (recvfrom(cfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length) < 0) {
					printf("File not sent\n");
					break;
				}

				for (i = 1; i <= total_pckt; i++)
				{
					memset(&frame, 0, sizeof(frame));
					ack_num = 0;
					frame.ID = i;
					frame.length = fread(frame.data, 1, BUF_SIZE, fptr);

					sendto(cfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
                                        recvfrom(cfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);

					while (ack_num != frame.ID)
					{
						sendto(cfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
						recvfrom(cfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);

						repeat_send++;
						if (repeat_send == 10) {
							break;
						}
					}

					printf("frame ----> %ld	Ack ----> %ld\n", i, ack_num);

					if (total_pckt == ack_num)
						printf("File sent\n");
				}
			}
		}
		else if ((strcmp(cmd, "delete") == 0) && (flname[0] != '\0')) {

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
