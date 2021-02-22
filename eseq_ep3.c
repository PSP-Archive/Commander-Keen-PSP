/* ESEQ_EP3.C
  Ending sequence for Episode 3.
  I never bothered to completely finish it...things like
  the Vorticon's camera flashes and stuff aren't implemented.
*/

#include "keen.h"
#include "game.h"

void eseq3_Mortimer(void)
{
int x,y,w,h;
  sound_play(SOUND_MORTIMER, PLAY_FORCE);

  x = GetStringAttribute("EP3_MORTIMER", "LEFT");
  y = GetStringAttribute("EP3_MORTIMER", "TOP");
  w = GetStringAttribute("EP3_MORTIMER", "WIDTH");
  h = GetStringAttribute("EP3_MORTIMER", "HEIGHT");

  eseq_showmsg(getstring("EP3_MORTIMER"),x,y,w,h,0);
  eseq_showmsg(getstring("EP3_MORTIMER2"),x,y,w,h,0);
  eseq_showmsg(getstring("EP3_MORTIMER3"),x,y,w,h,0);
  eseq_showmsg(getstring("EP3_MORTIMER4"),x,y,w,h,0);
  eseq_showmsg(getstring("EP3_MORTIMER5"),x,y,w,h,0);

  map_redraw();
  sound_play(SOUND_FOOTSLAM, PLAY_NOW);
}

char eseq3_AwardBigV(void)
{
  int enter;
  int x,y,w,h;
  int c;
  initgame();

  showmapatpos(81, 32, 32, 0);

  numplayers = 1;
  player[0].x = 244<<CSF;
  player[0].y = 104<<CSF;
  player[0].playframe = 0;

  fade.mode = FADE_GO;
  fade.rate = FADE_NORM;
  fade.dir = FADE_IN;
  fade.curamt = 0;
  fade.fadetimer = 0;

  x = GetStringAttribute("EP3_ESEQ_PAGE1", "LEFT");
  y = GetStringAttribute("EP3_ESEQ_PAGE1", "TOP");
  w = GetStringAttribute("EP3_ESEQ_PAGE1", "WIDTH");
  h = GetStringAttribute("EP3_ESEQ_PAGE1", "HEIGHT");

  eseq_showmsg(getstring("EP3_ESEQ_PAGE1"),x,y,w,h,1);
  eseq_showmsg(getstring("EP3_ESEQ_PAGE2"),x,y,w,h,1);
  eseq_showmsg(getstring("EP3_ESEQ_PAGE3"),x,y,w,h,1);
  eseq_showmsg(getstring("EP3_ESEQ_PAGE4"),x,y,w,h,1);

  finale_draw("finale.ck3");
  scrollx_buf = scrolly_buf = 0;
  player[0].hideplayer = 1;

  fade.mode = FADE_GO;
  fade.rate = FADE_NORM;
  fade.dir = FADE_OUT;
  fade.curamt = PAL_FADE_SHADES;
  fade.fadetimer = 0;
  do
  {
    gamedo_fades();
    SpeedThrottle();
  } while(fade.mode==FADE_GO);

  fade.mode = FADE_GO;
  fade.rate = FADE_NORM;
  fade.dir = FADE_IN;
  fade.curamt = 0;
  fade.fadetimer = 0;

  x = GetStringAttribute("THE_END", "LEFT");
  y = GetStringAttribute("THE_END", "TOP");
  w = GetStringAttribute("THE_END", "WIDTH");
  h = GetStringAttribute("THE_END", "HEIGHT");

  eseq_showmsg(getstring("THE_END"),x,y,w,h,0);

  // wait for enter pressed
  c = 0;
  do
  {
    gamedo_fades();
    if (c==0 && !immediate_keytable[KENTER]) c++;
    if (c==1 && immediate_keytable[KENTER]) c++;
    if (c==2 && fade.dir==FADE_IN)
    {
       fade.mode = FADE_GO;
       fade.rate = FADE_NORM;
       fade.dir = FADE_OUT;
       fade.curamt = PAL_FADE_SHADES;
       fade.fadetimer = 0;
    }
    SpeedThrottle();
    if (immediate_keytable[KQUIT] && fade.mode==FADE_COMPLETE) break;
  } while(fade.mode!=FADE_COMPLETE || fade.dir!=FADE_OUT);

  if (immediate_keytable[KQUIT]) return 1;
  return 0;
}
