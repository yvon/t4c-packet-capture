#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#define PAK_SERVER_PutPlayerInGame 0x000D
//   2   int8 unknown1;
//   3   int32 player_id;
//   7   int16 x_coord;
//   9   int16 y_coord;
//   11   int16 world;
//   13   int32 health;
//   17   int32 max_health;
//   21   int16 mana;
//   23   int16 max_mana;
//   25   int64 xp;
//   33   int64 next_level_xp;
//   41   int16 strength;
//   43   int16 endurance;
//   45   int16 dexterity;
//   47   int16 willpower; // unused
//   49   int16 wisdom;
//   51   int16 intelligence;
//   53   int16 luck; // unused
//   55   int8 seconds;
//   56   int8 minutes;
//   57   int8 hour;
//   58   int8 week;
//   59   int8 day;
//   60   int8 month;
//   61   int16 year;
//   63   int32 gold;
//   67   int16 level;
//   69   int64 base_level_xp;

#define PAK_SERVER_ServerMessage 0x003F
//   2   int32 unused; // was color, but client sets it to 0x00FF6400 (blue). Can be nulled.
//   6   string16 message;

#define PAK_SERVER_MessageOfTheDay 0x0042
//   2   string16 message;

#define PAK_SERVER_Page 0x001D
//   2   string16 sender_playername;
//   2+[2+S]   string16 message;

int craft_and_send_packet(unsigned char *data, int data_size, unsigned char from_server);

void send_page_message_to_client(char *from, char *message)
{
  int           packet_size = 0; 
  int           length;
  unsigned char packet[256]  = {0};
  unsigned char *ptr;
  
  ptr = packet;
  
  // Packet ID
  // Packet ID
  *(u_short *)(ptr) = htons(PAK_SERVER_Page);
  packet_size += 2;
  ptr += 2;

  // Sender name length
  length = strlen(from);
  *(u_short *)(ptr) = htons(length);
  packet_size += 2;
  ptr += 2;

  // Sender name
  memcpy(ptr, from, length);
  packet_size += length;
  ptr += length;

  // Message length
  length = strlen(message);
  *(u_short *)(ptr) = htons(length);
  packet_size += 2;
  ptr += 2;

  // Sender name
  memcpy(ptr, message, length);
  packet_size += length;
  ptr += length;

  craft_and_send_packet(packet, packet_size, 0);
}

void send_server_message_to_client(char *message)
{ 
  int           packet_size = 0; 
  int           length;
  unsigned char packet[40]  = {0};
  unsigned char *ptr;
  
  ptr = packet;
  
  // Packet ID
  *(u_short *)(ptr) = htons(PAK_SERVER_ServerMessage);
  packet_size += 2;
  ptr += 2;
  
  // Color
  *(u_long *)(ptr) = 0;
  packet_size += 4;
  ptr += 4;
  
  // Message length
  length = strlen(message);
  *(u_short *)(ptr) = htons(length);
  packet_size += 2;
  ptr += 2;

  // Message
  memcpy(ptr, message, length);
  packet_size += length;
  ptr += length;
  
  craft_and_send_packet(packet, packet_size, 1);
}

void received_pack_server_put_player_in_game(unsigned char *data, int len)
{
  printf("Hey, je te vois!\n");
  send_page_message_to_client("Asoen", "Hey, je te vois!");
}

void parse_server_packet(unsigned char *data, int len)
{
  u_short packet_id;
  
  packet_id = ntohs(*(u_short *)(data));
  
  if (packet_id == PAK_SERVER_PutPlayerInGame)
    received_pack_server_put_player_in_game(data, len);
}
