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
 * Motif Release 1.2.4
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: WmInitWs.c /main/18 1999/09/20 15:18:22 mgreess $"
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
#ifdef WSM
#include "WmHelp.h"
#endif /* WSM */
#include "WmICCC.h"
#ifdef PANELIST
#define DTWM_NEED_FNTPL
#include "WmIBitmap.h"
#endif /* PANELIST */
#ifndef NO_OL_COMPAT
#include "WmOL.h"
#endif /* NO_OL_COMPAT */
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <X11/Core.h>
#include <X11/keysym.h>
#include <Xm/AtomMgr.h>
#ifndef NO_HP_KEY_REMAP
#include <Xm/VirtKeysP.h>

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# include <Xm/DrawingA.h>
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

typedef struct
  {
    String default_name ;
    String new_name ;
  } str_xref_rec, *str_xref ;
#endif /* NO_HP_KEY_REMAP */
#ifdef WSM
#include <Dt/GetDispRes.h>
#include <Dt/SessionP.h>
#include <Dt/DtP.h>
#include <Dt/Message.h>
#include <Dt/WsmM.h>
#endif /* WSM */

/* Busy is also defined in the BMS  -> bms.h. This conflicts with
 * /usr/include/X11/Xasync.h on ibm.
 */
#ifdef _AIX
#ifdef Busy
#undef Busy
#endif
#endif
#include <X11/Xlibint.h>

/*
 * include extern functions
 */
#ifdef WSM
#include "WmBackdrop.h"
#endif /* WSM */
#include "WmCDInfo.h"
#include "WmColormap.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#ifdef WSM
#include "WmIPC.h"
#endif /* WSM */
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmMenu.h"
#ifdef WSM
#include "WmPresence.h"
#endif /* WSM */
#include "WmProperty.h"
#include "WmResCvt.h"
#include "WmResource.h"
#include "WmSignal.h"
#include "WmProtocol.h"
#include "WmCDecor.h"
#include "stdio.h"
#include "WmResParse.h"
#ifdef WSM
#include <stdlib.h>
#endif /* WSM */
#include "WmXSMP.h"

/*
 * Function Declarations:
 */

#include "WmInitWs.h"

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# include "WmWsmLib/wsm_proto.h"
# include "WmWsmLib/utm_send.h"
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#ifdef WSM
static void InsureDefaultBackdropDir(char **ppchBackdropDirs);
#endif /* WSM */
void InitWmDisplayEnv (void);
#ifndef NO_MESSAGE_CATALOG
void InitNlsStrings (void);
#endif
#ifndef NO_HP_KEY_REMAP
Boolean VirtKeys4DIN(Display *dpy); 
#endif /* NO_HP_KEY_REMAP */

#ifdef WSM
/* limited to 3 chars max */
#define UNSPECIFIED_SCREEN_NAME		"fbk"
char        **dpy2Argv;    /* copy  for second display */
int           dpy2Argc;
#ifndef PANELIST
WmScreenData *dtSD;       /* for the "DT screen" of the display */
#endif /* PANELIST */
#endif  /* WSM */
/*
 * Global Variables:
 */
extern int firstTime;
#ifndef NO_MESSAGE_CATALOG
extern char * pWarningStringFile;
extern char * pWarningStringLine;
#endif

/*
 * InitMouseBinding
 *
 * Special case for a two button mouse; move the BMENU binding 
 * from Button3 to Button2.  Fails for one-button mice.
 */
static void
InitMouseBinding(void)
{
    wmGD.numMouseButtons = XGetPointerMapping(DISPLAY, (unsigned char *)0, 0);
    
    if (wmGD.numMouseButtons < 3) {
        wmGD.bMenuButton = Button2;
    } else {
        wmGD.bMenuButton = Button3;
    }
}

/******************************<->*************************************
 *
 *  BuildLockMaskSequence ()
 *
 *  Set up the sequence of modifier masks to use to when grabbing 
 *  key- and button-bindings. This sequence of masks is NULL
 *  terminated.
 *
 *  Input:
 *  	wmGD.lockingModMask
 *
 *  Output:
 *	wmGD.pLockMaskSequence
 *
 *
 *************************************<->***********************************/
static void
BuildLockMaskSequence(void)
{
    int i, j, k;
    unsigned int mask;
    unsigned int thisbit;
    Boolean bit_on;
    int num_masks;
    int num_bits;
    int bit;
    int run;

    /*
     * Count the bits to determine the number of elements in 
     * the mask sequence.  The actual number of masks is
     * 2^<bitcount> - 1. We're not interested in the case
     * where there none of the mask bits are set.
     */
    mask = wmGD.lockingModMask;

    num_bits=0;
    while (mask)
    {
	if (mask & 0x1)
	{
	    num_bits++;
	}
	mask = mask >> 1;
    }
    num_masks = (0x1 << num_bits) - 1;

    /*
     * Allocate the space for the mask sequence + terminator.
     */
    wmGD.pLockMaskSequence = (unsigned int *) 
			XtCalloc (num_masks+1, sizeof (unsigned int));

    /*
     * Fill in the mask sequence
     */
    mask = wmGD.lockingModMask;
    thisbit = 0x1;
    bit = 0;
    while (mask && thisbit)
    {
	/* find next bit */
	while (!(thisbit & mask))
	{
	    thisbit = thisbit << 1;
	}

	/* clear it from mask */
	mask &= ~thisbit;

	bit++;

	/* 
	 * Set it in the appropriate slots in the
	 * mask sequence. The start of the loop is
	 * funny because we skip the case of all the
	 * bits cleared.
	 */
	run = (0x1 << bit-1);	/* number of consecutive masks to set
				   bits in */
	bit_on = False;		/* are we setting bits or not? */

	for (j=0, k=run-1; j<num_masks; j++, k--)
	{
	    if (k < 1)
	    {
		if (bit_on)
		    bit_on = False;
		else
		    bit_on = True;

		k = run; 
	    }

	    if (bit_on) wmGD.pLockMaskSequence[j] |= thisbit;
	}
    }
}


/******************************<->*************************************
 *
 *  SetupLockingModifierMask ()
 *
 *  Set up the mask used to ignore locking modifier keys (e.g. Shift Lock)
 *  when processing key- and button-bindings.
 *
 *  We want to try to ignore the set of locking modifers 
 *  such as Shift Lock, Num Lock, Kana Lock, etc. This involves
 *  some amount of guessing since these things can be mapped
 *  onto any of the Mod1-Mod5 modifiers. The approach taken is to 
 *  identify the mapping of the locking modifier keysyms to
 *  Mod1-Mod5 and build the set of masks needed to ignore them.
 *
 *************************************<->***********************************/

/*
 * This is the set of locking modifiers keysyms that might be
 * bound to Mod1-Mod5. (Caps Lock is handled independently of
 * this set.)
 */
static KeySym pksLockingMods[] = {
    XK_Scroll_Lock,
    XK_Kana_Lock,
    XK_Num_Lock,
    XK_Mode_switch
};

#define NUM_LOCKING_MODS (sizeof(pksLockingMods)/sizeof(KeySym))

static void
SetupLockingModifierMask(void)
{
    int i, j, start_index;
    XModifierKeymap *modifier_map = NULL;
    static Modifiers mod_masks[] = { None, Mod1Mask, Mod2Mask,
                                        Mod3Mask, Mod4Mask, Mod5Mask };
    Display *dpy = wmGD.display;
    int pkcLockingMods[NUM_LOCKING_MODS];

    int kcq, kc;

    for (i=0; i<NUM_LOCKING_MODS; i++)
    {
	pkcLockingMods[i] = XKeysymToKeycode(dpy, pksLockingMods[i]);
    }

    /* 
     * Start out with Caps lock and add others we discover.
     */
    wmGD.lockingModMask = LockMask;

    modifier_map = XGetModifierMapping(dpy);

    /* just check Mod1 through Mod5 */
    start_index = modifier_map->max_keypermod * Mod1MapIndex;

    for (i = start_index; i < modifier_map->max_keypermod * 8; i++) {
        int this_mod = ((i - start_index) / modifier_map->max_keypermod) + 1;
 
        kc = modifier_map->modifiermap[i];
        if (kc)
	{
	    for (j=0; j<NUM_LOCKING_MODS; j++)
	    {
		if (pkcLockingMods[j] == kc)
		{
		    wmGD.lockingModMask |= mod_masks[this_mod];
		    break;
		}
	    }
	}
    }

    BuildLockMaskSequence();

    if(modifier_map != NULL)
	XFreeModifiermap(modifier_map);

}

/******************************<->*************************************
 *
 *  MappingEventHandler (Widget, XtPointer, XEvent *, Boolean *)
 *
 *  Catch and handle changes to the mapping of the modifier keys.
 *
 *************************************<->***********************************/

static void
MappingEventHandler(
        Widget w,
        XtPointer client_data,
        XEvent *event,
        Boolean *cont_to_dispatch)
{
        if(event->xany.type != MappingNotify ||
                event->xmapping.request == MappingPointer)
                return;
 
        if(event->xmapping.request == MappingModifier)
                SetupLockingModifierMask();
}


/******************************<->*************************************
 *
 *  InitWmGlobal (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This function initializes the workspace manager.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (initialize the global data structure)
 * 
 *************************************<->***********************************/

