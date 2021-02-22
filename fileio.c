/* FILEIO.C
  Functions responsible for loading data from files, such as the one that
  decodes the level map files (loadmap()) and the one that loads in the
  tile attribute data contained in ep?attr.dat (loadtileattributes()).
  The functions for loading the graphics (EGALATCH&EGASPRIT) are in latch.c.
*/

#include "keen.h"

int curmapx, curmapy;
unsigned char mapdone;
void addmaptile(unsigned int t)
{
  map.mapdata[curmapx][curmapy] = t;
  curmapx++;
  if (curmapx >= map.xsize)
  {
    curmapx = 0;
    curmapy++;
    if (curmapy >= map.ysize) mapdone = 1;
  }
}

char NessieAlreadySpawned;
void addobjectlayertile(unsigned int t)
{
int o;
  switch(t)
  {
   case 0: break;       // blank
   case 255:            // player start
     player[0].x = curmapx << 4 << CSF;
     player[0].y = curmapy << 4 << CSF;
     map.objectlayer[curmapx][curmapy] = 0;
     break;
   case NESSIE_PATH:          // spawn nessie at first occurance of her path
     if (levelcontrol.episode==3)
     {
       if (!NessieAlreadySpawned)
       {
         o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_NESSIE);
         objects[o].hasbeenonscreen = 1;
         NessieAlreadySpawned = 1;
         NessieObjectHandle = o;
       }
     }
     goto levelmarker;
   break;
   default:             // level marker
levelmarker: ;
     if ((t&0x7fff) < 256 && levelcontrol.levels_completed[t&0x00ff])
     {
       map.objectlayer[curmapx][curmapy] = 0;
       map.mapdata[curmapx][curmapy] = tiles[map.mapdata[curmapx][curmapy]].chgtile;
     }
     else
     {
       map.objectlayer[curmapx][curmapy] = t;
     }
     break;
  }

  curmapx++;
  if (curmapx >= map.xsize)
  {
    curmapx = 0;
    curmapy++;
    if (curmapy >= map.ysize) mapdone = 1;
  }
}

