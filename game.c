	/* GAME.C
  Main and miscellaneous functions for in-game, contains the main
  game loop, etc.
*/

#include "keen.h"
#include "demobox.h"

char otherplayer;
// TODO:

// seperate status boxes for the different players
// sound keen makes when he's walking into a wall
// Your Ship Needs These Parts in multiplayer

int playerbaseframes[MAX_PLAYERS] = {0,0,0,0,0,0,0,0};

int max_scroll_x, max_scroll_y;
char debugmode=0,acceleratemode=0;

// and this is where the magic happens
void gameloop(void)
{
int i,o,x,y,c;
int enter,lastenterstate,lastquit;
unsigned long oldtimer;
unsigned int temp;
char spacedownlasttime=0;
char mpskiptimer=0,mpskip=200;

  if (player[0].x==0 || player[0].y==0)
  {
    crashflag=1;
    crashflag2 = levelcontrol.curlevel;
    crashflag3 = levelcontrol.episode;
    why_term_ptr = "No player start position! (flag2=levelcontrol.curlevel, flag3=levelcontrol.episode)";
  }

  if (!loadinggame)
  {
     gameloop_initialize();
  }
  else
  {
     loadinggame = 0;
     fade.mode = FADE_GO;
     fade.dir = FADE_IN;
     fade.curamt = 0;
     fade.fadetimer = 0;
     fade.rate = FADE_NORM;
  }

  // fire all guns immediately first time around
  gunfiretimer = (gunfirefreq+1);

  // if this is Mortimer's Castle, fade in and do the conversation
  // with Mortimer.
  if (levelcontrol.episode==3 && levelcontrol.curlevel==16)
  {
     for(i=0;i<MAX_PLAYERS;i++)
     {
       if (player[i].isPlaying) gamepdo_SelectFrame(i);
     }

     do
     {
        gamedo_fades();
        gamedo_AnimatedTiles();
        SpeedThrottle();
        gamedo_RenderScreen();
     } while(fade.mode!=FADE_COMPLETE && !immediate_keytable[KQUIT]);

     eseq3_Mortimer();
  }

  lastquit = 1;
  do
  {
     if (primaryplayer==1) otherplayer = 0; else otherplayer = 1;

     #ifdef NETWORK_PLAY
//       if (numplayers>1) net_getdata();
         if (is_server)
         {
           Net_Server_Run();
         }
         else if (is_client)
         {
           Net_Client_Run();
         }
     #endif

     gamedo_fades();

     // periodically make all enemy gun fixtures fire (in ep3)
     // (also ice cannons in ep1) we do this in a global variable
     // so they're all in sync. when gunfiretimer==0 all gun SE
     // objects will fire.
     if (gunfiretimer > gunfirefreq)
     {
       gunfiretimer = 0;
     }
     else gunfiretimer++;

     // gather input and copy to player[].keytable[] structures
     gamedo_getInput();

     // run the player "AI" for each player in the game
     if (!map.isworldmap)
     {
       for(i=0;i<MAX_PLAYERS;i++)
       {
         if (player[i].isPlaying) gamepdo_HandlePlayer(i);
       }
     }
     else
     {
       for(i=0;i<MAX_PLAYERS;i++)
       {
         if (player[i].isPlaying) gamepdo_wm_HandlePlayer(i);
       }
     }

     gamedo_AnimatedTiles();
     gamedo_enemyai();

     gamedo_HandleFKeys();

     /* scroll triggers */
     if (!levelcontrol.gameovermode && levelcontrol.level_done==LEVEL_NOT_DONE)
     {
        ScreenIsScrolling = 0;
        if (gamedo_ScrollTriggers(primaryplayer)) ScreenIsScrolling = 1;
     }

     // do frameskipping, and render/blit the screen if it's time
     gamedo_frameskipping();

     // when we complete a fade out flag to exit the game loop
     if (fade.mode==FADE_COMPLETE)
     {
         if (fade.dir==FADE_OUT)
         {
            demomode = DEMO_NODEMO;
            levelcontrol.level_done = LEVEL_COMPLETE;
            levelcontrol.command = LVLC_CHANGE_LEVEL;
            if (levelcontrol.curlevel != WM_MAP_NUM)
            { // exiting a level, going back to world map
              if (levelcontrol.success==1)
              { // mark level as completed on world map                
                levelcontrol.levels_completed[levelcontrol.curlevel] = 1;
              }
              levelcontrol.chglevelto = WM_MAP_NUM;
            }
         }
         else
         {
           fade.mode = NO_FADE;
         }
     }

     // when walking through the exit door don't show keen's sprite past
     // the door frame (so it looks like he walks "through" the door)
     if (levelcontrol.level_done==LEVEL_DONE_WALK)
     {
        gamepdo_walkbehindexitdoor(levelcontrol.level_finished_by);
     }

     // allow enter to return to main menu
     // if we're in game over mode
     enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]);
     if (levelcontrol.gameovermode)
     {
       if (enter&&!lastenterstate)
       {
         if (fade.mode!=FADE_GO && fade.dir!=FADE_OUT)
         {
           fade.dir = FADE_OUT;
           fade.curamt = PAL_FADE_SHADES;
           fade.fadetimer = 0;
           fade.rate = FADE_NORM;
           fade.mode = FADE_GO;
         }
       }

       if (fade.mode==FADE_COMPLETE && fade.dir==FADE_OUT)
       {
         levelcontrol.command = LVLC_GAME_OVER;
       }
     }
     lastenterstate = enter;

     #ifdef NETWORK_PLAY
//       if (numplayers>1) net_senddata();
     #endif

     if (immediate_keytable[KQUIT] && !last_immediate_keytable[KQUIT])
     {
       VerifyQuit();
     }
     if (QuitState != NO_QUIT) return;

     memcpy(last_immediate_keytable, immediate_keytable, KEYTABLE_SIZE);

     // limit frame rate
     if (!acceleratemode)
     {
        SpeedThrottle();
        #ifdef NETWORK_PLAY
//          if (numplayers>1) net_sync();
        #endif
     }
     else poll_events();

  } while(!crashflag && levelcontrol.command==LVLC_NOCOMMAND);

}

// gives keycard for door doortile to player p
void give_keycard(int doortile, int p)
{
int i;
   sound_play(SOUND_GET_CARD, PLAY_NOW);
   if (doortile==DOOR_YELLOW) player[p].inventory.HasCardYellow = 1;
   else if (doortile==DOOR_RED) player[p].inventory.HasCardRed = 1;
   else if (doortile==DOOR_GREEN) player[p].inventory.HasCardGreen = 1;
   else if (doortile==DOOR_BLUE) player[p].inventory.HasCardBlue = 1;
   else if (doortile==DOOR_YELLOW_EP3) player[p].inventory.HasCardYellow = 1;
   else if (doortile==DOOR_RED_EP3) player[p].inventory.HasCardRed = 1;
   else if (doortile==DOOR_GREEN_EP3) player[p].inventory.HasCardGreen = 1;
   else if (doortile==DOOR_BLUE_EP3) player[p].inventory.HasCardBlue = 1;
   else
   {
     crashflag = 1;
     crashflag2 = doortile;
     why_term_ptr = "give_keycard(): invalid value for doortile parameter.";
   }
}

