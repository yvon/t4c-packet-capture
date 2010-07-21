#ifndef _CRAFTING_H_
#define _CRAFTING_H_

int inject_paket(unsigned char *packet, int size, struct sockaddr_in *sin, int sinlen);
int craft_outgoing_packet(unsigned char *data, int data_len);
int craft_incoming_packet(unsigned char *data, int data_len);

#endif
