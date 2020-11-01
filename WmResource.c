/* 
 * Motif
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: WmResource.c /main/14 1997/04/15 10:30:02 dbl $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993, 1994 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"

#define MWM_NEED_IIMAGE
#include "WmIBitmap.h"

#include <stdio.h>

#include <Xm/XmP.h>
#include <Xm/RowColumn.h>
#ifndef MOTIF_ONE_DOT_ONE
#include <Xm/ScreenP.h>		/* for XmGetXmScreen and screen.moveOpaque */
#endif

/*
 * include extern functions
 */
#include "WmResource.h"
#include "WmError.h"
#include "WmGraphics.h"
#include "WmMenu.h"
#include "WmResParse.h"
#ifdef WSM
#include "WmBackdrop.h"
#include "WmIconBox.h"
#include "WmWrkspace.h"
#include <Dt/GetDispRes.h>
#define cfileP 	(wmGD.pWmPB->pFile) /* fopen'ed configuration file or NULL */
#endif /* WSM */
#include "WmXSMP.h"

/*
 * Function Declarations:
 */
XmColorData *_WmGetDefaultColors ();

void _WmTopShadowPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageFDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBSCDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageBSPDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageTSCDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageTSPDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteFDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBSCDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteBSPDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteTSCDefault (Widget widget, int offset, XrmValue *value);
void _WmMatteTSPDefault (Widget widget, int offset, XrmValue *value);
void _WmBackgroundDefault (Widget widget, int offset, XrmValue *value);
void _WmForegroundDefault (Widget widget, int offset, XrmValue *value);
void _WmBackgroundPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmBottomShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmTopShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmABackgroundDefault (Widget widget, int offset, XrmValue *value);
void _WmAForegroundDefault (Widget widget, int offset, XrmValue *value);
void _WmABackgroundPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmABottomShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmATopShadowColorDefault (Widget widget, int offset, XrmValue *value);
void _WmATopShadowPixmapDefault (Widget widget, int offset, XrmValue *value);
void _WmFocusAutoRaiseDefault (Widget widget, int offset, XrmValue *value);
void _WmMultiClickTimeDefault (Widget widget, int offset, XrmValue *value);
void ProcessWmResources (void);
void ProcessGlobalScreenResources (void);
void SetStdGlobalResourceValues (void);
void ProcessScreenListResource (void);
void ProcessAppearanceResources (WmScreenData *pSD);
void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
void ProcessWorkspaceResources (WmWorkspaceData *pWS);
void ProcessClientResources (ClientData *pCD);
void SetStdClientResourceValues (ClientData *pCD);
void SetStdScreenResourceValues (WmScreenData *pSD);
GC GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
#ifdef WSM
static void WriteOutXrmColors (WmScreenData *pSD);
#endif /* WSM */
#ifdef WSM
void ProcessPresenceResources (WmScreenData *pSD);
void ProcessDefaultBackdropImages (WmScreenData *pSD);
void _WmBackdropBgDefault (Widget widget, int offset, XrmValue *value);
void _WmBackdropFgDefault (Widget widget, int offset, XrmValue *value);
void _WmBackdropColorSetDefault (Widget widget, int offset, XrmValue *value);
void _WmIconImageMaximumDefault (Widget widget, int offset, XrmValue *value);
void _WmSecondariesOnTopDefault (Widget widget, int offset, XrmValue *value);
int DefaultWsColorSetId (WmWorkspaceData *pWS);
#endif /* WSM */
void _WmGetDynamicDefault (Widget widget, unsigned char type, String defaultColor, Pixel newBackground, XrmValue *value);
Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);



/*
 * Global Variables:
 */

/* builtin window menu specification */

#ifndef NO_MESSAGE_CATALOG
/*
 * Use the same name as builtin to let the message catalog menu
 * take precedence over any menus that might match in sys.mwmrc
 */
char defaultSystemMenuName[] = "_MwmWindowMenu_";
#else
char defaultSystemMenuName[] = "DefaultWindowMenu";
#endif	/* NO_MESSAGE_CATALOG */
char builtinSystemMenuName[] = "_MwmWindowMenu_";
#ifndef MCCABE
#define BUILTINSYSTEMMENU "_MwmWindowMenu_\n\
{\n\
	Restore		_R	Alt<Key>F5	f.restore\n\
	Move		_M	Alt<Key>F7	f.move\n\
	Size		_S	Alt<Key>F8	f.resize\n\
	Minimize	_n	Alt<Key>F9	f.minimize\n\
	Maximize	_x	Alt<Key>F10	f.maximize\n\
	Lower		_L	Alt<Key>F3	f.lower\n\
	no-label				f.separator\n\
	Close		_C	Alt<Key>F4	f.kill\n\
}"
#ifdef NO_MESSAGE_CATALOG
char builtinSystemMenu[] = BUILTINSYSTEMMENU;
#else /* !defined(NO_MESSAGE_CATALOG)*/
char *builtinSystemMenu = BUILTINSYSTEMMENU;
#ifdef WSM
#define DEFAULT_DTWM_SYSTEMMENU "_MwmWindowMenu_\n\
{\n\
	Restore		_R	f.restore\n\
	Move		_M	f.move\n\
	Size		_S	f.resize\n\
	Minimize	_n	f.minimize\n\
	Maximize	_x	f.maximize\n\
	Lower		_L	f.lower\n\
	no-label		f.separator\n\
      \"Occupy Workspace...\"	_O	f.workspace_presence\n\
      \"Occupy All Workspaces\"	_A	f.occupy_all\n\
      \"Unoccupy Workspace\"	_U	f.remove\n\
	no-label			f.separator\n\
	Close	_C	Alt<Key>F4	f.kill\n\
}"
#endif /* WSM */

void InitBuiltinSystemMenu(void)
{
    char * tmpString;
    char *ResString = NULL;
    char *MovString = NULL;
    char *SizString = NULL;
    char *MinString = NULL;
    char *MaxString = NULL;
    char *LowString = NULL;
#ifdef WSM
    char *OcpString = NULL;
    char *OcaString = NULL;
    char *RemString = NULL;
#endif /* WSM */
    char *CloString = NULL;
    char dsm[2048];
    Boolean gotItAll;
    gotItAll = True;
    if(gotItAll)
    {
#if 1
        tmpString = ((char *)GETMESSAGE(62, 60, "Restore _R  Alt<Key>F5 f.restore"));
#else
        tmpString = ((char *)GETMESSAGE(62, 49, "Restore _R  f.restore"));
#endif
        if ((ResString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 2, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(ResString, tmpString);
        }
    }
    if(gotItAll)
    {
#if 1
        tmpString = ((char *)GETMESSAGE(62, 61, "Move _M  Alt<Key>F7 f.move"));
#else
        tmpString = ((char *)GETMESSAGE(62, 50, "Move _M  f.move"));
#endif
        if ((MovString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 4, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(MovString, tmpString);
        }
    }
    if(gotItAll)
    {
#if 1
        tmpString = ((char *)GETMESSAGE(62, 62, "Size _S  Alt<Key>F8 f.resize"));
#else
        tmpString = ((char *)GETMESSAGE(62, 51, "Size _S  f.resize"));
#endif
        if ((SizString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 6, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(SizString, tmpString);
        }
    }
    if(gotItAll)
    {
#if 1
        tmpString = ((char *)GETMESSAGE(62, 63, "Minimize _n  Alt<Key>F9 f.minimize"));
#else
        tmpString = ((char *)GETMESSAGE(62, 52, "Minimize _n  f.minimize"));
#endif
        if ((MinString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 8, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(MinString, tmpString);
        }
    }
    if(gotItAll)
    {
#if 1
	tmpString = ((char *)GETMESSAGE(62, 64, "Maximize _x  Alt<Key>F10 f.maximize"));
#else
	tmpString = ((char *)GETMESSAGE(62, 53, "Maximize _x  f.maximize"));
#endif
	if ((MaxString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 10, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(MaxString, tmpString);
        }
    }
    if(gotItAll)
    {
#if 1
        tmpString = ((char *)GETMESSAGE(62, 65, "Lower _L  Alt<Key>F3 f.lower"));
#else
        tmpString = ((char *)GETMESSAGE(62, 54, "Lower _L  f.lower"));
#endif
        if ((LowString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 12, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(LowString, tmpString);
        }
    }
#ifdef WSM
    if (DtwmBehavior)
    {
	if(gotItAll)
	{
	    tmpString = ((char *)GETMESSAGE(62, 55, "Occupy\\ Workspace\\.\\.\\. _O  f.workspace_presence"));
	    if ((OcpString =
		 (char *)XtMalloc ((unsigned int) 
				 (strlen(tmpString) + 1))) == NULL)
	    {
		Warning (((char *)GETMESSAGE(62, 14, "Insufficient memory for local default menu.")));
		gotItAll = False;
	    }
	    else
	    {
		strcpy(OcpString, tmpString);
	    }
	}
	if(gotItAll)
	{
	    tmpString = ((char *)GETMESSAGE(62, 56, "Occupy\\ All\\ Workspaces _A  f.occupy_all"));
	    if ((OcaString =
		 (char *)XtMalloc ((unsigned int) 
				 (strlen(tmpString) + 1))) == NULL)
	    {
		Warning (((char *)GETMESSAGE(62, 16, "Insufficient memory for local default menu.")));
		gotItAll = False;
	    }
	    else
	    {
		strcpy(OcaString, tmpString);
	    }
	}
	if(gotItAll)
	{
	    tmpString = ((char *)GETMESSAGE(62, 57, "Unoccupy\\ Workspace _U  f.remove"));
	    if ((RemString =
		 (char *)XtMalloc ((unsigned int) 
				 (strlen(tmpString) + 1))) == NULL)
	    {
		Warning (((char *)GETMESSAGE(62, 18, "Insufficient memory for local default menu.")));
		gotItAll = False;
	    }
	    else
	    {
		strcpy(RemString, tmpString);
	    }
	}
    } /* if DTWM */
#endif /* WSM */
    if(gotItAll)
    {
        tmpString = ((char *)GETMESSAGE(62, 48, "Close _C Alt<Key>F4 f.kill"));
        if ((CloString =
             (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
        {
            Warning (((char *)GETMESSAGE(62, 20, "Insufficient memory for local default menu.")));
            gotItAll = False;
        }
        else
        {
            strcpy(CloString, tmpString);
        }
    }

    if (!gotItAll)
    {
#ifdef WSM
	if (DtwmBehavior)
	{
	    builtinSystemMenu = (char *) 
			XtNewString((String)DEFAULT_DTWM_SYSTEMMENU);
	}
	else
	{
	    builtinSystemMenu = (char *) 
			XtNewString((String)BUILTINSYSTEMMENU);
	}
#else /* WSM */
	builtinSystemMenu = (char *)
			XtNewString((String)BUILTINSYSTEMMENU);
#endif /* WSM */
    }
    else
    {
        /* put it together */
        strcpy(dsm, defaultSystemMenuName);
        strcat(dsm, "\n{\n");
        strcat(dsm, ResString);
        strcat(dsm, "\n");
        strcat(dsm, MovString);
        strcat(dsm, "\n");
        strcat(dsm, SizString);
        strcat(dsm, "\n");
        strcat(dsm, MinString);
        strcat(dsm, "\n");
        strcat(dsm, MaxString);
        strcat(dsm, "\n");
        strcat(dsm, LowString);
        strcat(dsm, "\n");
        strcat(dsm, " no-label  f.separator\n");
#ifdef WSM
	if (DtwmBehavior)
	{
	    strcat(dsm, OcpString);
	    strcat(dsm, "\n");
	    strcat(dsm, OcaString);
	    strcat(dsm, "\n");
	    strcat(dsm, RemString);
	    strcat(dsm, "\n");
	    strcat(dsm, " no-label  f.separator\n");
	}
#endif /* WSM */
        strcat(dsm, CloString);
        strcat(dsm, "\n}");
	
	if ((builtinSystemMenu =
	     (char *)XtMalloc ((unsigned int) (strlen(dsm) + 1))) == NULL)
	{
	   Warning (((char *)GETMESSAGE(62, 21, "Insufficient memory for localized default system menu")));
#ifdef WSM
	    if (DtwmBehavior)
	    {
		builtinSystemMenu = (char *) 
			XtNewString((String)DEFAULT_DTWM_SYSTEMMENU);
	    }
	    else
	    {
		builtinSystemMenu = (char *) 
			XtNewString((String)BUILTINSYSTEMMENU);
	    }
#else /* WSM */
	    builtinSystemMenu = (char *) 
			XtNewString((String)BUILTINSYSTEMMENU);
#endif /* WSM */
	}
	else
	{
	    strcpy(builtinSystemMenu, dsm);
	}
    }

    if (ResString != NULL)
       XtFree(ResString);
    if (MovString != NULL)
       XtFree(MovString);
    if (SizString != NULL)
       XtFree(SizString);
    if (MinString != NULL)
       XtFree(MinString);
    if (MaxString != NULL)
       XtFree(MaxString);
    if (LowString != NULL)
       XtFree(LowString);
#ifdef WSM
    if (OcpString != NULL)
       XtFree(OcpString);
    if (OcaString != NULL)
       XtFree(OcaString);
    if (RemString != NULL)
       XtFree(RemString);
#endif /* WSM */
    if (CloString != NULL)
       XtFree(CloString);
    
} /* END OF FUNCTION  InitBuiltinSystemMenu */
#endif /* NO_MESSAGE_CATALOG */
#else /* MCCABE */
char builtinSystemMenu[];
#endif /* MCCABE */

#ifdef WSM
#define HARD_CODED_PRIMARY   3
#endif /* WSM */
char defaultRootMenuName[] = "DefaultRootMenu";
char builtinRootMenuName[] = "_MwmRootMenu_";
#ifndef MCCABE
#define BUILTINROOTMENU "DefaultRootMenu\n\
{\n\
	\"Root Menu\"		f.title\n\
	\"New Window\"		f.exec \"xterm &\"\n\
	\"Shuffle Up\"		f.circle_up\n\
	\"Shuffle Down\"	f.circle_down\n\
	\"Refresh\"		f.refresh\n\
	\"Pack Icons\"		f.pack_icons\n\
	 no-label		f.separator\n\
	\"Restart...\"		f.restart\n\
}";
char builtinRootMenu[] = BUILTINROOTMENU
#else /* MCCABE */
char builtinRootMenu[];
#endif /* MCCABE */


/* builtin key bindings specification */

char defaultKeyBindingsName[] = "DefaultKeyBindings";
char builtinKeyBindingsName[] = "_MwmKeyBindings_";
#ifndef MCCABE
#define BUILTINKEYBINDINGS "_MwmKeyBindings_\n\
{\n\
	Shift<Key>Escape	window|icon		f.post_wmenu\n\
	Alt<Key>space		window|icon		f.post_wmenu\n\
	Alt<Key>Tab		root|icon|window	f.next_key\n\
	Alt Shift<Key>Tab	root|icon|window	f.prev_key\n\
	Alt<Key>Escape		root|icon|window	f.circle_down\n\
	Alt Shift<Key>Escape	root|icon|window	f.circle_up\n\
	Alt Shift Ctrl<Key>exclam root|icon|window	f.set_behavior\n\
	Alt Ctrl<Key>1		  root|icon|window	f.set_behavior\n\
	Alt<Key>F6		window			f.next_key transient\n\
	Alt Shift<Key>F6	window			f.prev_key transient\n\
	Shift<Key>F10		icon			f.post_wmenu\n\
}";
char builtinKeyBindings[] = BUILTINKEYBINDINGS

#else
char builtinKeyBindings[];
#endif

/*
 * NOTE: Default Toggle Behavior key bindings.  There are TWO key bindings as
 * of 1.1.4 and 1.2.  Make sure you make any modify builtinKeyBindings (above)
 * whenever modifying behaviorKeyBindings.
 */

char behaviorKeyBindingName[] = "_MwmBehaviorKey_";
#ifndef MCCABE
#define BEHAVIORKEYBINDINGS "_MwmBehaviorKey_\n\
{\n\
	Alt Shift Ctrl<Key>exclam root|icon|window	f.set_behavior\n\
	Alt Ctrl<Key>1		  root|icon|window	f.set_behavior\n\
}";
char behaviorKeyBindings[] = BEHAVIORKEYBINDINGS

#else
char behaviorKeyBindings[];
#endif


/* default button bindings specification */
/* note - the %s will be replaced by the real DefaultRootMenu */

char defaultButtonBindingsName[] = "DefaultButtonBindings";
char builtinButtonBindingsName[] = "_MwmButtonBindings_";
#ifndef MCCABE
# if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
#  define BUILTINBUTTONBINDINGS "_MwmButtonBindings_\n\
{\n\
	<Btn1Down>	icon|frame	f.raise\n\
	<Btn3Down>	icon|frame	f.post_wmenu\n\
	<Btn3Down>	root		f.menu %s\n\
}";
# else
#  define BUILTINBUTTONBINDINGS "_MwmButtonBindings_\n\
{\n\
	<Btn1Down>	icon|frame	f.raise\n\
	<Btn3Down>	icon|frame	f.post_wmenu\n\
	<Btn3Down>	root		f.menu DefaultRootMenu\n\
}";
# endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
char builtinButtonBindings[] = BUILTINBUTTONBINDINGS

#else
char builtinButtonBindings[];
#endif


static ClientData *_pCD;
static String _defaultBackground;
static String _defaultActiveBackground;
static AppearanceData *_pAppearanceData;
#ifdef WSM
static WmWorkspaceData *pResWS;
static WmScreenData *pResSD;
#endif /* WSM */

static char _defaultColor1HEX[] = "#A8A8A8A8A8A8";
static char _defaultColor2HEX[] = "#5F5F92929E9E";

static char _defaultColor1[] = "LightGrey";
static char _defaultColor2[] = "CadetBlue";
#define DEFAULT_COLOR_NONE	NULL

Const char _foreground[]    = "foreground";
Const char _75_foreground[] = "75_foreground";
Const char _50_foreground[] = "50_foreground";
Const char _25_foreground[] = "25_foreground";
#ifdef WSM
Const char *_Dither = XmCO_DITHER;
Const char *_NoDither = XmCO_NO_DITHER;
Const char CLIENT_FRAME_PART[] = "client";
Const char ICON_FRAME_PART[] = "icon";
Const char FEEDBACK_FRAME_PART[] = "feedback";
Const char MENU_ITEM_PART[] = "menu";
#endif /* WSM */

#define WmBGC          XmBACKGROUND
#define WmFGC          XmFOREGROUND
#define WmTSC          XmTOP_SHADOW
#define WmBSC          XmBOTTOM_SHADOW

#define MAX_SHORT	0xffff

#ifndef BITMAPDIR
#define BITMAPDIR "/usr/include/X11/bitmaps/"
#endif


/*************************************<->*************************************
 *
 *  wmGlobalResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm general
 *  appearance and behavior resources.  These resources are specified
 *  with the following syntax:
 *
 *      "Mwm*<resource_identifier>".
 *
 *************************************<->***********************************/


XtResource wmGlobalResources[] =
{

    {
	WmNautoKeyFocus,
	WmCAutoKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, autoKeyFocus),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNautoRaiseDelay,
	WmCAutoRaiseDelay,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, autoRaiseDelay),
	XtRImmediate,
	(XtPointer)500
    },

    {
	WmNbitmapDirectory,
	WmCBitmapDirectory,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, bitmapDirectory),
	XtRString,
	(XtPointer)BITMAPDIR
    },
#ifdef MINIMAL_DT
    {
	WmNblinkOnExec,
	WmCBlinkOnExec,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, blinkOnExec),
	XtRImmediate,
	(XtPointer)False
    },
#endif /* MINIMAL_DT */
    {
	WmNframeStyle,
	WmCFrameStyle,
	WmRFrameStyle,
	sizeof (FrameStyle),
	XtOffsetOf(WmGlobalData, frameStyle),
	XtRImmediate,
	(XtPointer)WmRECESSED
    },

    {
	WmNclientAutoPlace,
	WmCClientAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, clientAutoPlace),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNcolormapFocusPolicy,
	WmCColormapFocusPolicy,
	WmRCFocusPolicy,
	sizeof (int),
        XtOffsetOf(WmGlobalData, colormapFocusPolicy),
	XtRImmediate,
	(XtPointer)CMAP_FOCUS_KEYBOARD
    },

    {
	WmNconfigFile,
	WmCConfigFile,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, configFile),
	XtRImmediate,
	(XtPointer)NULL
    },
#ifdef WSM

    {
	WmNcppCommand,
	WmCCppCommand,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, cppCommand),
	XtRImmediate,
	(XtPointer)NULL
    },
#endif /* WSM */

    {
	WmNdeiconifyKeyFocus,
	WmCDeiconifyKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, deiconifyKeyFocus),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNdoubleClickTime,
	WmCDoubleClickTime,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, doubleClickTime),
	XtRCallProc,
	(XtPointer)_WmMultiClickTimeDefault
    },

    {
	WmNenableWarp,
	WmCEnableWarp,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, enableWarp),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNenforceKeyFocus,
	WmCEnforceKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, enforceKeyFocus),
	XtRImmediate,
	(XtPointer)True
    },
#ifdef WSM
    {
	WmNframeExternalShadowWidth,
	WmCFrameExternalShadowWidth,
	XtRDimension,
	sizeof (Dimension),
        XtOffsetOf(WmGlobalData, frameExternalShadowWidth),
	XtRImmediate,
	(XtPointer)2
    },
#endif /* WSM */

    {
	WmNfreezeOnConfig,
	WmCFreezeOnConfig,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, freezeOnConfig),
	XtRImmediate,
	(XtPointer)True
    },
