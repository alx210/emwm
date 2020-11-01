#ifndef _WmGlobal_h
#define _WmGlobal_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


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
/*   $XConsortium: WmGlobal.h /main/16 1996/10/30 11:55:23 drk $ */
/*
 * (c) Copyright 1987,1988,1989,1990,1992,1993,1994 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/* ANSI C definitions,  This should be the first thing in WmGlobal.h */
#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif


/*
 * Included Files:
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifndef NO_SHAPE
#include <X11/extensions/shape.h>
#endif /* NO_SHAPE  */
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#ifdef WSM
#include <Xm/ColorObjP.h>
#include <Dt/Service.h>
#include <Dt/Wsm.h>
#include <Dt/WsmP.h>
#include "WmParse.h"
#ifdef PANELIST
#include <Dt/Action.h>
#endif /* PANELIST */
#endif /* WSM */

#if defined(sun) && defined(ALLPLANES)
#include <X11/extensions/allplanes.h>
#endif /* defined(sun) && defined(ALLPLANES) */

/*
 * Value definitions and macros:
 */

#ifdef MOTIF_ONE_DOT_ONE
#define XmFONTLIST_DEFAULT_TAG	"XmSTRING_DEFAULT_CHARSET"
#endif
#ifdef WSM

extern int WmIdentity;

/*
 * Color server defines
 */
#define CSERVE_NORMAL		   0
#define CSERVE_NOT_AVAILABLE       1
#define CSERVE_FAILURE             2
#endif /* WSM */

/* window manager name and class used to get resources: */
#define	WM_RESOURCE_CLASS	"Mwm"
#define WM_RESOURCE_NAME	"mwm"

#ifdef WSM
#define	DT_WM_RESOURCE_CLASS	"Dtwm"
#define DT_WM_RESOURCE_NAME	"dtwm"


extern Pixel		FPbackground;
extern Pixel		FPforeground;
extern Pixel		FPtopshadow;
extern Pixel		FPbottomshadow;
extern Pixel		FPselectcolor;

#define  USE_ACTIVE_PIXELSET		0
#define  USE_INACTIVE_PIXELSET		1
#define  USE_PRIMARY_PIXELSET		2
#define  USE_SECONDARY_PIXELSET		3

#define MWM	        	     0
#define DT_MWM		 	     1

#define MwmBehavior		(WmIdentity == MWM)
#define DtwmBehavior		(WmIdentity == DT_MWM)

#else
#define MwmBehavior		(True)

#endif /* WSM */

/* ICCC atom names: */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# define _XA_TARGETS		"TARGETS"
# define _XA_MULTIPLE		"MULTIPLE"
# define _XA_TIMESTAMP		"TIMESTAMP"
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#define _XA_WM_STATE		"WM_STATE"
#define _XA_WM_PROTOCOLS	"WM_PROTOCOLS"
#define _XA_WM_CHANGE_STATE	"WM_CHANGE_STATE"
#define _XA_WM_SAVE_YOURSELF	"WM_SAVE_YOURSELF"
#define _XA_WM_DELETE_WINDOW	"WM_DELETE_WINDOW"
#define _XA_WM_TAKE_FOCUS	"WM_TAKE_FOCUS"
#define _XA_WM_COLORMAP_WINDOWS	"WM_COLORMAP_WINDOWS"

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/* original set of query targets */
# define _XA_MOTIF_WM_CLIENT_WINDOW		"_MOTIF_WM_CLIENT_WINDOW"
# define _XA_MOTIF_WM_POINTER_WINDOW		"_MOTIF_WM_POINTER_WINDOW"
# define _XA_MOTIF_WM_ALL_CLIENTS		"_MOTIF_WM_ALL_CLIENTS"
	  
/* menu command interface support */
# define _XA_MOTIF_WM_DEFINE_COMMAND		"_MOTIF_WM_DEFINE_COMMAND"
# define _XA_MOTIF_WM_INCLUDE_COMMAND		"_MOTIF_WM_INCLUDE_COMMAND"
# define _XA_MOTIF_WM_REMOVE_COMMAND		"_MOTIF_WM_REMOVE_COMMAND"
# define _XA_MOTIF_WM_ENABLE_COMMAND		"_MOTIF_WM_ENABLE_COMMAND"
# define _XA_MOTIF_WM_DISABLE_COMMAND		"_MOTIF_WM_DISABLE_COMMAND"
# define _XA_MOTIF_WM_RENAME_COMMAND		"_MOTIF_WM_RENAME_COMMAND"
# define _XA_MOTIF_WM_INVOKE_COMMAND		"_MOTIF_WM_INVOKE_COMMAND"
# define _XA_MOTIF_WM_REQUEST_COMMAND		"_MOTIF_WM_REQUEST_COMMAND"
# define _XA_MOTIF_WM_WINDOW_FLAGS		"_MOTIF_WM_WINDOW_FLAGS"

/* automation support */
# define _XA_MOTIF_WM_AUTOMATION 		"_MOTIF_WM_AUTOMATION"
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

/* window manager exit value on fatal errors: */
#define WM_ERROR_EXIT_VALUE	1

/* built-in button bindings for window manager actions: */
#define SELECT_BUTTON			Button1
#define SELECT_BUTTON_MASK		Button1Mask
#define SELECT_BUTTON_MOTION_MASK	Button1MotionMask

#define FOCUS_SELECT_BUTTON	SELECT_BUTTON
#define FOCUS_SELECT_MODIFIERS	0

/* direct manipulation button */
#define DMANIP_BUTTON			Button2
#define DMANIP_BUTTON_MASK		Button2Mask
#define DMANIP_BUTTON_MOTION_MASK	Button2MotionMask

/* menu button */
#define BMENU_BUTTON			Button3
#define BMENU_BUTTON_MASK		Button3Mask

/* max number of buttons on a mouse */
#define NUM_BUTTONS			5

/* Needed by PostMenu() to specify key post: */
#define NoButton		0

/* manage window flags: */
#define MANAGEW_WM_STARTUP	(1L << 0)
#define MANAGEW_WM_RESTART	(1L << 1)
#define MANAGEW_NORMAL		(1L << 2)
#define MANAGEW_ICON_BOX	(1L << 3)
#define MANAGEW_CONFIRM_BOX	(1L << 4)
#define MANAGEW_WM_RESTART_ICON	(1L << 5)

#ifdef WSM
#define MANAGEW_WM_CLIENTS	(MANAGEW_ICON_BOX | \
				 MANAGEW_CONFIRM_BOX )
#else /* WSM */
#define MANAGEW_WM_CLIENTS	(MANAGEW_ICON_BOX | MANAGEW_CONFIRM_BOX)
#endif /* WSM */

/* keyboard input focus flag values (for calls to SetKeyboardFocus) */
#define ALWAYS_SET_FOCUS	(1L << 0)
#define REFRESH_LAST_FOCUS	(1L << 1)
#define CLIENT_AREA_FOCUS	(1L << 2)
#define SCREEN_SWITCH_FOCUS	(1L << 3)
/* special value for use for Do_Focus_Key to set to internal window */
#define WORKSPACE_IF_NULL	(1L << 4)

/* Menu posting flag values (for calls to PostMenu) */
#define POST_AT_XY		(1L << 0)
#define POST_TRAVERSAL_ON	(1L << 1)
#define POST_STICKY		(1L << 2)

/* feedback box styles */
#define FB_OFF			(0)
#define FB_SIZE			(1L << 0)
#define FB_POSITION		(1L << 1)

/* confirmbox and waitbox indexes */
#define DEFAULT_BEHAVIOR_ACTION		0
#define CUSTOM_BEHAVIOR_ACTION		1
#define RESTART_ACTION		2
#define QUIT_MWM_ACTION		3

/* extract text height in pixels from a (XFontStruct *) */
#define TEXT_HEIGHT(pfs) (((pfs)->ascent)+((pfs)->descent))

/* icon frame shadow widths */
#ifdef WSM
#define ICON_EXTERNAL_SHADOW_WIDTH	(wmGD.iconExternalShadowWidth)
#else /* WSM */
#define ICON_EXTERNAL_SHADOW_WIDTH	2
#endif /* WSM */
#define ICON_INTERNAL_SHADOW_WIDTH	1

/* padding widths */
#define ICON_IMAGE_TOP_PAD	2
#define ICON_IMAGE_BOTTOM_PAD	2
#define ICON_IMAGE_LEFT_PAD	2
#define ICON_IMAGE_RIGHT_PAD	2

/* image offsets */
#define ICON_INNER_X_OFFSET	\
	    (ICON_IMAGE_LEFT_PAD+ICON_EXTERNAL_SHADOW_WIDTH)
#define ICON_INNER_Y_OFFSET	\
	    (ICON_IMAGE_TOP_PAD+ICON_EXTERNAL_SHADOW_WIDTH)


#define ICON_IMAGE_X_OFFSET ICON_INNER_X_OFFSET+ICON_INTERNAL_SHADOW_WIDTH
#define ICON_IMAGE_Y_OFFSET ICON_INNER_Y_OFFSET+ICON_INTERNAL_SHADOW_WIDTH



/* number of rectangles to allocate */
#define NUM_MATTE_TS_RECTS	(6)
#define NUM_MATTE_BS_RECTS	(6)

