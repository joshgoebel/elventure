#include "ArduboyGamby.h"

byte GambyGraphicsMode::inputs = 0;
byte GambyGraphicsMode::column_pos;
byte GambyGraphicsMode::page_pos;

GambyGraphicsMode::GambyGraphicsMode(Arduboy &a)
{
  arduboy = &a;
}

void GambyGraphicsMode::clearScreen()
{
  // blank the hardware, not the buffer
  arduboy->blank();
}

void GambyGraphicsMode::update()
{
  // paint the buffer to the screen
  arduboy->display();
}

void GambyGraphicsMode::readInputs()
{
  inputs = arduboy->getInput();
}

void GambyGraphicsMode::drawSprite(byte x, byte y, byte w, byte h, const byte *sprite) {
  byte this_byte;
  byte this_bit=8;
  for (byte i=0; i<w; i++) {
    for (byte j=h; j>0; j--) {
      if (this_bit == 8) {
        sprite++;
        this_byte = ~pgm_read_byte(sprite);
        this_bit=0;
      }
      arduboy->drawPixel(x+i,y+j,(this_byte << this_bit) & B10000000);
      this_bit++;
    }
  }
}

void GambyGraphicsMode::drawSprite(byte x, byte y, const byte *sprite) {
  drawSprite(x,y,sprite, (byte)0);
}


void GambyGraphicsMode::drawSprite(byte x, byte y, const byte *sprite, byte frame) {
  byte w = pgm_read_byte(sprite);
  byte h = pgm_read_byte(++sprite);

  // TODO: This can probably be optimized.
  if (frame > 0) {
    unsigned int pixelsPerFrame = h * w;
    // Round up to the nearest 8 bits
    sprite += ((pixelsPerFrame & 0x7 ? ((pixelsPerFrame &  0xfff8) + 8) : pixelsPerFrame) >> 3) * frame;
  }

  drawSprite(x,y,w,h,++sprite);
}

// for drawing icons

void GambyGraphicsMode::setPos(byte col, byte page)
{
  column_pos = col;
  page_pos = page;
}


// icons are always 8 pixels tall
void GambyGraphicsMode::drawIcon(byte width, const byte *icon)
{
  column_pos += width;

  byte x = column_pos;
  byte y = page_pos * GAMBY_PAGE_HEIGHT;

  drawSprite(x, y, width, GAMBY_ICON_HEIGHT, icon);
}

void GambyGraphicsMode::drawIcon(const byte *icon) {
  drawIcon(icon, 0);
}

void GambyGraphicsMode::drawIcon(const byte *icon, byte frame) {
  byte w = pgm_read_byte(icon++);
  if (frame > 0 ) {
    icon += w * frame; 
  }

  drawIcon(w, icon);
}