#ifdef WSM

    {
	WmNuseWindowOutline,
	WmCUseWindowOutline,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, useWindowOutline),
	XtRImmediate,
	(XtPointer)False
    },
#endif /* WSM */

    {
	WmNiconAutoPlace,
	WmCIconAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconAutoPlace),
	XtRImmediate,
	(XtPointer)True
    },
#ifdef WSM
    {
	WmNiconExternalShadowWidth,
	WmCIconExternalShadowWidth,
	XtRDimension,
	sizeof (Dimension),
        XtOffsetOf(WmGlobalData, iconExternalShadowWidth),
	XtRImmediate,
	(XtPointer)2
    },
#endif /* WSM */

    {
	WmNiconClick,
	WmCIconClick,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconClick),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNinteractivePlacement,
        WmCInteractivePlacement,
        XtRBoolean,
        sizeof (Boolean),
        XtOffsetOf(WmGlobalData, interactivePlacement),
        XtRImmediate,
        (XtPointer)False
    },

    {
	WmNkeyboardFocusPolicy,
	WmCKeyboardFocusPolicy,
	WmRKFocusPolicy,
	sizeof (int),
        XtOffsetOf(WmGlobalData, keyboardFocusPolicy),
	XtRImmediate,
	(XtPointer)KEYBOARD_FOCUS_EXPLICIT
    },

    {
	WmNlowerOnIconify,
	WmCLowerOnIconify,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, lowerOnIconify),
	XtRImmediate,
	(XtPointer)True
    },
#ifdef WSM

    {
	WmNmarqueeSelectGranularity,
	WmCMarqueeSelectGranularity,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, marqueeSelectGranularity),
	XtRImmediate,
	(XtPointer)0
    },
#endif /* WSM */

    {
	WmNmoveThreshold,
	WmCMoveThreshold,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, moveThreshold),
	XtRImmediate,
	(XtPointer)4
    },

    {
	WmNpassButtons,
	WmCPassButtons,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, passButtons),
	XtRImmediate,
	(XtPointer)False
    },

    {
	WmNpassSelectButton,
	WmCPassSelectButton,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, passSelectButton),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNpositionIsFrame,
	WmCPositionIsFrame,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionIsFrame),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNpositionOnScreen,
	WmCPositionOnScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionOnScreen),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNquitTimeout,
	WmCQuitTimeout,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, quitTimeout),
	XtRImmediate,
	(XtPointer)1000
    },

    {
	WmNraiseKeyFocus,
	WmCRaiseKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, raiseKeyFocus),
	XtRImmediate,
	(XtPointer)False
    },
#ifdef WSM

    {
	WmNrefreshByClearing,
	WmCRefreshByClearing,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, refreshByClearing),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNrootButtonClick,
	WmCRootButtonClick,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, rootButtonClick),
	XtRImmediate,
	(XtPointer)True
    },
#endif /* WSM */

#ifndef WSM
    {
        WmNsessionClientDB,
	WmCSessionClientDB,
	XtRString,
	sizeof(String),
	XtOffsetOf(WmGlobalData, sessionClientDB),
	XtRImmediate,
	(XtPointer)NULL
    },
#endif /* ! WSM */

    {
	WmNshowFeedback,
	WmCShowFeedback,
	WmRShowFeedback,
	sizeof (int),
        XtOffsetOf(WmGlobalData, showFeedback),
	XtRImmediate,
	(XtPointer)(WM_SHOW_FB_DEFAULT)
    },

    {
	WmNstartupKeyFocus,
	WmCStartupKeyFocus,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, startupKeyFocus),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNsystemButtonClick,
	WmCSystemButtonClick,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, systemButtonClick),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNsystemButtonClick2,
	WmCSystemButtonClick2,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, systemButtonClick2),
	XtRImmediate,
	(XtPointer)True
    },
#if defined(PANELIST)
    {
	WmNuseFrontPanel,
	WmCUseFrontPanel,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, useFrontPanel),
	XtRImmediate,
	(XtPointer)True
    },
#endif /* PANELIST */
#ifdef WSM
    {
	WmNhelpDirectory,
	WmCHelpDirectory,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, helpDirectory),
	XtRImmediate,
	(XtPointer)"DT/Dtwm/"
    },

#endif /* WSM */
#ifdef MINIMAL_DT
    {
	WmNdtLite,
	WmCDtLite,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, dtLite),
	XtRImmediate,
	(XtPointer)False
    }
#endif /* MINIMAL_DT */
}; /* END OF wmGlobalResources[] */


/*
 * These determine the screens to manage at startup.
 * These are broken out to enhance startup performance.
 */
XtResource wmGlobalScreenResources[] =
{
    {
	WmNmultiScreen,
	WmCMultiScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, multiScreen),
	XtRImmediate,
#ifdef WSM
	(XtPointer)True
#else /* WSM */
	(XtPointer)False
#endif /* WSM */
    },

    {
	WmNscreens,
	WmCScreens,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, screenList),
	XtRImmediate,
	(XtPointer)NULL
    },
#ifdef WSM
    {   WmNbackdropDirectories, 
	WmCBackdropDirectories, 
	XmRString, 
	sizeof(char *),
	XtOffsetOf(WmGlobalData, backdropDirs), 
	XmRString,
	DEFAULT_BACKDROP_DIR
    },
#endif /* WSM */
};



/******************************<->*************************************
 *
 *  wmStdGlobalResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm general appearance
 *  and behavior resources that are not automatically set for the standard
 *  (default) behavior.  These resources are specified with the following
 *  syntax:
 *
 *      "Mwm*<resource_identifier>".
 *
 ******************************<->***********************************/

XtResource wmStdGlobalResources[] =
{

    {
	WmNbitmapDirectory,
	WmCBitmapDirectory,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, bitmapDirectory),
	XtRString,
	(XtPointer)BITMAPDIR
    },

    {
	WmNconfigFile,
	WmCConfigFile,
	XtRString,
	sizeof (String),
        XtOffsetOf(WmGlobalData, configFile),
	XtRImmediate,
	(XtPointer)NULL
    },

    {
	WmNframeStyle,
	WmCFrameStyle,
	WmRFrameStyle,
	sizeof (FrameStyle),
	XtOffsetOf(WmGlobalData, frameStyle),
	XtRImmediate,
	(XtPointer)WmRECESSED
    },

    {
	WmNiconAutoPlace,
	WmCIconAutoPlace,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, iconAutoPlace),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNmoveThreshold,
	WmCMoveThreshold,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, moveThreshold),
	XtRImmediate,
	(XtPointer)4
    },

    {
	WmNpositionIsFrame,
	WmCPositionIsFrame,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionIsFrame),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNpositionOnScreen,
	WmCPositionOnScreen,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf(WmGlobalData, positionOnScreen),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNquitTimeout,
	WmCQuitTimeout,
	XtRInt,
	sizeof (int),
        XtOffsetOf(WmGlobalData, quitTimeout),
	XtRImmediate,
	(XtPointer)1000
    },

    {
	WmNshowFeedback,
	WmCShowFeedback,
	WmRShowFeedback,
	sizeof (int),
        XtOffsetOf(WmGlobalData, showFeedback),
	XtRImmediate,
	(XtPointer)(WM_SHOW_FB_DEFAULT)
    },

};


/******************************<->*************************************
 *
 *  wmScreenResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm screen specific
 *  appearance and behavior resources.  These resources are specified
 *  with the following syntax:
 *
 *      "Mwm*screen<#>*<resource_identifier>".
 *
 ******************************<->***********************************/

XtResource wmScreenResources[] =
{
    {
	WmNbuttonBindings,
	WmCButtonBindings,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, buttonBindings),
	XtRString,
	(XtPointer)defaultButtonBindingsName
    },

    {
	WmNcleanText,
	WmCCleanText,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, cleanText),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNfeedbackGeometry,
	WmCFeedbackGeometry,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, feedbackGeometry),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNfadeNormalIcon,
	WmCFadeNormalIcon,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, fadeNormalIcon),
	XtRImmediate,
	(XtPointer)False
    },

    {
	WmNiconDecoration,
	WmCIconDecoration,
	WmRIconDecor,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconDecoration),
	XtRImmediate,
	(XtPointer)USE_ICON_DEFAULT_APPEARANCE
    },

#ifdef WSM
    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRCallProc,
	(XtPointer) _WmIconImageMaximumDefault
    },
#else /* WSM */
    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRString,
	"50x50"
    },
#endif /* WSM */

    {
	WmNiconImageMinimum,
	WmCIconImageMinimum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMinimum),
	XtRString,
	"16x16"
    },

    {
	WmNiconPlacement,
	WmCIconPlacement,
	WmRIconPlacement,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacement),
	XtRImmediate,
	(XtPointer)(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY)
    },

    {
	WmNiconPlacementMargin,
	WmCIconPlacementMargin,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacementMargin),
	XtRImmediate,
	(XtPointer)-1
    },

    {
	WmNkeyBindings,
	WmCKeyBindings,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, keyBindings),
	XtRString,
	(XtPointer)defaultKeyBindingsName
    },

    {
	WmNframeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, frameBorderWidth),
	XtRImmediate,
	(XtPointer) BIGSIZE
    },
#ifndef WSM

    {
	WmNiconBoxGeometry,
	WmCIconBoxGeometry,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxGeometry),
	XtRString,
	(XtPointer)NULL
    },
#endif /* WSM */

    {
	WmNiconBoxName,
	WmCIconBoxName,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxName),
	XtRString,
	(XtPointer)"iconbox"
    },

    {
	WmNiconBoxSBDisplayPolicy,
	WmCIconBoxSBDisplayPolicy,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, iconBoxSBDisplayPolicy),
	XtRString,
	(XtPointer)"all"
    },

    {
	WmNiconBoxScheme,
	WmCIconBoxScheme,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconBoxScheme),
	XtRImmediate,
	(XtPointer)0
    },

    {
	WmNiconBoxTitle,
	WmCIconBoxTitle,
	XmRXmString,
	sizeof (XmString),
	XtOffsetOf (WmScreenData, iconBoxTitle),
	XmRXmString,
	(XtPointer)NULL
    },

    {
	WmNlimitResize,
	WmCLimitResize,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, limitResize),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNmaximumMaximumSize,
	WmCMaximumMaximumSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, maximumMaximumSize),
	XtRString,
	"0x0"
    },

    {
	WmNresizeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, resizeBorderWidth),
	XtRImmediate,
	(XtPointer) BIGSIZE
    },

    {
	WmNresizeCursors,
	WmCResizeCursors,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, resizeCursors),
	XtRImmediate,
	(XtPointer)True
    },

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    {
	WmNrootMenu,
	WmCRootMenu,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, rootMenu),
	XtRString,
	(XtPointer)builtinRootMenuName
    },
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    {
	WmNtransientDecoration,
	WmCTransientDecoration,
	WmRClientDecor,
	sizeof (int),
	XtOffsetOf (WmScreenData, transientDecoration),
	XtRImmediate,
	(XtPointer)(WM_DECOR_SYSTEM | WM_DECOR_RESIZEH)
    },

    {
	WmNtransientFunctions,
	WmCTransientFunctions,
	WmRClientFunction,
	sizeof (int),
	XtOffsetOf (WmScreenData, transientFunctions),
	XtRImmediate,
	(XtPointer)(WM_FUNC_ALL & ~(MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE))
    },

#ifdef PANELIST
    {
	WmNsubpanelDecoration,
	WmCSubpanelDecoration,
	WmRClientDecor,
	sizeof (int),
	XtOffsetOf (WmScreenData, subpanelDecoration),
	XtRImmediate,
	(XtPointer)(WM_DECOR_SYSTEM)
    },

    {
	WmNsubpanelResources,
	WmCSubpanelResources,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, subpanelResources),
	XtRString,
	(XtPointer)NULL
    },
#endif /* PANELIST */

    {
	WmNuseIconBox,
	WmCUseIconBox,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, useIconBox),
	XtRImmediate,
	(XtPointer)False
    },

    {
	WmNmoveOpaque,
	WmCMoveOpaque,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (WmScreenData, moveOpaque),
	XtRImmediate,
	(XtPointer)False

#ifdef WSM
    },

    {
	WmNhelpResources,
	WmCHelpResources,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, helpResources),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNinitialWorkspace,
	WmCInitialWorkspace,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmScreenData, initialWorkspace),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNworkspaceList,
	WmCWorkspaceList,
	XtRString,
	sizeof (String),
        XtOffsetOf (WmScreenData, workspaceList),
	XtRImmediate,
	(XtPointer)NULL
    },

    {
	WmNworkspaceCount,
	WmCWorkspaceCount,
	XtRInt,
	sizeof (int),
        XtOffsetOf (WmScreenData, numWorkspaces),
	XtRImmediate,
	(XtPointer)0
#endif /* WSM */
    }

};


/******************************<->*************************************
 *
 *  wmStdScreenResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm screen specific
 *  appearance and behavior resources that are not automatically set for 
 *  the standard (default) behavior.  These resources are specified with 
 *  the following syntax:
 *
 *      "Mwm*screen<#>*<resource_identifier>".
 *
 ******************************<->***********************************/

XtResource wmStdScreenResources[] =
{
    {
	WmNframeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, frameBorderWidth),
	XtRImmediate,
	(XtPointer) BIGSIZE
    },

#ifdef WSM
    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRCallProc,
	(XtPointer) _WmIconImageMaximumDefault
    },
#else /* WSM */
    {
	WmNiconImageMaximum,
	WmCIconImageMaximum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMaximum),
	XtRString,
	"50x50"
    },
#endif /* WSM */

    {
	WmNiconImageMinimum,
	WmCIconImageMinimum,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, iconImageMinimum),
	XtRString,
	"32x32"
    },

    {
	WmNiconPlacementMargin,
	WmCIconPlacementMargin,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, iconPlacementMargin),
	XtRImmediate,
	(XtPointer)-1
    },

    {
	WmNmaximumMaximumSize,
	WmCMaximumMaximumSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (WmScreenData, maximumMaximumSize),
	XtRString,
	"0x0"
    },

    {
	WmNresizeBorderWidth,
	WmCFrameBorderWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (WmScreenData, resizeBorderWidth),
	XtRImmediate,
	(XtPointer) BIGSIZE
    }
};



/******************************<->*************************************
 *
 *  wmWorkspaceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm workspace 
 *  specific appearance and behavior resources.  These resources are 
 *  specified with the following syntax:
 *
 *      "Mwm*[screen<#>*]<workspace>*<resource_identifier>".
 *
 ******************************<->***********************************/
#ifdef WSM
XtResource wmWorkspaceResources[] =
{
    {
	WmNiconBoxGeometry,
	WmCIconBoxGeometry,
	XtRString,
	sizeof (String),
	XtOffsetOf (WmWorkspaceData, iconBoxGeometry),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNtitle,
	WmCTitle,
	XmRXmString,
	sizeof (XmString),
	XtOffsetOf (WmWorkspaceData, title),
	XmRXmString,
	(XtPointer)NULL
    }

};
#else /* WSM */
XtResource *wmWorkspaceResources = NULL;
#endif /* WSM */



/******************************<->*************************************
 *
 *  wmStdWorkspaceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of mwm workspace specific 
 *  appearance and behavior resources that are not automatically set for 
 *  the standard (default) behavior.  These resources are specified with 
 *  the following syntax:
 *
 *      "Mwm*[screen<#>*]<workspace>*<resource_identifier>".
 *
 *************************************<->***********************************/

#ifdef WSM
XtResource wmStdWorkspaceResources[] =
{
    {
	WmNtitle,
	WmCTitle,
	XmRXmString,
	sizeof (XmString),
	XtOffsetOf (WmWorkspaceData, title),
	XmRXmString,
	(XtPointer)NULL
    }
};
#else /* WSM */
XtResource *wmStdWorkspaceResources = NULL;
#endif /* WSM */

#ifdef WSM

/*************************************<->*************************************
 *
 *  wmBackdropResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of workspace specific
 *  resources that apply to the backdrop.
 *
 *  These resources are specified with the following syntax:
 *
 *      "Mwm*[screen*][workspace*]backdrop*<resource_id>:"
 *
 *  NOTE: The order of these resources is important for correct
 *        dynamic processing!!!!
 *
 *************************************<->***********************************/

