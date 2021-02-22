// gamedo.c
void gamedo_getInput(void);
int gamedo_ScrollTriggers(int theplayer);
void gamedo_AnimatedTiles(void);
void gamedo_enemyai(void);
void gamedo_render_drawobjects(void);
void gamedo_render_eraseobjects(void);
void gamedo_render_drawdebug(void);
void gamedo_render_erasedebug(void);
void gamedo_RenderScreen(void);
void gamedo_HandleFKeys(void);
void gamedo_fades(void);
void gamedo_frameskipping(void);
void gamedo_frameskipping_blitonly(void);
// gamepdo.c
void gamepdo_HandlePlayer(int cp);
void gamepdo_walkbehindexitdoor(int cp);
void gamepdo_dieanim(int cp);
void gamepdo_keencicle(int cp);
void gamepdo_ProcessInput(int cp);
void gamepdo_setdir(int cp);
void gamepdo_setblockedlru(int cp);
void gamepdo_getgoodies(int cp);
void gamepdo_walkinganim(int cp);
void gamepdo_walking(int cp);
void gamepdo_playpushed(int cp);
void gamepdo_InertiaAndFriction_X(int cp);
void gamepdo_JumpAndPogo(int cp);
void gamepdo_falling(int cp);
void gamepdo_raygun(int cp);
void gamepdo_special(int cp);
void gamepdo_SelectFrame(int cp);
void gamepdo_ankh(int cp);
void gamepdo_StatusBox(int cp);
// gamepdowm.c
void gamepdo_wm_HandlePlayer(int cp);
void gamepdo_wm_SelectFrame(int cp);
void gamepdo_wm_setblockedlrud(int cp);
void gamepdo_wm_setdir(int cp);
void gamepdo_InertiaAndFriction_Y(int cp);
void gamepdo_wm_AllowEnterLevel(int cp);
char wm_issolid(int xb, int yb);

// game.c
void SetGameOver(void);
void overrun_detect(void);
void scrolltest(void);
void gameloop(void);
void gameloop_initialize(void);
void give_keycard(int doortile, int p);
void take_keycard(int doortile, int p);
void open_door(int doortile, int doorsprite, int mpx, int mpy, int p);
void extralifeat(int p);
void keen_get_goodie(int px, int py, int theplayer);
void initgame(void);
void initgamefirsttime(void);
char spawn_object(int x, int y, int otype);
void common_enemy_ai(int o);
char hitdetect(int object1, int object2);
void killplayer(int theplayer);
void freezeplayer(int theplayer);
void unregister_animtiles(int tile);
void PlayerTouchedExit(int theplayer);
void endlevel(int success);
char checkobjsolid(int x, int y, int cp);
char checkissolidl(int x, int y, int cp);
char checkissolidr(int x, int y, int cp);
void initsprites(void);
void CopyTileToSprite(int t, int s, int ntilestocopy, int transparent);
void procgoodie_ep1(int t, int mpx, int mpy, int theplayer);
void procgoodie_ep2(int t, int mpx, int mpy, int theplayer);
void procgoodie_ep3(int t, int mpx, int mpy, int theplayer);
void GiveAnkh(int cp);
// map.c
void map_scroll_right(void);
void map_scroll_left(void);
void map_scroll_down(void);
void map_scroll_up(void);
void map_draw_vstripe(unsigned int x, unsigned int mpx);
void map_draw_hstripe(unsigned int y, unsigned int mpy);
void nosb_map_draw_vstripe(unsigned int x, unsigned int mapx);
unsigned int getmaptileat(unsigned int x, unsigned int y);
unsigned int getlevelat(unsigned int x, unsigned int y);
void drawmap(void);
void map_unregister_all_animtiles(void);
void map_deanimate(int x, int y);
void map_animate(int x, int y);
char map_findobject(int obj, int *xout, int *yout);
char map_findtile(int tile, int *xout, int *yout);
void map_redraw(void);
// dos\snddrv.c
char SoundDrv_Start(void);
void SoundDrv_Stop(void);
char sound_load(char *fname, char *searchname, char loadnum);
void sound_do(void);
char sound_play(int snd, char mode);
char sound_is_playing(int snd);
void sound_stop(int snd);
void sound_stop_all(void);
void sound_pause(void);
void sound_resume(void);
// graphics.c
char Graphics_Start(void);
void Graphics_Stop(void);
void inline sb_setpixel(int x, int y, unsigned char c);
unsigned char inline sb_getpixel(int x, int y);
void drawtile(int x, int y, unsigned int t);
void drawtile_direct(int x, int y, unsigned int t);
void drawtilewithmask(int x, int y, unsigned int til, unsigned int tmask);
void drawprioritytile(int x, int y, unsigned int til);
void drawsprite_direct(int x, int y, unsigned int t);
void drawsprite(int x, int y, unsigned int s, int objectnum);
void erasesprite(int x, int y, unsigned int s, int objectnum);
void drawcharacter(int x, int y, int f);
void sb_drawcharacter(int x, int y, int f);
void sb_drawcharacterinverse(int x, int y, int f);
void save_area(int x1, int y1, int x2, int y2);
void restore_area(int x1, int y1, int x2, int y2);
void setvideomode(unsigned char mode);
void addpal(int c, int r, int g, int b);
void pal_init(int dark);
void pal_fade(int fadeamt);
void font_draw(unsigned char *text, int xoff, int yoff, int highlight);
void sb_font_draw(unsigned char *text, int xoff, int yoff);
void sb_font_draw_inverse(unsigned char *text, int xoff, int yoff);
// viddrv.c
char VidDrv_Start(void);
char VidDrv_CreateSurfaces(void);
void VidDrv_Stop(void);
void VidDrv_reset(void);
void pal_set(char colour, char red, char green, char blue);
void setpixel(int x, int y, unsigned char c);
unsigned char getpixel(int x, int y);
void sb_blit(void);
void poll_events(void);
void update_screen(void);