void InitWmGlobal (int argc, char *argv [], char *environ [])
{
    XSetWindowAttributes sAttributes;
    int scr;
    int managed = 0;
    char pch[80];
    Boolean activeSet = False;
    Boolean processedGlobalResources = False;
    WmScreenData *pSD;
    Arg args[20];
    int argnum;
    char *res_class;
    int savedArgc;

    wmGD.errorFlag = False;
#ifdef WSM
#ifndef PANELIST
    dtSD = NULL; 
#else /* PANELIST  */
    wmGD.dtSD = NULL;
    wmGD.iSlideUpsInProgress = 0;
#endif /*PANELIST  */
#endif  /* WSM */

    SetupWmSignalHandlers (0); /* dummy paramater */


    /*
     * Do (pre-toolkit) initialization:
     */

    wmGD.windowContextType = XUniqueContext ();
    wmGD.screenContextType = XUniqueContext ();
#ifndef	IBM_169380
    wmGD.cmapWindowContextType = XUniqueContext ();
#endif
#ifdef WSM
    wmGD.mwmWindowContextType = XUniqueContext ();
#endif /* WSM */

    /* copy argv (the XtInititalize changes the original) for use in restart */
    savedArgc = argc;
    CopyArgv (argc, argv);

    wmGD.environ = environ;

#ifdef WSM
    wmGD.pWmPB = _DtWmParseNewBuf();
#endif /* WSM */



    /* set our name */
    if ((wmGD.mwmName = (char*)strrchr (wmGD.argv[0], '/')) != NULL)
    {
        wmGD.mwmName++;
    }
    else
    {
        wmGD.mwmName = wmGD.argv[0];
    }
#ifdef WSM
    if (MwmBehavior)
    {
	res_class = WM_RESOURCE_CLASS;
    }
    else 
    {
	res_class = DT_WM_RESOURCE_CLASS;
    }
    wmGD.statusColorServer = CSERVE_NOT_AVAILABLE;

#else /* WSM */
    res_class = WM_RESOURCE_CLASS;
#endif /* WSM */

    wmGD.display = (Display *)NULL;
    wmGD.topLevelW = (Widget)NULL;

    /*
     * Do X Tookit initialization:
     */   

    XtToolkitInitialize();

    wmGD.mwmAppContext = XtCreateApplicationContext();
    AddWmResourceConverters ();
    wmGD.display = XtOpenDisplay (wmGD.mwmAppContext,
				  NULL,
				  wmGD.mwmName,
				  res_class,
				  NULL,
				  0,
				  &argc, /* R5 changed from Cardinal to int*/
				  argv);
    
    if (!wmGD.display)
    {
	Warning(((char *)GETMESSAGE(40, 1, "Could not open display.")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

#if defined(sun) && defined(ALLPLANES)
    {
	int dummy;

	wmGD.allplanes = XAllPlanesQueryExtension(wmGD.display, 
				&dummy, &dummy);	
    }
#endif /* defined(sun) && defined(ALLPLANES) */

    /*
     * Setup error handling:
     */

    WmInitErrorHandler(wmGD.display);

    /*
     * Initialize cursor size info and 
     * display the startup cursor.
     */
    
    InitCursorInfo ();
#ifdef WSM
    InitWmDisplayEnv ();
#endif
    ShowWaitState (TRUE);

    /*
     * Initialize support for BMenu virtual mouse binding
     */
    InitMouseBinding();
    /*
     * Set up the _MOTIF_BINDINGS property on the root window
     * of screen 0.  Must do this before we create shells.
     */
    
    ProcessMotifBindings ();
#ifndef NO_HP_KEY_REMAP

    /* VirtKeys4DIN deals with a shortcoming in the OSF/Motif
     *   mechanism for selecting a virtual key binding table.
     * When a client connects to a display, code inside of libXm
     *   gets the vendor identifier from the server and uses this
     *   identifier to select a default virtual key binding table
     *   from an internal list of possible tables (provided to
     *   OSF by vendors).  A virtual key binding table maps OSF
     *   virtual keysyms to an "appropriate" set of X keysyms for
     *   a particular server vendor.  The problem with this
     *   mechanism is that it only allows for a _single_ default
     *   virtual key binding table per server vendor.  If a
     *   hardware vendor ships more than one distinct keyboard,
     *   then the single virtual key binding table selected for
     *   that server might not be appropriate for all keyboards.
     * The HP migration from the "ITF" keyboard to the "PC-style"
     *   keyboard causes this problem to be exposed for Motif
     *   clients.  The default HP virtual key binding table maps
     *   osfInsert and osfDelete to hpInsertChar and hpDeleteChar,
     *   respectively.  But since hpInsertChar and hpDeleteChar
     *   are absent from the PC-style keyboard, HP servers with
     *   this keyboard do not generate key events that map to the
     *   important osfInsert and osfDelete virtual keys.
     * The Motif 1.2 version of libXm installs (one or two)
     *   properties on the root window, these properties exporting
     *   the virtual key binding table to be used by all (subsequently
     *   connected) Motif clients.  The VirtKeys4DIN routine attempts
     *   to ensure that the virtual key binding table exported by
     *   those properties does not include dependencies on keysyms
     *   that are not available in the server's current modmap.
     *   The routine accomplishes this by searching the keyboard
     *   mapping of the display for the absence of known problematic
     *   keysyms.  For those keysyms that are missing from the
     *   keyboard map, the corresponding dependencies in the
     *   virtual key binding table are altered to use pre-determined
     *   substitutes (which are generic X keysyms that are present
     *   on the PC-style keyboard mapping).
     * The side-effects of this routine are that if there are no
     *   key binding properties on the root window when this routine
     *   is called, there will be a property installed (this occurs
     *   with all Motif 1.2 clients anyway).  Since the virtual key
     *   binding table is only altered if it contains a keysym that
     *   is missing from the server's keyboard mapping, there is
     *   little chance of deleterious effects.
     */
    VirtKeys4DIN(DISPLAY);
#endif /* NO_HP_KEY_REMAP */
    
    argnum = 0;
    XtSetArg (args[argnum], XtNgeometry, NULL);	argnum++;
    XtSetArg (args[argnum], XtNx, 10000);	argnum++;
    XtSetArg (args[argnum], XtNy, 0);		argnum++;
    XtSetArg (args[argnum], XtNwidth, 10);	argnum++;
    XtSetArg (args[argnum], XtNheight, 10);	argnum++;
    XtSetArg (args[argnum], XtNmappedWhenManaged, False);	argnum++;
    XtSetArg (args[argnum], XtNjoinSession, True);		argnum++;
#ifdef WSM
    XtSetArg (args[argnum], XtNrestartStyle, SmRestartNever);	argnum++;
#else
    XtSetArg (args[argnum], XtNrestartStyle, SmRestartIfRunning); argnum++;
#endif
    XtSetArg (args[argnum], XtNargc, savedArgc); argnum++;
    XtSetArg (args[argnum], XtNargv, wmGD.argv); argnum++;

    /* create topmost shell (application shell) */
    wmGD.topLevelW = XtAppCreateShell (NULL, 
			      res_class,
			      sessionShellWidgetClass,
			      DISPLAY,
			      args,
			      argnum);

#ifdef __osf__
    _XmColorObjCreate ( wmGD.topLevelW, NULL, NULL);
    _XmColorObjCreate ( wmGD.topLevelW, NULL, NULL);
#endif

    XtAddEventHandler(wmGD.topLevelW, NoEventMask, True,
			MappingEventHandler, NULL);

    /* Add callbacks used for communication with Session Manager. */
    AddSMCallbacks ();

    /* allocate namespace for screens */
    InitScreenNames();
    
    /* 
     * Determine the screen management policy (all or none)
     * Process command line arguments that we handle 
     * This could change the number of screens we manage 
     */
    ProcessGlobalScreenResources ();
    ProcessCommandLine (argc, argv);

#ifdef WSM
    /*
     * Make sure backdrops are in our icon search path. 
     * This call MUST occur before ANY icons are looked up either
     * explicitly or through resource processing!!!
     * Uses variables set by ProcessGlobalScreenResources and
     * ProcessCommandLine.
     */
    {
	int sNum;
	Boolean         useMaskRtn;
	Boolean         useMultiColorIcons;
	Boolean         useIconFileCacheRtn;
	String		sBdPath;

	sNum = (wmGD.numScreens == 1) ? DefaultScreen(DISPLAY) : 0;

	XmeGetIconControlInfo(ScreenOfDisplay(DISPLAY, sNum), &useMaskRtn,
		    &useMultiColorIcons, &useIconFileCacheRtn);

        sBdPath = wmGD.backdropDirs;
	InsureDefaultBackdropDir ((char **) &sBdPath);

	_DtWsmSetBackdropSearchPath(XScreenOfDisplay(DISPLAY, sNum), 
			sBdPath, useMultiColorIcons);

        XtFree(sBdPath);
    }
#endif /* WSM */

    /*
     * Allocate data and initialize for screens we manage:
     */

    if (!(wmGD.Screens = (WmScreenData *) 
	    XtCalloc (wmGD.numScreens, sizeof(WmScreenData))))
    {
	ShowWaitState (FALSE);
	Warning (((char *)GETMESSAGE(40, 2, "Insufficient memory for Screen data")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }
    else 
    {

	sAttributes.event_mask = SubstructureRedirectMask;

	for (scr=0; scr<wmGD.numScreens; scr++) 
	{
	    int sNum;
	    
	    /* 
	     * Gain control of the root windows of each screen:
	     */

	    sNum = (wmGD.numScreens == 1) ? DefaultScreen(DISPLAY) : scr;
	    wmGD.errorFlag = False;

	    XChangeWindowAttributes (DISPLAY, RootWindow (DISPLAY, sNum), 
		CWEventMask, &sAttributes);
	    /*
	     * Do XSync to force server action and catch errors
	     * immediately.
	     */
	    XSync (DISPLAY, False /* do not discard events */);

	    if ((wmGD.errorFlag) &&
		(RootWindow (DISPLAY, sNum) == (Window) wmGD.errorResource) &&
		(wmGD.errorRequestCode == X_ChangeWindowAttributes))
	    {
		sprintf(pch, 
		    ((char *)GETMESSAGE(40, 3, "Another window manager is running on screen %d")), sNum);
		Warning ((char *) &pch[0]);
		wmGD.Screens[scr].managed = False;
	    }
	    else 
	    {
		if (!processedGlobalResources)
		{
#ifdef WSM
		    enum { 
		      XA_DT_SESSION_HINTS, XA_DT_SM_WM_PROTOCOL,
		      XA_DT_SM_START_ACK_WINDOWS, XA_DT_SM_STOP_ACK_WINDOWS,
		      XA_DT_WM_WINDOW_ACK, XA_DT_WM_EXIT_SESSION,
		      XA_DT_WM_LOCK_DISPLAY, XA_DT_WM_READY, NUM_ATOMS };
		    static char *atom_names[] = {
		      _XA_DT_SESSION_HINTS, _XA_DT_SM_WM_PROTOCOL,
		      _XA_DT_SM_START_ACK_WINDOWS, _XA_DT_SM_STOP_ACK_WINDOWS,
		      _XA_DT_WM_WINDOW_ACK, _XA_DT_WM_EXIT_SESSION,
		      _XA_DT_WM_LOCK_DISPLAY, _XA_DT_WM_READY };

		    Atom atoms[XtNumber(atom_names)];
		    XInternAtoms(DISPLAY, atom_names, XtNumber(atom_names), 
				 False, atoms);

		    wmGD.xa_DT_SESSION_HINTS = atoms[XA_DT_SESSION_HINTS];
		    wmGD.xa_DT_SM_WM_PROTOCOL = atoms[XA_DT_SM_WM_PROTOCOL];
		    wmGD.xa_DT_SM_START_ACK_WINDOWS =
		      atoms[XA_DT_SM_START_ACK_WINDOWS]; 
		    wmGD.xa_DT_SM_STOP_ACK_WINDOWS =
		      atoms[XA_DT_SM_STOP_ACK_WINDOWS]; 
		    wmGD.xa_DT_WM_WINDOW_ACK = atoms[XA_DT_WM_WINDOW_ACK];
		    wmGD.xa_DT_WM_EXIT_SESSION = atoms[XA_DT_WM_EXIT_SESSION];
                    wmGD.xa_DT_WM_LOCK_DISPLAY = atoms[XA_DT_WM_LOCK_DISPLAY];
                    wmGD.xa_DT_WM_READY = atoms[XA_DT_WM_READY];
#endif /* WSM */
#ifndef NO_OL_COMPAT
		    InitOLCompat();
#endif /* NO_OL_COMPAT */
#ifndef NO_SHAPE
		    wmGD.hasShape = XShapeQueryExtension (DISPLAY,
							  &wmGD.shapeEventBase,
							  &wmGD.shapeErrorBase);
#endif /*  NO_SHAPE  */

                    wmGD.replayEnterEvent = False;
		    wmGD.menuActive = NULL;
		    wmGD.menuUnpostKeySpec = NULL;
		    wmGD.F_NextKeySpec = NULL;
		    wmGD.F_PrevKeySpec = NULL;
		    wmGD.passKeysActive = False;
		    wmGD.passKeysKeySpec = NULL;
		    wmGD.checkHotspot = False;
		    wmGD.configAction = NO_ACTION;
		    wmGD.configPart = FRAME_NONE;
		    wmGD.configSet = False;
		    wmGD.preMove = False;
		    wmGD.gadgetClient = NULL;
		    wmGD.wmTimers = NULL;
		    wmGD.clientDefaultTitle = 
			XmStringCreateLocalized(DEFAULT_CLIENT_TITLE);
		    wmGD.iconDefaultTitle = 
			XmStringCreateLocalized(DEFAULT_ICON_TITLE);
		    wmGD.attributesWindow = (Window)NULL;
		    wmGD.clickData.pCD = NULL;
		    wmGD.clickData.clickPending = False;
		    wmGD.clickData.doubleClickPending = False;
		    wmGD.systemModalActive = False;
		    wmGD.activeIconTextDisplayed = False;
		    wmGD.movingIcon = False;
		    wmGD.queryScreen = True;
		    wmGD.dataType = GLOBAL_DATA_TYPE;

		    wmGD.pLockMaskSequence = NULL;
		    SetupLockingModifierMask ();
#ifdef WSM
		    wmGD.requestContextWin = (Window) 0L;
		    wmGD.cppCommand = NULL;
		    wmGD.evLastButton.button = 0;
		    wmGD.bReplayedButton = False;
		    wmGD.bSuspendSecondaryRestack = False;
		    /*
		     *  Get a second display connection for 
		     *  internal WM windows.
		     */
		    wmGD.display1 = XtOpenDisplay (wmGD.mwmAppContext,
						   NULL,
						   wmGD.mwmName,
						   res_class,
						   NULL,
						   0,
						   &dpy2Argc,
						   dpy2Argv);
		    if (!wmGD.display1)
		    {
			ShowWaitState (FALSE);
			Warning(((char *)GETMESSAGE(40, 4, "Could not open second display connection.")));
			ExitWM (WM_ERROR_EXIT_VALUE);
		    }

		    _DtGetSmWindow(DISPLAY, 
				  RootWindow(DISPLAY, 0), 
				  &wmGD.dtSmWindow) ;
#ifdef PANACOMM
		    /*
		     * If this is the first screen we've managed,
		     * tell the session manager we're ready 
		     */
		    if (!processedGlobalResources)
		    {
			SendClientMsg( wmGD.dtSmWindow,
				      (long) wmGD.xa_DT_SM_WM_PROTOCOL,
				      (long) wmGD.xa_DT_WM_READY,
				      CurrentTime, NULL, 0);
		    }
#endif /* PANACOMM */

		    /* create topmost shell (application shell) */
		    argnum = 0;
		    XtSetArg (args[argnum], XtNgeometry, NULL);	argnum++;
		    XtSetArg (args[argnum], XtNx, 10000);	argnum++;
		    XtSetArg (args[argnum], XtNy, 0);		argnum++;
		    XtSetArg (args[argnum], XtNwidth, 10);	argnum++;
		    XtSetArg (args[argnum], XtNheight, 10);	argnum++;
		    XtSetArg (args[argnum], 
				XtNmappedWhenManaged, False);	argnum++;

		    wmGD.topLevelW1 = 
			XtAppCreateShell (NULL, 
					  res_class,
					  applicationShellWidgetClass,
					  DISPLAY1,
					  args,
					  argnum);

#endif /* WSM */
		    

		    /* 
		     * if this is the first screen we can manage, 
		     * process global.
		     */
		    
		    processedGlobalResources = True;

		    /*
		     * Get the _MOTIF_WM_INFO property and determine 
		     * the startup / restart state.
		     */
		    
		    ProcessMotifWmInfo (RootWindow (DISPLAY, sNum));
		    
		    /*
		     * Process global window manager resources:
		     */
#ifndef NO_MESSAGE_CATALOG
    		    InitBuiltinSystemMenu();
#endif
		    
		    ProcessWmResources ();

		}
		
		InitWmScreen (&(wmGD.Screens[scr]), sNum);
		wmGD.Screens[scr].managed = True;
		managed++;
#ifdef WSM
		GetDtSessionHints(&(wmGD.Screens[scr]), sNum);
#endif /* WSM */

		if (!activeSet) 
		{
		    activeSet = True;
		    ACTIVE_PSD = &wmGD.Screens[scr];
		}
	    }
	}

	if (managed == 0) 
	{
	    /*
	     * No screens for me to manage, give up.
	     */
	    ShowWaitState (FALSE);
	    Warning (((char *)GETMESSAGE(40, 5, "Unable to manage any screens on display.")));
	    ExitWM (WM_ERROR_EXIT_VALUE);
	}
    }
#ifdef WSM
    /*  
     * Initialize the IPC mechanism
     */
    dtInitialize(argv[0], wmGD.mwmAppContext);
#ifndef NO_MESSAGE_CATALOG
    /*
     * Set up NLS error messages.
     * Must be done after DtInitialize.
     */
    InitNlsStrings ();
#endif

    /*
     *  For multiple connections to the server, turn off
     *  the geometry manager's insistence on synchronous
     *  management.
     */

    argnum = 0;
    XtSetArg (args[argnum], XmNuseAsyncGeometry, True);	argnum++;
    XtSetValues (wmGD.topLevelW, args, argnum);
    XtSetValues (wmGD.topLevelW1, args, argnum);

#endif /* WSM */
    

    /*
     * Prepare to have child processes (e.g., exec'ed commands).
     * The X connection should not be passed on to child processes
     * (it should be automatically closed when a fork is done).
     */

    if (fcntl (ConnectionNumber (DISPLAY), F_SETFD, 1) == -1)
    {
	ShowWaitState (FALSE);
	Warning (((char *)GETMESSAGE(40, 6, "Cannot configure X connection")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

#ifdef WSM

    {
      enum { XA_DT_WORKSPACE_HINTS, XA_DT_WORKSPACE_PRESENCE,
	     XA_DT_WORKSPACE_INFO, XA_WmNall,
	     XA_DT_WORKSPACE_EMBEDDED_CLIENTS, XA_DT_WM_REQUEST,
	     XA_DT_WORKSPACE_LIST, XA_DT_WORKSPACE_CURRENT, NUM_ATOMS };
      static char *atom_names[] = {
	     _XA_DT_WORKSPACE_HINTS, _XA_DT_WORKSPACE_PRESENCE,
	     _XA_DT_WORKSPACE_INFO, WmNall,
	     _XA_DT_WORKSPACE_EMBEDDED_CLIENTS, _XA_DT_WM_REQUEST,
	     _XA_DT_WORKSPACE_LIST, _XA_DT_WORKSPACE_CURRENT };

      Atom atoms[XtNumber(atom_names)];
      XInternAtoms(DISPLAY, atom_names, XtNumber(atom_names), False, atoms);

      wmGD.xa_DT_WORKSPACE_HINTS = atoms[XA_DT_WORKSPACE_HINTS];
      wmGD.xa_DT_WORKSPACE_PRESENCE = atoms[XA_DT_WORKSPACE_PRESENCE];
      wmGD.xa_DT_WORKSPACE_INFO = atoms[XA_DT_WORKSPACE_INFO];
      wmGD.xa_ALL_WORKSPACES = atoms[XA_WmNall];
      wmGD.xa_DT_EMBEDDED_CLIENTS = atoms[XA_DT_WORKSPACE_EMBEDDED_CLIENTS];
      wmGD.xa_DT_WM_REQUEST = atoms[XA_DT_WM_REQUEST];
      wmGD.xa_DT_WORKSPACE_LIST = atoms[XA_DT_WORKSPACE_LIST];
      wmGD.xa_DT_WORKSPACE_CURRENT = atoms[XA_DT_WORKSPACE_CURRENT];
    }

#endif /* WSM */

    /* Initialize properties used in session management. */
    wmGD.xa_SM_CLIENT_ID =
      XmInternAtom (DISPLAY, _XA_DT_SM_CLIENT_ID, False);
    wmGD.xa_WMSAVE_HINT =
      XmInternAtom (DISPLAY, _XA_DT_WMSAVE_HINT, False);

    /* Load client resource database. */
    wmGD.clientResourceDB = LoadClientResourceDB();

    /*
     * Make the window manager workspace window.
     * Setup the _MOTIF_WM_INFO property on the root window.
     */

    SetupWmWorkspaceWindows ();


    /* make the cursors that the window manager uses */
    MakeWorkspaceCursors ();


    /* Sync the table used by Mwm's modifier parser to actual modMasks used */
    SyncModifierStrings();

    /*
     * Setup screen data and resources (after processing Wm resources.
     */
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

	if (pSD->managed)
	{
#ifdef WSM
	    if (XDefaultScreen (wmGD.display) == pSD->screen)
	    {
		wmGD.commandWindow = wmGD.Screens[scr].wmWorkspaceWin;
	    }

#endif /* WSM */
	    /*
	     * Initialize workspace colormap data.
	     */

	    InitWorkspaceColormap (pSD);

	    /*
	     * Process the window manager resource description file (.mwmrc):
	     */

#ifdef PANELIST
	    ProcessWmFile (pSD, False /* not nested */);

#else /* PANELIST */
	    ProcessWmFile (pSD);
#endif /* PANELIST */


	    /*
	     * Setup default resources for the system menu and key bindings:
	     */

	    SetupDefaultResources (pSD);


	    /*
	     * Make global window manager facilities:
	     */

	    if(pSD->iconDecoration & ICON_ACTIVE_LABEL_PART)
	    {
		/* create active icon window */
		CreateActiveIconTextWindow(pSD); 
	    }


	    /*
	     * Make menus and other resources that are used by window manager
	     * functions that are activated by menus, buttons and keys.
	     */

	    MakeWmFunctionResources (pSD);
	}

#ifdef WSM
        /*
	 *
	 *  Set root cursor to be a pointer for dtwm
	 *
	 */

# ifdef __osf__
	/* Fixes problem on multiscreen where cursor is only
         * set on primary screen.
	 */
	if (DtwmBehavior)
	{
	    XDefineCursor (DISPLAY,
		RootWindow (DISPLAY, scr),
		wmGD.workspaceCursor);
	}
# endif
#endif /* WSM */

    }
#ifdef PANELIST
    /*
     * Remove any temp config file we created if we needed to
     * convert DT 2.0 syntax to DT 3.0
     */
    DeleteTempConfigFileIfAny();
#endif /* PANELIST */
#ifdef WSM
    /*
     * Point second display's resource data base
     * to the first display's resource data base
     * so dtwm "clients" change colors dynamically.
     *
     *  NEW LOCATION
     */
      wmGD.display1->db = wmGD.display->db;
#endif /*  WSM */

    /*
     * Realize the top level widget, make the window override
     * redirect so we don't manage it, and then move it out of the way
     */

    XtRealizeWidget (wmGD.topLevelW);
#ifdef WSM
    XtRealizeWidget (wmGD.topLevelW1);

    /*
     * Initialize the message handling.
     * (This must be done after the realize because a window
     *  is required for ICCCM-style messaging).
     */
    dtInitializeMessaging (wmGD.topLevelW);
#endif /* WSM */

    sAttributes.override_redirect = True;
    XChangeWindowAttributes (DISPLAY, XtWindow (wmGD.topLevelW),
		CWOverrideRedirect, &sAttributes);


    /* setup window manager inter-client communications conventions handling */
    SetupWmICCC ();

    /*
     * Use the WM_SAVE_YOURSELF protocol
     * for notification of when to save ourself
     */
#ifdef WSM
    SetMwmSaveSessionInfo(wmGD.commandWindow);
#endif
    /*
     * Initialize window manager event handling:
     */

    InitEventHandling ();



    /*
     * Initialize frame component graphics
     */
    {
	for (scr = 0; scr < wmGD.numScreens; scr++)
	{
	    pSD = &(wmGD.Screens[scr]);

	    if (pSD->managed)
	    {
		InitClientDecoration (pSD);

		/*
		 * Make an icon box if specificed:
		 */
		if (pSD->useIconBox)
		{
		    InitIconBox (pSD);
		}

		/*
		 * Adopt client windows that exist before wm startup:
		 */

		AdoptInitialClients (pSD);

		/*
		 * Setup initial keyboard focus and colormap focus:
		 */

		InitColormapFocus (pSD);

	    }
	}

        for (scr = 0; scr < wmGD.numScreens; scr++)
        {
#ifdef WSM
	    int iws;
#endif /* WSM */
            pSD = &(wmGD.Screens[scr]);
	    
            if (pSD->managed)
            {
                ACTIVE_PSD = &wmGD.Screens[scr];
#ifdef WSM
                MapIconBoxes (pSD->pActiveWS);

		ChangeBackdrop (pSD->pActiveWS);

#ifdef HP_VUE
		UpdateWorkspaceInfoProperty (pSD); /* backward compatible */
#endif /* HP_VUE */

		SetCurrentWorkspaceProperty (pSD);
		SetWorkspaceListProperty (pSD);

		for (iws=0; iws < pSD->numWorkspaces; iws++)
		{
		    SetWorkspaceInfoProperty (&(pSD->pWS[iws]));
		}


XFlush (DISPLAY);



                /* MapWorkspaceBox (); */

#ifdef PANELIST
		/*
		 * Allocate front panel widgets
		 */
		if (wmGD.useFrontPanel &&  (pSD == wmGD.dtSD))
		{
		    Pixmap 	iconBitmap;
		    Arg		al[5];
		    int 	ac;
		    Widget	wFpShell;
		    WmPanelistObject  pPanelist;

                    wmGD.dtSD->wPanelist =
		       WmPanelistAllocate(pSD->screenTopLevelW1, 
		                          (XtPointer) &wmGD, (XtPointer) pSD);

		    pPanelist = (WmPanelistObject) pSD->wPanelist;

		    if (pPanelist != NULL && O_Shell(pPanelist))
		    {
			/*
			 * Make a default front panel icon image.
			 */
			iconBitmap = XCreateBitmapFromData (DISPLAY, 
				    pSD->rootWindow, 
				    (char *) fntpl_i_bm_bits, 
				    fntpl_i_bm_width, 
				    fntpl_i_bm_height);

			ac = 0; 
			XtSetArg (al[ac], XmNiconPixmap, iconBitmap); ac++; 
			XtSetValues (O_Shell(pPanelist), al, ac);

		    }
		}

		if (wmGD.useFrontPanel && pSD->wPanelist && 
		    (pSD == wmGD.dtSD))
		{
		    /*
		     * Make the front panel visible
		     */
		    WmPanelistShow (pSD->wPanelist);

		    /*
		     * Find special clients associated with the
		     * front panel. This needs to be done after
		     * WmPanelistShow where the data is set up.
		     */
		    ScanForPushRecallClients (pSD);
		    ScanForEmbeddedClients (pSD);
		}
#endif /* PANELIST */
		
		RestoreHelpDialogs(pSD);
#else /* WSM */
                MapIconBoxes (pSD->pActiveWS);
#endif /* WSM */
            }
        }
        firstTime = 0;
    }
    
    InitKeyboardFocus ();

#ifndef WSM
    InitWmDisplayEnv ();
#endif
    ShowWaitState (FALSE);

#ifdef WSM
    /*
     * Tell the rest of DT that we're up
     */
    dtReadyNotification();
#endif /* WSM */

#ifdef DEBUG_RESOURCE_DATABASE
    XrmPutFileDatabase(wmGD.display->db, "/tmp/dtwm.resDB");
#endif /* DEBUG_RESOURCE_DATABASE */

} /* END OF FUNCTION InitWmGlobal */



/******************************<->*************************************
 *
 *  InitWmScreen
 *
 *
 *  Description:
 *  -----------
 *  This function initializes a screen data block.
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to preallocated screen data block
 *  sNum = screen number for this screen
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

void
InitWmScreen (WmScreenData *pSD, int sNum)
{
    Arg args[12];
    int argnum;

#ifdef WSM
    int wsnum;
    WmWorkspaceData *pwsI;
    int buf_size;
    int i;
    static int dupnum = 0;
    int iwsx;
#endif /* WSM */

    char *pDisplayName;
#define LENCBUFFER 256
    char buffer[LENCBUFFER];		/* screen name & display name! */
    char displayName[LENCBUFFER];
    char *token1, *token2;


   /*
    * Set screen data values
    */

    pSD->rootWindow = RootWindow (DISPLAY, sNum);
    pSD->clientCounter = 0;
    pSD->defaultSystemMenuUseBuiltin = TRUE;
    pSD->displayString = NULL;
    pSD->acceleratorMenuCount = 0;
    pSD->activeIconTextWin = (Window)NULL;
    pSD->focusPriority = 0;
    pSD->inputScreenWindow = (Window)NULL;
    pSD->colormapFocus = NULL;
    pSD->keySpecs = NULL;
    pSD->screen = sNum;
    pSD->confirmboxW[DEFAULT_BEHAVIOR_ACTION] = NULL;
    pSD->confirmboxW[CUSTOM_BEHAVIOR_ACTION] = NULL;
    pSD->confirmboxW[RESTART_ACTION] = NULL;
    pSD->confirmboxW[QUIT_MWM_ACTION] = NULL;
    pSD->feedbackWin = (Window)NULL;
    pSD->fbStyle = FB_OFF;
    pSD->fbWinWidth = 0;
    pSD->fbWinHeight = 0;
    pSD->fbLocation[0] = '\0';
    pSD->fbSize[0] = '\0';
    pSD->fbLocX = 0;
    pSD->fbLocY = 0;
    pSD->fbSizeX = 0;
    pSD->fbSizeY = 0;
    pSD->fbLastX = -1;
    pSD->fbLastY = -1;
    pSD->fbLastWidth = -1;
    pSD->fbLastHeight = -1;
    pSD->fbTop = NULL;
    pSD->fbBottom = NULL;
    pSD->actionNbr = -1;
    pSD->clientList = NULL;
    pSD->lastClient = NULL;
    pSD->lastInstalledColormap = (Colormap)NULL;
    pSD->shrinkWrapGC = NULL;
    pSD->bitmapCache = NULL;
    pSD->bitmapCacheSize = 0;
    pSD->bitmapCacheCount = 0;
    pSD->dataType = SCREEN_DATA_TYPE;
    pSD->managed = False;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    pSD->cciTree = NULL;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#ifdef WSM
    pSD->initialWorkspace=NULL;
    pSD->presence.shellW = NULL;
    pSD->presence.onScreen = False;
    pSD->presence.userDismissed = True;
    pSD->workspaceList = NULL;
    pSD->numWorkspaces = 0;
    pSD->numWsDataAllocated = 0;
    pSD->lastBackdropWin = NULL;
    pSD->pDtSessionItems = NULL;
    pSD->totalSessionItems = 0;
    pSD->remainingSessionItems = 0;

    pSD->cachedHelp = NULL;
    pSD->dtHelp.shell  = (Widget)NULL;
    pSD->dtHelp.dialog = (Widget)NULL;
    pSD->dtHelp.errorDialog = (Widget)NULL;
    pSD->dtHelp.xPos = 0;
    pSD->dtHelp.yPos = 0;
    pSD->dtHelp.restored = False;
    pSD->dtHelp.onScreen = False;
    pSD->dtHelp.userDismissed = False;
    pSD->dtHelp.pCDforClient= NULL;
    pSD->helpResources=NULL;
    pSD->bMarqueeSelectionInitialized = False;
    pSD->woN = (Window) 0L;
    pSD->woS = (Window) 0L;
    pSD->woE = (Window) 0L;
    pSD->woW = (Window) 0L;
#endif /* WSM */
#ifdef PANELIST
    pSD->wPanelist = NULL;
    pSD->pECD = NULL;
    pSD->numPushRecallClients = 0;
    pSD->numEmbeddedClients = 0;
    pSD->pPRCD = NULL;
    pSD->iconBoxControl = False;
#endif /* PANELIST */
#ifdef WSM
    pSD->displayResolutionType = _DtGetDisplayResolution(DISPLAY, sNum);

    /*
     *  We've got display resolution type--now, let's get color
     *  characteristics.
     */

    ProcessWmColors (pSD);

    if (!(strcmp ((char *)wmGD.screenNames[sNum], UNSPECIFIED_SCREEN_NAME)))
    {
	sprintf (buffer, "%d", sNum);

	buf_size = strlen(buffer) + 1;

	if ((wmGD.screenNames[sNum] = 
	     (unsigned char *)XtRealloc (wmGD.screenNames[sNum], buf_size)) == NULL)
	{
	    Warning (((char *)GETMESSAGE(40, 7, "Cannot create enough memory for the screen names")));
	    ExitWM (WM_ERROR_EXIT_VALUE);
	}
	else
	{
	    strcpy((char *)wmGD.screenNames[sNum], buffer);
	}
    } /* if wmGD.screenNames[sNum] == UNSPECIFIED_SCREEN_NAME */

#endif /* WSM */
    /*
     * Save screen context
     */
    XSaveContext (DISPLAY, pSD->rootWindow, wmGD.screenContextType,
	(caddr_t)pSD);
    /*
     * Create shell widget for screen resource hierarchy
     */

    argnum = 0;
    XtSetArg (args[argnum], XtNgeometry, NULL);	argnum++;
    XtSetArg (args[argnum], XtNx, 10000);	argnum++;
    XtSetArg (args[argnum], XtNy, 10000);	argnum++;
    XtSetArg (args[argnum], XtNwidth, 10);	argnum++;
    XtSetArg (args[argnum], XtNheight, 10);	argnum++;
    XtSetArg (args[argnum], XtNoverrideRedirect, True);	argnum++;

    XtSetArg (args[argnum], XtNdepth, 
	    DefaultDepth(DISPLAY, sNum));	argnum++;
    XtSetArg (args[argnum], XtNscreen, 
	    ScreenOfDisplay(DISPLAY, sNum)); 	argnum++;
    XtSetArg (args[argnum], XtNcolormap, 
	    DefaultColormap(DISPLAY, sNum)); 	argnum++;
    XtSetArg (args[argnum], XtNvisual, 
	    DefaultVisual(DISPLAY, sNum)); 	argnum++;

    pSD->screenTopLevelW = XtCreatePopupShell ((String) wmGD.screenNames[sNum],
					       vendorShellWidgetClass,
					       wmGD.topLevelW,
					       args,
					       argnum);

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    /* Create a DrawingArea as a child of the popupShell.  This will be used
     * to handle UTM traffic relating to cci.  We need this
     * particular widget to get the callbacks from conversion requests made
     * against Mwm and the requests Mwm makes against other clients.
     */
    pSD->utmShell = XmCreateDrawingArea(pSD->screenTopLevelW, "UTM_Shell",
					NULL, 0);
    XtManageChild(pSD->utmShell);

    /*
     * Setup the destinationCallback handler to handle conversion
     * requests made by Mwm against other clients.
     */
    XtAddCallback(pSD->utmShell, XmNdestinationCallback, UTMDestinationProc,
		  NULL);

    /* Must realize to own WM_i if unmapped, causes mwm to
       freeze when menu is displayed. */
    XtPopup(pSD->screenTopLevelW, XtGrabNone);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#ifdef WSM
    argnum = 0;
    XtSetArg (args[argnum], XtNgeometry, NULL);			argnum++;
    XtSetArg (args[argnum], XtNx, 10000);			argnum++;
    XtSetArg (args[argnum], XtNy, 10000);			argnum++;
    XtSetArg (args[argnum], XtNwidth, 10);			argnum++;
    XtSetArg (args[argnum], XtNheight, 10);			argnum++;
    XtSetArg (args[argnum], XtNoverrideRedirect, True);		argnum++;
    XtSetArg (args[argnum], XtNmappedWhenManaged, False);	argnum++;

    XtSetArg (args[argnum], XtNdepth, 
	    DefaultDepth(DISPLAY1, sNum));	argnum++;
    XtSetArg (args[argnum], XtNscreen, 
	    ScreenOfDisplay(DISPLAY1, sNum)); 	argnum++;
    XtSetArg (args[argnum], XtNcolormap, 
	    DefaultColormap(DISPLAY1, sNum)); 	argnum++;

    pSD->screenTopLevelW1 = XtCreatePopupShell ((String) wmGD.screenNames[sNum],
					       vendorShellWidgetClass,
					       wmGD.topLevelW1,
					       args,
					       argnum);
    XtRealizeWidget (pSD->screenTopLevelW1);
#endif /* WSM */
    /*
     * Fetch screen based resources
     */
    ProcessScreenResources (pSD, wmGD.screenNames[sNum]);

    /*
     * Initialize other screen resources and parameters
     */
    MakeXorGC (pSD);
    InitIconSize(pSD);

#ifdef WSM
    /*
     *  Complete initialization of workspace structures
     */
    for (wsnum = 0, 
	 pwsI = pSD->pWS; wsnum < pSD->numWorkspaces; 
	 wsnum++, pwsI++)
    {
	/*
	 * Set up workspace for this screen
	 */
	InitWmWorkspace (pwsI, pSD);

    }

    if (pSD->initialWorkspace)
    {
	/* 
	 * restore to the last initialWorkspace saved from
	 * Quit, Restart, OR save session 
	 */

	/*
	 * Compare initialWorkspace against workspace name,
	 * NOT workspace title.
	 */
	for (iwsx = 0; iwsx < pSD->numWorkspaces; iwsx++)
	{
	    if (!strcmp(pSD->pWS[iwsx].name, pSD->initialWorkspace))
	    {
		break;
	    }
	}
	/* check bounds */
	if (iwsx >= pSD->numWorkspaces)
	{
	    /* make first workspace in list the active one to start with */
	    pSD->pActiveWS = pSD->pWS;
	}
	else
	{
	    pSD->pActiveWS = &pSD->pWS[iwsx];
	}
    }
    else
    {
	/* make first workspace in list the active one to start with */
	pSD->pActiveWS = pSD->pWS;
    }
#else /* WSM */
    /*
     *  Allocate and initialize a workspace structure
     */
    
    if (!(pSD->pWS = (WmWorkspaceData *) XtMalloc (sizeof(WmWorkspaceData))))
    {
	ShowWaitState (FALSE);
	Warning (((char *)GETMESSAGE(40, 8, "Insufficient memory for Workspace data")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

    /*
     * Set up workspace for this screen
     */
    InitWmWorkspace (pSD->pWS, pSD);
    pSD->pActiveWS = pSD->pWS;
#endif /* WSM */


    pDisplayName = DisplayString (DISPLAY);

    /*
     * Construct displayString for this string.  
     *
     * NOTE: The variable buffer is reused here. It was
     * used earlier to generate a screen name.
     */

    strcpy(displayName, pDisplayName);

    token1 = (char*)strtok(displayName, ":");		/* parse of hostname */

    if((token2 = (char*)strtok(NULL, ".")) ||		/* parse off dpy & scr # */
       (token2 = (char*)strtok(NULL, "")) ||
       (displayName[0] == ':'))
    {
	if (displayName[0] == ':')		/* local dpy (special case) */
	{
	    if ((token2 = (char*)strtok(token1, ".")) != NULL)	/* parse dpy# */
		sprintf(buffer, "DISPLAY=:%s.%d",
			token2,	sNum);
	} else {				/* otherwise process normally */
	    sprintf(buffer, "DISPLAY=%s:%s.%d",
		    token1, token2, sNum);
	}

	/*		
	 * Allocate space for the display string
	 */
    
	if ((pSD->displayString =
	     (String)XtMalloc ((unsigned int) (strlen(buffer) + 1))) == NULL)
	{
	    Warning (((char *)GETMESSAGE(40, 9, 
				    "Insufficient memory for displayString")));
	}
	else
	{
	    strcpy(pSD->displayString, buffer);
	}

    }



} /* END OF FUNCTION  InitWmScreen */


/*************************************<->*************************************
 *
 *  InitWmWorkspace
 *
 *
 *  Description:
 *  -----------
 *  This function initializes a workspace data block.
 *
 *  Inputs:
 *  -------
 *  pWS = pointer to preallocated workspace data block
 *  pSD = ptr to parent screen data block
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD)
{
    Arg args[10];
    int argnum;

#ifndef WSM
#define DEFAULT_WS_NAME "workspace"
#endif /* not WSM */

    pWS->pSD = pSD;
    pWS->pIconBox = NULL;
    pWS->dataType = WORKSPACE_DATA_TYPE;
#ifdef WSM
    pWS->backdrop.window = 0;
    pWS->backdrop.nameAtom = 0;
    pWS->backdrop.image = NULL;
    pWS->numClients = 0;
    pWS->sizeClientList = 0;
    pWS->ppClients = 0;
    pWS->buttonW = NULL;
#else /* WSM */

    if ((pWS->name = (char *) 
	    XtMalloc ((1+strlen(DEFAULT_WS_NAME)) * sizeof (char))) == NULL)
    {
	ShowWaitState (FALSE);
	ExitWM (WM_ERROR_EXIT_VALUE);
    }
    strcpy (pWS->name, DEFAULT_WS_NAME);
#endif /* WSM */

    /*
     * Create widget for workspace resource hierarchy
     */
    argnum = 0;
    XtSetArg (args[argnum], XtNdepth, 
	    DefaultDepth(DISPLAY, pSD->screen));	argnum++;
    XtSetArg (args[argnum], XtNscreen, 
	    ScreenOfDisplay(DISPLAY, pSD->screen)); 	argnum++;
    XtSetArg (args[argnum], XtNcolormap, 
	    DefaultColormap(DISPLAY, pSD->screen)); 	argnum++;
    XtSetArg (args[argnum], XtNwidth,  5);		argnum++;
    XtSetArg (args[argnum], XtNheight,  5);		argnum++;

    pWS->workspaceTopLevelW = XtCreateWidget (	pWS->name,
						xmPrimitiveWidgetClass,
    						pSD->screenTopLevelW,
					   	args,
						argnum);

#ifdef WSM
    /* internalize the workspace name */
    pWS->id = XInternAtom (DISPLAY, pWS->name, False);
#endif /* WSM */

    /*
     * Process workspace based resources
     */
    ProcessWorkspaceResources (pWS);	

    /* setup icon placement */
    if (wmGD.iconAutoPlace)
    {
	InitIconPlacement (pWS); 
    }

} /* END OF FUNCTION  InitWmWorkspace */

#ifdef WSM

/******************************<->*************************************
 *
 *  InsureDefaultBackdropDir(char **ppchBackdropDirs)
 *
 *
 *  Description:
 *  -----------
 *  This function checks and edits a directory path to insure
 *  that the system backdrop directroy (/usr/dt/backdrops) is in the 
 *  path. If not it adds it to the end. Further, it always adds the user's 
 *  backdrop directory ($HOME/.dt/backdrops) to the beginning of the path 
 *  and the system admin directory (/etc/dt/backdrops) before the system
 *  directory.
 *
 *  Inputs:
 *  -------
 *  ppchBackdropDirs  - Pointer to a pointer to a directory path 
 *			(must be allocated memory)
 *
 *  Outputs:
 *  -------
 *  *ppchBackdropDirs - Directory path may be modified, path 
 *			pointer may be realloc'ed.
 * 
 *  Comments:
 *  --------
 *  Assumes that the default directory does not start with a
 *  multi-byte character.
 * 
 ******************************<->***********************************/
static void
InsureDefaultBackdropDir(char **ppchBackdropDirs)
{
  int len;
  Boolean bFound = False;
  char *pch, *pchEnd, *pch2, *tmpptr;
  char *pchD = DEFAULT_BACKDROP_DIR;
  unsigned int chlen;
  char * homeDir;

  /*
   * Set up initial stuff
   */
  pch = *ppchBackdropDirs;
  len = strlen (pchD);
  pchEnd = pch + strlen(pch);
  
  while (!bFound && (pch != NULL) && (*pch != NULL))
    {
      if (strncmp (pch, pchD, len) == 0)
	{
	  /* found partial match, confirm complete match ...
	   * complete match if char off end of partial match
	   * is a NULL or a colon 
	   */
	  pch2 = pch + len;	
	  if ((pch2 <= pchEnd) &&
	      ((*pch2 == NULL) ||
	       (((mblen (pch2, MB_CUR_MAX) == 1) &&
		 (*pch2 == ':')))))
	    {
	      bFound = True;
	    }
	}
      else 
	{
	  /* find next path component */
	  pch = strchr (pch, (int) ':'); 
	  if ((pch != NULL) && (*pch != NULL))
	    { 
	      /* skip path separator */
	      chlen = mblen (pch, MB_CUR_MAX);
	      pch += chlen;
	    }
	}
    }
  

  /*
   * Always add the user's home directory to the beginning of the string
   */
  homeDir = (char *) XmeGetHomeDirName();  
    
  /*
   * If found add the user's home directory ($HOME/.dt/backdrops) and the 
   * admin directory /etc/dt/backdrops to the beginning of the string
   */
  
  if (bFound)  
    {
      len = strlen (homeDir) + strlen("/.dt/backdrops") + 
	    strlen (*ppchBackdropDirs) + strlen("/etc/dt/backdrops") + 3;
      tmpptr = XtMalloc (len * sizeof (char *));
      strcpy (tmpptr, homeDir);
      strcat (tmpptr, "/.dt/backdrops");
      strcat (tmpptr, ":");
      strcat (tmpptr, "/etc/dt/backdrops");
      strcat (tmpptr, ":");
      strcat (tmpptr, *ppchBackdropDirs);
      *ppchBackdropDirs = tmpptr;
    }
  else
    /*
     * If string not found, then add home directory to the beginning of 
     * string and the admin directory and system directory to the end.
     */
    {
      len = strlen (homeDir) + strlen("/.dt/backdrops") + 
	    strlen (*ppchBackdropDirs) + strlen(pchD) + 
	    strlen("/etc/dt/backdrops") + 4;
      tmpptr = XtMalloc (len * sizeof (char *));
      strcpy (tmpptr, homeDir);
      strcat (tmpptr, "/.dt/backdrops");
      strcat (tmpptr, ":");
      strcat (tmpptr, *ppchBackdropDirs); 
      strcat (tmpptr, ":");
      strcat (tmpptr, "/etc/dt/backdrops"); 
      strcat (tmpptr, ":");
      strcat (tmpptr, pchD);
      *ppchBackdropDirs = tmpptr;
    }
  
} /* END OF FUNCTION InsureDefaultBackdropDirs */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  ProcessMotifWmInfo (rootWindowOfScreen)
 *
 *
 *  Description:
 *  -----------
 *  This function is used retrieve and save the information in the 
 *  _MOTIF_WM_INFO property.  If the property does not exist then
 *  the start / restart state is set to initial startup with the
 *  user specified (not standard) configuration.
 *
 *
 *  Outputs:
 *  -------
 *  wmGD.useStandardBehavior = True if set indicated in property
 *
 *  wmGD.wmRestarted = True if the window manager was restarted
 * 
 *************************************<->***********************************/

void ProcessMotifWmInfo (Window rootWindowOfScreen)
{
    MwmInfo *pMwmInfo;

    wmGD.xa_MWM_INFO = XInternAtom (DISPLAY, _XA_MWM_INFO, False);
    if ((pMwmInfo = (MotifWmInfo *)GetMwmInfo (rootWindowOfScreen)) != NULL)
    {
	wmGD.useStandardBehavior =
		(pMwmInfo->flags & MWM_INFO_STARTUP_STANDARD) ? True : False;
	wmGD.wmRestarted = True;
	XFree ((char *)pMwmInfo);
    }
    else
    {
	wmGD.useStandardBehavior = False;
	wmGD.wmRestarted = False;
    }

} /* END OF FUNCTION ProcessMotifWmInfo */



/*************************************<->*************************************
 *
 *  SetupWmWorkspaceWindows ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to setup a window that can be used in doing window
 *  management functions.  This window is not visible on the screen.
 *
 *
 *  Outputs:
 *  -------
 *  pSD->wmWorkspaceWin = window that is used to hold wm properties
 * 
 *************************************<->***********************************/

void SetupWmWorkspaceWindows (void)
{
    int scr;
    WmScreenData *pSD;
    XSetWindowAttributes sAttributes;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);
	if (pSD->managed)
	{
	    sAttributes.override_redirect = True;
	    sAttributes.event_mask = FocusChangeMask | PropertyChangeMask;
	    pSD->wmWorkspaceWin = XCreateWindow (DISPLAY, pSD->rootWindow, 
				      -100, -100, 10, 10, 0, 0, 
				      InputOnly, CopyFromParent,
				      (CWOverrideRedirect |CWEventMask),
				      &sAttributes);

	    XMapWindow (DISPLAY, pSD->wmWorkspaceWin);

	    SetMwmInfo (pSD->rootWindow, 
			(long) ((wmGD.useStandardBehavior) ?
                        MWM_INFO_STARTUP_STANDARD : MWM_INFO_STARTUP_CUSTOM), 
			pSD->wmWorkspaceWin);
#ifdef WSM
	    XSaveContext (DISPLAY, pSD->wmWorkspaceWin, 
		    wmGD.mwmWindowContextType, (caddr_t)pSD);
#endif /* WSM */
	}
    }

} /* END OF FUNCTION SetupWmWorkspaceWindow */



/*************************************<->*************************************
 *
 *  MakeWorkspaceCursors ()
 *
 *
 *  Description:
 *  -----------
 *  This function makes the cursors that the window manager uses.
 *
 *
 *  Inputs:
 *  ------
 *  XXinput = ...
 *
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (stretchCursors ...)
 * 
 *************************************<->***********************************/

void MakeWorkspaceCursors (void)
{
    wmGD.workspaceCursor = XCreateFontCursor (DISPLAY, XC_left_ptr);

    wmGD.stretchCursors[STRETCH_NORTH_WEST] =
	XCreateFontCursor (DISPLAY, XC_top_left_corner);
    wmGD.stretchCursors[STRETCH_NORTH] =
	XCreateFontCursor (DISPLAY, XC_top_side);
    wmGD.stretchCursors[STRETCH_NORTH_EAST] =
	XCreateFontCursor (DISPLAY, XC_top_right_corner);
    wmGD.stretchCursors[STRETCH_EAST] =
	XCreateFontCursor (DISPLAY, XC_right_side);
    wmGD.stretchCursors[STRETCH_SOUTH_EAST] =
	XCreateFontCursor (DISPLAY, XC_bottom_right_corner);
    wmGD.stretchCursors[STRETCH_SOUTH] =
	XCreateFontCursor (DISPLAY, XC_bottom_side);
    wmGD.stretchCursors[STRETCH_SOUTH_WEST] =
	XCreateFontCursor (DISPLAY, XC_bottom_left_corner);
    wmGD.stretchCursors[STRETCH_WEST] =
	XCreateFontCursor (DISPLAY, XC_left_side);

    wmGD.configCursor = XCreateFontCursor (DISPLAY, XC_fleur);

    wmGD.movePlacementCursor = XCreateFontCursor (DISPLAY, XC_ul_angle);
    wmGD.sizePlacementCursor = XCreateFontCursor (DISPLAY, XC_lr_angle);


} /* END OF FUNCTION MakeWorkspaceCursors */



/*************************************<->*************************************
 *
 *  MakeWmFunctionResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function makes menus and other resources that are used by window
 *  manager functions.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD  = (menuSpecs, keySpecs, buttonSpecs)
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD (menuSpecs) = new menu panes, protocol atoms
 *
 *************************************<->***********************************/

void MakeWmFunctionResources (WmScreenData *pSD)
{
    ButtonSpec *buttonSpec;
    KeySpec    *keySpec;
    MenuSpec   *menuSpec;
    Context     menuContext;


    /*
     * Scan through the menu specifications and make wm protocol atoms.
     */


    /*
     * Scan through the button binding specifications making menus if the
     * f.menu function is invoked.
     */

    buttonSpec = pSD->buttonSpecs;
    while (buttonSpec)
    {
	if (buttonSpec->wmFunction == F_Menu)
	{
	    if (buttonSpec->context & F_CONTEXT_WINDOW)
	    {
		menuContext = F_CONTEXT_WINDOW;
	    }
	    else if (buttonSpec->context & F_CONTEXT_ICON)
	    {
		menuContext = F_CONTEXT_ICON;
	    }
	    else
	    {
		menuContext = F_CONTEXT_ROOT;
	    }

	    menuSpec = MAKE_MENU (pSD, NULL, buttonSpec->wmFuncArgs,
				 menuContext,
	                         buttonSpec->context, 
				 (MenuItem *) NULL, FALSE);

	    if (menuSpec)
	    /* 
	     * If successful, save in pSD->acceleratorMenuSpecs 
	     * Note: these accelerators have nonzero contexts.
	     */
	    {
		SaveMenuAccelerators (pSD, menuSpec);
	    }
	    else
	    {
		buttonSpec->wmFunction = F_Nop;
	    }
	}
	buttonSpec = buttonSpec->nextButtonSpec;
    }


    /*
     * Scan through the key binding specifications making menus if the
     * f.menu function is invoked.
     */

    keySpec = pSD->keySpecs;
    while (keySpec)
    {
	if (keySpec->wmFunction == F_Menu)
	{
	    if (keySpec->context & F_CONTEXT_WINDOW)
	    {
		menuContext = F_CONTEXT_WINDOW;
	    }
	    else if (keySpec->context & F_CONTEXT_ICON)
	    {
		menuContext = F_CONTEXT_ICON;
	    }
	    else
	    {
		menuContext = F_CONTEXT_ROOT;
	    }

	    menuSpec = MAKE_MENU (pSD, NULL, keySpec->wmFuncArgs, menuContext,
	                         keySpec->context, 
				 (MenuItem *) NULL, FALSE);

	    if (menuSpec)
	    /* 
	     * If successful, save in pSD->acceleratorMenuSpecs 
	     * Note: these accelerators have nonzero contexts.
	     */
	    {
		SaveMenuAccelerators (pSD, menuSpec);
	    }
	    else
	    {
		keySpec->wmFunction = F_Nop;
	    }
	}
	keySpec = keySpec->nextKeySpec;
    }


} /* END OF FUNCTION MakeWmFunctionResources */



/*************************************<->*************************************
 *
 *  MakeXorGC (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Make an XOR graphic context for resizing and moving
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Modifies global data
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void MakeXorGC (WmScreenData *pSD)
{
    XGCValues gcv;
    XtGCMask  mask;

    mask = GCFunction | GCLineWidth | GCSubwindowMode | GCCapStyle;
    gcv.function = GXinvert;
    gcv.line_width = 0;
    gcv.cap_style = CapNotLast;
    gcv.subwindow_mode = IncludeInferiors;

    /* Fix so that the rubberbanding for resize and move will
     *  have more contrasting colors.
     */

    gcv.plane_mask = BlackPixelOfScreen( DefaultScreenOfDisplay( DISPLAY )) ^ 
                     WhitePixelOfScreen( DefaultScreenOfDisplay( DISPLAY ));
    mask = mask | GCPlaneMask;

    pSD->xorGC = XCreateGC (DISPLAY, pSD->rootWindow, mask, &gcv);


} /* END OF FUNCTION MakeXorGC */



/*************************************<->*************************************
 *
 *  CopyArgv (argc, argv)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a copy of the window manager's argv for use by
 *  the f.restart function.  A copy must be kept because XtInitialize
 *  changes argv.
 *
 *
 *  Inputs:
 *  ------
 *  argc = the number of strings in argv
 *
 *  argv = window manager parameters
 *
 * 
 *  Outputs:
 *  -------
 *  Return = a copy of argv
 *
 *************************************<->***********************************/


void CopyArgv (int argc, char *argv [])
{
    int i;


    if ((wmGD.argv = (char **)XtMalloc ((argc + 1) * sizeof (char *))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 10, "Insufficient memory for window manager data")));
	wmGD.argv = argv;
#ifdef WSM
	dpy2Argv = argv;
#endif /* WSM */
    }
    else
    {
	for (i = 0; i < argc; i++)
	{
	    wmGD.argv[i] = argv[i];
	}
	wmGD.argv[i] = NULL;
#ifdef WSM
	if ((dpy2Argv = (char **)XtMalloc((argc + 1) * sizeof(char *))) == NULL)
	{
	    Warning (((char *)GETMESSAGE(40, 11, "Insufficient memory for window manager data")));
	    dpy2Argv = argv;
	}
	else
	{
	    for (i = 0; i < argc; i++)
	    {
		dpy2Argv[i] = argv[i];
	    }
	    dpy2Argc = argc;
	    dpy2Argv[i] = NULL;
	}
#endif /* WSM */
    }
    
} /* END OF FUNCTION CopyArgv */


/*************************************<->*************************************
 *
 *  InitScreenNames ()
 *
 *
 *  Description:
 *  -----------
 *  Initializes the name space for screen names
 *
 *  Outputs:
 *  -------
 *  Modifies global data
 *    + screenNames
 *
 *  Comments:
 *  --------
 *  Initializes screenNames to contain a numeric name for each screen
 *
 *************************************<->***********************************/

void InitScreenNames (void)
{
    int num, numScreens;
    
    numScreens = ScreenCount (wmGD.display);
    
    if (!(wmGD.screenNames = 
	  (unsigned char **) XtMalloc (numScreens * sizeof(char *))))
    {
	ShowWaitState (FALSE);
	Warning (((char *)GETMESSAGE(40, 12, "Insufficient memory for screen names")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }
    
    for (num=0; num<numScreens; num++)
    {
	if (!(wmGD.screenNames[num] = 
	      (unsigned char *) XtMalloc (4*sizeof(char))))
	{
	    ShowWaitState (FALSE);
	    Warning (((char *)GETMESSAGE(40, 13, "Insufficient memory for screen names")));
	    ExitWM (WM_ERROR_EXIT_VALUE);
	}
	/* default name is left justified, 3-chars max, zero terminated */
#ifdef WSM
	sprintf((char *)wmGD.screenNames[num], UNSPECIFIED_SCREEN_NAME);
#else  /* WSM */
	sprintf((char *)wmGD.screenNames[num],"%d",num%1000);
#endif /* WSM */
    }
}
#ifndef NO_MESSAGE_CATALOG


void InitNlsStrings (void)
{
    char * tmpString;

#ifdef WSM
    /*
     * Initialize messages
     */
    wmGD.okLabel=XmStringCreateLocalized(_DtOkString);
    wmGD.cancelLabel=XmStringCreateLocalized(_DtCancelString);
    wmGD.helpLabel=XmStringCreateLocalized(_DtHelpString);
#endif /* WSM */    

    /*
     * catgets returns a pointer to an area that is over written
     * on each call to catgets.
     */

    tmpString = ((char *)GETMESSAGE(40, 14, "Icons"));
    if ((wmNLS.default_icon_box_title =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 15, "Insufficient memory for local message string")));
	wmNLS.default_icon_box_title = "Icons";
    }
    else
    {
	strcpy(wmNLS.default_icon_box_title, tmpString);
    }

#ifdef WSM
    tmpString = ((char *)GETMESSAGE(40, 20, "%s: %s on line %d of configuration file %s\n"));
    if ((pWarningStringFile =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 17, "Insufficient memory for local message string")));
	pWarningStringFile = "%s: %s on line %d of configuration file %s\n";
    }
    else
    {
	strcpy(pWarningStringFile, tmpString);
    }

    tmpString = ((char *)GETMESSAGE(40, 21, "%s: %s on line %d of specification string\n"));
    if ((pWarningStringLine =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 19, "Insufficient memory for local message string")));
	pWarningStringLine = "%s: %s on line %d of specification string\n";
    }
    else
    {
	strcpy(pWarningStringLine, tmpString);
    }


    tmpString = ((char *)GETMESSAGE(40, 22, "About Workspace Manager"));
    if ((wmNLS.defaultVersionTitle =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 15, "Insufficient memory for local message string")));
	wmNLS.defaultVersionTitle = "About Workspace Manager";
    }
    else
    {
	strcpy(wmNLS.defaultVersionTitle, tmpString);
    }

    tmpString = ((char *)GETMESSAGE(40, 23, "Workspace Manager - Help"));
    if ((wmNLS.defaultDtwmHelpTitle =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 15, "Insufficient memory for local message string")));
	wmNLS.defaultDtwmHelpTitle = "Workspace Manager - Help";
    }
    else
    {
	strcpy(wmNLS.defaultDtwmHelpTitle, tmpString);
    }

    tmpString = ((char *)GETMESSAGE(40, 24, "Workspace Manager - Help"));
    if ((wmNLS.defaultHelpTitle =
	 (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
	Warning (((char *)GETMESSAGE(40, 15, "Insufficient memory for local message string")));
	wmNLS.defaultHelpTitle = "Workspace Manager - Help";
    }
    else
    {
	strcpy(wmNLS.defaultHelpTitle, tmpString);
    }
#endif /* WSM */

} /* InitNlsStrings  */
#endif



/******************************<->*************************************
 *
 *  InitWmDisplayEnv
 *
 *
 *  Description:
 *  -----------
 *  This function saves the display string for putenv in F_Exec.
 *
 *  Inputs:
 *  -------
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

void
InitWmDisplayEnv (void)
{
    char *pDisplayName;
    char buffer[256];
    char displayName[256];

    pDisplayName = DisplayString (DISPLAY);
    
    /*
     * Construct displayString for this string.  
     */
    strcpy(displayName, pDisplayName);
    sprintf(buffer, "DISPLAY=%s",displayName);
    
    /*		
     * Allocate space for the display string
     */
    if ((wmGD.displayString =
	 (String)XtMalloc ((unsigned int) (strlen(buffer) + 1))) == NULL)
    {
	wmGD.displayString = NULL;
        Warning (((char *)GETMESSAGE(40, 9, 
				     "Insufficient memory for displayString")));
    }
    else
    {
	strcpy(wmGD.displayString, buffer);
#ifdef WSM
	putenv(wmGD.displayString);
#endif /* WSM */
    }
    
} /* END OF FUNCTION  InitWmDisplayEnv */

