#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>      // socket creation
#include <errno.h>           // error codes
#include <stdbool.h>         // boolean types
#include <netinet/ip_icmp.h> // provides declarations for icmp header
#include <netinet/udp.h>     // provides declarations for udp header
#include <netinet/tcp.h>     // provides declarations for tcp header
#include <netinet/ip.h>      // provides declarations for ip header
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 65536

int tcp = 0;
int udp = 0;
int icmp = 0;
int igmp = 0;
int others = 0;
int total = 0;

void check_socket(int socket);
void print_ip_header(unsigned char *buffer, struct sockaddr_in *source, struct sockaddr_in *destination);
void process_packet(unsigned char *buffer, size_t size, struct sockaddr_in *source, struct sockaddr_in *destination);
void print_tcp_packet(unsigned char *buffer, size_t size, struct sockaddr_in *source, struct sockaddr_in *destination);
void print_data(unsigned char *data , unsigned short size);

int main(int argc, char *argv[])
{
    int i;
    int raw_socket;
    unsigned char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    struct sockaddr addr;
    struct sockaddr_in *source = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    struct sockaddr_in *destination = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    socklen_t addr_size = (socklen_t)sizeof(addr);
    size_t buffer_len;

    /* Creates raw socket
    AF_INET - IP protocol
    SOCK_RAW - raw socket
    IPPROTO_TCP - TCP protocol
    IPPROTO_UDP - UDP protocol
    IPPROTO_ICMP - ICMP protocol
    */
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    check_socket(raw_socket);

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        bytes_read = recvfrom(raw_socket, buffer, sizeof(buffer), 0, &addr, &addr_size);
        if (bytes_read < 0)
        {
            perror("Error receiving data");
            printf("Error code: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        printf("\n\nBytes read: %ld\n", bytes_read);
        process_packet(buffer, bytes_read, source, destination);
        //for (i = 0; i < bytes_read; i++)
        //    putchar(((char *)buffer)[i]); // raw binary data
    }

    close(raw_socket);

    return 0;
}

void check_socket(int socket)
{
    if (socket < 0)
    {
        perror("Socket criation error");
        printf("Error code: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket created\n");
    }
}

void process_packet(unsigned char *buffer, size_t size, struct sockaddr_in *source, struct sockaddr_in *destination)
{
  struct iphdr *ip_header = (struct iphdr *) buffer;
  total++;

  switch (ip_header->protocol) // checks protocol
  {
    case 1: // ICMP protocol
      icmp++;
      break;

    case 2: // IGMP protocol
      igmp++;
      break;

    case 6: // TCP protocol
      tcp++;
      print_tcp_packet(buffer, size, source, destination);
      break;

    case 17: // UDP protocol
      udp++;
      break;

    default: // other protocols
      others++;
      break;
  }
  printf("TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r",tcp,udp,icmp,igmp,others,total);
}

void print_tcp_packet(unsigned char *buffer, size_t size, struct sockaddr_in *source, struct sockaddr_in *destination)
{
  unsigned short ip_header_length;

  struct iphdr *ip_header = (struct iphdr*)buffer;
  ip_header_length = ip_header->ihl*4;

  struct tcphdr *tcp_header = (struct tcphdr*)(buffer + ip_header_length);

  printf("\n\n***********************TCP Packet*************************\n");

  print_ip_header(buffer, source, destination);

  printf("\n");
	printf("TCP Header\n");
	printf("   |-Source Port          : %u\n",ntohs(tcp_header->source));
	printf("   |-Destination Port     : %u\n",ntohs(tcp_header->dest));
	printf("   |-Sequence Number      : %u\n",ntohl(tcp_header->seq));
	printf("   |-Acknowledge Number   : %u\n",ntohl(tcp_header->ack_seq));
	printf("   |-Header Length        : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp_header->doff,(unsigned int)tcp_header->doff*4);
	printf("   |-Urgent Flag          : %d\n",(unsigned int)tcp_header->urg);
	printf("   |-Acknowledgement Flag : %d\n",(unsigned int)tcp_header->ack);
	printf("   |-Push Flag            : %d\n",(unsigned int)tcp_header->psh);
  printf("   |-Reset Flag           : %d\n",(unsigned int)tcp_header->rst);
	printf("   |-Synchronise Flag     : %d\n",(unsigned int)tcp_header->syn);
	printf("   |-Finish Flag          : %d\n",(unsigned int)tcp_header->fin);
	printf("   |-Window               : %d\n",ntohs(tcp_header->window));
	printf("   |-Checksum             : %d\n",ntohs(tcp_header->check));
  printf("   |-Urgent Pointer       : %d\n",tcp_header->urg_ptr);
	printf("\n");
	printf("                        DATA Dump                         ");
	printf("\n");

  printf("IP Header\n");
  print_data(buffer, ip_header_length);

  printf("TCP Header\n");
  print_data(buffer + ip_header_length, tcp_header->doff*4);

  printf("Data Payload\n");
  print_data(buffer + ip_header_length + tcp_header->doff*4, (size - tcp_header->doff*4-ip_header->ihl*4));

  printf("\n###########################################################\n");
}

void print_ip_header(unsigned char *buffer, struct sockaddr_in *source, struct sockaddr_in *destination)
{
  unsigned short ip_header_length;
  struct iphdr *ip_header = (struct iphdr *) buffer;
  ip_header_length = ip_header->ihl*4;

  memset(source, 0, sizeof(source));
  source->sin_addr.s_addr = ip_header->saddr;

  memset(destination, 0, sizeof(destination));
  destination->sin_addr.s_addr = ip_header->daddr;

  printf("IP Header\n");
	printf("   |-IP Version        : %d\n",(unsigned int)ip_header->version);
	printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)ip_header->ihl,((unsigned int)(ip_header->ihl))*4);
	printf("   |-Type Of Service   : %d\n",(unsigned int)ip_header->tos);
	printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(ip_header->tot_len));
	printf("   |-Identification    : %d\n",ntohs(ip_header->id));
	printf("   |-TTL               : %d\n",(unsigned int)ip_header->ttl);
	printf("   |-Protocol          : %d\n",(unsigned int)ip_header->protocol);
	printf("   |-Checksum          : %d\n",ntohs(ip_header->check));
	printf("   |-Source IP         : %s\n",inet_ntoa(source->sin_addr));
	printf("   |-Destination IP    : %s\n",inet_ntoa(destination->sin_addr));
}

void print_data(unsigned char *data , unsigned short size)
{
  int i, j;

	for(i = 0; i < size; i++) {
		if((i != 0) && (i % 16 == 0)) {   //if one line of hex printing is complete...
			printf("         ");
			for(j = i - 16; j < i; j++) {
				if(data[j] >= 32 && data[j] <= 128) {
				      printf("%c",(unsigned char)data[j]); //if its a number or alphabet
        }
				else {
          printf("."); //otherwise print a dot
        }
			}
			printf("\n");
		}
		if(i % 16 == 0) {
      printf("   ");
    }
		printf(" %02X",(unsigned int)data[i]);
		if(i == size-1) { //print the last spaces
			for (j = 0; j < 15 - i % 16; j++) {
        printf("   "); //extra spaces
      }
			printf("         ");
			for(j = i - i % 16; j <= i; j++) {
				if(data[j] >= 32 && data[j] <= 128) {
          printf("%c",(unsigned char)data[j]);
        }
				else {
          printf(".");
        }
			}
			printf("\n");
		}
	}
}
