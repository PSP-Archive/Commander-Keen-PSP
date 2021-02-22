/* GRAPHICS.C
  This file contains low- to mid-level graphics functions,
  which are NOT platform-specific. All the low-level stuff in
  here is stuff that draws to the scroll buffer (and so is
  not platform-specific).
*/

#include "keen.h"

typedef struct stColour
{
  int r,g,b;
} stColour;
stColour palette[256];

char allocmem(void)
{
  VidDrv_printf("allocmem(): allocating %d bytes for scroll buffer...", SCROLLBUF_MEMSIZE);
  scrollbuf = malloc(SCROLLBUF_MEMSIZE);
  if (!scrollbuf)
  {
     VidDrv_printf("Failure\n");
     return 1;
  } else VidDrv_printf("OK\n");

  if (options[OPT_ZOOM].value > 1)
  {
     VidDrv_printf("allocmem(): allocating %d bytes for blit buffer...", BLITBUF_MEMSIZE);
     blitbuf = malloc(BLITBUF_MEMSIZE);
     if (!blitbuf)
     {
        VidDrv_printf("Failure\n");
        return 1;
     } else VidDrv_printf("OK\n");
  }

  return 0;
}

void freemem(void)
{
  if (scrollbuf)
  {
     free(scrollbuf);
     VidDrv_printf("  * Scrollbuffer memory released to system.\n");
  }
  if (blitbuf)
  {
     free(blitbuf);
     VidDrv_printf("  * Blitbuffer memory released to system.\n");
  }
}

void inline sb_setpixel(int x, int y, unsigned char c)
{
  scrollbuf[(y<<9) + x] = c;
}

unsigned char inline sb_getpixel(int x, int y)
{
  return scrollbuf[(y<<9) + x];
}

// draw a tile directly to the display (bypass the scroll buffer)
void drawtile_direct(int x, int y, unsigned int t)
{
unsigned char xa,ya;
  for(ya=0;ya<16;ya++)
   for(xa=0;xa<16;xa++)
    setpixel(x+xa, y+ya, tiledata[t][ya][xa]);
}

// draws a sprite directly to the display (only used by status window)
void drawsprite_direct(int x, int y, unsigned int t)
{
unsigned char xa,ya;
  for(ya=0;ya<sprites[t].ysize;ya++)
   for(xa=0;xa<sprites[t].xsize;xa++)
    if (sprites[t].maskdata[ya][xa])
     setpixel(x+xa, y+ya, sprites[t].imgdata[ya][xa]);
}


void drawtile(int x, int y, unsigned int t)
{
unsigned char xa,ya;
unsigned char *offset = &scrollbuf[(y<<9)+x];
  for(ya=0;ya<16;ya++)
  {
    memcpy(offset, &tiledata[t][ya][0], 16);
    offset+=512;
  }
}

// draws a masked tile ("til") to the scrollbuffer.
// adjusts based on the X&Y scroll so that when the buffer is blitted
// the tile will appear at (x,y). only pixels which have a corresponding
// black pixel in tile "tmask" will be drawn.
void drawtilewithmask(int x, int y, unsigned int til, unsigned int tmask)
{
unsigned char xa,ya;
unsigned int bufoffX,bufoffY;
unsigned int xstart,ystart;
// clip the tile
  if (x>320 || y>200) return;
  if (x<-16||y<-16) return;
  if (x<0) xstart=-x; else xstart = 0;
  if (y<0) ystart=-y; else ystart = 0;

  bufoffY = ((y+ystart+scrolly_buf)&511)<<9;   // points to start of line
  for(ya=ystart;ya<16;ya++)
  {
    bufoffX = (x+xstart+scrollx_buf)&511;       // offset within line
    for(xa=xstart;xa<16;xa++)
    {
      if (tiledata[tmask][ya][xa] != 15)
      {
        scrollbuf[bufoffY+bufoffX] = tiledata[til][ya][xa];
      }
      bufoffX = (bufoffX+1)&511;
    }
    // move to next line and wrap to top of buffer if needed
    bufoffY += 512;
    if (bufoffY >= (512*512)) bufoffY = 0;
  }
}