#ifndef NO_HP_KEY_REMAP
static str_xref
GetReplacementList(
        Display *dsp,
        str_xref std_xref,
	int count)
{
  int min_kc ;
  int max_kc ;
  int ks_per_kc ;
  int kc_count ;
  KeySym *key_map ; 
  unsigned i ;
  str_xref xref_rtn = NULL ;
  unsigned num_xref = 0 ;

  XDisplayKeycodes( dsp, &min_kc, &max_kc) ;
  kc_count = max_kc + 1 - min_kc ;
  key_map = XGetKeyboardMapping( dsp, min_kc, kc_count, &ks_per_kc) ;
  if(    key_map == NULL    )
    {
      return NULL ;
    }
  kc_count *= ks_per_kc ;

  i = 0 ;
  while(    i < count    )
    {
      KeySym ks = XStringToKeysym( std_xref[i].default_name) ;
      unsigned j = 0 ;

      while(    j < kc_count    )
        {
          if(    key_map[j] == ks    )
            {
              /* Found keysym used in virtkey table in keymap,
               *   so break ->  j != kc_count
               */
              break ;
            }
          ++j ;
        }
      if(    j == kc_count    )
        {
          /* Didn't find keysym of virtkey table, so add record to
           *   returned list which will later cause replacement in
           *   virtkeys table.
           */
          xref_rtn = (str_xref) XtRealloc( (char *) xref_rtn,
                                      sizeof( str_xref_rec) * (num_xref + 2)) ;
          xref_rtn[num_xref++] = std_xref[i] ;
          xref_rtn[num_xref].default_name = NULL ;
        }
      ++i ;
    }
  XFree( (char *) key_map) ;
  return xref_rtn ;
}