#define NUM_IMAGE_TOP_RECTS	\
	    ((2*ICON_EXTERNAL_SHADOW_WIDTH)+(2*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_IMAGE_BOTTOM_RECTS	\
	    ((2*ICON_EXTERNAL_SHADOW_WIDTH)+(2*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_LABEL_TOP_RECTS	(2*ICON_EXTERNAL_SHADOW_WIDTH)
#define NUM_LABEL_BOTTOM_RECTS	(2*ICON_EXTERNAL_SHADOW_WIDTH)

#define NUM_BOTH_TOP_RECTS	\
	    ((3*ICON_EXTERNAL_SHADOW_WIDTH)+(3*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_BOTH_BOTTOM_RECTS	\
	    ((3*ICON_EXTERNAL_SHADOW_WIDTH)+(3*ICON_INTERNAL_SHADOW_WIDTH)) 


#define NUM_STATIC_TOP_RECTS	(2*ICON_INTERNAL_SHADOW_WIDTH)
#define NUM_STATIC_BOTTOM_RECTS	(2*ICON_INTERNAL_SHADOW_WIDTH)


/* client frame shadow widths */
#ifdef WSM
#define FRAME_EXTERNAL_SHADOW_WIDTH	(wmGD.frameExternalShadowWidth)
#else /* WSM */
#define FRAME_EXTERNAL_SHADOW_WIDTH	2
#endif /* WSM */
#define FRAME_INTERNAL_SHADOW_WIDTH	1
#define FRAME_CLIENT_SHADOW_WIDTH	1
#define FRAME_MATTE_SHADOW_WIDTH	1

/* padding around text in title bar */
#define WM_TOP_TITLE_PADDING	1
#define WM_BOTTOM_TITLE_PADDING	1
#define WM_TOP_TITLE_SHADOW	FRAME_INTERNAL_SHADOW_WIDTH
#define WM_BOTTOM_TITLE_SHADOW	FRAME_INTERNAL_SHADOW_WIDTH

#define WM_TITLE_BAR_PADDING	(WM_TOP_TITLE_PADDING \
				 +WM_BOTTOM_TITLE_PADDING \
				 +WM_TOP_TITLE_SHADOW \
				 +WM_BOTTOM_TITLE_SHADOW)

/* stretch directions  - (starts at NW and goes clockwise) */
#define STRETCH_NO_DIRECTION	-1
#define STRETCH_NORTH_WEST	0
#define STRETCH_NORTH		1
#define STRETCH_NORTH_EAST	2
#define STRETCH_EAST		3
#define STRETCH_SOUTH_EAST	4 
#define STRETCH_SOUTH 		5
#define STRETCH_SOUTH_WEST	6
#define STRETCH_WEST		7

#define STRETCH_COUNT		8

#ifdef WSM

/* Workspace allocation granularity */
#define WS_ALLOC_AMOUNT			8

/* Window list allocation granularity */
#define WINDOW_ALLOC_AMOUNT		16
#endif /* WSM */

/* function flag masks */
#define WM_FUNC_DEFAULT		MWM_FUNC_ALL
#define WM_FUNC_NONE		0
#define WM_FUNC_ALL		(MWM_FUNC_RESIZE | MWM_FUNC_MOVE |\
				 MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE |\
				 MWM_FUNC_CLOSE)

/* decorations flag masks */
#define WM_DECOR_DEFAULT	MWM_DECOR_ALL
#define WM_DECOR_NONE		0
#define WM_DECOR_BORDER		(MWM_DECOR_BORDER)
#define WM_DECOR_TITLE		(MWM_DECOR_TITLE)
#define WM_DECOR_SYSTEM		(WM_DECOR_TITLE | MWM_DECOR_MENU)
#define WM_DECOR_MINIMIZE	(WM_DECOR_TITLE | MWM_DECOR_MINIMIZE)
#define WM_DECOR_MAXIMIZE	(WM_DECOR_TITLE | MWM_DECOR_MAXIMIZE)
#define WM_DECOR_TITLEBAR	(WM_DECOR_SYSTEM | WM_DECOR_MINIMIZE |\
				 WM_DECOR_MAXIMIZE)
#define WM_DECOR_RESIZEH	(WM_DECOR_BORDER | MWM_DECOR_RESIZEH)
#define WM_DECOR_RESIZE		(WM_DECOR_RESIZEH)
#define WM_DECOR_ALL		(WM_DECOR_TITLEBAR | WM_DECOR_RESIZEH)

#ifdef PANELIST
#define WM_DECOR_PANEL_DEFAULT	WM_DECOR_BORDER
#endif /* PANELIST */

/* icon box definitions */
#define ICON_BOX_FUNCTIONS	(MWM_FUNC_RESIZE | MWM_FUNC_MOVE |\
				 MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE)

#ifdef PANELIST
/* accessory panel definitions */
#define WM_FUNC_PANEL_DEFAULT	(MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE)
#define WM_FUNC_SUBPANEL_DEFAULT (MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE|\
				MWM_FUNC_CLOSE)
#endif /* PANELIST */
#ifdef WSM
/* workspace controller definitions */
#define CONTROL_BOX_FUNCTIONS	(MWM_FUNC_MOVE)

/* workspace presence definitions */
#define PRESENCE_BOX_FUNCTIONS	(MWM_FUNC_MOVE)
#endif /* WSM */

/* show feedback definitions */
#define WM_SHOW_FB_BEHAVIOR	(1L << 0)
#define WM_SHOW_FB_MOVE		(1L << 1)
#define WM_SHOW_FB_PLACEMENT	(1L << 2)
#define WM_SHOW_FB_RESIZE	(1L << 3)
#define WM_SHOW_FB_RESTART	(1L << 4)
#define WM_SHOW_FB_QUIT         (1L << 5)
#define WM_SHOW_FB_KILL         (1L << 6)

#define WM_SHOW_FB_ALL		(WM_SHOW_FB_BEHAVIOR  | WM_SHOW_FB_MOVE    |\
				 WM_SHOW_FB_PLACEMENT | WM_SHOW_FB_RESIZE  |\
				 WM_SHOW_FB_RESTART   | WM_SHOW_FB_QUIT    |\
				 WM_SHOW_FB_KILL)

#define WM_SHOW_FB_NONE		0

#define WM_SHOW_FB_DEFAULT	WM_SHOW_FB_ALL

#ifdef WSM
/* flags identifying resources to save */

#define WM_RES_WORKSPACE_LIST		(1L << 0)
#define WM_RES_BACKDROP_IMAGE		(1L << 1)
#define WM_RES_WORKSPACE_TITLE		(1L << 2)
#define WM_RES_INITIAL_WORKSPACE	(1L << 3)
#define WM_RES_FP_POSITION      	(1L << 4)
#define WM_RES_ICONBOX_GEOMETRY      	(1L << 5)
#define WM_RES_WORKSPACE_COUNT		(1L << 6)

#endif /* WSM */



/*************************************<->*************************************
 *
 *  Miscellaneous utility window manager data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in resource processing, ...
 *
 *************************************<->***********************************/

typedef struct _WHSize
{
    int		width;
    int		height;

} WHSize;


typedef struct _AspectRatio
{
    int		x;
    int		y;

} AspectRatio;


typedef struct _WmColorData
{
    Screen *screen;
    Colormap colormap;
    Pixel background;
    Pixel foreground;
    Pixel top_shadow;
    Pixel bottom_shadow;

} WmColorData;

#ifdef WSM
typedef Atom WorkspaceID;
#endif


/*************************************<->*************************************
 *
 *  Event processing data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in saving button and key
 *  specifications that are used in processing events that are used to do 
 *  window manager functions (e.g., set the colormap focus).
 *
 *************************************<->***********************************/

typedef unsigned long Context;
typedef unsigned long Behavior;
typedef unsigned long GroupArg;

typedef Boolean (*WmFunction) ();

#define NO_MODIFIER	0		/* value for state field */

typedef struct _KeySpec
{
    unsigned int state;
    KeyCode      keycode;
    Context	 context;
    Context	 subContext;
    WmFunction	 wmFunction;
    String	 wmFuncArgs;
    struct _KeySpec *nextKeySpec;

} KeySpec;

typedef struct _ButtonSpec
{
    unsigned int state;
    unsigned int button;
    unsigned int eventType;
    Boolean	click;
    Context	context;
    Context	subContext;
    WmFunction	wmFunction;
    String	wmFuncArgs;
    struct _ButtonSpec *nextButtonSpec;

} ButtonSpec;


/*
 * Context field values:
 */

#define F_CONTEXT_NONE		0
#define F_CONTEXT_ROOT		(1L << 0)
#define F_CONTEXT_ICON		(1L << 1)
#define F_CONTEXT_NORMAL	(1L << 2)
#define F_CONTEXT_MAXIMIZE	(1L << 3)
#define F_CONTEXT_ICONBOX	(1L << 4)
#ifdef WSM
#define F_CONTEXT_IFKEY		(1L << 7)
#endif /* WSM */
#define F_CONTEXT_WINDOW	(F_CONTEXT_NORMAL|F_CONTEXT_MAXIMIZE)
#define F_CONTEXT_ALL		(F_CONTEXT_ROOT|F_CONTEXT_ICON|F_CONTEXT_WINDOW)


/*
 * context field mark for catching menu recursion 
 *   (tied to F_CONTEXT_... values):
 */

#define CR_MENU_MARK		(1L << 5)


/*
 * Part context defines for event processing.  The part context is used
 * to make a subcontext mask.
 */

/* window (frame and client) part contexts */
#define WINDOW_PART_NONE	0
#define FRAME_NONE		WINDOW_PART_NONE
#define FRAME_CLIENT		1
#define FRAME_SYSTEM		2
#define FRAME_TITLE		3
#define FRAME_MINIMIZE		4
#define FRAME_MAXIMIZE		5
#define FRAME_RESIZE_NW		6
#define FRAME_RESIZE_N		7
#define FRAME_RESIZE_NE		8
#define FRAME_RESIZE_E		9
#define FRAME_RESIZE_SE		10
#define FRAME_RESIZE_S 		11
#define FRAME_RESIZE_SW		12
#define FRAME_RESIZE_W		13
#define FRAME_NBORDER		14
#define FRAME_MATTE		15
#define FRAME_MISC		FRAME_MATTE
#ifdef WSM
#define FRAME_TITLEBAR		17
#endif /* WSM */

/* icon part contexts */
#define ICON_PART_NONE		0
#define ICON_PART_ALL		16

/* root part contexts */
#define ROOT_PART_NONE		0
#define ROOT_PART_ALL		17

/* iconbox part contexts */
#define ICONBOX_PART_NONE	0
#define ICONBOX_PART_IBOX	18
#define ICONBOX_PART_IICON	19
#define ICONBOX_PART_WICON	20


/*
 * Subcontext field values:
 */

#define F_SUBCONTEXT_NONE		(1L << WINDOW_PART_NONE)

#define F_SUBCONTEXT_I_ALL		(1L << ICON_PART_ALL)

#define F_SUBCONTEXT_R_ALL		(1L << ROOT_PART_ALL)


#define F_SUBCONTEXT_IB_IBOX		(1L << ICONBOX_PART_IBOX)
#define F_SUBCONTEXT_IB_IICON		(1L << ICONBOX_PART_IICON)
#define F_SUBCONTEXT_IB_WICON		(1L << ICONBOX_PART_WICON)

#define F_SUBCONTEXT_IB_ICONS		(F_SUBCONTEXT_IB_IICON |\
					 F_SUBCONTEXT_IB_WICON)

#define F_SUBCONTEXT_IB_ALL		(F_SUBCONTEXT_IB_IBOX |\
					 F_SUBCONTEXT_IB_IICON |\
					 F_SUBCONTEXT_IB_WICON)


#define F_SUBCONTEXT_W_CLIENT		(1L << FRAME_CLIENT)
#define F_SUBCONTEXT_W_APP		F_SUBCONTEXT_W_CLIENT
#define F_SUBCONTEXT_W_SYSTEM		(1L << FRAME_SYSTEM)
#define F_SUBCONTEXT_W_TITLE		(1L << FRAME_TITLE)
#define F_SUBCONTEXT_W_MINIMIZE		(1L << FRAME_MINIMIZE)
#define F_SUBCONTEXT_W_MAXIMIZE		(1L << FRAME_MAXIMIZE)
#define F_SUBCONTEXT_W_RESIZE_NW	(1L << FRAME_RESIZE_NW)
#define F_SUBCONTEXT_W_RESIZE_N		(1L << FRAME_RESIZE_N)
#define F_SUBCONTEXT_W_RESIZE_NE	(1L << FRAME_RESIZE_NE)
#define F_SUBCONTEXT_W_RESIZE_E		(1L << FRAME_RESIZE_E)
#define F_SUBCONTEXT_W_RESIZE_SE	(1L << FRAME_RESIZE_SE)
#define F_SUBCONTEXT_W_RESIZE_S		(1L << FRAME_RESIZE_S)
#define F_SUBCONTEXT_W_RESIZE_SW	(1L << FRAME_RESIZE_SW)
#define F_SUBCONTEXT_W_RESIZE_W		(1L << FRAME_RESIZE_W)
#define F_SUBCONTEXT_W_NBORDER		(1L << FRAME_NBORDER)
#define F_SUBCONTEXT_W_MATTE		(1L << FRAME_MATTE)
#define F_SUBCONTEXT_W_MISC		F_SUBCONTEXT_W_MATTE


