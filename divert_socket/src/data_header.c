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

void build_data_header(unsigned char *datagramme, int packet_size, unsigned int *last_packet_id)
{
  int checksum, sum;
  unsigned char flags;
  unsigned char loop_cn;
  
  *last_packet_id += 1; // Increment the counter

  loop_cn = *last_packet_id / 0xff;
  datagramme[0] = *last_packet_id % 0xff;

  flags = datagramme[1] & 0xf0;
  datagramme[1] = flags ^ loop_cn;  
  
  sum = sum_packet_data_without_checksum(datagramme, packet_size);
  if (sum > 0x56)
    checksum = (0x100 - ((sum - 0x56) % 0x100)) % 0x100;
  else
    checksum = 0x56 - sum;
  datagramme[2] = checksum;
}
