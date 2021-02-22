/* MENU.C
  The main menu, intro, and other such stuff.
*/

#include "keen.h"
#include "pressf10.h"

int IntroCanceled;

#define TWIRL_SPEED_SLOW        100
#define TWIRL_SPEED_FAST        20

#define MENUS_Y                 32
#define MAINMENU_X              32

#define FONT_TWIRL              9
#define TWIRL_SPEED             30
#define TWIRL_NUM_FRAMES        5

void showmapatpos(int level, int xoff, int yoff, int wm)
{
int i;
char levelname[80];
  VidDrv_printf("showmapatpos(%d, %d, %d, %d);\n",level,xoff,yoff,wm);
  levelcontrol.dark = 0;
  pal_init(levelcontrol.dark);

  initgame();           // reset scroll
  sprintf(levelname, "level%02d.ck%d", level, levelcontrol.episode);
  loadmap(levelname, level, wm);
  drawmap();
  for(i=0;i<xoff;i++) map_scroll_right();
  for(i=0;i<yoff;i++) map_scroll_down();
  sb_blit();
}

#define MAINMENU_GOTO_DEMO_TIME      4000

int mainmenu(int defaultopt)
{
int x,y,i;
int curselY, selopt, destselY, selmovetimer;
int twirltimer,twirlframe;
int lastdnstate, lastupstate;
int enter, lastenterstate;
int bmnum;
int menuappeartimer;
int GotoDemoTimer;
int menuvisible;
int dlgX,dlgY,dlgW,dlgH;
unsigned char lastkeytable[KEYTABLE_SIZE+1];
char lastquit;
//int vstringX;

  curselY = destselY = (defaultopt * 8);
  selmovetimer = selopt = 0;
  twirltimer = twirlframe = 0;

  fade.mode = FADE_GO;
  fade.dir = FADE_IN;
  fade.curamt = 0;
  fade.rate = FADE_NORM;
  fade.fadetimer = 0;
  gamedo_fades();
  menuappeartimer = 100;                // time to wait before menu appears

redraw: ;
  showmapatpos(90, MAINMENU_X, MENUS_Y, 0);

  AllPlayersInvisible();

  memcpy(lastkeytable, immediate_keytable, KEYTABLE_SIZE);

  bmnum = GetBitmapNumberFromName("TITLE");
  x = (320/2)-(bitmaps[bmnum].xsize/2);
  DrawBitmap(x, 0, bmnum);

  // get dialog dimensions
  dlgX = GetStringAttribute("MAIN_MENU", "LEFT");
  dlgY = GetStringAttribute("MAIN_MENU", "TOP");
  dlgW = GetStringAttribute("MAIN_MENU", "WIDTH");
  dlgH = GetStringAttribute("MAIN_MENU", "HEIGHT");
  // get location to draw version string
//  vstringX = 320 - (strlen(VERSIONSTRING)*8);

  lastupstate = lastdnstate = lastenterstate = 1;
  lastquit = 1;
  GotoDemoTimer = 0;
  menuvisible = 1;
  do
  {
    gamedo_fades();

//    sb_font_draw_inverse(VERSIONSTRING, vstringX, 200-8);

    // after a while time out and go to the demo
    if (GotoDemoTimer > MAINMENU_GOTO_DEMO_TIME)
    {
      if (fade.mode==NO_FADE)
      {
        fade.dir = FADE_OUT;
        fade.curamt = PAL_FADE_SHADES;
        fade.fadetimer = 0;
        fade.rate = FADE_NORM;
        fade.mode = FADE_GO;
      }
    }
    else GotoDemoTimer++;
   
    gamedo_AnimatedTiles();
    if (!menuappeartimer && menuvisible)
    {
      sb_dialogbox(dlgX, dlgY, dlgW, dlgH);
      sb_font_draw(getstring("MAIN_MENU"), (dlgX+1)<<3, (dlgY+1)<<3);

      sb_drawcharacter((dlgX+1)<<3, curselY+((dlgY+1)*8), FONT_TWIRL+twirlframe);
    }

    if (fade.mode==NO_FADE && menuappeartimer)
    {
      if (IntroCanceled) menuappeartimer--;
      for(i=0;i<KEYTABLE_REALKEYS_SIZE;i++)
      {
        if (immediate_keytable[i])
        {
          menuappeartimer = 0;
          lastdnstate = lastupstate = lastenterstate = 1;
        }
      }
    }

    if (immediate_keytable[KDOWN] && !lastdnstate && fade.mode==NO_FADE)
    {
      if (destselY < MAINMENU_NUM_OPTIONS*8)
      {
        destselY += 8;
        if (destselY>>3 == MAINMNU_BLANKSPOT) destselY += 8;
      }
      GotoDemoTimer = 0;
    }
    else if (immediate_keytable[KUP] && !lastupstate && fade.mode==NO_FADE)
    {
      if (destselY > 0)
      {
        destselY -= 8;
        if (destselY>>3 == MAINMNU_BLANKSPOT) destselY -= 8;
      }
      GotoDemoTimer = 0;
    }
    lastupstate = immediate_keytable[KUP];
    lastdnstate = immediate_keytable[KDOWN];

    enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]);
    if (enter && !lastenterstate && fade.mode==NO_FADE)
    {
       if ((destselY >> 3)==MAINMNU_LOADGAME)
       {
         loadslot = save_slot_box(0);
         if (loadslot)
         {
           fade.dir = FADE_OUT;
           fade.curamt = PAL_FADE_SHADES;
           fade.fadetimer = 0;
           fade.rate = FADE_NORM;
           fade.mode = FADE_GO;
           GotoDemoTimer = 0;
         }
         bmnum = GetBitmapNumberFromName("TITLE");
         x = (320/2)-(bitmaps[bmnum].xsize/2);
         DrawBitmap(x, 0, bmnum);
         menuvisible = 0;
       }
       else if ((destselY >> 3)==MAINMNU_OPTIONS)
       {
         if (configmenu())
         {    // need to restart game
            return RESTART_GAME;
         }
         else goto redraw;
       }
       else
       {
         fade.dir = FADE_OUT;
         fade.curamt = PAL_FADE_SHADES;
         fade.fadetimer = 0;
         fade.rate = FADE_NORM;
         fade.mode = FADE_GO;
         GotoDemoTimer = 0;
       }
    }
    if (fade.dir==FADE_OUT && fade.mode==FADE_COMPLETE)
    {
      if (GotoDemoTimer > MAINMENU_GOTO_DEMO_TIME)
      {
        return MAINMNU_TIMEOUT;
      }
      else
      {
        return (destselY >> 3);
      }
    }
    if (fade.dir==FADE_IN && fade.mode==FADE_COMPLETE) fade.mode=NO_FADE;
    lastenterstate = enter;

    #define SELMOVE_SPD         3
    if (selmovetimer>SELMOVE_SPD)
    {
      if (curselY < destselY) curselY++;
      if (curselY > destselY) curselY--;
      selmovetimer = 0;
    } else selmovetimer++;

    if (twirltimer>TWIRL_SPEED_SLOW ||\
       (twirltimer>TWIRL_SPEED_FAST && destselY!=curselY))
    {

      twirlframe++;
      if (twirlframe>TWIRL_NUM_FRAMES) twirlframe=0;
      twirltimer=0;
    } else twirltimer++;

    gamedo_frameskipping();

    SpeedThrottle();

    if (immediate_keytable[KQUIT] && !lastquit) return MAINMNU_QUIT;
    if (QuitState==QUIT_PROGRAM) return MAINMNU_QUIT;
    lastquit = immediate_keytable[KQUIT];

    memcpy(lastkeytable, immediate_keytable, KEYTABLE_SIZE);
  } while(!crashflag);
  return MAINMNU_QUIT;
}