#define F_SUBCONTEXT_W_RBORDER		(F_SUBCONTEXT_W_RESIZE_NW |\
					 F_SUBCONTEXT_W_RESIZE_N |\
					 F_SUBCONTEXT_W_RESIZE_NE |\
					 F_SUBCONTEXT_W_RESIZE_E |\
					 F_SUBCONTEXT_W_RESIZE_SE |\
					 F_SUBCONTEXT_W_RESIZE_S |\
					 F_SUBCONTEXT_W_RESIZE_SW |\
					 F_SUBCONTEXT_W_RESIZE_W)

#define F_SUBCONTEXT_W_BORDER		(F_SUBCONTEXT_W_RBORDER |\
					 F_SUBCONTEXT_W_NBORDER)

#define F_SUBCONTEXT_W_TITLEBAR		(F_SUBCONTEXT_W_SYSTEM |\
					 F_SUBCONTEXT_W_TITLE |\
					 F_SUBCONTEXT_W_MINIMIZE |\
					 F_SUBCONTEXT_W_MAXIMIZE)

#define F_SUBCONTEXT_W_FRAME		(F_SUBCONTEXT_W_BORDER |\
					 F_SUBCONTEXT_W_TITLEBAR)

#define F_SUBCONTEXT_W_ALL		(F_SUBCONTEXT_W_FRAME |\
					 F_SUBCONTEXT_W_MATTE |\
					 F_SUBCONTEXT_W_CLIENT)


/*
 * Click / double-click processing data:
 */

typedef struct _ClickData
{
    Boolean	clickPending;
    Boolean	doubleClickPending;
#ifdef WSM
    Boolean	bReplayed;
#endif /* WSM */
    unsigned int button;
    unsigned int state;
    unsigned int releaseState;
    struct _ClientData *pCD;
    Context	context;
    Context	subContext;
    Context	clickContext;
    Context	doubleClickContext;
    Time	time;

} ClickData;


/*
 * Frame part identification aids:
 */

typedef struct _Gadget_Rectangle
{
    short	id;
    XRectangle  rect;

} GadgetRectangle;


/*
 * Behavior function argument field values:
 */

#define F_BEHAVIOR_DEFAULT	(1L << 0)
#define F_BEHAVIOR_CUSTOM	(1L << 1)
#define F_BEHAVIOR_SWITCH	(1L << 2)


/*
 * Window/icon group function argument field values:
 */

#define F_GROUP_WINDOW		(1L << 0)
#define F_GROUP_ICON		(1L << 1)
#define F_GROUP_DEFAULT		(F_GROUP_WINDOW | F_GROUP_ICON)
#define F_GROUP_TRANSIENT	(1L << 2)
#define F_GROUP_ALL		(F_GROUP_DEFAULT | F_GROUP_TRANSIENT)
#define F_GROUP_GROUP		(1L << 3)

#ifdef WSM

/*************************************<->*************************************
 *
 *  Workspace data structures ...
 *
 ***************************************************************************/

/*
 * Specific data for workspacePresence dialog box
 */

typedef struct _WsPresenceData
{
    XmString		title;			/* resource */

    Widget		shellW;
    Widget		formW;
    Widget		windowLabelW;
    Widget		windowNameW;
    Widget		workspaceLabelW;
    Widget		workspaceScrolledListW;
    Widget		workspaceListW;
    Widget		allWsW;
    Widget		sepW;
    Widget		OkW;
    Widget		CancelW;
    Widget		HelpW;

    struct _ClientData 	*pCDforClient;
    Context 		contextForClient;

    Boolean 		*ItemSelected;		/* workspaces in list */
    XmStringTable 	ItemStrings;		/* workspace names */
    int 		currentWsItem;
    Boolean		onScreen;
    Boolean             userDismissed;
    int			numWorkspaces;

} WsPresenceData;

#define NUM_WSP_WIDGETS 	11

typedef struct _WsPresenceData *PtrWsPresenceData;


#ifdef PANELIST
/*
 * Specific data for top level help dialog 
 */

typedef struct _WsDtHelpData
{
    XmString		title;			/* resource */
    Widget		shell;
    Widget		dialog;
    Widget		errorDialog;
    Position            xPos;
    Position            yPos;
    Boolean             restored;
    Boolean		onScreen;
    Boolean             userDismissed;
    Boolean		bMapped;
    struct _ClientData 	*pCDforClient;
} WsDtHelpData;

typedef struct _WsDtHelpData *PtrWsDtHelpData;
#endif /*  PANELIST */


/*************************************<->*************************************
 *
 *  DtSessionItems
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _DtSessionItem
{
    Boolean                processed;
    int                    clientState;
    String                 workspaces;
    String                 clientMachine;
    String                 command;
    int                    commandArgc;
    char                   **commandArgv;
    struct _SessionGeom    *sessionGeom;
} DtSessionItem;

typedef struct _SessionGeom
{
    int          flags;
    int          clientX;
    int          clientY;
    int          clientWidth;
    int          clientHeight;
} SessionGeom;

/*      
 *  Status of Session Manager Contention Management
 */
#define  SM_UNITIALIZED			 0
#define  SM_START_ACK			 1
#define  SM_STOP_ACK                	 2

#endif /* WSM */


/*************************************<->*************************************
 *
 *  Menu specification data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are used in creating window manager menus that
 *  are specified using resource files.  A list of menu specifications
 *  (MenuSpec) is made when the resource files are parsed.  The desktop
 *  menu and system menus are created as needed using the menu specification
 *  list.
 *
 *************************************<->***********************************/

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*
 * Used to denote where the separators belong in a pair of separators
 * used to surround a client command.
 */

enum { TOP_SEPARATOR, BOTTOM_SEPARATOR };

/*
 * Used to denote what kind of change to make to a client command. 
 */
typedef enum { ENABLE, DISABLE, REMOVE, RENAME } CmdModifier;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

typedef struct _MenuItem
{
    int		 labelType;
    String	 label;
    int          labelBitmapIndex;
    KeySym	 mnemonic;
    unsigned int accelState;
    KeyCode      accelKeyCode;
    String	 accelText;
    WmFunction	 wmFunction;
    String	 wmFuncArgs;
    Context	 greyedContext;
    long         mgtMask;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    String       clientCommandName; /* as specified by the user in
				       his .mwmrc file. */
    CARD32	 clientCommandID;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    struct _MenuItem *nextMenuItem;

} MenuItem;

#ifdef WSM

/*
 * We use the top part of mgtMask for workspace function bits.
 * When OSF MWM outgrows the lower 16 bits, we'll have to
 * change how we do things.
 */

#define MWM_MGT_MASK	0x0000FFFF
#define DTWM_MGT_MASK	0xFFFF0000

#endif /* WSM */

typedef struct _MenuButton
{
    MenuItem	*menuItem;
    Widget	buttonWidget;
    Boolean     managed;

} MenuButton;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
typedef struct _MenuExclusion
{
  String                 command_string;
  struct _MenuExclusion *nextExclusion;
} MenuExclusion;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

typedef struct _MenuSpec
{
    String	  name;
    Context	  currentContext;
    Widget	  menuWidget;      /* RowColumn widget */
    unsigned int  whichButton;    /* tracks whichButton resource for top menu */
    unsigned int  height;          /* height of top menu */
    MenuItem	 *menuItems;       /* linked list of MenuItem structures */
    MenuButton   *menuButtons;     /* array of MenuButton structures */
    unsigned int  menuButtonSize;  /* size of menuButtons array */
    unsigned int  menuButtonCount; /* number of menuButtons elements in use */
    Context	  accelContext;    /* accelerator context */
    KeySpec	 *accelKeySpecs;   /* list of accelerator KeySpecs */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    MenuExclusion *exclusions;      /* list of client commands to be
				       excluded from this menu. */
    Boolean        clientLocal;     /* this menu is owned by a client and not 
				       shared with any other clients */
    CARD32         commandID;       /* if this is a client command, then this
				       its id value - globally unique. */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    struct _MenuSpec *nextMenuSpec;

} MenuSpec;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/* The range to which a client command operation should apply. */
typedef enum { SINGLE, ROOT, ALL } OpRange;

typedef struct _CmdTree {
  CARD32           commandID;   /* unique identifier for this command. */
  CARD32           notifyWindow;/* window to receive InvokeCommand request. */
  char            *name;        /* name of command refered to in .mwmrc. */
  char            *defaultName; /* default label of menu. */
  struct _CmdTree *subTrees;    /* list of child commands or command sets. */
  struct _CmdTree *next;

} CmdTree;


typedef struct _matchlist {
    MenuSpec          *menuspec;
    MenuItem          *menuitem;
    String             command_string;
    CmdTree           *treenode;
    WmFunction         function;
    String             funcargs;
    Context            greyed_context;
    struct _matchlist *next;

} MatchList;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  Window and function specification data structures ...
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _WindowItem
{
    String              window;
    struct _WindowItem *nextWindowItem;

} WindowItem;

typedef struct _WindowSet
{
    String             name;
    WindowItem        *windowItems;
    struct _WindowSet *nextWindowSet;

} WindowSet;

typedef struct _FunctionItem
{
    WmFunction		  wmFunction;
    String                wmFuncArgs;
    struct _FunctionItem *nextFunctionItem;

} FunctionItem;

typedef struct _FunctionSet
{
    String               name;
    FunctionItem        *functionItems;
    struct _FunctionSet *nextFunctionSet;

} FunctionSet;

/*************************************<->*************************************
 *
 *  Window manager timer data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This data stucture is used to keep track of window manager timers.  Each
 *  active timer has an instance of the data structure.
 *
 *************************************<->***********************************/

typedef struct _WmTimer
{
    XtIntervalId	timerId;
    struct _ClientData	*timerCD;
    unsigned int	timerType;
    struct _WmTimer	*nextWmTimer;

} WmTimer;

/* Timer types: */
#define TIMER_NONE		0
#define TIMER_QUIT		1
#define TIMER_RAISE		2


/*************************************<->*************************************
 *
 *  Window manager frame component data structures
 *
 *
 *  Description:
 *  -----------
 *  This data stucture is used for drawing frame component graphics.
 *
 *************************************<->***********************************/

typedef struct _RList
{
    int		allocated;		/* number of allocated XRectangles */
    int		used;			/* number currently in use */
    XRectangle	*prect;			/* array of XRectangles */
} RList;




/*************************************<->*************************************
 *
 *  Window manager component appearance data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This structure is used to hold component appearance data for client,
 *  icon, and feedback subparts. 
 * 
 *************************************<->***********************************/


