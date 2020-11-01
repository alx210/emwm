# $Id: Makefile,v 1.6 2018/12/13 20:20:41 alx Exp $
# XMWM Makefile

PREFIX = /usr/local

INCDIRS = -I./Xm -I/usr/local/include
LIBDIRS = -L/usr/local/lib

DEFINES = -DLARGECURSORS -DR2_COMPAT -DUNMAP_ON_RESTART \
	-DNO_OL_COMPAT -DNO_MESSAGE_CATALOG \
	-DMWMRCDIR='"$(PREFIX)/lib/X11"'

SYSLIBS =  -lXm -lXt -lXext -lXinerama -lX11
CFLAGS := -O2 -Wall $(INCDIRS) $(DEFINES)

mwm_objs = \
	WmCDInfo.o	WmCDecor.o	WmCEvent.o\
	WmCPlace.o	WmColormap.o	WmError.o\
	WmEvent.o	WmFeedback.o	WmFunction.o\
	WmGraphics.o	WmIDecor.o	WmIPlace.o\
	WmIconBox.o	WmKeyFocus.o	WmMain.o\
	WmManage.o	WmProperty.o	WmResCvt.o\
	WmResParse.o	WmResource.o	WmSignal.o\
	WmWinConf.o	WmWinInfo.o	WmWinList.o\
	WmWinState.o	WmWsm.o		WmXSMP.o\
	WmCmd.o 	WmImage.o	WmInitWs.o\
	WmMenu.o	WmProtocol.o	version.o \
	WmXinerama.o

wsm_objs = \
	WmWsmLib/debug.o WmWsmLib/disp.o\
	WmWsmLib/free.o WmWsmLib/pack.o\
	WmWsmLib/recv.o	WmWsmLib/send.o\
	WmWsmLib/util.o WmWsmLib/utm_send.o

msg_cat = Mwm.msg
rc_data = system.mwmrc


emwm: $(mwm_objs) $(wsm_objs)
	$(CC) $(LIBDIRS) -o $@ $(mwm_objs) $(wsm_objs) $(SYSLIBS)

depend:
	mkdep $(CFLAGS) $(mwm_objs:.o=.c) $(wsm_objs:.o=.c)

clean:
	-rm -f $(mwm_objs) $(wsm_objs) emwm

install:
	install -m 775 emwm $(PREFIX)/bin/emwm
	install -m 664 emwm.1 $(PREFIX)/man/man1/emwm.1
	install -m 664 $(rc_data) $(PREFIX)/lib/X11/$(rc_data)