// draws a tile ("til") to the scrollbuffer. adjusts based on the X&Y scroll
// so that when the buffer is blitted the tile will appear at (x,y).
// used for priority tiles (tiles[].priority)
void drawprioritytile(int x, int y, unsigned int til)
{
unsigned char xa,ya;
unsigned int bufoffX,bufoffY;
unsigned int xstart,ystart;
// clip the tile
  if (x>320 || y>200) return;
  if (x<-16 || y<-16) return;
  if (x<0) xstart=-x; else xstart = 0;
  if (y<0) ystart=-y; else ystart = 0;

  bufoffY = ((y+ystart+scrolly_buf)&511)<<9;    // points to start of line
  for(ya=ystart;ya<16;ya++)
  {
    bufoffX = (x+xstart+scrollx_buf)&511;       // offset within line
    for(xa=xstart;xa<16;xa++)
    {
       scrollbuf[bufoffY+bufoffX] = tiledata[til][ya][xa];
       bufoffX = (bufoffX+1)&511;
    }
    // move to next line and wrap to top of buffer if needed
    bufoffY += 512;
    if (bufoffY >= (512*512)) bufoffY = 0;
  }
}


// draws a sprite to the scrollbuffer.
// adjusts based on the X&Y scroll so that when the buffer is blitted
// the sprite will appear at (x,y). saves the image beneath the sprite
// into the erasedata[] of object objectnum.
void drawsprite(int x, int y, unsigned int s, int objectnum)
{
unsigned char xa,ya;
unsigned int bufoffX, bufoffY;
unsigned int xstart,ystart;

// clip the sprite
  if (x>320 || y>200) return;
  if (x<-sprites[s].xsize||y<-sprites[s].ysize) return;
  // if sprite is partially off the top or left of the screen, invert
  // the sign on the coordinate to make it positive, and start drawing
  // the sprite from there.
  if (x<0) xstart=-x; else xstart = 0;
  if (y<0) ystart=-y; else ystart = 0;

  bufoffY = ((y+ystart+scrolly_buf)&511)<<9;   // points to start of line
  for(ya=ystart;ya<sprites[s].ysize;ya++)
  {
   bufoffX = (x+xstart+scrollx_buf)&511;       // offset within line
   for(xa=xstart;xa<sprites[s].xsize;xa++)
   {
     if (sprites[s].maskdata[ya][xa])
     {
       objects[objectnum].erasedata[ya][xa] = scrollbuf[bufoffY+bufoffX];
       scrollbuf[bufoffY+bufoffX] = sprites[s].imgdata[ya][xa];
     }
     bufoffX = (bufoffX+1)&511;
   }
   // move to next line and wrap to top of buffer if needed
   bufoffY += 512;
   if (bufoffY >= (512*512)) bufoffY = 0;
  }
}

// complement of drawsprite(). uses the saved image in objectnum to erase
// a previously-drawn sprite.
void erasesprite(int x, int y, unsigned int s, int objectnum)
{
unsigned char xa,ya;
unsigned int bufoffX, bufoffY;
unsigned int xstart,ystart;

// clip the sprite
  if (x>320 || y>200) return;
  if (x<-sprites[s].xsize||y<-sprites[s].ysize) return;
  // if sprite is partially off the top or left of the screen, invert
  // the sign on the coordinate to make it positive, and start drawing
  // the sprite from there.
  if (x<0) xstart=-x; else xstart = 0;
  if (y<0) ystart=-y; else ystart = 0;

  bufoffY = ((y+ystart+scrolly_buf)&511)<<9;   // points to start of line
  for(ya=ystart;ya<sprites[s].ysize;ya++)
  {
   bufoffX = (x+xstart+scrollx_buf)&511;       // offset within line
   for(xa=xstart;xa<sprites[s].xsize;xa++)
   {
     if (sprites[s].maskdata[ya][xa])
     {
       scrollbuf[bufoffY+bufoffX] = objects[objectnum].erasedata[ya][xa];
     }
     bufoffX = (bufoffX+1)&511;
   }
   // move to next line and wrap to top of buffer if needed
   bufoffY += 512;
   if (bufoffY >= (512*512)) bufoffY = 0;
  }
}