// take away the specified keycard from player p
void take_keycard(int doortile, int p)
{
int i;
   if (doortile==DOOR_YELLOW) player[p].inventory.HasCardYellow = 0;
   else if (doortile==DOOR_RED) player[p].inventory.HasCardRed = 0;
   else if (doortile==DOOR_GREEN) player[p].inventory.HasCardGreen = 0;
   else if (doortile==DOOR_BLUE) player[p].inventory.HasCardBlue = 0;
   else if (doortile==DOOR_YELLOW_EP3) player[p].inventory.HasCardYellow = 0;
   else if (doortile==DOOR_RED_EP3) player[p].inventory.HasCardRed = 0;
   else if (doortile==DOOR_GREEN_EP3) player[p].inventory.HasCardGreen = 0;
   else if (doortile==DOOR_BLUE_EP3) player[p].inventory.HasCardBlue = 0;
}

// unregisters all animated tiles with baseframe tile
void unregister_animtiles(int tile)
{
int i;
     for(i=0;i<MAX_ANIMTILES-1;i++)
     {
        if (animtiles[i].baseframe == tile)
        {
           animtiles[i].slotinuse = 0;
        }
     }
}

void open_door(int doortile, int doorsprite, int mpx, int mpy, int cp)
{
int o,chgtotile;

   sound_play(SOUND_DOOR_OPEN, PLAY_NOW);
   take_keycard(doortile, cp);

   /* erase door from map */
   if (levelcontrol.episode==3)
   {
     chgtotile = map.mapdata[mpx-1][mpy];
   }
   else
   {
     chgtotile = tiles[map.mapdata[mpx][mpy]].chgtile;
   }
   map_chgtile(mpx, mpy, chgtotile);
   map_chgtile(mpx, mpy+1, chgtotile);

   // replace the door tiles with a door object, which will do the animation
   o = spawn_object(mpx<<4<<CSF,mpy<<4<<CSF,OBJ_DOOR);
   objects[o].sprite = doorsprite;      
}

// checks if score is > than "extra life at" and award 1-UPs when appropriate
void extralifeat(int cp)
{
  if (player[cp].inventory.score > player[cp].inventory.extralifeat)
  {
    sound_play(SOUND_EXTRA_LIFE, PLAY_NOW);
    player[cp].inventory.lives++;
    player[cp].inventory.extralifeat += 20000;
  }
}

// have keen pick up the goodie at screen pixel position (px, py)
void keen_get_goodie(int px, int py, int theplayer)
{
int mpx,mpy,t;
int i;
   mpx = px>>4;
   mpy = py>>4;
   t = map.mapdata[mpx][mpy];

   if (tiles[t].pickupable)
   {  // pick up the goodie, i.e. erase it from the map
      map_chgtile(mpx, mpy, tiles[t].chgtile);
      if (tiles[t].isAnimated) map_deanimate(mpx, mpy);
   }
   else if (tiles[t].lethal)
   {  // whoah, this "goodie" isn't so good...
      killplayer(theplayer);
      return;
   }

   // do whatever the goodie is supposed to do...
   if (levelcontrol.episode==1)
   {
     procgoodie_ep1(t, mpx, mpy, theplayer);
   }
   else if (levelcontrol.episode==2)
   {
     procgoodie_ep2(t, mpx, mpy, theplayer);
   }
   else if (levelcontrol.episode==3)
   {
     procgoodie_ep3(t, mpx, mpy, theplayer);
   }
}