typedef struct _AppearanceData
{
    XmFontList	fontList;			/* resource */
    XFontStruct	*font;
#ifndef NO_MULTIBYTE
    unsigned int	titleHeight;		/* title bar's height */
#endif
    Boolean	saveUnder;			/* resource */
    Pixel	background;			/* resource */
    Pixel	foreground;			/* resource */
    String	backgroundPStr;			/* resource */
    Pixmap	backgroundPixmap;
    Pixel	bottomShadowColor;		/* resource */
    String	bottomShadowPStr;		/* resource */
    Pixmap	bottomShadowPixmap;
    Pixel	topShadowColor;			/* resource */
    String	topShadowPStr;			/* resource */
    Pixmap	topShadowPixmap;
    Pixel	activeBackground;		/* resource */
    String	activeBackgroundPStr;		/* resource */
    Pixmap	activeBackgroundPixmap;
    Pixel	activeBottomShadowColor;	/* resource */
    String	activeBottomShadowPStr;		/* resource */
    Pixmap	activeBottomShadowPixmap;
    Pixel	activeForeground;		/* resource */
    Pixel	activeTopShadowColor;		/* resource */
    String	activeTopShadowPStr;		/* resource */
    Pixmap	activeTopShadowPixmap;

    GC		inactiveGC;
    GC		inactiveTopShadowGC;
    GC		inactiveBottomShadowGC;
    GC		activeGC;
    GC		activeTopShadowGC;
    GC		activeBottomShadowGC;

} AppearanceData;


typedef struct _AppearanceData *PtrAppearanceData;


/*************************************<->*************************************
 *
 *  IconInfo
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _IconInfo
{
        Widget theWidget;
        struct _ClientData *pCD;
} IconInfo;

typedef struct _IconInfo *PtrIconInfo;



/*************************************<->*************************************
 *
 *  IconPlacement
 *
 *
 *  Description:
 *  -----------
 *
 *************************************<->***********************************/

typedef struct _IconPlacementData
{
    Boolean	onRootWindow;		/* true if icons on root window */
    unsigned 	iconPlacement;		/* style of placement */
    int		placementRows;		/* number of rows in placement space */
    int		placementCols;		/* number of cols in placement space */
    int		totalPlaces;		/* total number of placment slots */
    int		iPlaceW;		/* width increment (to next place) */
    int		iPlaceH;		/* height increment (to next place) */
    IconInfo	*placeList;		/* list of icon info */
    int		placeIconX;
    int		placeIconY;
    int		*placementRowY;
    int		*placementColX;
} IconPlacementData;



/*************************************<->*************************************
 *
 *  IconBoxData
 *
 *
 *  Description:
 *  -----------
 *  This structure is used to hold window and widget information for
 *  each icon box.
 *
 *************************************<->***********************************/

typedef struct _IconBoxData
{
    Widget	shellWidget;
    Widget	frameWidget;
    Widget      scrolledWidget;
    Widget	vScrollBar;
    Widget	hScrollBar;
    Widget      bBoardWidget;
    Widget	clipWidget; 
    int		numberOfIcons;
    int		currentRow;
    int		currentCol;
    int		lastRow;
    int		lastCol;
#ifdef WSM
    WorkspaceID		wsID;		/* workspace identifier */
#endif /* WSM */
    struct _ClientData	*pCD_iconBox;	/* ptr to its own clientdata */
    struct _IconBoxData *pNextIconBox;	/* ptr to next icon box */
    struct _IconPlacementData IPD;	/* icon placement data */
} IconBoxData;

typedef struct _IconBoxData *PtrIconBoxData;

#define IB_SPACING		0
#define IB_MARGIN_HEIGHT	3 
#define IB_MARGIN_WIDTH		3 
#define IB_HIGHLIGHT_BORDER	3


/*************************************<->*************************************
 *
 *  Bitmap/Pixmap cache data
 *
 *
 *  Description:
 *  -----------
 *  These structures are used for the bitmap and pixmap caches held
 *  on a per-screen basis. (This is per-screen because you can't do
 *  XCopyPlane from one pixmap to another when they have different
 *  roots.)
 *
 *************************************<->***********************************/

#define NO_PIXMAP    0
#define LABEL_PIXMAP 1
#define ICON_PIXMAP  2

typedef struct _PixmapCache
{
   unsigned int  pixmapType;   /* icon or label? */
   Pixel         foreground;
   Pixel         background;
   Pixmap        pixmap;
   struct _PixmapCache *next;

} PixmapCache;

typedef struct _BitmapCache
{
   char         *path;
   Pixmap        bitmap;
   unsigned int  width;
   unsigned int  height;
   PixmapCache  *pixmapCache;

} BitmapCache;





/******************************<->*************************************
 *
 *  Client window list entry data structure ...
 *
 *
 *  Description:
 *  -----------
 *  This structure provides the data for an entry in the client window list.
 *  The client window list has an entry for each non-transient client
 *  window and each non-iconbox icon.
 * 
 ******************************<->***********************************/

typedef struct _ClientListEntry
{
    struct _ClientListEntry *nextSibling;
    struct _ClientListEntry *prevSibling;
    int		type;
    struct _ClientData *pCD;

} ClientListEntry;



/*************************************<->*************************************
 *
 *  Frame information
 *
 *
 *  Description:
 *  -----------
 *  This structure contains geometry information for the window manager 
 *  frame.
 * 
 *************************************<->***********************************/

typedef struct _FrameInfo
{
    int			x;
    int			y;
    unsigned int	width;
    unsigned int	height;
    unsigned int	upperBorderWidth;
    unsigned int	lowerBorderWidth;
    unsigned int	cornerWidth;
    unsigned int	cornerHeight;
    unsigned int	titleBarHeight;

} FrameInfo;



/*************************************<->*************************************
 *
 *  WmScreenData
 *
 *
 *  Description:
 *  -----------
 *  This is the data structure for holding the window manager's
 *  screen data. There is one instantiation of the structure for
 *  each screen.
 *
 *************************************<->***********************************/

typedef struct _WmScreenData
{
    int		dataType;
    int		screen;			/* number for this screen */
    Boolean	managed;
    Window	rootWindow;
    Widget	screenTopLevelW;
    Widget	screenTopLevelW1;       /* for internal WM components */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    Widget	utmShell;		/* DrawingArea used for UTM */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    Widget      confirmboxW[4];
#ifdef PANELIST
    Widget	wPanelist;		/* panel object */
#endif /* PANELIST */
#ifdef WSM
    WsPresenceData	presence;	/* workspace presence dialog*/
    Widget	switcherW;		/* workspace switcher */
    Widget	switcherShellW;		/* shell for ws switcher */
#endif /* WSM */
    Window	wmWorkspaceWin;		/* holds wm properties */
    Window	feedbackWin;
    Window	activeIconTextWin;
    Window	activeLabelParent;
    String	displayString;		/* used for putenv in F_Exec */
#ifdef WSM
    int displayResolutionType;
#endif /* WSM */
#ifdef PANELIST
    struct _WmFpEmbeddedClientData  *pECD; /* clients living in front panel */
    int		numEmbeddedClients;
    struct _WmFpPushRecallClientData  *pPRCD; /* push_recall clients */
    int		numPushRecallClients;
#endif /* PANELIST */

    /* wm state info: */

    unsigned long clientCounter;
    long	focusPriority;
    Window	inputScreenWindow;
    struct _ClientData	*colormapFocus;
    Colormap	workspaceColormap;
    Colormap	lastInstalledColormap;
    struct _WmWorkspaceData	*pActiveWS;	/* for this screen */
#ifdef WSM
    struct _WmWorkspaceData	*pLastWS;	/* previously active WS */
#endif /* WSM */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    CmdTree     *cciTree;               /* pointer to cci definitions */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    /* per screen caches */
    BitmapCache *bitmapCache;
    unsigned int bitmapCacheSize;
    unsigned int bitmapCacheCount;

    /* per screen icon info */
    Boolean	fadeNormalIcon;			/* resource */
    int		iconPlacement;			/* resource */
    int		iconPlacementMargin;		/* resource */
    int		iconDecoration;			/* resource */
    WHSize	iconImageMaximum;		/* resource */
    WHSize	iconImageMinimum;		/* resource */
    Pixmap	builtinIconPixmap;
    int		iconWidth;
    int		iconHeight;
    int		iconImageHeight;
    int		iconLabelHeight;
    GC		shrinkWrapGC;
    GC		fadeIconGC;
    GC		fadeIconTextGC;

#ifdef WSM
    
    /* per screen configuration outline windows */
    Window	woN;		/* North outline window */
    Window	woS;		/* South outline window */
    Window	woE;		/* East outline window */
    Window	woW;		/* West outline window */
#endif /* WSM */

    /* per screen feedback data */
    unsigned long fbStyle;
    unsigned int fbWinWidth;
    unsigned int fbWinHeight;
    char fbLocation[20];
    char fbSize[20];
    int fbLocX;
    int fbLocY;
    int fbSizeX;
    int fbSizeY;
    int fbLastX;
    int fbLastY;
    unsigned int fbLastWidth;
    unsigned int fbLastHeight;
    RList *fbTop;
    RList *fbBottom;
    int     actionNbr;

    /* resource description file data: */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    String	rootMenu;			/* resource */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    String	buttonBindings;			/* resource */
    ButtonSpec	*buttonSpecs;
    String	keyBindings;			/* resource */
    KeySpec	*keySpecs;
    MenuSpec   **acceleratorMenuSpecs;
    unsigned int acceleratorMenuCount;
    MenuSpec	*menuSpecs;

    Boolean	defaultSystemMenuUseBuiltin;

    Pixmap	defaultPixmap;
    GC		xorGC;

    /* per screen appearance resources */

    Boolean	cleanText;			/* resource */
    Boolean	decoupleTitleAppearance;	/* see clientTitleAppearance */
    int		frameBorderWidth;		/* resource */
    String	feedbackGeometry;		/* resource */
#ifndef WSM
    String	iconBoxGeometry;		/* resource */
#endif /* WSM */
    String	iconBoxName;			/* resource */
    String      iconBoxSBDisplayPolicy;         /* resource */
    int		iconBoxScheme;			/* resource - testing */
    XmString    iconBoxTitle;			/* resource */
    int		externalBevel;
    int		joinBevel;
    Boolean	limitResize;			/* resource */
    WHSize	maximumMaximumSize;		/* resource */
    int		resizeBorderWidth;		/* resource */
    Boolean	resizeCursors;			/* resource */
    int		transientDecoration;		/* resource */
    int		transientFunctions;		/* resource */
    Boolean	useIconBox;			/* resource */
#ifdef PANELIST
    int		subpanelDecoration;		/* resource */
    String      subpanelResources;              /*to restore subpanels */
    Boolean	iconBoxControl;			/* FP control for icon box */
#endif /* PANELIST */
    Boolean     moveOpaque;                     /* move window not outlines */

#ifdef WSM

    /* pixel set info (from color object) */
    XmPixelSet	*pPixelData;		/* all pixel data */
    XmPixelSet	*pActivePixelSet;	/* ptr into pPixelData */
    XmPixelSet	*pInactivePixelSet;	/* ptr into pPixelData */
    XmPixelSet	*pPrimaryPixelSet;	/* ptr into pPixelData */
    XmPixelSet	*pSecondaryPixelSet;	/* ptr into pPixelData */
    XmPixelSet	*pTextPixelSet;		/* ptr into pPixelData */
    int		colorUse;		/* indication from color obj */
#endif /* WSM */

    /* client frame component appearance resources and data: */

    AppearanceData clientAppearance;		/* resources ... */
    AppearanceData clientTitleAppearance;	/* resources ... */
    XPoint	transientOffset;
    int		Num_Resize_Ts_Elements;
    int		Num_Resize_Bs_Elements;
    int		Num_Title_Ts_Elements;
    int		Num_Title_Bs_Elements;

    /* icon component appearance resources and data: */

    AppearanceData iconAppearance;		/* resources ... */

    /* feedback component appearance resources and data: */

    AppearanceData feedbackAppearance;		/* resources ... */

    /* client list pointers: */

    ClientListEntry 	*clientList;
    ClientListEntry 	*lastClient;

#ifdef WSM
    /* DtSessionHints for clients */
    struct _DtSessionItem     *pDtSessionItems;
    int                        totalSessionItems;
    int                        remainingSessionItems;


    /* workspace list for this screen */
    String                      initialWorkspace; /* private resource */
    String			workspaceList;    /* resource */
    int				numWorkspaces;
    int				numWsDataAllocated;
    struct _WmWorkspaceData	*pWS;
    Window			lastBackdropWin;
    struct _WsDtHelpData        dtHelp;
    struct _CacheListStruct     *cachedHelp;
    String                      helpResources;  /* to restore help */
    DtSvcHandle	hWsm;		/* WORKSPACEMGR message handle */
    Boolean	bMarqueeSelectionInitialized;

#else /* WSM */
    /* workspace for this screen */

    struct _WmWorkspaceData	*pWS;
#endif /* WSM */

} WmScreenData;

