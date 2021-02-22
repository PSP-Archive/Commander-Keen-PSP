#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef TARGET_DOS
  #include <dpmi.h>
  #include <sys/farptr.h>
  #include <go32.h>
  #include <dos.h>
  #include "dos\timer.h"
#endif

#ifdef TARGET_WIN32
  #include <windows.h>
#endif

#include "sounds.h"
#include "funcdefs.h"
#include "latch.h"
#include "game.h"

#define CSF    5

//#define OVSIZE    3000

// when crashflag is activated by setting it to QUIT_NONFATAL,
// the application will immediately shut down, however the
// "a Fatal Error Occurred" message box will not pop up and
// the sysbeep will not sound.
#define QUIT_NONFATAL   555

#define REVISION        "CloneKeen Beta 8.3"
#define SAVEGAMEVERSION '4'
#define ATTRFILEVERSION  1

#define WM_MAP_NUM      80

#define MAX_TILES    700
#define MAX_SPRITES  300
#define MAX_FONT     256
#define MAX_BITMAPS  20

#define MAX_OBJECTS    100
#define MAX_ANIMTILES  200

#define PAL_FADE_SHADES   20
#define PAL_FADE_WHITEOUT 40
typedef struct stFade
{
 int mode;
 int dir;
 int curamt;
 int fadetimer;
 int rate;
} stFade;
#define NO_FADE         0
#define FADE_GO         1
#define FADE_COMPLETE   2

#define FADE_IN         1
#define FADE_OUT        2
#define FADE_NORM       3
#define FADE_FAST       1
#define FADE_SLOW       30

#define NO_QUIT                 0
#define QUIT_PROGRAM            1
#define QUIT_TO_TITLE           2

#define MAX_LEVELS     100
#define SCROLLBUF_XSIZE  512
#define SCROLLBUF_YSIZE  512
#define SCROLLBUF_MEMSIZE ((SCROLLBUF_XSIZE)*(SCROLLBUF_YSIZE+300))
#define SCROLLBUF_NUMTILESX (SCROLLBUF_XSIZE / 16)
#define SCROLLBUF_NUMTILESY (SCROLLBUF_YSIZE / 16)

#define BLITBUF_XSIZE  320
#define BLITBUF_YSIZE  200
#define BLITBUF_MEMSIZE ((BLITBUF_XSIZE)*(BLITBUF_YSIZE+30))
 // for each entry in the animtileinuse array that is nonzero, that
 // location on the display is an animated tile which is currently registered
 // in animtiles[]. Used in map_draw_hstripe and map_draw_vstripe.
 // When drawing a new stripe over one that previously contained an animated
 // tile, this lets it know it needs to unregister the animated tile that
 // used to be there. the nonzero value corresponds to the associated entry
 // in animtiles[]. the x,y pixel position is the index in here * 16.
 #define ATILEINUSE_SIZEX               33
 #define ATILEINUSE_SIZEY               33

 #define MAX_PLAYERS            8

 #define WORLD_MAP              80
 #define FINAL_MAP              16

 #define LVLC_NOCOMMAND         0
 #define LVLC_CHANGE_LEVEL      1
 #define LVLC_END_SEQUENCE      2
 #define LVLC_GAME_OVER         3
 #define LVLC_TANTALUS_RAY      4       // switch on tantalus ray pressed
typedef struct stLevelControl
{
 // level control
 int command;                 // used to give a command to playgame_levelmanager()
 int chglevelto;              // parameter to LVLC_CHANGE_LEVEL
 int tobonuslevel;            // if 1 player will warp to bonus level on return to WM (for ep1)
 // data about current level
 int curlevel;                // number of current level
 char success;                // 1 if level was finished, 0 if he died
 char isfinallevel;           // 1 if this is the final level
 char canexit;                // 1 if player is allowed to use the exit door
 char gameovermode;           // 1 if "Game Over" is displayed
 char dokeensleft;            // 1 if we need to do the "Keens Left"
 char dark;                   // 1 if level is currently dark (lights are out)

 int episode;                 // which episode we're playing (1-3)

 // array of which levels have been completed (have "Done" tiles over them
 // on the world map)
 int levels_completed[MAX_LEVELS+1];

 // exitXpos: the X pixel position (not <<CSFed) of the frame of the exit
 // door. when walking out the door, keen's sprite will not be drawn past
 // this point.
 unsigned int level_done, level_done_timer;
 unsigned int level_finished_by;      // index of player that finished level
 unsigned int exitXpos;
} stLevelControl;

