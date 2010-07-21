#ifndef _UDP_H_
#define _UDP_H_

void build_udp_header(struct udphdr *udph, int udp_len, int src_port, int dst_port);
int get_client_udp_port();
int get_server_udp_port();
void collect_client_udp_port_from_packet(unsigned char *packet, struct sockaddr_in *sin);

#endif
