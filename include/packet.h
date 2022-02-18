#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#include "packet.h"

void check_socket(int socket);
void print_ethernet_header(unsigned char *buffer, ssize_t size);
void print_ip_header(unsigned char *buffer, ssize_t size);
void process_packet(unsigned char *buffer, ssize_t size);
void print_tcp_packet(unsigned char *buffer, ssize_t size);
void print_udp_packet(unsigned char *buffer, ssize_t size);
void print_icmp_packet(unsigned char *buffer, ssize_t size);
void print_igmp_packet(unsigned char *buffer, ssize_t size);
void print_data(unsigned char *data, ssize_t size);

#endif // PACKET_H_INCLUDED
