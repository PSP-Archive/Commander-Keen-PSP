extern char PlatExtending;
extern stFade fade;
extern stMap map;
extern unsigned int AnimTileInUse[ATILEINUSE_SIZEX][ATILEINUSE_SIZEY];
extern stTile tiles[MAX_TILES+1];
extern char localmp;
unsigned char tiledata[MAX_TILES+1][16][16];
extern stSprite sprites[MAX_SPRITES+1];
extern stBitmap bitmaps[MAX_BITMAPS+1];
extern stAnimTile animtiles[MAX_ANIMTILES+1];
extern char font[MAX_FONT+1][8][8];
extern stObject objects[MAX_OBJECTS+1];
extern stPlayer player[MAX_PLAYERS];
extern stPlayer net_lastplayer[MAX_PLAYERS];
extern stOption options[NUM_OPTIONS];
extern unsigned char *scrollbuf;
extern unsigned char *blitbuf;
extern char keytable[KEYTABLE_SIZE+1];
extern char immediate_keytable[KEYTABLE_SIZE+1];
extern char last_immediate_keytable[KEYTABLE_SIZE+1];
extern int opx,opy;
extern int font_start;
extern char QuitState;

extern stString strings[MAX_STRINGS+1];
extern int numStrings;

extern int animtiletimer, curanimtileframe;

extern int thisplayer;
extern int primaryplayer;
extern int numplayers;

extern char frameskiptimer;

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

extern char *why_term_ptr;
int crashflag,crashflag2,crashflag3;

unsigned int objdefsprites[NUM_OBJ_TYPES+1];

extern int max_scroll_x, max_scroll_y;
extern int atilescount,aobjectscount;
extern int playerbaseframes[MAX_PLAYERS];
extern char debugmode,acceleratemode;

extern stLevelControl levelcontrol;

extern char loadinggame, loadslot;

extern char *BitmapData;

extern char ScreenIsScrolling;
extern int blockedby;
extern int gunfiretimer, gunfirefreq;
extern char cheatmode;

extern int NessieObjectHandle;
extern int DemoObjectHandle;
extern int BlankSprite;
extern int DemoSprite;
extern EgaHead LatchHeader;

extern int demomode;
extern FILE *demofile;
extern unsigned int demo_RLERunLen;
extern unsigned char demo_data[DEMO_MAX_SIZE+1];
extern unsigned int demo_data_index;
extern int framebyframe;

extern char otherplayer;

extern char is_server;
extern char is_client;
extern int fps, curfps;
extern char showfps;

extern char disable_fps_adjustment;