#define OPT_EPISODE             0
#define OPT_FRAMESKIP           1
#define OPT_FULLYAUTOMATIC      2
#define OPT_SUPERPOGO           3
#define OPT_MEAN                4
#define OPT_ALLOWPKING          5
#define OPT_CHEATS              6
#define OPT_FULLSCREEN          7
#define OPT_ZOOM                8

#define NUM_OPTIONS             9
typedef struct stOption
{
  char *name;
  char value;
} stOption;

typedef struct stMap
{
 unsigned int xsize, ysize;            // size of the map
 unsigned char isworldmap;             // if 1, this is the world map
 unsigned int mapdata[256][256];       // the map data
 // in-game, contains monsters and special object tags like for switches 
 // on world map contains level numbers and flags for things like teleporters
 unsigned int objectlayer[256][256];
 char firsttime;  // used when generating multiplayer positions on world map
} stMap;
typedef struct stTile
{
 int solidfall;       // if =1, things can not fall through
 int solidl;          // if =1, things can not walk through left->right
 int solidr;          // if =1, things can not walk through right->left
 int solidceil;       // if =1, things can not go up through
 int goodie;          // if =1, is reported to get_goodie on touch
 int standgoodie;     // if =1, is reported to get_goodie when standing on it
 int lethal;          // if =1 and goodie=1, is deadly to the touch
 int pickupable;      // if =1, will be erased from map when touched
 int priority;        // if =1, will appear in front of objects
 int ice;             // if =1, it's very slippery!
 int semiice;         // if =1, player has no friction but can walk normally
 int masktile;        // if nonzero, specifies a mask for this tile
 int bonklethal;      // if you hit your head on it you die (hanging moss)
 int chgtile;         // tile to change to when level completed (for wm)
                      // or tile to change to when picked up (in-level)
 // stuff for animated tiles
 unsigned char isAnimated;  // if =1, tile is animated
 unsigned int animOffset;   // starting offset from the base frame
 unsigned int animmask;    // frame bitmask--determines animation length
} stTile;
typedef struct stBitmap
{
 int xsize;
 int ysize;
 unsigned char *bmptr;
 char name[9];
} stBitmap;
typedef struct stSprite
{
 char xsize, ysize;
 unsigned char imgdata[64][64];
 unsigned char maskdata[64][64];
 // bounding box for hit detection
 unsigned int bboxX1, bboxY1;
 unsigned int bboxX2, bboxY2;
} stSprite;

typedef struct stInventory
{
 unsigned long score;
 unsigned long extralifeat;
 unsigned int charges;        // ray gun ammo
 signed int lives;
 unsigned char HasPogo;
 unsigned char HasCardYellow;
 unsigned char HasCardRed;
 unsigned char HasCardGreen;
 unsigned char HasCardBlue;
 // ep1 only
 unsigned char HasJoystick;
 unsigned char HasFuel;
 unsigned char HasBattery;
 unsigned char HasVacuum;
} stInventory;

// for strings loaded from "strings.dat"
#define MAX_STRINGS             100
#define MAX_ATTRIBUTES          16
typedef struct stString
{
  unsigned char *name;    // pointer to malloc'd area containing string name
  unsigned char *stringptr;    // pointer to malloc'd area containing string

  int numAttributes;
  unsigned char *attrnames[MAX_ATTRIBUTES+1];
  unsigned int attrvalues[MAX_ATTRIBUTES+1];
} stString;

