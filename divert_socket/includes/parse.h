#ifndef _PARSE_H_
#define _PARSE_H_

int parse_incoming_packet(unsigned char *data, int data_len);
int parse_outgoing_packet(unsigned char *data, int data_len);

#endif