char configmenu(void)
{
int x,y,i;
int curselY, selopt, destselY, selmovetimer;
int twirltimer,twirlframe;
char lastupstate, lastdnstate;
int enter, lastenterstate;
int sel;
int dlgX,dlgY,dlgW,dlgH;
int BlankSpot1, BlankSpot2, AcceptSpot, CancelSpot;
unsigned char lastkeytable[KEYTABLE_SIZE+1];
char MyOptions[NUM_OPTIONS];
char restart;
char restart_video;
//int vstringX;

  curselY = destselY = 0;
  selmovetimer = 0;
  twirltimer = twirlframe = 0;

  AllPlayersInvisible();

  dlgX = 4;
  dlgY = 6;
  dlgW = 31;
  dlgH = 6+NUM_OPTIONS;

  for(i=0;i<NUM_OPTIONS;i++) MyOptions[i] = options[i].value;

  lastupstate = lastdnstate = lastenterstate = 1;
  do
  {  
    gamedo_AnimatedTiles();

    sb_dialogbox(dlgX, dlgY, dlgW, dlgH);

    y = (dlgY+1);
    for(i=0;i<NUM_OPTIONS;i++)
    {
      sb_font_draw(options[i].name, (dlgX+3)<<3, y<<3);
      if (i==OPT_EPISODE)
      {         
          sb_drawcharacter((dlgX+dlgW-3)<<3, y<<3, MyOptions[i] + '0');
      }
      else if (i==OPT_FRAMESKIP)
      {
          x = (dlgX+dlgW-3);
          if (MyOptions[i] >= 10)
            sb_drawcharacter((x++)<<3, y<<3, '0' + (MyOptions[i]/10));
          sb_drawcharacter(x<<3, y<<3, '0' + (MyOptions[i]%10));
      }
      else if (i==OPT_ZOOM)
      {         
          sb_drawcharacter((dlgX+dlgW-3)<<3, y<<3, MyOptions[i] + '0');
          sb_drawcharacter((dlgX+dlgW-2)<<3, y<<3, 'x');
      }
      else
      {
          if (MyOptions[i])
          {
            sb_font_draw("ON", (dlgX+dlgW-3)<<3, y<<3);
          }
          else
          {
            sb_font_draw("OFF", (dlgX+dlgW-3)<<3, y<<3);
          }
      }
      y++;
    }

    BlankSpot1 = y-(dlgY+1);
    y += 2;
    BlankSpot2 = y-(dlgY+1)-1;
    sb_font_draw("Accept", (dlgX+3)<<3, y<<3);
    AcceptSpot = y-(dlgY+1);
    y++;
    sb_font_draw("Cancel", (dlgX+3)<<3, y<<3);
    CancelSpot = y-(dlgY+1);

    sb_drawcharacter((dlgX+1)<<3, curselY+((dlgY+1)*8), FONT_TWIRL+twirlframe);

    if (immediate_keytable[KDOWN] && !lastdnstate)
    {
      if (destselY < CancelSpot*8)
      {
        destselY += 8;
        if (destselY>>3 == BlankSpot1) destselY += 16;
      }
    }
    else if (immediate_keytable[KUP] && !lastupstate)
    {
      if (destselY > 0)
      {
        destselY -= 8;
        if (destselY>>3 == BlankSpot2) destselY -= 16;
      }
    }
    lastdnstate = immediate_keytable[KDOWN];
    lastupstate = immediate_keytable[KUP];

    enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]);
    if (enter && !lastenterstate)
    {
      if (curselY==destselY)
      {
        sel = destselY>>3;
        if (sel==AcceptSpot)
        {
          // apply the changes and return
          restart = 0;
          if (MyOptions[OPT_EPISODE] != options[OPT_EPISODE].value) restart = 1;

          // do we need to restart video driver?
          restart_video = 0;
          if (MyOptions[OPT_FULLSCREEN] != options[OPT_FULLSCREEN].value) restart_video = 1;
          if (MyOptions[OPT_ZOOM] != options[OPT_ZOOM].value) restart_video = 1;

          for(i=0;i<NUM_OPTIONS;i++) options[i].value = MyOptions[i];

          levelcontrol.episode = options[OPT_EPISODE].value;          

          if (restart_video)
          {
             VidDrv_printf("changing video settings via options menu: fs=%d zoom=%d\n", options[OPT_FULLSCREEN].value, options[OPT_ZOOM].value);  
             VidDrv_reset();
          }
          return restart;
        }
        else if (sel==CancelSpot)
        {
          return 0;
        }
        else if (sel==OPT_EPISODE)
        {
          MyOptions[sel]++;
          if (MyOptions[sel] > 3) MyOptions[sel] = 1;
        }
        else if (sel==OPT_FRAMESKIP)
        {
          MyOptions[sel]++;
          if (MyOptions[sel] > 20) MyOptions[sel] = 0;
        }
        else if (sel==OPT_ZOOM)
        {
          if (MyOptions[sel]==2) MyOptions[sel]=1; else MyOptions[sel]=2;
        }
        else
        {
          MyOptions[sel] ^= 1;
        }
      }
    }
    lastenterstate = enter;

    if (selmovetimer > SELMOVE_SPD/2)
    {
      if (curselY < destselY) curselY++;
      if (curselY > destselY) curselY--;
      selmovetimer = 0;
    } else selmovetimer++;

    if (twirltimer>TWIRL_SPEED_SLOW ||\
       (twirltimer>TWIRL_SPEED_FAST && destselY!=curselY))
    {

      twirlframe++;
      if (twirlframe>TWIRL_NUM_FRAMES) twirlframe=0;
      twirltimer=0;
    } else twirltimer++;

    gamedo_frameskipping();

    SpeedThrottle();
    if (immediate_keytable[KQUIT]) break;
    memcpy(lastkeytable, immediate_keytable, KEYTABLE_SIZE);
  } while(!crashflag);
  return;
}