typedef struct _WmScreenData *PtrScreenData;


/* 
 * Convenience macros for data access
 */
#define ROOT_FOR_CLIENT(pcd) ((pcd)->pSD->rootWindow)
#define SCREEN_FOR_CLIENT(pcd) ((pcd)->pSD->screen)
#define PSD_FOR_CLIENT(pcd) ((pcd)->pSD)
#define BUTTON_SPECS(pcd) ((pcd)->pSD->buttonSpecs)
#define KEY_SPECS(pcd) ((pcd)->pSD->keySpecs)
#define ACCELERATOR_MENU_COUNT(pcd) ((pcd)->pSD->acceleratorMenuCount)
#define ACCELERATOR_MENU_SPECS(pcd) ((pcd)->pSD->acceleratorMenuSpecs)
#define WORKSPACE_COLORMAP(pcd) ((pcd)->pSD->workspaceColormap)
#define FADE_NORMAL_ICON(pcd) ((pcd)->pSD->fadeNormalIcon)
/*
#define ICON_DEFAULT_TITLE(pcd) ((pcd)->iconDefaultTitle)
*/
#define ICON_DECORATION(pcd) ((pcd)->pSD->iconDecoration)
#define ICON_HEIGHT(pcd) ((pcd)->pSD->iconHeight)
#define ICON_WIDTH(pcd) ((pcd)->pSD->iconWidth)
#define ICON_IMAGE_HEIGHT(pcd) ((pcd)->pSD->iconImageHeight)
#define ICON_LABEL_HEIGHT(pcd) ((pcd)->pSD->iconLabelHeight)
#define ICON_IMAGE_MAXIMUM(pcd) ((pcd)->pSD->iconImageMaximum)
#define ICON_IMAGE_MINIMUM(pcd) ((pcd)->pSD->iconImageMinimum)
#define SHRINK_WRAP_GC(pcd) ((pcd)->pSD->shrinkWrapGC)
#define FADE_ICON_GC(pcd) ((pcd)->pSD->fadeIconGC)
#define FADE_ICON_TEXT_GC(pcd) ((pcd)->pSD->fadeIconTextGC)
#define DEFAULT_PIXMAP(pcd) ((pcd)->pSD->defaultPixmap)
#ifdef WSM
#define ICON_PLACE(pcd) ((pcd)->pWsList[(pcd)->currentWsc].iconPlace)
#define ICON_X(pcd) ((pcd)->pWsList[(pcd)->currentWsc].iconX)
#define ICON_Y(pcd) ((pcd)->pWsList[(pcd)->currentWsc].iconY)
#define P_ICON_BOX(pcd) ((pcd)->pWsList[(pcd)->currentWsc].pIconBox)
#define ICON_FRAME_WIN(pcd) ((pcd)->pWsList[(pcd)->currentWsc].iconFrameWin)
#else /* WSM */
#define ICON_PLACE(pcd) ((pcd)->iconPlace)
#define ICON_X(pcd) ((pcd)->iconX)
#define ICON_Y(pcd) ((pcd)->iconY)
#define P_ICON_BOX(pcd) ((pcd)->pIconBox)
#define ICON_FRAME_WIN(pcd) ((pcd)->iconFrameWin)
#endif /* WSM */

#ifdef WSM
/*
 * Definitions for Screen data
 */
#define MAX_WORKSPACE_COUNT	64



/*************************************<->*************************************
 *
 *  BackdropData
 *
 *
 *  Description:
 *  -----------
 *  This structure hold information for the workspace background
 * 
 *************************************<->***********************************/

typedef struct _WmBackdropData
{
    String		image;			/* resource */
    Atom		nameAtom;
    Pixmap		imagePixmap;
    int			colorSet;		/* resource */
    Pixel 		background;		/* resource */
    Pixel 		foreground;		/* resource */
    unsigned int	flags;
    Window		window;
} BackdropData;


/*
 * bit definiton for "flags" member of BackdropData
 */
#define BACKDROP_NONE		0
#define BACKDROP_CLIENT		(1L<<1)
#define BACKDROP_BITMAP		(1L<<2)
#define BACKDROP_IMAGE_ALLOCED	(1L<<3)	 /* image string can be freed */

typedef struct _WmBackdropData *PtrBackdropData;

#define DEFAULT_BACKDROP_DIR CDE_INSTALLATION_TOP "/backdrops"

#ifdef PANELIST
/*
 * direction for slide-out panels
 */
typedef enum _SlideDirection
{
    SLIDE_NOT, SLIDE_NORTH, SLIDE_EAST, SLIDE_SOUTH, SLIDE_WEST
} SlideDirection;

/* 
 * Slide out record for subpanels
 */
typedef struct _SlideOutRec 
{
    struct _ClientData	*pCD;
    Window		coverWin;
    Dimension		incWidth;
    Dimension		incHeight;
    Dimension		currWidth;
    Dimension		currHeight;
    Position		currX;
    Position		currY;
    unsigned int	interval;
    SlideDirection	direction;
    Boolean		mapping;
    Widget		wSubpanel;
} SlideOutRec;

/*
 * Data structure for arguments to f.action
 */
typedef struct _WmActionArg {
    String	  actionName;
    int		  numArgs;
    DtActionArg * aap;
    String	  szExecParms;
} WmActionArg;

#endif /* PANELIST */
#endif /* WSM */
 


/*************************************<->*************************************
 *
 *  WmWorkspaceData
 *
 *
 *  Description:
 *  -----------
 *  This is the structure for holding the workspace specific data. 
 *  (This has been broken out in anticipation of possible future 
 *  enhancements.)
 * 
 *************************************<->***********************************/

typedef struct _WmWorkspaceData
{
    int			dataType;

#ifdef WSM
    WorkspaceID		id;
    int         	map_state;
    BackdropData	backdrop;
    Widget		buttonW;
    XmString		title;		/* resource (visible name) */
    String	        iconBoxGeometry;/* resource */
#endif /* WSM */
    String		name;		/* workspace name */
					/* (used for resource fetching) */

    WmScreenData	*pSD;		/* screen data for this workspace */
    IconBoxData 	*pIconBox;	/* icon box data for this workspace */
    IconPlacementData 	IPData;

    Widget		workspaceTopLevelW;

    /* workspace state information */

    struct _ClientData	*keyboardFocus;	/* ptr to client with the key focus */
    struct _ClientData	*nextKeyboardFocus; /* next client to get focus */
#ifdef WSM
    struct _ClientData **ppClients;	/* list of client data ptrs */
    unsigned int	numClients; 	/* number of client in list */
    unsigned int	sizeClientList;	/* size of client list */
#endif  /* WSM */

} WmWorkspaceData;

typedef struct _WmWorkspaceData *PtrWorkspaceData;


/*
 * Convenience macros for data access
 */
#define CLIENT_APPEARANCE(pcd) ((pcd)->pSD->clientAppearance)
#define CLIENT_TITLE_APPEARANCE(pcd) ((pcd)->pSD->clientTitleAppearance)
#define DECOUPLE_TITLE_APPEARANCE(pcd) ((pcd)->pSD->decoupleTitleAppearance)
/*
#define CLIENT_DEFAULT_TITLE(pcd) ((pcd)->pSD->clientDefaultTitle)
*/
#define MAX_MAX_SIZE(pcd) ((pcd)->pSD->maximumMaximumSize)
#define SHOW_RESIZE_CURSORS(pcd) ((pcd)->pSD->resizeCursors)
#define JOIN_BEVEL(pcd) ((pcd)->pSD->joinBevel)
#define EXTERNAL_BEVEL(pcd) ((pcd)->pSD->externalBevel)
#define FRAME_BORDER_WIDTH(pcd) ((pcd)->pSD->frameBorderWidth)
#define RESIZE_BORDER_WIDTH(pcd) ((pcd)->pSD->resizeBorderWidth)
#define NUM_TITLE_TS_ELEMENTS(pcd) ((pcd)->pSD->Num_Title_Ts_Elements)
#define NUM_TITLE_BS_ELEMENTS(pcd) ((pcd)->pSD->Num_Title_Bs_Elements)
#define NUM_RESIZE_TS_ELEMENTS(pcd) ((pcd)->pSD->Num_Resize_Ts_Elements)
#define NUM_RESIZE_BS_ELEMENTS(pcd) ((pcd)->pSD->Num_Resize_Bs_Elements)
#define ICON_APPEARANCE(pcd) ((pcd)->pSD->iconAppearance)

#define ACTIVE_LABEL_PARENT(pcd) ((pcd)->pSD->activeLabelParent)

#ifndef WSM
#define ICON_BOX_GEOMETRY(pcd) ((pcd)->pSD->iconBoxGeometry)
#endif /* WSM */
#define ICON_BOX_TITLE(pcd) ((pcd)->pSD->iconBoxTitle)

#define TRANSIENT_DECORATION(pcd) ((pcd)->pSD->transientDecoration)
#define TRANSIENT_FUNCTIONS(pcd) ((pcd)->pSD->transientFunctions)


/*************************************<->*************************************
 *
 *  ClientData
 *
 *
 *  Description:
 *  -----------
 *  This data structure is instantiated for every client window that is
 *  managed by the window manager.  The information that is saved in the
 *  data structure is specific to the associated client window.
 *
 *  ClientData is instantiated at the time a client window is intially
 *  managed and is destroyed when the client window is withdrawn from
 *  window management (the ClientData may not be destroyed when a 
 *  client window is withdrawn if frame/icons are cached).
 *
 *************************************<->***********************************/

