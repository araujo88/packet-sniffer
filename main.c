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

int main(int argc, char *argv[])
{
    int i;
    int raw_socket;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    struct sockaddr addr, source, destination;
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
        for (i = 0; i < bytes_read; i++)
            putchar(((char *)buffer)[i]); // raw binary data
    }

    close(raw_socket);

    return 0;
}

void process_packet(char *buffer, size_t size)
{
  struct iphdr *ip_header = (struct iphdr *) buffer;

  switch (ip_header->protocol) // checks protocol
  {
    case 1: // ICMP protocol
      break;

    case 2: // IGMP protocol
      break;

    case 6: // TCP protocol
      break;

    case 17: // UDP protocol
      break;

    default: // other protocols
      break;
  }
}

void print_ip_header(char *buffer, struct sockaddr source, struct sockaddr destination, size_t size)
{
  unsigned short ip_header_length;
  struct iphdr *ip_header = (struct iphdr *) buffer;
  ip_header_length = ip_header->ihl*4;

  memset(&source, 0, sizeof(source));
  source.sin_addr.s_addr = iph->saddr;

  memset(&destination, 0, sizeof(destination));
  destination.sin_addr.s_addr = iph->daddr;

  printf("IP Header\n");
	printf("   |-IP Version        : %d\n",(unsigned int)iph->version);
	printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
	printf("   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
	printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
	printf("   |-Identification    : %d\n",ntohs(iph->id));
	printf("   |-TTL      : %d\n",(unsigned int)iph->ttl);
	printf("   |-Protocol : %d\n",(unsigned int)iph->protocol);
	printf("   |-Checksum : %d\n",ntohs(iph->check));
	printf("   |-Source IP        : %s\n",inet_ntoa(source.sin_addr));
	printf("   |-Destination IP   : %s\n",inet_ntoa(destination.sin_addr));
}
