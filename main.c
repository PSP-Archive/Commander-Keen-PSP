/* MAIN.C
  This is CloneKeen's main source file.

  The CloneKeen source may be freely distributed and
  modified as per the GPL but please give credit to
  the original author, Caitlin Shaw.

  Please go ahead and port this game to other platforms.
  I would love to see it on Linux, Mac, or CE but I do
  not have the platforms/knowledge to port to these platforms.

  If you make any changes or improvements to the code that
  you feel merit inclusion in the source tree email them
  to me at rogueeve@mailshack.com or get my latest email
  from the SourceForge site.

  Thanks to ID Software for the "Commander Keen: Invasion of
  the Vorticons" games. "Commander Keen" and it's associated
  graphics, level, and sound files are the property of ID
  Software. CloneKeen requires the original version of a
  Commander Keen game in order to be able to emulate that
  episode.

  CloneKeen comes with one new level for episode 1 (shareware),
  and 2 new levels for episode 2. This levels are released
  under the same license as the rest of the program.

  keen 17 -ep1 -fs
  keen 18 -ep2 -fs
  keen 19 -ep2 -fs

  Enjoy the Code
         -Caitlin

  CloneKeen 2003-2005 Caitlin Shaw
*/

#include "keen.h"

extern int IntroCanceled;
int NessieObjectHandle;
int DemoObjectHandle;
int BlankSprite;
int DemoSprite;
int current_demo;
int framebyframe;
int fps=0, curfps=0;
unsigned int demo_RLERunLen;
unsigned char demo_data[DEMO_MAX_SIZE+1];
unsigned int demo_data_index;

char QuitState = NO_QUIT;

stString strings[MAX_STRINGS+1];
int numStrings;

int demomode;
FILE *demofile;

char ScreenIsScrolling;
int gunfiretimer, gunfirefreq;

extern unsigned long scroll_x;
extern unsigned int scrollx_buf;
extern unsigned char scrollpix;
extern unsigned int mapx;
extern unsigned int mapxstripepos;
extern int scroll_y;
extern unsigned int scrolly_buf;
extern unsigned char scrollpixy;
extern unsigned int mapy;
extern unsigned int mapystripepos;

stLevelControl levelcontrol;

char loadinggame, loadslot;

FILE *log=NULL;

stFade fade;
stMap map;
unsigned int AnimTileInUse[ATILEINUSE_SIZEX][ATILEINUSE_SIZEY];
stTile tiles[MAX_TILES+1];
unsigned char tiledata[MAX_TILES+1][16][16];
stSprite sprites[MAX_SPRITES+1];
stBitmap bitmaps[MAX_BITMAPS+1];
stObject objects[MAX_OBJECTS+1];
char font[MAX_FONT+1][8][8];
stAnimTile animtiles[MAX_ANIMTILES+1];
stPlayer player[MAX_PLAYERS];
stPlayer net_lastplayer[MAX_PLAYERS];
stOption options[NUM_OPTIONS];
unsigned char *scrollbuf = NULL;
unsigned char *blitbuf = NULL;
char immediate_keytable[KEYTABLE_SIZE+1];
char last_immediate_keytable[KEYTABLE_SIZE+1];
int scroll_y = 0;
unsigned int objdefsprites[NUM_OBJ_TYPES+1];
char frameskiptimer=0;

int thisplayer;
int primaryplayer;
int numplayers;
char is_client;
char is_server;
char showfps;
char localmp;

int crashflag,crashflag2,crashflag3;
char *why_term_ptr = "No reason given.";

void setoption(char opt, char *name, char value)
{
  if (name != NULL)
    options[opt].name = name;

  options[opt].value = value;
}

void SetDefaultOptions(void)
{
  setoption(OPT_FULLYAUTOMATIC, "Fully Automatic Raygun", 0);
  setoption(OPT_SUPERPOGO, "SuperPogo", 0);
  setoption(OPT_MEAN, "Increase Difficulty", 0);
  setoption(OPT_ALLOWPKING, "Allow PKing", 1);
  setoption(OPT_CHEATS, "Enable all cheats", 0);
}