XtResource wmBackdropResources[] =
{
    {
	WmNcolorSetId,
	WmCColorSetId,
	XtRInt,
	sizeof (int),
	XtOffsetOf (BackdropData, colorSet),
	XtRCallProc,
	(XtPointer) _WmBackdropColorSetDefault
    },

    {
	WmNimageBackground,
	WmCImageBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (BackdropData, background),
	XtRCallProc,
	(XtPointer) _WmBackdropBgDefault
    },

    {
	WmNimageForeground,
	WmCImageForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (BackdropData, foreground),
	XtRCallProc,
	(XtPointer) _WmBackdropFgDefault
    },

    {
	WmNimage,
	WmCImage,
	XtRString,
	sizeof (String),
	XtOffsetOf (BackdropData, image),
	XtRString,
	(XtPointer)NULL
    },

};


/*************************************<->*************************************
 *
 *  wmWsPresenceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of specific
 *  resources that apply to the WorkspacePresence dialog.
 *
 *  These resources are specified with the following syntax:
 *
 *      "Mwm*[screen*][workspace*]workspacePresence*<resource_id>:"
 *
 *************************************<->***********************************/

XtResource wmWsPresenceResources[] =
{
    {
	WmNtitle,
	WmCTitle,
	XmRXmString,
	sizeof (XmString),
	XtOffsetOf (WsPresenceData, title),
	XmRXmString,
	(XtPointer)NULL
    }
};
#endif /* WSM */


/*************************************<->*************************************
 *
 *  wmClientResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources.  These resources are specified with the
 *  following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

XtResource wmClientResources[] =
{

#ifdef WSM
    {
	WmNabsentMapBehavior,
	WmCAbsentMapBehavior,
	WmRAbsentMapBehavior,
	sizeof (int),
	XtOffsetOf (ClientData, absentMapBehavior),
	XtRImmediate,
	(XtPointer)(AMAP_BEHAVIOR_ADD)
    },
#endif /* WSM */
    {
	WmNclientDecoration,
	WmCClientDecoration,
	WmRClientDecor,
	sizeof (int),
	XtOffsetOf (ClientData, clientDecoration),
	XtRImmediate,
	(XtPointer)(WM_DECOR_DEFAULT)
    },

    {
	WmNclientFunctions,
	WmCClientFunctions,
	WmRClientFunction,
	sizeof (int),
	XtOffsetOf (ClientData, clientFunctions),
	XtRImmediate,
	(XtPointer)(WM_FUNC_DEFAULT)
    },

    {
	WmNfocusAutoRaise,
	WmCFocusAutoRaise,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, focusAutoRaise),
	XtRCallProc,
	(XtPointer)_WmFocusAutoRaiseDefault
    },

    {
	WmNiconImage,
	WmCIconImage,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImage),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNiconImageBackground,
	WmCIconImageBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageBackground),
	XtRCallProc,
	(XtPointer)_WmIconImageBDefault
    },

    {
	WmNiconImageForeground,
	WmCIconImageForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageForeground),
	XtRCallProc,
	(XtPointer)_WmIconImageFDefault
    },

    {
	WmNiconImageBottomShadowColor,
	WmCIconImageBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageBottomShadowColor),
	XtRCallProc,
	(XtPointer)_WmIconImageBSCDefault
    },

    {
	WmNiconImageBottomShadowPixmap,
	WmCIconImageBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, iconImageBottomShadowPStr),
	XtRCallProc,
	(XtPointer)_WmIconImageBSPDefault
    },

    {
	WmNiconImageTopShadowColor,
	WmCIconImageTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageTopShadowColor),
	XtRCallProc,
	(XtPointer)_WmIconImageTSCDefault
    },

    {
	WmNiconImageTopShadowPixmap,
	WmCIconImageTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImageTopShadowPStr),
	XtRCallProc,
	(XtPointer)_WmIconImageTSPDefault
    },

    {
	WmNignoreWMSaveHints,
	WmCIgnoreWMSaveHints,
	XtRBoolean,
	sizeof (Boolean),
	XtOffsetOf (ClientData, ignoreWMSaveHints),
	XtRImmediate,
	(XtPointer)True
    },

    {
	WmNmatteWidth,
	WmCMatteWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (ClientData, matteWidth),
	XtRImmediate,
	(XtPointer)0
    },

    {
	WmNmaximumClientSize,
	WmCMaximumClientSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (ClientData, maximumClientSize),
	XtRString,
	"0x0"
    },
#ifdef WSM

    {
	WmNsecondariesOnTop,
	WmCSecondariesOnTop,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, secondariesOnTop),
	XtRCallProc,
	(XtPointer)_WmSecondariesOnTopDefault
    },
#endif /* WSM */

    {
	WmNsystemMenu,
	WmCSystemMenu,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, systemMenu),
	XtRString,
	(XtPointer)defaultSystemMenuName
    },

    {
	WmNuseClientIcon,
	WmCUseClientIcon,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, useClientIcon),
	XtRImmediate,
#ifdef WSM
	(XtPointer)True
#else
	(XtPointer)False
#endif /* WSM */
    },

    {
	WmNusePPosition,
	WmCUsePPosition,
	WmRUsePPosition,
	sizeof (int),
	XtOffsetOf (ClientData, usePPosition),
	XtRImmediate,
	(XtPointer)(USE_PPOSITION_NONZERO)
    }

}; /* END OF STRUCTURE wmClientResources */



/*************************************<->*************************************
 *
 *  wmStdClientResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources that are not automatically set for the standard
 *  (default) behavior.  These resources are specified with the
 *  following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

XtResource wmStdClientResources[] =
{

    {
	WmNiconImage,
	WmCIconImage,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImage),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNiconImageBackground,
	WmCIconImageBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageBackground),
	XtRCallProc,
	(XtPointer)_WmIconImageBDefault
    },

    {
	WmNiconImageForeground,
	WmCIconImageForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, iconImageForeground),
	XtRCallProc,
	(XtPointer)_WmIconImageFDefault
    },

    {
	WmNiconImageBottomShadowColor,
	WmCIconImageBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageBottomShadowColor),
	XtRCallProc,
	(XtPointer)_WmIconImageBSCDefault
    },

    {
	WmNiconImageBottomShadowPixmap,
	WmCIconImageBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, iconImageBottomShadowPStr),
	XtRCallProc,
	(XtPointer)_WmIconImageBSPDefault
    },

    {
	WmNiconImageTopShadowColor,
	WmCIconImageTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, iconImageTopShadowColor),
	XtRCallProc,
	(XtPointer)_WmIconImageTSCDefault
    },

    {
	WmNiconImageTopShadowPixmap,
	WmCIconImageTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, iconImageTopShadowPStr),
	XtRCallProc,
	(XtPointer)_WmIconImageTSPDefault
    },

    {
	WmNmatteWidth,
	WmCMatteWidth,
	XtRInt,
	sizeof (int),
	XtOffsetOf (ClientData, matteWidth),
	XtRImmediate,
	(XtPointer)0
    },

    {
	WmNmaximumClientSize,
	WmCMaximumClientSize,
	WmRSize,
	sizeof (WHSize),
	XtOffsetOf (ClientData, maximumClientSize),
	XtRString,
	"0x0"
    },
#ifdef WSM
    {
	WmNsecondariesOnTop,
	WmCSecondariesOnTop,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, secondariesOnTop),
	XtRCallProc,
	(XtPointer)_WmSecondariesOnTopDefault
    },
#endif /* WSM */

    {
	WmNuseClientIcon,
	WmCUseClientIcon,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (ClientData, useClientIcon),
	XtRImmediate,
	(XtPointer)False
    }
};



/*************************************<->*************************************
 *
 *  wmClientResourcesM
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of client specific 
 *  window manager resources that affect the appearance of the client
 *  matte.  These resources are specified with the following syntax:
 *
 *      "Mwm*<client_name_or_class>*<resource_identifier>"
 *
 *************************************<->***********************************/

XtResource wmClientResourcesM[] =
{
    {
	WmNmatteBackground,
	WmCMatteBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (ClientData, matteBackground),
	XtRCallProc,
	(XtPointer)_WmMatteBDefault
    },

    {
	WmNmatteForeground,
	WmCMatteForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteForeground),
	XtRCallProc,
	(XtPointer)_WmMatteFDefault
    },

    {
	WmNmatteBottomShadowColor,
	WmCMatteBottomShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteBottomShadowColor),
	XtRCallProc,
	(XtPointer)_WmMatteBSCDefault
    },

    {
	WmNmatteBottomShadowPixmap,
	WmCMatteBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (ClientData, matteBottomShadowPStr),
	XtRCallProc,
	(XtPointer)_WmMatteBSPDefault
    },

    {
	WmNmatteTopShadowColor,
	WmCMatteTopShadowColor,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (ClientData, matteTopShadowColor),
	XtRCallProc,
	(XtPointer)_WmMatteTSCDefault
    },

    {
	WmNmatteTopShadowPixmap,
	WmCMatteTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (ClientData, matteTopShadowPStr),
	XtRCallProc,
	(XtPointer)_WmMatteTSPDefault
    }
};



/*************************************<->*************************************
 *
 *  wmAppearanceResources
 *
 *
 *  Description:
 *  -----------
 *  This data structure is used in the processing of component appearance
 *  resources.  These resources are specified with the following syntax:
 *
 *      "Mwm*<resource_identifier>"
 *      "Mwm*client*<resource_identifier>"
 *      "Mwm*icon*<resource_identifier>"
 *      "Mwm*feedback*<resource_identifier>"
 *
 *************************************<->***********************************/

XtResource wmAppearanceResources[] =
{

    {
	XmNfontList,
	XmCFontList,
	XmRFontList,
	sizeof (XmFontList),
	XtOffsetOf (AppearanceData, fontList),
	XtRString,
	"fixed"
    },

    {
	WmNsaveUnder,
	WmCSaveUnder,
	XtRBoolean,
	sizeof (Boolean),
        XtOffsetOf (AppearanceData, saveUnder),
	XtRImmediate,
	(XtPointer)False
    },

    {
	XtNbackground,
	XtCBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, background),
	XtRCallProc,
	(XtPointer)_WmBackgroundDefault
    },

    {
	XtNforeground,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, foreground),
	XtRCallProc,
	(XtPointer)_WmForegroundDefault
    },

    {
	XmNbottomShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, bottomShadowColor),
	XtRCallProc,
	(XtPointer)_WmBottomShadowColorDefault
    },

    {
	XmNbottomShadowPixmap,
	XmCBottomShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, bottomShadowPStr),
	XtRString,
	(XtPointer)NULL
    },

    {
	XmNtopShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
        XtOffsetOf (AppearanceData, topShadowColor),
	XtRCallProc,
	(XtPointer)_WmTopShadowColorDefault
    },

    {
	XmNbackgroundPixmap,
	XmCBackgroundPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, backgroundPStr),
	XtRCallProc,
	(XtPointer)_WmBackgroundPixmapDefault
    },

    {
	XmNtopShadowPixmap,
	XmCTopShadowPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, topShadowPStr),
	XtRCallProc,
	(XtPointer)_WmTopShadowPixmapDefault
    },

    {
	WmNactiveBackground,
	XtCBackground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeBackground),
	XtRCallProc,
	(XtPointer)_WmABackgroundDefault
    },

    {
	WmNactiveForeground,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeForeground),
	XtRCallProc,
	(XtPointer)_WmAForegroundDefault
    },

    {
	WmNactiveBottomShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeBottomShadowColor),
	XtRCallProc,
	(XtPointer)_WmABottomShadowColorDefault
    },

    {
	WmNactiveBottomShadowPixmap,
	XmCBottomShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (AppearanceData, activeBottomShadowPStr),
	XtRString,
	(XtPointer)NULL
    },

    {
	WmNactiveTopShadowColor,
	XtCForeground,
	XtRPixel,
	sizeof (Pixel),
	XtOffsetOf (AppearanceData, activeTopShadowColor),
	XtRCallProc,
	(XtPointer)_WmATopShadowColorDefault
    },

    {
	WmNactiveBackgroundPixmap,
	XmCBackgroundPixmap,
	XtRString,
	sizeof (String),
        XtOffsetOf (AppearanceData, activeBackgroundPStr),
	XtRCallProc,
	(XtPointer)_WmABackgroundPixmapDefault
    },

    {
	WmNactiveTopShadowPixmap,
	XmCTopShadowPixmap,
	XtRString,
	sizeof (String),
	XtOffsetOf (AppearanceData, activeTopShadowPStr),
	XtRCallProc,
	(XtPointer)_WmATopShadowPixmapDefault
    }

};



/*************************************<->*************************************
 *
 *  _WmIconImageFDefault (widget, offset, value)
 *  _WmIconImageBDefault (widget, offset, value)
 *  _WmIconImageBSCDefault (widget, offset, value)
 *  _WmIconImageBSPDefault (widget, offset, value)
 *  _WmIconImageTSCDefault (widget, offset, value)
 *  _WmIconImageTSPDefault (widget, offset, value)
 *  _WmMatteFDefault (widget, offset, value)
 *  _WmMatteBDefault (widget, offset, value)
 *  _WmMatteBSCDefault (widget, offset, value)
 *  _WmMatteBSPDefault (widget, offset, value)
 *  _WmMatteTSCDefault (widget, offset, value)
 *  _WmMatteTSPDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  These functions are used to generate dynamic defaults for various
 *  client-specific appearance related resources.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  _pCD = (static global) pointer to client data associated with resources
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

void 
_WmIconImageFDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmFGC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageFDefault */

void 
_WmIconImageBDefault (Widget widget, int offset, XrmValue *value)
{
    value->addr = (char *)&(_pCD->pSD->iconAppearance.background);
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmIconImageBDefault */


void 
_WmIconImageBSCDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBSC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageBSCDefault */


void 
_WmIconImageBSPDefault (Widget widget, int offset, XrmValue *value)
{

    value->addr = (char *)_pCD->pSD->iconAppearance.bottomShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmIconImageBSCDefault */


void 
_WmIconImageTSCDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmTSC, 0,
	_pCD->iconImageBackground, value);

} /* END OF FUNCTION _WmIconImageTSCDefault */


void 
_WmIconImageTSPDefault (Widget widget, int offset, XrmValue *value)
{

    value->addr = (char *)_pCD->pSD->iconAppearance.topShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmIconImageTSPDefault */


void 
_WmMatteFDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmFGC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteFDefault */


void 
_WmMatteBDefault (Widget widget, int offset, XrmValue *value)
{
    value->addr = (char *)&(_pCD->pSD->clientAppearance.background);
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmMatteBDefault */


void 
_WmMatteBSCDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBSC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteBSCDefault */


void 
_WmMatteBSPDefault (Widget widget, int offset, XrmValue *value)
{

    value->addr = (char *)_pCD->pSD->clientAppearance.bottomShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmMatteBSCDefault */


void 
_WmMatteTSCDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmTSC, 0,
	_pCD->matteBackground, value);

} /* END OF FUNCTION _WmMatteTSCDefault */


void 
_WmMatteTSPDefault (Widget widget, int offset, XrmValue *value)
{

    value->addr = (char *)_pCD->pSD->clientAppearance.topShadowPStr;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmMatteTSCDefault */



/*************************************<->*************************************
 *
 *  _WmBackgroundDefault (widget, offset, value)
 *  _WmForegroundDefault (widget, offset, value)
 *  _WmBackgroundPixmapDefault (widget, offset, value)
 *  _WmBottomShadowColorDefault (widget, offset, value)
 *  _WmTopShadowColorDefault (widget, offset, value)
 *  _WmTopShadowPixmapDefault (widget, offset, value)
 *  _WmABackgroundDefault (widget, offset, value)
 *  _WmAForegroundDefault (widget, offset, value)
 *  _WmABackgroundPixmapDefault (widget, offset, value)
 *  _WmABottomShadowColorDefault (widget, offset, value)
 *  _WmRFBackgroundDefault (widget, offset, value)
 *  _WmRFForegroundDefault (widget, offset, value)
 *  _WmATopShadowColorDefault (widget, offset, value)
 *  _WmATopShadowPixmapDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  These functions are used to generate dynamic defaults for various
 *  component appearance related resources (not client-specific).
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  _defaultBackground = (static global) default background color (inactive)
 *
 *  _defaultActiveBackground = (static global) default bg color (active)
 *
 *  _pAppearanceData = (static global) pointer to resouce set structure
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

void 
_WmBackgroundDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBGC, _defaultBackground, 0, value);

} /* END OF FUNCTION _WmBackgroundDefault */


void 
_WmForegroundDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmFGC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmForegroundDefault */


void 
_WmBackgroundPixmapDefault (Widget widget, int offset, XrmValue *value)
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->topShadowColor == _pAppearanceData->background))
    {
	string = (String) _25_foreground;
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmBackgroundPixmapDefault */


void 
_WmBottomShadowColorDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBSC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmBottomShadowColorDefault */


void 
_WmTopShadowColorDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmTSC, 0, _pAppearanceData->background,
	value);

} /* END OF FUNCTION _WmTopShadowColorDefault */


void 
_WmTopShadowPixmapDefault (Widget widget, int offset, XrmValue *value)
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->topShadowColor == _pAppearanceData->background))
    {
	/* Fix monochrome 3D appearance */
	string = (String) _50_foreground;
	if (_pAppearanceData->backgroundPStr != NULL)
	    if (!strcmp(_pAppearanceData->backgroundPStr, _25_foreground) ||
		!strcmp(_pAppearanceData->backgroundPStr, _50_foreground))
	    {
		string = (String) _foreground;
	    }
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmTopShadowPixmapDefault */


void 
_WmABackgroundDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBGC, _defaultActiveBackground, 0, value);

} /* END OF FUNCTION _WmABackgroundDefault */


void 
_WmAForegroundDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmFGC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmAForegroundDefault */

void 
_WmABackgroundPixmapDefault (Widget widget, int offset, XrmValue *value)
{
    static String string;


    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->activeTopShadowColor ==
	 				_pAppearanceData->activeBackground))
    {
	string = (String) _50_foreground;
    }
    else
    {
	string = NULL;
    }

    value->addr = (char *)string;
    value->size = sizeof (String);

} /* END OF FUNCTION _WmABackgroundPixmapDefault */

void 
_WmABottomShadowColorDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmBSC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmABottomShadowColorDefault */


void 
_WmATopShadowColorDefault (Widget widget, int offset, XrmValue *value)
{
    _WmGetDynamicDefault (widget, WmTSC, 0, _pAppearanceData->activeBackground,
	value);

} /* END OF FUNCTION _WmATopShadowColorDefault */