extern char fade_black;
int intro(void)
{
int x,y,i;
int xb,yb;

int curPage, changedPage;
int textVisible;
char stStringName[40];
char *stStringData;
char stTextLine1[80];
char stTextLine2[80];
char stTextLine3[80];
char SlowPrintBuffer[80];
int x1,x2,x3,x4;
int y1,y2,y3,y4;
char *copyPtr, copyIndex;

int ontime, offtime, textTimer;
int y1adj, y2adj, y3adj;
int lastpage;
int SlowPrint, SlowPrintTimer, SlowPrintIndex;
int SlowPrintSpeed;
char keypressed;
char lastkeypressed;
char *SlowPrintPtr;
                          
char *ScrollText1      = "Presenting";
char *ScrollTextErase  = "               ";
char *ScrollText2      = "CloneKeen";
char *ScrollText4      = "by Caitlin Shaw";
char ScrollText3[20];
char ScrollTextTimer;
int STimer;
char SState;
char ShowPressF10;
#define PRESSF10_X      (160 - (PRESSF10_WIDTH/2))
#define PRESSF10_Y      3
//#define PRESSF10_X      (315 - PRESSF10_WIDTH)
//#define PRESSF10_Y      (195 - PRESSF10_HEIGHT)

  if (options[OPT_FULLSCREEN].value && options[OPT_ZOOM].value>1)
    ShowPressF10 = 0;           // 0 = mask hide all
  else
    ShowPressF10 = 15;          // 15 = mask show all

  // display the starry background and initiate a fade-in
  showmapatpos(90, 104<<4, 32, 0);
  fade.mode = FADE_GO;
  fade.rate = FADE_NORM;
  fade.dir = FADE_IN;
  fade.curamt = 0;
  fade.fadetimer = 0;

  #define SCROLL_STOP_Y          75
  #define F10_STOP_Y             210

  /* move "Presenting" up the screen */
  fade_black = 1;
  sprintf(ScrollText3, "Episode %d", levelcontrol.episode);
  x1 = (320/2) - ((strlen(ScrollText1)*8)/2);
  y1 = 235;
  x2 = (320/2) - ((strlen(ScrollText2)*8)/2);
  y2 = SCROLL_STOP_Y+16;
  x3 = (320/2) - ((strlen(ScrollText3)*8)/2);
  y3 = y2+24;
  x4 = (320/2) - ((strlen(ScrollText4)*8)/2);
  y4 = y3+16;
  SState = 0;
  STimer = 0;
  ScrollTextTimer = 0;

  lastkeypressed = 1;
  do
  {
    // do fades
    gamedo_fades();
    gamedo_AnimatedTiles();  

    if (fade.dir != FADE_OUT)
    {
       // if user pressed a key cancel the intro
       keypressed = 0;
       for(i=0;i<KEYTABLE_REALKEYS_SIZE;i++)
       {
          if (immediate_keytable[i])
          {
             keypressed = i;
             break;
          }
       }
       if (keypressed && !lastkeypressed)
       {
             if (keypressed == KF10)
             {
                if (options[OPT_ZOOM].value != 2 || options[OPT_FULLSCREEN].value != 1)
                {
                   options[OPT_FULLSCREEN].value = 1;
                   options[OPT_ZOOM].value = 2;
                   VidDrv_reset();
                   map_redraw();
                }
             }
             else
             {
                fade.dir = FADE_OUT;
                fade.curamt = PAL_FADE_SHADES;
                fade.fadetimer = 0;
                fade.rate = FADE_NORM;
                fade.mode = FADE_GO;
                IntroCanceled = 1;
             }
       }
       if (keypressed) lastkeypressed = 1; else lastkeypressed = 0;

    }  // end if(fade.dir!=FADE_OUT)

    sb_font_draw_inverse(ScrollText1, x1, y1);

    if (SState >= 1)
    {
      sb_font_draw_inverse(ScrollText2, x2, y2);
      if (SState==1)
      {
        if (STimer > 200)
        {
          SState = 2;
          STimer = 0;
        }
        else STimer++;
      }
    }

    if (SState >= 2)
    {
      sb_font_draw_inverse(ScrollText3, x3, y3);
      if (SState==2)
      {
         if (STimer > 200)
         {
            SState = 3;
            STimer = 0;
         }
         else STimer++;
      }
    }

    if (SState >= 3)
    {
      sb_font_draw_inverse(ScrollText4, x4, y4);
      if (STimer > 1000)
      {
        fade_black = 0;
        break;
      }
      else STimer++;
    }

    // if Press F10 message is still visible, display it
    if (ShowPressF10)
    {
      if (y1 <= F10_STOP_Y) ShowPressF10 = 0;
      if (options[OPT_ZOOM].value==2 && options[OPT_FULLSCREEN].value==1) ShowPressF10 = 0;

      i = 0;
      for(y=0;y<PRESSF10_HEIGHT;y++)
      {
        yb = ((PRESSF10_Y+y+scrolly_buf)&511)<<9;
        for(x=0;x<PRESSF10_WIDTH;x++)
        {
          scrollbuf[yb+((PRESSF10_X+x+scrollx_buf)&511)] = (pressf10_image[i++] & ShowPressF10);
        }
      }
    }

    // blit the scrollbuffer to the display
    gamedo_frameskipping_blitonly();

    if (SState==0)
    {
       sb_font_draw_inverse(ScrollTextErase, x1, y1);

       if (y1 > SCROLL_STOP_Y)
       {
          if (ScrollTextTimer > 10)
          {
            y1--;
            ScrollTextTimer = 0;
          }
          else ScrollTextTimer++;
       }
       else
       {
          if (STimer > 200)
          {
            SState = 1;
            STimer = 0;
          }
          else STimer++;
       }
    }

    // when fade is complete we're done!
    if (fade.dir==FADE_OUT && fade.mode==FADE_COMPLETE)
    {
      fade_black = 0;
      return 0;
    }

    SpeedThrottle();
    if (immediate_keytable[KQUIT] || crashflag)
    {
       fade_black = 0;
       return 1;
    }
  } while(1);

  sb_font_draw_inverse(ScrollTextErase, x1, y1);
  sb_font_draw_inverse(ScrollTextErase, x2, y2);
  sb_font_draw_inverse(ScrollTextErase, x3, y3);
  sb_font_draw_inverse(ScrollTextErase, x4, y4);

  /* the appearing/disapearing text */
  curPage = 1;
  changedPage = 1;
  do
  {
    // do fades
    gamedo_fades();
    gamedo_AnimatedTiles();

       // need to load a new page from the strings file?
       if (changedPage)
       {
          // load the data for the new page
          sprintf(stStringName, "Tribute_Page_%d", curPage);
          stStringData = getstring(stStringName);
          ontime = GetStringAttribute(stStringName, "ONTIME");
          offtime = GetStringAttribute(stStringName, "OFFTIME");
          y1adj = GetStringAttribute(stStringName, "Y1ADJ");
          y2adj = GetStringAttribute(stStringName, "Y2ADJ");
          y3adj = GetStringAttribute(stStringName, "Y3ADJ");
          lastpage = GetStringAttribute(stStringName, "LASTPAGE");
          SlowPrint = GetStringAttribute(stStringName, "SLOWPRINT");

          // if no SLOWPRINT attribute disable slowprinting
          if (SlowPrint==-1)
          {
            SlowPrint = 0;
          }
          else
          {
            SlowPrintSpeed = GetStringAttribute(stStringName, "SLOWPRINTSPD");
          }

          // for y adjustments that weren't specified use 0
          if (y1adj==-1) y1adj = 0;
          if (y2adj==-1) y2adj = 0;
          if (y3adj==-1) y3adj = 0;

          stTextLine1[0] = stTextLine2[0] = stTextLine3[0] = 0;

          // we have the text as three CR-terminated lines, now split it
          // up into the 3 buffers
          copyIndex = 0;
          copyPtr = stTextLine1;
          for(i=0;i<strlen(stStringData);i++)
          {
            if (stStringData[i] != 13)
            {
              *copyPtr = stStringData[i];
              copyPtr++;
            }
            else
            {  // hit a CR
              // null-terminate
              *copyPtr = 0;
              // start copying to next buffer
              if (copyIndex==0) copyPtr = stTextLine2;
              else if (copyIndex==1) copyPtr = stTextLine3;
              else if (copyIndex==2) break;
              copyIndex++;
            }
          }
          *copyPtr = 0;    // null-terminate

          // figure out what X position these lines to be drawn in order
          // to be centered.
          x1 = (320/2)-((strlen(stTextLine1)*8)/2);
          x2 = (320/2)-((strlen(stTextLine2)*8)/2);
          x3 = (320/2)-((strlen(stTextLine3)*8)/2);
          // figure out their Y positions
          y1 = 90 + y1adj;
          y2 = 98 + y2adj;
          y3 = 106 + y3adj;

          // if we're going to do slowprinting then copy the line we're going
          // to slowprint into the slowprint buffer and fill it with spaces
          if (SlowPrint)
          {
            // set up a pointer to the line we're going to slowprint
            if (SlowPrint==1) SlowPrintPtr = &stTextLine1[0];
            else if (SlowPrint==2) SlowPrintPtr = &stTextLine2[0];
            else SlowPrintPtr = &stTextLine3[0];
   
            // copy the text line into the slow print buffer.
            // replace '@''s with the episode number
            for(i=0;i<strlen(SlowPrintBuffer)+1;i++)
            {
              if (SlowPrintPtr[i]=='@')
              {
                SlowPrintBuffer[i] = levelcontrol.episode + '0';
              }
              else
              {
                SlowPrintBuffer[i] = SlowPrintPtr[i];
              }
            }

            // clear out the text line
            for(i=0;i<strlen(SlowPrintPtr);i++) SlowPrintPtr[i] = ' ';
          }

          // set up some variables
          textVisible = 1;
          textTimer = ontime;
          changedPage = 0;
          SlowPrintTimer = 0;
          SlowPrintIndex = 0;
       }

       // handle slowprinting
       if (SlowPrint)
       {
          if (SlowPrintTimer > SlowPrintSpeed)
          {  // time to print the next character of the line we're slowprinting
             if (SlowPrintBuffer[SlowPrintIndex])
             {
               SlowPrintPtr[SlowPrintIndex] = SlowPrintBuffer[SlowPrintIndex];
               SlowPrintIndex++;
             }
             else
             { // reached the NULL (slowprint complete)
               SlowPrint = 0;
             }
   
             SlowPrintTimer = 0;
          }
          else SlowPrintTimer++;
       }

    if (fade.dir != FADE_OUT)
    {
       if (!textTimer)
       {  // time to either invisibilize the text, or go to the next page
         if (textVisible)
         {
           // erase the text
           for(i=0;i<strlen(stTextLine1);i++) stTextLine1[i] = ' ';
           for(i=0;i<strlen(stTextLine2);i++) stTextLine2[i] = ' ';
           for(i=0;i<strlen(stTextLine3);i++) stTextLine3[i] = ' ';
           textVisible = 0;
           textTimer = offtime;
         }
         else
         {
           // time for more text
           if (lastpage != 1)
           {
             curPage++;
             changedPage = 1;
           }
           else
           {  // reached last page...initiate fadeout
             fade.dir = FADE_OUT;
             fade.curamt = PAL_FADE_SHADES;
             fade.fadetimer = 0;
             fade.rate = FADE_NORM;
             fade.mode = FADE_GO;
             IntroCanceled = 0;
           }
         }
       }
       else textTimer--;

       // if user pressed a key cancel the intro
       keypressed = 0;
       for(i=0;i<KEYTABLE_REALKEYS_SIZE;i++)
       {
          if (immediate_keytable[i])
          {
             keypressed = 1;
             break;
          }
       }
       if (keypressed && !lastkeypressed)
       {
             fade.dir = FADE_OUT;
             fade.curamt = PAL_FADE_SHADES;
             fade.fadetimer = 0;
             fade.rate = FADE_NORM;
             fade.mode = FADE_GO;
             IntroCanceled = 1;
       }
       lastkeypressed = keypressed;


    }  // end if(fade.dir!=FADE_OUT)
  
    // draw/erase all three lines of text
    sb_font_draw_inverse(stTextLine1, x1, y1);
    sb_font_draw_inverse(stTextLine2, x2, y2);
    sb_font_draw_inverse(stTextLine3, x3, y3);

    // blit the scrollbuffer to the display
    gamedo_frameskipping_blitonly();

    // when fade is complete we're done!
    if (fade.dir==FADE_OUT && fade.mode==FADE_COMPLETE)
    {
      return 0;
    }

    SpeedThrottle();
  } while(!immediate_keytable[KQUIT] && !crashflag);
  return 1;
}

