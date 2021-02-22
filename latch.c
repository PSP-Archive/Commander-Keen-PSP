/* LATCH.C
  This module is reponsible for decoding the EGALATCH and EGASPRIT
  graphics data.
*/

#include "keen.h"

unsigned long BitmapBufferRAMSize;

unsigned long getbit_bytepos[5];
unsigned char getbit_bitmask[5];

EgaHead LatchHeader;
SpriteHead *SpriteTable = NULL;
BitmapHead *BitmapTable = NULL;
char *BitmapData;
char *RawData;

// initilizes the positions getbit will retrieve data from
void setplanepositions(unsigned long p1, unsigned long p2, unsigned long p3,\
                       unsigned long p4, unsigned long p5)
{
int i;
  getbit_bytepos[0] = p1;
  getbit_bytepos[1] = p2;
  getbit_bytepos[2] = p3;
  getbit_bytepos[3] = p4;
  getbit_bytepos[4] = p5;

  for(i=0;i<=4;i++)
  {
    getbit_bitmask[i] = 128;
  }
}

// retrieves a bit from plane "plane". the positions of the planes
// should have been previously initilized with setplanepositions()
unsigned char getbit(char *buf, unsigned char plane)
{
int retval;
int byt;
  if (!getbit_bitmask[plane])
  {
    getbit_bitmask[plane] = 128;
    getbit_bytepos[plane]++;
  }

  byt = buf[getbit_bytepos[plane]];

  if (byt & getbit_bitmask[plane])
  {
    retval = 1;
  }
  else
  {
    retval = 0;
  }

  getbit_bitmask[plane] >>= 1;

  return retval;
}

