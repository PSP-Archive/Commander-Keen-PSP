/* MISC.C
  All KINDS of assorted crap :) Has most of the in-game dialog boxes
  such as the status box etc.

  Also like I said there's all kinds of assorted crap in here.
  That's why it's called "misc.c" (get it? :))
*/

#include "keen.h"

void banner(void)
{
char buf[80];

  sprintf(buf, "%s  *Unknown* build check banner()", REVISION);
  #ifdef TARGET_WIN32
    sprintf(buf, "%s  Win32 build", REVISION);
  #endif
  #ifdef TARGET_LNX
    sprintf(buf, "%s  UNIX build", REVISION);
  #endif
  VidDrv_printf("%s", buf);

  VidDrv_printf("\nby Caitlin Shaw, 2003-2005\n");
  VidDrv_printf("\n");
  VidDrv_printf("BY A FAN, FOR FANS. ALL \"COMMANDER KEEN\" GRAPHICS,\n");
  VidDrv_printf("SOUND, AND LEVEL FILES ARE THE PROPERTY OF ID SOFTWARE.\n");
}

void cleanup(void)
{
int i,c;

  Graphics_Stop();
  VidDrv_printf("  * Graphics driver shut down.\n");

  freemem();
  if (BitmapData)
  {
     free(BitmapData);
     VidDrv_printf("  * Bitmap data released to system.\n");
  }
  VidDrv_printf("  * Freed %d strings.\n", freestrings());

  KeyDrv_Stop();
  VidDrv_printf("  * Keyboard driver shut down.\n");
  TimeDrv_Stop();
  VidDrv_printf("  * Timer driver shut down.\n");
  SoundDrv_Stop();
  VidDrv_printf("  * Sound driver shut down.\n");

  #ifdef NETWORK_PLAY
      if (is_server)
      {
        NetDrv_Server_Stop();
        VidDrv_printf("  * Network (server) shut down.\n");
      }
      if (is_client)
      {
        NetDrv_Client_Stop();
        VidDrv_printf("  * Network (client) shut down.\n");
      }
  #endif

  if (demofile)
  {
    fclose(demofile);
    VidDrv_printf("  * Demo file closed.\n");
  }

  #ifdef BUILD_SDL
    SDL_Quit();
    VidDrv_printf("  * SDL shut down.\n");
  #endif
  VidDrv_printf("\n");
}

// draw an empty dialog box, for youseeinyourmind(), etc.
void dialogbox(int x1, int y1, int w, int h)
{
int x,y,i,j;

  drawcharacter(x1*8, y1*8, 1);
  drawcharacter((x1+w)*8, y1*8, 3);
  for(x=(x1*8)+8,i=0;i<w-1;i++)
  {
    drawcharacter(x, y1*8, 2);
    x+=8;
  }
  y=(y1+1)*8;
  for(j=0;j<h-2;j++)
  {
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) drawcharacter(x, y, 4);
      else if (i==w) drawcharacter(x, y, 5);
      else drawcharacter(x, y, ' ');
      x+=8;
    }
    y+=8;
  }
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) drawcharacter(x, y, 6);
      else if (i==w) drawcharacter(x, y, 8);
      else drawcharacter(x, y, 7);
      x+=8;
    }
}
// draw an empty dialog box, for youseeinyourmind(), etc.
void sb_dialogbox(int x1, int y1, int w, int h)
{
int x,y,i,j;

  sb_drawcharacter(x1*8, y1*8, 1);
  sb_drawcharacter((x1+w)*8, y1*8, 3);
  for(x=(x1*8)+8,i=0;i<w-1;i++)
  {
    sb_drawcharacter(x, y1*8, 2);
    x+=8;
  }
  y=(y1+1)*8;
  for(j=0;j<h-2;j++)
  {
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) sb_drawcharacter(x, y, 4);
      else if (i==w) sb_drawcharacter(x, y, 5);
      else sb_drawcharacter(x, y, ' ');
      x+=8;
    }
    y+=8;
  }
    for(x=(x1*8),i=0;i<=w;i++)
    {
      if (i==0) sb_drawcharacter(x, y, 6);
      else if (i==w) sb_drawcharacter(x, y, 8);
      else sb_drawcharacter(x, y, 7);
      x+=8;
    }
}

#define YORPSTATUEHEAD     486
#define YORPSTATUEHEADUSED 485
void youseeinyourmind(int mpx, int mpy, int isgarg)
{
int twirlframe, twirltimer;
char strname[80];
int dlgX,dlgY,dlgW,dlgH,twirlX,twirlY;

    #define TWIRL_SPEED        30

    sound_pause();

    // get the name of the string we need to display
    sprintf(strname, "EP1_YSIYM_LVL%d", levelcontrol.curlevel);

    dlgX = GetStringAttribute(strname, "LEFT");
    dlgY = GetStringAttribute(strname, "TOP");
    dlgW = GetStringAttribute(strname, "WIDTH");
    dlgH = GetStringAttribute(strname, "HEIGHT");
    twirlX = GetStringAttribute(strname, "TWIRLX");
    twirlY = GetStringAttribute(strname, "TWIRLY");

    dialogbox(dlgX,dlgY,dlgW,dlgH);
    font_draw(getstring(strname), (dlgX+1)<<3, (dlgY+1)<<3,0);

    twirlframe = 0;
    twirltimer = TWIRL_SPEED+1;
    // wait for enter
    do
    {
      if (twirltimer>TWIRL_SPEED)
      {
        drawcharacter((dlgX+twirlX)<<3, (dlgY+twirlY)<<3, 9+twirlframe);
        update_screen();
        twirlframe++;
        if (twirlframe>5) twirlframe=0;
        twirltimer=0;
      } else twirltimer++;
      SpeedThrottle();
      update_screen();
    } while(!immediate_keytable[KENTER] && !immediate_keytable[KQUIT]);

    // make the statue head stop glowing
    if (!isgarg)
    {
      map_chgtile(mpx, mpy, YORPSTATUEHEADUSED);
      map_deanimate(mpx, mpy);
    }
    else
    { // it's a garg statue
      map_chgtile(mpx+1, mpy, 434);
      map_deanimate(mpx+1, mpy);
    }

    sound_resume();
}