void initgame(void)
{
int x,y,i;

  animtiletimer = curanimtileframe = 0;
  PlatExtending = 0;

  // reset player walk frame widths
  for(i=0;i<numplayers;i++)
  {
    sprites[playerbaseframes[i]+0].xsize = PLAYERSPRITE_WIDTH;
    sprites[playerbaseframes[i]+1].xsize = PLAYERSPRITE_WIDTH;
    sprites[playerbaseframes[i]+2].xsize = PLAYERSPRITE_WIDTH;
    sprites[playerbaseframes[i]+3].xsize = PLAYERSPRITE_WIDTH;
  }

  // set gun/ice cannon fire freq
  if (levelcontrol.episode==1)
  {
    gunfirefreq = ICECANNON_FIRE_FREQ;
  }
  else
  {
    gunfirefreq = GUN_FIRE_FREQ;
  }

  // reset the ysize attribute of all doors
  sprites[DOOR_YELLOW_SPRITE].ysize = 32;
  sprites[DOOR_RED_SPRITE].ysize = 32;
  sprites[DOOR_GREEN_SPRITE].ysize = 32;
  sprites[DOOR_BLUE_SPRITE].ysize = 32;

  levelcontrol.level_done_timer = 0;
  levelcontrol.gameovermode = 0;

  // re-goodie Yorp Statue (ep1), & Vorticon Elder switch (ep2)
  if (levelcontrol.episode==1)
  {
    tiles[YORPSTATUEHEAD].goodie = 1;
    tiles[YORPSTATUEBOTTOM].goodie = 1;
    // garg statue
    tiles[433].goodie = tiles[436].goodie = 1;
    tiles[446].goodie = tiles[447].goodie = 1;
  }
  else if (levelcontrol.episode==2)
  {
    tiles[429].standgoodie = 1;
    tiles[432].isAnimated = 1;
  }

  // all objects -> not exist
 for(i=1;i<MAX_OBJECTS-1;i++) objects[i].exists = 0;
 // clear out AnimTileInUse array
 for(y=0;y<ATILEINUSE_SIZEX-1;y++)
 {
   for(x=0;x<ATILEINUSE_SIZEY-1;x++)
   {
      AnimTileInUse[x][y] = 0;
   }
 }
 // set all animated tile slots to "not in use"
 map_unregister_all_animtiles();

// initilize default sprites for objects
  memset(objdefsprites, 0, sizeof(objdefsprites));
  if (levelcontrol.episode==1)
  {
    objdefsprites[OBJ_YORP] = OBJ_YORP_DEFSPRITE;
    objdefsprites[OBJ_GARG] = OBJ_GARG_DEFSPRITE;
    objdefsprites[OBJ_BUTLER] = OBJ_BUTLER_DEFSPRITE;
    objdefsprites[OBJ_TANK] = OBJ_TANK_DEFSPRITE;
    objdefsprites[OBJ_ICECHUNK] = OBJ_ICECHUNK_DEFSPRITE;
    objdefsprites[OBJ_ICEBIT] = OBJ_ICEBIT_DEFSPRITE;
    objdefsprites[OBJ_ROPE] = OBJ_ROPE_DEFSPRITE;

    objdefsprites[OBJ_RAY] = OBJ_RAY_DEFSPRITE_EP1;
    objdefsprites[OBJ_VORT] = OBJ_VORT_DEFSPRITE_EP1;
  }
  else if (levelcontrol.episode==2)
  {
    objdefsprites[OBJ_WALKER] = OBJ_WALKER_DEFSPRITE;
    objdefsprites[OBJ_TANKEP2] = OBJ_TANKEP2_DEFSPRITE;
    objdefsprites[OBJ_BEAR] = OBJ_BEAR_DEFSPRITE;

    objdefsprites[OBJ_RAY] = OBJ_RAY_DEFSPRITE_EP2;
    objdefsprites[OBJ_VORT] = OBJ_VORT_DEFSPRITE_EP2;
    objdefsprites[OBJ_PLATFORM] = OBJ_PLATFORM_DEFSPRITE_EP2;
    objdefsprites[OBJ_BABY] = OBJ_BABY_DEFSPRITE_EP2;
  }
  else if (levelcontrol.episode==3)
  {
    objdefsprites[OBJ_FOOB] = OBJ_FOOB_DEFSPRITE;
    objdefsprites[OBJ_NINJA] = OBJ_NINJA_DEFSPRITE;
    objdefsprites[OBJ_MOTHER] = OBJ_MOTHER_DEFSPRITE;
    objdefsprites[OBJ_MEEP] = OBJ_MEEP_DEFSPRITE;
    objdefsprites[OBJ_BALL] = OBJ_BJ_DEFSPRITE;
    objdefsprites[OBJ_JACK] = OBJ_BJ_DEFSPRITE;

    objdefsprites[OBJ_RAY] = OBJ_RAY_DEFSPRITE_EP3;
    objdefsprites[OBJ_VORT] = OBJ_VORT_DEFSPRITE_EP3;
    objdefsprites[OBJ_PLATFORM] = OBJ_PLATFORM_DEFSPRITE_EP3;
    objdefsprites[OBJ_BABY] = OBJ_BABY_DEFSPRITE_EP3;
  }

  objdefsprites[OBJ_DOOR] = DOOR_YELLOW_SPRITE;
  objdefsprites[OBJ_TELEPORTER] = OBJ_TELEPORTER_DEFSPRITE;

  objdefsprites[OBJ_SECTOREFFECTOR] = BlankSprite;

// initilize game variables
  levelcontrol.level_done = LEVEL_NOT_DONE;
  animtiletimer = curanimtileframe = 0;
  DemoObjectHandle = 0;

  for(i=0;i<MAX_PLAYERS;i++)
  {
    player[i].isPlaying = 0;
  }
  for(i=0;i<numplayers;i++)
  {
    player[i].isPlaying = 1;
    player[i].useObject = (i+1); // player objects start at 1 cause DemoObject is index 0
    player[i].dpadcount = player[i].dpadlastcount = 0;
    player[i].hideplayer = 0;
    player[i].mounted = 0;
    player[i].ppogostick = 0;
    player[i].pjumping = 0;
    player[i].pfalling = 0;
    player[i].pwalking = player[i].playspeed = 0;
    player[i].pinertia_x = player[i].pinertia_y = 0;
    player[i].playpushed_x = 0;
    player[i].pfiring = 0;
    player[i].psliding = player[i].psemisliding = 0;
    player[i].pdie = 0;
    player[i].pfrozentime = 0;
    player[i].dpadcount = player[i].dpadlastcount = 0;
    player[i].ankhtime = 0;
    player[i].keyprocstate = 0;         // KPROC_IDLE
    player[i].pjustjumped = player[i].pjustfell = 0;
  }
  // each player is tied to a "puppet" object.
  // initilize objects used by players.
  for(i=0;i<numplayers;i++)
  {
    if (player[i].isPlaying)
    {
       objects[player[i].useObject].exists = 1;
       objects[player[i].useObject].onscreen = 1;
       objects[player[i].useObject].type = OBJ_PLAYER;
       objects[player[i].useObject].sprite = 0;
       objects[player[i].useObject].onscreen = 1;
       objects[player[i].useObject].AssociatedWithPlayer = i;
       objects[player[i].useObject].honorPriority = 1;
    }
  }

  memset(last_immediate_keytable, 1, KEYTABLE_SIZE);
  

  scroll_x = 0;
  scrollx_buf = 0;
  scrollpix = 0;
  mapx = 0;
  mapxstripepos = 0;

  scroll_y = 0;
  scrolly_buf = 0;
  scrollpixy = 0;
  mapy = 0;
  mapystripepos = 0;

  thisplayer = 0;
}

void initgamefirsttime(void)
{
int i;

   if (loadtileattributes(levelcontrol.episode)) return;

   map.firsttime = 1;

   memset(player, 0, sizeof(player));

   for(i=0;i<MAX_LEVELS;i++)
     levelcontrol.levels_completed[i] = 0;

   for(i=0;i<MAX_PLAYERS;i++)
   {
       player[i].inventory.extralifeat = 20000;
       player[i].inventory.lives = 4;
       player[i].godmode = 0;

       if (levelcontrol.episode==1)
       {
         player[i].inventory.charges = 0;
       }
       else if (levelcontrol.episode==2)
       {
         player[i].inventory.charges = 3;
       }
       else
       {
         player[i].inventory.charges = 5;
       }
       if (demomode) player[i].inventory.charges = 100;

       // start with pogo stick in all episodes but 1
       if (levelcontrol.episode!=1 || demomode)
         { player[i].inventory.HasPogo = 1; }
       else
         { player[i].inventory.HasPogo = 0; }
   }

   initsprites();

   if (demomode) srand(375);

   primaryplayer = 0;
}

char spawn_object(int x, int y, int otype)
{
int i;
 // find an unused object slot
 for(i=1;i<MAX_OBJECTS-1;i++)
 {
   if (!objects[i].exists && objects[i].type != OBJ_PLAYER)
   {
     objects[i].x = x;
     objects[i].y = y;
     objects[i].type = otype;
     objects[i].sprite = objdefsprites[otype];
     objects[i].exists = 1;
     objects[i].needinit = 1;
     objects[i].onscreen = 0;
     objects[i].hasbeenonscreen = 0;
     objects[i].canbezapped = 0;
     objects[i].zapped = 0;
     objects[i].inhibitfall = 0;
     objects[i].honorPriority = 1;
     SetAllCanSupportPlayer(i, 0);
     return i;
   }
 }
 // object could not be created
 crashflag = 1;
 crashflag2 = otype;
 crashflag3 = 0;
 why_term_ptr = "Object could not be created.";
 return 0;
}