/* Structs used for different enemies data, these are in a union */
typedef struct stYorpData
{
  unsigned char state;

  unsigned char looktimes,lookposition;
  unsigned char timer, dietimer;
  unsigned char walkframe;
  unsigned int dist_traveled;
  signed int yorpdie_inertia_y;

  unsigned char movedir;
} stYorpData;
typedef struct stGargData
{
  unsigned char state;

  unsigned char looktimes,lookframe;
  unsigned char timer, dietimer, keenonsameleveltimer;
  unsigned char about_to_charge;
  unsigned char walkframe;
  unsigned int dist_traveled;
  signed int gargdie_inertia_y;

  unsigned char movedir;
  unsigned char detectedPlayer, detectedPlayerIndex;
} stGargData;
typedef struct stVortData
{
  unsigned char state;

  unsigned char timer,timer2;
  unsigned int animtimer;
  unsigned char palflashtimer, palflashamt;
  unsigned char frame;
  unsigned int dist_traveled;
  signed int inertiay;

  char ep1style;                // episode 1 style four-shots-to-kill

  unsigned char movedir;
  // these hold the animation frames indexes since they're
  // different for each episode
  int WalkLeftFrame;
  int WalkRightFrame;
  int LookFrame;
  int JumpRightFrame;
  int JumpLeftFrame;
  int DyingFrame;
  int DeadFrame;
} stVortData;
typedef struct stBearData
{
  unsigned char state;

  unsigned char timer,timer2;
  unsigned int animtimer;
  unsigned char frame;
  signed int inertiay;
  unsigned char movedir;
  unsigned int timesincefire;

  int dist_traveled;
} stBearData;
typedef struct stButlerData
{
  unsigned char state;
  unsigned char timer,animtimer;
  unsigned char frame;
  unsigned int dist_traveled;

  unsigned char movedir;
} stButlerData;
typedef struct stTankData
{
  unsigned char state;

  unsigned char timer,animtimer;
  unsigned char frame;
  unsigned int dist_traveled;

  unsigned char movedir;

  int ponsameleveltime;
  unsigned char alreadyfiredcauseonsamelevel;
  unsigned char fireafterlook;

  unsigned char detectedPlayer;         // 1 if player on same level
  unsigned char detectedPlayerIndex;    // index of player that was detected

  // for tank2
  unsigned int timetillnextshot;
  unsigned int firetimes;
  unsigned int timetillcanfire;
  unsigned int timetillcanfirecauseonsamelevel;
} stTankData;
typedef struct stRayData
{
  char state;
  char direction;
  char zapzottimer;

  char dontHitEnable;
  int dontHit;         // index of an object type ray will not harm

  // for soundwave
  int animframe, animtimer;
  int offscreentime;

  // for earth chunks
  int baseframe;
} stRayData;
typedef struct stDoorData
{
  char timer;
  char distance_traveled;
} stDoorData;
typedef struct stIceChunk
{
  char movedir;
  char state;
  unsigned int originalX, originalY;
  int timer;
} stIceChunk;
typedef struct stTeleportData
{
  char animtimer;
  char animframe;
  char numframechanges;

  char direction;
  int whichplayer;
  unsigned int destx;
  signed int desty;

  int baseframe;
  int idleframe;

  char NoExitingTeleporter;
  char snap;

  char fadeamt;
  char fadetimer;
} stTeleportData;
typedef struct stRopeData
{
  char state;
  int droptimer;
  int droptimes;
  int stoneX, stoneY;
  int vortboss;
} stRopeData;

typedef struct stWalkerData
{
  unsigned char state;

  unsigned char animtimer, dietimer;
  unsigned char walkframe;
  signed int walkerdie_inertia_y;
  int fallinctimer,fallspeed;

  unsigned char walkdir;
  unsigned char kickedplayer[MAX_PLAYERS];
} stWalkerData;

typedef struct stPlatformData
{
  unsigned char state;
  unsigned char animframe;
  unsigned int animtimer;
  unsigned int waittimer;

  unsigned char movedir;
  unsigned char kickedplayer[MAX_PLAYERS];
} stPlatformData;

typedef struct stSEData
{
  unsigned int type;

  unsigned char state;
  unsigned int timer;
  unsigned int platx, platy;
  unsigned int bgtile;
  unsigned int dir;

  int counter,destroytiles;
  unsigned int frame;
  int mx,my;
  int blowx,blowy;
} stSEData;

typedef struct stBabyData
{
  char state;
  char dir;
  signed int inertia_x, inertia_y;
  int jumpdectimer, xdectimer;
  int jumpdecrate;
  int dietimer;

  char walkframe;
  int walktimer;
} stBabyData;

