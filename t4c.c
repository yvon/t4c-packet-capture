#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>

void debug_print_packet_data(const u_char *pkt_data)
{
  int i, j;
  int packet_size;
  char c;
  
  // Get the packet size from the UDP header
  // TOTO: This is not the right method !
  packet_size = (u_short)*(pkt_data + 39);
  printf("Packet. Size: %d\n", packet_size);
  
  for (i = 0; i < packet_size; i += 16) {
    // Hexadecimal data
    for (j = 0; j < 16; j++)
    {
      if (i + j < packet_size)
        printf("%.2X ", *(pkt_data + 43 + i + j));
      else
        printf("   ");
    }

    printf(" ");

    // Printable characters
    for (j = 0; j < 16 && i + j < packet_size; j++)
    {
      c = *(pkt_data + 43 + i + j);
      if (c < 32 || c > 126)
        c = '.';
      printf("%c", c);
    }
    
    printf("\n");
  }
  printf("\n");
}

int main()
{
  pcap_t *fp;
  char errbuf[PCAP_ERRBUF_SIZE];
  char *source = NULL;
  char *filter = NULL;
  struct bpf_program fcode;
  bpf_u_int32 NetMask;
  int res;
  struct pcap_pkthdr *header;
  const u_char *pkt_data;
  
  pcap_if_t *alldevs;
  /* Retrieve the device list */
  if(pcap_findalldevs(&alldevs, errbuf) == -1)
  {
    fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
    return (-1);
  }
  
  if(!alldevs || !(alldevs->name))
  {
    fprintf(stderr, "No device capture found!");
    return (-1);
  }
  
  // Select the first device capture as source
  source = alldevs->name;
  
  // open a capture from the network
  if ((fp = pcap_open_live(source,    // name of the device
    65536,                            // portion of the packet to capture. 
                                      // 65536 grants that the whole packet will be captured on all the MACs.
    1,                                // promiscuous mode (nonzero means promiscuous)
    1000,                             // read timeout
    errbuf                            // error buffer
    )) == NULL)
  {
    fprintf(stderr,"\nUnable to open the adapter.\n");
    return -2;
  }

  // Capture the packets on UDP port 11677 (T4C)
  filter = "udp and port 11677";
  // We should loop through the adapters returned by the pcap_findalldevs_ex()
  // in order to locate the correct one.
  //
  // Let's do things simpler: we suppose to be in a C class network ;-)
  NetMask=0xffffff;

  //compile the filter
  if(pcap_compile(fp, &fcode, filter, 1, NetMask) < 0)
  {
    fprintf(stderr,"\nError compiling filter: wrong syntax.\n");

    pcap_close(fp);
    return -3;
  }

  //set the filter
  if(pcap_setfilter(fp, &fcode)<0)
  {
    fprintf(stderr,"\nError setting the filter\n");

    pcap_close(fp);
    return -4;
  }
  
  //start the capture
  while((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
  {
    if (res == 0)
      continue;
      
    debug_print_packet_data(pkt_data);
  }

  pcap_close(fp);
  return 0;
}
