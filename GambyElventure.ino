/*                                   __    
                                   /___/
                                   / /
   /-------------/              __/_/__
     |  |-------/       //===//__<*>_ //===//
     |  .----/                 / / /
     |  .---/                 / / /
     |  |------/             / / /
   /----------/ L + V + E + N + T + U + R + E *=-
   An Adventure Game for the Gamby Game Shield 
   v3.1

   2013 - trodoss
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
 */
 

#include <Arduboy.h>
#include <ArduboyExtra.h>
#include "ArduboyGamby.h"
#include <avr/pgmspace.h>
#include <physics.h>

#define SCREENWIDTH     96
#define SCREENHEIGHT    64

#include "map.h"
#include "room.h"
#include "elf.h"
#include "display.h"
#include "logo.h"
#include "sound.h"

#define PAUSE_BETWEEN_ACTIONS 100

#define PAUSE_BUTTON BUTTON_1
#define THROW_BUTTON BUTTON_2

#define GAME_TITLE   0
#define GAME_PLAYING 1
#define GAME_PAUSED  2
#define GAME_WON     3
#define GAME_OVER    4

Arduboy arduboy;
Physics physics;
GambyGraphicsMode gamby(arduboy);
char game_state;

void setup()
{
  arduboy.start();
  arduboy.audio.on();
  // arduboy.setFrameRate(60);
  arduboy.display();
  arduboy.initRandomSeed();
  
  // randomSeed(analogRead(0));
  
  start_title();
}

void loop()
{  
  static long next_action = 0;
  byte lastInputs;

  if (millis() < next_action) {
    // have to keep our music going
    if (game_state != GAME_PAUSED) {
      update_sound();
    }
    // sleep for ~1ms until millis() timer wakes us
    arduboy.idle();
    // wait until the next timing loop
    return;
  }

  // almost every loop we need to work with input
  gamby.readInputs();
  lastInputs = gamby.inputs;

  switch (game_state)
  {
    case GAME_TITLE:
  	case GAME_OVER:
  	case GAME_WON:
      if (millis() >= next_action)
      {
        //wait for a button to be pressed to continue
        if (lastInputs & THROW_BUTTON)
        {
          if (game_state == GAME_WON)
          {
            //if the game has been won, then reset the inventory of the elf
            resetElf(true);
          } else if (game_state == GAME_OVER) {
            //if the game was just over (died) then do not reset the inventory
            resetElf(false);
          }
          start_game();
        }
        next_action = millis() + PAUSE_BETWEEN_ACTIONS; 
      }	
	    break;
	   
    case GAME_PLAYING:
      if (millis() >= next_action)
      {    
        if (lastInputs & DPAD_UP) moveElf(FACING_UP);
        if (lastInputs & DPAD_DOWN) moveElf(FACING_DOWN);
        if (lastInputs & DPAD_RIGHT) moveElf(FACING_RIGHT);
        if (lastInputs & DPAD_LEFT) moveElf(FACING_LEFT);
        if (lastInputs & THROW_BUTTON) throwSword();
        if (lastInputs & PAUSE_BUTTON) 
        {
         drawDisplay(getElf());
         game_state = GAME_PAUSED;
        }
        
	      if (game_state != GAME_PAUSED)
        {
          handleRoomElements();
		 
          //check the elf's state
          if (getElfState() != ELFSTATE_PLAYING)
          {
            if (getElfState() == ELFSTATE_DEAD)
            {
             start_game_over();
            } else {
             start_game_won();
            }
          }
        }
        next_action = millis() + PAUSE_BETWEEN_ACTIONS; 
      }
      gamby.update();
      break;
 
    case GAME_PAUSED:
      if (millis() >= next_action)
      {
        if (lastInputs & PAUSE_BUTTON)
        {
          //redraw the current screen and continue
          gamby.clearScreen();
          drawMapElements(CURRENT_ROOM, (Vector){0,0,0});
          showElf();
          gamby.update();
          game_state = GAME_PLAYING;
        }
       
        next_action = millis() + PAUSE_BETWEEN_ACTIONS; 
      }
      break;       
  }
}

void start_game()
{
  //clear the contents of the screen
  gamby.clearScreen();
  gamby.update();
  
  //start the game
  setMapRoom(0);
  showElf();
  play_song(0);
  game_state = GAME_PLAYING;  
}

void start_title()
{
  //clear the contents of the screen
  gamby.clearScreen();
  gamby.update();
  
  game_state = GAME_TITLE;
  drawLogo(0);
  play_song(4);
}

void start_game_over()
{
  //clear the contents of the screen
  gamby.clearScreen();
  gamby.update();
  
  game_state = GAME_OVER;
  drawLogo(4);
  play_song_once(2);
}

void start_game_won()
{
  //clear the contents of the screen
  gamby.clearScreen();
  gamby.update();
  
  game_state = GAME_WON;
  drawLogo(8);
  play_song(3);
}