void VorticonElder(int mpx, int mpy)
{
int twirlframe, twirltimer;
int dlgX,dlgY,dlgW,dlgH,twirlX,twirlY;
char *strName;

    #define TWIRL_SPEED        30

    sound_pause();

    switch(levelcontrol.curlevel)
    {
    case 8:
      strName = "EP2_VE_NOJUMPINDARK";
      break;
    case 10:
      strName = "EP2_VE_EVILBELTS";
      break;

    default:
      crashflag = 1;
      why_term_ptr = "VE box: Illegal level #.";
      break;
    }

    dlgX = GetStringAttribute(strName, "LEFT");
    dlgY = GetStringAttribute(strName, "TOP");
    dlgW = GetStringAttribute(strName, "WIDTH");
    dlgH = GetStringAttribute(strName, "HEIGHT");
    twirlX = GetStringAttribute(strName, "TWIRLX");
    twirlY = GetStringAttribute(strName, "TWIRLY");

    dialogbox(dlgX, dlgY, dlgW, dlgH);
    font_draw(getstring(strName), (dlgX+1)<<3, (dlgY+1)<<3,0);

    twirlframe = 0;
    twirltimer = TWIRL_SPEED+1;
  // wait for enter
    do
    {
      if (twirltimer>TWIRL_SPEED)
      {
        drawcharacter((dlgX+twirlX)<<3, (dlgY+twirlY)<<3, 9+twirlframe);
        update_screen();
        twirlframe++;
        if (twirlframe>5) twirlframe=0;
        twirltimer=0;
      } else twirltimer++;
      SpeedThrottle();
      update_screen();
    } while(!immediate_keytable[KENTER] && !immediate_keytable[KQUIT]);

    // make the switch stop glowing
    map_chgtile(mpx, mpy, 432);
    tiles[432].isAnimated = 0;
    map_deanimate(mpx, mpy);

    sound_resume();
}


void inventory_draw_ep1(int p)
{
int x,y,t,i,j;
char tempbuf[40];
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("EP1_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP1_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP1_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP1_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  font_draw(getstring("EP1_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0); 

// fill in what we have
  // 321: joystick/battery/vacuum/fuel not gotten
  // 414: raygun, 415, pogo
  // 424: yellow/red/green/blue cards
  // 448: ship parts, gotten
  // raygun icon
  drawtile_direct((dlgX+4)<<3, ((dlgY+8)<<3)+3, 414);
  // pogo
  if (player[p].inventory.HasPogo) drawtile_direct(((dlgX+12)<<3)+4, ((dlgY+9)<<3)+3, 415);
  // cards
  if (player[p].inventory.HasCardYellow) drawtile_direct((dlgX+21)<<3, ((dlgY+8)<<3)+3, 424);
  if (player[p].inventory.HasCardRed) drawtile_direct((dlgX+25)<<3, ((dlgY+8)<<3)+3, 425);
  if (player[p].inventory.HasCardGreen) drawtile_direct((dlgX+21)<<3, ((dlgY+10)<<3)+4, 426);
  if (player[p].inventory.HasCardBlue) drawtile_direct((dlgX+25)<<3, ((dlgY+10)<<3)+4, 427);
  // ship parts
  if (player[p].inventory.HasJoystick) t=448; else t=321;
  drawtile_direct((dlgX+18)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasBattery) t=449; else t=322;
  drawtile_direct((dlgX+21)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasVacuum) t=450; else t=323;
  drawtile_direct((dlgX+24)<<3, ((dlgY+4)<<3)+3, t);
  if (player[p].inventory.HasFuel) t=451; else t=324;
  drawtile_direct((dlgX+27)<<3, ((dlgY+4)<<3)+3, t);
  // ray gun charges
  i = player[p].inventory.charges;
  if (i>999) i=999;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+4)<<3, (dlgY+12)<<3, 0);

  // score
  i = player[p].inventory.score;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+12-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // extra life at
  i = player[p].inventory.extralifeat;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+28-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // lives
  i = player[p].inventory.lives;
  x = ((dlgX+1)<<3)+4;
  if (i>7) i=7;
  for(j=0;j<i;j++)
  {
    drawsprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
    x += sprites[0].xsize;
  }
}