typedef struct _ClientData
{
    int		dataType;			/* client data type */

    Window	client;
    long	clientFlags;
    int		icccVersion;
    int		clientState;			/* WM_HINTS field */
    int		inputFocusModel;		/* WM_HINTS field */
    int		inputMode;
    long	focusPriority;
    unsigned long clientID;
    int		wmUnmapCount;
#ifdef PANELIST
    struct _WmFpEmbeddedClientData  *pECD; /* embedded client data */
    struct _WmFpPushRecallClientData  *pPRCD; /* embedded client data */
#endif /* PANELIST */
#ifdef WSM
    Atom *	paInitialProperties;	/* initial window properties */
    int		numInitialProperties;	/* number of initial properties */
#endif /* WSM */

    /* client supported protocols: */

    Atom	*clientProtocols;		/* WM_PROTOCOLS */
    int		clientProtocolCount;
    long	protocolFlags;
    long	*mwmMessages;			/* _MWM_MESSAGES */
    int		mwmMessagesCount;

    /* client colormap data: */

    Colormap	clientColormap;			/* selected client colormap */
    Window	*cmapWindows;			/* from WM_COLORMAP_WINDOWS */
    Colormap	*clientCmapList;
    int		clientCmapCount;		/* len of clientCmapList */
    int		clientCmapIndex;		/* current cmap in list */
#ifndef OLD_COLORMAP /* colormap */
    int		*clientCmapFlags;		/* installed, uninstalled */
    Bool	clientCmapFlagsInitialized;	/* Are clientCmapFlags valid? */
#endif

    /* associated window data: */

    ClientListEntry clientEntry;
    ClientListEntry iconEntry;
    XID		windowGroup;			/* WM_HINTS field */
#ifndef NO_OL_COMPAT
    Boolean	bPseudoTransient;		/* transientFor window group */
#endif /* NO_OL_COMPAT */
#ifndef WSM
    IconBoxData *pIconBox;			/* icon box for this win */
#endif /* WSM */
    IconBoxData *thisIconBox;			/* icon box data for self */
    						/*   if this is an icon box */
    Context    grabContext;                     /* used to support icon box */
                                                /* icon key, button, menus */
    Window	transientFor;			/* transient for another win */
    struct _ClientData *transientLeader;	/* trans leader of this win */
    struct _ClientData *transientChildren;	/* transients of this win */
    struct _ClientData *transientSiblings;	/* related transient win's */
#ifdef WSM
    int		primaryStackPosition;		/* stack pos'n of primary */
    Boolean	secondariesOnTop;		/* resource */
#endif /* WSM */
    int		primaryModalCount;		/* primary modal win count */
    int		fullModalCount;			/* full modal win count */

    /* client resource data */

    String	clientClass;			/* WM_CLASS field */
    String	clientName;			/* WM_CLASS field */
    int		clientDecoration;		/* resource */
    int		clientFunctions;		/* resource */
    Boolean	focusAutoRaise;			/* resource */
    Boolean	focusAutoRaiseDisabled;		/* to support f.lower */
    Boolean	focusAutoRaiseDisablePending;	/* to support f.lower */
    String	iconImage;			/* resource */
    Pixel	iconImageBackground;		/* resource */
    Pixel	iconImageBottomShadowColor;	/* resource */
    String	iconImageBottomShadowPStr;	/* resource */
    Pixmap	iconImageBottomShadowPixmap;
    Pixel	iconImageForeground;		/* resource */
    Pixel	iconImageTopShadowColor;	/* resource */
    String	iconImageTopShadowPStr;		/* resource */
    Pixmap	iconImageTopShadowPixmap;
    Boolean	ignoreWMSaveHints;		/* resource */
    int		internalBevel;			/* resource */
    Pixel	matteBackground;		/* resource */
    Pixel	matteBottomShadowColor;		/* resource */
    String	matteBottomShadowPStr;		/* resource */
    Pixmap	matteBottomShadowPixmap;
    Pixel	matteForeground;		/* resource */
    Pixel	matteTopShadowColor;		/* resource */
    String	matteTopShadowPStr;		/* resource */
    Pixmap	matteTopShadowPixmap;
    int		matteWidth;			/* resource */
    WHSize	maximumClientSize;		/* resource */
    String	smClientID;			/* SM_CLIENT_ID */
    String	systemMenu;			/* resource */
    MenuItem    *mwmMenuItems;			/* custom menu items or NULL */
    MenuSpec	*systemMenuSpec;
    Boolean	useClientIcon;			/* resource */
    int		wmSaveHintFlags;		/* WMSAVE_HINT */

    /* client frame data: */

    long	sizeFlags;			/* WM_NORMAL_HINTS field */
    long	decor;				/* client decoration*/
    long	decorFlags;			/* depressed gadgets flags */
    int		minWidth;			/* WM_NORMAL_HINTS field */
    int		minHeight;			/* WM_NORMAL_HINTS field */
    Boolean	maxConfig;			/* True => use max config */
    int		maxX;				/* maximized window X loc */
    int		maxY;				/* maximized window Y loc */
    int		maxWidthLimit;
    int		maxWidth;			/* WM_NORMAL_HINTS field */
    int		maxHeightLimit;
    int		maxHeight;			/* WM_NORMAL_HINTS field */
    int		oldMaxWidth;			/* for good HPterm behavior */
    int		oldMaxHeight;			/* for good HPterm behavior */
    int		widthInc;			/* WM_NORMAL_HINTS field */
    int		heightInc;			/* WM_NORMAL_HINTS field */
    AspectRatio	minAspect;			/* WM_NORMAL_HINTS field */
    AspectRatio	maxAspect;			/* WM_NORMAL_HINTS field */
    int		baseWidth;			/* WM_NORMAL_HINTS field */
    int		baseHeight;			/* WM_NORMAL_HINTS field */
    int		windowGravity;			/* WM_NORMAL_HINTS field */
    int		clientX;			/* normal window X loc */
    int		clientY;			/* normal window Y loc */
    int		clientWidth;			/* normal window width */
    int		clientHeight;			/* normal window height */
    XPoint	clientOffset;			/* frame to client window */
    XmString	clientTitle;			/* WM_NAME field */
    Window	clientFrameWin;			/* top-level, frame window */
    Window	clientStretchWin[STRETCH_COUNT];/* for resizing border */
    Window	clientTitleWin;			/* for title bar */
    Window	clientBaseWin;			/* for matte & reparenting */
    int		xBorderWidth;			/* original X border width */
    FrameInfo	frameInfo;			/* frame geometry data */

    /* client window frame graphic data: */

    RList	*pclientTopShadows;		/* top shadow areas */
    RList	*pclientBottomShadows;		/* bottom shadow areas */

    RList	*pclientTitleTopShadows;	/* top shadow areas */
    RList	*pclientTitleBottomShadows;	/* bottom shadow areas */

    RList	*pclientMatteTopShadows;	/* matte top shadows */
    RList	*pclientMatteBottomShadows;	/* matte bottom shadows */

    /* rectangles for client frame gadgets */

    XRectangle		titleRectangle;		/* title bar area */
    GadgetRectangle	*pTitleGadgets;		/* title bar gadgets */
    int			cTitleGadgets;		/* count of title rects */
    GadgetRectangle	*pResizeGadgets;	/* resize areas */
    XRectangle		matteRectangle;		/* matte / base window area */

    /* client appearance data: */

    GC		clientMatteTopShadowGC;
    GC		clientMatteBottomShadowGC;
    WmScreenData	*pSD;			/* where visuals come from */

    /* icon data: */

    long	iconFlags;
    XmString	iconTitle;			/* WM_ICON_NAME field */
#ifndef WSM
    int		iconX;				/* WM_HINTS field */
    int		iconY;				/* WM_HINTS field */
    int		iconPlace;
    Window	iconFrameWin;
#endif /* WSM */
    Pixmap	iconPixmap;			/* WM_HINTS field */
    Pixmap	iconMask;			/* WM_HINTS field */
    Window	iconWindow;			/* WM_HINTS field */

    RList	*piconTopShadows;		/* these change to 	*/
    						/* to reflect the 	*/
    RList	*piconBottomShadows;		/* depressed icon image */

#ifdef WSM
    /* workspace data */

    int         absentMapBehavior;              /* resource */
    int		numInhabited;		/* number of WS's inhabited */
    int		sizeWsList;		/* size of wsc data list */
    struct _WsClientData *pWsList;	/* list of workspace-client data */
    int		currentWsc;		/* index to current wsc data */
    WorkspaceID	*pWorkspaceHints;	/* _DT_WORKSPACE_HINTS */
    int		numWorkspaceHints;	/* size of pWorkspaceHints */
    Boolean	putInAll;		/* persistent window flag */
    long	dtwmFunctions;		/* _DT_WM_HINTS */
    long	dtwmBehaviors;		/* _DT_WM_HINTS */	
    Window	attachWindow;		/* _DT_WM_HINTS */
#ifdef PANELIST
    SlideDirection	slideDirection;	/* slide-up direction */
    SlideOutRec	*pSOR;			/* slide-out record */
#endif /* PANELIST */
#endif /* WSM */
#ifndef NO_SHAPE
    short       wShaped;                /* this window has a bounding shape */
#endif /* NO_SHAPE  */

    int		usePPosition;		/* indicate whether to use PPosition */

    long	window_status;			/* used for Tear-off Menus */

} ClientData;

typedef struct _ClientData *PtrClientData;

/* client data convenience macros */

#define IS_APP_MODALIZED(pcd) \
    (((pcd)->primaryModalCount)||((pcd)->fullModalCount))

#define IS_MAXIMIZE_VERTICAL(pcd) \
  ((pcd->maximumClientSize.height == BIGSIZE) && \
   (pcd->maximumClientSize.width == 0))

#define IS_MAXIMIZE_HORIZONTAL(pcd) \
  ((pcd->maximumClientSize.width == BIGSIZE) && \
   (pcd->maximumClientSize.height == 0))

/* window management state of client windows (clientState): */
#define WITHDRAWN_STATE		0
#define NORMAL_STATE		1
#define MINIMIZED_STATE		2
#define MAXIMIZED_STATE		3
#ifdef WSM
#define UNSEEN_STATE            8
#endif /* WSM */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# define NO_CHANGE              -1
# define UNSET                   0
# define SET                     1
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

/* clientFlags field values: */
#define CLIENT_HINTS_TITLE		(1L << 0)
#define CLIENT_REPARENTED		(1L << 1)
#define CLIENT_TRANSIENT		(1L << 2)
#define CLIENT_CONTEXT_SAVED		(1L << 3)
#define CLIENT_IN_SAVE_SET		(1L << 4)
#define USERS_MAX_POSITION		(1L << 5)
#define WM_INITIALIZATION		(1L << 6)
#define CLIENT_DESTROYED		(1L << 7)
#define ICON_REPARENTED			(1L << 8)
#define ICON_IN_SAVE_SET		(1L << 9)
#define CLIENT_TERMINATING		(1L << 10)
#define ICON_BOX                        (1L << 11)  /* one of our icon boxes */
#define CONFIRM_BOX                     (1L << 12)  /* a confirmation box */

