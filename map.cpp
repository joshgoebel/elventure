#include <avr/pgmspace.h>
#include "ArduboyGamby.h"
#include "elf.h"
#include "elf_bitmap.h"

#include "map.h"
#include "map_bitmap.h"
#include "map_data.h"
#include "room.h"
#include "sound.h"

extern GambyGraphicsMode gamby;

#define SIZEOF_MAP_RECORD 10

#define SCROLL_UP 0
#define SCROLL_DOWN 1
#define SCROLL_LEFT 2
#define SCROLL_RIGHT 3

Vector transition_vectors[] = {{0,1,0},{0,-1,0},{1,0,0},{-1,0,0}};

//store the current room
char map_curr_room = 0;



uint8_t checkCollision(Rect box, Vector v)
{ 
  uint8_t tile;

  // new position would be
  box.x += v.x * v.len;
  box.y += v.y * v.len;
  // calculate bounding box
  byte x2 = box.x + box.width - 1;
  byte y2 = box.y + box.height - 1;


  // basic bounds
  if (( box.x<0 || x2 > 95) || (box.y<0 || y2 > 63)) {
    return OFFSCREEN;
  }

  uint8_t grid_x = box.x / 8;
  uint8_t grid_y = box.y / 8 ;
  uint8_t grid_x2 = x2 / 8;// + (x2 & 0x07 ? 1 : 0);
  uint8_t grid_y2 = y2 / 8;// + (y2 % 8 > 3 ? 1 : 0);

  // arduboy.setCursor(0,0);
  // arduboy.print("screen");
  // arduboy.print(box.x);
  // arduboy.print("x");
  // arduboy.print(box.y);
  // arduboy.println();
  // arduboy.print(x2);
  // arduboy.print("x");
  // arduboy.println(y2);
  // arduboy.println();

  // arduboy.print("grid");
  // arduboy.print(grid_x);
  // arduboy.print("x");
  // arduboy.print(grid_y);
  // arduboy.println();
  // arduboy.print(grid_x2);
  // arduboy.print("x");
  // arduboy.println(grid_y2);

  // check our entire overlap for anything over than
  // floor
  for (uint8_t x = grid_x; x <= grid_x2; x++) {
    for (uint8_t y = grid_y; y <= grid_y2; y++) {
      tile = getMapBlock(x, y, map_curr_room);
      // arduboy.println(tile);
      if (tile != 0) {
        // arduboy.print("collide ");
        // arduboy.print(x);
        // arduboy.print("x");
        // arduboy.print(y);
        return MAP_COLLISION;
      }
    }
  }

  return NO_COLLISION;
}

uint8_t findSmoothRoute(Rect box, Vector &fv)
{
  char *alt;
  Vector v;
  v.x = fv.x;
  v.y = fv.y;
  // arduboy.println((uint8_t)fv.x);
  // arduboy.println((uint8_t)fv.y);
  v.len = fv.len;
  if (fv.y == 0) {
    alt = &v.y;
  } else {
    alt = &v.x;
  }
  
  // arduboy.print("alt ");
  // arduboy.println(*alt);
  // arduboy.println("find smooth");
  // arduboy.println();

  *alt = 1;
  if(!checkCollision(box, v)) { 
    fv.x = v.x;
    fv.y = v.y;
    // arduboy.print("found ");    
    // arduboy.print((uint8_t)v.x);
    // arduboy.print("x");
    // arduboy.println((uint8_t)v.y);
  
    return NO_COLLISION; }
  *alt = -1;
  if(!checkCollision(box, v)) { 
    fv.x = v.x;
    fv.y = v.y;

    // arduboy.print("found ");    
    // arduboy.print((uint8_t)v.x);
    // arduboy.print("x");
    // arduboy.println((uint8_t)v.y);

    return NO_COLLISION; }

  return MAP_COLLISION;
}