void inventory_draw_ep2(int p)
{
int x,y,t,i,j;
char tempbuf[40];
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("EP2_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP2_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP2_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP2_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  font_draw(getstring("EP2_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0); 

  // cards
  if (player[p].inventory.HasCardYellow) drawtile_direct(((dlgX+21)<<3)-4, ((dlgY+8)<<3)+3, 424);
  if (player[p].inventory.HasCardRed) drawtile_direct(((dlgX+25)<<3)-4, ((dlgY+8)<<3)+3, 425);
  if (player[p].inventory.HasCardGreen) drawtile_direct(((dlgX+21)<<3)-4, ((dlgY+10)<<3)+4, 426);
  if (player[p].inventory.HasCardBlue) drawtile_direct(((dlgX+25)<<3)-4, ((dlgY+10)<<3)+4, 427);
  // cities saved
  if (levelcontrol.levels_completed[4]) font_draw(getstring("EP2_LVL4_TargetName"), (dlgX+1)<<3, (dlgY+8)<<3, 0);
  if (levelcontrol.levels_completed[6]) font_draw(getstring("EP2_LVL6_TargetName"), (dlgX+8)<<3, (dlgY+8)<<3, 0);
  if (levelcontrol.levels_completed[7]) font_draw(getstring("EP2_LVL7_TargetName"), (dlgX+1)<<3, (dlgY+9)<<3, 0);
  if (levelcontrol.levels_completed[13]) font_draw(getstring("EP2_LVL13_TargetName"), (dlgX+8)<<3, (dlgY+9)<<3, 0);
  if (levelcontrol.levels_completed[11]) font_draw(getstring("EP2_LVL11_TargetName"), (dlgX+1)<<3, (dlgY+10)<<3, 0);
  if (levelcontrol.levels_completed[9]) font_draw(getstring("EP2_LVL9_TargetName"), (dlgX+8)<<3, (dlgY+10)<<3, 0);
  if (levelcontrol.levels_completed[15]) font_draw(getstring("EP2_LVL15_TargetName"), (dlgX+1)<<3, (dlgY+11)<<3, 0);
  if (levelcontrol.levels_completed[16]) font_draw(getstring("EP2_LVL16_TargetName"), (dlgX+8)<<3, (dlgY+11)<<3, 0);

  // raygun icon
  drawtile_direct((dlgX+20)<<3, ((dlgY+5)<<3)-5, 414);

  // ray gun charges text
  i = player[p].inventory.charges;
  if (i>999) i=999;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+27-strlen(tempbuf))<<3, ((dlgY+5)<<3)-1, 0);

  // score
  i = player[p].inventory.score;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+12-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // extra life at
  i = player[p].inventory.extralifeat;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+28-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // lives
  i = player[p].inventory.lives;
  x = ((dlgX + 1)<<3)+4;
  if (i>7) i=7;
  for(j=0;j<i;j++)
  {
    drawsprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
    x += sprites[0].xsize;
  }

}

void inventory_draw_ep3(int p)
{
int x,y,t,i,j;
int ankhtimepercent;
char tempbuf[40];
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("EP3_StatusBox", "LEFT");
  dlgY = GetStringAttribute("EP3_StatusBox", "TOP");
  dlgW = GetStringAttribute("EP3_StatusBox", "WIDTH");
  dlgH = GetStringAttribute("EP3_StatusBox", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  font_draw(getstring("EP3_StatusBox"), (dlgX+1)<<3, (dlgY+1)<<3, 0); 

  // calculate % ankh time left
  ankhtimepercent = (int)((float)player[p].ankhtime / (PLAY_ANKH_TIME/100));
  // ankh time
  drawtile_direct((dlgX+4)<<3, ((dlgY+8)<<3)+3, 214);
  sprintf(tempbuf, "%d", ankhtimepercent);
  font_draw(tempbuf, (dlgX+8)<<3, ((dlgY+8)<<3)+7, 0);

  // raygun icon
  drawtile_direct((dlgX+23)<<3, ((dlgY+5)<<3)-5, 216);

  // ray gun charges text
  i = player[p].inventory.charges;
  if (i>999) i=999;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+26)<<3, ((dlgY+5)<<3)-1, 0);

  // cards
  if (player[p].inventory.HasCardYellow) drawtile_direct(((dlgX+14)<<3)+4, ((dlgY+8)<<3)+4, 217);
  if (player[p].inventory.HasCardRed) drawtile_direct(((dlgX+18)<<3)+4, ((dlgY+8)<<3)+4, 218);
  if (player[p].inventory.HasCardGreen) drawtile_direct(((dlgX+22)<<3)+4, ((dlgY+8)<<3)+4, 219);
  if (player[p].inventory.HasCardBlue) drawtile_direct(((dlgX+26)<<3)+4, ((dlgY+8)<<3)+4, 220);

  // score
  i = player[p].inventory.score;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+12-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // extra life at
  i = player[p].inventory.extralifeat;
  sprintf(tempbuf, "%d", i);
  font_draw(tempbuf, (dlgX+28-strlen(tempbuf))<<3, (dlgY+2)<<3, 0);
  // lives
  i = player[p].inventory.lives;
  x = ((dlgX+1)<<3)+4;
  if (i>9) i=9;
  for(j=0;j<i;j++)
  {
    drawsprite_direct(x, (dlgY+4)<<3, playerbaseframes[p]);
    x += sprites[0].xsize;
  }
}