void DrawConsoleMessages(void);
void AddConsoleMsg(char *the_msg);
void DeleteConsoleMsgs(void);

// .ai functions
// ep1
void yorp_ai(int o);
void garg_ai(int o);
void vort_ai(int o);
void butler_ai(int o);
void tank_ai(int o);
void ray_ai(int o);
void door_ai(int o);
void icechunk_ai(int o);
void icebit_ai(int o);
void teleporter_ai(int o);
void rope_ai(int o);
// ep2
void walker_ai(int o);
void tankep2_ai(int o);
void platform_ai(int o);
void bear_ai(int o);
void se_ai(int o);
void baby_ai(int o);
void explosion_ai(int o);
void earthchunk_ai(int o);
// ep3
void foob_ai(int o);
void ninja_ai(int o);
void meep_ai(int o);
void sndwave_ai(int o);
void mother_ai(int o);
void fireball_ai(int o);
void ballandjack_ai(int o);
void platvert_ai(int o);
void nessie_ai(int o);

// fileio.c
void addmaptile(unsigned int t);
void addenemytile(unsigned int t);
unsigned int fgeti(FILE *fp);
unsigned int loadmap(char *fname, int lvlnum, int isworldmap);
char loadtiles(char *fname);
char loadsprites(char *spritename);
char loadfont(char *fontname);
char loadstrings(char *fname);
int freestrings(void);
char* getstring(char *name);
// keydrv.c
char KeyDrv_Start(void);
void KeyDrv_Stop(void);
// misc.c
char allocmem(void);
void banner(void);
void cleanup(void);
void dialogbox(int x1, int y1, int w, int h);
void sb_dialogbox(int x1, int y1, int w, int h);
int VerifyQuit(void);
void statusbox(void);
void youseeinyourmind(int mpx, int mpy, int isgarg);
void VorticonElder(int mpx, int mpy);
void showinventory(int p);
void sshot(char *visiblefile, char *scrollfile);
void YourShipNeedsTheseParts(void);
void ShipEp3(void);
void game_save(char *fname);
int game_load(char *fname);
char save_slot_box(int issave);
void game_save_interface(void);
int savegameiswm(char *fname);
int endsequence(void);
char gameiswon(void);
void usage(void);
void radar(void);
void SetAllCanSupportPlayer(int o, int state);

// menu.c
int mainmenu(int defaultopt);
char configmenu(void);
void showmapatpos(int level, int xoff, int yoff, int wm);
int intro(void);

// keen.c
void playgame_levelmanager(int argc, char **argv, int dtm);
char play_demo(int demonum);

// eseq_ep1.c
int eseq1_ReturnsToShip(void);
int eseq1_ShipFlys(void);
int eseq1_BackAtHome(void);
void addshipqueue(int cmd, int time, int flag1);
void eseq_ToBeContinued(void);

// eseq_ep2.c
int eseq2_TantalusRay(void);
void eseq2_vibrate(void);
int eseq2_HeadsForEarth(void);
int eseq2_LimpsHome(void);
void eseq_showmsg(char *text, int boxleft, int boxtop, int boxwidth, int boxheight, char autodismiss);

// eseq_ep3.c
void eseq3_Mortimer(void);
char eseq3_AwardBigV(void);

// TimeDrv.c
char TimeDrv_Start(void);
void TimeDrv_Stop(void);
void SpeedThrottle(void);

