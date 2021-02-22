/* GAMEDO.C
  Contains all of the gamedo_xxx functions...which are called from the
  main game loop. These functions perform some task that is done each
  time around the game loop, not directly related to the player.
*/

#include "keen.h"

extern unsigned long gotPlayX;

extern unsigned long CurrentTickCount;

extern unsigned int unknownKey;

int animtiletimer, curanimtileframe;

// gathers data from input controllers: keyboard, joystick, network,
// whatever to populate each player's keytable
unsigned char oldleftkey = 5;
unsigned char oldrightkey = 5;
unsigned char oldupkey = 5;
unsigned char olddownkey = 5;
unsigned char oldctrlkey = 5;
unsigned char oldaltkey = 5;
void gamedo_getInput(void)
{
int i;
int byt;
unsigned int msb, lsb;
int p, keysdirty;

     if (demomode==DEMO_PLAYBACK)
     {
        // time to get a new key block?
        if (!demo_RLERunLen)
        {
          /* get next RLE run length */
          lsb = demo_data[demo_data_index++];
          msb = demo_data[demo_data_index++];
          demo_RLERunLen = (msb<<8) | lsb;
          byt = demo_data[demo_data_index++];         // get keys down

          player[0].keytable[KLEFT] = 0;
          player[0].keytable[KRIGHT] = 0;
          player[0].keytable[KCTRL] = 0;
          player[0].keytable[KALT] = 0;

          if (byt & 1) player[0].keytable[KLEFT] = 1;
          if (byt & 2) player[0].keytable[KRIGHT] = 1;
          if (byt & 4) player[0].keytable[KCTRL] = 1;
          if (byt & 8) player[0].keytable[KALT] = 1;
          if (byt & 16)
          {  // demo STOP command
            if (fade.mode!=FADE_GO) endlevel(1);
          }
        }
        else
        {
          // we're still in the last RLE run, don't change any keys
          demo_RLERunLen--;
        }

        // user trying to cancel the demo?
        for(i=0;i<KEYTABLE_REALKEYS_SIZE;i++)
        {
          if (immediate_keytable[i] && !last_immediate_keytable[i])
          {
            if (fade.mode!=FADE_GO) endlevel(0);
          }
        }

        return;
     }

     if (is_server) return;

     p = primaryplayer;
     player[p].keytable[KQUIT] = immediate_keytable[KQUIT];
     player[p].keytable[KLEFT] = immediate_keytable[KLEFT];
     player[p].keytable[KRIGHT] = immediate_keytable[KRIGHT];
     player[p].keytable[KUP] = immediate_keytable[KUP];
     player[p].keytable[KDOWN] = immediate_keytable[KDOWN];
     player[p].keytable[KCTRL] = immediate_keytable[KCTRL];
     player[p].keytable[KALT] = immediate_keytable[KALT];
     player[p].keytable[KENTER] = immediate_keytable[KENTER];

     if (!immediate_keytable[KC] && !immediate_keytable[KT])
       player[p].keytable[KSPACE] = immediate_keytable[KSPACE];

     #ifdef NETWORK_PLAY
//       if (numplayers>1)
       if (is_client)
       {
         if (player[p].keytable[KLEFT] != oldleftkey) keysdirty = 1;
         else if (player[p].keytable[KRIGHT] != oldrightkey) keysdirty = 1;
         else if (player[p].keytable[KUP] != oldupkey) keysdirty = 1;
         else if (player[p].keytable[KDOWN] != olddownkey) keysdirty = 1;
         else if (player[p].keytable[KCTRL] != oldctrlkey) keysdirty = 1;
         else if (player[p].keytable[KALT] != oldaltkey) keysdirty = 1;
         else keysdirty = 0;
         if (keysdirty)
         {
           net_sendkeys();
           oldleftkey = player[p].keytable[KLEFT];
           oldrightkey = player[p].keytable[KRIGHT];
           oldupkey = player[p].keytable[KUP];
           olddownkey = player[p].keytable[KDOWN];
           oldctrlkey = player[p].keytable[KCTRL];
           oldaltkey = player[p].keytable[KALT];
         }
       }
    #endif

    if (numplayers>1 && localmp)
    {
       player[1].keytable[KQUIT] = 0;
       player[1].keytable[KLEFT] = immediate_keytable[KLEFT2];
       player[1].keytable[KRIGHT] = immediate_keytable[KRIGHT2];
       player[1].keytable[KUP] = immediate_keytable[KUP2];
       player[1].keytable[KDOWN] = immediate_keytable[KDOWN2];
       player[1].keytable[KCTRL] = immediate_keytable[KCTRL2];
       player[1].keytable[KALT] = immediate_keytable[KALT2];
       player[1].keytable[KENTER] = 0;
       player[1].keytable[KSPACE] = 0;

       player[2].keytable[KQUIT] = 0;
       player[2].keytable[KLEFT] = immediate_keytable[KLEFT3];
       player[2].keytable[KRIGHT] = immediate_keytable[KRIGHT3];
       player[2].keytable[KUP] = immediate_keytable[KUP3];
       player[2].keytable[KDOWN] = immediate_keytable[KDOWN3];
       player[2].keytable[KCTRL] = immediate_keytable[KCTRL3];
       player[2].keytable[KALT] = immediate_keytable[KALT3];
       player[2].keytable[KENTER] = 0;
       player[2].keytable[KSPACE] = 0;
    }

    if (demomode==DEMO_RECORD)
    {
       fputc(player[0].keytable[KLEFT], demofile);
       fputc(player[0].keytable[KRIGHT], demofile);
       fputc(player[0].keytable[KCTRL], demofile);
       fputc(player[0].keytable[KALT], demofile);
       fputc(immediate_keytable[KF1], demofile);
    }
}

