#include <netinet/ip.h>
#include <netinet/udp.h>

static int client_udp_port = 0;
static int server_udp_port = 11677;

void build_udp_header(struct udphdr *udph, int udp_len, int src_port, int dst_port)
{  
  udph->uh_sport = htons(src_port);
  udph->uh_dport = htons(dst_port);
  udph->uh_ulen = htons(udp_len);
  udph->uh_sum = 0;
}

int get_client_udp_port()
{
  return (client_udp_port);
}

int get_server_udp_port()
{
  return (server_udp_port);
}

void collect_client_udp_port_from_packet(unsigned char *packet, struct sockaddr_in *sin)
{
  if (client_udp_port != 0)
    return;
  
  struct udphdr *udph;
  udph = (struct udphdr *)(packet + sizeof(struct ip));
    
  if (sin->sin_addr.s_addr == 0)  // Outgoing packet
    client_udp_port = ntohs(udph->uh_sport);
  else                            // Incoming packet
    client_udp_port = ntohs(udph->uh_dport);
}