void keensleft(void)
{
int enter, lastenterstate;
int x,y,i,p;
int boxY, boxH;
int boxtimer;
int ep3;

  // on episode 3 we have to subtract one from the map tiles
  // because the tiles start at 31, not 32 like on the other eps
  ep3 = 0;
  if (levelcontrol.episode==3) ep3 = 1;

  #define KEENSLEFT_TIME        400

  for(i=0;i<MAX_PLAYERS;i++)
  {
    if (player[i].isPlaying)
    {
      gamepdo_wm_SelectFrame(i);
      player[i].hideplayer = 0;
    }
  }
  gamedo_RenderScreen();

  #define KEENSLEFT_X        7
  #define KEENSLEFT_Y        11
  #define KEENSLEFT_W        24
  #define KEENSLEFT_H        4

  boxY = KEENSLEFT_Y - (numplayers);
  boxH = KEENSLEFT_H + (numplayers * 2);

  dialogbox(KEENSLEFT_X,boxY,KEENSLEFT_W,boxH);
  font_draw(getstring("LIVES_LEFT_BACKGROUND"),(KEENSLEFT_X+1)*8,(boxY+1)*8,0);
  font_draw(getstring("LIVES_LEFT"),((KEENSLEFT_X+7)*8)+4,(boxY+1)*8,0);
  y = ((boxY+2)*8)+4;
  if (numplayers>1) y--;
  for(p=0;p<numplayers;p++)
  {
    x = ((KEENSLEFT_X+1)*8)+4;
    for(i=0;i<player[p].inventory.lives&&i<=10;i++)
    {
      drawsprite_direct(x, y, PMAPDOWNFRAME+playerbaseframes[p]-ep3);
      x+=16;
    }
    y+=18;
  }
  update_screen();

  sound_play(SOUND_KEENSLEFT, PLAY_NOW);

  boxtimer = 0;
  do
  {
   
    gamedo_fades();

    if (boxtimer > KEENSLEFT_TIME)
    {
      break;
    } else boxtimer++;

    enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]||immediate_keytable[KSPACE]||immediate_keytable[KLEFT]||immediate_keytable[KRIGHT]||immediate_keytable[KUP]||immediate_keytable[KDOWN]);
    if (enter&&!lastenterstate)
    {
      break;
    }
    if (immediate_keytable[KQUIT])
    {
      return;
    }

    lastenterstate = enter;
    SpeedThrottle();
  } while(!crashflag);

}