void showinventory(int p)
{
int x,y,t,i,j;
char tempbuf[40];

  sound_pause();

  // draw the episode-specific stuff
  if (levelcontrol.episode==1)
  {
    inventory_draw_ep1(p);
  }
  else if (levelcontrol.episode==2)
  {
    inventory_draw_ep2(p);
  }
  else if (levelcontrol.episode==3)
  {
    inventory_draw_ep3(p);
  }


  update_screen();

  // wait for space released then pressed again
  while(immediate_keytable[KSPACE])
  {
    poll_events();

    if (immediate_keytable[KQUIT])
    {
      return;
    }
  }
  while(!immediate_keytable[KSPACE])
  {
    poll_events();

    if (immediate_keytable[KQUIT])
    {
      return;
    }
  }
  sound_resume();
}

void sshot(char *visiblefile, char *scrollfile)
{
FILE *fp;
int x,y;

  fp = fopen(visiblefile, "wb");
  if (!fp) return;

  for(y=0;y<200;y++)
   for(x=0;x<320;x++)
    fputc(getpixel(x,y), fp);

  fclose(fp);

  fp = fopen(scrollfile, "wb");
  if (!fp) return;

  for(y=0;y<512;y++)
   for(x=0;x<512;x++)
    fputc(sb_getpixel(x,y), fp);

  fclose(fp);
}

void YourShipNeedsTheseParts(void)
{
int cp;
int dlgX,dlgY,dlgW,dlgH;

  cp = 0;               //FIXME
  sound_pause();

  dlgX = GetStringAttribute("EP1_SHIP", "LEFT");
  dlgY = GetStringAttribute("EP1_SHIP", "TOP");
  dlgW = GetStringAttribute("EP1_SHIP", "WIDTH");
  dlgH = GetStringAttribute("EP1_SHIP", "HEIGHT");

  dialogbox(dlgX,dlgY,dlgW,dlgH);

  font_draw(getstring("EP1_SHIP"), (dlgX+1)<<3, (dlgY+1)<<3,0);

  // draw needed parts
  if (!player[cp].inventory.HasJoystick)
    drawtile_direct((dlgX+9)<<3, (dlgY+3)<<3, 448);

  if (!player[cp].inventory.HasBattery)
    drawtile_direct((dlgX+12)<<3, (dlgY+3)<<3, 449);

  if (!player[cp].inventory.HasVacuum)
    drawtile_direct((dlgX+15)<<3, (dlgY+3)<<3, 450);

  if (!player[cp].inventory.HasFuel)
    drawtile_direct((dlgX+18)<<3, (dlgY+3)<<3, 451);

  update_screen();

  // wait for ctrl/space/enter released then pressed again then released
  while(immediate_keytable[KSPACE] || immediate_keytable[KCTRL] || immediate_keytable[KENTER])
  {
    poll_events();

    if (immediate_keytable[KQUIT])
    {
      crashflag = 1;
      why_term_ptr = "ESC pressed.";
      return;
    }
  }
  while(!immediate_keytable[KSPACE] && !immediate_keytable[KCTRL] && !immediate_keytable[KENTER]) {
    poll_events();

    if (immediate_keytable[KQUIT])
    {
      crashflag = 1;
      why_term_ptr = "ESC pressed.";
      return;
    }
  }
  while(immediate_keytable[KSPACE] || immediate_keytable[KCTRL] || immediate_keytable[KENTER]) {
    poll_events();

    if (immediate_keytable[KQUIT])
    {
      crashflag = 1;
      why_term_ptr = "ESC pressed.";
      return;
    }
  }
  sound_resume();
}

void ShipEp3(void)
{
int t;
char strname[80];
int twirlframe, twirltimer;
int lastenter;
int dlgX,dlgY,dlgW,dlgH,twirlX,twirlY;

  sound_pause();

  // display one of four random strings
  sprintf(strname, "EP3_SHIP%d", (rand()%4)+1);

  dlgX = GetStringAttribute(strname, "LEFT");
  dlgY = GetStringAttribute(strname, "TOP");
  dlgW = GetStringAttribute(strname, "WIDTH");
  dlgH = GetStringAttribute(strname, "HEIGHT");
  twirlX = GetStringAttribute(strname, "TWIRLX");
  twirlY = GetStringAttribute(strname, "TWIRLY");

  dialogbox(dlgX,dlgY,dlgW,dlgH);
  font_draw(getstring(strname), (dlgX+1)<<3, (dlgY+1)<<3,0);

  update_screen();

    // wait for ctrl/space/enter released then pressed again then released
    twirlframe = 0;
    twirltimer = TWIRL_SPEED+1;
    // wait for enter
    do
    {
      if (twirltimer>TWIRL_SPEED)
      {
        drawcharacter((dlgX+twirlX)<<3, (dlgY+twirlY)<<3, twirlframe+9);
        update_screen();
        twirlframe++;
        if (twirlframe>5) twirlframe=0;
        twirltimer=0;
      } else twirltimer++;
      if (immediate_keytable[KENTER] && !lastenter) break;
      lastenter = immediate_keytable[KENTER];
      SpeedThrottle();
    } while(!immediate_keytable[KQUIT]);

  sound_resume();
}