typedef struct stFoobData
{
  char state;
  char dir;

  int animframe, animtimer;
  int OnSameLevelTime;
  int OffOfSameLevelTime;
  int spooktimer;
  int SpookedByWho;
} stFoobData;

typedef struct stNinjaData
{
  char state;
  char dir;

  int animframe, animtimer;
  unsigned int timetillkick;

  signed int XInertia, YInertia;
  unsigned int XFrictionTimer, YFrictionTimer;
  unsigned int XFrictionRate, YFrictionRate;
  int KickMoveTimer;
  int isdying;
  int dietimer;
} stNinjaData;

typedef struct stMotherData
{
  char state;
  char dir;
  char hittimes;

  int animframe, animtimer;
  int timer;
} stMotherData;

typedef struct stMeepData
{
  char state;
  char dir;

  int animframe, animtimer;
  int timer;
} stMeepData;

typedef struct stBallJackData
{
  char dir;
  int animframe, animtimer;
  int speed;
} stBallJackData;

#define NESSIETRAILLEN   5
typedef struct stNessieData
{
  char state;
  char leftrightdir, updowndir;
  unsigned int baseframe;

  int tiletrailX[NESSIETRAILLEN+1];
  int tiletrailY[NESSIETRAILLEN+1];
  int tiletrailhead;

  char animframe, animtimer;
  unsigned int destx, desty;

  unsigned int pausetimer;
  unsigned int pausex, pausey;

  unsigned int mortimer_swim_amt;
  unsigned int mounted[MAX_PLAYERS];
} stNessieData;

// and the object structure containing the union of the above structs
typedef struct stObject
{
 unsigned int type;        // yorp, vorticon, etc.
 unsigned int exists;
 unsigned int onscreen;    // 1=(scrx,scry) position is visible onscreen
 unsigned int hasbeenonscreen;
 unsigned int sprite;      // which sprite should this object be drawn with
 unsigned int x, y;        // x,y location in map coords, CSFed
 int scrx, scry;           // x,y pixel position on screen

 // if type is OBJ_PLAYER, this contains the player number that this object
 // is associated with
 int AssociatedWithPlayer;

 // if zero, priority tiles will not be honored and object will always
 // appear in front of the background
 char honorPriority;

 char canbezapped;         // if 0 ray will not stop on hitdetect
 char zapped;              // number of times got hit by keen's raygun

 char inhibitfall;         // if 1 common_enemy_ai will not do falling
 char cansupportplayer[MAX_PLAYERS];
 
 unsigned int blockedl, blockedr, blockedu, blockedd;
 signed int xinertia, yinertia;
 unsigned char xinertiatimer, yinertiatimer;

 unsigned char touchPlayer;      // 1=hit detection with player
 unsigned char touchedBy;        // which player was hit
 // Y position on this object the hit was detected
 // this is used for the yorps' bonk-on-the-head thing.
 // objects are scanned bottom to top, and first pixel
 // touching player is what goes in here.
 unsigned char hity;

 unsigned int needinit;    // 1=new object--requires initilization
 unsigned char wasoffscreen;  // set to 1 when object goes offscreen
 // data for ai and such, used differently depending on
 // what kind of object it is
 union ai
 {
   // ep1
   stYorpData yorp;
   stGargData garg;
   stVortData vort;
   stButlerData butler;
   stTankData tank;
   stRayData ray;
   stDoorData door;
   stIceChunk icechunk;
   stTeleportData teleport;
   stRopeData rope;
   // ep2
   stWalkerData walker;
   stPlatformData platform;
   stBearData bear;
   stSEData se;
   stBabyData baby;
   // ep3
   stFoobData foob;
   stNinjaData ninja;
   stMeepData meep;
   stMotherData mother;
   stBallJackData bj;
   stNessieData nessie;
 } ai;
 unsigned char erasedata[64][64];   // backbuffer to erase this object
} stObject;

// (map) stripe attribute structures, for animated tiles
// slot 0 is not used. data starts at slot 1. see description
// of AnimTileInUse in map structure to see why.
typedef struct stAnimTile
{
  int slotinuse;        // if 0, this entry should not be drawn
  int x;                // x pixel position in scrollbuf[] where tile is
  int y;                // y pixel position in scrollbuf[]
  int baseframe;        // base frame, i.e. the first frame of animation
  int offset;           // offset from base frame
} stAnimTile;