// handles scrolling, for player cp
// returns nonzero if the scroll was changed
int gamedo_ScrollTriggers(int theplayer)
{
signed int px, py;
int scrollchanged;

   if (player[theplayer].pdie) return 0;

   px = (player[theplayer].x>>CSF)-scroll_x;
   py = (player[theplayer].y>>CSF)-scroll_y;
   scrollchanged = 0;

   /* left-right scrolling */
   if(px > SCROLLTRIGGERRIGHT && scroll_x < max_scroll_x)
   {
      map_scroll_right();
      scrollchanged = 1;
   }
   else if(px < SCROLLTRIGGERLEFT && scroll_x > 32)
   {
      map_scroll_left();
      scrollchanged = 1;
   }

   /* up-down scrolling */
   if (py > SCROLLTRIGGERDOWN && scroll_y < max_scroll_y)
   {
      map_scroll_down();
      scrollchanged = 1;
   }
   else if (py < SCROLLTRIGGERUP && scroll_y > 32)
   {
      map_scroll_up();
      scrollchanged = 1;
   }

   return scrollchanged;
}

// animates animated tiles
void gamedo_AnimatedTiles(void)
{
int i;
   /* animate animated tiles */
   if (animtiletimer>ANIM_TILE_TIME)
   {
      /* advance to next frame */
      curanimtileframe = (curanimtileframe+1)&7;
      /* re-draw all animated tiles */
      for(i=1;i<MAX_ANIMTILES-1;i++)
      {
         if (animtiles[i].slotinuse)
         {
           drawtile(animtiles[i].x, animtiles[i].y, animtiles[i].baseframe+((animtiles[i].offset+curanimtileframe)%tiles[animtiles[i].baseframe].animmask));
        }
      }
      animtiletimer = 0;
   }
   else animtiletimer++;
}

