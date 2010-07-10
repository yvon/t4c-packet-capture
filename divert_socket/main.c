#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#include "parse.h"

#define IPPROTO_DIVERT 254
#define BUFSIZE 65535

char *progname;
u_short divert_socket_port;
int fd;

struct in_addr client_ip, server_ip;
u_short client_port, server_port;

// Proxy point of vue
unsigned int last_recv_packet_id, last_sent_packet_id;
// Proxyless
unsigned char client_last_packet_id, server_last_packet_id = 0;

u_short ip_sum_calc(u_short len_ip_header, u_short *buff)
{
  u_short word16;
  u_long  sum = 0;
  u_short i;
    
  // make 16 bit words out of every two adjacent 8 bit words in the packet
  // and add them up
  for (i = 0; i < len_ip_header; i = i + 2)
  {
    word16 = ((buff[i] << 8) & 0xFF00) + (buff[i+1] & 0xFF);
    sum = sum + (u_long)word16;
    // take only 16 bits out of the 32 bit sum and add up the carries
    while (sum >> 16)
      sum = (sum & 0xFFFF) + (sum >> 16);
    
  }
  
  // one's complement the result
  sum = ~sum;

  return ((u_short) sum);
}

unsigned short checksum (int count, unsigned short * addr) {
    unsigned long sum = 0;
    unsigned short short_sum;

    while (count > 1) {
        sum += ntohs(*addr++);
        count -= 2;
    }

    // Add left-over byte, if any
    if (count > 0)
        sum += * (unsigned char *) addr;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    short_sum = (unsigned short)sum;
    return (~short_sum);
}

void build_ip_udp_headers(unsigned char *data, int len, struct in_addr ip_src, struct in_addr ip_dst, int src_port, int dst_port)
{
  struct ip *iph = (struct ip*)data; 
    
  iph->ip_hl = 5; 
  iph->ip_v = 4; 
  iph->ip_len = ntohs(len + 20 + 8); 
  iph->ip_tos = 0;
  iph->ip_id = htons(0xffff);
  iph->ip_ttl = 127;
  iph->ip_off = 0;
  iph->ip_p = 17; 
  iph->ip_sum = 0;
  iph->ip_src = ip_src; 
  iph->ip_dst = ip_dst;
  
  iph->ip_sum = htons(checksum(20, (unsigned short *)iph));
  
  struct udphdr *udph = (struct udphdr*)(data + sizeof(struct ip));
  
  udph->uh_sport = htons(src_port);
  udph->uh_dport = htons(dst_port);
  udph->uh_ulen = htons(len + sizeof(struct udphdr));
  udph->uh_sum = 0;
}

void build_client_ip_udp_headers(unsigned char *data, int len)
{
  build_ip_udp_headers(data, len, client_ip, server_ip, client_port, server_port);
}

void build_server_ip_udp_headers(unsigned char *data, int len)
{
  build_ip_udp_headers(data, len, server_ip, client_ip, server_port, client_port);
}

void debug_print_packet_data(unsigned char *data, int len)
{
  int i, j;
  unsigned char c;
  
  for (i = 0; i < len; i += 16) {
    // Hexadecimal data
    for (j = 0; j < 16; j++)
    {
      if (i + j < len)
      {
        c = *(data + i + j);
        printf("%.2X ", c);
      }
      else
      {
        printf("   ");
      }
    }

    printf(" ");

    // Printable characters
    for (j = 0; j < 16 && i + j < len; j++)
    {
      c = *(data + i + j);
      if (c < 32 || c > 126)
      {
        c = '.';
      }
      printf("%c", c);
    }
    printf("\n");
  }
}

void debug_print_packet_headers(unsigned char *packet, int size)
{
  struct ip *iph;
  struct udphr *udph;
  
  
}

void debug_sum_packet_data(unsigned char *data, int len)
{
  int i;
  unsigned int sum = 0;
  
  for (i = 0; i < len; i++)
  {
    sum += data[i];
  }
  
  printf("SUM %d (%.2X)", sum, sum);
}

int get_packet_size(unsigned char *datagramme)
{
  int packet_size;
  
  // Get the datagramme size from the UDP header
  packet_size = ntohs(*(u_short *)(datagramme - 4)) - 8;
  
  return (packet_size);
}