void game_save(char *fname)
{
int i;
FILE *fp;

   fp = fopen(fname, "wb");

   // save the header/version check
   fputc('S', fp);
   fputc(SAVEGAMEVERSION, fp);

   // save all necessary structures to the file
   if (map.isworldmap) fputc('W', fp); else fputc('L', fp);

   sgrle_compress(fp, (unsigned char *)&numplayers, sizeof(numplayers));
   sgrle_compress(fp, (unsigned char *)&levelcontrol, sizeof(levelcontrol));
   sgrle_compress(fp, (unsigned char *)&scroll_x, sizeof(scroll_x));
   sgrle_compress(fp, (unsigned char *)&scroll_y, sizeof(scroll_y));
   sgrle_compress(fp, (unsigned char *)&max_scroll_x, sizeof(max_scroll_x));
   sgrle_compress(fp, (unsigned char *)&max_scroll_y, sizeof(max_scroll_y));
   sgrle_compress(fp, (unsigned char *)&map, sizeof(map));
   for(i=0;i<numplayers;i++)
   {
     sgrle_compress(fp, (unsigned char *)&player[i], sizeof(player[i]));
   }
   sgrle_compress(fp, (unsigned char *)&objects[0], sizeof(objects));
   sgrle_compress(fp, (unsigned char *)&tiles[0], sizeof(tiles));

   fclose(fp);

   return;
}

int savegameiswm(char *fname)
{
FILE *fp;
int i;

   fp = fopen(fname, "rb");
   if (!fp) return 0;          // file didn't exist, don't try to go further

   fgetc(fp); fgetc(fp);
   i = fgetc(fp);

   fclose(fp);

   if (i=='W') return 1; else return 0;
}

/*
int game_load(char *fname)
{
FILE *fp;
long i;
unsigned long scrx;
int scry;

   fp = fopen(fname, "rb");
   if (!fp) return 1;

   // do the header and version check
   if (fgetc(fp) != 'S') { fclose(fp); return 1; }
   if (fgetc(fp) != SAVEGAMEVERSION) { fclose(fp); return 1; }
   fgetc(fp);           // iswm flag--not needed here

   // save all necessary structures to the file
   fread(&numplayers, sizeof(numplayers), 1, fp);
   fread(&levelcontrol, sizeof(levelcontrol), 1, fp);   
   fread(&scrx, sizeof(scrx), 1, fp);
   fread(&scry, sizeof(scry), 1, fp);
   fread(&max_scroll_x, sizeof(max_scroll_x), 1, fp);
   fread(&max_scroll_y, sizeof(max_scroll_y), 1, fp);
   fread(&map, sizeof(map), 1, fp);

   initgame();           // reset scroll
   drawmap();
   for(i=0;i<scrx;i++) map_scroll_right();
   for(i=0;i<scry;i++) map_scroll_down();

   fread(&player[0], sizeof(player[0]), numplayers, fp);
   fread(&objects[0], sizeof(objects), 1, fp);
   fread(&tiles[0], sizeof(tiles), 1, fp);

   fclose(fp);

   debugmode = 0;
   return 0;
}
*/

int game_load(char *fname)
{
FILE *fp;
long i;
unsigned long scrx;
int scry;

   fp = fopen(fname, "rb");
   if (!fp) return 1;

   // do the header and version check
   if (fgetc(fp) != 'S') { fclose(fp); return 1; }
   if (fgetc(fp) != SAVEGAMEVERSION) { fclose(fp); return 1; }
   fgetc(fp);           // iswm flag--not needed here

   // load all structures from the file
   sgrle_reset();
   sgrle_decompress(fp, (unsigned char *)&numplayers, sizeof(numplayers));
   sgrle_decompress(fp, (unsigned char *)&levelcontrol, sizeof(levelcontrol));
   sgrle_decompress(fp, (unsigned char *)&scrx, sizeof(scrx));
   sgrle_decompress(fp, (unsigned char *)&scry, sizeof(scry));
   sgrle_decompress(fp, (unsigned char *)&max_scroll_x, sizeof(max_scroll_x));
   sgrle_decompress(fp, (unsigned char *)&max_scroll_y, sizeof(max_scroll_y));
   sgrle_decompress(fp, (unsigned char *)&map, sizeof(map));

   initgame();           // reset scroll
   drawmap();
   for(i=0;i<scrx;i++) map_scroll_right();
   for(i=0;i<scry;i++) map_scroll_down();

   for(i=0;i<numplayers;i++)
   {
     sgrle_decompress(fp, (unsigned char *)&player[i], sizeof(player[i]));
   }
   sgrle_decompress(fp, (unsigned char *)&objects[0], sizeof(objects));
   sgrle_decompress(fp, (unsigned char *)&tiles[0], sizeof(tiles));

   fclose(fp);

   debugmode = 0;
   return 0;
}