// common enemy/object ai, such as falling, setting blocked variables,
// detecting player contact, etc.
void common_enemy_ai(int o)
{
int x,y,xa,ya,xsize,ysize;
int temp;
int cplayer;

       if (objects[o].type==OBJ_DEMOMSG) return;

       xsize = sprites[objects[o].sprite].xsize;
       ysize = sprites[objects[o].sprite].ysize;

 // set value of blockedd--should object fall?
       temp = (objects[o].y>>CSF)+ysize;
       if ((temp>>4)<<4 != temp)
       {
          objects[o].blockedd = 0;
       }
       else
       { // on a tile boundary, test if tile under object is solid
          objects[o].blockedd = 0;
          x = (objects[o].x>>CSF);
          y = (objects[o].y>>CSF)+ysize+1;
          for(xa=0;xa<xsize-2;xa+=16)
          {
            if (tiles[getmaptileat(x+xa,y)].solidfall)
            {
              objects[o].blockedd = 1;
              goto setblockedd;
            }
          }
          if (tiles[getmaptileat(x+xsize-2, y)].solidfall)
          {
             objects[o].blockedd = 1;
          }
          setblockedd: ;
       }

  // set blockedu
    objects[o].blockedu = 0;
    x = (objects[o].x>>CSF);
    y = (objects[o].y>>CSF)-1;
    for(xa=0;xa<xsize;xa+=16)
    {
        if (tiles[getmaptileat(x+xa,y)].solidceil)
        {
          objects[o].blockedu = 1;
          goto setblockedu;
        }
    }
    if (tiles[getmaptileat(x+xsize-1, y)].solidceil)
    {
      objects[o].blockedu = 1;
    }
    setblockedu: ;

 // set blockedl
    objects[o].blockedl = 0;
    x = (objects[o].x>>CSF)-1;
    y = (objects[o].y>>CSF)+1;
    for(ya=0;ya<ysize;ya+=16)
    {
        if (tiles[getmaptileat(x,y+ya)].solidr)
        {
          objects[o].blockedl = 1;
          goto setblockedl;
        }
    }
    if (tiles[getmaptileat(x, ((objects[o].y>>CSF)+ysize-1))].solidr)
    {
      objects[o].blockedl = 1;
    }
    setblockedl: ;

 // set blockedr
    objects[o].blockedr = 0;
    x = (objects[o].x>>CSF)+xsize;
    y = (objects[o].y>>CSF)+1;
    for(ya=0;ya<ysize;ya+=16)
    {
        if (tiles[getmaptileat(x,y+ya)].solidl)
        {
          objects[o].blockedr = 1;
          goto setblockedr;
        }
    }
    if (tiles[getmaptileat(x, ((objects[o].y>>CSF)+ysize-1))].solidl)
    {
      objects[o].blockedr = 1;
    }
    setblockedr: ;
  
    // hit detection with players
    objects[o].touchPlayer = 0;
    for(cplayer=0;cplayer<MAX_PLAYERS;cplayer++)
    {
      if (player[cplayer].isPlaying)
      {
        objects[player[cplayer].useObject].x = player[cplayer].x;
        objects[player[cplayer].useObject].y = player[cplayer].y;
        objects[player[cplayer].useObject].sprite = 0;
        if (!player[cplayer].pdie)
        {
          if (hitdetect(o, player[cplayer].useObject))
          {
            objects[o].touchPlayer = 1;
            objects[o].touchedBy = cplayer;
            break;
          }
        }
      }
    }

// have object fall if it should
  if (!objects[o].inhibitfall)
  {
       #define OBJFALLSPEED   20
       if (objects[o].blockedd)
       {
         objects[o].yinertia = 0;
       }
       else
       {
#define OBJ_YINERTIA_RATE  5
         if (objects[o].yinertiatimer>OBJ_YINERTIA_RATE)
         {
           if (objects[o].yinertia < OBJFALLSPEED) objects[o].yinertia++;
           objects[o].yinertiatimer = 0;
         } else objects[o].yinertiatimer++;
       }
       objects[o].y += objects[o].yinertia;
  }
}

// returns nonzero if object1 overlaps object2
char hitdetect(int object1, int object2)
{
int s1, s2;
unsigned int rect1x1, rect1y1, rect1x2, rect1y2;
unsigned int rect2x1, rect2y1, rect2x2, rect2y2;

  // get the sprites used by the two objects
  s1 = objects[object1].sprite;
  s2 = objects[object2].sprite;

  // get the bounding rectangle of the first object
  rect1x1 = objects[object1].x + sprites[s1].bboxX1;
  rect1y1 = objects[object1].y + sprites[s1].bboxY1;
  rect1x2 = objects[object1].x + sprites[s1].bboxX2;
  rect1y2 = objects[object1].y + sprites[s1].bboxY2;

  // get the bounding rectangle of the second object
  rect2x1 = objects[object2].x + sprites[s2].bboxX1;
  rect2y1 = objects[object2].y + sprites[s2].bboxY1;
  rect2x2 = objects[object2].x + sprites[s2].bboxX2;
  rect2y2 = objects[object2].y + sprites[s2].bboxY2;

  // find out if the rectangles overlap
  if ((rect1x1 < rect2x1) && (rect1x2 < rect2x1)) return 0;
  if ((rect1x1 > rect2x2) && (rect1x2 > rect2x2)) return 0;
  if ((rect1y1 < rect2y1) && (rect1y2 < rect2y1)) return 0;
  if ((rect1y1 > rect2y2) && (rect1y2 > rect2y2)) return 0;

  return 1;
}

void killplayer(int theplayer)
{
   if (player[theplayer].godmode || immediate_keytable[KTAB]) return;
   if (player[theplayer].ankhtime) return;
   if (levelcontrol.level_done) return;
   if (!player[theplayer].pdie)
   {
      player[theplayer].pdie = PDIE_DYING;
      player[theplayer].pdieframe = 0;
      player[theplayer].pdietimer = 0;
      player[theplayer].pdietillfly = DIE_TILL_FLY_TIME;
      player[theplayer].pdie_xvect = rand()%(DIE_MAX_XVECT*2);
      player[theplayer].pdie_xvect -= DIE_MAX_XVECT;
      player[theplayer].inventory.lives--;
      player[theplayer].y += (8<<CSF);
      gamepdo_SelectFrame(theplayer);
      sound_play(SOUND_KEEN_DIE, PLAY_NOW);
   }
}

void freezeplayer(int theplayer)
{
   if (player[theplayer].godmode || immediate_keytable[KTAB]) return;
   if (player[theplayer].ankhtime) return;
   // give the player a little "kick"
   player[theplayer].pjumptime = PJUMP_NORMALTIME_1;
   player[theplayer].pjumpupdecreaserate = PJUMP_UPDECREASERATE_1;
   player[theplayer].pjumpupspeed = 15;
   player[theplayer].pjumping = PJUMPUP;
   player[theplayer].pjumpupspeed_decreasetimer = 0;
   player[theplayer].pjustjumped = 1;

   // and freeze him (stun him on ep2/3)
   player[theplayer].pfrozentime = PFROZEN_TIME;
   player[theplayer].pfrozenframe = 0;
   player[theplayer].pfrozenanimtimer = 0;
   player[theplayer].ppogostick = 0;
}