// do object and enemy AI
void gamedo_enemyai(void)
{
int i;
// handle objects and do enemy AI
   for(i=1;i<MAX_OBJECTS-1;i++)
   {
      if (!objects[i].exists || objects[i].type==OBJ_PLAYER) continue;

      objects[i].scrx = (objects[i].x>>CSF)-scroll_x;
      objects[i].scry = (objects[i].y>>CSF)-scroll_y;
      if (objects[i].scrx < -(sprites[objects[i].sprite].xsize) || objects[i].scrx > 320 \
          || objects[i].scry < -(sprites[objects[i].sprite].ysize) || objects[i].scry > 200)
          {
             objects[i].onscreen = 0;
             objects[i].wasoffscreen = 1;
             if (objects[i].type==OBJ_ICEBIT) objects[i].exists = 0;
          }
          else
          {
             #ifdef TARGET_WIN32
//               if (numplayers>1)
//                 if (!objects[i].hasbeenonscreen)
//                   net_sendobjectonscreen(i);
             #endif

             objects[i].onscreen = 1;
             objects[i].hasbeenonscreen = 1;
          }

      if (objects[i].hasbeenonscreen || objects[i].type==OBJ_RAY || \
          objects[i].type==OBJ_ICECHUNK || objects[i].type==OBJ_PLATFORM ||\
          objects[i].type==OBJ_PLATVERT)
      {
         common_enemy_ai(i);
         switch(objects[i].type)
         {
          //KEEN1
          case OBJ_YORP: yorp_ai(i); break;
          case OBJ_GARG: garg_ai(i); break;
          case OBJ_VORT: vort_ai(i); break;
          case OBJ_BUTLER: butler_ai(i); break;
          case OBJ_TANK: tank_ai(i); break;
          case OBJ_RAY: ray_ai(i); break;
          case OBJ_DOOR: door_ai(i); break;
          case OBJ_ICECHUNK: icechunk_ai(i); break;
          case OBJ_ICEBIT: icebit_ai(i); break;
          case OBJ_TELEPORTER: teleporter_ai(i); break;
          case OBJ_ROPE: rope_ai(i); break;
          //KEEN2
          case OBJ_WALKER: walker_ai(i); break;
          case OBJ_TANKEP2: tankep2_ai(i); break;
          case OBJ_PLATFORM: platform_ai(i); break;
          case OBJ_BEAR: bear_ai(i); break;
          case OBJ_SECTOREFFECTOR: se_ai(i); break;
          case OBJ_BABY: baby_ai(i); break;
          case OBJ_EXPLOSION: explosion_ai(i); break;
          case OBJ_EARTHCHUNK: earthchunk_ai(i); break;
          //KEEN3
          case OBJ_FOOB: foob_ai(i); break;
          case OBJ_NINJA: ninja_ai(i); break;
          case OBJ_MEEP: meep_ai(i); break;
          case OBJ_SNDWAVE: sndwave_ai(i); break;
          case OBJ_MOTHER: mother_ai(i); break;
          case OBJ_FIREBALL: fireball_ai(i); break;
          case OBJ_BALL: ballandjack_ai(i); break;
          case OBJ_JACK: ballandjack_ai(i); break;
          case OBJ_PLATVERT: platvert_ai(i); break;
          case OBJ_NESSIE: nessie_ai(i); break;

          case OBJ_DEMOMSG: break;
          default:
            crashflag = 1;
            crashflag2 = i;
            crashflag3 = objects[i].type;
            why_term_ptr = "Invalid object flag2 of type flag3";
            break;
         }         

        objects[i].scrx = (objects[i].x>>CSF)-scroll_x;
        objects[i].scry = (objects[i].y>>CSF)-scroll_y;
      }
   }
}


int savew, saveh;

void gamedo_render_drawobjects(void)
{
int i,x,y,o,tl,xsize,ysize;
int xa,ya;

   // copy player data to their associated objects show they can get drawn
   // in the object-drawing loop with the rest of the objects
   for(i=0;i<numplayers;i++)
   {
     o = player[i].useObject;

     if (!player[i].hideplayer)
     {
       objects[o].sprite = player[i].playframe + playerbaseframes[i];
     }
     else
     {
       objects[o].sprite = BlankSprite;
     }

     objects[o].x = player[i].x;
     objects[o].y = player[i].y;
     objects[o].scrx = (player[i].x>>CSF)-scroll_x;
     objects[o].scry = (player[i].y>>CSF)-scroll_y;
   }

   // if we're playing a demo keep the "DEMO" message on the screen
   // as an object
   if (demomode==DEMO_PLAYBACK)
   {
     #define DEMO_X_POS         137
     #define DEMO_Y_POS         6
     objects[DemoObjectHandle].exists = 1;
     objects[DemoObjectHandle].onscreen = 1;
     objects[DemoObjectHandle].type = OBJ_DEMOMSG;
     objects[DemoObjectHandle].sprite = DemoSprite;
     objects[DemoObjectHandle].x = (DEMO_X_POS+scroll_x)<<CSF;
     objects[DemoObjectHandle].y = (DEMO_Y_POS+scroll_y)<<CSF;
     objects[DemoObjectHandle].honorPriority = 0;
   }
   else objects[DemoObjectHandle].exists = 0;

   // draw all objects. drawn in reverse order because the player sprites
   // are in the first few indexes and we want them to come out on top.
   for(i=MAX_OBJECTS-1;i>=0;i--)
   {
      if (objects[i].exists && objects[i].onscreen)
      {
        objects[i].scrx = ((objects[i].x>>CSF)-scroll_x);
        objects[i].scry = ((objects[i].y>>CSF)-scroll_y);
        drawsprite(objects[i].scrx, objects[i].scry, objects[i].sprite, i);

        if (objects[i].honorPriority)
        {
            /* handle priority tiles and tiles with masks */
            // get the upper-left coordinates to start checking for tiles
            x = (((objects[i].x>>CSF)-1)>>4)<<4;
            y = (((objects[i].y>>CSF)-1)>>4)<<4;

            // get the xsize/ysize of this sprite--round up to the nearest 16
            xsize = ((sprites[objects[i].sprite].xsize)>>4<<4);
            if (xsize != sprites[objects[i].sprite].xsize) xsize+=16;

            ysize = ((sprites[objects[i].sprite].ysize)>>4<<4);
            if (ysize != sprites[objects[i].sprite].ysize) ysize+=16;
    
            // now redraw any priority/masked tiles that we covered up
            // with the sprite
            for(ya=0;ya<=ysize;ya+=16)
            {
              for(xa=0;xa<=xsize;xa+=16)
              {
                tl = getmaptileat(x+xa,y+ya);
                if (tiles[tl].masktile)
                {
                   drawtilewithmask(x+xa-scroll_x,y+ya-scroll_y,tl,tiles[tl].masktile);
                }
                else if (tiles[tl].priority)
                {
                   if (tiles[tl].isAnimated)
                   {
                     tl = (tl-tiles[tl].animOffset)+((tiles[tl].animOffset+curanimtileframe)%tiles[tl].animmask);
                   }
                   drawprioritytile(x+xa-scroll_x,y+ya-scroll_y,tl);
                }
              }
            }
        }

      }
   }

}