static Boolean
GetBindingsProperty(
        Display *dsp,
        Atom property,
        String *binding)
{
  Atom actual_type ;
  int actual_format ;
  unsigned long num_items ;
  unsigned long bytes_after ;
  unsigned char *prop = NULL ;

  XGetWindowProperty( dsp, RootWindow( dsp, 0), property, 0, 1000000L,
      FALSE, XA_STRING, &actual_type, &actual_format, &num_items, &bytes_after,
                                                                       &prop) ;
  if(    (actual_type != XA_STRING)
      || (actual_format != 8)
      || (num_items == 0)    )
    {
      if(    prop != NULL    )
        {
          XFree( prop) ;
        }
      return FALSE ;
    }
  *binding = (String) prop ;
  return TRUE ;
}

static void
SetBindingsProperty(
        Display *dsp,
        Atom property,
        String binding)
{
  XChangeProperty( dsp, RootWindow( dsp, 0), property, XA_STRING, 8,
                PropModeReplace, (unsigned char *) binding, strlen( binding)) ;
}

static String
FixupBindingsString(
        String bindingsString,
        str_xref repl_xref)
{
  String fixed_str = XtNewString( bindingsString) ;
  String ptr_next = fixed_str ;

  while(    repl_xref->default_name != NULL    )
    {
      String ks_ptr = strstr( ptr_next, repl_xref->default_name) ;
      unsigned orig_len = strlen( repl_xref->default_name) ;

      if(    ks_ptr == NULL    )
        {
          /* Only increment to next replacement when no other instances
           *   are found in fixed_str.
           */
          ++repl_xref ;
          ptr_next = fixed_str ;
          continue ;
        }

      if(    (strpbrk( (ks_ptr - 1), " \t>") == (ks_ptr - 1))
          && (strpbrk( ks_ptr, " \t\n") == (ks_ptr + orig_len))    )
        {
          unsigned new_len = strlen( repl_xref->new_name) ;
          unsigned suffix_len = strlen( ks_ptr + orig_len) ;

          if(    new_len > orig_len    )
            {
              unsigned new_ttl_len = strlen( fixed_str) + new_len - orig_len ;
              unsigned prefix_len ;

              *ks_ptr = '\0' ;
              prefix_len = strlen( fixed_str) ;
              fixed_str = XtRealloc( fixed_str, (new_ttl_len + 1)) ;
              ks_ptr = fixed_str + prefix_len ;
            }
          memmove( (ks_ptr + new_len), (ks_ptr + orig_len), (suffix_len + 1)) ;
          memcpy( ks_ptr, repl_xref->new_name, new_len) ;

          ptr_next = ks_ptr + new_len ;
        }
      else
        {
          ptr_next = ks_ptr + 1 ;
        }
    }
  return fixed_str ;
}


