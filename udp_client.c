/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len, float *time); 

int main(int argc, char **argv)
{
	int len, sockfd, timeout_len = 10; // set timeout here
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;

	float time, throughput;
	FILE *fp;

	if (argc != 2) {
		printf("parameters not match");
		exit(0);
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             //get host's information
		printf("error when get hostby name");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);			//printf socket information
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	// Configure timeout
	struct timeval tv;
	tv.tv_sec = timeout_len;	// timeout of 10s
	tv.tv_usec = 0;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	}
  
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	
	// open file
	if((fp = fopen ("send.txt", "rt")) == NULL) {
		printf("File not found\n");
		exit(0);
	}

	str_cli1(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len, &time);   // receive and send
    throughput = 8 * (len / time) / 1000;

	printf("\nTime: %.3fms, Throughput: %f Mbps\nData Unit Size: %d Bytes, Data Sent: %d Bytes\n", time, throughput, PACKET_SIZE, (int)len);

	close(sockfd);
	fclose(fp);	

	// open file
	if((fp = fopen ("time.csv", "a")) == NULL) {
		printf("File not found\n");
		exit(0);
	}

	fprintf(fp, "\n%.3f, %f, %d, %d", time, throughput, PACKET_SIZE, (int)len);
	fclose(fp);
	exit(0);
}

// new
void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len, float *time) {
	
	char *buf;

	long fsize, idx = 0;   // file size and index of buf
	char send_buf[PACKET_SIZE];   // send up to 1000 bytes
	uint8_t ack;

	int n, strlen;  
	float elapsedTime;
	struct timeval start_time, end_time;
    
	fseek (fp , 0 , SEEK_END);	
	fsize = ftell (fp);	// bytes to end of file
	rewind (fp);

    printf("File Size: %d bytes\n", (int)fsize);
	printf("Packet Size: %d bytes\n", PACKET_SIZE);

    // allocate memory to contain the whole file.
	buf = (char*) malloc(fsize + 1);
	if (buf == NULL) exit (2);	// exit on failure
	fread (buf, 1, fsize, fp); 	// copy the file into the buffer.

	buf[fsize] = '\0';				// append the end byte (extra byte sent)
	gettimeofday(&start_time, NULL);		// get the current time		

	while(idx <= fsize) {	// while index is within file size
        if ((fsize + 1 - idx) <= PACKET_SIZE)
            strlen = fsize + 1 - idx;
		else
            strlen = PACKET_SIZE;

        printf("\nstrlen = %d, idx = %ld\n", strlen, idx);
        memcpy(send_buf, (buf + idx), strlen);
        
		//send the data
		n = sendto(sockfd, &send_buf, strlen, 0, addr, addrlen);
		if(n == -1) {
			printf("Send error!");								
			exit(1);
		}
		printf("Sent a packet\n");
        
        //receive the ack
		n = recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t*)&addrlen);
		if(n == -1) {
			printf("Timeout!\n");								
			exit(1);
		} else if (ack == 1) {
			//ACK
			idx += strlen;
			printf("Received an ACK\n");
		} else if (ack == 255) {
			//NACK -- Do not progress until ACK is received
			printf("Received an NACK\n");
		}
	}

    //get current time
    gettimeofday(&end_time, NULL);  
	*len = idx;                         

	elapsedTime = (end_time.tv_sec - start_time.tv_sec) * 1000.0;      // sec to ms
    elapsedTime += (end_time.tv_usec  - start_time.tv_usec) / 1000.0;   // us to ms
	*time = elapsedTime;
}
