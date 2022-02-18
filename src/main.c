#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>       // socket creation
#include <errno.h>            // error codes
#include <stdbool.h>          // boolean types
#include <netinet/if_ether.h> // for ETH_P_ALL
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include "packet.h"

#define BUFFER_SIZE 65536

int raw_socket;
extern struct sockaddr_in *source;
extern struct sockaddr_in *destination;

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
