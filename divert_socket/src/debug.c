#include <stdio.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

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

void debug_show_ip_header(unsigned char *data)
{
  struct ip *iph = (struct ip *)(data);
  
  printf("iph->ip_hl = %d\n\
iph->ip_v = %d\n\
iph->ip_len = %d\n\
iph->ip_tos = %d\n\
iph->ip_id  = %x\n\
iph->ip_ttl = %d\n\
iph->ip_off = %d\n\
iph->ip_p = %d\n\
iph->ip_sum = %d\n\
iph->ip_src = %s\n\
iph->ip_dst = %s\n\
iph->ip_sum = %d\n",
  iph->ip_hl,
  iph->ip_v,
  ntohs(iph->ip_len),
  iph->ip_tos,
  ntohs(iph->ip_id),
  iph->ip_ttl,
  ntohs(iph->ip_off),
  iph->ip_p,
  ntohs(iph->ip_sum),
  inet_ntoa(iph->ip_src),
  inet_ntoa(iph->ip_dst),
  iph->ip_sum);
}