char determine_direction(unsigned char *datagramme)
{
  // Client of server ?
  char direction;
  
  // Determine the ports from the UDP head
  int src_port = ntohs(*(u_short *)(datagramme - 8));
  int dest_port = ntohs(*(u_short *)(datagramme - 6));
  
  if (src_port == 11677)
    direction = '<';
  else if (dest_port == 11677)
    direction = '>';
  else
    direction = '?';
  
  return (direction);
}

int sum_packet_data_without_checksum(unsigned char *data, int len)
{
  int i;
  unsigned int sum;
  
  for (sum = 0, i = 0; i < len; i++)
  {
    if (i != 2)
      sum += data[i];
  }
  
  return (sum);
}

void draw_header(unsigned char *datagramme, int packet_size, unsigned int *last_packet_id, unsigned char *without_proxy_last_packet_id)
{
  int checksum, sum;
  unsigned char flags;
  unsigned char loop_cn;
  
  *without_proxy_last_packet_id = datagramme[0];
  *last_packet_id += 1; // Increment the counter

  loop_cn = *last_packet_id / 0xff;
  datagramme[0] = *last_packet_id % 0xff;

  flags = datagramme[1] & 0xf0;
  datagramme[1] = flags ^ loop_cn;  
  
  if (last_packet_id == &last_sent_packet_id)
    printf("ID: %d, SEQ: %d\n", *last_packet_id, loop_cn);
  
  sum = sum_packet_data_without_checksum(datagramme, packet_size);
  if (sum > 0x56)
    checksum = (0x100 - ((sum - 0x56) % 0x100)) % 0x100;
  else
    checksum = 0x56 - sum;
  datagramme[2] = checksum;
}

void redraw_header(unsigned char *datagramme, int packet_size, unsigned int *last_packet_id, unsigned char *without_proxy_last_packet_id)
{
  // if ((*without_proxy_last_packet_id + 1) % 0xff == datagramme[0])
  draw_header(datagramme, packet_size, last_packet_id, without_proxy_last_packet_id);
}


int inject_paket(int fd, unsigned char *packet, int size, struct sockaddr_in *sin, int sinlen)
{
  int n;
  
  sin->sin_port = 4242; // We don't want to see you again!
  
  n = sendto(fd, packet, size, (size_t)0, (struct sockaddr *)sin, sinlen);
  
  if (n<=0) 
  {
    switch (errno)
    {
      case EACCES:
      printf("The SO_BROADCAST option is not set on the socket and a broadcast address is given as the destination.\n");
      case EAGAIN:
      printf("The socket is marked non-blocking and the requested operation would block.\n");
      case EBADF:
      printf("An invalid descriptor is specified.\n");
      case ECONNRESET:
      printf("A connection is forcibly closed by a peer.\n");
      case EFAULT:
      printf("An invalid user space address is specified for a parameter.\n");
      case EHOSTUNREACH:
      printf("The destination address specifies an unreachable host.\n");
      case EINTR:
      printf("A signal interrupts the system call before any data is transmitted.\n");
      case EMSGSIZE:
      printf("The socket requires that message be sent atomically, and the size of the message to be sent makes this impossible.\n");
      case ENETDOWN:
      printf("The local network interface used to reach the destination is down.\n");
      case ENETUNREACH:
      printf("No route to the network is present.\n");
      case ENOBUFS:
      printf("The system is unable to allocate an internal buffer.  The operation may succeed when buffers become available.\n");
      printf("The output queue for a network interface is full.  This generally indicates that the interface has stopped sending, but may be caused by transient congestion.\n");
      case ENOTSOCK:
      printf("The argument socket is not a socket.\n");
      case EOPNOTSUPP:
      printf("socket does not support (some of) the option(s) specified in flags.\n");
      case EPIPE:
      printf("The socket is shut down for writing or the socket is connection-mode and is no longer connected.  In the latter case, and if the socket is of type SOCK_STREAM, the SIGPIPE signal is generated to the calling thread.\n");
      default :
      printf("%s: Oops: errno = %i\n", progname, errno);
    }
  }
    
  return (n);
}

int send_client_packet(unsigned char *packet, int size)
{
  int n;
  struct sockaddr_in sin;
  
  sin.sin_family = AF_INET;
  sin.sin_port = divert_socket_port;
  sin.sin_addr.s_addr = INADDR_ANY;
  
  n = inject_paket(fd, packet, size, &sin, sizeof(sin));
  return (n);
}

