#include "c_utility.h"

struct frame_t {
	long int ID;
	long int length;
	char data[BUF_SIZE];
};

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
	struct stat st;
	struct frame_t frame;

	int sfd;
	long int ack_num = 0;
	ssize_t numRead;
	ssize_t length;
	off_t f_size;
	char ack_send[4] = "ACK";
	//char ack_recv[4];
	char cmd_recv[10];
	char flname_recv[20];
	char msg_recv[BUF_SIZE];

	FILE *fptr;

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

			if (flname_recv[0] != '\0') {
				
				if (access(flname_recv, F_OK) == 0) {

					int total_pckt = 0, repeat_send = 0;
					long int i = 0;
					
					stat(flname_recv, &st);
					f_size = st.st_size;			//Size of the file

					fptr = fopen(flname_recv, "rb");
					
					if ((f_size % BUF_SIZE) != 0)
						total_pckt = (f_size / BUF_SIZE) + 1;				//Number of packets to send
					else
						total_pckt = (f_size / BUF_SIZE);

					printf("Total number of packets ---> %d\n", total_pckt);
					
					length = sizeof(cl_addr);

					sendto(sfd, &(total_pckt), sizeof(total_pckt), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));	//Send number of packets (to be transmitted) to reciever
					
					if (recvfrom(sfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length) < 0) {
						printf("File not sent\n");
						break;
					}

					for(i = 1; i <= total_pckt; i++)
					{
						memset(&frame, 0, sizeof(frame));
						ack_num = 0;
						frame.ID = i;
						frame.length = fread(frame.data, 1, BUF_SIZE, fptr);

						sendto(sfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
						recvfrom(sfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length);

						while(ack_num != frame.ID)
						{
							sendto(sfd, &(frame), sizeof(frame), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
							recvfrom(sfd, &(ack_num), sizeof(ack_num), 0, (struct sockaddr *) &cl_addr, (socklen_t *) &length);

							repeat_send++;
							if (repeat_send == 10) {
								break;
							}
						}

						printf("frame ----> %ld	Ack ----> %ld \n", i, ack_num);

						if (total_pckt == ack_num)
							printf("File sent\n");
					}
				}
				else {
					//sendto(sfd, "Invalid Filename", 16, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
				}
			}
			else {
				//sendto(sfd, "No filename given", 17, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
			}

		}
		else if (strcmp(cmd_recv, "put") == 0) {
			print_msg("Server: Put called with file name --> %s\n", flname_recv);

			if (sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");
		}
		else if (strcmp(cmd_recv, "delete") == 0) {

			if (flname_recv[0] == '\0') {
				print_msg("No file given\n");
				sendto(sfd, "No filename given", 17, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
			}
			else {
				if(access(flname_recv, F_OK) == -1)
					sendto(sfd, "Invalid Filename", 16, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
				else{
					if(access(flname_recv, R_OK) == -1)
						sendto(sfd, "File does not have read permission", 34, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
					else {
						print_msg("Filename is %s\n", flname_recv);
						remove(flname_recv);
						sendto(sfd, ack_send, sizeof(ack_send), 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr));
					}
				}
			}
		}
		else if (strcmp(cmd_recv, "ls") == 0) {
			
			char file_entry[200];
			memset(file_entry, 0, sizeof(file_entry));

			fptr = fopen("a.log", "wb");

			if (ls(fptr) == -1)
				print_error("ls");

			fclose(fptr);
			
			fptr = fopen("a.log", "rb");	
			int filesize = fread(file_entry, 1, 200, fptr);

			print_msg("Filesize = %d	%ld\n", filesize, strlen(file_entry));
			
			if (sendto(sfd, file_entry, filesize, 0, (struct sockaddr *) &cl_addr, sizeof(cl_addr)) == -1)
				print_error("Server: send");

			remove("a.log");
			fclose(fptr);
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