void drawcharacter(int x, int y, int f)
{
unsigned char xa,ya;

  for(ya=0;ya<8;ya++)
  {
    for(xa=0;xa<8;xa++)
    {
        setpixel(x+xa, y+ya, font[f][ya][xa]);
    }
  }
}

void sb_drawcharacter(int x, int y, int f)
{
unsigned char xa,ya;
unsigned int yb;

  for(ya=0;ya<8;ya++)
  {
    yb = ((y+ya+scrolly_buf)&511)<<9;
    for(xa=0;xa<8;xa++)
    {
        scrollbuf[yb+((x+xa+scrollx_buf)&511)] = font[f][ya][xa];
    }
  }
}

unsigned char savebuf[200][320];
void save_area(int x1, int y1, int x2, int y2)
{
unsigned char xa,ya;
unsigned int yb;

  for(ya=0;ya<y2-y1;ya++)
  {
    yb = ((y1+ya+scrolly_buf)&511)<<9;
    for(xa=0;xa<x2-y1;xa++)
    {
      savebuf[ya][xa] = scrollbuf[yb+((x1+xa+scrollx_buf)&511)];
    }
  }
}

void restore_area(int x1, int y1, int x2, int y2)
{
unsigned char xa,ya;
unsigned int yb;

  for(ya=0;ya<y2-y1;ya++)
  {
    yb = ((y1+ya+scrolly_buf)&511)<<9;
    for(xa=0;xa<x2-y1;xa++)
    {
      scrollbuf[yb+((x1+xa+scrollx_buf)&511)] = savebuf[ya][xa];
    }
  }
}

char Graphics_Start(void)
{
  // tell the video driver (platform-specific) to start up
  if (VidDrv_Start())
  {
    printf("Graphics_Start(): VidDrv_Start() failed to initilize display\n");
    return 1;
  }

  // set up the palette
  VidDrv_printf("Graphics_Start(): configuring palette.\n");
  pal_init(0);
  pal_fade(0);
  return 0;
}

void Graphics_Stop(void)
{
  // shut down the video driver
  VidDrv_Stop();
}

void configpal(int c, int r, int g, int b)
{
  palette[c].r = r;
  palette[c].g = g;
  palette[c].b = b;
}

// loads the EGA palette into the palette[] array. if dark=1, loads in
// the palette used when the lights are off (in ep2)
void pal_init(int dark)
{
  if (!dark)
  {
     configpal(0, 0x00,0x00,0x00);
     configpal(1, 0x00,0x00,0xa8);
     configpal(2, 0x00,0xa8,0x00);
     configpal(3, 0x00,0xa8,0xa8);
     configpal(4, 0xa8,0x00,0x00);
     configpal(5, 0xa8,0x00,0xa8);
     configpal(6, 0xa8,0x54,0x00);
     configpal(7, 0xa8,0xa8,0xa8);
     configpal(8, 0x54,0x54,0x54);
     configpal(9, 0x54,0x54,0xfc);
     configpal(10, 0x54,0xfc,0x54);
     configpal(11, 0x54,0xfc,0xfc);
     configpal(12, 0xfc,0x54,0x54);
     configpal(13, 0xfc,0x54,0xfc);
     configpal(14, 0xfc,0xfc,0x54);
     configpal(15, 0xfc,0xfc,0xfc);
  }
  else
  {
     configpal(0, 0x00,0x00,0x00);
     configpal(1, 0x00,0x00,0x00);
     configpal(2, 0x00,0x00,0x00);
     configpal(3, 0x00,0x00,0x00);
     configpal(4, 0x00,0x00,0x00);
     configpal(5, 0x00,0x00,0x00);
     configpal(6, 0x00,0x00,0x00);
     configpal(7, 0x54,0x54,0x54);
     configpal(8, 0x00,0x00,0x00);
     configpal(9, 0x00,0x00,0xa8);
     configpal(10, 0x00,0xa8,0x00);
     configpal(11, 0x00,0xa8,0xa8);
     configpal(12, 0xa8,0x00,0x00);
     configpal(13, 0xa8,0x00,0xa8);
     configpal(14, 0xa8,0x54,0x00);
     configpal(15, 0xa8,0xa8,0xa8);
  }

  // 16 is black, for flashing during vorticon death sequence
  // (all black in the graphics is mapped to 16, then the border around
  // the screen is the only thing left at color 0, so we can change 0's
  // palette to change the border color)
  configpal(16,0x00,0x00,0x00);
}