void 
_WmATopShadowPixmapDefault (Widget widget, int offset, XrmValue *value)
{
    static String string;

    if ((Monochrome (XtScreen (widget))) ||
	(_pAppearanceData->activeTopShadowColor ==
	                             _pAppearanceData->activeBackground))
    {
	/* Fix monochrome 3D appearance */
	string = (String) _50_foreground;
	if (_pAppearanceData->activeBackgroundPStr != NULL)
	    if (!strcmp
		    (_pAppearanceData->activeBackgroundPStr, _25_foreground) ||
		!strcmp
		    (_pAppearanceData->activeBackgroundPStr, _50_foreground))
	    {
		string = (String) _foreground;
	    }
    }
    else
    {
	string = NULL;
    }
    
    value->addr = (char *)string;
    value->size = sizeof (String);
    
} /* END OF FUNCTION _WmATopShadowPixmapDefault */


#ifdef WSM
void 
_WmBackdropBgDefault (Widget widget, int offset, XrmValue *value)
{
    static Pixel pixValue;
    unsigned int colorSetId = (unsigned int) pResWS->backdrop.colorSet;
    WmScreenData *pSD;

    if (wmGD.statusColorServer == CSERVE_NORMAL)
    {
	if ((colorSetId == 0) || (colorSetId > XmCO_MAX_NUM_COLORS))
	{
	    colorSetId = (unsigned int) DefaultWsColorSetId (pResWS);
	}

	switch (pResWS->pSD->colorUse)
	{
	    case XmCO_BLACK_WHITE:
		pixValue = pResWS->pSD->pPixelData[colorSetId-1].bg;
		break;

	    default:
	    case XmCO_LOW_COLOR:
	    case XmCO_MEDIUM_COLOR:
	    case XmCO_HIGH_COLOR:
		pixValue = pResWS->pSD->pPixelData[colorSetId-1].bs;
		break;
	}
    }
    else
    {
	/*
	 *  Color server is unavailable.  Has user specified a colorset?
	 *
	 *  If not, go monochrome.
	 *
	 */
	    pixValue = WhitePixel (DISPLAY, pResWS->pSD->screen);
    }

    /* return the dynamic default */

    value->addr = (char *) &pixValue;
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmBackdropBgDefault */

void 
_WmBackdropFgDefault (Widget widget, int offset, XrmValue *value)
{
    static Pixel pixValue;
    unsigned int colorSetId = (unsigned int) pResWS->backdrop.colorSet;
    WmScreenData *pSD;

    if (wmGD.statusColorServer == CSERVE_NORMAL)
    {
	if ((colorSetId == 0) || (colorSetId > XmCO_MAX_NUM_COLORS))
	{
	    colorSetId = (unsigned int) DefaultWsColorSetId (pResWS);
	}

	switch (pResWS->pSD->colorUse)
	{
	    case XmCO_BLACK_WHITE:
		pixValue = pResWS->pSD->pPixelData[colorSetId-1].fg;
		break;

	    default:
	    case XmCO_LOW_COLOR:
	    case XmCO_MEDIUM_COLOR:
	    case XmCO_HIGH_COLOR:
		pixValue = pResWS->pSD->pPixelData[colorSetId-1].bg;
		break;
	}
    }
    else
    {
	/*
	 *  Color server is unavailable.  Has user specified a colorset?
	 *
	 *  If not, go monochrome.
	 *
	 */
	    pixValue = BlackPixel (DISPLAY, pResWS->pSD->screen);
    }
    value->addr = (char *) &pixValue;
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmBackdropFgDefault */

void 
_WmBackdropColorSetDefault (Widget widget, int offset, XrmValue *value)
{
    static unsigned int colorSetId;

    if (wmGD.statusColorServer == CSERVE_NORMAL)
    {
	colorSetId = (unsigned int) DefaultWsColorSetId (pResWS);
    }
    else
    {
	colorSetId = 0; /* invalid color set */
    }

    value->addr = (char *) &colorSetId;
    value->size = sizeof (Pixel);

} /* END OF FUNCTION _WmBackdropColorSetIdDefault */

void 
_WmIconImageMaximumDefault (Widget widget, int offset, XrmValue *value)
{
    static WHSize cval;

    if ((pResSD->displayResolutionType == LOW_RES_DISPLAY) ||
	(pResSD->displayResolutionType == VGA_RES_DISPLAY))
    {
	cval.width = 32;
	cval.height = 32;
    }
    else
    {
	cval.width = 48;
	cval.height = 48;
    }

    value->addr = (char *)  &cval;
    value->size = sizeof (WHSize);

} /* END OF FUNCTION _WmIconImageMaximumDefault */


/*************************************<->*************************************
 *
 *  DefaultWsColorSetId (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function returns the default colorSetId for a given workspace
 *
 *
 *  Inputs:
 *  ------
 *  pWS = ptr to workspace data
 * 
 *  Outputs:
 *  -------
 *  return = default color set ID.
 *
 *  "active" and "inactive" color sets are not used.
 * 
 *************************************<->***********************************/

int 
DefaultWsColorSetId (WmWorkspaceData *pWS)
{
    static int _ws_high_color_map[] = { 3, 5, 6, 7 };
#define        _WS_HIGH_COLOR_COUNT	4    
    int i;
    WmScreenData *pSD;
    int iIndex;
    int rval = 8;

    if (pWS)
    {
	pSD = pWS->pSD;

	iIndex = (int) ((unsigned long)(pWS) - 
			  (unsigned long)(pSD->pWS))/sizeof(WmWorkspaceData);
	if (iIndex < 0) 
	    iIndex = 0; /* bad pWS or pSD, shouldn't get here */

	switch (pSD->colorUse)
	{
	    case XmCO_BLACK_WHITE:
	    case XmCO_LOW_COLOR:
		rval = 1 +
		  (pSD->pInactivePixelSet-pSD->pPixelData)/sizeof(XmPixelSet);
		break;

	    case XmCO_MEDIUM_COLOR:
		rval = HARD_CODED_PRIMARY;
		break;

	    case XmCO_HIGH_COLOR:
		i = iIndex % _WS_HIGH_COLOR_COUNT;
		rval = _ws_high_color_map[i];
		break;
	}
    }

    return (rval);

} /* END OF FUNCTION DefaultWsColorSetId */

#endif /* WSM */



/*************************************<->*************************************
 *
 *  _WmFocusAutoRaiseDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the focusAutoRaise resource.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 * 
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

void 
_WmFocusAutoRaiseDefault (Widget widget, int offset, XrmValue *value)
{
    static Boolean focusAutoRaise;

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	focusAutoRaise = True;
    }
    else
    {
	focusAutoRaise = False;
    }

    value->addr = (char *)&focusAutoRaise;
    value->size = sizeof (Boolean);

} /* END OF FUNCTION _WmFocusAutoRaiseDefault */


/*************************************<->*************************************
 *
 *  _WmMultiClickTimeDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the doubleClickTime resource.
 *  We dynamically default to the XtR4 multiClickTime value.
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

void 
_WmMultiClickTimeDefault (Widget widget, int offset, XrmValue *value)
{
    static int multiClickTime;

    multiClickTime = XtGetMultiClickTime(XtDisplay(widget));

    value->addr = (char *)&multiClickTime;
    value->size = sizeof (int);

} /* END OF FUNCTION _WmMultiClickTimeDefault */

#ifdef WSM

/*************************************<->*************************************
 *
 *  _WmSecondariesOnTopDefault (widget, offset, value)
 *
 *
 *  Description:
 *  -----------
 *  This function generates a default value for the secondariesOnTop 
 *  resource.
 *
 *  Inputs:
 *  ------
 *  widget = this is the parent widget for the wm subpart
 *
 *  offset = this is the resource offset
 *
 *  value = this is a pointer to a XrmValue in which to store the result
 *
 *  Outputs:
 *  -------
 *  value = default resource value and size
 * 
 *************************************<->***********************************/

void 
_WmSecondariesOnTopDefault (Widget widget, int offset, XrmValue *value)
{
    static Boolean secondariesOnTop;

    /*
     * Inherit setting from primary window if this window is 
     * secondary. 
     */

    if (_pCD->transientLeader != NULL)
	secondariesOnTop = _pCD->transientLeader->secondariesOnTop;
    else
	secondariesOnTop = True;

    value->addr = (char *)&secondariesOnTop;
    value->size = sizeof (Boolean);

} /* END OF FUNCTION _WmSecondariesOnTopDefault */
#endif /* WSM */



/******************************<->*************************************
 *
 *  ProcessWmResources ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve and process window manager resources
 *  that are not client-specific.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalResources = pointer to wm resource list
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

void 
ProcessWmResources (void)
{

    /*
     * Process the mwm general appearance and behavior resources.  Retrieve
     * a limited set of resource values if the window manager is starting
     * up with the standard behavior.
     */

    if (wmGD.useStandardBehavior)
    {
	XtGetApplicationResources (wmGD.topLevelW, (XtPointer) &wmGD,
	    wmStdGlobalResources, XtNumber (wmStdGlobalResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdGlobalResourceValues ();
    }
    else
    {
	XtGetApplicationResources (wmGD.topLevelW, (XtPointer) &wmGD,
	    wmGlobalResources, XtNumber (wmGlobalResources), NULL, 0);
    }

    if (wmGD.autoRaiseDelay < 0)
    {
       wmGD.autoRaiseDelay = 500;
       Warning (((char *)GETMESSAGE(62, 66, "Out of range autoRaiseDelay resource value. Must be non-negative")));
    }

} /* END OF FUNCTION ProcessWmResources */



/******************************<->*************************************
 *
 *  ProcessGlobalScreenResources ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve window manager resources to 
 *  determine the screens to manage.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalScreenResources = pointer to wm resource list
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

void 
ProcessGlobalScreenResources (void)
{
    XtGetApplicationResources (wmGD.topLevelW, &wmGD,
	wmGlobalScreenResources, 
	XtNumber (wmGlobalScreenResources), NULL, 0);

    if (wmGD.multiScreen)
    {
        wmGD.numScreens = ScreenCount(DISPLAY);
    }
    else
    {
	wmGD.numScreens = 1;
    }

    if (wmGD.screenList != NULL)
    {
	ProcessScreenListResource();
    }
}



/*************************************<->*************************************
 *
 *  SetStdGlobalResourceValues ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (global data filled out with resource values)
 * 
 *************************************<->***********************************/

void 
SetStdGlobalResourceValues (void)
{
    wmGD.autoKeyFocus = True;
    wmGD.clientAutoPlace = True;
    wmGD.colormapFocusPolicy = CMAP_FOCUS_KEYBOARD;
    wmGD.deiconifyKeyFocus = True;
    wmGD.doubleClickTime = 500;
    wmGD.freezeOnConfig = True;
    wmGD.iconAutoPlace = True;
    wmGD.iconClick = True;
    wmGD.interactivePlacement = False;
    wmGD.keyboardFocusPolicy = KEYBOARD_FOCUS_EXPLICIT;
    wmGD.lowerOnIconify = True;
    wmGD.passSelectButton = True;
    wmGD.startupKeyFocus = True;
    wmGD.systemButtonClick = True;
    wmGD.systemButtonClick2 = True;
#if defined(PANELIST)
    wmGD.useFrontPanel=False;
#endif /* PANELIST */

} /* END OF FUNCTION SetStdGlobalResourceValues */



/*************************************<->*************************************
 *
 *  ProcessScreenListResource ()
 *
 *
 *  Description:
 *  -----------
 *  This processes the names in the screenList resource.
 *
 *
 *  Inputs:
 *  ------
 *  wmGlobalResources = pointer to wmGD.screenList 
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.screenNames
 * 
 *************************************<->***********************************/

void 
ProcessScreenListResource (void)
{
    unsigned char *lineP;
    unsigned char *string;
    int sNum = 0;
    int nameCount = 0;

    lineP = (unsigned char *)wmGD.screenList;

    /*
     *  Parse screenList. 
     */
    while (((string = GetString(&lineP)) != NULL) && 
	   (sNum < ScreenCount(DISPLAY)))
    {
	if (!(wmGD.screenNames[sNum] = (unsigned char *) 
	    WmRealloc ((char*)wmGD.screenNames[sNum], strlen((char*)string)+1)))
	{
	    ExitWM(WM_ERROR_EXIT_VALUE);
	}
	else 
	{
	    strcpy((char *)wmGD.screenNames[sNum], (char *)string);
	    nameCount++;
	    sNum++;
	}
    }

    /*
     * If the number of listed screens (sNum) is < screen count, fill in the 
     * remaining screen names with the name of the first screen specified,
     * if such exists.
     */
    if (nameCount > 0)
    {
	string = wmGD.screenNames[0];    /* name of the first screen */
	while (sNum < ScreenCount(DISPLAY))
	{
	    if (!(wmGD.screenNames[sNum] = (unsigned char *) 
		WmRealloc ((char*)wmGD.screenNames[sNum], 
				strlen((char *)string)+1)))
	    {
		ExitWM(WM_ERROR_EXIT_VALUE);
	    }
	    else 
	    {
		strcpy((char *)wmGD.screenNames[sNum], (char *)string);
		sNum++;
	    }
	}
    }

	
} /* END OF FUNCTION ProcessScreenListResource */

#ifdef WSM

/******************************<->*************************************
 *
 *  ProcessWmColors ()
 *
 *
 *  Description:
 *  -----------
 *  Retrieve the color sets from the colorserver.
 *
 *  Inputs:
 *  ------
 *  none
 * 
 *  Outputs:
 *  -------
 *  modifies parts of global pixel sets
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

void 
ProcessWmColors (WmScreenData *pSD)
{
    short active, inactive, primary, secondary;

    if ((pSD->pPixelData = (XmPixelSet *) 
	    XtMalloc (XmCO_NUM_COLORS * sizeof(XmPixelSet)))) 
    {
        /*
	 *
	 *
	 *  ASSUMPTION:  If XmeGetPixelData() returns true,
	 *  we have a good color server at our disposal.
	 *
	 *
	 */
	if (XmeGetPixelData (pSD->screen, &pSD->colorUse,
				pSD->pPixelData, &active, &inactive,
				&primary, &secondary))
	{
	    pSD->pActivePixelSet = &(pSD->pPixelData[active]);
	    pSD->pInactivePixelSet = &(pSD->pPixelData[inactive]);
	    pSD->pPrimaryPixelSet = &(pSD->pPixelData[primary]);
	    pSD->pSecondaryPixelSet = &(pSD->pPixelData[secondary]);

            /*  Hack here.  The index "4" is the proper array reference.   */
	    /*  This is used because XmGetPixelData has not been properly  */
	    /*  updated.                                                   */
	    
	    pSD->pTextPixelSet = &(pSD->pPixelData[3]);

	    wmGD.statusColorServer = CSERVE_NORMAL;

	}
	else
	{
	    XtFree((char *)pSD->pPixelData);
	    pSD->pPixelData = NULL;
	    pSD->pActivePixelSet = NULL;
	    pSD->pInactivePixelSet = NULL;
	    pSD->pPrimaryPixelSet = NULL;
	    pSD->pSecondaryPixelSet = NULL;
	    pSD->pTextPixelSet = NULL;
	}
    }
    else 
    {
	Warning (((char *)GETMESSAGE(62, 22, "Insufficient memory for color data")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

} /* END OF FUNCTION ProcessWmColors */


/******************************<->*************************************
 *
 *  WriteOutXrmColors ()
 *
 *
 *  Description:
 *  -----------
 *  Update the XRM database with pixel values from the color server.
 *
 *  Inputs:
 *  ------
 *  pSD    = contains pixel sets
 * 
 *  Outputs:
 *  -------
 *  updated resource database
 *  
 *  Comments:
 *  --------
 *  N.B.  Must change to write out data on a PER-SCREEN basis.
 *        e.g., "Dtwm*0*background"
 ******************************<->***********************************/

static void 
WriteOutXrmColors (WmScreenData *pSD)
{
    XrmDatabase     db;
    XrmValue        value;
    int		    thisScreen = pSD->screen;
    XmPixelSet     *tpixset, *ppixset;
    XmPixelSet     *spixset;

    char *res_class;
    String screen_name;

    if (MwmBehavior)
    {
	res_class = WM_RESOURCE_CLASS;
    }
    else 
    {
	res_class = DT_WM_RESOURCE_CLASS;
    }

    screen_name = (String) wmGD.screenNames[pSD->screen];

    db = XtScreenDatabase(XScreenOfDisplay(DISPLAY, thisScreen));

    /** update the clients database with new colors **/
    value.size = sizeof(Pixel);

    /*
     *
     *    WM ACTIVE RESOURCES--e.g., for the active frame 
     *
     */
    if (pSD->pActivePixelSet)
    {
	tpixset = pSD->pActivePixelSet;
	spixset = pSD->pSecondaryPixelSet;

	if (pSD->colorUse == XmCO_BLACK_WHITE)
	{
	    /*
	     *
	     *  Limit ourselves here to the client (frame)
	     *  visuals
	     *
	     */

	    /* activeForeground */
	    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, WmNactiveForeground, 
			NULL), XtRPixel, &value);

	    /* activeBackground */
	    value.addr = (XtPointer) &(WhitePixel(DISPLAY, pSD->screen));

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, WmNactiveBackground,
			NULL), XtRPixel, &value);

	    XrmPutStringResource (&db, ResCat (res_class, screen_name, 
		     WmNactiveBackgroundPixmap, NULL),
		    _foreground);

	    /* activeTopShadow */
	    XrmPutStringResource (&db, ResCat (res_class, screen_name, 
		    WmNactiveTopShadowPixmap, NULL),
		_Dither);

	    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));
	    XrmPutResource (&db,
		ResCat (res_class, screen_name, WmNactiveTopShadowColor,
			NULL), XtRPixel, &value);

	    /* activeBottomShadow */
	    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));
	    XrmPutResource (&db,
		ResCat (res_class, screen_name, 
			WmNactiveBottomShadowColor, NULL), XtRPixel, &value);

	    XrmPutStringResource (&db, ResCat (res_class, screen_name, 
		    WmNactiveBottomShadowPixmap, NULL),
		    _foreground);
	}

	else /* active colors for non-BW systems */
	{
	    value.addr = (XtPointer) &(tpixset->bg);
	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, WmNactiveBackground, NULL),
		XtRPixel, &value);

	    value.addr = (XtPointer) &(tpixset->fg);
	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, WmNactiveForeground, NULL),
		XtRPixel, &value);

		value.addr = (XtPointer) &(tpixset->ts);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, WmNactiveTopShadowColor, NULL),
		    XtRPixel, &value);

		value.addr = (XtPointer) &(tpixset->bs);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, WmNactiveBottomShadowColor, NULL),
		    XtRPixel, &value);

		if (XmCO_DitherTopShadow(DISPLAY, thisScreen, tpixset)) 
		{
		    XrmPutStringResource (&db, 
			ResCat (res_class, screen_name, WmNactiveTopShadowPixmap, NULL),
			_Dither);

		}

		if (XmCO_DitherBottomShadow(DISPLAY, thisScreen, tpixset)) 
		{
		    XrmPutStringResource (&db,
			ResCat (res_class, screen_name, WmNactiveBottomShadowPixmap, 
				NULL),
			_Dither);

		}
	}
    }
    /*
     *
     *    WM INACTIVE colors--e.g., for dialogues
     *
     */
    if (pSD->pInactivePixelSet)
    {
	tpixset = pSD->pInactivePixelSet;
	spixset = pSD->pSecondaryPixelSet;

        if (pSD->colorUse == XmCO_BLACK_WHITE)
	{

	   /*
	    *
	    *  Set colors/pixmaps for the frames--leave the
	    *  menus out of it so that their text won't look
	    *  unsatisfactory against a dithered background.
	    *
	    */

	    /* foreground */
	    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNforeground), XtRPixel, &value);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNforeground), XtRPixel, &value);


	    /* background */
	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNbackgroundPixmap), _Dither);

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNbackgroundPixmap), _Dither);

	    value.addr = (XtPointer) &(WhitePixel(DISPLAY, pSD->screen));
	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNbackground), XtRPixel, &value);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNbackground), XtRPixel, &value);

	    /* topshadow */
	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNtopShadowPixmap), _foreground);

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNtopShadowPixmap), _foreground);

	    value.addr = (XtPointer) &(WhitePixel(DISPLAY, pSD->screen));
	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNtopShadowColor), XtRPixel, &value);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNtopShadowColor), XtRPixel, &value);


	    /* bottomshadow */
	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNbottomShadowPixmap), _foreground);

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNbottomShadowPixmap), _foreground);

	    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			WmNbottomShadowColor), XtRPixel, &value);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			WmNbottomShadowColor), XtRPixel, &value);

		/*
		 *
		 *  Ensure that the icon images have a black foreground and
		 *  a white background.
		 *
		 */
		    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageForeground,
				NULL), XtRPixel, &value);

		    value.addr = (XtPointer) &(WhitePixel(DISPLAY, pSD->screen));

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageBackground,
				NULL), XtRPixel, &value);

	    /* Now deal with XmCO_BLACK_WHITE Menus */

	    /* XmCO_BLACK_WHITE menu foreground */
	    value.addr = (XtPointer) &(tpixset->fg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNforeground), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE menu background */
	    value.addr = (XtPointer) &(tpixset->bg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNbackground), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE menu top shadow */

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNtopShadowPixmap), _50_foreground);

	    /* use foreground color for this pixmap */
	    value.addr = (XtPointer) &(tpixset->fg);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNtopShadowColor), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE menu bottom shadow */

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNbottomShadowPixmap), _75_foreground);

	    /* use foreground color for this pixmap */
	    value.addr = (XtPointer) &(tpixset->fg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			WmNbottomShadowColor), XtRPixel, &value);

	    /* Finally, deal with XmCO_BLACK_WHITE Confirm Boxes */

	    /* XmCO_BLACK_WHITE confirm box foreground */
	    value.addr = (XtPointer) &(spixset->fg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNforeground), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE confirm box background */
	    value.addr = (XtPointer) &(spixset->bg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNbackground), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE confirm box top shadow */

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNtopShadowPixmap), _50_foreground);

	    /* use foreground color */
	    value.addr = (XtPointer) &(spixset->fg);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNtopShadowColor), XtRPixel, &value);

	    /* XmCO_BLACK_WHITE confirm box bottom shadow */

	    XrmPutStringResource (&db, 
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNbottomShadowPixmap), _75_foreground);

	    /* use foreground color */
	    value.addr = (XtPointer) &(spixset->fg);

	    XrmPutResource (&db, 
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			WmNbottomShadowColor), XtRPixel, &value);

	    /* use select color for icon box trough color */	

	    value.addr = (XtPointer) &(tpixset->sc);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART, 
			XmNtroughColor), XtRPixel, &value);

	    /* use select color for arm and select colors in dialogs */	

	    value.addr = (XtPointer) &(spixset->sc);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			XmNarmColor), XtRPixel, &value);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			XmNselectColor), XtRPixel, &value);

	    XrmPutResource (&db,
		ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			XmNtroughColor), XtRPixel, &value);
	}
	else /* inactive colors for non-BW systems */
	{
		XmPixelSet     *fpixset;

		/*
		 * Set mwm component colors 
		 */
		value.addr = (XtPointer) &(tpixset->bg);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			    WmNbackground), XtRPixel, &value);

		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			    WmNbackground), XtRPixel, &value);

		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			    WmNbackground), XtRPixel, &value);

		value.addr = (XtPointer) &(spixset->bg);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			    WmNbackground), XtRPixel, &value);

		value.addr = (XtPointer) &(tpixset->ts);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			    WmNtopShadowColor), XtRPixel, &value);

		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			    WmNtopShadowColor), XtRPixel, &value);

		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			    WmNtopShadowColor), XtRPixel, &value);

		value.addr = (XtPointer) &(spixset->ts);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			    WmNtopShadowColor), XtRPixel, &value);

		value.addr = (XtPointer) &(tpixset->bs);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			    WmNbottomShadowColor), XtRPixel, &value);

		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			    WmNbottomShadowColor), XtRPixel, &value);

		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			    WmNbottomShadowColor), XtRPixel, &value);

		value.addr = (XtPointer) &(spixset->bs);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			    WmNbottomShadowColor), XtRPixel, &value);

		value.addr = (XtPointer) &(tpixset->fg);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART,
			    WmNforeground), XtRPixel, &value);

		XrmPutResource (&db,
		    ResCat (res_class, screen_name, (char *)ICON_FRAME_PART,
			    WmNforeground), XtRPixel, &value);

		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)MENU_ITEM_PART,
			    WmNforeground), XtRPixel, &value);

		value.addr = (XtPointer) &(spixset->fg);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART,
			    WmNforeground), XtRPixel, &value);

		/*
		 * Set select color only for menus and feedback mwm
		 * parts. Client and Icon parts aren't real widgets.
		 * Set client trough color for icon box.
		 */
		value.addr = (XtPointer) &(tpixset->sc);
		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)MENU_ITEM_PART, 
			    XmNselectColor), XtRPixel, &value);
	 
		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)MENU_ITEM_PART, 
			    XmNarmColor), XtRPixel, &value);

		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)MENU_ITEM_PART, 
			    XmNtroughColor), XtRPixel, &value);

		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)CLIENT_FRAME_PART, 
			    XmNtroughColor), XtRPixel, &value);

		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			    XmNselectColor), XtRPixel, &value);
	 
		value.addr = (XtPointer) &(spixset->sc);
		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			    XmNarmColor), XtRPixel, &value);

		XrmPutResource (&db,
		   ResCat (res_class, screen_name, (char *)FEEDBACK_FRAME_PART, 
			    XmNtroughColor), XtRPixel, &value);

		/*
		 * Set Dtwm dialog colors
		 */
		fpixset = pSD->pSecondaryPixelSet;

		value.addr = (XtPointer) &(fpixset->bg);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, XmNbackground, NULL),
		    XtRPixel, &value);

		value.addr = (XtPointer) &(fpixset->fg);
		XrmPutResource (&db,
		    ResCat (res_class, screen_name, XmNforeground, NULL),
		    XtRPixel, &value);

		value.addr = (XtPointer) &(fpixset->ts);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, XmNtopShadowColor, NULL),
		    XtRPixel, &value);

		value.addr = (XtPointer) &(fpixset->bs);
		XrmPutResource (&db, 
		    ResCat (res_class, screen_name, XmNbottomShadowColor, NULL),
		    XtRPixel, &value);

		/*
		 *
		 *  Set up the select color, as for buttons in the dialogue
		 *  boxes.
		 *
		 */
		 value.addr = (XtPointer) &(fpixset->sc);
		 XrmPutResource (&db,
		    ResCat (res_class, screen_name, XmNselectColor, NULL),
			XtRPixel, &value);
	 
		 /* value.addr = (XtPointer) &(fpixset->sc); */
		 XrmPutResource (&db,
		    ResCat (res_class, screen_name, XmNarmColor, NULL),
			XtRPixel, &value);

		 /* value.addr = (XtPointer) &(fpixset->sc); */
		 XrmPutResource (&db,
		    ResCat (res_class, screen_name, XmNtroughColor, NULL),
			XtRPixel, &value);

		if (XmCO_DitherTopShadow(DISPLAY, thisScreen, fpixset)) 
		{
		    XrmPutStringResource (&db, 
			ResCat (res_class, screen_name, WmNtopShadowPixmap, NULL),
			_Dither);

		    if (pSD->colorUse == XmCO_BLACK_WHITE)
		    {
			XrmPutStringResource (&db, 
			    ResCat (res_class, screen_name, 
				WmNbottomShadowPixmap, NULL),
			    _NoDither);

		    }
		}

		if (XmCO_DitherBottomShadow(DISPLAY, thisScreen, fpixset)) 
		{
		    XrmPutStringResource (&db, 
			ResCat (res_class, screen_name, WmNbottomShadowPixmap, NULL),
			_Dither);

		    if (pSD->colorUse == XmCO_BLACK_WHITE)
		    {
			XrmPutStringResource (&db, 
			    ResCat (res_class, screen_name, 
				WmNtopShadowPixmap, NULL),
			    _NoDither);

		    }
		}

		if (tpixset->bs != tpixset->ts)
		/*
		 *
		 *   If the inactive bottomshadow and topshadow are
		 *   different (i.e., valid), then make the icon image
		 *   use those colors.
		 */
		{
		    value.addr = (XtPointer) &(tpixset->bs);

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageForeground,
				NULL), XtRPixel, &value);

		    value.addr = (XtPointer) &(tpixset->ts);

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageBackground,
				NULL), XtRPixel, &value);

		    value.addr = (XtPointer) &(tpixset->bs);

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, 
				WmNiconImageBottomShadowColor,
				NULL), XtRPixel, &value);

		    value.addr = (XtPointer) &(tpixset->ts);

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, 
				WmNiconImageTopShadowColor,
				NULL), XtRPixel, &value);
		}
		else
		/*
		 *
		 *  Ensure that the icon images have a black foreground and
		 *  a white background.
		 *
		 */
		{
		    value.addr = (XtPointer) &(BlackPixel(DISPLAY, pSD->screen));

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageForeground,
				NULL), XtRPixel, &value);

		    value.addr = (XtPointer) &(WhitePixel(DISPLAY, pSD->screen));

		    XrmPutResource (&db, 
			ResCat (res_class, screen_name, WmNiconImageBackground,
				NULL), XtRPixel, &value);
		}
	}
    }


    if (pSD->pTextPixelSet)
    {
       value.addr = (XtPointer) &(pSD->pTextPixelSet->bg);

       XrmPutResource (&db,
          ResCat (res_class, screen_name, "XmTextField",
                  WmNbackground), XtRPixel, &value);

       XrmPutResource (&db,
          ResCat (res_class, screen_name, "XmText",
                  WmNbackground), XtRPixel, &value);
    }
}


