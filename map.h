#ifndef __MAP__
#define __MAP__

#include "physics.h"

#define SCROLL_UP 0
#define SCROLL_DOWN 1
#define SCROLL_LEFT 2
#define SCROLL_RIGHT 3

#define CURRENT_ROOM -1

#define OFFSCREEN 55
#define MAP_COLLISION 11
#define NO_COLLISION 0


typedef struct {
  char x;
  char y;
  char len;
} Vector;

uint8_t checkCollision(Rect box, Vector v);
uint8_t findSmoothRoute(Rect box, Vector &fv);

char checkMapRoomMove(char x, char y);
char getMapBlock(char map_x, char map_y, char room);
void drawMapRoom();
void scrollMap(char direction);
char getMapCurrentRoom();
void roomTransition(char room_start, char room_finish, char direction);
void setMapRoom(char room);
void drawMapElements(char room, Vector vector);

#endif __MAP__
