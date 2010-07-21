#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "crafting.h"

#define PAK_SERVER_UpdateCoordinates 0x0001 // a unit is moving on screen
//   2   int16 x_coord; // new X coordinate on map
//   4   int16 y_coord; // new Y coordinate on map
//   6   int16 skin_id; // unit skin ID
//   8   int32 unit_id; // unit ID
//   12   int8 light_percentage; // radiance from 0 to 100
//   13   int8 unit_type; // 0 = monster, 1 = NPC, 2 = player
//   14   int8 health_percentage; // visual health from red (0) to green (100)
#define PAK_SERVER_SynchronizePlayerCoordinates 0x0009 // server is making sure client is at the right location
//   2   int16 x_coord;
//   4   int16 y_coord;
//   6   int16 world;
#define PAK_CLIENT_PutPlayerInGame 0x000D
//   2   string8 playername;
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
#define PAK_CLIENT_Attack 0x0018
//   2   int16 x_coord;
//   4   int16 y_coord;
//   6   int32 unit_id;
#define PAK_SERVER_Unknown24 0x0018
#define PAK_CLIENT_CastSpell 0x0020
//   2   int16 spell_id;
//   4   int16 target_x_coord;
//   6   int16 target_y_coord;
//   8   int32 target_unit_id;
#define PAK_SERVER_Unknown32 0x0020
#define PAK_CLIENT_UpdateLife 0x0021
#define PAK_SERVER_UpdateLife 0x0021
//   2   int32 health;
//   6   int32 max_health;

struct s_player {
  uint32_t player_id;
  uint16_t x_coord;
  uint16_t y_coord;
  uint32_t health;
  uint32_t max_health;
  uint16_t mana;
  uint16_t max_mana;
};

struct s_player player;

void debug_show_player()
{
  printf("Player:\n\
    player_id  -> %d\n\
    x_coord    -> %d\n\
    y_coord    -> %d\n\
    health     -> %d\n\
    max_health -> %d\n\
    mana       -> %d\n\
    max_mana   -> %d\n",
    player.player_id,
    player.x_coord,
    player.y_coord,
    player.health,
    player.max_health,
    player.mana,
    player.max_mana);
}

uint16_t read_short(unsigned char *data, int pos)
{
  return (ntohs(*(uint16_t *)(data + pos)));
}

uint16_t read_long(unsigned char *data, int pos)
{
  return (ntohl(*(uint32_t *)(data + pos)));
}

void write_short(uint16_t num, unsigned char *data, int pos)
{
  uint16_t *ptr;
  
  ptr   = (uint16_t *)(data + pos);
  *ptr  = htons(num);
}

void write_long(uint32_t num, unsigned char *data, int pos)
{
  uint32_t *ptr;
  
  ptr   = (uint32_t *)(data + pos);
  *ptr  = htonl(num);
}

int player_in_game()
{
  return (player.player_id != 0);
}

void cast_spell(uint16_t spell_id, uint16_t target_x_coord, uint16_t target_y_coord, uint32_t target_unit_id)
{
  printf("Automated spelling\n");
  
  unsigned char data[20];
  
  data[1] = 0x90;
  
  write_short(PAK_CLIENT_CastSpell, data, 8);
  write_short(spell_id,             data, 10);
  write_short(target_x_coord,       data, 12);
  write_short(target_y_coord,       data, 14);
  write_long (target_unit_id,       data, 16);
  
  craft_outgoing_packet(data, 20);
}

void cast_spell_on_player(uint16_t spell_id)
{
  cast_spell(spell_id, player.x_coord, player.y_coord, player.player_id);
}

void cast_heal_on_player()
{
  cast_spell_on_player(10033);
}

uint16_t get_packet_id(unsigned char *data, unsigned char **packet_arguments)
{
  uint16_t packet_id;
  
  packet_id = ntohs(*(u_short *)(data + 8));
  *packet_arguments = (unsigned char *)(data + 10);
  
  if (packet_id == 0x0e) // Reason of this ???
  {
    packet_id = ntohs(*(u_short *)(data + 14));
    *packet_arguments = (unsigned char *)(data + 16);
  }
  
  return (packet_id);
}

int parse_incoming_packet(unsigned char *data, int data_len)
{
  uint16_t packet_id;
  unsigned char *arguments;
  
  packet_id = get_packet_id(data, &arguments);
  
  if (packet_id == PAK_SERVER_UpdateCoordinates)
  {
    // Player moving?
    if (player_in_game() && read_long(arguments, 6) == player.player_id)
    {
      player.x_coord = read_short(arguments, 0);
      player.y_coord = read_short(arguments, 2);
    }
  }
  
  else if (packet_id == PAK_SERVER_SynchronizePlayerCoordinates && player_in_game())
  {
    player.x_coord = read_short(arguments, 0);
    player.y_coord = read_short(arguments, 2);
  }  
  
  else if (packet_id == PAK_SERVER_UpdateLife && player_in_game())
  {
    player.health     = ntohl(*(uint32_t *)(arguments));
    player.max_health = ntohl(*(uint32_t *)(arguments + 4));
    
    if (player.health * 100 / player.max_health <= 50)
      cast_heal_on_player();
  }

  else if (packet_id == PAK_SERVER_PutPlayerInGame)
  {
    player.player_id  = ntohl(*(uint32_t *)(arguments + 1));
    player.x_coord    = ntohs(*(uint16_t *)(arguments + 5));
    player.y_coord    = ntohs(*(uint16_t *)(arguments + 7));
    player.health     = ntohl(*(uint32_t *)(arguments + 11));
    player.max_health = ntohl(*(uint32_t *)(arguments + 15));
    player.mana       = ntohs(*(uint16_t *)(arguments + 19));
    player.max_mana   = ntohs(*(uint16_t *)(arguments + 21));
  }
   
  return (1); // Reinject the packet
}

int parse_outgoing_packet(unsigned char *data, int data_len)
{
  uint16_t packet_id;
  unsigned char *arguments;
  
  packet_id = get_packet_id(data, &arguments);
  
  if (packet_id == PAK_CLIENT_Attack)
  {
    printf("Attacking unit:\n\
      x_coord -> %d\n\
      y_coord -> %d\n\
      unit_id -> %d\n",
      read_short(arguments, 0),
      read_short(arguments, 2),
      read_long(arguments, 4));
  }
  
  else if (packet_id == PAK_CLIENT_CastSpell)
  {
    printf("Spelling:\n\
      spell_id        -> %d\n\
      target_x_coord  -> %d\n\
      target_y_coord  -> %d\n\
      target_unit_id  -> %d\n",
      read_short(arguments, 0),
      read_short(arguments, 2),
      read_short(arguments, 4),
      read_long(arguments, 6));
  }
  
  return (1); // Reinject the packet
}