// load the EGAHEAD file
char latch_loadheader(int episode)
{
FILE *headfile;
char fname[80];
unsigned long SpriteTableRAMSize;
unsigned long BitmapTableRAMSize;
char buf[12];
int i,j,k;

    sprintf(fname, "data/EGAHEAD.CK%c", episode + '0');
 
    headfile = fopen(fname, "rb");
    if (!headfile)
    {
      VidDrv_printf("latch_loadheader(): unable to open '%s'.\n", fname);
      return 1;
    }

    VidDrv_printf("latch_loadheader(): reading main header from '%s'...\n", fname);

    // read the main header data from EGAHEAD
    LatchHeader.LatchPlaneSize = fgetl(headfile);
    LatchHeader.SpritePlaneSize = fgetl(headfile);
    LatchHeader.OffBitmapTable = fgetl(headfile);
    LatchHeader.OffSpriteTable = fgetl(headfile);
    LatchHeader.Num8Tiles = fgeti(headfile);
    LatchHeader.Off8Tiles = fgetl(headfile);
    LatchHeader.Num32Tiles = fgeti(headfile);
    LatchHeader.Off32Tiles = fgetl(headfile);
    LatchHeader.Num16Tiles = fgeti(headfile);
    LatchHeader.Off16Tiles = fgetl(headfile);
    LatchHeader.NumBitmaps = fgeti(headfile);
    LatchHeader.OffBitmaps = fgetl(headfile);
    LatchHeader.NumSprites = fgeti(headfile);
    LatchHeader.OffSprites = fgetl(headfile);
    LatchHeader.Compressed = fgeti(headfile);

    VidDrv_printf("   LatchPlaneSize = 0x%04x\n", LatchHeader.LatchPlaneSize);
    VidDrv_printf("   SpritePlaneSize = 0x%04x\n", LatchHeader.SpritePlaneSize);
    VidDrv_printf("   OffBitmapTable = 0x%04x\n", LatchHeader.OffBitmapTable);
    VidDrv_printf("   OffSpriteTable = 0x%04x\n", LatchHeader.OffSpriteTable);
    VidDrv_printf("   Num8Tiles = %d\n", LatchHeader.Num8Tiles);
    VidDrv_printf("   Off8Tiles = 0x%04x\n", LatchHeader.Off8Tiles);
    VidDrv_printf("   Num32Tiles = %d\n", LatchHeader.Num32Tiles);
    VidDrv_printf("   Off32Tiles = 0x%04x\n", LatchHeader.Off32Tiles);
    VidDrv_printf("   Num16Tiles = %d\n", LatchHeader.Num16Tiles);
    VidDrv_printf("   Off16Tiles = 0x%04x\n", LatchHeader.Off16Tiles);
    VidDrv_printf("   NumBitmaps = %d\n", LatchHeader.NumBitmaps);
    VidDrv_printf("   OffBitmaps = 0x%04x\n", LatchHeader.OffBitmaps);
    VidDrv_printf("   NumSprites = %d\n", LatchHeader.NumSprites);
    VidDrv_printf("   OffSprites = 0x%04x\n", LatchHeader.OffSprites);
    VidDrv_printf("   Compressed = %d\n", LatchHeader.Compressed);

    /** read in the sprite table **/

    // allocate memory for the sprite table
    SpriteTableRAMSize = sizeof(SpriteHead) * (LatchHeader.NumSprites + 1);
    VidDrv_printf("latch_loadheader(): Allocating %d bytes for sprite table.\n", SpriteTableRAMSize);

    SpriteTable = malloc(SpriteTableRAMSize);
    if (!SpriteTable)
    {
      VidDrv_printf("latch_loadheader(): Can't allocate sprite table!\n");
      return 1;
    }

    VidDrv_printf("latch_loadheader(): Reading sprite table from '%s'...\n", fname);

    fseek(headfile, LatchHeader.OffSpriteTable, SEEK_SET);
    for(i=0;i<LatchHeader.NumSprites;i++)
    {
      SpriteTable[i].Width = fgeti(headfile) * 8;
      SpriteTable[i].Height = fgeti(headfile);
      SpriteTable[i].OffsetDelta = fgeti(headfile);
      SpriteTable[i].OffsetParas = fgeti(headfile);
      SpriteTable[i].Rx1 = (fgeti(headfile) >> 8);
      SpriteTable[i].Ry1 = (fgeti(headfile) >> 8);
      SpriteTable[i].Rx2 = (fgeti(headfile) >> 8);
      SpriteTable[i].Ry2 = (fgeti(headfile) >> 8);
      for(j=0;j<16;j++) SpriteTable[i].Name[j] = fgetc(headfile);
      // for some reason each sprite occurs 4 times in the table.
      // we're only interested in the first occurance.
      for(j=0;j<3;j++)
      {
        for(k=0;k<sizeof(SpriteHead);k++) fgetc(headfile);
      }

    }

    /** read in the bitmap table **/

    // allocate memory for the bitmap table
    BitmapTableRAMSize = sizeof(BitmapHead) * (LatchHeader.NumBitmaps + 1);
    VidDrv_printf("latch_loadheader(): Allocating %d bytes for bitmap table.\n", BitmapTableRAMSize);

    BitmapTable = malloc(BitmapTableRAMSize);
    if (!BitmapTable)
    {
      VidDrv_printf("latch_loadheader(): Can't allocate bitmap table!\n");
      return 1;
    }

    VidDrv_printf("latch_loadheader(): reading bitmap table from '%s'...\n", fname);

    fseek(headfile, LatchHeader.OffBitmapTable, SEEK_SET);

    BitmapBufferRAMSize = 0;
    for(i=0;i<LatchHeader.NumBitmaps;i++)
    {
      BitmapTable[i].Width = fgeti(headfile) * 8;
      BitmapTable[i].Height = fgeti(headfile);
      BitmapTable[i].Offset = fgetl(headfile);
      for(j=0;j<8;j++) BitmapTable[i].Name[j] = fgetc(headfile);

      // keep a tally of the bitmap sizes so we'll know how much RAM we have
      // to allocate for all of the bitmaps once they're decoded
      BitmapBufferRAMSize += (BitmapTable[i].Width * BitmapTable[i].Height);

      // print the bitmap info to the console for debug
      for(j=0;j<8;j++) buf[j] = BitmapTable[i].Name[j];
      buf[j] = 0;
      VidDrv_printf("   Bitmap '%s': %dx%d at offset %04x. RAMAllocSize=0x%04x\n", buf,BitmapTable[i].Width,BitmapTable[i].Height,BitmapTable[i].Offset,BitmapBufferRAMSize);
    }
    BitmapBufferRAMSize++;

    fclose(headfile);
    return 0;
}

