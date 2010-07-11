#ifndef _IP_H_
#define _IP_H_

void build_ip_header(struct ip *iph, int ip_len, struct in_addr *ip_src, struct in_addr *ip_dst);
struct in_addr *get_server_ip();
struct in_addr *get_client_ip();
void set_server_ip(char *ip);
void initialize_client_ip();
void collect_client_ip_from_packet(unsigned char *packet, struct sockaddr_in *sin);

#endif