// pops up the "which slot do you want to save to" box.
// is issave=1, it's a save box, if issave=0, it's a load box
// returns either the selected slot or 0 if canceled
char save_slot_box(int issave)
{
char saveslot;
char keytable[KEYTABLE_SIZE+1],lastkeytable[KEYTABLE_SIZE+1];
FILE *fp;
char fname[40];
char slotexists;
char enter, lastenterstate, lastyes, lastno;
int x, bmnum;
int dlgX,dlgY,dlgW,dlgH;

top: ;
  if (issave)
  {
     dlgX = GetStringAttribute("WhichSlotSave", "LEFT");
     dlgY = GetStringAttribute("WhichSlotSave", "TOP");
     dlgW = GetStringAttribute("WhichSlotSave", "WIDTH");
     dlgH = GetStringAttribute("WhichSlotSave", "HEIGHT");
  }
  else
  {
     dlgX = GetStringAttribute("WhichSlotLoad", "LEFT");
     dlgY = GetStringAttribute("WhichSlotLoad", "TOP");
     dlgW = GetStringAttribute("WhichSlotLoad", "WIDTH");
     dlgH = GetStringAttribute("WhichSlotLoad", "HEIGHT");
     map_redraw();
     bmnum = GetBitmapNumberFromName("TITLE");
     x = (320/2)-(bitmaps[bmnum].xsize/2);
     DrawBitmap(x, 0, bmnum);
  }

  saveslot = 0;
  memcpy(&lastkeytable, &immediate_keytable, KEYTABLE_SIZE);
  do
  {
    memcpy(&keytable, &immediate_keytable, KEYTABLE_SIZE);

    gamedo_render_drawobjects();

    sb_dialogbox(dlgX,dlgY,dlgW,dlgH);
    if (issave)
    {
      sb_font_draw(getstring("WhichSlotSave"),(dlgX+1)<<3,(dlgY+1)<<3);
    }
    else
    {
      sb_font_draw(getstring("WhichSlotLoad"),(dlgX+1)<<3,(dlgY+1)<<3);
      gamedo_AnimatedTiles();
    }

    if (keytable[KNUM1] && !lastkeytable[KNUM1]) saveslot = 1;
    if (keytable[KNUM2] && !lastkeytable[KNUM2]) saveslot = 2;
    if (keytable[KNUM3] && !lastkeytable[KNUM3]) saveslot = 3;
    if (keytable[KNUM4] && !lastkeytable[KNUM4]) saveslot = 4;
    if (keytable[KNUM5] && !lastkeytable[KNUM5]) saveslot = 5;
    if (keytable[KNUM6] && !lastkeytable[KNUM6]) saveslot = 6;
    if (keytable[KNUM7] && !lastkeytable[KNUM7]) saveslot = 7;
    if (keytable[KNUM8] && !lastkeytable[KNUM8]) saveslot = 8;
    if (keytable[KNUM9] && !lastkeytable[KNUM9]) saveslot = 9;

    sb_blit();
    gamedo_render_eraseobjects();

    memcpy(&lastkeytable, &keytable, KEYTABLE_SIZE);

    SpeedThrottle();
  } while(!immediate_keytable[KQUIT] && !saveslot);

  /* check if the selected save file exists */
  sprintf(fname, "ep%csave%c.dat", levelcontrol.episode+'0', saveslot+'0');
  slotexists = 0;
  fp = fopen(fname, "rb");
  if (fp)
  {
    fclose(fp);
    slotexists = 1;
  }

  if ((issave && !slotexists) || (!issave && slotexists))
  {
    map_redraw();
    return saveslot;
  }

  if (issave)
  {
     dlgX = GetStringAttribute("SaveSlotOverwrite", "LEFT");
     dlgY = GetStringAttribute("SaveSlotOverwrite", "TOP");
     dlgW = GetStringAttribute("SaveSlotOverwrite", "WIDTH");
     dlgH = GetStringAttribute("SaveSlotOverwrite", "HEIGHT");
  }
  else
  {
     dlgX = GetStringAttribute("LoadNoSuchSlot", "LEFT");
     dlgY = GetStringAttribute("LoadNoSuchSlot", "TOP");
     dlgW = GetStringAttribute("LoadNoSuchSlot", "WIDTH");
     dlgH = GetStringAttribute("LoadNoSuchSlot", "HEIGHT");
  }

  // either we're trying to save over an existing game, or we're
  // loading a game that doesn't exist.
  lastenterstate = 1;
  do
  {

    gamedo_render_drawobjects();

    sb_dialogbox(dlgX,dlgY,dlgW,dlgH);
    if (issave)
    {
      sb_font_draw(getstring("SaveSlotOverwrite"),(dlgX+1)<<3,(dlgY+1)<<3);
      if (immediate_keytable[KN] && !lastno)
      {
        map_redraw();
        goto top;
      }
      else if (immediate_keytable[KY] && !lastyes)
      {
        map_redraw();
        return saveslot;
      }

      lastyes = immediate_keytable[KY];
      lastno = immediate_keytable[KN];
    }
    else
    {
      sb_font_draw(getstring("LoadNoSuchSlot"),(dlgX+1)<<3,(dlgY+1)<<3);

      enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]||immediate_keytable[KUP]||immediate_keytable[KDOWN]||immediate_keytable[KLEFT]||immediate_keytable[KRIGHT]);
      if (enter && !lastenterstate)
      {
        map_redraw();
        goto top;
      }
      lastenterstate = enter;

      gamedo_AnimatedTiles();
    }

    sb_blit();
    gamedo_render_eraseobjects();

    SpeedThrottle();
  } while(!immediate_keytable[KQUIT]);

  map_redraw();
  return 0;
}