#define NUM_OBJ_TYPES      40
// ** objects from KEEN1
#define OBJ_YORP           1
#define OBJ_GARG           2
#define OBJ_VORT           3
#define OBJ_BUTLER         4
#define OBJ_TANK           5
#define OBJ_RAY            6     // keen's raygun blast
#define OBJ_DOOR           7     // an opening door
#define OBJ_ICECHUNK       8     // ice chunk from ice cannon
#define OBJ_ICEBIT         9     // piece of shattered ice chunk
#define OBJ_PLAYER         10
#define OBJ_TELEPORTER     11    // world map teleporter
#define OBJ_ROPE           12

// ** objects from KEEN2 (some of these are in ep3 as well)
#define OBJ_WALKER               13
#define OBJ_TANKEP2              14
#define OBJ_PLATFORM             15
#define OBJ_BEAR                 16
#define OBJ_SECTOREFFECTOR       17
#define OBJ_BABY                 18
#define OBJ_EXPLOSION            19
#define OBJ_EARTHCHUNK           20

// ** objects from KEEN3
#define OBJ_FOOB                 21
#define OBJ_NINJA                22
#define OBJ_MEEP                 23
#define OBJ_SNDWAVE              24
#define OBJ_MOTHER               25
#define OBJ_FIREBALL             26
#define OBJ_BALL                 27
#define OBJ_JACK                 28
#define OBJ_PLATVERT             29
#define OBJ_NESSIE               30

#define OBJ_DEMOMSG              31

// default sprites...when an object is spawned it's sprite is set to this
// sprite. the object AI will immediately reset the sprite frame, so it
// wouldn't really matter what these are...except that it does because
// the width and height of the default sprite will determine exactly when
// the object because active the first time it scrolls onto the screen
// from the top or left. if the default sprite is wrong the object may
// suddenly appear on the screen instead of smoothly scrolling on.
#define OBJ_YORP_DEFSPRITE       50
#define OBJ_GARG_DEFSPRITE       60
#define OBJ_VORT_DEFSPRITE_EP1   78
#define OBJ_VORT_DEFSPRITE_EP2   82
#define OBJ_VORT_DEFSPRITE_EP3   71
#define OBJ_BUTLER_DEFSPRITE     88
#define OBJ_TANK_DEFSPRITE       98
#define OBJ_RAY_DEFSPRITE_EP1    108
#define OBJ_RAY_DEFSPRITE_EP2    122
#define OBJ_RAY_DEFSPRITE_EP3    102
#define OBJ_ICECHUNK_DEFSPRITE   112
#define OBJ_ICEBIT_DEFSPRITE     113
#define OBJ_TELEPORTER_DEFSPRITE 180
#define OBJ_ROPE_DEFSPRITE       184

#define OBJ_PLATFORM_DEFSPRITE_EP2   126
#define OBJ_PLATFORM_DEFSPRITE_EP3   107
#define OBJ_WALKER_DEFSPRITE     102
#define OBJ_TANKEP2_DEFSPRITE    112
#define OBJ_BEAR_DEFSPRITE       88

#define OBJ_FOOB_DEFSPRITE       95
#define OBJ_NINJA_DEFSPRITE      77
#define OBJ_MOTHER_DEFSPRITE     87
#define OBJ_BJ_DEFSPRITE         109
#define OBJ_MEEP_DEFSPRITE       118

#define OBJ_BABY_DEFSPRITE_EP2   52
#define OBJ_BABY_DEFSPRITE_EP3   51

// key defines, locations in keytable[]
#define KEYTABLE_SIZE            53
#define KEYTABLE_REALKEYS_SIZE   49

#define KQUIT         0
#define KLEFT         1
#define KRIGHT        2
#define KUP           3
#define KDOWN         4
#define KCTRL         5   // simply, CTRL, mapped to JUMP or FIRE
#define KALT          6   // simply, ALT, mapped to POGO or fire
#define KENTER        7
#define KSPACE        8
#define KF1           9
#define KF2           10
#define KF3           11
#define KF4           12
#define KF5           13
#define KF6           14
#define KF7           15
#define KF8           16
#define KF9           17
#define KF10          18