void gamedo_render_drawdebug(void)
{
int tl,y;
int h;
char debugmsg[80];

   if (debugmode)
   {
      if (debugmode==1)
      {
        savew = 190;
        saveh = 80;
        save_area(4,4,savew,saveh);
        y = 5-8;
        sprintf(debugmsg, "p1x/y: %d/%d", player[0].x, player[0].y);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "p2x/y: %d/%d", player[1].x, player[1].y);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "scroll_x/y = %d/%d", (unsigned int)scroll_x, (unsigned int)scroll_y);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "scrollbuf_x/y: %d/%d", scrollx_buf, scrolly_buf);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "iw,pw: %d/%d", player[0].inhibitwalking, player[0].pwalking);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "pinertia_x: %d", player[0].pinertia_x);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "psupt: (%d,%d)", player[0].psupportingtile, player[0].psupportingobject);
        sb_font_draw(debugmsg, 5, y+=8);

        sprintf(debugmsg, "lvl,tile = %d,%d", getlevelat((player[0].x>>CSF)+4, (player[0].y>>CSF)+9), tl);
        sb_font_draw(debugmsg, 5, y+=8);

/*
        sprintf(debugmsg, "NOH=%d", NessieObjectHandle);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "x,y=(%d,%d)", objects[NessieObjectHandle].x,objects[NessieObjectHandle].y);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF=(%d,%d)", objects[NessieObjectHandle].x>>CSF,objects[NessieObjectHandle].y>>CSF);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF>>4=(%d,%d)", objects[NessieObjectHandle].x>>CSF>>4,objects[NessieObjectHandle].y>>CSF>>4);
        sb_font_draw(debugmsg, 5, y+=8);

        sprintf(debugmsg, "nessiestate = %d", objects[NessieObjectHandle].ai.nessie.state);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "pausetimer = %d", objects[NessieObjectHandle].ai.nessie.pausetimer);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "pausex/y = (%d,%d)", objects[NessieObjectHandle].ai.nessie.pausex,objects[NessieObjectHandle].ai.nessie.pausey);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "destx/y = %d/%d", objects[NessieObjectHandle].ai.nessie.destx,objects[NessieObjectHandle].ai.nessie.desty);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF = %d/%d", objects[NessieObjectHandle].ai.nessie.destx>>CSF,objects[NessieObjectHandle].ai.nessie.desty>>CSF);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, " >>CSF>>4 = %d/%d", objects[NessieObjectHandle].ai.nessie.destx>>CSF>>4,objects[NessieObjectHandle].ai.nessie.desty>>CSF>>4);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "mort_swim_amt = %d", objects[NessieObjectHandle].ai.nessie.mortimer_swim_amt);
        sb_font_draw(debugmsg, 5, y+=8);

        h = objects[NessieObjectHandle].ai.nessie.tiletrailhead;

        sprintf(debugmsg, "tthead=%d", h);
        sb_font_draw(debugmsg, 5, y+=8);
        
        sprintf(debugmsg, "ttX=%d,%d,%d,%d,%d", objects[NessieObjectHandle].ai.nessie.tiletrailX[0],objects[NessieObjectHandle].ai.nessie.tiletrailX[1],objects[NessieObjectHandle].ai.nessie.tiletrailX[2],objects[NessieObjectHandle].ai.nessie.tiletrailX[3],objects[NessieObjectHandle].ai.nessie.tiletrailX[4]);
        sb_font_draw(debugmsg, 5, y+=8);
        sprintf(debugmsg, "ttY=%d,%d,%d,%d,%d", objects[NessieObjectHandle].ai.nessie.tiletrailY[0],objects[NessieObjectHandle].ai.nessie.tiletrailY[1],objects[NessieObjectHandle].ai.nessie.tiletrailY[2],objects[NessieObjectHandle].ai.nessie.tiletrailY[3],objects[NessieObjectHandle].ai.nessie.tiletrailY[4]);
        sb_font_draw(debugmsg, 5, y+=8);
*/
      }
      else if (debugmode==2)
      {
        savew = map.xsize+4;
        saveh = map.ysize+4;
        save_area(4,4,savew,saveh);
        radar();
      }
   }
}