void game_save_interface(void)
{
int waittimer;
char fname[40];
char enter, lastenterstate;
char saveslot;
int dlgX,dlgY,dlgW,dlgH;

  dlgX = GetStringAttribute("GameSaveSuccess", "LEFT");
  dlgY = GetStringAttribute("GameSaveSuccess", "TOP");
  dlgW = GetStringAttribute("GameSaveSuccess", "WIDTH");
  dlgH = GetStringAttribute("GameSaveSuccess", "HEIGHT");

  saveslot = save_slot_box(1);
  if (!saveslot) return;                // canceled

  /* save the game */
  sprintf(fname, "ep%csave%c.dat", levelcontrol.episode+'0', saveslot+'0');
  game_save(fname);

  /* display the "your game has been saved" box */
  waittimer = 0;
  lastenterstate = 1;
  do
  {
    enter = (immediate_keytable[KENTER]||immediate_keytable[KCTRL]||immediate_keytable[KALT]||immediate_keytable[KUP]||immediate_keytable[KDOWN]||immediate_keytable[KLEFT]||immediate_keytable[KRIGHT]);

    waittimer++;
    if (waittimer > 5000) break;
    if (enter && !lastenterstate) break;

    gamedo_render_drawobjects();

    sb_dialogbox(dlgX,dlgY,dlgW,dlgH);
    sb_font_draw(getstring("GameSaveSuccess"),(dlgX+1)<<3,(dlgY+1)<<3);

    sb_blit();
    gamedo_render_eraseobjects();

    lastenterstate = enter;
    SpeedThrottle();
  } while(!immediate_keytable[KQUIT]);

  map_redraw();
}

int VerifyQuit(void)
{
char enter, lastenterstate;
int dlgX,dlgY,dlgW,dlgH;
char *text;
char lastno, lastyes, lastquit;

  if (fade.mode==FADE_GO) return NO_QUIT;

  text = getstring("VerifyQuit");
  dlgX = GetStringAttribute("VerifyQuit", "LEFT");
  dlgY = GetStringAttribute("VerifyQuit", "TOP");
  dlgW = GetStringAttribute("VerifyQuit", "WIDTH");
  dlgH = GetStringAttribute("VerifyQuit", "HEIGHT");

  // either we're trying to save over an existing game, or we're
  // loading a game that doesn't exist.
  lastenterstate = lastno = lastyes = lastquit = 1;
  do
  {
    gamedo_render_drawobjects();
    gamedo_AnimatedTiles();

    sb_dialogbox(dlgX, dlgY, dlgW, dlgH);
    sb_font_draw(text, (dlgX+1)<<3, (dlgY+1)<<3);
    if (immediate_keytable[KQ] && !lastno)
    {
      map_redraw();
      QuitState = QUIT_PROGRAM;
      return 0;
    }
    else if (immediate_keytable[KT] && !lastyes)
    {
      map_redraw();
      QuitState = QUIT_TO_TITLE;
      return QuitState;
    }
    else if (immediate_keytable[KQUIT] && !lastquit)
    {
      map_redraw();
      QuitState = NO_QUIT;
      return QuitState;
    }

    lastyes = immediate_keytable[KT];
    lastno = immediate_keytable[KQ];
    lastquit = immediate_keytable[KQUIT];

    sb_blit();
    gamedo_render_eraseobjects();

    SpeedThrottle();
  } while(1);
}

int endsequence(void)
{
int i;

  if (levelcontrol.episode==1)
  {
     if (eseq1_ReturnsToShip()) return;
     if (eseq1_ShipFlys()) return;
     eseq1_BackAtHome();
  }
  else if (levelcontrol.episode==2)
  {
     if (eseq2_HeadsForEarth()) return;
     if (eseq2_LimpsHome()) return;
     if (eseq2_SnowedOutside()) return;
  }
  else if (levelcontrol.episode==3)
  {
     if (eseq3_AwardBigV()) return;
  }
}

void AllPlayersInvisible(void)
{
int i;

  for(i=0;i<MAX_PLAYERS;i++)
  {
    if (player[i].isPlaying)
    {
      objects[player[i].useObject].onscreen = 0;
      player[i].hideplayer = 1;
    }
  }
}

char gameiswon(void)
{
int partcount;
int i;

       if (levelcontrol.episode==1)
       {
          /* episode 1: game is won when all parts are collected */

           // count the number of parts the players have acquired
           partcount = 0;
           for(i=0;i<MAX_PLAYERS;i++)
           {
             if (player[i].isPlaying)
             {
               if (player[i].inventory.HasJoystick) partcount++;
               if (player[i].inventory.HasBattery) partcount++;
               if (player[i].inventory.HasFuel) partcount++;
               if (player[i].inventory.HasVacuum) partcount++;
             }
           }
        
           // go to end sequence if all the parts have been got
           if (partcount >= 4)
           {
             return 1;
           }
           else return 0;
       }
       else if (levelcontrol.episode==2)
       {
         /* episode 2: game is won when all cities are saved */
         if (!levelcontrol.levels_completed[4]) return 0;
         if (!levelcontrol.levels_completed[6]) return 0;
         if (!levelcontrol.levels_completed[7]) return 0;
         if (!levelcontrol.levels_completed[13]) return 0;
         if (!levelcontrol.levels_completed[11]) return 0;
         if (!levelcontrol.levels_completed[9]) return 0;
         if (!levelcontrol.levels_completed[15]) return 0;
         if (!levelcontrol.levels_completed[16]) return 0;
         return 1;
       }
       else if (levelcontrol.episode==3)
       {
         /* episode 3: game is won when mortimer is defeated */
         if (levelcontrol.levels_completed[16])
         {
           return 1;
         }
         else
         {
           return 0;
         }
       }

return 0;
}