char latch_loadlatch(int episode)
{
FILE *latchfile;
unsigned long plane1, plane2, plane3, plane4;
char fname[80];
int x,y,i,t,b,c,p;
char *bmdataptr;
unsigned long RawDataSize;
unsigned char ch;

    sprintf(fname, "data/EGALATCH.CK%c", episode + '0');
 
    VidDrv_printf("latch_loadlatch(): Opening file '%s'.\n", fname);

    latchfile = fopen(fname, "rb");
    if (!latchfile)
    {
      VidDrv_printf("latch_loadlatch(): Unable to open '%s'!\n", fname);
      return 1;
    }

    // figure out how much RAM we'll need to read all 4 planes of
    // latch data into memory.
    RawDataSize = (LatchHeader.LatchPlaneSize * 4);
    RawData = malloc(RawDataSize);
    if (!RawData)
    {
      VidDrv_printf("latch_loadlatch(): Unable to allocate RawData buffer!\n");
      return 1;
    }

    // get the data out of the file into memory, decompressing if necessary.
    if (LatchHeader.Compressed)
    {
      VidDrv_printf("latch_loadlatch(): Decompressing...\n");
      fseek(latchfile, 6, SEEK_SET);
      if (lz_decompress(latchfile, RawData)) return 1;
    }
    else
    {
      VidDrv_printf("latch_loadlatch(): Reading %d bytes...\n", RawDataSize);
      fread(RawData, RawDataSize, 1, latchfile);
    }
    fclose(latchfile);

    // these are the offsets of the different video planes as
    // relative to each other--that is if a pixel in plane1
    // is at N, the byte for that same pixel in plane3 will be
    // at (N + plane3).
    plane1 = 0;
    plane2 = (LatchHeader.LatchPlaneSize * 1);
    plane3 = (LatchHeader.LatchPlaneSize * 2);
    plane4 = (LatchHeader.LatchPlaneSize * 3);

    // ** read the 8x8 tiles **
    VidDrv_printf("latch_loadlatch(): Decoding 8x8 tiles...\n", fname);

    // set up the getbit() function
    setplanepositions(plane1 + LatchHeader.Off8Tiles, \
                      plane2 + LatchHeader.Off8Tiles, \
                      plane3 + LatchHeader.Off8Tiles, \
                      plane4 + LatchHeader.Off8Tiles, \
                      0);

    for(p=0;p<4;p++)
    {
      for(t=0;t<LatchHeader.Num8Tiles;t++)
      {
        for(y=0;y<8;y++)
        {
          for(x=0;x<8;x++)
          {
            // if we're on the first plane start with black,
            // else merge with the previously accumulated data
            if (p==0)
            {
              c = 0;
            }
            else
            {
              c = font[t][y][x];
            }
            // read a bit out of the current plane, shift it into the
            // correct position and merge it
            c |= (getbit(RawData, p) << p);
            // map black pixels to color 16 because of the way the
            // vorticon death sequence works in ep1
            if (p==3 && c==0) c = 16;
            font[t][y][x] = c;
          }
        }
      }
    }

    // ** read the 16x16 tiles **
    VidDrv_printf("latch_loadlatch(): Decoding 16x16 tiles...\n", fname);

    // set up the getbit() function
    setplanepositions(plane1 + LatchHeader.Off16Tiles, \
                      plane2 + LatchHeader.Off16Tiles, \
                      plane3 + LatchHeader.Off16Tiles, \
                      plane4 + LatchHeader.Off16Tiles, \
                      0);

    for(p=0;p<4;p++)
    {
      for(t=0;t<LatchHeader.Num16Tiles;t++)
      {
        for(y=0;y<16;y++)
        {
          for(x=0;x<16;x++)
          {
            if (p==0)
            {
              c = 0;
            }
            else
            {
              c = tiledata[t][y][x];
            }
            c |= (getbit(RawData, p) << p);
            if (p==3 && c==0) c = 16;
            tiledata[t][y][x] = c;
          }
        }
      }
    }

    // ** read the bitmaps **
    VidDrv_printf("latch_loadlatch(): Allocating %d bytes for bitmap data...\n", BitmapBufferRAMSize);
    BitmapData = malloc(BitmapBufferRAMSize);
    if (!BitmapData)
    {
      VidDrv_printf("Cannot allocate memory for bitmaps.\n");
      return 1;
    }

    VidDrv_printf("latch_loadlatch(): Decoding bitmaps...\n", fname);

    // set up the getbit() function
    setplanepositions(plane1 + LatchHeader.OffBitmaps, \
                      plane2 + LatchHeader.OffBitmaps, \
                      plane3 + LatchHeader.OffBitmaps, \
                      plane4 + LatchHeader.OffBitmaps, \
                      0);

    // decode bitmaps into the BitmapData structure. The bitmaps are
    // loaded into one continous stream of image data, with the bitmaps[]
    // array giving pointers to where each bitmap starts within the stream.

    for(p=0;p<4;p++)
    {
      // this points to the location that we're currently
      // decoding bitmap data to
      bmdataptr = &BitmapData[0];

      for(b=0;b<LatchHeader.NumBitmaps;b++)
      {
        bitmaps[b].xsize = BitmapTable[b].Width;
        bitmaps[b].ysize = BitmapTable[b].Height;
        bitmaps[b].bmptr = bmdataptr;
        memcpy(&bitmaps[b].name[0], &BitmapTable[b].Name[0], 8);
        bitmaps[b].name[8] = 0;  //ensure null-terminated

        for(y=0;y<bitmaps[b].ysize;y++)
        {
          for(x=0;x<bitmaps[b].xsize;x++)
          {
            if (p==0)
            {
              c = 0;
            }
            else
            {
              c = *bmdataptr;
            }
            c |= (getbit(RawData, p) << p);
            if (p==3 && c==0) c = 16;
            *bmdataptr = c;
            bmdataptr++;
          }
        }
      }
    }

    free(RawData);
    return 0;
}

