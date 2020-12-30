# EMWM Makefile

PREFIX = /usr
MWMRCDIR = $(PREFIX)/share/X11/mwm
MANDIR = $(PREFIX)/share/man
APPLRESDIR = $(PREFIX)/share/X11/app-defaults

INCDIRS = -I./Xm -I/usr/local/include 
LIBDIRS = -L/usr/local/lib

DEFINES = -DLARGECURSORS -DUNMAP_ON_RESTART \
	-DNO_OL_COMPAT -DNO_MESSAGE_CATALOG \
	-DMWMRCDIR='"$(MWMRCDIR)"' -DNO_HP_KEY_REMAP

SYSLIBS =  -lm -lXm -lXt -lXext -lXrandr -lXinerama -lX11
CFLAGS := -O2 -Wall $(INCDIRS) $(DEFINES)
# CFLAGS := -O0 -g -Wall $(INCDIRS) $(DEFINES) -DDEBUG

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
	WmMenu.o	WmProtocol.o	WmXinerama.o \
	WmEwmh.o

wsm_objs = \
	WmWsmLib/debug.o WmWsmLib/disp.o\
	WmWsmLib/free.o WmWsmLib/pack.o\
	WmWsmLib/recv.o WmWsmLib/send.o\
	WmWsmLib/util.o WmWsmLib/utm_send.o

msg_cat = Mwm.msg
rc_data = system.mwmrc

emwm: $(mwm_objs) $(wsm_objs)
	$(CC) $(LIBDIRS) -o $@ $(mwm_objs) $(wsm_objs) $(SYSLIBS)


.PHONY: clean install

.depend:
	$(CC) -MM $(INCDIRS) $(mwm_objs:.o=.c) $(wsm_objs:.o=.c) > $@

clean:
	-rm -f $(mwm_objs) $(wsm_objs) emwm .depend

install:
	install -m 775 emwm $(PREFIX)/bin/emwm
	install -m 775 -d $(MANDIR)/man1
	install -m 664 emwm.1 $(MANDIR)/man1/emwm.1
	install -m 775 -d $(MWMRCDIR)
	if ! [ -f $(MWMRCDIR)/$(rc_data) ]; then \
	install -m 664 $(rc_data) $(MWMRCDIR)/$(rc_data); fi
	install -m 775 -d $(APPLRESDIR)
	if ! [ -f $(APPLRESDIR)/Mwm ]; then \
	install -m 664 Mwm.ad $(APPLRESDIR)/Mwm; fi

-include .depend