void addenemytile(unsigned int t)
{
int o,x;
  map.objectlayer[curmapx][curmapy] = t;

  if (t)
  {
    if (t==255)
    {
       player[0].x = curmapx << 4 << CSF;
       player[0].y = ((curmapy << 4) + 8) << CSF;
    }
    else
    {
      switch(t)
      {
      case 0: break;
      case -1: break;
      case 1:  // yorp (ep1) vort (ep2&3)
           if (levelcontrol.episode==1)
           {
              x = curmapx;
              if (tiles[map.mapdata[x][curmapy+1]].solidl) x--;
              spawn_object(x<<4<<CSF, ((curmapy<<4)+8)<<CSF, OBJ_YORP);
           }
           else
           {
              // in ep2 level 16 there a vorticon embedded in the floor for
              // some reason! that's what the if() is for--to fix it.
              if (tiles[map.mapdata[curmapx][curmapy+1]].solidl)
              {
                spawn_object(curmapx<<4<<CSF, ((curmapy<<4)-16)<<CSF, OBJ_VORT);
              }
              else
              {
                spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_VORT);
              }
           }
           break;
      case 2:    // garg (ep1) baby vorticon (ep2&3)
           if (levelcontrol.episode==1)
           {
             // those bastards. sometimes embedding garg's in the floor in
             // the original maps.
             if (tiles[map.mapdata[curmapx+1][curmapy+1]].solidl)
             {
               if (levelcontrol.chglevelto==7)
               {
                 spawn_object(curmapx<<4<<CSF, (curmapy-1)<<4<<CSF, OBJ_GARG);
               }
               else
               {
                 spawn_object((curmapx-1)<<4<<CSF, (curmapy)<<4<<CSF, OBJ_GARG);
               }
             }
             else
             {
               spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_GARG);
             }
           }
           else
           {
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_BABY);
           }
           break;
      case 3:    // vorticon (ep1) bear (ep2)
           if (levelcontrol.episode==1)
           {
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_VORT);
           }
           else if (levelcontrol.episode==2)
           {
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_BEAR);
           }
           else if (levelcontrol.episode==3)
           {
              spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_MOTHER);
           }
           break;
      case 4:    // butler (ep1) OR walker (ep2) OR meep (ep3)
           if (levelcontrol.episode==1)
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_BUTLER);
           else if (levelcontrol.episode==2)
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_WALKER);
           else if (levelcontrol.episode==3)
             spawn_object(curmapx<<4<<CSF, ((curmapy<<4)+8)<<CSF, OBJ_MEEP);
           break;
      case 5:    // tank robot (ep1&2) karate bear (ep3)
           if (levelcontrol.episode==1)
           {
              o = spawn_object(curmapx<<4<<CSF, ((curmapy<<4)+8)<<CSF, OBJ_TANK);
              // set tank robot guarding bonus level to be active at startup
              if (levelcontrol.chglevelto==13)
              {
                objects[o].hasbeenonscreen = 1;
              }
           }
           else if (levelcontrol.episode==2)
              spawn_object(curmapx<<4<<CSF, ((curmapy<<4)+0)<<CSF, OBJ_TANKEP2);
           else if (levelcontrol.episode==3)
           {
              if (tiles[map.mapdata[curmapx][curmapy+1]].solidl)
              {
                spawn_object(curmapx<<4<<CSF, (curmapy-1)<<4<<CSF, OBJ_NINJA);
              }
              else
              {
                spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_NINJA);
              }
           }
           break;
      case 6:    // up-right-flying ice chunk (ep1) horiz platform (ep2)
                 // foob (ep3)
           if (levelcontrol.episode==1)
           {
              o = spawn_object((((curmapx+1)<<4)+4)<<CSF, ((curmapy<<4)-4)<<CSF, OBJ_SECTOREFFECTOR);
              objects[o].ai.se.type = SE_ICECANNON;
              objects[o].ai.se.dir = DUPRIGHT;
              objects[o].inhibitfall = 1;
              objects[o].hasbeenonscreen = 1;
           }
           else if (levelcontrol.episode==2)
           {
              o = spawn_object(curmapx<<4<<CSF, ((curmapy<<4)-3)<<CSF, OBJ_PLATFORM);
           }
           else if (levelcontrol.episode==3)
           {
              o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_FOOB);
           }
           break;
      case 7:   // spark (ep2) ball (ep3)
           if (levelcontrol.episode==2)
           {
              o = spawn_object(curmapx<<4<<CSF,curmapy<<4<<CSF,OBJ_SECTOREFFECTOR);
              objects[o].ai.se.type = SE_SPARK;
              levelcontrol.canexit = 0;    // can't exit till spark is shot
           }
           else
           {
              o = spawn_object(curmapx<<4<<CSF,curmapy<<4<<CSF,OBJ_BALL);
              objects[o].hasbeenonscreen = 1;
           }
           break;
      case 8:    // jack (ep3)
           if (levelcontrol.episode==3)
           {
              o = spawn_object(curmapx<<4<<CSF,curmapy<<4<<CSF,OBJ_JACK);
              objects[o].hasbeenonscreen = 1;
           }
           break;
      case 9:    // up-left-flying ice chunk (ep1) horiz platform (ep3)
           if (levelcontrol.episode==1)
           {
             o = spawn_object(((curmapx<<4)-4)<<CSF, ((curmapy<<4)-4)<<CSF, OBJ_SECTOREFFECTOR);
//             objects[o].ai.icechunk.movedir = DUPLEFT;
              objects[o].ai.se.type = SE_ICECANNON;
              objects[o].ai.se.dir = DUPLEFT;
             objects[o].inhibitfall = 1;
             objects[o].hasbeenonscreen = 1;
           }
           else if (levelcontrol.episode==3)
           {
             o = spawn_object(curmapx<<4<<CSF, ((curmapy<<4)+0)<<CSF, OBJ_PLATFORM);
           }
           break;
      case 10:   // rope holding the stone above the final vorticon (ep1)
                 // vert platform (ep3)
           if (levelcontrol.episode==1)
           {
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_ROPE);
           }
           else if (levelcontrol.episode==3)
           {
             spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_PLATVERT);
           }
           break;
      case 11:   // jumping vorticon (ep3)
           if (levelcontrol.episode==3)
           {
             spawn_object(curmapx<<4<<CSF, ((curmapy<<4)-8)<<CSF, OBJ_VORT);
           }
           break;
      case 12:   // sparks in mortimer's machine
          o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
          objects[o].ai.se.type = SE_MORTIMER_SPARK;
          objects[o].hasbeenonscreen = 1;           
          break;
      case 13:   // mortimer's heart
          o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
          objects[o].ai.se.type = SE_MORTIMER_HEART;
          objects[o].hasbeenonscreen = 1;           
          break;
      case 14:   // right-pointing raygun (ep3)
           if (levelcontrol.episode==3)
           {
             o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
             objects[o].ai.se.type = SE_GUN_RIGHT;
             objects[o].hasbeenonscreen = 1;
           }
           break;
      case 15:   // vertical raygun (ep3)
           if (levelcontrol.episode==3)
           {
             o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
             objects[o].ai.se.type = SE_GUN_VERT;
             objects[o].hasbeenonscreen = 1;
           }
           break;
      case 16:  // mortimer's arms
          o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
          objects[o].ai.se.type = SE_MORTIMER_ARM;
          objects[o].hasbeenonscreen = 1;           
      break;
      case 17:  // mortimer's left leg
          o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
          objects[o].ai.se.type = SE_MORTIMER_LEG_LEFT;
          objects[o].hasbeenonscreen = 1;
      break;
      case 18:  // mortimer's right leg
          o = spawn_object(curmapx<<4<<CSF, curmapy<<4<<CSF, OBJ_SECTOREFFECTOR);
          objects[o].ai.se.type = SE_MORTIMER_LEG_RIGHT;
          objects[o].hasbeenonscreen = 1;
      break;
      default:
