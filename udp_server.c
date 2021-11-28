/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/

#include "headsock.h"

double error_prob = 0.2;

void str_ser1(int sockfd, struct sockaddr *addr, int addrlen);     	// diff        

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {			//create socket
		printf("error in socket");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {           //bind socket
		printf("error in binding");
		exit(1);
	}
	
	printf("start receiving\n");
	str_ser1(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));

	close(sockfd);
	exit(0);
}


void str_ser1(int sockfd, struct sockaddr *addr, int addrlen)
{
	long offset = 0;
	double random = 0.0;
	int n = 0;
	char buf[100000];	// receive up to 100000 bytes
	FILE *fp;

	char receive_buf[PACKET_SIZE];	// receive up to 500 bytes
	uint8_t ack;

	if ((fp = fopen ("receive.txt", "wt")) == NULL)
	{
		printf("File doesn't exist\n");
		exit(0);
	}
	
	while(1) {
		// receive packet
		if ((n = recvfrom(sockfd, &receive_buf, PACKET_SIZE, 0, addr, (socklen_t *)&addrlen)) == -1) {
			printf("Error receiving\n");
			exit(1);
		}

		// error simulation
        random = (double) rand() / (RAND_MAX);
        printf("Random number is %f \n", random);

        if(random < error_prob) {
			// drop "damaged" packet
            printf("Damaged packet\n");
			ack = 255;
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen)) == -1) { // send NACK 
				printf("Send error!");								
				exit(1);
			}
			printf("Sent a NACK\n\n");
        }
		else if (receive_buf[n - 1] == '\0') {	// last packet received
			// parse packet
			memcpy((buf + offset), receive_buf, n);
			printf("Received %d bytes. offset = %d\n", n, (int)offset);
			offset += (n - 1);

			ack = 1;
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen)) == -1) {
				printf("Send error!");								
				exit(1);
			}
			printf("Sent an ACK\n\nLast packet!\n\n");
			break;

		} 
		else {
			// parse packet
			memcpy((buf + offset), receive_buf, n);
			printf("Received %d bytes. offset = %d\n", n, (int)offset);
			offset += n;

			ack = 1;
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen)) == -1) { // send ack 
				printf("Send error!");								
				exit(1);
			}
			printf("Sent an ACK\n\n");
		}
	}

	fwrite (buf , 1 , offset , fp);	//write data to file
	fclose(fp);

	printf("File received.\nTotal data received: %d bytes\n", (int)offset);
}