#include <netinet/ip.h>
#include <arpa/inet.h>

static struct in_addr server_ip;
static struct in_addr client_ip;

struct in_addr *get_server_ip()
{
  return (&server_ip);
}

struct in_addr *get_client_ip()
{
  return (&client_ip);
}

void set_server_ip(char *ip)
{
  inet_aton(ip, &server_ip);
}

void initialize_client_ip()
{
  client_ip.s_addr = 0;
}

void collect_client_ip_from_packet(unsigned char *packet, struct sockaddr_in *sin)
{
  if (client_ip.s_addr != 0)
    return;
  
  struct ip *iph;
  iph = (struct ip *)packet;
  
  if (sin->sin_addr.s_addr == 0)  // Outgoing packet
    client_ip.s_addr = iph->ip_src.s_addr;
  else                            // Incoming packet
    client_ip.s_addr = iph->ip_dst.s_addr;
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

void build_ip_header(struct ip *iph, int ip_len, struct in_addr *ip_src, struct in_addr *ip_dst)
{    
  iph->ip_hl  = 5; 
  iph->ip_v   = 4; 
  iph->ip_len = ntohs(ip_len); 
  iph->ip_tos = 0;
  iph->ip_id  = htons(0xffff);
  iph->ip_ttl = 127;
  iph->ip_off = 0;
  iph->ip_p   = 17; 
  iph->ip_sum = 0;
  iph->ip_src = *ip_src; 
  iph->ip_dst = *ip_dst;
  
  iph->ip_sum = htons(checksum(20, (unsigned short *)iph));
}
