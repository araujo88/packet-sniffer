#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>      // socket creation
#include <errno.h>           // error codes
#include <stdbool.h>         // boolean types
#include <netinet/ip_icmp.h> // provides declarations for icmp header
//#include <netinet/igmp.h>     // provides declarations for igmp header
#include <linux/igmp.h>
#include <netinet/udp.h>      // provides declarations for udp header
#include <netinet/tcp.h>      // provides declarations for tcp header
#include <netinet/ip.h>       // provides declarations for ip header
#include <netinet/if_ether.h> // for ETH_P_ALL
#include <net/ethernet.h>     // for ether_header
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h> // for interrupt signal handler

#define BUFFER_SIZE 65536

int raw_socket;
int tcp = 0;
int udp = 0;
int icmp = 0;
int igmp = 0;
int others = 0;
int total = 0;
struct sockaddr_in *source;
struct sockaddr_in *destination;

void check_socket(int socket);
void print_ethernet_header(unsigned char *buffer, ssize_t size);
void print_ip_header(unsigned char *buffer, ssize_t size);
void process_packet(unsigned char *buffer, ssize_t size);
void print_tcp_packet(unsigned char *buffer, ssize_t size);
void print_udp_packet(unsigned char *buffer, ssize_t size);
void print_icmp_packet(unsigned char *buffer, ssize_t size);
void print_igmp_packet(unsigned char *buffer, ssize_t size);
void print_data(unsigned char *data, ssize_t size);
void handle_signal(int sig);

int main(int argc, char *argv[])
{
  unsigned char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  struct sockaddr addr;
  socklen_t addr_size = (socklen_t)sizeof(addr);

  source = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  destination = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

  /* Creates raw socket
  AF_INET - IP protocol
  SOCK_RAW - raw socket
  IPPROTO_TCP - TCP protocol
  IPPROTO_UDP - UDP protocol
  IPPROTO_ICMP - ICMP protocol
  */
  // raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  check_socket(raw_socket);

  signal(SIGINT, handle_signal);
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
    process_packet(buffer, bytes_read);
    // for (i = 0; i < bytes_read; i++)
    //     putchar(((char *)buffer)[i]); // raw binary data
  }

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

void process_packet(unsigned char *buffer, ssize_t size)
{
  struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr)); // excludes ethernet header
  total++;

  switch (ip_header->protocol) // checks protocol
  {
  case 1: // ICMP protocol
    print_icmp_packet(buffer, size);
    icmp++;
    break;

  case 2: // IGMP protocol
    print_igmp_packet(buffer, size);
    igmp++;
    break;

  case 6: // TCP protocol
    tcp++;
    print_tcp_packet(buffer, size);
    break;

  case 17: // UDP protocol
    udp++;
    print_udp_packet(buffer, size);
    break;

  default: // other protocols
    others++;
    break;
  }
  printf("TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r", tcp, udp, icmp, igmp, others, total);
}

void print_tcp_packet(unsigned char *buffer, ssize_t size)
{
  size_t ethernet_header_length = sizeof(struct ethhdr);
  struct iphdr *ip_header = (struct iphdr *)(buffer + ethernet_header_length); // excludes ethernet header
  unsigned short ip_header_length = ip_header->ihl * 4;
  struct tcphdr *tcp_header = (struct tcphdr *)(buffer + ip_header_length + ethernet_header_length); // excludes ip and ethernet header
  size_t header_size = ethernet_header_length + ip_header_length + tcp_header->doff * 4;

  printf("\n\n***********************TCP Packet*************************\n");

  print_ip_header(buffer, size);

  printf("\n");
  printf("TCP Header\n");
  printf("   |-Source Port          : %u\n", ntohs(tcp_header->source));
  printf("   |-Destination Port     : %u\n", ntohs(tcp_header->dest));
  printf("   |-Sequence Number      : %u\n", ntohl(tcp_header->seq));
  printf("   |-Acknowledge Number   : %u\n", ntohl(tcp_header->ack_seq));
  printf("   |-Header Length        : %d DWORDS or %d BYTES\n", (unsigned int)tcp_header->doff, (unsigned int)tcp_header->doff * 4);
  printf("   |-Urgent Flag          : %d\n", (unsigned int)tcp_header->urg);
  printf("   |-Acknowledgement Flag : %d\n", (unsigned int)tcp_header->ack);
  printf("   |-Push Flag            : %d\n", (unsigned int)tcp_header->psh);
  printf("   |-Reset Flag           : %d\n", (unsigned int)tcp_header->rst);
  printf("   |-Synchronise Flag     : %d\n", (unsigned int)tcp_header->syn);
  printf("   |-Finish Flag          : %d\n", (unsigned int)tcp_header->fin);
  printf("   |-Window               : %d\n", ntohs(tcp_header->window));
  printf("   |-Checksum             : %d\n", ntohs(tcp_header->check));
  printf("   |-Urgent Pointer       : %d\n", tcp_header->urg_ptr);
  printf("\n");
  printf("                        DATA Dump                         ");
  printf("\n");

  printf("IP Header\n");
  print_data(buffer, ip_header_length);

  printf("TCP Header\n");
  print_data(buffer + ip_header_length, tcp_header->doff * 4);

  printf("Data Payload\n");
  print_data(buffer + header_size, size - header_size);

  printf("\n###########################################################\n");
}

