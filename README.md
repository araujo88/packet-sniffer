# packet-sniffer
A simple packet sniffer coded in C

## Build

`gcc main.c -o main.o`

## Usage

`sudo ./main.o | tee log`

## Limitations

 - Supports only TCP/IP protocol
 - Supports only incoming data

## TODO
 - Include Ethernet headers
 - Include UDP protocol
 - Include IGMP protocol
 - Include ICMP protocol
 - Include outgoing data