#ifdef TARGET_PSP

#include <pspkernel.h>

PSP_MODULE_INFO("Keen", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common) {
	crashflag = QUIT_NONFATAL;
    why_term_ptr = "Homebutton: Got quit event!";

	sceKernelDelayThread(1000000);
	sceKernelExitGame();
	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

#endif

int main(int argc, char **argv)
{
#ifdef TARGET_PSP
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
#endif

  int i,c;
  int opt;
  int count;
  int dtm = 0;
  char tempbuf[80];
  int retval;
  int eseq;
  int defaultopt;
  char msg[1024];

  FILE *fp;

  levelcontrol.episode = 1;
  demomode = DEMO_NODEMO;
  current_demo = 1;
  framebyframe = 0;
  debugmode = 0;
  acceleratemode = 0;
  eseq = 0;
  is_client = 0;
  is_server = 0;
  localmp = 1;
  showfps = 0;

  VidDrv_InitConsole();

  banner();
  VidDrv_printf("\n");

  // set default config-menu options
  SetDefaultOptions();
  setoption(OPT_FULLSCREEN, "SDL Fullscreen Mode", 0);
  setoption(OPT_ZOOM, "Image Zoom", 1);
  setoption(OPT_FRAMESKIP, "Frameskip", 2);

  /* process command line options */
  VidDrv_printf("Processing command-line options.\n");
  numplayers = 1;

  if (argc>1)
  {
    for(i=1;i<argc;i++)
    {     
      strcpy(tempbuf, argv[i]);
      if (strcmp(tempbuf, "-1player")==0)
      {
        numplayers = 1;
      }
      else if (strcmp(tempbuf, "-2player")==0)
      {
        numplayers = 2;
      }
      else if (strcmp(tempbuf, "-3player")==0)
      {
        numplayers = 3;
      }
      else if (strcmp(tempbuf, "-4player")==0)
      {
        numplayers = 4;
      }
      else if (strcmp(tempbuf, "-single")==0)
      {
        numplayers = 1;
      }
      else if (strcmp(tempbuf, "-nopk")==0)     // do not allow players to kill each other
      {
        options[OPT_ALLOWPKING].value = 0;
      }
      else if (strcmp(tempbuf, "-ep1")==0)      // select episode 1
      {
        levelcontrol.episode = 1;
      }
      else if (strcmp(tempbuf, "-ep2")==0)      // select episode 2
      {
        levelcontrol.episode = 2;
      }
      else if (strcmp(tempbuf, "-ep3")==0)      // select episode 3
      {
        levelcontrol.episode = 3;
      }
      else if (strcmp(tempbuf, "-dtm")==0)      // direct to map
      {
        dtm = 1;
      }
      else if (strcmp(tempbuf, "-mean")==0)     // increase difficulty
      {
        options[OPT_MEAN].value = 1;
      }
      else if (strcmp(tempbuf, "-cheat")==0)    // enable cheat codes
      {
        options[OPT_CHEATS].value = 1;
      }
      else if (strcmp(tempbuf, "-rec")==0)      // record a demo
      {
        demomode = DEMO_RECORD;
      }
      else if (strcmp(tempbuf, "-eseq")==0)     // play end sequence
      {
        eseq = 1;
      }
      else if (strcmp(tempbuf, "-fs")==0)       // full-screen
      {
        options[OPT_FULLSCREEN].value = 1;
      }
      else if (strcmp(tempbuf, "-dbl")==0)       // 2x
      {
        options[OPT_ZOOM].value = 2;
      }
      else if (strcmp(tempbuf, "-showfps")==0)  // show fps
      {
        showfps = 1;
      }
      else if (strcmp(tempbuf, "-host")==0)     // start network server
      {
        is_server = 1;
        localmp = 0;
      }
      else if (strcmp(tempbuf, "-join")==0)     // connect to a server
      {
        is_client = 1;
        localmp = 0;
      }
      else if (i!=1 || atoi(argv[i])==0)
      {
        VidDrv_printf("Wait a minute...what the hell does '%s' mean?\n",tempbuf);
        usage();

        #ifdef TARGET_WIN32
           MessageBoxA(NULL, "Invalid command line. Usage instructions have been output to ck.log", "CloneKeen", MB_OK);
        #endif

        return;
      }
    }
  }

restart_game: ;
  setoption(OPT_EPISODE, "Episode", levelcontrol.episode);

  if (demomode==DEMO_RECORD)
  {
    demofile = fopen("demo.dat", "wb");
    fputc(levelcontrol.episode, demofile);      // episode
    fputc(atoi(argv[1]), demofile);             // level
  }
  else demofile = NULL;

  // allocate memory for scrollbuffer, etc.
  if (allocmem()) goto abort;
  // decode graphics from EGALATCH & EGASPRIT
  if (latch_loadgraphics(levelcontrol.episode)) goto abort;
  if (loadstrings("strings.dat")) goto abort;

  /* initilize/activate all drivers */
  VidDrv_printf("Starting sound driver...\n");
  if (SoundDrv_Start()) goto abort;
  VidDrv_printf("Starting keyboard driver...\n");
  if (KeyDrv_Start()) goto abort;
  VidDrv_printf("Starting graphics driver...\n");
  if (Graphics_Start()) goto abort;
  VidDrv_printf("Starting timer driver...\n");
  if (TimeDrv_Start()) goto abort;

  #ifdef NETWORK_PLAY
    //  if (numplayers > 1)
      {
        if (is_server)
        {
          VidDrv_printf("Starting network driver (server)...\n");
          if (Net_Server_Start()) goto abort;
        }
        else if (is_client)
        {
          VidDrv_printf("Starting network driver (client)...\n");
          if (Net_Client_Start()) goto abort;
        }
      }
  #endif

  player[0].x = player[0].y = 0;
  initgamefirsttime();
  initgame();
  if (dtm) goto directtomap;

  #ifdef BUILD_SDL
  if (argc>1 && *argv[1] != '-')
  {   // command to start at a specific level ("keen 4 -ep1", etc)
    playgame_levelmanager(argc, argv, 0);
    goto exitt;
  }
  #endif

  VidDrv_printf("Game starting...\n");

  if (eseq)
  {
    endsequence();
    goto exitt;
  }

  //if (intro()) goto exitt;

  defaultopt = 0;
  do   
  {
    if (QuitState==QUIT_TO_TITLE) QuitState = NO_QUIT;
    VidDrv_printf("calling mainmenu()\n");
    opt = mainmenu(defaultopt);
    VidDrv_printf("gcl: opt = %d\n", opt);
    defaultopt = 0;
    IntroCanceled = 0;
    switch(opt)
    {
    case MAINMNU_1PLAYER:
      numplayers = 1;
directtomap: ;
      defaultopt = 0;
      current_demo = 1;
      initgamefirsttime();
      loadinggame = 0;
      playgame_levelmanager(argc, argv, dtm);
      break;
    case MAINMNU_2PLAYER:
      defaultopt = 0;
      current_demo = 1;
      numplayers = 2;
      initgamefirsttime();
      loadinggame = 0;
      playgame_levelmanager(argc, argv, dtm);
      break;
    case MAINMNU_LOADGAME:
      if (loadslot)
      {
         loadinggame = 1;
         defaultopt = 0;
         current_demo = 1;
         numplayers = 2;
         initgamefirsttime();
         playgame_levelmanager(argc, argv, dtm);
      }
      break;
    case MAINMNU_TIMEOUT:
    case MAINMNU_DEMO:
      retval = play_demo(current_demo);

      if (retval==DEMO_RESULT_FILE_BAD)
      {
         // we tried to play a demo that did not exist--assume we
         // reached the last demo and go back to the intro
         intro();
         current_demo = 0;
      }
      else if (retval==DEMO_RESULT_CANCELED)
      { // user hit a key to cancel demo
         IntroCanceled = 1;            // pop up menu
      }

      if (IntroCanceled)
      { // user canceled out of demo (or intro if at end of demos)
         // if user selected "demo" have it selected when he comes back
         if (opt==MAINMNU_DEMO)
         {
           defaultopt = MAINMNU_DEMO;
         }
      }

      current_demo++;
    break;
    case RESTART_GAME:
      VidDrv_printf("********************\n");
      VidDrv_printf(" Restarting game...\n");
      VidDrv_printf("********************\n\n");
      cleanup();
      goto restart_game;
    break;

    default: break;
    }
    VidDrv_printf("bottom of game control loop opt=%d crashflag=%d\n", opt, crashflag);
  } while(opt != MAINMNU_QUIT && !crashflag);

VidDrv_printf("WARNING! WARNING! dropped out of game control loop!!!!\n");
exitt: ;
  Graphics_Stop();
ok: ;
  banner();
  VidDrv_printf("\nThanks for playing!\n\n");
  cleanup();
  if (crashflag)
  {
    if (crashflag != QUIT_NONFATAL) VidDrv_printf("\a");
    VidDrv_printf("abnormal program termination, error code %d/%d/%d\nexplanation: %s\n", crashflag,crashflag2,crashflag3, why_term_ptr);

    VidDrv_printf("numplayers: %d\n", numplayers);
    for(count=0,i=0;i<MAX_PLAYERS;i++) if (player[i].isPlaying) count++;
    VidDrv_printf("# of player instances with isPlaying set: %d\n", count);
    if (crashflag != QUIT_NONFATAL) goto startup_error;
  }
  return 0;
abort: ;
  VidDrv_printf("Fatal error, cleaning up.\n");
  cleanup();
//  VidDrv_printf("Press any key to exit.");
  goto startup_error;

startup_error: ;
  VidDrv_printf("A fatal error has occurred; game is shutting down.\n");
  #ifdef TARGET_WIN32
    if (!crashflag)
    {
       sprintf(msg, "A fatal error has occurred, check ck.log for details.\n");
    }
    else
    {
       sprintf(msg, "Abnormal program termination: code %d/%d/%d\n%s\n\nCloneKeen will now shut down. For details on this error, check ck.log.",crashflag,crashflag2,crashflag3,why_term_ptr);
    }
    MessageBoxA(NULL, msg, "CloneKeen main.c", MB_OK);
    MessageBoxA(NULL, "Sorry about that. When reporting this error please check ck.log and include any debug messages after the 'Game starting...' line.","CloneKeen",MB_OK);
  #endif

#ifdef TARGET_PSP
	sceKernelExitGame();
#endif

  return 1;
}


void playgame_levelmanager(int argc, char **argv, int dtm)
{
  int i, o, wm, firsttime = 1;
  char levelname[80];
  char SaveGameFileName[40];
  int newlevel;

  levelcontrol.command = LVLC_CHANGE_LEVEL;
  levelcontrol.chglevelto = WM_MAP_NUM;
  levelcontrol.tobonuslevel = 0;
  levelcontrol.success = 0;
  map.firsttime = 1;
  do
  {
    initgame();

    if (argc>1 && *argv[1] != '-' && dtm==0)
    {
      levelcontrol.chglevelto = atoi(argv[1]);
      if (!levelcontrol.chglevelto) levelcontrol.chglevelto = 1;
    }

    newlevel = levelcontrol.chglevelto;
    if (levelcontrol.episode==1 && options[OPT_MEAN].value)
    {
        // in high-difficulity mode switch levels 5 & 9 so
        // you can't get the pogo stick until you make it
        // to the dark side of mars.
        if (newlevel==5)
           newlevel = 9;
        else if (newlevel==9)
           newlevel = 5;
    }
    sprintf(levelname, "LEVEL%02d.CK%d", newlevel, levelcontrol.episode);

    if (levelcontrol.chglevelto==WORLD_MAP)
    {
      wm = 1;
    }
    else
    {
      wm = 0;
    }
    levelcontrol.canexit = 1;   // assume can exit before loading map
    if (loadmap(levelname, levelcontrol.chglevelto, wm))
    {
      crashflag = 1;
      crashflag2 = levelcontrol.chglevelto;
      why_term_ptr = "Unable to load the map (# shown in crashflag2).";
    }
    levelcontrol.curlevel = levelcontrol.chglevelto;
    if (levelcontrol.curlevel == FINAL_MAP)
    {
      levelcontrol.isfinallevel = 1;
      levelcontrol.canexit = 0;
    }
    else
    {
      levelcontrol.isfinallevel = 0;
    }

    if (firsttime)
    {
      for(i=0;i<MAX_PLAYERS;i++)
      {
        player[i].mapplayx = player[i].x;
        player[i].mapplayy = player[i].y;
      }
    }
    firsttime = 0;

    levelcontrol.command = LVLC_NOCOMMAND;

    levelcontrol.dark = 0;
    if (loadinggame)
    {
      sprintf(SaveGameFileName, "ep%csave%c.dat", levelcontrol.episode+'0', loadslot+'0');
      wm = savegameiswm(SaveGameFileName);
      if (game_load(SaveGameFileName))
      {
        crashflag = 1;
        crashflag2 = loadslot;
        why_term_ptr = "Error loading game (slot # in flag2)! The save file may be corrupt or created by a different version of CloneKeen.";
        return;
      }
    }
    pal_init(levelcontrol.dark);

    if (wm)
    {  // entering map from normal level, or first time around
      if (!levelcontrol.tobonuslevel)
      {
        if (!loadinggame)
        {
          for(i=0;i<MAX_PLAYERS;i++)
          {
            player[i].x = player[i].mapplayx;
            player[i].y = player[i].mapplayy;
          }
        }
      }
      else
      {  // respawn at the bonus level
        for(i=0;i<MAX_PLAYERS;i++)
        {
          player[i].x = BONUSLEVEL_RESPAWN_X;
          player[i].y = BONUSLEVEL_RESPAWN_Y;
          if (player[i].isPlaying && player[i].inventory.lives)
          {
             player[i].hideplayer = 1;
             o = spawn_object((player[i].x>>CSF>>4)<<CSF<<4,((player[i].y>>CSF>>4)+1)<<CSF<<4,OBJ_TELEPORTER);
             objects[o].ai.teleport.direction = TELEPORTING_IN;
             objects[o].ai.teleport.whichplayer = i;
             objects[o].ai.teleport.baseframe = TELEPORT_RED_BASEFRAME_EP1;
             objects[o].ai.teleport.idleframe = TELEPORT_RED_IDLEFRAME_EP1;
             sound_play(SOUND_TELEPORT, PLAY_NOW);
          }
        }
      }

      if (!levelcontrol.success || firsttime)
      {
        if (!levelcontrol.tobonuslevel) levelcontrol.dokeensleft = 1;
        // when you die you lose all keycards
        for(i=0;i<MAX_PLAYERS;i++)
        {
          if (player[i].isPlaying)
          {
            if (levelcontrol.episode!=3)
            {
              take_keycard(DOOR_YELLOW, i);
              take_keycard(DOOR_RED, i);
              take_keycard(DOOR_GREEN, i);
              take_keycard(DOOR_BLUE, i);
            }
            else
            {
              take_keycard(DOOR_YELLOW_EP3, i);
              take_keycard(DOOR_RED_EP3, i);
              take_keycard(DOOR_GREEN_EP3, i);
              take_keycard(DOOR_BLUE_EP3, i);
            }
          }
        }
      }
      else levelcontrol.dokeensleft = 0;

      gameloop();

      for(i=0;i<MAX_PLAYERS;i++)
      {
        player[i].mapplayx = player[i].x;
        player[i].mapplayy = player[i].y;
      }

    }
    else
    {  // entering a normal level from map
       levelcontrol.dokeensleft = 0;
       gameloop();

       // after completion of a normal level check if the game is won
       if (gameiswon())
       {
          levelcontrol.command = LVLC_END_SEQUENCE;
       }
    }
  } while(levelcontrol.command==LVLC_CHANGE_LEVEL && !crashflag);

  if (levelcontrol.command==LVLC_END_SEQUENCE)
  {
    endsequence();
    VidDrv_printf("eseq complete\n");
  }
  else if (levelcontrol.command==LVLC_TANTALUS_RAY)
  {
    eseq2_vibrate();
    eseq2_TantalusRay();
    IntroCanceled = 1;               // popup main menu immediately
  }
}

// plays the demo file specified in fname
// returns:
//  DEMO_RESULT_FILE_BAD               demo does not exist or file corrupt
//  DEMO_RESULT_COMPLETED              demo played all the way through
//  DEMO_RESULT_CANCELED               user canceled the demo
char play_demo(int demonum)
{
int i;
int byt;
int lvl;
char filename[40];
char SaveOptions[NUM_OPTIONS];

   /* open the demo file */
   sprintf(filename, "ep%ddemo%d.dat", levelcontrol.episode, demonum);
   demofile = fopen(filename, "rb");
   if (!demofile)
   {    
     return DEMO_RESULT_FILE_BAD;
   }

   /* read in the header */
   if (fgetc(demofile) != 'D') goto demoHeaderCorrupt;
   if (fgetc(demofile) != 'M') goto demoHeaderCorrupt;
   if (fgetc(demofile) != 'O') goto demoHeaderCorrupt;
   if (fgetc(demofile) != levelcontrol.episode) goto demoHeaderCorrupt;
   lvl = fgetc(demofile);

   /* load the compressed demo into the demo_data[] array */
   for(i=0;i<DEMO_MAX_SIZE;i++)
   {
     byt = fgetc(demofile);     // get byte from file
     if (byt<0) goto gotEOF;    // check for EOF
     demo_data[i] = byt;        // copy to demo_data[]
   }
   why_term_ptr = "Demo file is too big! (Increase DEMO_MAX_SIZE)";
   crashflag = 1;
gotEOF: ;
   fclose(demofile);
   demofile = NULL;

   /* initilize some variables */
   demo_RLERunLen = 0;
   demo_data_index = 0;
   demomode = DEMO_PLAYBACK;
   loadinggame = 0;
   levelcontrol.curlevel = lvl;
   levelcontrol.command = LVLC_NOCOMMAND;

   initgamefirsttime();
   initgame();

   /* now load the map and play the level */
   sprintf(filename, "level%02d.ck%d", levelcontrol.curlevel, levelcontrol.episode);
   if (loadmap(filename, levelcontrol.curlevel, 0)) return DEMO_RESULT_FILE_BAD;

   for(i=0;i<NUM_OPTIONS;i++) SaveOptions[i] = options[i].value;
   SetDefaultOptions();
   gameloop();
   for(i=0;i<NUM_OPTIONS;i++) options[i].value = SaveOptions[i];

   /* based on success/failure returned from gameloop let the
      calling procedure know whether the user canceled or not */

   if (levelcontrol.success)
   {
     return DEMO_RESULT_COMPLETED;
   }
   else
   {
     return DEMO_RESULT_CANCELED;
   }

// this label is jumped to when there's an error reading the header.
// it closes the demo file and aborts.
demoHeaderCorrupt: ;
   fclose(demofile);
   demofile = NULL;
   return DEMO_RESULT_FILE_BAD;
}
