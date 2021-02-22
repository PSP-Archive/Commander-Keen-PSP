TARGET = keen

PSP_EBOOT_SFO = PARAM.SFO
PSP_EBOOT_ICON = ICON0.png
PSP_EBOOT_ICON1 = NULL
PSP_EBOOT_UNKPNG = NULL
PSP_EBOOT_PIC1 = PIC1.PNG
PSP_EBOOT_SND0 = NULL
PSP_EBOOT_PSAR = NULL
PSP_EBOOT = EBOOT.PBP

OBJS = \
	eseq_ep1.o \
	eseq_ep2.o \
	eseq_ep3.o \
	fileio.o \
	finale.o \
	game.o \
	gamedo.o \
	gamepdo.o \
	gm_pdowm.o \
	graphics.o \
	keen.o \
	latch.o \
	lz.o \
	main.o \
	map.o \
	menu.o \
	misc.o \
	sgrle.o \
	\
	ai/baby.o \
	ai/balljack.o \
	ai/bear.o \
	ai/butler.o \
	ai/door.o \
	ai/earth.o \
	ai/fireball.o \
	ai/foob.o \
	ai/garg.o \
	ai/icebit.o \
	ai/icechunk.o \
	ai/meep.o \
	ai/mother.o \
	ai/nessie.o \
	ai/ninja.o \
	ai/platform.o \
	ai/platvert.o \
	ai/ray.o \
	ai/rope.o \
	ai/se.o \
	ai/sndwave.o \
	ai/tank.o \
	ai/tankep2.o \
	ai/teleport.o \
	ai/vort.o \
	ai/walker.o \
	ai/yorp.o \
	\
	sdl/keydrv.o \
	sdl/snddrv.o \
	sdl/timedrv.o \
	sdl/viddrv.o \
	\


INCDIR =
CFLAGS = -G0 -Wall -O2 -DBUILD_SDL -DTARGET_PSP
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lSDLmain -lSDL -lpsphprm -lpspaudio -lpspgum -lpspgu -lm -lstdc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Commander Keen

PSPSDK=/usr/local/pspdev/psp/sdk
include $(PSPSDK)/lib/build.mak

install: all
	psp-mscopy10 keen f

ifneq ($(VS_PATH),)
CC       = c:/programme/cygwin/usr/local/pspdev/bin/psp-gcc
CC       = c:/programme/cygwin/usr/local/pspdev/bin/psp-gcc
CXX      = c:/programme/cygwin/usr/local/pspdev/bin/psp-g++
AS       = c:/programme/cygwin/usr/local/pspdev/bin/psp-gcc
LD       = c:/programme/cygwin/usr/local/pspdev/bin/psp-gcc
AR       = c:/programme/cygwin/usr/local/pspdev/bin/psp-ar
RANLIB   = c:/programme/cygwin/usr/local/pspdev/bin/psp-ranlib
STRIP    = c:/programme/cygwin/usr/local/pspdev/bin/psp-strip
MKSFO    = c:/programme/cygwin/usr/local/pspdev/bin/mksfo
PACK_PBP = c:/programme/cygwin/usr/local/pspdev/bin/pack-pbp
FIXUP    = c:/programme/cygwin/usr/local/pspdev/bin/psp-fixup-imports
endif