void print_udp_packet(unsigned char *buffer, ssize_t size)
{
  size_t ethernet_header_length = sizeof(struct ethhdr);
  struct iphdr *ip_header = (struct iphdr *)(buffer + ethernet_header_length); // excludes ethernet header
  unsigned short ip_header_length = ip_header->ihl * 4;
  struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header_length + ethernet_header_length);
  size_t udp_header_length = sizeof(udp_header);

  int header_size = ethernet_header_length + ip_header_length + udp_header_length;

  printf("\n\n***********************UDP Packet*************************\n");

  print_ip_header(buffer, size);

  printf("\nUDP Header\n");
  printf("   |-Source Port      : %d\n", ntohs(udp_header->source));
  printf("   |-Destination Port : %d\n", ntohs(udp_header->dest));
  printf("   |-UDP Length       : %d\n", ntohs(udp_header->len));
  printf("   |-UDP Checksum     : %d\n", ntohs(udp_header->check));

  printf("\n");
  printf("IP Header\n");
  print_data(buffer, ip_header_length);

  printf("UDP Header\n");
  print_data(buffer + ip_header_length, udp_header_length);

  printf("Data Payload\n");

  // Move the pointer ahead and reduce the size of string
  print_data(buffer + header_size, size - header_size);

  printf("\n###########################################################\n");
}

void print_icmp_packet(unsigned char *buffer, ssize_t size)
{
  size_t ethernet_header_length = sizeof(struct ethhdr);
  struct iphdr *ip_header = (struct iphdr *)(buffer + ethernet_header_length); // excludes ethernet header
  unsigned short ip_header_length = ip_header->ihl * 4;
  struct icmphdr *icmp_header = (struct icmphdr *)(buffer + ip_header_length + ethernet_header_length);
  size_t icmp_header_length = sizeof(icmp_header);

  int header_size = ethernet_header_length + ip_header_length + icmp_header_length;

  printf("\n\n***********************ICMP Packet*************************\n");

  print_ip_header(buffer, size);

  printf("\n");
  printf("ICMP Header\n");
  printf("   |-Type : %d", (unsigned int)(icmp_header->type));

  if ((unsigned int)(icmp_header->type) == 11)
  {
    printf("  (TTL Expired)\n");
  }
  else if ((unsigned int)(icmp_header->type) == ICMP_ECHOREPLY)
  {
    printf("  (ICMP Echo Reply)\n");
  }

  printf("   |-Code : %d\n", (unsigned int)(icmp_header->code));
  printf("   |-Checksum : %d\n", ntohs(icmp_header->checksum));
  printf("\n");

  printf("IP Header\n");
  print_data(buffer, ip_header_length);

  printf("ICMP Header\n");
  print_data(buffer + ip_header_length, icmp_header_length);

  printf("Data Payload\n");

  // Move the pointer ahead and reduce the size of string
  print_data(buffer + header_size, size - header_size);

  printf("\n###########################################################\n");
}

void print_igmp_packet(unsigned char *buffer, ssize_t size)
{
  size_t ethernet_header_length = sizeof(struct ethhdr);
  struct iphdr *ip_header = (struct iphdr *)(buffer + ethernet_header_length); // excludes ethernet header
  unsigned short ip_header_length = ip_header->ihl * 4;
  struct igmphdr *igmp_header = (struct igmphdr *)(buffer + ip_header_length + ethernet_header_length);
  size_t igmp_header_length = sizeof(igmp_header);

  int header_size = ethernet_header_length + ip_header_length + igmp_header_length;

  printf("\n\n***********************IGMP Packet*************************\n");

  print_ip_header(buffer, size);

  printf("\n");
  printf("IGMP Header\n");
  printf("   |-Type : %d\n", (unsigned int)(igmp_header->type));
  printf("   |-Code : %x\n", (unsigned int)(igmp_header->code));
  printf("   |-Checksum : %d\n", ntohs(igmp_header->csum));
  printf("   |-Group : %d\n", (unsigned int)(igmp_header->group));
  printf("\n");

  printf("IP Header\n");
  print_data(buffer, ip_header_length);

  printf("IGMP Header\n");
  print_data(buffer + ip_header_length, igmp_header_length);

  printf("Data Payload\n");

  // Move the pointer ahead and reduce the size of string
  print_data(buffer + header_size, size - header_size);

  printf("\n###########################################################\n");
}