/******************************<->*************************************
 *
 *  ResCat (s1,s2,s3,s4)
 *
 *
 *  Description:
 *  -----------
 *  Cats up to four strings together with '*' in between.
 *
 *
 *  Inputs:
 *  ------
 *  s1...s4 = pointers to Strings or NULL pointers (no string)
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to statically allocated string that has 
 *           the passed in string cat'ed together with '*'s 
 *           in between.
 *
 *  Comments:
 *  --------
 *  Does no limit checking on the static buffer 
 * 
 *************************************<->***********************************/

String
ResCat (String s1, String s2, String s3, String s4)
{


    int count;

    Boolean useResourceClass = True;

    wmGD.tmpBuffer[0] = '\0';

    count = MAXBUF - 1;

    if (s1)
    {
	if ((MwmBehavior) &&
	    !strcmp (s1, WM_RESOURCE_CLASS))
	/*
	 *
	 *  if this routine is called with a class name
	 *  ("Mwm" or "Dtwm"), then DON'T use it.
	 *  We want our resources to be written out
	 *  as:  *iconImageForeground:   <pixel_val>
	 *
	 *  as opposed to:  Dtwm*iconImageForeground:   <pixel_val>
	 *
	 */
	{
		useResourceClass = False;
	}
	else if (!strcmp (s1, DT_WM_RESOURCE_CLASS))
	{
		useResourceClass = False;
	}
	else
	{
	    strncat((char *)wmGD.tmpBuffer, s1, count);
	    count -= strlen(s1);
	}

	if (s2 && (count > 0))
	{
	    strncat ((char *)wmGD.tmpBuffer, "*", count);
	    count -= 1;
	    strncat ((char *)wmGD.tmpBuffer, s2, count);
	    count -= strlen (s2);

	    if (s3 && (count > 0))
	    {
		strncat ((char *)wmGD.tmpBuffer, "*", count);
		count -= 1;
		strncat ((char *)wmGD.tmpBuffer, s3, count);
		count -= strlen (s3);

		if (s4)
		{
		    strncat ((char *)wmGD.tmpBuffer, "*", count);
		    count -= 1;
		    strncat ((char *)wmGD.tmpBuffer, s4, count);
		}
	    }
	}
    }
    return ((String) wmGD.tmpBuffer);

} /* END OF FUNCTION ResCat */



/******************************<->*************************************
 *
 *  CheckForNoDither (pAD)
 *
 *
 *  Description:
 *  -----------
 *  Checks for reserved string as pixmap name of dither that indicates
 *  no dithering and replaces the string with a NULL.
 *
 *
 *  Inputs:
 *  ------
 *  pAD   = pointer to appearance data
 * 
 *  Outputs:
 *  -------
 *  pAD   = pointer to appearance data (may be modified)
 *
 *  Comments:
 *  ---------
 *  This check is done to avoid repeated calls to XmGetPixmap when
 *  managing windows. XmGetPixmap doesn't cache failures, and the
 *  NoDither string should fail every time. We want to prevent 
 *  XmGetPixmap from call XtResolvePathName to rummage through
 *  the file system.
 *
 *************************************<->***********************************/

void 
CheckForNoDither (AppearanceData *pAD)
{
    if (pAD->backgroundPStr && 
	!strcmp(pAD->backgroundPStr, _NoDither))
    {
	pAD->backgroundPStr = NULL;
    }
    if (pAD->bottomShadowPStr && 
	!strcmp(pAD->bottomShadowPStr, _NoDither))
    {
	pAD->bottomShadowPStr = NULL;
    }
    if (pAD->topShadowPStr && 
	!strcmp(pAD->topShadowPStr, _NoDither))
    {
	pAD->topShadowPStr = NULL;
    }
    if (pAD->activeBackgroundPStr && 
	!strcmp(pAD->activeBackgroundPStr, _NoDither))
    {
	pAD->activeBackgroundPStr = NULL;
    }
    if (pAD->activeBottomShadowPStr && 
	!strcmp(pAD->activeBottomShadowPStr, _NoDither))
    {
	pAD->activeBottomShadowPStr = NULL;
    }
    if (pAD->activeTopShadowPStr &&
	!strcmp(pAD->activeTopShadowPStr, _NoDither))
    {
	pAD->activeTopShadowPStr = NULL;
    }

} /* END OF FUNCTION CheckForNoDither */

#endif /* WSM */



/******************************<->*************************************
 *
 *  ProcessAppearanceResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Retrieve and process the general appearance resources for the mwm
 *  subparts: "client", "icon", and "feedback"
 *
 *
 *  Inputs:
 *  ------
 *  pSD   = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  modifies parts of global data wmGD.
 *
 *  Comments:
 *  --------
 *  o Changeable GCs are created with XCreateGC. The base GCs used for
 *    text output will have clip_masks defined for them later.
 *  
 * 
 *************************************<->***********************************/