void PlayerTouchedExit(int theplayer)
{
int i;
       if (!player[theplayer].pjumping && !player[theplayer].pfalling\
           && !player[theplayer].ppogostick && \
           levelcontrol.level_done==LEVEL_NOT_DONE)
       {
            // don't allow player to walk through a door if he's standing
            // on an object such as a platform or an enemy
            if (player[theplayer].psupportingobject)
            {
              return;
            }

            // if player has ankh shut it off
            if (player[theplayer].ankhtime)
            {
              player[theplayer].ankhtime = 0;
              objects[player[theplayer].ankhshieldobject].exists = 0;
            }

            player[theplayer].ppogostick = 0;

            sound_play(SOUND_LEVEL_DONE, PLAY_NOW);
            levelcontrol.level_done = LEVEL_DONE_WALK;
            levelcontrol.level_finished_by = theplayer;
       }
}

void endlevel(int success)
{
  if (fade.mode == NO_FADE)
  {
    fade.dir = FADE_OUT;
    fade.curamt = PAL_FADE_SHADES;
    fade.fadetimer = 0;
    fade.rate = FADE_NORM;
    fade.mode = FADE_GO;
    levelcontrol.success = success;
    levelcontrol.tobonuslevel = 0;
  }
}

void SetGameOver(void)
{
int x,y,bmnum;
   if (!levelcontrol.gameovermode)
   {
     levelcontrol.gameovermode = 1;
     sound_play(SOUND_GAME_OVER, PLAY_NOW);
   }
}


// this is so objects can block the player,
// player can stand on them, etc.
// x and y are the CSFed coordinates to check (e.g. playx and playy)
// returns nonzero if there is a solid object
// at that point
char checkobjsolid(int x, int y, int cp)
{
  int o;

   for(o=1;o<MAX_OBJECTS;o++)
   {
      if (objects[o].exists && objects[o].cansupportplayer[cp])
      {
        if (x >= objects[o].x+sprites[objects[o].sprite].bboxX1)
          if (x <= objects[o].x+sprites[objects[o].sprite].bboxX2)
            if (y >= objects[o].y+sprites[objects[o].sprite].bboxY1)
              if (y <= objects[o].y+sprites[objects[o].sprite].bboxY2)
                return o;
      }
   }
 return 0;
}

// returns 1 if player cp has the card to door t
char CheckDoorBlock(int t, int cp)
{
   if (levelcontrol.episode!=3)
   {  // episode 1-2
        if (t==DOOR_YELLOW || t==DOOR_YELLOW+1)
        {
          if (!player[cp].inventory.HasCardYellow)
          {
            player[cp].blockedby = DOOR_YELLOW;
            return 1;
          }
        }
        else if (t==DOOR_RED || t==DOOR_RED+1)
        {
          if (!player[cp].inventory.HasCardRed)
          {
            player[cp].blockedby = DOOR_RED;
            return 1;
          }
        }
        else if (t==DOOR_GREEN || t==DOOR_GREEN+1)
        {
          if (!player[cp].inventory.HasCardGreen)
          {
            player[cp].blockedby = DOOR_GREEN;
            return 1;
          }
        }
        else if (t==DOOR_BLUE || t==DOOR_BLUE+1)
        {
          if (!player[cp].inventory.HasCardBlue)
          {
            player[cp].blockedby = DOOR_BLUE;
            return 1;
          }
        }
   }
   else
   {  // episode 3
        if (t==DOOR_YELLOW_EP3 || t==DOOR_YELLOW_EP3+1)
        {
          if (!player[cp].inventory.HasCardYellow)
          {
            player[cp].blockedby = DOOR_YELLOW_EP3;
            return 1;
          }
        }
        else if (t==DOOR_RED_EP3 || t==DOOR_RED_EP3+1)
        {
          if (!player[cp].inventory.HasCardRed)
          {
            player[cp].blockedby = DOOR_RED_EP3;
            return 1;
          }
        }
        else if (t==DOOR_GREEN_EP3 || t==DOOR_GREEN_EP3+1)
        {
          if (!player[cp].inventory.HasCardGreen)
          {
            player[cp].blockedby = DOOR_GREEN_EP3;
            return 1;
          }
        }
        else if (t==DOOR_BLUE_EP3 || t==DOOR_BLUE_EP3+1)
        {
          if (!player[cp].inventory.HasCardBlue)
          {
            player[cp].blockedby = DOOR_BLUE_EP3;
            return 1;
          }
        }
   }

   return 0;
}

// checks if tile at (x,y) is solid to the player walking left into it.
// returns 1 and sets blockedby if so.
char checkissolidl(int x, int y, int cp)
{
int t;
  t = getmaptileat(x, y);
  if (tiles[t].solidl)
  {
    player[cp].blockedby = t;
    return 1;
  }
  if (checkobjsolid(x<<CSF,y<<CSF,cp))
  {
    player[cp].blockedby = 0;
    return 1;
  }
  else
  {
    // don't let player walk through doors he doesn't have the key to
    if (CheckDoorBlock(t, cp))
    {
      return 1;
    }
  }
  return 0;
}

// checks if tile at (x,y) is solid to the player walking right into it.
// returns 1 and sets blockedby if so.
char checkissolidr(int x, int y, int cp)
{
int t;
  t = getmaptileat(x, y);
  if (tiles[t].solidr)
  {
    player[cp].blockedby = t;
    return 1;
  }
  else if (checkobjsolid(x<<CSF,y<<CSF,cp))
  {
    player[cp].blockedby = 0;
    return 1;
  }
  else
  {
    // don't let player walk through doors he doesn't have the key to
    if (CheckDoorBlock(t, cp))
    {
      return 1;
    }
  }
  return 0;
}

// copies tile data into a sprite. multiple tiles can be copied into
// a single sprite--they will be stacked up vertically.
void CopyTileToSprite(int t, int s, int ntilestocopy, int transparentcol)
{
int x,y1,y2,tboundary;
    sprites[s].xsize = 16;
    sprites[s].ysize = 16 * ntilestocopy;
    sprites[s].bboxX1 = sprites[s].bboxY1 = 0;
    sprites[s].bboxX2 = (sprites[s].xsize << CSF);
    sprites[s].bboxY2 = (sprites[s].ysize << CSF);

    tboundary = 0;
    y2 = 0;
    for(y1=0;y1<(16*ntilestocopy);y1++)
    {
      for(x=0;x<16;x++)
      {
        sprites[s].imgdata[y1][x] = tiledata[t][y2][x];
        if (sprites[s].imgdata[y1][x] != transparentcol)
        {
          sprites[s].maskdata[y1][x] = 15;
        }
        else
        {
          sprites[s].maskdata[y1][x] = 0;
        }
      }
      y2++;
      if (y2>=16)
      {
        y2 = 0;
        t++;
      }
    }
}

// creates a mask from a sourcetile, places it in desttile
void MakeMask(int sourcetile, int desttile, int transparentcol)
{
int x,y,c;
   for(y=0;y<16;y++)
   {
     for(x=0;x<16;x++)
     {
        c = tiledata[sourcetile][y][x];
        if (c != transparentcol) c = 0; else c = 15;
        tiledata[desttile][y][x] = c;
     }
   }
}