#ifdef PANELIST
#define FRONT_PANEL_BOX                 (1L << 14)  /* a DT 3.0 front panel */
#define GOT_DT_WM_HINTS		(1L << 15)
#endif /* PANELIST */
#ifdef WSM
#define SM_LAUNCHED                     (1L << 17) /* launched by dtsession */
#endif /* WSM */

#define SM_X                     	(1L << 18) /* X from DB/dtsession */
#define SM_Y                     	(1L << 19) /* Y from DB/dtsession */
#define SM_WIDTH                     	(1L << 20) /* width fm DB/dtsession */
#define SM_HEIGHT                    	(1L << 21) /* height fm DB/dtsession */
#define SM_CLIENT_STATE                	(1L << 22) /* clientState fm DB/dtsession */
#define SM_ICON_X                       (1L << 23) /* icon X from DB */
#define SM_ICON_Y                       (1L << 24) /* icon Y from DB */

#define CLIENT_WM_CLIENTS		(ICON_BOX | CONFIRM_BOX)

/* decorFlags bit fields */
#define SYSTEM_DEPRESSED		(1L << 0)
#define TITLE_DEPRESSED			(1L << 1)
#define MINIMIZE_DEPRESSED		(1L << 2)
#define MAXIMIZE_DEPRESSED		(1L << 3)

/* iconFlags field values: */
#define ICON_HINTS_POSITION		(1L << 0)
#define ICON_HINTS_TITLE		(1L << 1)
#define ICON_HINTS_PIXMAP		(1L << 2)

/* client protocol flags and sizes: */
#define PROTOCOL_WM_SAVE_YOURSELF	(1L << 0)
#define PROTOCOL_WM_DELETE_WINDOW	(1L << 1)
#define PROTOCOL_WM_TAKE_FOCUS		(1L << 2)
#define PROTOCOL_MWM_MESSAGES		(1L << 3)
#define PROTOCOL_MWM_OFFSET		(1L << 4)

#define MAX_CLIENT_PROTOCOL_COUNT	40
#define MAX_COLORMAP_WINDOWS_COUNT	40
#define MAX_MWM_MESSAGES_COUNT		40

/* bevel width limits between window manager frame and client window */
#define MIN_INTERNAL_BEVEL		0
#define MAX_INTERNAL_BEVEL		2

/* global return buffer */
#define MAXWMPATH				1023
#define MAXBUF				(MAXWMPATH+1)

#define PIXMAP_IS_VALID(pix) \
  ((pix) != XmUNSPECIFIED_PIXMAP && (pix) != None && (pix) != (Pixmap)NULL)
  
#ifdef WSM

/*************************************<->*************************************
 *
 *  WsClientData
 *
 *
 *  Description:
 *  -----------
 *  This datum compartmentalizes client data that must be replicated 
 *  on a per workspace basis.
 * 
 *************************************<->***********************************/
typedef struct _WsClientData
{
    WorkspaceID	wsID;			/* workspace identifier */
    int		iconPlace;		/* icon placment index */
    int		iconX;
    int		iconY;
    Window	iconFrameWin;
    IconBoxData *pIconBox;		/* icon box for this win */

} WsClientData;

#endif /* WSM */

/*
 * frame style types
 */
typedef enum _FrameStyle
{
    WmRECESSED,
    WmSLAB
} FrameStyle;


/*************************************<->*************************************
 *
 *  WmGlobalData
 *
 *
 *  Description:
 *  -----------
 *  This is the main data structure for holding the window manager's
 *  global data. There is one instantiation of the structure for
 *  the window manager.
 * 
 *************************************<->***********************************/

typedef struct _WmGlobalData
{
    int		dataType;
    char	**argv;			/* command line argument vector */
    char	**environ;		/* environment vector */	
    char	*mwmName;		/* name of our executable */
    Widget	topLevelW;
#ifdef WSM
    Widget	topLevelW1;             /* from which WM components hang */
#endif /* WSM */
    Boolean     confirmDialogMapped;    /* confirm dialog is mapped */
    XtAppContext	mwmAppContext;	/* application context for mwm */
    XContext	windowContextType;	/* window context for XSaveContext */
    XContext	screenContextType;	/* screen context for XSaveContext */
#ifndef	IBM_169380
    XContext  cmapWindowContextType;  /* list of pCD's in WM_COLORMAP_WINDOWS                                              context for XSaveContext */
#endif
#ifdef WSM
    XContext	mwmWindowContextType;	/* mwm win context for XSaveContext */
    Window      dtSmWindow;            /* used for contention management */
    Window      commandWindow;          /* WM_SAVE_YOURSELF win for dtwm */
#endif /* WSM */

    /* presentation resource id's: */

    String	displayString;		/* used for putenv in F_Exec */
    Display	*display;		/* display we are running to */
#ifdef WSM
    Display	*display1;		/* second display connection */
    int         statusColorServer;      /* CSERVE_NORMAL, CSERVE_NOT_AVAILABLE */
    DtWmpParseBuf	*pWmPB;		/* global parse buffer */
#endif /* WSM */
    int		numScreens;		/* number of screens */
    unsigned char	**screenNames;	/* default names for screens */
    WmScreenData	*Screens;	/* array of screen info */

    Cursor	workspaceCursor;		/* basic arrow cursor */
    Cursor	stretchCursors[STRETCH_COUNT];
    Cursor	configCursor;
    Cursor	movePlacementCursor;
    Cursor	sizePlacementCursor;

#ifndef NO_MESSAGE_CATALOG
    XmString okLabel;
    XmString cancelLabel;
    XmString helpLabel;
#endif


    /* wm state info: */

    WmScreenData *pActiveSD;		/* with keyfocus window */
    Boolean	useStandardBehavior;	/* behavior flag */
    Boolean	wmRestarted;		/* restart flag */
    Boolean	errorFlag;		/* handle on async errors */
    XID		errorResource;		/* from XErrorEvent */
    unsigned char errorRequestCode;	/* from XErrorEvent */

		    /* The following are global because pointer is grabbed */
    MenuSpec	*menuActive;		/* ptr to currently active menu */
    KeySpec	*menuUnpostKeySpec;	/* key to upost current menu */
    ClientData	*menuClient;		/* client for which menu is posted */
    KeySpec     *F_NextKeySpec;         /* used when travering to windows */
    KeySpec     *F_PrevKeySpec;         /* used when travering to windows */

    Context     grabContext;            /* used in GrabWin when no event */

    ClickData	clickData;		/* double-click detection data */
    int		configAction;		/* none, resize, move */
    unsigned int configButton;		/* button used for config */
    unsigned int configPart;		/* resize frame part */
    Boolean 	configSet;		/* True if configPart set */
    Boolean	movingIcon;		/* True if icon being moved */
    Boolean	preMove;		/* move threshold support */
    int		preMoveX;
    int		preMoveY;
    ClientData	*gadgetClient;		/* last client with */
    int		gadgetDepressed;	/* depressed gadget */
    Boolean	checkHotspot;		/* event hotspot flag */
    XRectangle	hotspotRectangle;	/* event hotspot area */
    WmTimer	*wmTimers;		/* timer data */
    Boolean	passKeysActive;		/* flag for pass keys function */
    KeySpec	*passKeysKeySpec;	/* key for pass keys function */
    Boolean	activeIconTextDisplayed;	/* True if active label up */
    Boolean	queryScreen;		/* True if not sure of active screen */

    /* keyboard focus data: */

    ClientData	*keyboardFocus;		/* ptr to client with the key focus */
    ClientData	*nextKeyboardFocus;	/* next client to get focus */
    Boolean	systemModalActive;
    ClientData	*systemModalClient;
    Window	systemModalWindow;

    /* Resource database used to restore client geometries, etc. */
    XrmDatabase clientResourceDB;
#ifndef WSM
    char	*dbFileName;
    String	sessionClientDB;		/* resource */
#endif

    /* atoms used in inter-client communication: */

    Atom	xa_WM_STATE;
    Atom	xa_WM_PROTOCOLS;
    Atom	xa_WM_CHANGE_STATE;
    Atom	xa_WM_SAVE_YOURSELF;
    Atom	xa_WM_DELETE_WINDOW;
    Atom	xa_WM_TAKE_FOCUS;
    Atom	xa_WM_COLORMAP_WINDOWS;
    Atom	xa_MWM_HINTS;
    Atom	xa_MWM_MESSAGES;
    Atom	xa_MWM_MENU;
    Atom	xa_MWM_INFO;
    Atom	xa_MWM_OFFSET;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    Atom       *xa_WM;

    Atom	xa_TARGETS;
    Atom	xa_MULTIPLE;
    Atom	xa_TIMESTAMP;

    /* original set of query targets */
    Atom	_MOTIF_WM_CLIENT_WINDOW;
    Atom	_MOTIF_WM_POINTER_WINDOW;
    Atom	_MOTIF_WM_ALL_CLIENTS	;
	  
    /* menu command interface support */
    Atom	_MOTIF_WM_DEFINE_COMMAND;
    Atom	_MOTIF_WM_INCLUDE_COMMAND;
    Atom	_MOTIF_WM_REMOVE_COMMAND;
    Atom	_MOTIF_WM_ENABLE_COMMAND;
    Atom	_MOTIF_WM_DISABLE_COMMAND;
    Atom	_MOTIF_WM_RENAME_COMMAND;
    Atom	_MOTIF_WM_INVOKE_COMMAND;
    Atom	_MOTIF_WM_REQUEST_COMMAND;
    Atom	_MOTIF_WM_WINDOW_FLAGS;

    /* automation support */
    Atom        _MOTIF_WM_AUTOMATION;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    Atom	xa_MOTIF_BINDINGS;
    Atom	xa_COMPOUND_TEXT;
    Atom	xa_SM_CLIENT_ID;
    Atom	xa_WMSAVE_HINT;


#ifdef WSM

    /* atoms used for workspace management: */

    Atom	xa_DT_WORKSPACE_HINTS;
    Atom	xa_DT_WORKSPACE_PRESENCE;
    Atom	xa_DT_WORKSPACE_INFO;
    Atom	xa_DT_EMBEDDED_CLIENTS;
    Atom	xa_DT_WORKSPACE_LIST;
    Atom	xa_DT_WORKSPACE_CURRENT;

    Atom	xa_ALL_WORKSPACES;
    Atom        xa_DT_SESSION_HINTS;
    Atom	xa_DT_WM_REQUEST;

    Atom        xa_DT_SM_WM_PROTOCOL;
    Atom        xa_DT_SM_START_ACK_WINDOWS;
    Atom        xa_DT_SM_STOP_ACK_WINDOWS;
    Atom        xa_DT_WM_WINDOW_ACK;
    Atom        xa_DT_WM_EXIT_SESSION;
    Atom        xa_DT_WM_LOCK_DISPLAY;
    Atom        xa_DT_WM_READY;
#endif /* WSM */
#ifndef NO_OL_COMPAT
    Atom	xa_OL_WIN_ATTR;
    Atom	xa_OL_DECOR_RESIZE;
    Atom	xa_OL_DECOR_HEADER;
    Atom	xa_OL_DECOR_CLOSE;
    Atom	xa_OL_DECOR_PIN;
    Atom	xa_OL_DECOR_ADD;
    Atom	xa_OL_DECOR_DEL;
    Atom	xa_OL_WT_BASE;
    Atom	xa_OL_WT_COMMAND;
    Atom	xa_OL_WT_HELP;
    Atom	xa_OL_WT_NOTICE;
    Atom	xa_OL_WT_OTHER;
    Atom	xa_OL_PIN_IN;
    Atom	xa_OL_PIN_OUT;
    Atom	xa_OL_MENU_LIMITED;
    Atom	xa_OL_MENU_FULL;
#endif /* NO_OL_COMPAT */

    /* mwm specific appearance and behavior resources and data: */

    Boolean	autoKeyFocus;			/* resource */
    int		autoRaiseDelay;			/* resource */
    String	bitmapDirectory;		/* resource */
#ifdef WSM
    String	backdropDirs;			/* resource */
#endif /* WSM */
    Boolean	clientAutoPlace;		/* resource */
    int		colormapFocusPolicy;		/* resource */
    String	configFile;			/* resource */
#ifdef WSM
    String	cppCommand;			/* resource */
#endif /* WSM */
    Boolean	deiconifyKeyFocus;		/* resource */
    int		doubleClickTime;		/* resource */
    Boolean	enableWarp;			/* resource */
    Boolean	enforceKeyFocus;		/* resource */
    Boolean	freezeOnConfig;			/* resource - testing */
#ifdef WSM
    Boolean	useWindowOutline;		/* resource */
#endif /* WSM */
    Boolean	iconAutoPlace;			/* resource */
    Boolean	iconClick;			/* resource */
    Boolean	interactivePlacement;		/* resource */
    int		keyboardFocusPolicy;		/* resource */
    Boolean	lowerOnIconify;			/* resource */
    int		moveThreshold;			/* resource */
    Boolean     passButtonsCheck; /* used to override passButtons */
    Boolean	passButtons;			/* resource */
    Boolean	passSelectButton;		/* resource */
    Boolean	positionIsFrame;		/* resource */
    Boolean	positionOnScreen;		/* resource */
    int		quitTimeout;			/* resource */
    Boolean     raiseKeyFocus;                  /* resource */
    Boolean     multiScreen;                  	/* resource */
    String	screenList;			/* resource */
    int		showFeedback;			/* resource */
#ifdef WSM
    Boolean	refreshByClearing;		/* resource */
    Boolean	rootButtonClick;		/* resource */
#endif /* WSM */
    Boolean	startupKeyFocus;		/* resource */
    Boolean	systemButtonClick;		/* resource */
    Boolean	systemButtonClick2;		/* resource */
    Boolean	useLargeCursors;
#if  defined(PANELIST)
    Boolean	useFrontPanel;			/* resource */
#endif /* PANELIST */
#ifdef WSM
    String      helpDirectory;		        /* resource */
    Window	requestContextWin;		/* for WmRequest f.fcns */
#endif /* WSM */
#ifdef MINIMAL_DT
    Boolean     dtLite;                        /* resource */
    Boolean     blinkOnExec;                    /* resource */
#endif /* MINIMAL_DT */
#ifdef PANELIST
    WmScreenData *dtSD; /* screen for front panel */
    int         iSlideUpsInProgress;
#endif /*PANELIST  */
    Boolean	waitForClicks;			/* resource */
    FrameStyle	frameStyle;			/* resource */
#ifdef WSM
    Dimension	iconExternalShadowWidth;	/* resource */
    Dimension	frameExternalShadowWidth;	/* resource */
    int		marqueeSelectGranularity;	/* resource */
    XButtonEvent evLastButton;			/* for detecting replayed 
						   button events */
    Boolean	bReplayedButton;		/* true if button replayed */
    Boolean	bSuspendSecondaryRestack;	/* overrides transient
						   stacking */
#endif /* WSM */

    XmString	clientDefaultTitle;
    XmString	iconDefaultTitle;

    Window	attributesWindow;
    XWindowAttributes	windowAttributes;

#ifndef NO_SHAPE
    Boolean     hasShape;                /* server supports Shape extension */
    int         shapeEventBase, shapeErrorBase;
#endif /* NO_SHAPE */
    /* Need to replay enter notify events on windows with the
       pointer that used to be modalized.  This is for pointer focus. */
    int         replayEnterEvent;
    XEnterWindowEvent savedEnterEvent;

    unsigned int lockingModMask;	/* mask of locking modifier keys */
    unsigned int *pLockMaskSequence;	

    unsigned char tmpBuffer[MAXBUF];	/* replaces static buffers used */
					/* for large return values */

    int numMouseButtons;		/* num of mouse buttons available */
    unsigned int bMenuButton;		/* BMenu binding (button/state) */
#if defined(sun) && defined(ALLPLANES)
    Bool	allplanes;		/* is SUN_ALLPLANES available? */
#endif /* defined(sun) && defined(ALLPLANES) */
} WmGlobalData;