void 
ProcessAppearanceResources (WmScreenData *pSD)
{
    Widget clientW;		/* dummy widget for resource fetching */
    int i;
    Arg args[10];


    /*
     * Get the client subpart resources:
     */

    /* save info in static globals for dynamic default processing */
    _defaultBackground = _defaultColor1;
    _defaultActiveBackground = _defaultColor2;
    _pAppearanceData = &(pSD->clientAppearance);

    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (XtPointer) &(pSD->clientAppearance),
	      WmNclient, WmCClient, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef WSM
    CheckForNoDither (&(pSD->clientAppearance));
#endif /* WSM */


    /*
     * Process the client resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->clientAppearance), True);


    /*
     * Get the client.title subpart resources:
     */

	/* insert "client" widget in hierarchy */

    i = 0;
    clientW = XtCreateWidget (WmNclient, xmRowColumnWidgetClass, 
			pSD->screenTopLevelW, (ArgList) args, i);


	/* fetch "client.title" subpart appearance resources */

    _pAppearanceData = &(pSD->clientTitleAppearance);

    (void)XtGetSubresources (clientW, (XtPointer) &(pSD->clientTitleAppearance),
	      WmNtitle, WmCTitle, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef WSM
    CheckForNoDither (&(pSD->clientTitleAppearance));
#endif /* WSM */


    /*
     * Process the client.title resource values:
     */


    /* 
     * check if client title appearance is different from the rest of frame.
     */
    if (SimilarAppearanceData (&(pSD->clientAppearance), 
			       &(pSD->clientTitleAppearance)))
    {
        /* title bar doesn't need special graphic processing */
	pSD->decoupleTitleAppearance = False;
    }
    else 
    {
	/* make background, top and bottom shadow pixmaps */
	MakeAppearanceResources (pSD, &(pSD->clientTitleAppearance), True);
	pSD->decoupleTitleAppearance = True;
    }

    XtDestroyWidget (clientW);	/* all done with dummy widget */


    /*
     * Get the icon subpart resources:
     */

    _pAppearanceData = &(pSD->iconAppearance);

    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (XtPointer) &(pSD->iconAppearance),
	      WmNicon, WmCIcon, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef WSM
    CheckForNoDither (&(pSD->iconAppearance));
#endif /* WSM */


    /*
     * Process the icon resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->iconAppearance), True);


    /*
     * Get the feedback subpart resources:
     * !!! only get "inactive" resources !!!
     */

    _defaultBackground = _defaultColor2;
    _defaultActiveBackground = _defaultColor2;
    _pAppearanceData = &(pSD->feedbackAppearance);

    (void)XtGetSubresources (pSD->screenTopLevelW, 
	      (XtPointer) &(pSD->feedbackAppearance),
	      WmNfeedback, WmCFeedback, wmAppearanceResources, 
	      XtNumber (wmAppearanceResources), NULL, 0);
#ifdef WSM
    CheckForNoDither (&(pSD->feedbackAppearance));
#endif /* WSM */

    /*
     * Process the feedback resource values:
     */

    /* make background, top and bottom shadow pixmaps */

    MakeAppearanceResources (pSD, &(pSD->feedbackAppearance), False);


} /* END OF FUNCTION ProcessAppearanceResources */


/*************************************<->*************************************
 *
 *  MakeAppearanceResources (pSD, pAData, makeActiveResources)
 *
 *
 *  Description:
 *  -----------
 *  This function makes top, bottom and background pixmaps for a window
 *  manager component.  Inactive and active (if specified) GC's are
 *  also made.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 *  pAData = pointer to appearance data structure containing resource info
 *
 *  makeActiveResources = if True then make active resources
 * 
 *  Outputs:
 *  -------
 *  *pAData = pixmap and GC fields filled out
 *
 *************************************<->***********************************/

void 
MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources)
{
    Pixel foreground;

    /*
     * Extract a font from the font list.
     */

    if (! XmeRenderTableGetDefaultFont(pAData->fontList, &(pAData->font)))
    {
	sprintf((char *)wmGD.tmpBuffer, ((char *)GETMESSAGE(62, 23, "failed to load font: %.100s\0")), (char*) pAData->fontList);
	Warning((char *)wmGD.tmpBuffer);
	ExitWM(WM_ERROR_EXIT_VALUE);
    }

#ifndef NO_MULTIBYTE
    /*
     *  Calculate title bar's height and store it in pAData.
     */
    pAData->titleHeight = (pAData->font)->ascent + (pAData->font)->descent
        + WM_TITLE_BAR_PADDING;
#endif


    /*
     * Make standard (inactive) appearance resources.
     */

    /* background pixmap */

    if (pAData->backgroundPStr)
    {
	pAData->backgroundPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY, 
					   pSD->screen),
				       pAData->backgroundPStr,
				       pAData->foreground,
				       pAData->background);

	if (pAData->backgroundPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->backgroundPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->backgroundPixmap = (Pixmap)NULL;
    }

    /* top shadow pixmap */

    if (pAData->topShadowPStr)
    {
	/*
	 * Make sure top shadow color is not the same as background
	 * otherwise the wrong pixmap will be generated.
	 */
	if (pAData->topShadowColor != pAData->background)
	    foreground = pAData->topShadowColor;
	else
	    foreground = pAData->foreground;
	pAData->topShadowPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY,
					   pSD->screen),
				       pAData->topShadowPStr,
				       foreground,
				       pAData->background);

	if (pAData->topShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->topShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->topShadowPixmap = (Pixmap)NULL;
    }


    /* bottom shadow pixmap */

    if (pAData->bottomShadowPStr)
    {
	/*
	 * Make sure bottom shadow color is not the same as background
	 * otherwise the wrong pixmap will be generated.
	 */
	if (pAData->bottomShadowColor != pAData->background)
	    foreground = pAData->bottomShadowColor;
	else
	    foreground = pAData->foreground;
	pAData->bottomShadowPixmap = XmGetPixmap (
			               ScreenOfDisplay (DISPLAY,
					   pSD->screen),
				       pAData->bottomShadowPStr,
				       foreground,
				       pAData->background);

	if (pAData->bottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->bottomShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->bottomShadowPixmap = (Pixmap)NULL;
    }

    /* inactive appearance GC */

    GetAppearanceGCs (pSD,
		      pAData->foreground,
		      pAData->background,
		      pAData->font,
		      pAData->backgroundPixmap,
		      pAData->topShadowColor,
		      pAData->topShadowPixmap,
		      pAData->bottomShadowColor,
		      pAData->bottomShadowPixmap,
		      &(pAData->inactiveGC),
		      &(pAData->inactiveTopShadowGC),
		      &(pAData->inactiveBottomShadowGC));



    /*
     * Make active apppearance resources if specified.
     */

    if (!makeActiveResources)
    {
	return;
    }

    /* active background pixmap */

    if (pAData->activeBackgroundPStr)
    {
	pAData->activeBackgroundPixmap = XmGetPixmap (
			                     ScreenOfDisplay (DISPLAY,
						 pSD->screen),
				             pAData->activeBackgroundPStr,
				             pAData->activeForeground,
				             pAData->activeBackground);

	if (pAData->activeBackgroundPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeBackgroundPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeBackgroundPixmap = (Pixmap)NULL;
    }

    /* active top shadow pixmap */

    if (pAData->activeTopShadowPStr)
    {
	pAData->activeTopShadowPixmap = XmGetPixmap (
			                    ScreenOfDisplay (DISPLAY,
						pSD->screen),
				            pAData->activeTopShadowPStr,
				            pAData->activeTopShadowColor,
				            pAData->activeBackground);

	if (pAData->activeTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeTopShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeTopShadowPixmap = (Pixmap)NULL;
    }


    /* active bottom shadow pixmap */

    if (pAData->activeBottomShadowPStr)
    {
	pAData->activeBottomShadowPixmap = XmGetPixmap (
			                       ScreenOfDisplay (DISPLAY,
						   pSD->screen),
				               pAData->activeBottomShadowPStr,
				               pAData->activeBottomShadowColor,
				               pAData->activeBackground);

	if (pAData->activeBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	{
	    pAData->activeBottomShadowPixmap = (Pixmap)NULL;
	}
    }
    else
    {
	pAData->activeBottomShadowPixmap = (Pixmap)NULL;
    }

    /* inactive appearance GC */

    GetAppearanceGCs (pSD,
		      pAData->activeForeground,
		      pAData->activeBackground,
		      pAData->font,
		      pAData->activeBackgroundPixmap,
		      pAData->activeTopShadowColor,
		      pAData->activeTopShadowPixmap,
		      pAData->activeBottomShadowColor,
		      pAData->activeBottomShadowPixmap,
		      &(pAData->activeGC),
		      &(pAData->activeTopShadowGC),
		      &(pAData->activeBottomShadowGC));


} /* END OF FUNCTION MakeAppearanceResources */



/*************************************<->*************************************
 *
 *  GetAppearanceGCs (pSD, fg, bg, font, bg_pixmap, ts_color, 
 *                    ts_pixmap, bs_color, bs_pixmap, pGC, ptsGC, pbsGC)
 *
 *
 *  Description:
 *  -----------
 *  Creates the appearance GCs for any of the icon, client, or feedback 
 *  resources.
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  fg		- base foreground color
 *  bg		- base background color
 *  font	- font
 *  bg_pixmap	- background pixmap
 *  ts_color	- top shadow color
 *  ts_pixmap	- top shadow pixmap
 *  bs_color	- bottom shadow color
 *  bs_pixmap	- bottom shadow pixmap
 *  pGC		- pointer to location to receive base GC
 *  ptsGC	- pointer to location to receive top shadow GC
 *  pbsGC	- pointer to location to receive bottom shadow GC
 * 
 *  Outputs:
 *  -------
 *  *pGC	- base GC
 *  *ptsGC	- top shadow GC
 *  *pbsGC	- bottom shadow GC
 *  
 *
 *  Comments:
 *  --------
 * 
 * 
 *************************************<->***********************************/

void 
GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC)
{
    XGCValues gcv;
    XtGCMask  mask;


    /*
     * Get base GC
     */

    mask = GCForeground | GCBackground | GCFont;
    gcv.foreground = fg;
    gcv.background = bg;
    gcv.font = font->fid;

    if (bg_pixmap)
    {
	mask |= GCTile;
	gcv.tile = bg_pixmap;
    }

    *pGC = XCreateGC (DISPLAY, pSD->rootWindow, mask, &gcv);

    /*
     * !!! Need GC error detection !!!
     */

    *ptsGC = GetHighlightGC (pSD, ts_color, bg, ts_pixmap);

    *pbsGC = GetHighlightGC (pSD, bs_color, bg, bs_pixmap);

} /* END OF FUNCTION GetAppearanceGCs */




/*************************************<->*************************************
 *
 *  ProcessScreenResources (pSD, screenName)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are screen specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  screenName = name of screen
 *
 * 
 *  Outputs:
 *  -------
 *  pSD = resource data for screen is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on workspace name
 * 
 *************************************<->***********************************/

void 
ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName)
{
#ifdef WSM
    pResSD = pSD;	/* save current screen data for default processing */
    /*
     * Use the screen name (e.g., "0") as the default resource name.
     */

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (wmGD.topLevelW, (XtPointer) pSD, 
	    (String) screenName, 
	    (String) screenName,
	    wmStdScreenResources, 
	    XtNumber (wmStdScreenResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdScreenResourceValues (pSD);
    }
    else
    {
	XtGetSubresources (wmGD.topLevelW, (XtPointer) pSD, 
	    (String)screenName, (String) screenName,
	    wmScreenResources, 
	    XtNumber (wmScreenResources), NULL, 0);

#ifndef MOTIF_ONE_DOT_ONE
	pSD->moveOpaque = (((XmScreen) XmGetXmScreen(XtScreen(pSD->screenTopLevelW)))
			   -> screen.moveOpaque);
#endif
    }

#else /* WSM */
    /*
     * Retrieve screen specific resources.
     */

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (wmGD.topLevelW, (XtPointer) pSD, 
	    (String) screenName, (String)screenName, wmStdScreenResources, 
	    XtNumber (wmStdScreenResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdScreenResourceValues (pSD);
    }
    else
    {
	XtGetSubresources (wmGD.topLevelW, (XtPointer) pSD, 
	    (String)screenName, (String)screenName, wmScreenResources, 
	    XtNumber (wmScreenResources), NULL, 0);

#ifndef MOTIF_ONE_DOT_ONE
	pSD->moveOpaque =(((XmScreen) XmGetXmScreen(XtScreen(pSD->screenTopLevelW)))
			  -> screen.moveOpaque);
#endif
    }
#endif /* WSM */

    /*
     * Do some additional processing on the window manager resource values.
     */


    if (pSD->iconImageMinimum.width < ICON_IMAGE_MIN_WIDTH)
    {
	pSD->iconImageMinimum.width = ICON_IMAGE_MIN_WIDTH;
    }
    else if (pSD->iconImageMinimum.width > ICON_IMAGE_MAX_WIDTH)
    {
	pSD->iconImageMinimum.width = ICON_IMAGE_MAX_WIDTH;
    }

    if (pSD->iconImageMinimum.height < ICON_IMAGE_MIN_HEIGHT)
    {
	pSD->iconImageMinimum.height = ICON_IMAGE_MIN_HEIGHT;
    }
    else if (pSD->iconImageMinimum.height > ICON_IMAGE_MAX_HEIGHT)
    {
	pSD->iconImageMinimum.height = ICON_IMAGE_MAX_HEIGHT;
    }

    if (pSD->iconImageMaximum.width < pSD->iconImageMinimum.width)
    {
	pSD->iconImageMaximum.width = pSD->iconImageMinimum.width;
    }
    else if (pSD->iconImageMaximum.width > ICON_IMAGE_MAX_WIDTH)
    {
	pSD->iconImageMaximum.width = ICON_IMAGE_MAX_WIDTH;
    }

    if (pSD->iconImageMaximum.height < pSD->iconImageMinimum.height)
    {
	pSD->iconImageMaximum.height = pSD->iconImageMinimum.height;
    }
    else if (pSD->iconImageMaximum.height > ICON_IMAGE_MAX_HEIGHT)
    {
	pSD->iconImageMaximum.height = ICON_IMAGE_MAX_HEIGHT;
    }

    if (pSD->iconPlacementMargin > MAXIMUM_ICON_MARGIN)
    {
	pSD->iconPlacementMargin = MAXIMUM_ICON_MARGIN;
    }

    if (pSD->maximumMaximumSize.width <= 0)
    {
	pSD->maximumMaximumSize.width =
			2 * DisplayWidth (DISPLAY, pSD->screen);
    }

    if (pSD->maximumMaximumSize.height <= 0)
    {
	pSD->maximumMaximumSize.height =
			2 * DisplayHeight (DISPLAY, pSD->screen);
    }

    /*
     * Set the icon appearance default based on whether or not the icon box
     * is being used.
     */

    if (pSD->iconDecoration & USE_ICON_DEFAULT_APPEARANCE)
    {
	if (pSD->useIconBox)
	{
	    pSD->iconDecoration = ICON_APPEARANCE_ICONBOX;
	}
	else
	{
	    pSD->iconDecoration = ICON_APPEARANCE_STANDALONE;
	}
    }

    /*
     * If resizeBorderWidth or frameBorderWidth is unset then initialize
     * to dynamic defaults.
     */

    if ((pSD->resizeBorderWidth == (Dimension)BIGSIZE) ||
	(pSD->frameBorderWidth == (Dimension)BIGSIZE))
    {
	double xres, yres, avg_res;

	xres = (((double) DisplayWidth(DISPLAY, pSD->screen)) / 
		((double) DisplayWidthMM(DISPLAY, pSD->screen)));
	yres = (((double) DisplayHeight(DISPLAY, pSD->screen)) / 
		((double) DisplayHeightMM(DISPLAY, pSD->screen)));

	avg_res = (xres + yres) / 2.0;

	/* Multiply times width in mm (avg. 7-8 pixels) */
	if (pSD->resizeBorderWidth == (Dimension)BIGSIZE)
	{
	    pSD->resizeBorderWidth = (int) (avg_res * 2.2);

	    /* limit size because big borders look ugly */
#ifndef WSM
	    if (wmGD.frameStyle == WmSLAB)
	    {
#endif /* WSM */
		if (pSD->resizeBorderWidth > 6) pSD->resizeBorderWidth = 6;
#ifndef WSM
	    }
	    else
	    {
		if (pSD->resizeBorderWidth > 7) pSD->resizeBorderWidth = 7;
	    }
#endif /* WSM */
	}

	/* Multiply times width in mm (avg. 5-6 pixels) */
	if (pSD->frameBorderWidth == (Dimension)BIGSIZE)
	{
	    pSD->frameBorderWidth = (int) (avg_res * 1.7);

	    /* limit size because big borders look ugly */
            if (wmGD.frameStyle == WmSLAB)
	    {
		if (pSD->frameBorderWidth > 4) pSD->frameBorderWidth = 4;
	    }
	    else
	    {
		if (pSD->frameBorderWidth > 5) pSD->frameBorderWidth = 5;
	    }
	}
    }


    pSD->externalBevel = FRAME_EXTERNAL_SHADOW_WIDTH;
    pSD->joinBevel = FRAME_INTERNAL_SHADOW_WIDTH;
    if (pSD->frameBorderWidth < 
	   (pSD->externalBevel + MIN_INTERNAL_BEVEL))
    {
	pSD->frameBorderWidth = 
	    pSD->externalBevel + MIN_INTERNAL_BEVEL;
    }
    else if (pSD->frameBorderWidth > MAXIMUM_FRAME_BORDER_WIDTH)
    {
	pSD->frameBorderWidth = MAXIMUM_FRAME_BORDER_WIDTH;
    }

    if (pSD->resizeBorderWidth < 
	   (pSD->externalBevel + MIN_INTERNAL_BEVEL))
    {
	pSD->resizeBorderWidth = 
	    (pSD->externalBevel + MIN_INTERNAL_BEVEL);
    }
    else if (pSD->resizeBorderWidth > MAXIMUM_FRAME_BORDER_WIDTH)
    {
	pSD->resizeBorderWidth = MAXIMUM_FRAME_BORDER_WIDTH;
    }
#ifdef WSM

    /*
     * Update the resource database.
     */
    WriteOutXrmColors (pSD);

#endif /* WSM */

    /*
     * Process the component appearance resources for client, 
     * icon and feedback parts of mwm.
     */

     ProcessAppearanceResources (pSD);

#ifdef WSM
    /* 
     * Process the workspace list and name the initial
     * workspaces
     */

     ProcessWorkspaceList (pSD);

     /*
      * Process default backdrop images to be used in low-color
      * situations
      */
     ProcessDefaultBackdropImages (pSD);
    
#endif /* WSM */
    /*
     * Save the default icon pixmap in global data. We'll use it only
     * as a last resort.
     */

    pSD->builtinIconPixmap = 
	XCreateBitmapFromData (DISPLAY, pSD->rootWindow, (char *)iImage_bits, 
				       iImage_width, iImage_height);

} /* END OF FUNCTION ProcessScreenResources */

#ifdef WSM

/*************************************<->*************************************
 *
 *  ProcessDefaultBackdropImages (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function processes the default backdrop images to be used
 *  in low color or black and white workspaces.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 * 
 *  Outputs:
 *  -------
 *  pSD = resource data for screen is set
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
ProcessDefaultBackdropImages (WmScreenData *pSD)
{
} /* END OF FUNCTION ProcessDefaultBackdropImages */



/*************************************<->*************************************
 *
 *  ProcessWorkspaceList (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function processes the workspaceCount and workspaceList 
 *  resources for a particular screen. It creates space for the initial 
 *  workspace data structures and adds in names for workspaces.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 * 
 *  Outputs:
 *  -------
 *  pSD = resource data for screen is set
 *
 *
 *  Comments:
 *  --------
 *  NOTE: The workspaceCount resource has precedence over the
 *  workspaceList resource. workspaceCount determines the number of
 *  workspaces to create for the screen. Once the number is determined,
 *  workspaceList is used to fill in the "names." If workspaceList is
 *  not present or doesn't have enough names, then missing names are
 *  generated automatically. If workspaceList is present and
 *  workspaceCount is not present, then the workspaceCount is determined
 *  by the number of names in workspaceList.
 * 
 *************************************<->***********************************/

void 
ProcessWorkspaceList (WmScreenData *pSD)
{
    int i, wsNameCount, wsNamesAlloced;
    WmWorkspaceData *pwsI;
    unsigned char *lineP = NULL;
    unsigned char *string;
    Boolean bHaveWorkspaceList;
    Boolean bHaveWorkspaceCount;
    char **ppchWsNames = NULL;

    /*
     * Validate initial resource settings
     */
    bHaveWorkspaceCount = (pSD->numWorkspaces >= 1);
    bHaveWorkspaceList = (pSD->workspaceList != NULL);

    if (bHaveWorkspaceList)
    {
	/*
	 * Parse out array of workspace names
	 */
	wsNamesAlloced = WS_ALLOC_AMOUNT;
	ppchWsNames = (char **) XtMalloc (wsNamesAlloced * sizeof (char *));
	if (pSD->workspaceList)
	{
	    lineP = (unsigned char *) pSD->workspaceList;
	}
	else
	{
	    lineP = (unsigned char *)NULL;
	}
	wsNameCount = 0;

	while (((string = GetString(&lineP)) != NULL))
	{
	    ppchWsNames[wsNameCount] = (char *) string;

	    if (++wsNameCount >= wsNamesAlloced)
	    {
	       /*
		*  Need to add more workspaces
		*/
		wsNamesAlloced += WS_ALLOC_AMOUNT;
		if (!(ppchWsNames = (char **) XtRealloc 
			  ((char *)ppchWsNames,
			    wsNamesAlloced * sizeof(char *))))
		{
		    ExitWM (WM_ERROR_EXIT_VALUE);
		}
	    }
	}

	if (!bHaveWorkspaceCount)
	{
	    pSD->numWorkspaces = wsNameCount;
	}
    }
    else if (!bHaveWorkspaceCount)
    {
	/*
	 * Neither workspaceCount nor workspaceList specified!!
	 * Assume one workspace.
	 */
	pSD->numWorkspaces = 1;
    }

    if (pSD->numWorkspaces > MAX_WORKSPACE_COUNT)
	    pSD->numWorkspaces = MAX_WORKSPACE_COUNT;

    /*
     *  Allocate the array of workspace data
     */
    pSD->numWsDataAllocated = (pSD->numWorkspaces + WS_ALLOC_AMOUNT);
    pSD->numWsDataAllocated -= pSD->numWsDataAllocated % WS_ALLOC_AMOUNT;
    if (!(pSD->pWS = (WmWorkspaceData *) 
	    XtMalloc (pSD->numWsDataAllocated * sizeof(WmWorkspaceData))))
    {
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

    pwsI = pSD->pWS;

    for (i = 0; i < pSD->numWorkspaces; i++, pwsI++)
    {
	if (bHaveWorkspaceList && i < wsNameCount)
	{
	    string = (unsigned char *) ppchWsNames[i];
	}
	else
	{
	    string = GenerateWorkspaceName (pSD, i);
	}
	if (!(pwsI->name = (String) XtMalloc (1+strlen((char *)string))))
	{
	    Warning (((char *)GETMESSAGE(62, 27, "Insufficient memory for workspace data")));
	    ExitWM(WM_ERROR_EXIT_VALUE);
	}
	else 
	{
	    strcpy(pwsI->name, (char *)string);
	}
    }

    if (ppchWsNames) XtFree ((char *) ppchWsNames);

} /* END OF FUNCTION ProcessWorkspaceList */

#endif /* WSM */


/******************************<->*************************************
 *
 *  ProcessWorkspaceResources (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are workspace specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 * 
 *  Outputs:
 *  -------
 *  pWS = resource data for workspace is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on workspace name
 * 
 ******************************<->***********************************/

void 
ProcessWorkspaceResources (WmWorkspaceData *pWS)
{

    /*
     * Retrieve workspace specific resources.
     */
#ifdef WSM
    pResWS = pWS;	/* save current ws for default processing */
#endif /* WSM */

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (pWS->pSD->screenTopLevelW, (XtPointer) pWS, 
	    pWS->name, pWS->name, wmStdWorkspaceResources, 
	    XtNumber (wmStdWorkspaceResources), NULL, 0);

	/*
	 * Fill in the standard resource values.
	 *
	 * (no code for this right now)
	 */
#ifdef WSM
        pWS->iconBoxGeometry = NULL;
#endif /* WSM */
    }
    else
    {
	XtGetSubresources (pWS->pSD->screenTopLevelW, (XtPointer) pWS, 
	    pWS->name, pWS->name, wmWorkspaceResources, 
	    XtNumber (wmWorkspaceResources), NULL, 0);

#ifdef WSM
        /*  Dup iconbox geometry, it may be free'd later on.  */

        if (pWS->iconBoxGeometry)
        {
            pWS->iconBoxGeometry = XtNewString (pWS->iconBoxGeometry);
        }
#endif /* WSM */
    }

#ifdef WSM
    if (pWS->title == NULL)
    {
	/*
	 * Setup default workspace title 
	 */
	pWS->title = XmStringCreateLocalized(pWS->name);
    }
    else
    {
       /*
	* Copy resource just in case there's a duplicate
	* Duplicates point to the same data, freeing on 
	* rename can cause a crash.
	*/
	pWS->title = XmStringCopy(pWS->title);

    }

    /*
     * Dup iconbox geometry, it may be free'd later on.
     */
    if (pWS->iconBoxGeometry)
    {
	pWS->iconBoxGeometry = XtNewString (pWS->iconBoxGeometry);
    }

    /*
     * Get backdrop resources
     */
    XtGetSubresources (pWS->workspaceTopLevelW, 
	(XtPointer) &(pWS->backdrop), 
	WmNbackdrop, WmCBackdrop, wmBackdropResources, 
	XtNumber (wmBackdropResources), NULL, 0);

    ProcessBackdropResources (pWS, NULL);
#endif /* WSM */

} /* END OF FUNCTION ProcessWorkspaceResources */

#ifdef WSM

/******************************<->*************************************
 *
 *  ProcessPresenceResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources for the workspace presence
 *  dialog.
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *
 * 
 *  Outputs:
 *  -------
 *  pSD = resource data for workspace presence dialog are set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources 
 * 
 ******************************<->***********************************/

void 
ProcessPresenceResources (WmScreenData *pSD)
{
#ifndef NO_MESSAGE_CATALOG
    static char *default_ws_pres_title = NULL;
#else
    static char *default_ws_pres_title = "Workspace Presence";
#endif
    Arg args[5];
    int n;
    unsigned char *pch1, *pch2;

#ifndef NO_MESSAGE_CATALOG
    /* 
     * Set up localized default title string on initial time through
     */
    if (default_ws_pres_title == NULL)
    {
	char * tmpString; 
	/*
	 * catgets returns a pointer to an area that is over written 
	 * on each call to catgets.  
	 */

	tmpString = ((char *)GETMESSAGE(62, 59, "Occupy Workspace"));
	if ((default_ws_pres_title =
	     (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
	{
	    Warning (((char *)GETMESSAGE(62, 31, "Insufficient memory for local message string")));
	    default_ws_pres_title = "Occupy Workspace";
	}
	else
	{
	    strcpy(default_ws_pres_title, tmpString);
	}
    }
#endif

    if (pSD->presence.shellW)
    {
	XtGetSubresources (pSD->presence.shellW, (XtPointer) &pSD->presence, 
		WmNworkspacePresence, WmCWorkspacePresence, 
		wmWsPresenceResources, 
		XtNumber (wmWsPresenceResources), NULL, 0);

	pch2 = NULL;

	if (pSD->presence.title)
	{
	    pch1 = (unsigned char *) 
		    WmXmStringToString (pSD->presence.title);

	    if (pch1 && (pch2 = (unsigned char *) 
				XtMalloc (1+strlen((char *)pch1))))
	    {
		strcpy ((char *)pch2, (char *)pch1);
	    }
	}

	if (!pch2)
	{
	    pch2 = (unsigned char *) default_ws_pres_title;
	}

	n = 0;
	XtSetArg (args[n], XmNtitle, pch2);		n++;
	XtSetValues (pSD->presence.shellW, args, n);
    }

} /* END OF FUNCTION ProcessPresenceResources */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  ProcessClientResources (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves resources that are client specific.  If the
 *  window manager is providing standard behavior then retrieve the limited
 *  set of resources that don't affect standard behavior and set the
 *  values of the other resources to the standard values.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = resource data for client is set
 *
 *
 *  Comments:
 *  --------
 *  o Gets subresources based on client name and class.
 *  o Creates GC for the client Matte, if there is one.
 * 
 *************************************<->***********************************/

void 
ProcessClientResources (ClientData *pCD)
{
    String clientName;
    String clientClass;
    WmScreenData *pSD = pCD->pSD;

    /*
     * Retrieve basic client specific resources.
     */

    _pCD = pCD;	/* save in static global for dynamic default processing */
    clientName = (pCD->clientName) ? pCD->clientName : WmNdefaults;
    clientClass = (pCD->clientClass) ? pCD->clientClass : WmNdefaults;

    if (wmGD.useStandardBehavior)
    {
	XtGetSubresources (pSD->screenTopLevelW, (XtPointer) pCD, clientName,
	    clientClass, wmStdClientResources, XtNumber (wmStdClientResources),
	    NULL, 0);

	/*
	 * Fill in the standard resource values.
	 */

	SetStdClientResourceValues (pCD);
    }
    else
    {
	XtGetSubresources (pSD->screenTopLevelW, (XtPointer) pCD, clientName,
	    clientClass, wmClientResources, XtNumber (wmClientResources), NULL,
	    0);
    }

#ifdef NO_MESSAGE_CATALOG
    /*
     * If (window menu spec is not found) then use the builtin
     * system menu.
     */

    if ((pCD->systemMenu == defaultSystemMenuName) &&
	(pSD->defaultSystemMenuUseBuiltin == TRUE))
    {
	pCD->systemMenu = builtinSystemMenuName;
    }
#endif

    /*
     * If the client decorations or client functions have been defaulted
     * fix up the fields in the ProcessMwmHints function.
     */


    /* make top and bottom shadow pixmaps */

    if (pCD->iconImageBottomShadowPStr)
    {
	if ((pCD->iconImageBottomShadowPStr ==
		    pSD->iconAppearance.bottomShadowPStr) &&
	    (pCD->iconImageBottomShadowColor ==
		    pSD->iconAppearance.bottomShadowColor) &&
	    (pCD->iconImageBackground == 
		    pSD->iconAppearance.background))
	{
	    pCD->iconImageBottomShadowPixmap =
		    pSD->iconAppearance.bottomShadowPixmap;
	}
	else
	{
	    pCD->iconImageBottomShadowPixmap =
			    XmGetPixmap ( ScreenOfDisplay (DISPLAY,
				              pSD->screen),
				          pCD->iconImageBottomShadowPStr,
				          pCD->iconImageBottomShadowColor,
				          pCD->iconImageBackground);

	    if (pCD->iconImageBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	    {
	        pCD->iconImageBottomShadowPixmap = (Pixmap)NULL;
	    }
	}
    }
    else
    {
	pCD->iconImageBottomShadowPixmap = (Pixmap)NULL;
    }

    if (pCD->iconImageTopShadowPStr)
    {
	if ((pCD->iconImageTopShadowPStr ==
				pSD->iconAppearance.topShadowPStr) &&
	    (pCD->iconImageTopShadowColor ==
				pSD->iconAppearance.topShadowColor) &&
	    (pCD->iconImageBackground == pSD->iconAppearance.background))
	{
	    pCD->iconImageTopShadowPixmap =
					pSD->iconAppearance.topShadowPixmap;
	}
	else
	{
	    pCD->iconImageTopShadowPixmap =
			    XmGetPixmap ( ScreenOfDisplay (DISPLAY,
				              pSD->screen),
				          pCD->iconImageTopShadowPStr,
				          pCD->iconImageTopShadowColor,
				          pCD->iconImageBackground);

	    if (pCD->iconImageTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	    {
	        pCD->iconImageTopShadowPixmap = (Pixmap)NULL;
	    }
	}
    }
    else
    {
	pCD->iconImageTopShadowPixmap = (Pixmap)NULL;
    }

    if ((pCD->internalBevel < MIN_INTERNAL_BEVEL)  || 
	(pCD->internalBevel > MAX_INTERNAL_BEVEL))
    {
	pCD->internalBevel = MAX_INTERNAL_BEVEL;
    }


    /*
     * Retrieve matte resources and make internal matte resources.
     */

    if (pCD->matteWidth > 0)
    {
	XtGetSubresources (pSD->screenTopLevelW, (XtPointer) pCD, clientName,
	    clientClass, wmClientResourcesM, XtNumber (wmClientResourcesM),
	    NULL, 0);

        /* make top and bottom shadow pixmaps */

#ifdef WSM
	if (pCD->matteBottomShadowPStr &&
	    (!strcmp(pCD->matteBottomShadowPStr, _NoDither)))
	{
	    pCD->matteBottomShadowPStr = NULL;
	}
#endif /* WSM */
        if (pCD->matteBottomShadowPStr)
        {
	    if ((pCD->matteBottomShadowPStr ==
				    pSD->clientAppearance.bottomShadowPStr) &&
	        (pCD->matteBottomShadowColor ==
				    pSD->clientAppearance.bottomShadowColor) &&
	        (pCD->matteBackground == pSD->clientAppearance.background))
	    {
	        pCD->matteBottomShadowPixmap =
				pSD->clientAppearance.bottomShadowPixmap;
	    }
	    else
	    {
	        pCD->matteBottomShadowPixmap =
			        XmGetPixmap (ScreenOfDisplay (DISPLAY,
				                 pSD->screen),
				             pCD->matteBottomShadowPStr,
				             pCD->matteBottomShadowColor,
				             pCD->matteBackground);

	        if (pCD->matteBottomShadowPixmap == XmUNSPECIFIED_PIXMAP)
	        {
	            pCD->matteBottomShadowPixmap = (Pixmap)NULL;
	        }
	    }
        }
        else
        {
	    pCD->matteBottomShadowPixmap = (Pixmap)NULL;
        }

#ifdef WSM
	if (pCD->matteTopShadowPStr &&
	    (!strcmp(pCD->matteTopShadowPStr, _NoDither)))
	{
	    pCD->matteTopShadowPStr = NULL;
	}
#endif /* WSM */
        if (pCD->matteTopShadowPStr)
        {
	    if ((pCD->matteTopShadowPStr ==
				    pSD->clientAppearance.topShadowPStr) &&
	        (pCD->matteTopShadowColor ==
				    pSD->clientAppearance.topShadowColor) &&
	        (pCD->matteBackground == pSD->clientAppearance.background))
	    {
	        pCD->matteTopShadowPixmap =
					pSD->clientAppearance.topShadowPixmap;
	    }
	    else
	    {
	        pCD->matteTopShadowPixmap =
			        XmGetPixmap (ScreenOfDisplay (DISPLAY,
					         pSD->screen),
				             pCD->matteTopShadowPStr,
				             pCD->matteTopShadowColor,
				             pCD->matteBackground);

	        if (pCD->matteTopShadowPixmap == XmUNSPECIFIED_PIXMAP)
	        {
	            pCD->matteTopShadowPixmap = (Pixmap)NULL;
	        }
	    }
        }
        else
        {
	    pCD->matteTopShadowPixmap = (Pixmap)NULL;
        }


	/* make top and bottom shadow GC's */

	pCD->clientMatteTopShadowGC = GetHighlightGC (pCD->pSD,
				      	  pCD->matteTopShadowColor,
				      	  pCD->matteBackground,
				      	  pCD->matteTopShadowPixmap);

	pCD->clientMatteBottomShadowGC = GetHighlightGC (pCD->pSD,
				      	  pCD->matteBottomShadowColor,
				      	  pCD->matteBackground,
				      	  pCD->matteBottomShadowPixmap);
    }

} /* END OF FUNCTION ProcessClientResources */



/*************************************<->*************************************
 *
 *  SetStdClientResourceValues (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets client resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 *
 *  Input:
 *  -----
 *  pCD = pointer to the client data
 *
 * 
 *  Output:
 *  ------
 *  pCD = (client data filled out with resource values)
 * 
 *************************************<->***********************************/

void 
SetStdClientResourceValues (ClientData *pCD)
{
    pCD->clientDecoration = WM_DECOR_DEFAULT;
    pCD->clientFunctions = WM_FUNC_DEFAULT;
    pCD->focusAutoRaise = True;
    pCD->systemMenu = builtinSystemMenuName;
    pCD->usePPosition = USE_PPOSITION_NONZERO;
    pCD->ignoreWMSaveHints = True;

} /* END OF FUNCTION SetStdClientResourceValues */



/******************************<->*************************************
 *
 *  SetStdScreenResourceValues (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets screen resource data to standard values.  This setting
 *  is done in place of getting the values from the user settings in
 *  the resource database.
 *
 *  Input:
 *  -----
 *  pSD = pointer to the screen data
 *
 * 
 *  Output:
 *  ------
 *  pSD = (screen data filled out with resource values)
 * 
 ******************************<->***********************************/

void 
SetStdScreenResourceValues (WmScreenData *pSD)
{
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    pSD->rootMenu = builtinRootMenuName;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    pSD->buttonBindings = builtinButtonBindingsName;
    pSD->cleanText = True;
    pSD->iconDecoration =
		(ICON_LABEL_PART | ICON_IMAGE_PART | ICON_ACTIVE_LABEL_PART);
    pSD->iconPlacement =
		(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY);
    pSD->keyBindings = builtinKeyBindingsName;
    pSD->limitResize = True;
    pSD->resizeCursors = True;
    pSD->transientDecoration = (WM_DECOR_SYSTEM | WM_DECOR_RESIZEH);
    pSD->transientFunctions =
		(WM_FUNC_ALL & ~(MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE |
				 MWM_FUNC_RESIZE));
    pSD->useIconBox = False;

    pSD->feedbackGeometry = NULL;
    pSD->moveOpaque = False;

} /* END OF FUNCTION SetStdScreenResourceValues */


/*************************************<->*************************************
 *
 *  GetHighlightGC (pSD, fg, bg, pixmap)
 *
 *
 *  Description:
 *  -----------
 *  Get a graphic context for either drawing top- or bottom-shadow 
 *  highlights.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  fg = foreground color
 *  bg = background color
 *  pixmap = pixmap for highlight
 * 
 *  Outputs:
 *  -------
 *  RETRUN = GC with the input parameters incorporated.
 *
 *************************************<->***********************************/

GC GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap)
{
    XGCValues gcv;
    XtGCMask  mask;


    mask = GCForeground | GCBackground | GCLineWidth | GCFillStyle;
    gcv.background = bg;
    gcv.foreground = fg;
    gcv.line_width = 1;

    if (pixmap)
    {
	mask |= GCFillStyle | GCTile;
	gcv.fill_style = FillTiled;
	gcv.tile = pixmap;
    }
    else
    {
	gcv.fill_style = FillSolid;
    }

#ifdef OLD_CODE
    /*
     * NOTE: If additional mask bits are added, modify WmGetGC()
     * in WmGraphics.c to check those values for matches.
     */

    return (WmGetGC (pSD, mask, &gcv));
#endif /* OLD_CODE */

    return (XtGetGC (pSD->screenTopLevelW, mask, &gcv));

} /* END OF FUNCTION GetHighlightGC */



/*************************************<->*************************************
 *
 *  _WmGetDynamicDefault (widget, type, defaultColor, newBackground, value)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to generate a default color of the requested 
 *  type.  Default colors are generated for a 3-D appearance.
 *
 *
 *  Inputs:
 *  ------
 *  widget = this is the widget that is associated with the resource or
 *           that is the reference widget for the wm subpart.
 *
 *  type = this is the type of color resource (e.g., top shadow color).
 *
 *  defaultColor = pointer to default color name/specification.
 *
 *  newBackground = background pixel for generating 3-D colors.
 *
 * 
 *  Outputs:
 *  -------
 *  value = pointer to the XrmValue in which to store the color
 * 
 *************************************<->***********************************/

void 
_WmGetDynamicDefault (Widget widget, unsigned char type, String defaultColor, Pixel newBackground, XrmValue *value)
{
    static Screen *oldScreen = NULL;
    static Screen *newScreen;
    static Colormap oldColormap;
    static Colormap newColormap;
    static Pixel newValue;
    static Pixel background;
    static String oldDefaultColor = DEFAULT_COLOR_NONE;
    static XmColorData colorData;

    /* initialize the return value */

    value->size = sizeof (newValue);
    value->addr = (char *)&newValue;


    /*
     * Process monochrome defaults first.
     */

    newScreen = XtScreen (widget);

    if (Monochrome (newScreen))
    {
#ifdef WSM
    Boolean ok = False;
    /*
     * Check color server sets for this screen.
     */
    if (wmGD.statusColorServer == CSERVE_NORMAL)
    {
	WmScreenData *pSD;
	int i;

	for (i = 0; i < wmGD.numScreens; i++)
	{
	    if (XScreenNumberOfScreen(newScreen) == wmGD.Screens[i].screen)
	    {
		pSD = &wmGD.Screens[i];
		ok = True;
		break;
	    }
	}

	if (ok)
	{
	    ok = False;
	    for (i = 0; i < XmCO_MAX_NUM_COLORS; i++)
	    {
		if (pSD->pPixelData[i].bg == newBackground)
		{
		    switch (type)
		      {
		      case WmFGC: newValue = pSD->pPixelData[i].fg; break;
		      case WmBGC: newValue = pSD->pPixelData[i].bg; break;
		      case WmTSC: newValue = pSD->pPixelData[i].ts; break;
		      case WmBSC: newValue = pSD->pPixelData[i].bs; break;
		      }

		    ok = True;
		}
	    }
	}

    }
    if (!ok)
    {
#endif /* WSM */
        switch (type)
	  {
	  case WmFGC: newValue = BlackPixelOfScreen (newScreen); break;
	  case WmBGC: newValue = WhitePixelOfScreen (newScreen); break;
	  case WmTSC: newValue = WhitePixelOfScreen (newScreen); break;
	  case WmBSC: newValue = BlackPixelOfScreen (newScreen); break;
	  }
#ifdef WSM
    }
#endif /* WSM */
	return;
    }


    /*
     * Check to see if appropriate colors are available from the
     * previous request; if the color is a background color then get
     * default colors.  Generate 3-D colors if necessary.  Maintain
     * new colors in static variables for later reuse.
     */

    newColormap = widget->core.colormap;

    if ((oldScreen != NULL) && (oldScreen == newScreen) &&
	(oldColormap == newColormap) && (type != WmBGC) &&
	(background == newBackground))
    {
    }
    else if ((oldScreen == newScreen) && (oldColormap == newColormap) &&
	     (type == WmBGC) && (oldDefaultColor == defaultColor))
    {
    }
    else if (type == WmBGC)
    {
	/*
	 * Find or generate a background color and associated 3-D colors.
	 */

	oldDefaultColor = defaultColor;
/*
 * Fix for CR 5152 - Due to the use of Realloc in the color caches,
 *                   a static pointer is not acceptable.  Change it
 *                   to a static structure to maintain the data
 */
	colorData = *_WmGetDefaultColors (newScreen, newColormap, defaultColor);
    }
    else
    {
	/*
	 * Find or generate a color based on the associated background color.
	 */

	oldDefaultColor = DEFAULT_COLOR_NONE;
	background = newBackground;

	XmGetColors(newScreen, newColormap, background,
		    &colorData.foreground.pixel,
		    &colorData.top_shadow.pixel,
		    &colorData.bottom_shadow.pixel,
		    &colorData.select.pixel);
    }

    oldScreen = newScreen;
    oldColormap = newColormap;


    /*
     * Set up the return value.
     */

    colorData.allocated |= type;
    switch (type)
      {
      case XmBACKGROUND:    newValue = colorData.background.pixel;    break;
      case XmFOREGROUND:    newValue = colorData.foreground.pixel;    break;
      case XmTOP_SHADOW:    newValue = colorData.top_shadow.pixel;    break;
      case XmBOTTOM_SHADOW: newValue = colorData.bottom_shadow.pixel; break;
      case XmSELECT:        newValue = colorData.select.pixel;        break;
      default:              newValue = colorData.background.pixel;    break;
      }

} /* END OF FUNCTION _WmGetDynamicDefault */



/*************************************<->*************************************
 *
 *  _WmGetDefaultColors (screen, colormap, defaultColor)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to find or generate default 3-D colors based on a
 *  default background color.
 *
 *
 *  Inputs:
 *  ------
 *  screen = screen for which colors are to be generated.
 *
 *  colormap = colormap that is to be used to make colors.
 *
 *  defaultColor = pointer to a default color name/specification.
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to WmColorData structure containing 3-D colors.
 * 
 *************************************<->***********************************/

XmColorData * _WmGetDefaultColors (screen, colormap, defaultColor)
	Screen *screen;
	Colormap colormap;
	String defaultColor;

{
    static XmColorData *defaultSet[2] = {NULL, NULL};
    static int defaultCount[2] = {0, 0};
    static int defaultSize[2] = {0, 0};
    int setId;
    register XmColorData *set;
    register int count;
    register int size;
    register int i;
    Display *display = DisplayOfScreen (screen);
    XColor colorDef;

/*
 * Fix for CR 5152 - Due to the use of Realloc with _XmGetColors, it is
 *                   necessary to maintain a separate cache of color
 *                   data.  The Realloc may cause the data to be moved,
 *                   and the cache would contain pointers into the heap.
 */

    /*
     * Look through the cache to see if the defaults are already in the
     * cache.  There is a list of cached defaults for each default color.
     */

    if (defaultColor == _defaultColor2)
    {
	setId = 1;
    }
    else
    {
	setId = 0;
    }

    set = defaultSet[setId];
    count = defaultCount[setId];
    size = defaultSize[setId];
    
    for (i = 0; i < count; i++)
    {
	if (((set + i)->screen == screen) && ((set + i)->color_map == colormap))
	{
	    return (set + i);
	}
    }

    /* 
     * No match in the cache, make a new entry and generate the colors.
     */

    if (count == size)
    {
	size = (defaultSize[setId] += 10);
	set = defaultSet[setId] =
		(XmColorData *)WmRealloc ((char *) defaultSet[setId],
			            sizeof (XmColorData) * size);
    }

    /*
     * Make the default background color for the resource set.
     */

    if(!XParseColor (display, colormap, defaultColor, &colorDef))
    {
        if(!(strcmp(defaultColor, _defaultColor1)))
        {
            XParseColor (display, colormap, _defaultColor1HEX, &colorDef);
        }
        else
        {
            XParseColor (display, colormap, _defaultColor2HEX, &colorDef);
        }
    }

    XAllocColor (display, colormap, &colorDef);


    /*
     * Generate the 3-D colors and save them in the defaults cache.
     */

    XmGetColors(screen, colormap, colorDef.pixel,
		&set[count].foreground.pixel,
		&set[count].top_shadow.pixel,
		&set[count].bottom_shadow.pixel,
		&set[count].select.pixel);
    
    set[count].background.pixel = colorDef.pixel;
    
    set[count].screen    = screen;
    set[count].color_map = colormap;
    set[count].allocated = True;
    
    XQueryColor(DISPLAY, colormap, &(set[count].background));
    XQueryColor(DISPLAY, colormap, &(set[count].foreground));
    XQueryColor(DISPLAY, colormap, &(set[count].top_shadow));
    XQueryColor(DISPLAY, colormap, &(set[count].bottom_shadow));
    XQueryColor(DISPLAY, colormap, &(set[count].select));

    (defaultCount[setId])++;

    return (set + count);


} /* END OF FUNCTION _WmGetDefaultColors */



/*************************************<->*************************************
 *
 *  WmRealloc (ptr, size)
 *
 *
 *  Description:
 *  -----------
 *  This function is used reallocate a block of storage that has been
 *  malloc'ed.
 *
 *
 *  Inputs:
 *  ------
 *  ptr = pointer to storage that is to be realloc'ed; if NULL malloc an
 *        initial block of storage.
 *
 *  size = size of new storage
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to realloc'ed block of storage
 * 
 *************************************<->***********************************/

char * WmRealloc (ptr, size)
	char *ptr;
	unsigned size;

{
    if (ptr)
    {
	ptr = (char *)XtRealloc (ptr, size);
    }
    else
    {
	ptr = (char *)XtMalloc (size);
    }

    if (ptr == NULL)
    {
	Warning (((char *)GETMESSAGE(62, 37, "Insufficient memory for window manager data")));
    }

    return (ptr);

} /* END OF FUNCTION WmRealloc */



/*************************************<->*************************************
 *
 *  WmMalloc (ptr, size)
 *
 *
 *  Description:
 *  -----------
 *  This function is used malloc a block of storage.  If a previous block
 *  of storage is being replace the old block is free'd.
 *
 *
 *  Inputs:
 *  ------
 *  ptr = pointer to storage that is to be replaced (free'd).
 *
 *  size = size of new storage
 * 
 *  Outputs:
 *  -------
 *  RETURN = pointer to malloc'ed block of storage
 * 
 *************************************<->***********************************/

char * WmMalloc (ptr, size)
	char *ptr;
	unsigned size;

{
    if (ptr)
    {
	XtFree (ptr);
    }

    ptr = (char *)XtMalloc (size);

    if (ptr == NULL)
    {
	Warning (((char *)GETMESSAGE(62, 38, "Insufficient memory for window manager data")));
    }

    return (ptr);

} /* END OF FUNCTION WmMalloc */



/*************************************<->*************************************
 *
 *  SetupDefaultResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to setup default (builtin) resources for the
 *  key bindings.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  wmGD = (defaultKeyBindingsString, ...)
 *
 *  builtinKeyBindingsName = name of default key bindings set
 *
 * 
 *  Outputs:
 *  -------
 *   None 
 *
 *************************************<->***********************************/

void
SetupDefaultResources (pSD)

WmScreenData *pSD;

{
    KeySpec *nextKeySpec;
    String keyBindings;
    MenuSpec *menuSpec;


/*
 * If (using DefaultBindings mechanism and bindings are not found in .mwmrc)
 *	then use the builtin bindings.
 */
    if (!pSD->keySpecs && !wmGD.useStandardBehavior)
    {
	/*
	 * Print warning if user is NOT using "DefaultKeyBindings".
	 */
	if (strcmp (pSD->keyBindings, defaultKeyBindingsName))
	{
	   MWarning (((char *)GETMESSAGE(62, 67, "Key bindings %s not found, using builtin key bindings\n")),
	   pSD->keyBindings);
	}
	pSD->keyBindings = builtinKeyBindingsName;
    }

    if (!pSD->buttonSpecs && !wmGD.useStandardBehavior)
    {
	/*
	 * Print warning if user is NOT using "DefaultButtonBindings".
	 */
	if (strcmp (pSD->buttonBindings, defaultButtonBindingsName))
	{
	   MWarning (((char *)GETMESSAGE(62, 68, "Button bindings %s not found, using builtin button bindings\n")),
		     pSD->buttonBindings);
	}
	pSD->buttonBindings = builtinButtonBindingsName;
    }

    if (pSD->keyBindings == builtinKeyBindingsName)
    {
	/*
	 * Default key specifications are to be used and no default
	 * set has been provided by the user.  Make the built-in default
	 * set.
	 */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
        /*
	 * Before parsing the string, substitute the real name for
	 * the default rootmenu using the resource rootMenu
	 * for the %s in the string.
	 */

        char *buffer;

	buffer = (char *) XtMalloc(strlen(builtinKeyBindings) +
				   strlen(pSD->rootMenu) + 1);
	sprintf(buffer, builtinKeyBindings, pSD->rootMenu);

	ParseKeyStr (pSD, (unsigned char *)buffer);
#else
	ParseKeyStr (pSD, (unsigned char *)builtinKeyBindings);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    }
    else
    {
	/*
	 * Add the switch behavior key binding to the front of the list
	 * of user specified key bindings that have been parsed.
	 */

	nextKeySpec = pSD->keySpecs;
	keyBindings = pSD->keyBindings;
	pSD->keyBindings = behaviorKeyBindingName;
	pSD->keySpecs = NULL;

	ParseKeyStr (pSD, (unsigned char *)behaviorKeyBindings);

	if (pSD->keySpecs)
	{
	    /* Skip past the TWO key definitions (1.2 & 1.1.4) */
	    pSD->keySpecs->nextKeySpec->nextKeySpec = nextKeySpec;
	}
	else
	{
	    pSD->keySpecs = nextKeySpec;
	}
	pSD->keyBindings = keyBindings;
    }

    if (pSD->buttonBindings == builtinButtonBindingsName)
    {
	/*
	 * Default button specifications are to be used and no default
	 * set has been provided by the user.  Make the built-in default
	 * set.
	 */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
        /*
	 * Before parsing the string, substitute the real name for
	 * the default rootmenu using the resource rootMenu
	 * for the %s in the string.
	 */

        char *buffer;

	buffer = (char *) XtMalloc(strlen(builtinButtonBindings) +
				   strlen(pSD->rootMenu) + 1);
	sprintf(buffer, builtinButtonBindings, pSD->rootMenu);

	ParseButtonStr (pSD, (unsigned char *)buffer);
#else
	ParseButtonStr (pSD, (unsigned char *)builtinButtonBindings);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    }

#ifdef NO_MESSAGE_CATALOG
    /*
     * Set defaultSystemMenuUseBuiltin to FALSE if DefaultWindowMenu spec
     * is found.
     */

    menuSpec = pSD->menuSpecs;
    while ( menuSpec )
    {
	if (!strcmp(menuSpec->name, defaultSystemMenuName))
	{
		pSD->defaultSystemMenuUseBuiltin = FALSE;
		break;
	}
	menuSpec = menuSpec->nextMenuSpec;
    }
#endif

} /* END OF FUNCTION SetupDefaultResources */



/*************************************<->*************************************
 *
 *  SimilarAppearanceData (pAD1, pAD2)
 *
 *
 *  Description:
 *  -----------
 *  This function returns True if the two passed sets of AppearanceData
 *  are similar. This is designed to compare appearance data before
 *  creation of the GCs.
 *
 *
 *  Inputs:
 *  ------
 *  pAD1	pointer to AppearanceData 1
 *  pAD2	pointer to AppearanceData 2
 *
 * 
 *  Outputs:
 *  -------
 *  Function returns True if similar, False otherwise.
 * 
 *  Comments:
 *  ---------
 *  This function is only used to compare the client
 *  and client*title appearance data.
 *************************************<->***********************************/

Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2)
{
    Boolean rval;

#ifdef notdef
    if ((pAD1->fontList == pAD2->fontList) &&
	(pAD1->background == pAD2->background) &&
	(pAD1->foreground == pAD2->foreground) &&
	(pAD1->backgroundPStr == pAD2->backgroundPStr) &&
	(pAD1->backgroundPixmap == pAD2->backgroundPixmap) &&
	(pAD1->bottomShadowColor == pAD2->bottomShadowColor) &&
	(pAD1->bottomShadowPStr == pAD2->bottomShadowPStr) &&
	(pAD1->bottomShadowPixmap == pAD2->bottomShadowPixmap) &&
	(pAD1->topShadowColor == pAD2->topShadowColor) &&
	(pAD1->topShadowPStr == pAD2->topShadowPStr) &&
	(pAD1->topShadowPixmap == pAD2->topShadowPixmap) &&
	(pAD1->activeBackground == pAD2->activeBackground) &&
	(pAD1->activeForeground == pAD2->activeForeground) &&
	(pAD1->activeBackgroundPStr == pAD2->activeBackgroundPStr) &&
	(pAD1->activeBackgroundPixmap == pAD2->activeBackgroundPixmap) &&
	(pAD1->activeBottomShadowColor == pAD2->activeBottomShadowColor) &&
	(pAD1->activeBottomShadowPStr == pAD2->activeBottomShadowPStr) &&
	(pAD1->activeBottomShadowPixmap == pAD2->activeBottomShadowPixmap) &&
	(pAD1->activeTopShadowColor == pAD2->activeTopShadowColor) &&
	(pAD1->activeTopShadowPStr == pAD2->activeTopShadowPStr) &&
	(pAD1->activeTopShadowPixmap == pAD2->activeTopShadowPixmap) )
#else
    /*
     * !!! Should find out why all the Pixmap resources are unset !!!
     */

    if ((pAD1->fontList == pAD2->fontList) &&
	(pAD1->background == pAD2->background) &&
	(pAD1->foreground == pAD2->foreground) &&
	(pAD1->backgroundPStr == pAD2->backgroundPStr) &&
	(pAD1->bottomShadowColor == pAD2->bottomShadowColor) &&
	(pAD1->bottomShadowPStr == pAD2->bottomShadowPStr) &&
	(pAD1->topShadowColor == pAD2->topShadowColor) &&
	(pAD1->topShadowPStr == pAD2->topShadowPStr) &&
	(pAD1->activeBackground == pAD2->activeBackground) &&
	(pAD1->activeForeground == pAD2->activeForeground) &&
	(pAD1->activeBackgroundPStr == pAD2->activeBackgroundPStr) &&
	(pAD1->activeBottomShadowColor == pAD2->activeBottomShadowColor) &&
	(pAD1->activeBottomShadowPStr == pAD2->activeBottomShadowPStr) &&
	(pAD1->activeTopShadowColor == pAD2->activeTopShadowColor) &&
	(pAD1->activeTopShadowPStr == pAD2->activeTopShadowPStr) )
#endif
    {
	rval = True;
    }
    else 
    {
	rval = False;
    }

    return (rval);

} /* END OF FUNCTION SimilarAppearanceData */

#ifdef WSM

/*************************************<->*************************************
 *
 *  Monochrome (screen)
 *
 *
 *  Description:
 *  -----------
 *  This function returns True if the screen passed it to be treated
 *  as monochrome for the purpose of assigning default resources.
 *
 *
 *  Inputs:
 *  ------
 *  screen	pointer to Screen
 *
 * 
 *  Outputs:
 *  -------
 *  Function returns True if monochrome (or Static Gray), False otherwise.
 * 
 *************************************<->***********************************/

Boolean 
Monochrome (Screen *screen)
{
    WmScreenData *pSD;

    int scr;
    
    if (wmGD.statusColorServer == CSERVE_NORMAL)
    {
	for (scr = 0; scr < wmGD.numScreens; scr++)
	{
	    pSD = &(wmGD.Screens[scr]);

	    if (pSD->managed)
	    {
		if (XScreenOfDisplay (DISPLAY, pSD->screen) == screen)
		{
		    if (pSD->colorUse == XmCO_BLACK_WHITE)
		    {
			return(True);
		    }
		    else
		    {
			return(False);
		    }
		}
	    }
	}
    }
    /*
     *   If we don't know the answer to our question by now,
     *   fall back to the old mwm way of determining monochromicity.
     *
     */

    return ((DefaultDepthOfScreen(screen) == 1));
} /* END OF FUNCTION Monochrome */
#endif /* WSM */
#ifdef WSM
/****************************   eof    ***************************/
#endif /* WSM */