void usage(void)
{
  VidDrv_printf("Usage: keen [lvlnum] [-*player] [-nopk] [-ep*] [-dtm] [-nocheat] [-rec] -[eseq]\n\n");
  VidDrv_printf("lvlnum          specify a level number (such as 2) to go directly to that level\n");
  VidDrv_printf("-*player        select number of players (1-4); defaults to 1\n");
  VidDrv_printf("-nopk           do not allow players to kill each other in multiplayer games\n");
  VidDrv_printf("-ep*            select episode 1, 2, or 3; defaults to 1\n");
  VidDrv_printf("-dtm            go directly to the world map, bypassing intro and title screen\n");
  VidDrv_printf("-mean           increase game difficulty\n");
  VidDrv_printf("-cheat          enable function key cheat/debug codes\n");
  VidDrv_printf("-rec            record player actions to demo.dat for making a demo\n");
  VidDrv_printf("-eseq           for the impatient--cut directly to the ending sequence\n");    
#ifdef BUILD_SDL
  VidDrv_printf("-fs             use fullscreen mode\n");
  VidDrv_printf("-dbl            zoom image 2x\n");
  VidDrv_printf("-showfps        show FPS in upper-right of screen\n");
#endif
#ifdef TARGET_WIN32
  VidDrv_printf("\n-host & -join   for the experimental network play mode. These DON'T work yet.\n");
#endif

  VidDrv_printf("\n");
  VidDrv_printf("Examples:\n");
  VidDrv_printf("  keen 3 -ep2                play ep 2, level 3 in 1-player mode\n");
  VidDrv_printf("  keen -ep3 -dtm -2player    play ep3, skip title&intro, 2-player mode\n");
  VidDrv_printf("  keen -ep3                  play a normal game of ep3\n");
}

void radar(void)
{
int x,y,o;
int x1,y1,x2,y2;
int yoff;
  // draw the map
  for(y=0;y<map.ysize;y++)
  {
    yoff = ((y+4+scrolly_buf)&511)<<9;
    for(x=0;x<map.xsize;x++)
    {
      scrollbuf[yoff+((4+x+scrollx_buf)&511)] = map.mapdata[x][y]&15;
    }
  }

  // draw objects
  for(o=0;o<MAX_OBJECTS;o++)
  {
    if (objects[o].exists)
    {
      x = objects[o].x >> CSF >> 4;
      y = objects[o].y >> CSF >> 4;

      yoff = ((y+4+scrolly_buf)&511)<<9;
      scrollbuf[yoff+((4+x+scrollx_buf)&511)] = objects[o].type&15;
    }
  }

  // draw the area that is visible in the scrollbuffer
  x1 = mapx; y1 = mapy;
  x2 = x1+32; y2 = y1+32;
  for(y=y1;y<y2;y++)
  {
     if (y<map.ysize)
     {
       yoff = ((y+4+scrolly_buf)&511)<<9;
       scrollbuf[yoff+((4+x1+scrollx_buf)&511)] = 10;
       if (x2<map.xsize)
         scrollbuf[yoff+((4+x2+scrollx_buf)&511)] = 10;
     }
  }
  for(x=x1;x<=x2;x++)
  {
     if (y1 < map.ysize && x < map.xsize)
     {
       yoff = ((y1+4+scrolly_buf)&511)<<9;
       scrollbuf[yoff+((4+x+scrollx_buf)&511)] = 10;
     }
     if (y2 < map.ysize && x < map.xsize)
     {
       yoff = ((y2+4+scrolly_buf)&511)<<9;
       scrollbuf[yoff+((4+x+scrollx_buf)&511)] = 10;
     }
  }

  // draw the area that is visible on the screen
  // 320x200 = 20x12.5 tiles
  x1 = scroll_x>>4; y1 = scroll_y>>4;
  x2 = x1+20; y2 = y1+12;
  for(y=y1;y<y2;y++)
  {
     if (y<map.ysize)
     {
       yoff = ((y+4+scrolly_buf)&511)<<9;
       scrollbuf[yoff+((4+x1+scrollx_buf)&511)] = 12;
       if (x2<map.xsize)
         scrollbuf[yoff+((4+x2+scrollx_buf)&511)] = 12;
     }
  }
  for(x=x1;x<=x2;x++)
  {
     if (x < map.xsize)
     {
       if (y1 < map.ysize)
       {
         yoff = ((y1+4+scrolly_buf)&511)<<9;
         scrollbuf[yoff+((4+x+scrollx_buf)&511)] = 12;
       }
       if (y2 < map.ysize)
       {
         yoff = ((y2+4+scrolly_buf)&511)<<9;
         scrollbuf[yoff+((4+x+scrollx_buf)&511)] = 12;
       }
     }
  }
}

void SetAllCanSupportPlayer(int o, int state)
{
int i;
  for(i=0;i<numplayers;i++)
  {
    objects[o].cansupportplayer[i] = state;
  }
}