char fade_black = 0;
void pal_fade(int fadeamt)
{
int c;
int r,g,b;
if (framebyframe) fadeamt = PAL_FADE_SHADES;

   for(c=0;c<17;c++)
   {
      r = palette[c].r;
      g = palette[c].g;
      b = palette[c].b;

      if (fadeamt != PAL_FADE_SHADES)
      {
         if ((c==0||c==16) && fadeamt > PAL_FADE_SHADES && fade_black)
         {
            r = 255 / (PAL_FADE_WHITEOUT - PAL_FADE_SHADES);
            r = (r * (fadeamt - (PAL_FADE_WHITEOUT - PAL_FADE_SHADES)));
            g = b = r;
         }
         else
         {
            r /= PAL_FADE_SHADES;
            g /= PAL_FADE_SHADES;
            b /= PAL_FADE_SHADES;

            r *= fadeamt;
            g *= fadeamt;
            b *= fadeamt;
         }

         if (r > 0xff) r = 0xff;
         if (g > 0xff) g = 0xff;
         if (b > 0xff) b = 0xff;
      }

      pal_set(c, r, g, b);
   }
   pal_apply();
}

void DrawBitmap(int xa, int ya, int b)
{
int x,y;
unsigned char *bmdataptr;

  // for "b" arguments passed from GetBitmapNumberFromName(),
  // in case the specified name was not found
  if (b==-1) return;

  bmdataptr = bitmaps[b].bmptr;
  for(y=0;y<bitmaps[b].ysize;y++)
  {
   for(x=0;x<bitmaps[b].xsize;x++)
   {
     sb_setpixel((x+xa+scrollx_buf)&511,(y+ya+scrolly_buf)&511,*bmdataptr);
     bmdataptr++;
   }
  }
}

int GetBitmapNumberFromName(char *bmname)
{
int i;
  for(i=0;i<MAX_BITMAPS;i++)
  {
    bitmaps[i].name[8] = 0;     // ensure null-terminated
    if (!strcmp(bmname, bitmaps[i].name))
    {
      return i;
    }
  }
  return -1;
}


void sb_drawcharacterinverse(int x, int y, int f)
{
unsigned char xa,ya;
unsigned int yb;
int c;

  for(ya=0;ya<8;ya++)
  {
    yb = ((y+ya+scrolly_buf)&511)<<9;
    for(xa=0;xa<8;xa++)
    {
      if (font[f][ya][xa]!=15) c=11; else c=0;
      scrollbuf[yb+((x+xa+scrollx_buf)&511)] = c;
    }
  }
}

// font drawing functions
void font_draw(unsigned char *text, int xoff, int yoff, int highlight)
{
int i,x=xoff,y;
int c;

   y = yoff;
   for(i=0;i<strlen(text);i++)
   {
     c = text[i];
     if (!c) break;
     if (c!=13)
     {
       if (highlight) c|=128;
       drawcharacter(x, y, c);
       x+=8;
     }
     else
     {
       x=xoff;
       y+=8;
     }
   }
}
void sb_font_draw(unsigned char *text, int xoff, int yoff)
{
int i,x,y;

   x=xoff;
   y=yoff;
   for(i=0;i<strlen(text);i++)
   {
     if (!text[i]) break;
     if (text[i]!=13)
     {
       sb_drawcharacter(x, y, text[i]);
       x+=8;
     }
     else
     {
       x=xoff;
       y+=8;
     }
   }
}
void sb_font_draw_inverse(unsigned char *text, int xoff, int yoff)
{
int i,x=xoff,y;

   y=yoff;
   for(i=0;i<strlen(text);i++)
   {
     if (!text[i]) break;
     if (text[i]!=13)
     {
       sb_drawcharacterinverse(x, y, text[i]);
       x+=8;
     }
     else
     {
       x=xoff;
       y+=8;
     }
   }
}
