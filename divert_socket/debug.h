#ifndef _DEBUG_H_
#define _DEBUG_H_

void debug_sum_packet_data(unsigned char *data, int len);
void debug_print_packet_data(unsigned char *data, int len);
void debug_show_ip_header(unsigned char *data);

#endif