void print_ethernet_header(unsigned char *buffer, ssize_t size)
{
  struct ethhdr *ethernet_header = (struct ethhdr *)buffer;

  printf("\nEthernet Header\n");
  printf("   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", ethernet_header->h_dest[0], ethernet_header->h_dest[1], ethernet_header->h_dest[2], ethernet_header->h_dest[3], ethernet_header->h_dest[4], ethernet_header->h_dest[5]);
  printf("   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", ethernet_header->h_source[0], ethernet_header->h_source[1], ethernet_header->h_source[2], ethernet_header->h_source[3], ethernet_header->h_source[4], ethernet_header->h_source[5]);
  printf("   |-Protocol            : %u \n", (unsigned short)ethernet_header->h_proto);
}

void print_ip_header(unsigned char *buffer, ssize_t size)
{
  size_t ethernet_header_length = sizeof(struct ethhdr);
  struct iphdr *ip_header = (struct iphdr *)(buffer + ethernet_header_length); // excludes ethernet header
  unsigned short ip_header_length = ip_header->ihl * 4;

  print_ethernet_header(buffer, size);

  memset(source, 0, sizeof(source));
  source->sin_addr.s_addr = ip_header->saddr;

  memset(destination, 0, sizeof(destination));
  destination->sin_addr.s_addr = ip_header->daddr;

  printf("\nIP Header\n");
  printf("   |-IP Version        : %d\n", (unsigned int)ip_header->version);
  printf("   |-IP Header Length  : %d DWORDS or %d Bytes\n", (unsigned int)ip_header->ihl, ((unsigned int)ip_header_length));
  printf("   |-Type Of Service   : %d\n", (unsigned int)ip_header->tos);
  printf("   |-IP Total Length   : %d  Bytes(Size of Packet)\n", ntohs(ip_header->tot_len));
  printf("   |-Identification    : %d\n", ntohs(ip_header->id));
  printf("   |-TTL               : %d\n", (unsigned int)ip_header->ttl);
  printf("   |-Protocol          : %d\n", (unsigned int)ip_header->protocol);
  printf("   |-Checksum          : %d\n", ntohs(ip_header->check));
  printf("   |-Source IP         : %s\n", inet_ntoa(source->sin_addr));
  printf("   |-Destination IP    : %s\n", inet_ntoa(destination->sin_addr));
}

void print_data(unsigned char *data, ssize_t size)
{
  int i, j;

  for (i = 0; i < size; i++)
  {
    if ((i != 0) && (i % 16 == 0))
    { // if one line of hex printing is complete...
      printf("         ");
      for (j = i - 16; j < i; j++)
      {
        if (data[j] >= 32 && data[j] <= 128)
        {
          printf("%c", (unsigned char)data[j]); // if its a number or alphabet
        }
        else
        {
          printf("."); // otherwise print a dot
        }
      }
      printf("\n");
    }
    if (i % 16 == 0)
    {
      printf("   ");
    }
    printf(" %02X", (unsigned int)data[i]);
    if (i == size - 1)
    { // print the last spaces
      for (j = 0; j < 15 - i % 16; j++)
      {
        printf("   "); // extra spaces
      }
      printf("         ");
      for (j = i - i % 16; j <= i; j++)
      {
        if (data[j] >= 32 && data[j] <= 128)
        {
          printf("%c", (unsigned char)data[j]);
        }
        else
        {
          printf(".");
        }
      }
      printf("\n");
    }
  }
}

void handle_signal(int sig)
{
  printf("\nCaught interrupt signal %d\n", sig);
  puts("Releasing resources ...");
  free(source);
  free(destination);
  // closes the socket
  puts("Closing socket ...");
  if (close(raw_socket) == 0)
  {
    puts("Socket closed!");
    exit(EXIT_SUCCESS);
  }
  else
  {
    perror("An error occurred while closing the socket: ");
    printf("Error code: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}
