#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "ip.h"
#include "udp.h"
#include "data_header.h"
#include "debug.h"

extern char  *progname;
extern int fd;

int inject_paket(unsigned char *packet, int size, struct sockaddr_in *sin, int sinlen)
{
  int n;
  
  // We don't want to see you again!
  sin->sin_port = 9999;
  
  n = sendto(fd, packet, size, (size_t)0, (struct sockaddr *)sin, sinlen);
  
  #ifdef DEBUG
  debug_print_packet_data(packet, size);
  #endif
  
  if (n<=0) 
    printf("%s: Oops: errno = %i\n", progname, errno);
  
  return (n);
}

unsigned char *allocate_packet(unsigned char *data, int packet_size)
{
  unsigned char *packet;
  int           headers_size;
  int           data_size;
  
  headers_size  = sizeof(struct ip) + sizeof(struct udphdr);
  data_size     = packet_size - headers_size;
  
  packet = malloc(packet_size);
  memcpy(packet + headers_size, data, data_size);
  
  return (packet);
}

int craft_outgoing_packet(unsigned char *data, int data_len)
{
  static unsigned int last_packet_id = 0;
  
  int                 n, ip_len, udp_len;
  struct sockaddr_in  sin;
  unsigned char       *packet;
  struct ip           *iph;
  struct udphdr        *udph;
  
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  
  
  ip_len  = data_len + sizeof(struct ip) + sizeof(struct udphdr);
  udp_len = data_len + sizeof(struct udphdr);
  
  packet  = allocate_packet(data, ip_len);
  iph     = (struct ip *)packet;
  udph    = (struct udphdr *)(packet + sizeof(struct ip));
  data    = (unsigned char *)(packet + sizeof(struct ip) + sizeof(struct udphdr));
  
  build_data_header     (data,    data_len, &last_packet_id);
  build_ip_header       (iph,     ip_len,   get_client_ip(),        get_server_ip());
  build_udp_header      (udph,    udp_len,  get_client_udp_port(),  get_server_udp_port());
  
  #ifdef DEBUG
  debug_show_ip_header(packet);
  #endif
  
  n = inject_paket(packet, ip_len, &sin, sizeof(sin));
  return (n);
}

int craft_incoming_packet(unsigned char *data, int data_len)
{
  static unsigned int last_packet_id = 0;
  
  int                 n, ip_len, udp_len;
  struct sockaddr_in  sin;
  unsigned char       *packet;
  struct ip           *iph;
  struct udphdr        *udph;
  
  sin.sin_family  = AF_INET;
  sin.sin_addr    = *get_client_ip();
  
  ip_len  = data_len + sizeof(struct ip) + sizeof(struct udphdr);
  udp_len = data_len + sizeof(struct udphdr);
  
  packet  = allocate_packet(data, ip_len);
  iph     = (struct ip *)packet;
  udph    = (struct udphdr *)(packet + sizeof(struct ip));
  data    = (unsigned char *)(packet + sizeof(struct ip) + sizeof(struct udphdr));
  
  build_data_header     (data,    data_len, &last_packet_id);
  build_ip_header       (iph,     ip_len,   get_server_ip(),        get_client_ip());
  build_udp_header      (udph,    udp_len,  get_server_udp_port(),  get_client_udp_port());
  
  #ifdef DEBUG
  debug_show_ip_header(packet);
  #endif
  
  n = inject_paket(packet, ip_len, &sin, sizeof(sin));
  return (n);
}