/* quick references to global data: */
#define DISPLAY		wmGD.display
#ifdef WSM
#define DISPLAY1	wmGD.display1
#endif /* WSM */
#define ACTIVE_PSD	(wmGD.pActiveSD)
#define ACTIVE_SCREEN	(wmGD.pActiveSD->screen)
#define ACTIVE_WS	(wmGD.pActiveSD->pActiveWS)
#define ACTIVE_ROOT	(wmGD.pActiveSD->rootWindow)
#define ACTIVE_ICON_TEXT_WIN (wmGD.pActiveSD->activeIconTextWin)

#define NOLOCKMOD(state)  ((state) & ~wmGD.lockingModMask)
#ifdef WSM
/* absent map behavior policy values (absentMapBehavior): */
#define AMAP_BEHAVIOR_ADD       0
#define AMAP_BEHAVIOR_MOVE      1
#define AMAP_BEHAVIOR_IGNORE    2 
#endif /* WSM */

/* colormap focus policy values (colormapFocus): */
#define CMAP_FOCUS_EXPLICIT	0
#define CMAP_FOCUS_POINTER	1
#define CMAP_FOCUS_KEYBOARD	2

/* keyboard input focus policy values (keyboardFocus): */
#define KEYBOARD_FOCUS_EXPLICIT	0
#define KEYBOARD_FOCUS_POINTER	1

/* icon appearance values (iconAppearance): */
#define ICON_LABEL_PART			(1L << 0)
#define ICON_IMAGE_PART			(1L << 1)
#define ICON_ACTIVE_LABEL_PART		(1L << 2)
#define USE_ICON_DEFAULT_APPEARANCE	(1L << 3)
#define ICON_APPEARANCE_STANDALONE	(ICON_LABEL_PART | ICON_IMAGE_PART |\
					 ICON_ACTIVE_LABEL_PART)
#define ICON_APPEARANCE_ICONBOX		(ICON_LABEL_PART | ICON_IMAGE_PART)

/* icon placement values (iconPlacement, ...): */
#define ICON_PLACE_LEFT_PRIMARY		(1L << 0)
#define ICON_PLACE_RIGHT_PRIMARY	(1L << 1)
#define ICON_PLACE_TOP_PRIMARY		(1L << 2)
#define ICON_PLACE_BOTTOM_PRIMARY	(1L << 3)
#define ICON_PLACE_LEFT_SECONDARY	(1L << 4)
#define ICON_PLACE_RIGHT_SECONDARY	(1L << 5)
#define ICON_PLACE_TOP_SECONDARY	(1L << 6)
#define ICON_PLACE_BOTTOM_SECONDARY	(1L << 7)
#define ICON_PLACE_EDGE			(1L << 8)
#define ICON_PLACE_TIGHT		(1L << 9)
#define ICON_PLACE_RESERVE		(1L << 10)

#define NO_ICON_PLACE			-1
#define MINIMUM_ICON_SPACING		4
#define MAXIMUM_ICON_MARGIN		128
#define ICON_IMAGE_MAX_WIDTH		128
#define ICON_IMAGE_MAX_HEIGHT		128
#define ICON_IMAGE_MIN_WIDTH		16
#define ICON_IMAGE_MIN_HEIGHT		16

/*default client window title: */
#define DEFAULT_CLIENT_TITLE	"*****"
#define DEFAULT_ICON_TITLE	DEFAULT_CLIENT_TITLE

/* client decoration parameters */
#define MAXIMUM_FRAME_BORDER_WIDTH	64

/* configuration action (configAction): */
#define NO_ACTION			0
#define MOVE_CLIENT			1
#define RESIZE_CLIENT			2
#define PLACE_CLIENT			3
#ifdef WSM
#define MARQUEE_SELECT			4
#endif /* WSM */

/* Motif input bindings file name */
#define MOTIF_BINDINGS_FILE		".motifbind"

/* Data type definitions */
#define GLOBAL_DATA_TYPE		1001
#define CLIENT_DATA_TYPE		1002
#define SCREEN_DATA_TYPE		1003
#define WORKSPACE_DATA_TYPE		1004

#ifndef NO_MESSAGE_CATALOG
/*************************************<->*************************************
 *
 *  NlsStrings
 *
 *
 *  Description:
 *  -----------
 *  This structure is used to hold message strings that used to
 *  be defines
 *
 *************************************<->***********************************/

typedef struct _NlsStrings
{
    char *default_icon_box_title;
    char *builtinSystemMenu;
    char *defaultKeyBindings;
    char *builtinKeyBindings;
    char *defaultButtonBindings;
    char *defaultVersionTitle;
    char *defaultDtwmHelpTitle;
    char *defaultHelpTitle;
} NlsStrings;


extern NlsStrings wmNLS;

#endif

/* Stacking functions */
#define STACK_NORMAL			0
#define STACK_WITHIN_FAMILY		1
#define STACK_FREE_FAMILY		2

/* UsePPosition values */
#define USE_PPOSITION_OFF		0
#define USE_PPOSITION_ON		1
#define USE_PPOSITION_NONZERO		2

/* Largest dimension for special casing */
#define BIGSIZE 32767

/*
 * External references for global data:
 */

extern WmGlobalData	wmGD;
extern char	defaultSystemMenuName[];
extern char	defaultKeyBindings[];
extern char	defaultKeyBindingsName[];
#ifndef NO_MESSAGE_CATALOG
extern char	*builtinSystemMenu;
#else
extern char	builtinSystemMenu[];
#endif
extern char	builtinKeyBindings[];

extern Const char	_75_foreground[];
extern Const char	_50_foreground[];
extern Const char	_25_foreground[];


extern char *_DtGetMessage(char *filename, int set, int n, char *s);

/*
 * macro to get message catalog strings
 */
#ifndef NO_MESSAGE_CATALOG
# ifdef __ultrix
#  define _CLIENT_CAT_NAME "dtwm.cat"
# else  /* __ultrix */
#  define _CLIENT_CAT_NAME "dtwm"
# endif /* __ultrix */
# ifdef WSM
#  define GETMESSAGE(set, number, string)\
    _DtGetMessage(_CLIENT_CAT_NAME, set, number, string)
# else
#  define GETMESSAGE(set, number, string) (string)
# endif /* WSM */
#else
# define GETMESSAGE(set, number, string)\
    string
#endif
#endif /* _WmGlobal_h */