int send_server_packet(unsigned char *packet, int size)
{
  int n;
  struct sockaddr_in sin;
  
  sin.sin_family = AF_INET;
  sin.sin_port = divert_socket_port;
  sin.sin_addr = client_ip;
  
  n = inject_paket(fd, packet, size, &sin, sizeof(sin));
  return (n);
}

int craft_and_send_packet(unsigned char *data, int data_size, unsigned char from_server)
{
  int           packet_size;
  int           datagramme_size;
  unsigned char *packet;
  unsigned char *datagramme;
  int           n;
  
  datagramme_size = data_size + 8; // 8 => sizeof datagramme header
  packet_size = datagramme_size + 20 + 8;
  
  packet = malloc(packet_size);
  bzero(packet, packet_size);
  
  datagramme = packet + 20 + 8;
  datagramme[1] = 0x10;
  memcpy(datagramme + 8, data, data_size);
  
  if (from_server == 1)
  {
    draw_header(datagramme, datagramme_size, &last_recv_packet_id, &server_last_packet_id);
    build_server_ip_udp_headers(packet, datagramme_size);
    debug_print_packet_data(packet, packet_size);
    printf("\n");
    n = send_server_packet(packet, packet_size);
  }
  else
  {
    draw_header(datagramme, packet_size, &last_sent_packet_id, &client_last_packet_id);
    build_client_ip_udp_headers(packet, datagramme_size);
    debug_print_packet_data(packet, packet_size);
    printf("\n");
    n = send_client_packet(packet, packet_size);
  }
  return (n);
}

int main(int argc, char** argv) {
  int ret, n;
  struct sockaddr_in bindPort, sin;
  int sinlen;
  unsigned char packet[BUFSIZE];
  
  progname = argv[0];
  
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port number>\n", progname);
    exit (1); 
  }
  
  divert_socket_port = atoi(argv[1]);
  
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
  
  printf("%s: Waiting for data...\n", progname);
  
  sinlen = sizeof(struct sockaddr_in);
  unsigned char *datagramme;
  int packet_size;
  char direction; // => '<' (server to client) or '>' (client to server)

  client_ip.s_addr = 0;
  inet_aton("96.56.208.212", &server_ip);
  client_port = 0;
  server_port = 11677;

  struct ip *iph;
  struct udphr *udph; 
  
  while(1) {
    n = recvfrom(fd, packet, BUFSIZE, 0, (struct sockaddr *)&sin, (socklen_t *)&sinlen);
    
    // Header IPV4 => 20 bytes
    // Header UDP => 8 bytes
    iph = (struct ip*)packet; 
    udph = (struct udphr*)(packet + sizeof(struct ip));
    
    datagramme = packet + 20 + 8;
    
    direction = determine_direction(datagramme);
    
    packet_size = get_packet_size(datagramme);
    if (direction == '<')
    {
      if (client_ip.s_addr == 0)
        client_ip.s_addr = iph->ip_dst.s_addr;
      if (client_port == 0)
      {
        client_port = ntohs(*(u_short *)(packet + 20 + 2));
      }
        
    }
    else if (direction == '>')
    {;
      if (client_ip.s_addr == 0)
        client_ip.s_addr = iph->ip_src.s_addr;
      if (client_port == 0)
      {
        client_port = ntohs(*(u_short *)(packet + 20));
      }
    }
    // Should we care?
    if (packet_size > 4)
    {      
      // Redraw the headers
      if (direction == '<')
      {
        parse_server_packet(datagramme + 8, packet_size - 8);
        redraw_header(datagramme, packet_size, &last_recv_packet_id, &server_last_packet_id);
        build_server_ip_udp_headers(packet, packet_size);
        send_server_packet(packet, n);
      }
      else if (direction == '>')
      {
        redraw_header(datagramme, packet_size, &last_sent_packet_id, &client_last_packet_id);
        build_client_ip_udp_headers(packet, packet_size);
        send_client_packet(packet, n);
      }
      else
      {
        inject_paket(fd, packet, n, &sin, sinlen);
      }
    }
    else
    {
      inject_paket(fd, packet, n, &sin, sinlen);
    }
  }
}