//check the current room movement
char checkMapRoomMove(char x, char y)
{
  char map_x = x / 8;
  char map_y = y / 8;
        if ((map_x > 11) || (map_y > 7)) return 0;
  return getMapBlock(map_x, map_y, map_curr_room);
}

//return the map block based on map coordinates
char getMapBlock(char map_x, char map_y, char room)
{
  char index_ptr = 0;
  
  //determine the index start for room data 
  index_ptr = pgm_read_byte(map_room_data + (room * 12) + map_x);
  
  //determine which pattern is being referenced
  //and return this value
  return pgm_read_byte (map_pattern_data + (index_ptr * 8) + map_y);
}

void drawMapRoom()
{ 
  drawMapElements(CURRENT_ROOM, (Vector){0,0,0});
}

void drawMapElements(char room, Vector vector)
{
  int tile;

  uint16_t x_off = vector.x * vector.len;
  byte y_off = vector.y * vector.len;

  //if no room parameter is passed, assume the current room
  if (room == CURRENT_ROOM) {
    room = map_curr_room;
  }

  for (char y=0; y<8; y++)
  {
    for (char x=0; x<12; x++)
    {
      // prevent what would be offscreen drawing outside
      // of the map viewport
      if (x * 8 + x_off > 95) {
        continue;
      }

      //determine the current block before trying to draw
      tile = getMapBlock(x,y, room);
      gamby.drawSprite(x * 8 + x_off, 
        y * 8 + y_off, map_bitmap, tile);
    }
  }
}


void roomTransition(char room_start, char room_finish, char direction)
{
  char offset = -64;
  char c = 0;
  char elf_offset = 0;
  char elf_increment = 6;
  Elf elf = getElf();
  Vector vector = transition_vectors[direction];

  // we must be moving in the X direction 
  if (vector.y == 0) {
    offset = -96;
    elf_increment = 7;
  }

  while (offset) {
    // pre-increment to allow one last loop at offset==0 
    offset+=8;
    c+=8;
    elf_offset += elf_increment;

    // room_start stars with a 1 offset (1 pixel offscreen)
    vector.len = c;
    drawMapElements(room_start, vector);

    // room finish starts with max offset (1 pixel onscreen)
    vector.len = offset;
    drawMapElements(room_finish, vector);

    // elf walks quickly as room transitions
    elf.step = (elf.step == 1) ? 2 : 1;

    gamby.drawSprite(elf.x +
      (vector.x*elf_offset), elf.y + (vector.y*elf_offset), elf_bitmap, elf.facing );
    gamby.drawSprite(elf.x +
      (vector.x*elf_offset), elf.y + (vector.y*elf_offset) + 8, elf_bitmap, elf.facing + elf.step); 

    gamby.update();
    update_sound();
    // delay(50);
  } 

}


//initiates the scrolling of the map in a particular direction
void scrollMap(char direction)
{
  unsigned char next_map_room;

  switch (direction)
  {
    case SCROLL_UP:
      next_map_room = map_curr_room - MAP_WIDTH;
      break;
     
    case SCROLL_DOWN:
      next_map_room = map_curr_room + MAP_WIDTH;
      break;
     
    case SCROLL_LEFT:
      next_map_room = map_curr_room - 1;
      break;
     
    case SCROLL_RIGHT:
      next_map_room = map_curr_room + 1;
      break;
  }

  // this will also cover the next_map_room == 255 overflow situation 
  // from map_curr_room - 1
  if (next_map_room > MAP_ROOM_COUNT) {
    return;
  }

  roomTransition(map_curr_room, next_map_room, direction);
  
  // gamby.clearScreen();
  // delay(200);
  setMapRoom(next_map_room);
}

//return the current room value
char getMapCurrentRoom() { return map_curr_room; }

//set the map to a specific room (portals)
void setMapRoom(char room)
{
  map_curr_room = room;
  loadRoomElemments(room);
  drawMapRoom();

  gamby.update();
}