/*           pal_fade(20);
           printf("unknown enemy type %d at (%d,%d)\n", t, curmapx, curmapy);
           while(!immediate_keytable[KENTER]);
           while(immediate_keytable[KENTER]);
           */
           break;
      }
//      printf("enemy type %d added at (%d,%d)\n", t, curmapx, curmapy);
    }
  }
  curmapx++;
  if (curmapx >= map.xsize)
  {
    curmapx = 0;
    curmapy++;
    if (curmapy >= map.ysize) mapdone = 1;
  }
}

unsigned int fgeti(FILE *fp) {
unsigned int temp1, temp2;
  temp1 = fgetc(fp);
  temp2 = fgetc(fp);
  return (temp2<<8) | temp1;
}

unsigned long fgetl(FILE *fp) {
unsigned int temp1, temp2, temp3, temp4;
  temp1 = fgetc(fp);
  temp2 = fgetc(fp);
  temp3 = fgetc(fp);
  temp4 = fgetc(fp);
  return (temp4<<24) | (temp3<<16) | (temp2<<8) | temp1;
}

unsigned int loadmap(char *filename, int lvlnum, int isworldmap)
{
FILE *fp;
char fname[256];
unsigned int junk;
int i, t, cnt;
int howmany;
int numruns = 0;
int gottenazero;
int resetcnt, resetpt;
char enemyresets_ep1[20] = {0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
char enemyresets_ep2[20] = {0, 0,0,0,0,0,4,0,0,0,0,0,4,0,2,0,0};
char enemyresets_ep3[20] = {0, 0,0,5,0,0,0,0,0,0,0,4,6,0,0,0,0};

  NessieAlreadySpawned = 0;
  map.isworldmap = isworldmap;

  sprintf(fname, "data/%s", filename);
  fp = fopen(fname, "rb");
  if (!fp)
  {
    // only record this error message on build platforms that log errors
    // to a file and not to the screen.
    #ifdef TARGET_WIN32
      VidDrv_printf("loadmap(): unable to open file %s\n", fname);
    #endif
    return 1;
  }

  junk = fgetc(fp);
  junk = fgetc(fp);
  junk = fgetc(fp);
  junk = fgetc(fp);

  map.xsize = fgeti(fp);
  map.ysize = fgeti(fp);

  fseek(fp, 0x14, SEEK_SET);  // go to map data
//  printf("loadmap(): map dimensions (%d, %d)\n", map.xsize, map.ysize);

  if (map.xsize > 250 || map.ysize > 250)
  {
//    printf("loadmap(): level is too big\n");
    return 1;
  }

  // ignore the first 8 map blocks
  cnt = 0;
  do
  {
    t = fgeti(fp);
    if (t == 0xfefe)
    {
      howmany = fgeti(fp);
      t = fgeti(fp);
      cnt += howmany;
    }
    else cnt++;
  } while(cnt < 8);

  // decompress map RLE data
  curmapx = curmapy = mapdone = 0;
  while(!mapdone)
  {
    t = fgeti(fp);
    if (t == 0xFEFE)
    {
      howmany = fgeti(fp);
      t = fgeti(fp);
      for(i=0;i<howmany;i++) addmaptile(t);
      numruns++;
    }
    else
    {
      addmaptile(t);
    }
  }
//  printf("loadmap(): decompressed %d RLE runs of map data\n", numruns);

  // now do the enemies
    gottenazero = 0;

  // decompress enemy/objectlayer RLE data
    curmapx = curmapy = mapdone = numruns = 0;
    resetcnt = resetpt = 0;
    if (levelcontrol.episode==1)
    {
      if (lvlnum < sizeof(enemyresets_ep1)) resetpt = enemyresets_ep1[lvlnum];
    }
    else if (levelcontrol.episode==2)
    {
      if (lvlnum < sizeof(enemyresets_ep2)) resetpt = enemyresets_ep2[lvlnum];
    }
    else if (levelcontrol.episode==3)
    {
      if (lvlnum < sizeof(enemyresets_ep3)) resetpt = enemyresets_ep3[lvlnum];
    }

    while(!mapdone)
    {
      t = fgeti(fp);
      if (t == 0xFEFE)
      {
        howmany = fgeti(fp);
        t = fgeti(fp);
        if (t==0 && !gottenazero)
        {
          curmapx = curmapy = 0;
          gottenazero = 1;
        }
        for(i=0;i<howmany;i++)
        {
          if (map.isworldmap) addobjectlayertile(t); else addenemytile(t);
          if (++resetcnt==resetpt) curmapx=curmapy=0;
        }
        numruns++;
      }
      else
      {
        if (t==0 && !gottenazero)
        {
          curmapx = curmapy = 0;
          gottenazero = 1;
        }
        if (map.isworldmap) addobjectlayertile(t); else addenemytile(t);
        if (++resetcnt==resetpt) curmapx=curmapy=0;
      }
    }

//  printf("\nloadmap(): decompressed %d RLE runs of enemy data\n", numruns);

 // install enemy stoppoints as needed
 if (levelcontrol.episode==1 && lvlnum==13)
 {
    map.objectlayer[94][13] = GARG_STOPPOINT;
    map.objectlayer[113][13] = GARG_STOPPOINT;
    map.objectlayer[48][6] = GARG_STOPPOINT;
    map.objectlayer[80][5] = GARG_STOPPOINT;
    map.objectlayer[87][5] = GARG_STOPPOINT;
    map.objectlayer[39][18] = GARG_STOPPOINT;
 }
 else if (levelcontrol.episode==3 && lvlnum==6)
 {
    map.objectlayer[40][7] = BALL_NOPASSPOINT;
    map.objectlayer[50][7] = BALL_NOPASSPOINT;
 }
 else if (levelcontrol.episode==3 && lvlnum==9)
 {
    map.objectlayer[45][106] = BALL_NOPASSPOINT;

 }
 else if (levelcontrol.episode==3 && lvlnum==4)
 {
    map.objectlayer[94][17] = BALL_NOPASSPOINT;
 }

  fclose(fp);
  return 0;
}

char loadtileattributes(int episode)
{
FILE *fp;
int t,a,b,c,intendedep,intendedver;
char fname[80];

  sprintf(fname, "ep%dattr.dat", episode);

//  printf("loading tile attributes from '%s'...\n", fname);

  fp = fopen(fname, "rb");
  if (!fp)
  {
    crashflag = 1;
    crashflag2 = episode;
    why_term_ptr = "loadtileattributes(): Cannot open tile attribute file! Episode in flag2.";
    return 1;
  }

  /* check the header */
  // header format: 'A', 'T', 'R', episode, version
  a = fgetc(fp);
  b = fgetc(fp);
  c = fgetc(fp);
  if (a != 'A' || b != 'T' || c != 'R')
  {
//     printf("loadtileattributes(): %s corrupt!\n(\"ATR\" marker not found, instead it was %02x %02x %02x)\n", fname, a, b, c);
     crashflag = 1;
     why_term_ptr = "loadtileattributes(): Attribute file corrupt! ('ATR' marker not found)";
     return 1;
  }

  intendedep = fgetc(fp);
  if (intendedep != episode)
  {
//     printf("loadtileattributes(): file is intended for episode %d, but you're trying to use it with episode %d!\n", intendedep, episode);
     crashflag = 1;
     why_term_ptr = "loadtileattributes(): Attribute file is intended for a different episode!";
     return 1;
  }

  intendedver = fgetc(fp);
  if (intendedver < ATTRFILEVERSION)
  {
//     printf("loadtileattributes(): %s is too old for this version of CloneKeen!\n", fname);
//     printf("(file version %d, I need version %d)\n", intendedver, ATTRFILEVERSION);
     crashflag = 1;
     why_term_ptr = "loadtileattributes(): Attribute file is wrong version (too old).";
     return 1;
  }
  else if (intendedver > ATTRFILEVERSION)
  {
//     printf("loadtileattributes(): %s is too new for this version of CloneKeen!\n", fname);
//     printf("(file version %d, I need version %d)\n", intendedver, ATTRFILEVERSION);
     crashflag = 1;
     why_term_ptr = "loadtileattributes(): Attribute file is wrong version (too new).";
     return 1;
  }

  /* load in the tile attributes */

  for(t=0;t<MAX_TILES-1;t++) {
    tiles[t].solidl = fgetc(fp);
    if (tiles[t].solidl==-1)
    {
//      printf("loadtileattributes(): %s corrupt! (unexpected EOF)\n", fname);
      crashflag = 1;
      why_term_ptr = "loadtileattributes(): Attribute file corrupt (unexpected EOF)";
      return 1;
    }

    tiles[t].solidr = fgetc(fp);
    tiles[t].solidfall = fgetc(fp);
    tiles[t].solidceil = fgetc(fp);
    tiles[t].ice = fgetc(fp);
    tiles[t].semiice = fgetc(fp);
    tiles[t].priority = fgetc(fp);
    if (fgetc(fp)) tiles[t].masktile=t+1; else tiles[t].masktile = 0;
    tiles[t].goodie = fgetc(fp);
    tiles[t].standgoodie = fgetc(fp);
    tiles[t].pickupable = fgetc(fp);
    tiles[t].lethal = fgetc(fp);
    tiles[t].bonklethal = fgetc(fp);
    a = fgetc(fp); b = fgetc(fp);
    tiles[t].chgtile = (a*256)+b;
    tiles[t].isAnimated = fgetc(fp);
    tiles[t].animOffset = fgetc(fp);
    tiles[t].animmask = fgetc(fp);
  }
  fclose(fp);

  return 0;
}

char loadstrings_AddAttr(char *attr, int stringIndex)
{
char stAttrName[80];
char stAttrValue[80];
int attrvalue;
int RAMAllocSize;
char *copyPtr;
int i;

  // if the attribute does not have an equals sign bail
  if (!strstr(attr, "="))
  {
    VidDrv_printf("loadstrings_AddAttr(): '%s' is not a valid attribute definition.\n", attr);
    return 1;
  }

  // split the attribute up into it's name and it's value
  copyPtr = stAttrName;
  for(i=0;i<strlen(attr);i++)
  {
    if (attr[i] != ' ' && attr[i]!=9)      // strip out spaces and tabs
    {
      if (attr[i] != '=')
      {
        *copyPtr = attr[i];
        copyPtr++;
      }
      else
      { // hit the equals sign
        *copyPtr = 0;
        copyPtr = stAttrValue;
      }
    }
  }
  *copyPtr = 0;

  attrvalue = atoi(stAttrValue);

  // malloc space for the attribute name
  RAMAllocSize = strlen(stAttrName) + 1;
  strings[stringIndex].attrnames[strings[stringIndex].numAttributes] = malloc(RAMAllocSize+1);
  if (!strings[stringIndex].attrnames[strings[stringIndex].numAttributes])
  {
    printf("loadstrings_AddAttr(): Unable to allocate space for attribute name ('%s').\n", stAttrName);
    return 1;
  }

  // copy the data into the strings structure
  memcpy(strings[stringIndex].attrnames[strings[stringIndex].numAttributes], stAttrName, RAMAllocSize);
  strings[stringIndex].attrvalues[strings[stringIndex].numAttributes] = attrvalue;

  strings[stringIndex].numAttributes++;
  return 0;
}

// load strings from file *fname ("strings.dat")
char loadstrings(char *fname)
{
FILE *fp;
char state;
unsigned char stName[80];
unsigned char stString[1024];
unsigned char stAttr[80];
int i,c;
int nameIndex, stringIndex, attrIndex;
int waitChar, gotoState;
char highlight;
int RAMSize;
char *RAMPtr;

  #define STSTATE_WAITCHAR      0
  #define STSTATE_READNAME      1
  #define STSTATE_READSTRING    2
  #define STSTATE_READATTR      3

  VidDrv_printf("loadstrings(): Opening string file '%s'.\n", fname);
  fp = fopen(fname, "rb");
  if (!fp)
  {
    VidDrv_printf("loadstrings(): String file unable to open.\n");
    return 1;
  }

  // go through all the strings and NULL out the entries...this will
  // let us know which ones are in use (and need to be free()d at shutdown)
  for(i=0;i<MAX_STRINGS;i++)
  {
    strings[i].name = NULL;
    strings[i].stringptr = NULL;
    strings[i].numAttributes = 0;
  }

  nameIndex = 0;
  stringIndex = 0;
  numStrings = 0;
  highlight = 0;

  // read until we get to the first string name
  state = STSTATE_WAITCHAR;
  waitChar = '[';
  gotoState = STSTATE_READNAME;
  do
  {
    c = fgetc(fp);              // read byte from file

    if (c<0)
    {   // EOF
      break;
    }
    // ignore LF's
    if (c==10) continue;

    switch(state)
    {
     case STSTATE_WAITCHAR:
      // ignore chars until we read a waitChar, then go to state gotoState
      if (c==waitChar)
      {
        state = gotoState;
      }
      break;
     case STSTATE_READATTR:
      if (c==13)
      { // reached CR, start reading string
        if (attrIndex)
        {
          stAttr[attrIndex] = 0;
          if (loadstrings_AddAttr(stAttr, numStrings)) return 1;
        }
        state = STSTATE_READSTRING;
      }
      else if (c==' ')
      { // end of an attribute definition
        if (attrIndex)
        {
          stAttr[attrIndex] = 0;  // null-terminate
          if (loadstrings_AddAttr(stAttr, numStrings)) return 1;
        }
        attrIndex = 0;
      }
      else
      { // save char to attribute buffer
        stAttr[attrIndex] = c;
        attrIndex++;
      }
      break;
     case STSTATE_READNAME:
      // read in the string name until we get to ']'
      if (c != ']')
      {
        stName[nameIndex] = c;
        nameIndex++;
      }
      else
      {
        stName[nameIndex] = 0;  //null-terminate
	highlight = 0;
        // read any attributes until the CR
        state = STSTATE_READATTR;
        attrIndex = 0;
      }
      break;
     case STSTATE_READSTRING:
      // read in string data until we see another '['
      if (c != '[')
      {
        // allow delimiters:
        // you can put [ and ] in the string by using \( and \).
        // set a highlight (change font color to the +128 font) with \H
        // stop highlighting with \h
        if (stringIndex>0 && stString[stringIndex-1]=='\\'+(highlight*128))
        {  // delimiter detected
          if (c=='(')
          {
            stString[stringIndex - 1] = '[' + (highlight*128);
          }
          else if (c==')')
          {
            stString[stringIndex - 1] = ']' + (highlight*128);
          }
          else if (c=='H')
          {
            highlight = 1;
            stringIndex--;
          }
          else if (c=='h')
          {
            highlight = 0;
            stringIndex--;
          }
          else if (c=='\\')
          {
            stString[stringIndex - 1] = '\\' + (highlight*128);
          }
        }
        else
        { // normal non-delimited char
          stString[stringIndex] = c;
          if (highlight && c!=0 && c!=13)
          {
            stString[stringIndex] += 128;
          }
          stringIndex++;
        }
      }
      else
      {
        stString[stringIndex-1] = 0;  //null-terminate (cutting off final CR)

        /* save the string to the strings[] structure */

        // we're going to malloc() an area and copy the name, then the string,
        // into it. We'll need room for both the name and the string, plus
        // null-terminators for each.
        RAMSize = strlen(stName) + strlen(stString) + 2;
	RAMPtr = malloc(RAMSize);
        if (!RAMPtr)
        {
          printf("loadstrings(): Could not allocate memory for string '%s'\n", stName);
          return 1;
        }

        // assign our pointers
        strings[numStrings].name = &RAMPtr[0];
        strings[numStrings].stringptr = &RAMPtr[strlen(stName)+1];

        // copy the string info to the newly malloc()'d memory area
        memcpy(strings[numStrings].name, stName, strlen(stName)+1);
        memcpy(strings[numStrings].stringptr, stString, strlen(stString)+1);

        numStrings++;
        // read the name of the next string
        state = STSTATE_READNAME;
        nameIndex = 0;
        stringIndex = 0;
      }
      break;
    }

  } while(1);

  VidDrv_printf("loadstrings(): loaded %d strings from '%s'.\n", numStrings, fname);
  fclose(fp);
  return 0;
}

int freestrings(void)
{
int i,j;
int NumStringsFreed;

  NumStringsFreed = 0;
  for(i=0;i<MAX_STRINGS;i++)
  {
    if (strings[i].name)
    {
      // free the string name
      free(strings[i].name);
      strings[i].name = strings[i].stringptr = NULL;
      // free all attribute names
      for(j=0;j<strings[i].numAttributes;j++)
      {
        free(strings[i].attrnames[j]);
      }
      strings[i].numAttributes = 0;

      NumStringsFreed++;
    }
  }

  return NumStringsFreed;
}

char *MissingString = "MISSING STRING!";

// returns a pointer to the string with name 'name'
char* getstring(char *name)
{
int i;
  for(i=0;i<numStrings;i++)
  {
    if (!strcmp(name, strings[i].name))
    {
      return strings[i].stringptr;
    }
  }

  return MissingString;
}

// returns attribute attrname of string stringname, or -1 if it doesn't exist.
int GetStringAttribute(char *stringName, char *attrName)
{
int i,j;
  for(i=0;i<numStrings;i++)
  {
    if (!strcmp(stringName, strings[i].name))
    {
      // we found the string, now find the requested attribute
      for(j=0;j<strings[i].numAttributes;j++)
      {
        if (!strcmp(attrName, strings[i].attrnames[j]))
        {
          return strings[i].attrvalues[j];
        }
      }
      // failed to find attribute
      return -1;
    }
  }
  // failed to find string
  return -1;
}

