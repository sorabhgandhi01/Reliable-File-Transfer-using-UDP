#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>

#define BUF_SIZE 2048

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
	struct timeval t_out = {0, 0};

	char cmd_send[50];
	char flname[20];
	char cmd[10];
	char ack_send[4] = "ACK";
	
	ssize_t numRead = 0;
	ssize_t length = 0;
	off_t f_size = 0;
	long int ack_num = 0;
	int cfd, ack_recv = 0;

	FILE *fptr;

	memset(ack_send, 0, sizeof(ack_send));
	memset(&send_addr, 0, sizeof(send_addr));
	memset(&from_addr, 0, sizeof(from_addr));

	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(atoi(argv[2]));
	send_addr.sin_addr.s_addr = inet_addr(argv[1]);

	if ((cfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		print_error("CLient: socket");

	/*if (fcntl(cfd, F_SETFL, O_NONBLOCK) == -1)
		print_error("Client: fcntl");*/

	for (;;) {

		memset(cmd_send, 0, sizeof(cmd_send));
		memset(cmd, 0, sizeof(cmd));
		memset(flname, 0, sizeof(flname));

		printf("\n Menu \n Enter any of the following commands \n 1.) get [file_name] \n 2.) put [file_name] \n 3.) delete [file_name] \n 4.) ls \n 5.) exit \n");		
		scanf(" %[^\n]%*c", cmd_send);

		//printf("----> %s\n", cmd_send);
		
		sscanf(cmd_send, "%s %s", cmd, flname);

		if (sendto(cfd, cmd_send, sizeof(cmd_send), 0, (struct sockaddr *) &send_addr, sizeof(send_addr)) == -1) {
			print_error("Client: send");
		}

/*----------------------------------------------------------------------"get case"-------------------------------------------------------------------------*/

		if ((strcmp(cmd, "get") == 0) && (flname[0] != '\0' )) {

			long int total_frame = 0;
			long int bytes_rec = 0, i = 0;

			t_out.tv_sec = 2;
			setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval)); 

			recvfrom(cfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);

			t_out.tv_sec = 0;
                	setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval)); 
			
			if (total_frame > 0) {
				sendto(cfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
				printf("----> %ld\n", total_frame);
				
				fptr = fopen(flname, "wb");

				for (i = 1; i <= total_frame; i++)
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

					if (i == total_frame) {
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

/*------------------------------------------------------------------"put case"---------------------------------------------------------------------------*/

		else if ((strcmp(cmd, "put") == 0) && (flname[0] != '\0')) {
			
			if (access(flname, F_OK) == 0) {
				int total_frame = 0, resend_frame = 0, drop_frame = 0, t_out_flag = 0;
				long int i = 0;

				stat(flname, &st);
				f_size = st.st_size;

				t_out.tv_sec = 2;
				t_out.tv_usec = 0;
				setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval)); 

				fptr = fopen(flname, "rb");

				if ((f_size % BUF_SIZE) != 0)
					total_frame = (f_size / BUF_SIZE) + 1;
				else
					total_frame = (f_size / BUF_SIZE);

				printf("Total number of packets ---> %d	File size --> %ld\n", total_frame, f_size);

				sendto(cfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
				recvfrom(cfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length);

				printf("Ack num ---> %ld\n", ack_num);

				while (ack_num != total_frame)
				{
					sendto(cfd, &(total_frame), sizeof(total_frame), 0, (struct sockaddr *) &send_addr, sizeof(send_addr));
					recvfrom(cfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &from_addr, (socklen_t *) &length); 
					
					resend_frame++;

					if (resend_frame == 20) {
						t_out_flag = 1;
						break;
					}
				}

				for (i = 1; i <= total_frame; i++)
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
						printf("frame ---> %ld	dropped, %d times\n", frame.ID, ++drop_frame);
						resend_frame++;

						if (resend_frame == 200) {
							t_out_flag = 1;
							break;
						}
					}
					drop_frame = 0;
					resend_frame = 0;

					if (t_out_flag == 1) {
						printf("File not sent\n");
						break;
					}

					printf("frame ----> %ld	Ack ----> %ld\n", i, ack_num);

					if (total_frame == ack_num)
						printf("File sent\n");
				}
				fclose(fptr);
				
				printf("Disable the timeout\n");
				t_out.tv_sec = 0;
				setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&t_out, sizeof(struct timeval)); 
			}
		}

/*----------------------------------------------------------------------"delete case"-------------------------------------------------------------------------*/

		else if ((strcmp(cmd, "delete") == 0) && (flname[0] != '\0')) {

			length = sizeof(from_addr);
			ack_recv = 0;
                                                                                                                                 
			if((numRead = recvfrom(cfd, &(ack_recv), sizeof(ack_recv), 0,  (struct sockaddr *) &from_addr, (socklen_t *) &length)) < 0)
				print_error("recieve");
			
			if (ack_recv > 0)
				printf("Client: Deleted the file\n");
			else if (ack_recv < 0)
				printf("Client: Invalid file name\n");
			else
				printf("Client: File does not have appropriate permission\n");
		}

/*-----------------------------------------------------------------------"ls case"----------------------------------------------------------------------------*/

		else if (strcmp(cmd, "ls") == 0) {

			char filename[200];
			memset(filename, 0, sizeof(filename));

			length = sizeof(from_addr);

			if ((numRead = recvfrom(cfd, filename, sizeof(filename), 0,  (struct sockaddr *) &from_addr, (socklen_t *) &length)) < 0)
				print_error("recieve");

			if (filename[0] != '\0') {
				printf("Number of bytes recieved = %ld\n", numRead);
				printf("\nThis is the List of files and directories --> \n%s \n", filename);
			}
			else {
				printf("Recieved buffer is empty\n");
				continue;
			}
		}

/*----------------------------------------------------------------------"exit case"-------------------------------------------------------------------------*/

		else if (strcmp(cmd, "exit") == 0) {

			exit(EXIT_SUCCESS);

		}

/*--------------------------------------------------------------------"Invalid case"-------------------------------------------------------------------------*/

		else {
			printf("--------Invalid Command!----------\n");
		}


	}
		
	close(cfd);

	exit(EXIT_SUCCESS);
}
