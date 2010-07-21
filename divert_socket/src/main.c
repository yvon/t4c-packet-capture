#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "crafting.h"
#include "ip.h"
#include "udp.h"
#include "debug.h"

#define IPPROTO_DIVERT 254
#define BUFSIZE 65535

char  *progname;
int   fd; // Divert socket file descriptor

int reinject_paket(unsigned char *packet, int size, struct sockaddr_in *sin, int sinlen)
{
  return (inject_paket(packet, size, sin, sinlen));
}

int main(int argc, char** argv) {
  int           ret, n, sinlen;
  int           data_len, datagramme_len;
  struct        sockaddr_in bindPort, sin;
  unsigned char packet[BUFSIZE];
  unsigned char *data, *datagramme;
  
  progname = argv[0];
  
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port number>\n", progname);
    exit (1); 
  }
    
  fprintf(stderr, "%s:Creating a socket\n", progname);
  // Open a divert socket
  fd = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);
  
  if (fd == -1) {
    fprintf(stderr, "%s:We could not open a divert socket\n", progname);
    exit (1);
  }
  
  bindPort.sin_family = AF_INET;
  bindPort.sin_port = htons(atol(argv[1]));
  bindPort.sin_addr.s_addr = 0;
  
  fprintf(stderr, "%s:Binding a socket\n", progname);
  ret = bind(fd, (struct sockaddr *)&bindPort, sizeof(struct sockaddr_in));
  
  if (ret != 0) {
    close(fd);
    fprintf(stderr, "%s: Error bind(): %s", progname, strerror(ret));
    exit (2);
  }
    
  sinlen = sizeof(struct sockaddr_in);
  
  set_server_ip("96.56.208.212");
  initialize_client_ip();
  
  while(1) {
    n = recvfrom(fd, packet, BUFSIZE, 0, (struct sockaddr *)&sin, (socklen_t *)&sinlen);
    
    // Identify the packet components. Ignore the tcp and udp headers.
    data        = packet + sizeof(struct ip) + sizeof(struct udphdr);
    datagramme  = data + 8;
    
    // Store their respective sizes
    data_len        = n - sizeof(struct ip) - sizeof(struct udphdr);
    datagramme_len  = data_len- 8;
    
    if (data_len > 4) // Ensure that the packet is not an acknowledgment
    {
      collect_client_ip_from_packet(packet, &sin);
      collect_client_udp_port_from_packet(packet, &sin);
            
      if (sin.sin_addr.s_addr == 0)  // Outgoing packet
      {
        write(0, ">", 1);
        #ifdef DEBUG
        printf("____ OUTGOING ____\n");
        debug_show_ip_header(packet);
        printf("------------------\n");
        #endif
        craft_outgoing_packet(data, data_len);
      }
      else                            // Incoming packet
      {
        write(0, "<", 1);
        #ifdef DEBUG
        printf("____ INCOMING ____\n");
        debug_show_ip_header(packet);
        printf("------------------\n");
        #endif
        craft_incoming_packet(data, data_len);
      }
    }
    else
    {
      reinject_paket(packet, n, &sin, sinlen);
    }
  }
}