#define KLEFT2        19
#define KRIGHT2       20
#define KUP2          21
#define KDOWN2        22
#define KCTRL2        23
#define KALT2         24

#define KLEFT3        25
#define KRIGHT3       26
#define KUP3          27
#define KDOWN3        28
#define KCTRL3        29
#define KALT3         30

#define KPLUS         31
#define KMINUS        32

#define KNUM1         33
#define KNUM2         34
#define KNUM3         35
#define KNUM4         36
#define KNUM5         37
#define KNUM6         38
#define KNUM7         39
#define KNUM8         40
#define KNUM9         41

#define KY            42
#define KN            43
#define KP            44
#define KT            45
#define KQ            46
#define KC            47
#define KTAB          48

// cheat code activators compiled by keydrv.c
#define KGOD          49

// these aren't real keys, they're mapped from the
// above real keys (e.g. KCTRL+KALT == KFIRE). used only in
// the player[cp].keytable structures, not used in immediate_keytable[].
#define KJUMP         50
#define KPOGO         51
#define KFIRE         52

// some directions (mostly for OBJ_ICECHUNK and OBJ_ICEBIT)
#define DUPRIGHT         0
#define DUPLEFT          1
#define DUP              2
#define DDOWN            3
#define DDOWNRIGHT       4
#define DDOWNLEFT        5
#define DLEFT            6
#define DRIGHT           7

// directions for OBJ_EARTHCHUNK
#define EC_UPLEFTLEFT       0             // 22 degrees CC of UP/LEFT
#define EC_UPUPLEFT         1             // 22 degrees C of UP/LEFT
#define EC_UP               2             // straight UP
#define EC_UPUPRIGHT        3             // 22 degrees CC of UP/RIGHT
#define EC_UPRIGHTRIGHT     4             // 22 degrees C of UP/RIGHT
#define EC_DOWNLEFTLEFT     5             // 22 degrees CC of DOWN/LEFT
#define EC_DOWNDOWNLEFT     6             // 22 degrees C of DOWN/LEFT
#define EC_DOWN             7             // straight DOWN
#define EC_DOWNDOWNRIGHT    8             // 22 degrees CC of DOWN/RIGHT
#define EC_DOWNRIGHTRIGHT   9             // 22 degrees C of DOWN/RIGHT

#define EC_UPLEFT           10
#define EC_UPRIGHT          11
#define EC_DOWNLEFT         12
#define EC_DOWNRIGHT        13

// scroll triggers
#define SCROLLTRIGGERRIGHT     194
#define SCROLLTRIGGERLEFT      110
#define SCROLLTRIGGERUP        80
#define SCROLLTRIGGERDOWN      114

// this structure contains all the variables used by a player
typedef struct stPlayer
{
   // these coordinates are CSFed
   unsigned long x;
   signed int y;

   char isPlaying;
   int useObject;

   char godmode;

   // used on world map only
   char hideplayer;
   char mounted;
   signed int pinertia_y;

   unsigned long mapplayx;
   signed int mapplayy;
   
   unsigned char playframe;

   unsigned char pfalling,plastfalling,pfallspeed,pfallspeed_increasetimer;

   unsigned char pwalking,playspeed;
   unsigned char pwalkframe,pwalkframea,pwalkanimtimer;
   unsigned char pwalkincreasetimer, pfriction_timer_x, pfriction_timer_y;
   signed int pinertia_x,playpushed_x;
   unsigned char playpushed_decreasetimer;

   unsigned char blockedl,blockedr,blockedu,blockedd;
   unsigned int blockedby;

   unsigned char pjumping, pjumptime, pjumpupspeed_decreasetimer;
   unsigned char pjumpframe, pjumpanimtimer, pjumpupspeed;
   unsigned char pjumpnormaltime, pjumpupdecreaserate, pjustjumped;
   unsigned char pjustfell;
   unsigned char pjumpfloattimer;

   unsigned char pdir,pshowdir,lastpdir;

   char pfiring,pfireframetimer;
   char inhibitwalking, inhibitfall;

   int ctrltimer, alttimer;
   char keyprocstate;
   char wm_lastenterstate;

   char pdie, pdieframe, pdietimer;
   int pdietillfly;
   signed int pdie_xvect;
   int psupportingtile, psupportingobject, lastsupportingobject;
   char psliding;
   char psemisliding;
   char ppogostick;
   int pfrozentime,pfrozenframe,pfrozenanimtimer;

   unsigned char keytable[KEYTABLE_SIZE+1];
   unsigned char lastkeytable[KEYTABLE_SIZE+1];
   unsigned char dpadcount, dpadlastcount;

   unsigned int ankhtime, ankhshieldobject;

   stInventory inventory;
} stPlayer;