void gamedo_render_erasedebug(void)
{
   if (debugmode) restore_area(4,4,savew,saveh);
}

void gamedo_render_eraseobjects(void)
{
int i;

   // erase all objects.
   // note that this is done in the reverse order they are drawn.
   // this is necessary or you will see corrupted pixels when
   // two objects are occupying the same space.
   for(i=0;i<MAX_OBJECTS;i++)
   {
      if (objects[i].exists && objects[i].onscreen)
      {
          erasesprite(objects[i].scrx, objects[i].scry, objects[i].sprite, i);
      }
   }
}

extern int NumConsoleMessages;

// draws sprites, players, and debug messages (if debug mode is on),
// performs frameskipping and blits the display as needed,
// at end of functions erases all drawn objects from the scrollbuf.
void gamedo_RenderScreen(void)
{
int x,y,bmnum;

   gamedo_render_drawobjects();
   gamedo_render_drawdebug();

   if (levelcontrol.gameovermode)
   {
     // figure out where to center the gameover bitmap and draw it
     bmnum = GetBitmapNumberFromName("GAMEOVER");
     x = (320/2)-(bitmaps[bmnum].xsize/2);
     y = (200/2)-(bitmaps[bmnum].ysize/2);
     DrawBitmap(x, y, bmnum);
   }

   sb_blit();              // blit scrollbuffer to display

   gamedo_render_erasedebug();
   gamedo_render_eraseobjects();

   if (framebyframe)
   {
     while(!immediate_keytable[KF8] && !immediate_keytable[KQUIT])
     {
       if (immediate_keytable[KF7])
       {
         while(immediate_keytable[KF7] && !immediate_keytable[KQUIT])
         {
           poll_events();
         }
         framebyframe = 0;
         #ifdef BUILD_SDL
           NumConsoleMessages = 0;
         #endif
         return;
       }
       poll_events();
     }
     while(immediate_keytable[KF8] && !immediate_keytable[KQUIT])
     {
       poll_events();
     }
   }

   curfps++;
}