char latch_loadsprites(int episode)
{
FILE *spritfile;
unsigned long plane1, plane2, plane3, plane4, plane5;
char fname[80];
int x,y,s,c,p;
unsigned long RawDataSize;
int i;
unsigned char ch;

    sprintf(fname, "data/EGASPRIT.CK%c", episode + '0');
 
    VidDrv_printf("latch_loadsprites(): Opening file '%s'.\n", fname);

    spritfile = fopen(fname, "rb");
    if (!spritfile)
    {
      VidDrv_printf("latch_loadsprites(): Unable to open '%s'!\n", fname);
      return 1;
    }

    RawDataSize = (LatchHeader.SpritePlaneSize * 5);
    RawData = malloc(RawDataSize);
    if (!RawData)
    {
      VidDrv_printf("latch_loadlatch(): Unable to allocate RawData buffer!\n");
      return 1;
    }

    if (LatchHeader.Compressed)
    {
      VidDrv_printf("latch_loadsprites(): Decompressing...\n");
      fseek(spritfile, 6, SEEK_SET);
      if (lz_decompress(spritfile, RawData)) return 1;
    }
    else
    {
      VidDrv_printf("latch_loadsprites(): Reading %d bytes...\n", RawDataSize);
      fread(RawData, RawDataSize, 1, spritfile);
    }
    fclose(spritfile);

    // these are the offsets of the different video planes as
    // relative to each other--that is if a pixel in plane1
    // is at N, the byte for that same pixel in plane3 will be
    // at (N + plane3).
    plane1 = 0;
    plane2 = (LatchHeader.SpritePlaneSize * 1);
    plane3 = (LatchHeader.SpritePlaneSize * 2);
    plane4 = (LatchHeader.SpritePlaneSize * 3);
    plane5 = (LatchHeader.SpritePlaneSize * 4);

    // ** read the sprites **
    VidDrv_printf("latch_loadsprites(): Decoding sprites...\n", fname);

    // set up the getbit() function
    setplanepositions(plane1 + LatchHeader.OffSprites, \
                      plane2 + LatchHeader.OffSprites, \
                      plane3 + LatchHeader.OffSprites, \
                      plane4 + LatchHeader.OffSprites, \
                      plane5 + LatchHeader.OffSprites);

    // load the image data
    for(p=0;p<4;p++)
    {
      for(s=0;s<LatchHeader.NumSprites;s++)
      {
        sprites[s].xsize = SpriteTable[s].Width;
        sprites[s].ysize = SpriteTable[s].Height;
        sprites[s].bboxX1 = (SpriteTable[s].Rx1 << CSF);
        sprites[s].bboxY1 = (SpriteTable[s].Ry1 << CSF);
        sprites[s].bboxX2 = (SpriteTable[s].Rx2 << CSF);
        sprites[s].bboxY2 = (SpriteTable[s].Ry2 << CSF);
        for(y=0;y<sprites[s].ysize;y++)
        {
          for(x=0;x<sprites[s].xsize;x++)
          {
            if (p==0)
            {
              c = 0;
            }
            else
            {
              c = sprites[s].imgdata[y][x];
            }
            c |= (getbit(RawData, p) << p);
            if (p==3 && c==0) c = 16;
            sprites[s].imgdata[y][x] = c;
          }
        }
      }
    }

    // now load the 5th plane, which contains the sprite masks.
    // note that we invert the mask because our graphics functions
    // use white on black masks whereas keen uses black on white.
    for(s=0;s<LatchHeader.NumSprites;s++)
    {
      for(y=0;y<sprites[s].ysize;y++)
      {
        for(x=0;x<sprites[s].xsize;x++)
        {           
           sprites[s].maskdata[y][x] = (1 - getbit(RawData, 4));
        }
      }
    }

    return 0;
}

char latch_loadgraphics(int episode)
{
int retval = 0;
   SpriteTable = NULL;

   if (latch_loadheader(episode)) { retval = 1; goto abort; }
   if (latch_loadlatch(episode)) { retval = 1; goto abort; }
   if (latch_loadsprites(episode)) { retval = 1; goto abort; }

abort: ;
   if (SpriteTable) free(SpriteTable);
   if (BitmapTable) free(BitmapTable);
   return retval;
}