#define TILE_LITTLE_DONE     77
#define TILE_BIG_DONE1       78
#define TILE_BIG_DONE2       79
#define TILE_BIG_DONE3       80
#define TILE_BIG_DONE4       81

#define TILE_TELEPORTER_GREY_IDLE  99
#define TILE_TELEPORTER_RED_INUSE  338

// special level codes on worldmap
#define LVLS_TELEPORTER_BONUS      46            // bonus teleporter in ep1
#define LVLS_SHIP                  20

#define TELEPORTING_OUT         0
#define TELEPORTING_IN          1

#define TELEPORT_BONUS_DESTX    ((((23085>>CSF>>4)+2)<<4<<CSF)-(8<<CSF))
#define TELEPORT_BONUS_DESTY    (((12501>>CSF>>4)+2)<<4<<CSF)

#define BONUSLEVEL_RESPAWN_X    31812
#define BONUSLEVEL_RESPAWN_Y    18936

    #define MAINMNU_1PLAYER      0
    #define MAINMNU_2PLAYER      1
    #define MAINMNU_LOADGAME     2
    #define MAINMNU_OPTIONS      3
    #define MAINMNU_DEMO         4
    #define MAINMNU_BLANKSPOT    5
    #define MAINMNU_QUIT         6
    #define MAINMNU_TIMEOUT      10     // going to demo due to timeout
    #define RESTART_GAME         11

    #define MAINMENU_NUM_OPTIONS        MAINMNU_QUIT

#define TILE_SWITCH_UP             480
#define TILE_SWITCH_DOWN           493
#define TILE_LIGHTSWITCH           271
#define TILE_EXTENDING_PLATFORM    270

// "Sector Effector" types
#define SE_EXTEND_PLATFORM      1
#define SE_RETRACT_PLATFORM     2
#define SE_SPARK                3
#define SE_GUN_VERT             4
#define SE_GUN_RIGHT            5
#define SE_ANKHSHIELD           6
#define SE_ICECANNON            7
#define SE_MORTIMER_ARM         8
#define SE_MORTIMER_LEG_LEFT    9
#define SE_MORTIMER_LEG_RIGHT   10
#define SE_MORTIMER_SPARK       11
#define SE_MORTIMER_HEART       12
#define SE_MORTIMER_ZAPSUP      13
#define SE_MORTIMER_RANDOMZAPS  14

// animation rate of animated tiles
#define ANIM_TILE_TIME        50


#define TELEPORT_GRAY_BASEFRAME_EP1  342
#define TELEPORT_GRAY_IDLEFRAME_EP1  99

#define TELEPORT_RED_BASEFRAME_EP1   338
#define TELEPORT_RED_IDLEFRAME_EP1   325

#define TELEPORT_BASEFRAME_EP3  130
#define TELEPORT_IDLEFRAME_EP3  134

// special object markers
#define NESSIE_PATH             8192
#define NESSIE_PAUSE            8448
#define NESSIE_MOUNTPOINT       8704
#define GARG_STOPPOINT          1000
#define BALL_NOPASSPOINT        1001

// values for demomode global variable
#define DEMO_NODEMO             0
#define DEMO_RECORD             1
#define DEMO_PLAYBACK           2

#define DEMO_MAX_SIZE           2048

#define DEMO_RESULT_COMPLETED    0
#define DEMO_RESULT_CANCELED     1
#define DEMO_RESULT_FILE_BAD     2



#include "keenext.h"