// replaces all instances of color find in sprite s with
// color replace, as long as the y is greater than miny
void ReplaceSpriteColor(int s, int find, int replace, int miny)
{
int x,y;

 for(y=miny;y<sprites[s].ysize;y++)
 {
   for(x=0;x<sprites[s].xsize;x++)
   {
     if (sprites[s].imgdata[y][x]==find)
     {
       sprites[s].imgdata[y][x] = replace;
     }
   }
 }
}

// duplicates sprite source to dest
void CopySprite(int source, int dest)
{
int x,y;

  sprites[dest].xsize = sprites[source].xsize;
  sprites[dest].ysize = sprites[source].ysize;

  for(y=0;y<sprites[source].ysize;y++)
  {
    for(x=0;x<sprites[source].xsize;x++)
    {
      sprites[dest].imgdata[y][x] = sprites[source].imgdata[y][x];
      sprites[dest].maskdata[y][x] = sprites[source].maskdata[y][x];
    }
  }
}

// initilize sprites that come from tiles, such as the doors
void initsprites(void)
{
int x,y,i,s,indx;
FILE *fp;

    if (levelcontrol.episode!=3)
    {
      CopyTileToSprite(DOOR_YELLOW, DOOR_YELLOW_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_RED, DOOR_RED_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_GREEN, DOOR_GREEN_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_BLUE, DOOR_BLUE_SPRITE, 2, 7);
    }
    else
    {
      CopyTileToSprite(DOOR_YELLOW_EP3, DOOR_YELLOW_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_RED_EP3, DOOR_RED_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_GREEN_EP3, DOOR_GREEN_SPRITE, 2, 7);
      CopyTileToSprite(DOOR_BLUE_EP3, DOOR_BLUE_SPRITE, 2, 7);
    }

    // create BLANKSPRITE
    s = LatchHeader.NumSprites;
    BlankSprite = s;
    sprites[BlankSprite].xsize = sprites[BlankSprite].ysize = 0;

    // create DEMOSPRITE (the "demo" message the appears on the screen
    // during a demo--the graphics for it are stored in demobox.h)
    s++;
    DemoSprite = s;
    sprites[DemoSprite].xsize = 48;
    sprites[DemoSprite].ysize = 16;
    indx = 0;
    for(y=0;y<DEMOBOX_HEIGHT;y++)
    {
      for(x=0;x<DEMOBOX_WIDTH;x++)
      {
        sprites[DemoSprite].imgdata[y][x] = demobox_image[indx];
        sprites[DemoSprite].maskdata[y][x] = demobox_mask[indx];
        indx++;
      }
    }

    // create the sprites for player 2 
    s++;
    playerbaseframes[1] = s;
    for(i=0;i<48;i++)
    {
      CopySprite(i, s);
      ReplaceSpriteColor(s, 13, 10, 0);
      ReplaceSpriteColor(s, 5, 2, 0);
      ReplaceSpriteColor(s, 9, 14, 8);
      ReplaceSpriteColor(s, 1, 6, 8);
      ReplaceSpriteColor(s, 12, 11, 0);
      ReplaceSpriteColor(s, 4, 3, 0);
      s++;
    }

    // create the sprites for player 3
    playerbaseframes[2] = s;
    for(i=0;i<48;i++)
    {
      CopySprite(i, s);
      ReplaceSpriteColor(s, 12, 6, 0);
      ReplaceSpriteColor(s, 4, 6, 0);
      ReplaceSpriteColor(s, 13, 12, 0);
      ReplaceSpriteColor(s, 5, 4, 0);
      ReplaceSpriteColor(s, 9, 12, 8);
      ReplaceSpriteColor(s, 1, 4, 8);
      s++;
    }

    // set up mask for ice cannons (ep1)
    if (levelcontrol.episode==1)
    {

//       MakeMask(443, 155, 7);
  //     tiles[443].masktile = 155;
//       tiles[428].masktile = 155;
    }
}

void procgoodie_ep1(int t, int mpx, int mpy, int theplayer)
{
   if (t>=201 && t<=205) sound_play(SOUND_GET_BONUS, PLAY_NOW);
   else if (t==175||t==176) sound_play(SOUND_GET_ITEM, PLAY_NOW);
   switch(t)
   {
    // keycards
    case 190: give_keycard(DOOR_YELLOW, theplayer); break;
    case 191: give_keycard(DOOR_RED, theplayer); break;
    case 192: give_keycard(DOOR_GREEN, theplayer); break;
    case 193: give_keycard(DOOR_BLUE, theplayer); break;

    case DOOR_YELLOW:
           if (player[theplayer].inventory.HasCardYellow)
             open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_YELLOW+1:
           if (player[theplayer].inventory.HasCardYellow)
             open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_RED:
           if (player[theplayer].inventory.HasCardRed)
             open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_RED+1:
           if (player[theplayer].inventory.HasCardRed)
             open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_GREEN:
           if (player[theplayer].inventory.HasCardGreen)
             open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_GREEN+1:
           if (player[theplayer].inventory.HasCardGreen)
             open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_BLUE:
           if (player[theplayer].inventory.HasCardBlue)
             open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_BLUE+1:
           if (player[theplayer].inventory.HasCardBlue)
             open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy-1, theplayer);
         break;
    
    case 202:    // lollipop
         player[theplayer].inventory.score += 100; extralifeat(theplayer);
         break;
    case 203:    // pepsi
         player[theplayer].inventory.score += 200; extralifeat(theplayer);
         break;
    case 201:    // pizza
         player[theplayer].inventory.score += 500; extralifeat(theplayer);
         break;
    case 204:    // book
         player[theplayer].inventory.score += 1000; extralifeat(theplayer);
         break;
    case 205:    // teddy bear
         player[theplayer].inventory.score += 5000; extralifeat(theplayer);
         break;

    case 175:           // raygun
         player[theplayer].inventory.charges += 5;
    break;
    case 176:           // the Holy Pogo Stick
         player[theplayer].inventory.HasPogo = 1;
    break;
    
    case 221: case 222: case 223: case 224:   //joystick
     player[theplayer].inventory.HasJoystick = 1;
     sound_play(SOUND_GET_PART, PLAY_NOW);
    break;
    case 237: case 238: case 239: case 240:   //battery
     player[theplayer].inventory.HasBattery = 1;
     sound_play(SOUND_GET_PART, PLAY_NOW);
    break;
    case 241: case 242: case 243: case 244:   //vacuum cleaner
     player[theplayer].inventory.HasVacuum = 1;
     sound_play(SOUND_GET_PART, PLAY_NOW);
    break;
    case 245: case 246: case 247: case 248:   //fuel
     player[theplayer].inventory.HasFuel = 1;
     sound_play(SOUND_GET_PART, PLAY_NOW);
    break;

    // in-level teleporter
    // (in level13.ck1 that takes you to the bonus level)
    case 481:
    case 494:
        endlevel(0);
        levelcontrol.tobonuslevel = 1;
    break;

    case YORPSTATUEHEAD:
      youseeinyourmind(mpx, mpy, 0);
      tiles[YORPSTATUEHEAD].goodie = 0;
      tiles[YORPSTATUEBOTTOM].goodie = 0;
      break;
    case YORPSTATUEBOTTOM:
      youseeinyourmind(mpx, mpy-1, 0);
      tiles[YORPSTATUEHEAD].goodie = 0;
      tiles[YORPSTATUEBOTTOM].goodie = 0;
      break;
    // garg statue top-left
    case 433:
      youseeinyourmind(mpx, mpy, 1);
      tiles[433].goodie = tiles[436].goodie = 0;
      tiles[446].goodie = tiles[447].goodie = 0;
      break;
    // garg statue top-right when it's glowing
    case 436:
      youseeinyourmind(mpx-1, mpy, 1);
      tiles[433].goodie = tiles[436].goodie = 0;
      tiles[446].goodie = tiles[447].goodie = 0;
      break;
    // garg statue bottom-left
    case 446:
      youseeinyourmind(mpx, mpy-1, 1);
      tiles[433].goodie = tiles[436].goodie = 0;
      tiles[446].goodie = tiles[447].goodie = 0;
      break;
    // garg statue bottom-right
    case 447:
      youseeinyourmind(mpx-1, mpy-1, 1);
      tiles[433].goodie = tiles[436].goodie = 0;
      tiles[446].goodie = tiles[447].goodie = 0;
      break;
    // exit door
    case 159:
      if (levelcontrol.canexit && (!levelcontrol.isfinallevel || player[theplayer].inventory.HasFuel))
      {
        levelcontrol.exitXpos = (mpx+2)<<4;
        PlayerTouchedExit(theplayer);
      }
    break;

    // we fell off the bottom of the map
    case TILE_FELLOFFMAP_EP1:
      if (!player[theplayer].pdie)
      {
        sound_play(SOUND_KEEN_FALL, PLAY_FORCE);
        player[theplayer].ankhtime = 0;
        player[theplayer].godmode = 0;
        player[theplayer].pdie = PDIE_FELLOFFMAP;
      }
      break;

    default: crashflag = 1;
             crashflag2 = t;
             why_term_ptr = "procgoodie_ep1: Unknown goodie-- value given in flag2.";
             break;
   }


}