int ctspace=0, lastctspace=0;
void gamedo_HandleFKeys(void)
{
int i;

    if (immediate_keytable[KC] && \
        immediate_keytable[KT] && \
        immediate_keytable[KSPACE])
       {
          ctspace = 1;
       }
       else ctspace = 0;

       if (ctspace && !lastctspace)
       {
              for(i=0;i<MAX_PLAYERS;i++)
              {
                 if (player[i].isPlaying)
                 {
                    if (levelcontrol.episode!=3)
                    {
                       give_keycard(DOOR_YELLOW, i);
                       give_keycard(DOOR_RED, i);
                       give_keycard(DOOR_GREEN, i);
                       give_keycard(DOOR_BLUE, i);
                    }
                    else
                    {
                       give_keycard(DOOR_YELLOW_EP3, i);
                       give_keycard(DOOR_RED_EP3, i);
                       give_keycard(DOOR_GREEN_EP3, i);
                       give_keycard(DOOR_BLUE_EP3, i);
                    }
                    player[i].inventory.charges = 999;
                    player[i].inventory.HasPogo = 1;
                    player[i].inventory.lives = 10;
                 }
              }
         AddConsoleMsg("All items cheat");
       }

       lastctspace = ctspace;


       // GOD cheat -- toggle god mode
       if (immediate_keytable[KGOD]&&!last_immediate_keytable[KGOD])
       {
           for(i=0;i<MAX_PLAYERS;i++)
           {
              player[i].godmode ^= 1;
           }
           DeleteConsoleMsgs();
           if (player[0].godmode)
             AddConsoleMsg("God mode ON");
           else
             AddConsoleMsg("God mode OFF");

           sound_play(SOUND_GUN_CLICK, PLAY_FORCE);
       }

    if (options[OPT_CHEATS].value)
    {
            if (immediate_keytable[KTAB]) // noclip/revive
            {
              // resurrect any dead players. the rest of the KTAB magic is
              // scattered throughout the various functions.
              for(i=0;i<MAX_PLAYERS;i++)
              {
                 if (player[i].pdie)
                 {
                   player[i].pdie = PDIE_NODIE;
                   player[i].y -= (8<<CSF);                  
                 }
                 player[i].pfrozentime = 0;
              }
            }
            // F8 - frame by frame
            if(immediate_keytable[KF8]&&!last_immediate_keytable[KF8])
            {
              framebyframe = 1;
              #ifdef BUILD_SDL
                 AddConsoleMsg("Frame-by-frame mode  F8:advance F7:stop");
              #endif
            }
            // F9 - exit level immediately
            if(immediate_keytable[KF9]&&!last_immediate_keytable[KF9])
            {
               endlevel(1);
            }
            // F6 - onscreen debug--toggle through debug/radar/off
            if(immediate_keytable[KF6]&&!last_immediate_keytable[KF6])
            {
               debugmode++;
               if (debugmode>2) debugmode=0;
            }
            // F7 - accelerate mode/frame by frame frame advance
            if(immediate_keytable[KF7]&&!last_immediate_keytable[KF7])
            {
               if (!framebyframe) acceleratemode=1-acceleratemode;
            }
    }

    // F10 - change primary player
    if(immediate_keytable[KF10]&&!last_immediate_keytable[KF10])
    {
        primaryplayer++;
        if (primaryplayer>=numplayers) primaryplayer=0;
    }
    // F3 - save game
    if (immediate_keytable[KF3]&&!last_immediate_keytable[KF3])
    {
       game_save_interface();
       last_immediate_keytable[KQUIT] = 1;
    }

}

void gamedo_fades(void)
{
    if (fade.mode != FADE_GO) return;

    if (fade.fadetimer > fade.rate)
    {
      if (fade.dir==FADE_IN)
      {
        if (fade.curamt < PAL_FADE_SHADES)
        {
          fade.curamt++;                // coming in from black
        }
        else
        {
          fade.curamt--;                // coming in from white-out
        }
        if (fade.curamt==PAL_FADE_SHADES)
        {
           fade.mode = FADE_COMPLETE;
        }
        pal_fade(fade.curamt);
      }
      else if (fade.dir==FADE_OUT)
      {
        fade.curamt--;
        if (fade.curamt==0) fade.mode = FADE_COMPLETE;
        pal_fade(fade.curamt);
      }
      fade.fadetimer = 0;
    }
    else
    {
      fade.fadetimer++;
    }
}

void gamedo_frameskipping(void)
{
     if (framebyframe)
     {
       gamedo_RenderScreen();
       return;
     }

     if (frameskiptimer >= options[OPT_FRAMESKIP].value)
     {
       gamedo_RenderScreen();
       frameskiptimer = 0;
     } else frameskiptimer++;
}

// same as above but only does a sb_blit, not the full RenderScreen.
// used for intros etc.
void gamedo_frameskipping_blitonly(void)
{
static int frameskiptimer;
     if (framebyframe)
     {
       sb_blit();
       return;
     }

     if (frameskiptimer >= options[OPT_FRAMESKIP].value)
     {
       sb_blit();
       frameskiptimer = 0;
     } else frameskiptimer++;
}