Boolean
VirtKeys4DIN(
        Display *dsp)
{
  /* This routine examines the X server's key map table to determine
   * if certain HP-specific keysyms are missing.  If they are, then
   * the Motif virtual binding table properties are updated to utilize
   * generic X keysyms instead of the missing HP vendor keysyms.
   * In particular, this fixes the Motif virtual key binding table for
   * correct operation on HP systems using the AT2/DIN style keyboard.
   */
  static char *prop_names[] = { "_MOTIF_BINDINGS",
                                "_MOTIF_DEFAULT_BINDINGS" } ;
  static str_xref_rec std_xref[] = { { "hpInsertChar", "Insert" },
                                     { "hpDeleteChar", "Delete" },
				     { "End", "F7" },
                                   } ;
  Boolean PropChanged4DIN = FALSE ;
  unsigned i ;
  char *bindingsString ;
  unsigned prop_existed ;
  Atom *prop_atoms ;
  str_xref vkeysym_xref ;
  unsigned num_props = XtNumber( prop_names) ;

  vkeysym_xref = GetReplacementList( dsp, std_xref, XtNumber(std_xref)) ;
  if(    vkeysym_xref == NULL    )
    {
      return PropChanged4DIN ;
    }

  prop_atoms = (Atom *) XtMalloc( sizeof( Atom) * num_props) ;
  XInternAtoms(dsp, prop_names, num_props, FALSE, prop_atoms);

  prop_existed = FALSE ;
  i = 0 ;
  while(    i < num_props    )
    {
      if(    GetBindingsProperty( dsp, prop_atoms[i], &bindingsString)    )
        {
          String new_bstring = FixupBindingsString( bindingsString,
                                                                vkeysym_xref) ;
          prop_existed = TRUE ;
          XFree( bindingsString) ;

          if(    new_bstring != NULL    )
            {
              SetBindingsProperty( dsp, prop_atoms[i], new_bstring) ;
              XtFree( new_bstring) ;
            }
        }
      ++i ;
    }
  if(    !prop_existed    )
    {
      bindingsString = NULL ;
      _XmVirtKeysLoadFallbackBindings( dsp, &bindingsString) ;
      XtFree( bindingsString) ;

      i = 0 ;
      while(    i < num_props    )
        {
          if(    GetBindingsProperty( dsp, prop_atoms[i], &bindingsString)    )
            {
              String new_bstring = FixupBindingsString( bindingsString,
                                                                vkeysym_xref) ;
              XtFree( bindingsString) ;
              if(    new_bstring != NULL    )
                {
                  PropChanged4DIN = TRUE ;
                  SetBindingsProperty( dsp, prop_atoms[i], new_bstring) ;
                  XtFree( new_bstring) ;
                }
              XFree( bindingsString) ;
            }
          ++i ;
        }
    }

  XtFree( (char *) vkeysym_xref) ;
  XtFree( (char *) prop_atoms) ;
  return PropChanged4DIN ;
}
#endif /* NO_HP_KEY_REMAP */

#ifdef WSM
/****************************   eof    ***************************/
#endif /* WSM */