void procgoodie_ep2(int t, int mpx, int mpy, int theplayer)
{
   switch(t)
   {
    // keycards
    case 190: give_keycard(DOOR_YELLOW, theplayer); break;
    case 191: give_keycard(DOOR_RED, theplayer); break;
    case 192: give_keycard(DOOR_GREEN, theplayer); break;
    case 193: give_keycard(DOOR_BLUE, theplayer); break;

    case DOOR_YELLOW:
           if (player[theplayer].inventory.HasCardYellow)
             open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_YELLOW+1:
           if (player[theplayer].inventory.HasCardYellow)
             open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_RED:
           if (player[theplayer].inventory.HasCardRed)
             open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_RED+1:
           if (player[theplayer].inventory.HasCardRed)
             open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_GREEN:
           if (player[theplayer].inventory.HasCardGreen)
             open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_GREEN+1:
           if (player[theplayer].inventory.HasCardGreen)
             open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_BLUE:
           if (player[theplayer].inventory.HasCardBlue)
             open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_BLUE+1:
           if (player[theplayer].inventory.HasCardBlue)
             open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy-1, theplayer);
         break;
    
    case 202:    // candy bar
    case 306:
         player[theplayer].inventory.score += 100; extralifeat(theplayer);
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         break;
    case 203:    // hamburger
    case 307:
         player[theplayer].inventory.score += 200; extralifeat(theplayer);
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         break;
    case 201:    // coke
    case 308:
         player[theplayer].inventory.score += 500; extralifeat(theplayer);
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         break;
    case 204:    // cake
    case 309:
         player[theplayer].inventory.score += 1000; extralifeat(theplayer);
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         break;
    case 205:    // bear
    case 310:
         player[theplayer].inventory.score += 5000; extralifeat(theplayer);
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         break;

    case 175:           // raygun
    case 311:
         player[theplayer].inventory.charges += 5;
         sound_play(SOUND_GET_ITEM, PLAY_NOW);
    break;

    case 176: break;    // pogo (not used in ep2)

    case 429:           // switch for vorticon elder
      VorticonElder(mpx, mpy);
      tiles[429].standgoodie = 0;
    break;
    
    // exit door
    case 159:
      if (levelcontrol.canexit)
      {
        levelcontrol.exitXpos = (mpx+2)<<4;
        PlayerTouchedExit(theplayer);
      }
    break;

    default: crashflag = 1;
             crashflag2 = t;
             why_term_ptr = "procgoodie_ep2: Unknown goodie-- value given in flag2.";
             break;
   }

}

void procgoodie_ep3(int t, int mpx, int mpy, int theplayer)
{
   switch(t)
   {
    case 144:    // candy bar
    case 144+13:
    case 144+13+13:
    case 144+13+13+13:
    case 144+13+13+13+13:
    case 144+13+13+13+13+13:
         player[theplayer].inventory.score += 100;
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         extralifeat(theplayer);
         break;
    case 145:    // hamburger
    case 145+13:
    case 145+13+13:
    case 145+13+13+13:
    case 145+13+13+13+13:
    case 145+13+13+13+13+13:
         player[theplayer].inventory.score += 200;
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         extralifeat(theplayer);
         break;
    case 146:    // coke
    case 146+13:
    case 146+13+13:
    case 146+13+13+13:
    case 146+13+13+13+13:
    case 146+13+13+13+13+13:
         player[theplayer].inventory.score += 500;
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         extralifeat(theplayer);
         break;
    case 147:    // cake
    case 147+13:
    case 147+13+13:
    case 147+13+13+13:
    case 147+13+13+13+13:
    case 147+13+13+13+13+13:
         player[theplayer].inventory.score += 1000;
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         extralifeat(theplayer);
         break;
    case 148:    // bear
    case 148+13:
    case 148+13+13:
    case 148+13+13+13:
    case 148+13+13+13+13:
    case 148+13+13+13+13+13:
         player[theplayer].inventory.score += 5000;
         sound_play(SOUND_GET_BONUS, PLAY_NOW);
         extralifeat(theplayer);
         break;
    case 149:    // ankh
    case 149+13:
    case 149+13+13:
    case 149+13+13+13:
    case 149+13+13+13+13:
    case 149+13+13+13+13+13:
         GiveAnkh(theplayer);
         break;
    case 150:   // raygun charge
    case 150+13:
    case 150+13+13:
    case 150+13+13+13:
    case 150+13+13+13+13:
    case 150+13+13+13+13+13:
      player[theplayer].inventory.charges++;
      sound_play(SOUND_GET_ITEM, PLAY_NOW);
      break;
    case 151:   // raygun
    case 151+13:
    case 151+13+13:
    case 151+13+13+13:
    case 151+13+13+13+13:
    case 151+13+13+13+13+13:
      player[theplayer].inventory.charges += 5;
      sound_play(SOUND_GET_ITEM, PLAY_NOW);
      break;
    case 152:   // yellow keycard
    case 152+13:
    case 152+13+13:
    case 152+13+13+13:
    case 152+13+13+13+13:
    case 152+13+13+13+13+13:
      give_keycard(DOOR_YELLOW_EP3, theplayer);
      break;
    case 153:   // red keycard
    case 153+13:
    case 153+13+13:
    case 153+13+13+13:
    case 153+13+13+13+13:
    case 153+13+13+13+13+13:
      give_keycard(DOOR_RED_EP3, theplayer);
      break;
    case 154:   // green keycard
    case 154+13:
    case 154+13+13:
    case 154+13+13+13:
    case 154+13+13+13+13:
    case 154+13+13+13+13+13:
      give_keycard(DOOR_GREEN_EP3, theplayer);
      break;
    case 155:   // blue keycard
    case 155+13:
    case 155+13+13:
    case 155+13+13+13:
    case 155+13+13+13+13:
    case 155+13+13+13+13+13:
      give_keycard(DOOR_BLUE_EP3, theplayer);
      break;
    case DOOR_YELLOW_EP3:
         if (player[theplayer].inventory.HasCardYellow)
           open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_YELLOW_EP3+1:
         if (player[theplayer].inventory.HasCardYellow)
           open_door(DOOR_YELLOW, DOOR_YELLOW_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_RED_EP3:
         if (player[theplayer].inventory.HasCardRed)
           open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_RED_EP3+1:
         if (player[theplayer].inventory.HasCardRed)
           open_door(DOOR_RED, DOOR_RED_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_GREEN_EP3:
         if (player[theplayer].inventory.HasCardGreen)
           open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_GREEN_EP3+1:
         if (player[theplayer].inventory.HasCardGreen)
           open_door(DOOR_GREEN, DOOR_GREEN_SPRITE, mpx, mpy-1, theplayer);
         break;
    case DOOR_BLUE_EP3:
         if (player[theplayer].inventory.HasCardBlue)
           open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy, theplayer);
         break;
    case DOOR_BLUE_EP3+1:
         if (player[theplayer].inventory.HasCardBlue)
           open_door(DOOR_BLUE, DOOR_BLUE_SPRITE, mpx, mpy-1, theplayer);
         break;

    case 78:
    // cave spikes pointing up
      killplayer(theplayer);
    break;

    // left side of exit door
    case 255:
    case 242:
      if (levelcontrol.canexit)
      {
        levelcontrol.exitXpos = (mpx+2)<<4;
        PlayerTouchedExit(theplayer);
      }
      break;
    // middle of exit door (right side of green part)
    case 243:
    case 256:
    break;
    /*
      if (levelcontrol.canexit)
      {
        levelcontrol.exitXpos = (mpx+1)<<4;
        PlayerTouchedExit(theplayer);
      }
      break;*/
    // we fell off the bottom of the map
    case TILE_FELLOFFMAP_EP3:
      if (!player[theplayer].pdie)
      {
        sound_play(SOUND_KEEN_FALL, PLAY_FORCE);
        player[theplayer].ankhtime = 0;
        player[theplayer].godmode = 0;
        player[theplayer].pdie = PDIE_FELLOFFMAP;
      }
      break;

    default: crashflag = 1;
             crashflag2 = t;
             why_term_ptr = "procgoodie_ep3: Unknown goodie-- value given in flag2.";
             break;
   }
}

void GiveAnkh(int cp)
{
int o;
  if (!player[cp].ankhtime)
  {
    o = spawn_object(player[cp].x, player[cp].y, OBJ_SECTOREFFECTOR);
    objects[o].ai.se.type = SE_ANKHSHIELD;
    player[cp].ankhshieldobject = o;
  }

  sound_play(SOUND_ANKH, PLAY_NOW);
  player[cp].ankhtime = PLAY_ANKH_TIME;
  gamepdo_ankh(cp);
}

void gameloop_initialize(void)
{
int x,y,i,tl;
int timeout;

   if (levelcontrol.episode == 3)
   {
     // coat the top of the map ("oh no!" border) with a non-solid tile
     // so keen can jump partially off the top of the screen
     for(x=1;x<map.xsize-2;x++)
     {
         map.mapdata[x][1] = 143;
     }

     // make it lethal to fall off the bottom of the map
     // the "oh no" border is set solidceil=0, and here we're
     // going to coat the bottom of the map, below the "oh no"
     // border, with a special tile that has standgoodie set
     // and will trigger the "fell off the map" kill
     y = map.ysize;
     for(x=2;x<map.xsize-2;x++)
     {
        map.mapdata[x][y] = TILE_FELLOFFMAP_EP3;
     }
   }

  if (levelcontrol.episode==1)
  {
    // coat the bottom of the map below the border.
    // since the border has solidceil=1 this provides
    // a platform to catch yorps that fall off the map
    y = map.ysize;
    for(x=2;x<map.xsize-2;x++)
    {
       map.mapdata[x][y] = TILE_FELLOFFMAP_EP1;
    }
  }                             

   // draw map first time
   drawmap();

   // generate other player's start positions
   // don't generate player positions for the world
   // map, except the first time
   if (!map.isworldmap || map.firsttime)
   {
     x = player[0].x;
     for(i=1;i<numplayers;i++)
     {
      if (player[0].x>>CSF>>4 < (map.xsize/2) || levelcontrol.episode==1)
      {
        x += (18<<CSF);
      }
      else
      {
        x -= (18<<CSF);
      }
      player[i].x = x;
      player[i].y = player[0].y;
     }
   }
   map.firsttime = 0;
   // set the maximum amount the map is allowed to scroll
   max_scroll_x = (map.xsize-2-(320/16))<<4;
   max_scroll_y = (map.ysize<<4)-200-32;

   // players start facing left if the start position
   // is on the left half of the map, else right
   // for ep1 it's always facing right (because of level08)
   for(i=0;i<MAX_PLAYERS;i++)
   {
    if (map.isworldmap)
      player[i].pdir = player[i].pshowdir = DOWN;
    else
    {
      if (player[i].x>>CSF>>4 < (map.xsize/2) || levelcontrol.episode==1)
        player[i].pdir = player[i].pshowdir = RIGHT;
      else
        player[i].pdir = player[i].pshowdir = LEFT;
    }
   }

// scroll past the first two tiles (the level border), they'll
// now never be visible because you're not allowed to scroll
// left past X=32.
   for(i=0;i<2*16;i++)
   {
      map_scroll_right();
      map_scroll_down();
   }

// scroll the screen until the primary player is onscreen
// enough to where he doesn't set off the scroll triggers
  for(timeout=0;timeout<10000;timeout++)
  {
    if (!gamedo_ScrollTriggers(primaryplayer)) break;
  }

  // initiate fade-in
  fade.mode = FADE_GO;
  fade.dir = FADE_IN;
  fade.rate = FADE_NORM;
  fade.curamt = 0;
  fade.fadetimer = 0;

  // "keens left" when returning to world map after dying
  if (levelcontrol.dokeensleft)
  {
    keensleft();
    levelcontrol.dokeensleft = 0;
  }

}
