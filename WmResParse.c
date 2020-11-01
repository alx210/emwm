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
static char rcsid[] = "$XConsortium: WmResParse.c /main/9 1996/11/01 10:17:34 drk $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
/*
 * (c) Copyright 1987, 1988 DIGITAL EQUIPMENT CORPORATION */
/*
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#ifdef WSM
#include <Dt/UserMsg.h>
#include <Dt/Connect.h>
#include <Tt/tt_c.h>
#endif /* WSM */
#ifdef PANELIST
#include "WmParse.h"
#include "WmParseP.h"
#include "WmPanelP.h"
#endif /* PANELIST */
#include "WmResource.h"

#include <Xm/VirtKeysP.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <ctype.h>

#include <X11/Xlocale.h>

#ifndef NO_MULTIBYTE
#include <stdlib.h>
#endif

#ifdef MOTIF_ONE_DOT_ONE
#include <stdio.h>
#include <pwd.h>
#else
#include <Xm/XmP.h>             /* for XmeGetHomeDirName */
#endif
#ifdef WSM
#include <signal.h>
#endif /* WSM */

#define FIX_1127

/* maximum string lengths */

#define MAX_KEYSYM_STRLEN    100
#define MAX_EVENTTYPE_STRLEN  20
#define MAX_MODIFIER_STRLEN   20
#define MAX_CONTEXT_STRLEN    20
#define MAX_GROUP_STRLEN      20

#ifdef min
#undef min
#endif
#define min(a,b)	((a)>(b) ? (b) : (a))

#define MAXLINE     (MAXWMPATH+1)

#define MBBSIZ	    4096

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# define PARSE_MENU_ITEMS(pSD, mSpec) ParseMenuItems(pSD, mSpec)
#else
# define PARSE_MENU_ITEMS(pSD, mSpec) ParseMenuItems(pSD)
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

/*
 * include extern functions
 */
#include "WmResParse.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */
#include "WmError.h"
#include "WmFunction.h"
#include "WmImage.h"
#include "WmXSMP.h"

#ifdef MOTIF_ONE_DOT_ONE
extern char   *getenv ();
#endif
#ifdef PANELIST
# include <errno.h>
# ifdef X_NOT_STDC_ENV
extern int errno;
# endif
# define HOME_DT_WMRC    "/.dt/dtwmrc"
# define LANG_DT_WMRC    "/dtwmrc"
# define SYS_DT_WMRC     CDE_CONFIGURATION_TOP "/sys.dtwmrc"
#endif /* PANELIST */

/*
 * Global Variables And Tables:
 */
static char cfileName[MAXWMPATH+1];
#ifdef WSM
#ifndef NO_MESSAGE_CATALOG
char * pWarningStringFile;
char * pWarningStringLine;
#else
char pWarningStringFile[] = "%s: %s on line %d of configuration file %s\n";
char pWarningStringLine[] = "%s: %s on line %d of specification string\n";
#endif
#define cfileP 	(wmGD.pWmPB->pFile)
#define parseP	(wmGD.pWmPB->pchNext)
#define line	(wmGD.pWmPB->pchLine)
#define linec	(wmGD.pWmPB->lineNumber)
#else  /* WSM */
static FILE *cfileP = NULL;   /* fopen'ed configuration file or NULL */
static unsigned char  line[MAXLINE+1]; /* line buffer */
static int   linec = 0;       /* line counter for parser */
static unsigned char *parseP = NULL;   /* pointer to parse string */
#endif /* WSM */


typedef struct {
   char         *name;
   unsigned int  mask;
} MaskTableEntry;

static MaskTableEntry modifierStrings[] = {

    {"none",    None},
    {"ctrl",	ControlMask},
    {"shift",	ShiftMask},
    {"alt",	Mod1Mask},
    {"meta",	Mod1Mask},
    {"lock",	LockMask},
    {"mod1",	Mod1Mask},
    {"mod2",	Mod2Mask},
    {"mod3",	Mod3Mask},
    {"mod4",	Mod4Mask},
    {"mod5",	Mod5Mask},
    {NULL,      (unsigned int)NULL},
};

#define ALT_INDEX 3
#define META_INDEX 4

typedef struct {
   char         *event;
   unsigned int  eventType;
   Boolean       (*parseProc)();
   unsigned int  closure;
   Boolean       fClick;
} EventTableEntry;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))

# define CCI_USE_DEFAULT_NAME_TAG "DEFAULT_NAME"

String CCIEntryModifierNames[] = {
  "none",
  "inline",
  "cascade",
  "delimit",
  "delimit_inline",
  "delimit_cascade",
  "exclude"
};

typedef enum {
  NONE,             /* internal only. */
  INLINE,           /* not supported. */
  CASCADE,
  DELIMIT,
  DELIMIT_INLINE,   /* not supported. */
  DELIMIT_CASCADE,
  EXCLUDE
} CCIEntryModifier;

typedef struct _CCIFuncArg {
  CCIEntryModifier mod;
  String           cciEntry;
} CCIFuncArg;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#ifdef MOTIF_ONE_DOT_ONE
void GetHomeDirName(String  fileName);
#endif
#ifdef WSM
static String GetNetworkFileName (char *pchFile);
#endif /* WSM */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
static MenuItem	       *MakeSeparatorTemplate  (int);
static void ParseMenuItemName (unsigned char **linePP, MenuItem *menuItem);
static Boolean ParseClientCommand (unsigned char **linePP, MenuSpec *menuSpec,
				   MenuItem *menuItem, unsigned char *string,
				   Boolean *use_separators);
static void FixMenuItem (MenuSpec *menuSpec, MenuItem *menuItem);
static Boolean GetCCIModifier (String modString, CCIEntryModifier *mod);
static Boolean ParseWmFuncCCIArgs (unsigned char **linePP, 
				   WmFunction wmFunction, String *pArgs);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
FILE *FopenConfigFile (void);
void SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
static void ParseMenuSet (WmScreenData *pSD, unsigned char *lineP);
MenuItem *ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
static MenuItem *ParseMenuItems (WmScreenData *pSD
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
				 , MenuSpec *menuSpec
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
				);
static Boolean ParseWmLabel (WmScreenData *pSD, MenuItem *menuItem, 
			     unsigned char *string);
static void ParseWmMnemonic (unsigned char **linePP, MenuItem *menuItem);
static Boolean ParseWmAccelerator (unsigned char **linePP, MenuItem *menuItem);
int ParseWmFunction (unsigned char **linePP, unsigned int res_spec, 
			    WmFunction *pWmFunction);
#ifndef PANELIST
static Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
#endif /* PANELIST */
static Boolean ParseWmFuncNoArg (unsigned char **linePP, WmFunction wmFunction,
				 String *pArgs);
#ifndef PANELIST
static Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
#endif /* PANELIST */
void FreeMenuItem (MenuItem *menuItem);
static Boolean ParseWmFuncGrpArg (unsigned char **linePP, 
				  WmFunction wmFunction, GroupArg *pGroup);
static Boolean ParseWmFuncNbrArg (unsigned char **linePP, 
				  WmFunction wmFunction, 
				  unsigned long *pNumber);
void ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
static void ParseButtonSet (WmScreenData *pSD, unsigned char *lineP);
static Boolean ParseContext (unsigned char **linePP, Context *context, 
			     Context *subContext);
void
ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
static void ParseKeySet (WmScreenData *pSD, unsigned char *lineP);
Boolean ParseBtnEvent (unsigned char  **linePP,
		       unsigned int *eventType,
		       unsigned int *button,
		       unsigned int *state,
		       Boolean      *fClick);
Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
static Boolean ParseEvent (unsigned char **linePP, EventTableEntry *table,
			   unsigned int *eventType, unsigned int *detail,
			   unsigned int *state, Boolean *fClick);
static Boolean ParseModifiers(unsigned char **linePP, unsigned int *state);
static Boolean LookupModifier (unsigned char *name, unsigned int *valueP);
static Boolean ParseEventType (unsigned char **linePP, EventTableEntry *table,
			       unsigned int *eventType, Cardinal *ix);
static Boolean ParseImmed (unsigned char **linePP, unsigned int closure,
			   unsigned int  *detail);
static Boolean ParseKeySym (unsigned char **linePP, unsigned int closure,
			    unsigned int *detail);
static unsigned int StrToNum(unsigned char *str);
static unsigned int StrToHex(unsigned char *str);
static unsigned int StrToOct(unsigned char *str);
void ScanAlphanumeric (unsigned char **linePP);
void ScanWhitespace(unsigned char  **linePP);
void ToLower (unsigned char  *string);
void
PWarning (char *message);
static void ProcessAccelText (unsigned char *startP, unsigned char *endP,
			      unsigned char *destP);
void ProcessCommandLine (int argc,  char *argv[]);
static void ParseScreensArgument (int argc, char *argv[], int *pArgnum,
				  unsigned char *lineP);
void ProcessMotifBindings (void);
#ifdef PANELIST
static void ParseIncludeSet (WmScreenData *pSD, unsigned char *lineP);
static void ConfigStackInit (char *pchFileName);
static FILE *ConfigStackPush (unsigned char *pchFileName);
static void ConfigStackPop (void);
Boolean ParseWmFuncActionArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
static void PreprocessConfigFile (void);
#endif /* PANELIST */

static EventTableEntry buttonEvents[] = {

    {"btn1down",    ButtonPress,    ParseImmed,    SELECT_BUTTON,  FALSE},
    {"btn1up",      ButtonRelease,  ParseImmed,    SELECT_BUTTON,  FALSE},
    {"btn1click",   ButtonRelease,  ParseImmed,    SELECT_BUTTON,  TRUE},
    {"btn1click2",  ButtonPress,    ParseImmed,    SELECT_BUTTON,  TRUE},
    {"btn2down",    ButtonPress,    ParseImmed,    DMANIP_BUTTON,  FALSE},
    {"btn2up",      ButtonRelease,  ParseImmed,    DMANIP_BUTTON,  FALSE},
    {"btn2click",   ButtonRelease,  ParseImmed,    DMANIP_BUTTON,  TRUE},
    {"btn2click2",  ButtonPress,    ParseImmed,    DMANIP_BUTTON,  TRUE},
    {"btn3down",    ButtonPress,    ParseImmed,    BMENU_BUTTON,  FALSE},
    {"btn3up",      ButtonRelease,  ParseImmed,    BMENU_BUTTON,  FALSE},
    {"btn3click",   ButtonRelease,  ParseImmed,    BMENU_BUTTON,  TRUE},
    {"btn3click2",  ButtonPress,    ParseImmed,    BMENU_BUTTON,  TRUE},
    {"btn4down",    ButtonPress,    ParseImmed,    Button4,  FALSE},
    {"btn4up",      ButtonRelease,  ParseImmed,    Button4,  FALSE},
    {"btn4click",   ButtonRelease,  ParseImmed,    Button4,  TRUE},
    {"btn4click2",  ButtonPress,    ParseImmed,    Button4,  TRUE},
    {"btn5down",    ButtonPress,    ParseImmed,    Button5,  FALSE},
    {"btn5up",      ButtonRelease,  ParseImmed,    Button5,  FALSE},
    {"btn5click",   ButtonRelease,  ParseImmed,    Button5,  TRUE},
    {"btn5click2",  ButtonPress,    ParseImmed,    Button5,  TRUE},
    { NULL, (unsigned int)NULL, (Boolean(*)())NULL, (unsigned int)NULL, (Boolean)NULL}
};


static EventTableEntry keyEvents[] = {

    {"key",         KeyPress,    ParseKeySym,    0,  FALSE},
    { NULL, (unsigned int)NULL, (Boolean(*)())NULL, (unsigned int)NULL, (Boolean)NULL}
};

#ifdef PANELIST
typedef struct _ConfigFileStackEntry {
    char		*fileName;
    char 		*tempName;
    char 		*cppName;
    char		*wmgdConfigFile;
    long		offset;
    DtWmpParseBuf	*pWmPB;
    struct _ConfigFileStackEntry	*pIncluder;
    
} ConfigFileStackEntry;

static ConfigFileStackEntry *pConfigStack = NULL;
static ConfigFileStackEntry *pConfigStackTop = NULL;

#endif /* PANELIST */

unsigned int buttonModifierMasks[] = {
    0,
    SELECT_BUTTON_MASK,
    DMANIP_BUTTON_MASK,
    BMENU_BUTTON_MASK,
    Button4Mask,
    Button5Mask
};

/*
 * FUNCTION PARSER TABLE (function names must be in alphabetic order)
 */

typedef struct {
   char         * funcName;
   Context        greyedContext;
   unsigned int   resource;
   long           mgtMask;
   WmFunction     wmFunction;
   Boolean       (*parseProc)();
} FunctionTableEntry;


/*
 * NOTE: New functions MUST be added in ALPHABETICAL order.  A binary search
 *       is used to find the correct function name.
 */

FunctionTableEntry functionTable[] = {
#ifdef WSM
#ifdef PANELIST
    {"f.action",	0,
			CRS_ANY,
			0,
			F_Action,
			ParseWmFuncActionArg},
#else /* PANELIST */
    {"f.action",	0,
			CRS_ANY,
			0,
			F_Action,
			ParseWmFuncStrArg},
#endif  /* PANELIST */
#endif /* WSM */
    {"f.beep",		0,
			CRS_ANY,
			0,
			F_Beep,
			ParseWmFuncNoArg},
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    {"f.cci",		0,
	        	CRS_ANY,
			0,
			F_Nop,
			ParseWmFuncCCIArgs},
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    {"f.circle_down",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Down,
			ParseWmFuncGrpArg},
    {"f.circle_up",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Up,
			ParseWmFuncGrpArg},
#ifdef WSM
    {"f.create_workspace", 0,
			CRS_ANY,
			0,
			F_CreateWorkspace,
			ParseWmFuncNoArg},
    {"f.delete_workspace", 0,
			CRS_ANY,
			0,
			F_DeleteWorkspace,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.exec",		0,
			CRS_ANY,
			0,
			F_Exec,
			ParseWmFuncStrArg},
    {"f.focus_color",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Color,
			ParseWmFuncNoArg},
    {"f.focus_key",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Key,
			ParseWmFuncNoArg},
#ifdef WSM
    {"f.goto_workspace", 0,
			CRS_ANY,
			0,
			F_Goto_Workspace,
			ParseWmFuncStrArg},
#endif /* WSM */
#ifdef WSM
    {"f.help",          0,
			CRS_ANY,
			0,
			F_Help,
			ParseWmFuncStrArg},  /* [helpvolume && helptopic] */
    {"f.help_mode",     0,
			CRS_ANY,
			0,
			F_Help_Mode,
			ParseWmFuncNoArg},  /* for now */
#endif /* WSM */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    {"f.invoke_command",
	        	0, CRS_ANY,
			0,
			F_InvokeCommand,
			ParseWmFuncStrArg},
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    {"f.kill",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_CLOSE,
			F_Kill,
			ParseWmFuncNoArg},
    {"f.lower",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Lower,
                        ParseWmFuncMaybeStrArg},
#ifdef WSM
    {"f.marquee_selection",	
			F_CONTEXT_WINDOW|F_CONTEXT_ICON|F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			0,
			F_Marquee_Selection,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.maximize",	F_CONTEXT_ROOT|F_CONTEXT_MAXIMIZE|
	                               F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_MAXIMIZE,
			F_Maximize,
			ParseWmFuncNoArg},
    {"f.menu",		0,
			CRS_ANY,
			0,
			F_Menu,
			ParseWmFuncStrArg},
    {"f.minimize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			MWM_FUNC_MINIMIZE,
			F_Minimize,
			ParseWmFuncNoArg},
    {"f.move",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_MOVE,
			F_Move,
			ParseWmFuncNoArg},
    {"f.next_cmap",	0,
			CRS_ANY,
			0,
			F_Next_Cmap,
			ParseWmFuncNoArg},
    {"f.next_key",	0,
			CRS_ANY,
			0,
			F_Next_Key,
			ParseWmFuncGrpArg},
#ifdef WSM
    {"f.next_workspace",	0,
			CRS_ANY,
			0,
			F_Next_Workspace,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.nop",	        F_CONTEXT_ROOT|F_CONTEXT_ICON|F_CONTEXT_WINDOW|
	                    F_SUBCONTEXT_IB_WICON | F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			0,
			F_Nop,
			ParseWmFuncNoArg},
    {"f.normalize",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Normalize,
			ParseWmFuncNoArg},
#ifdef PANELIST
    {"f.normalize_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Normalize_And_Raise,
			ParseWmFuncMaybeStrArg},
#else /* PANELIST */
    {"f.normalize_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Normalize_And_Raise,
			ParseWmFuncNoArg},
#endif /* PANELIST */
#ifdef WSM
    {"f.occupy_all", F_CONTEXT_ICONBOX|F_CONTEXT_ROOT,
			CRS_ANY,
                        DtWM_FUNC_OCCUPY_WS,
 			F_AddToAllWorkspaces,
                        ParseWmFuncNoArg},
#endif /* WSM */
    {"f.pack_icons",	0,
			CRS_ANY,
			0,
			F_Pack_Icons,
			ParseWmFuncNoArg},
    {"f.pass_keys",	0,
			CRS_ANY,
			0,
			F_Pass_Key,
			ParseWmFuncNoArg},
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    {"f.post_rmenu",	0,
	        	CRS_KEY,
			0,
			F_Post_RMenu,
			ParseWmFuncNoArg},
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    {"f.post_wmenu",	0,
			CRS_BUTTON|CRS_KEY,
			0,
			F_Post_SMenu,
			ParseWmFuncNoArg},
    {"f.prev_cmap",	0,
			CRS_ANY,
			0,
			F_Prev_Cmap,
			ParseWmFuncNoArg},
    {"f.prev_key",	0,
			CRS_ANY,
			0,
			F_Prev_Key,
			ParseWmFuncGrpArg},
#ifdef WSM
    {"f.prev_workspace",	0,
			CRS_ANY,
			0,
			F_Prev_Workspace,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.quit_mwm",	F_CONTEXT_ICON|F_CONTEXT_WINDOW,
			CRS_ANY,
			0,
			F_Quit_Mwm,
			ParseWmFuncNoArg},
    {"f.raise",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise,
                        ParseWmFuncMaybeStrArg},
    {"f.raise_lower",	F_CONTEXT_ROOT |
    				F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise_Lower,
			ParseWmFuncNoArg},
    {"f.refresh",	0,
			CRS_ANY,
			0,
			F_Refresh,
			ParseWmFuncNoArg},
    {"f.refresh_win",	F_CONTEXT_ICON|F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Refresh_Win,
			ParseWmFuncNoArg},
#ifdef WSM
    {"f.remove",	F_CONTEXT_ROOT,
			CRS_ANY,
			DtWM_FUNC_OCCUPY_WS,
			F_Remove,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.resize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|
                                 F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_RESIZE,
			F_Resize,
			ParseWmFuncNoArg},
#ifdef WSM
    {"f.restart",	F_CONTEXT_ICON|F_CONTEXT_WINDOW,
			CRS_ANY,
			0,
			F_Restart,
			ParseWmFuncStrArg},
#else /* WSM */
    {"f.restart",	F_CONTEXT_ICON|F_CONTEXT_WINDOW,
			CRS_ANY,
			0,
			F_Restart,
			ParseWmFuncNoArg},
#endif /* WSM */
    {"f.restore",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Restore,
			ParseWmFuncNoArg},
    {"f.restore_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Restore_And_Raise,
			ParseWmFuncNoArg},
    {"f.screen",	0,
			CRS_ANY,
			0,
			F_Screen,
			ParseWmFuncStrArg},
    {"f.send_msg",	F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Send_Msg,
			ParseWmFuncNbrArg},
    {"f.separator",	0,
			CRS_MENU,
			0,
			F_Separator,
			ParseWmFuncNoArg},
    {"f.set_behavior",	0,
			CRS_ANY,
			0,
			F_Set_Behavior,
			ParseWmFuncNoArg},
#ifdef WSM
    {"f.set_context",	0,
			CRS_ANY,
			0,
			F_Set_Context,
			ParseWmFuncNbrArg},
#endif /* WSM */
    {"f.title",		0,
			CRS_MENU,
			0,
			F_Title,
			ParseWmFuncNoArg},
#if defined(PANELIST)
    {"f.toggle_frontpanel", 0,
			CRS_ANY,
			0,
			F_Toggle_Front_Panel,
			ParseWmFuncNoArg},

    {"f.version",       0,
			CRS_ANY,
			0,
			F_Version,
			ParseWmFuncNoArg},
#endif /* PANELIST */
#ifdef WSM

#ifdef OLD
    {"f.workspace_presence",F_CONTEXT_ICON|F_CONTEXT_ROOT|F_CONTEXT_ICONBOX|
                            F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
#endif /* OLD */
    {"f.workspace_presence", F_CONTEXT_ROOT|F_CONTEXT_ICONBOX|
                        F_SUBCONTEXT_IB_WICON,
 			CRS_ANY,
			DtWM_FUNC_OCCUPY_WS,
 			F_Workspace_Presence,
                 	ParseWmFuncNoArg},
#endif /* WSM */
#if defined(DEBUG) && defined(WSM)
    {"f.zz_debug",	0,
			CRS_ANY,
			0,
			F_ZZ_Debug,
                        ParseWmFuncStrArg},
#endif /* DEBUG */
};

/*
 * NOTE: New functions MUST be added in ALPHABETICAL order.  A binary search
 *       is used to find the correct function name.
 */

#define WMFUNCTIONTABLESIZE (sizeof(functionTable)/sizeof(functionTable[0]))

/*
 * Be sure to update these define, whenever adding/deleting a function.
 */
#ifdef WSM
# if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
#  define F_CCI_INDEX  2
# endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
int F_ACTION_INDEX;
int F_EXEC_INDEX;
int F_NOP_INDEX;
#else  /* WSM */
# if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
#  define F_CCI_INDEX  1
#  define F_EXEC_INDEX 4
#  define F_NOP_INDEX 16
# else
#  define F_EXEC_INDEX 3
#  define F_NOP_INDEX 14
# endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
#endif /* WSM */
#ifdef WSM

/******************************<->*************************************
 *
 *  void GetFunctionTableValues (int *execIndex, int *nopIndex, 
 *                               int *actionIndex)
 *
 *  Description:
 *  -----------
 *  This routine dynamically computes the size of the functionTable[],
 *  and the indices for key functions, such as f.exec, f.action, and
 *  f.nop
 *
 *  Inputs:
 *  ------
 *  We are setting the values of F_EXEC_INDEX, F_ACTION_INDEX, 
 *  and F_NOP_INDEX on a global level.  The addresses
 *  for same are passed in.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  This routine calls smaller routines for efficiency sake.
 *
 ******************************<->***********************************/
void
GetFunctionTableValues (int *execIndex, int *nopIndex,
		    int *actionIndex)
{

	GetExecIndex (WMFUNCTIONTABLESIZE, execIndex);

	GetActionIndex (WMFUNCTIONTABLESIZE, actionIndex);

	GetNopIndex (WMFUNCTIONTABLESIZE, nopIndex);

} /* END OF FUNCTION GetFunctionTableValues */



/******************************<->*************************************
 *

 *
 *  Description:
 *  -----------

 *
 *  Inputs:
 *  ------

 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/

void
GetExecIndex (int tableSize, int *execIndex)
{
	int i;

	for (i = 0; i < (tableSize); i++)
	{
		if (!(strcmp ("f.exec", functionTable[i].funcName)))
		{
			*execIndex = i;
			return;
		}
		else
		{
			*execIndex = 0;
		}
	}
} /* END OF FUNCTION GetExecIndex */


/******************************<->*************************************
 *

 *
 *  Description:
 *  -----------

 *
 *  Inputs:
 *  ------

 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/


void
GetActionIndex (int tableSize, int *actionIndex)
{
	int i;

	for (i = 0; i < (tableSize); i++)
	{
		if (!(strcmp ("f.action", functionTable[i].funcName)))
		{
			*actionIndex = i;
			return;
		}
		else
		{
			*actionIndex = 0;
		}
	}
} /* END OF FUNCTION GetActionIndex */



/******************************<->*************************************
 *

 * 
 *  Description:
 *  -----------

 *
 *  Inputs:
 *  ------

 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
void
GetNopIndex (int tableSize, int *nopIndex)
{
	int i;

	for (i = 0; i < (tableSize); i++)
	{
		if (!(strcmp ("f.nop", functionTable[i].funcName)))
		{
			*nopIndex = i;
			return;
		}
		else
		{
			*nopIndex = 0;
		}
	}
} /* END OF FUNCTION GetNopIndex */




/*************************************<->*************************************
 *
 *  void
 *  WmDtGetHelpArgs ()
 *
 *
 *  Description:
 *  -----------
 *  Get Help Args

 *
 *  Inputs:
 *  ------

 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/
void
WmDtGetHelpArgs(char *args, 
		 unsigned char* volume, 
		 unsigned char* topic, 
		 int *argsCount)
{
    unsigned char *string;
    unsigned char *lineP;

    cfileP = NULL;
    linec = 0;
    parseP = (unsigned char*) args;

    if(GetNextLine () != NULL)
    {
	*argsCount = 0;
	lineP = line;
	if ((string = GetSmartSMString (&lineP)) != NULL)
	{
	    *argsCount = *argsCount + 1;
	    strcpy ((char*)topic, (char*)string);
	}

	if ((string = GetSmartSMString (&lineP)) != NULL)
	{
	    *argsCount = *argsCount + 1;
	    strcpy ((char*)volume, (char *)string);
	}
    }

} /* END OF FUNCTION WmDtGetHelpArgs */





/******************************<->*************************************
 *
 *  void
 *  ParseDtSessionHints (pSD, property)
 *
 *
 *  Description:
 *  -----------
 *  This function parses a DtSessionHints string and returns a list of 
 *  DtSessionItems array.  The string should have the syntax:
 *

 *
 *
 *  Inputs:
 *  ------
 *  line = (global) line buffer
 *  pSD->rootWindow = default root window of display
 *
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void 
ParseDtSessionHints (WmScreenData *pSD, unsigned char *property)
{

    cfileP = NULL;
    linec = 0;
    parseP = property;

    ParseSessionItems (pSD);

} /* END OF FUNCTION ParseDtSessionHints */


/*************************************<->*************************************
 *
 *  FindDtSessionMatch(commandArgc, commandArgv, pCD, pSD, pWorkSpaceList,
 *                      clientMachine)
 *
 *  Description:
 *  -----------
 *  Try to find a match for this client in the session hints.
 *  Set up client-session data.
 *
 *
 *  Inputs:
 *  ------
 *  commandArgc    -  argument count
 *  commandArgv    -  WM_COMMAND argument vector
 *  pCD            -  pointer to client data
 *  pSD            -  pointer to screen data
 *  pWorkspaceList -  pointer to a list of workspaces (to be returned)
 *  clientMachine  -  string for -host option in session hints
 *
 *  Outputs:
 *  -------
 *  *pCD            -  client data (may be modified)
 *  FindDtSessionMatch - returns True if a match for this client
 *                        was found in the session hints.
 *  *pWorkspaceList - list of workspaces this client should be put 
 *                    into.  (needs to be freed)
 *
 *
 *  Comments:
 *  --------
 *  Various pieces of client state (in pCD) are reset when a match
 *     is found.
 *
 *  The caller must free *pWorkspaceList when done.
 * 
 *************************************<->***********************************/
Boolean FindDtSessionMatch(int commandArgc, char **commandArgv,
			    ClientData *pCD, WmScreenData *pSD,
			    char **pWorkSpaceList, char *clientMachine)

{
    int count, item;
    int relCount;
    int argNum;
    SessionGeom *sessionGeom;


    for (count = 0; count < pSD->totalSessionItems; count++)
    {
	if (!pSD->pDtSessionItems[count].processed &&
            pSD->pDtSessionItems[count].commandArgc == commandArgc)
	{
	    if ((clientMachine) &&
		(pSD->pDtSessionItems[count].clientMachine) &&
		(strcmp(clientMachine, 
			pSD->pDtSessionItems[count].clientMachine)))
	    {
		/*
		 * This item has clientMachine string but the 
		 * clientMachine does not match.
		 */
		continue;
	    }
            for (argNum = 0; argNum < commandArgc ; argNum++)
            {
                if(strcmp(commandArgv[argNum],
                          pSD->pDtSessionItems[count].commandArgv[argNum]))

                {
                    /*
                     * One mismatch and we quit looking at this item.
		     * Decrement argNum so a mismatch on the last item
		     * will not look like a match below when comparing
		     * argNum == commandArgc
                     */
		    argNum--;
                    break;
                }
	    }		
            if (argNum == commandArgc)
            {
                /*
                 * Made it through all strings so this is a match
                 */
		
                pSD->pDtSessionItems[count].processed = True;
                pSD->remainingSessionItems --;
                pCD->clientFlags |= SM_LAUNCHED;
		
		/*
		 * Free strings malloc'd for commandArgv for this item 
		 */
		
		for (relCount = 0; relCount < commandArgc; relCount++)
		{
		    XtFree(pSD->pDtSessionItems[count].commandArgv[relCount]);
		}
		XtFree((char *)pSD->pDtSessionItems[count].commandArgv);

                if(pSD->pDtSessionItems[count].clientState)
                {
                    pCD->clientState =
                        pSD->pDtSessionItems[count].clientState;
		    pCD->clientFlags |= SM_CLIENT_STATE;
                }
		
                if(pSD->pDtSessionItems[count].sessionGeom)
                {
                    sessionGeom = pSD->pDtSessionItems[count].sessionGeom;
                    if (sessionGeom->flags & XValue)
                    {
                        pCD->clientX = sessionGeom->clientX;
			pCD->clientFlags |= SM_X;
                    }
                    if (sessionGeom->flags & YValue)
                    {
                        pCD->clientY = sessionGeom->clientY;
			pCD->clientFlags |= SM_Y;
                    }
                    if (sessionGeom->flags & WidthValue)
                    {
                        pCD->clientWidth = sessionGeom->clientWidth;
			pCD->clientFlags |= SM_WIDTH;
                    }
                    if (sessionGeom->flags & HeightValue)
                    {
                        pCD->clientHeight = sessionGeom->clientHeight;
			pCD->clientFlags |= SM_HEIGHT;
                    }

		    /*
		     * Free SessionGeom malloc'd space for this item 
		     */
		    
		    XtFree((char *)pSD->pDtSessionItems[count].sessionGeom); 
                }

                if(pSD->pDtSessionItems[count].clientMachine)
                {
		    /*
		     * Free clientMachine malloc'd space for this item 
		     */
		    
		    XtFree((char *)
			   pSD->pDtSessionItems[count].clientMachine); 
		    pSD->pDtSessionItems[count].clientMachine = NULL;
                }
		
		
                if(pSD->pDtSessionItems[count].workspaces)
                {
		    /*
		     * The caller is responsible for freeing this
		     * data.
		     */
		    *pWorkSpaceList = pSD->pDtSessionItems[count].workspaces;
                }


		if(pSD->remainingSessionItems == 0)
		{
		    /*
		     * Free the whole pSD->pDtSessionHints structure 
		     */
		    XtFree((char *)pSD->pDtSessionItems);
		}
		
		return (True);
            }

        } /* not processed and argc's are the same */

    } /* for */
    
    return (False);
    
} /* END OF FUNCTION FindDtSessionMatch */





/*************************************<->*************************************
 *
 *  void
 *  ParseSessionItems (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Parse session items

 *
 *  Inputs:
 *  ------
 *  pSD    = pointer to screen data
 *  cfileP = (global) file pointer to  NULL
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display

 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void
ParseSessionItems (WmScreenData *pSD)
{
    unsigned char *string;
    unsigned char *lineP;
    int count;

    
    /*
     * Parse property string
     */

    

    if(GetNextLine () != NULL)
    {
       	pSD->totalSessionItems = atoi((char *)line);
	pSD->remainingSessionItems = pSD->totalSessionItems;
    }
    

    if((pSD->totalSessionItems < 1) ||
       !GetSessionHintsInfo(pSD, pSD->totalSessionItems))
    {
	/*
	 * No items or couldn't allocate space
	 */
	return;
    }
    
    count = 0;

    while ((count < pSD->totalSessionItems) && (GetNextLine () != NULL))
    {

	lineP = line;
	while ((string = GetSmartSMString (&lineP)) != NULL)
	{
	    if (!strcmp((char *)string, "-geometry"))
	    {
		/*
		 * Parse geometry if it is present
		 */
		string = GetSmartSMString(&lineP);
		ParseSessionGeometry (pSD, count, string);
	    }
	    
	    else if (!strcmp((char *)string, "-state"))
	    {
		/*
		 * Parse the state if it is present
		 */
		string = GetSmartSMString(&lineP);
		ParseSessionClientState (pSD, count, string);
	    }

	    else if (!strcmp((char *)string, "-workspaces"))	    
	    {
		/*
		 * Parse the workspaces string if it is present
		 */
		string = GetSmartSMString(&lineP);
		ParseSessionWorkspaces (pSD, count, string);
	    }	    

	    else if (!strcmp((char *)string, "-cmd"))	    
	    {
		/*
		 * Parse the command string if it is present
		 */
		string = GetSmartSMString(&lineP);
		ParseSessionCommand (pSD, count, &string);
	    }
	    
	    else if (!strcmp((char *)string, "-host"))	    
	    {
		/*
		 * Parse the host string if it is present
		 */
		string = GetSmartSMString(&lineP);
		ParseSessionHost (pSD, count, string);
	    }
	    
	} /* while GetSmartSMString */
	
	count++;

    } /* while GetNextLine */
    


} /* END OF FUNCTION ParseSessionItems */



/*************************************<->*************************************
 *
 *  ParseSessionClientState (pSD, count, string);
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void ParseSessionClientState (WmScreenData *pSD, int count,
			      unsigned char *string)



{

    if(!strcmp((char *)string, "NormalState"))
    {
	pSD->pDtSessionItems[count].clientState = NORMAL_STATE;
    }
    else if(!strcmp((char *)string, "IconicState"))
    {
	pSD->pDtSessionItems[count].clientState = MINIMIZED_STATE;
    }
    

} /* END OF FUNCTION ParseSessionClientState */


/*************************************<->*************************************
 *
 *  ParseSessionGeometry (pSD, count, string)
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void ParseSessionGeometry (WmScreenData *pSD, int count,
			   unsigned char *string)

{
    SessionGeom *pTmpSessionGeom;
    int mask /* = 0 */;
    int X, Y, width, height;
    X = Y = width = height = 0;
    
    /*
     *  XParseGeometry
     */

    mask = XParseGeometry((char *)string, &X, &Y, (unsigned int *)&width, 
			  (unsigned int *)&height);
    if (mask)
    {
	/*
	 * Allocate space for the geometry structure
	 */

	if ((pTmpSessionGeom = 
	     (SessionGeom *)XtMalloc (sizeof (SessionGeom))) == NULL)
	{
	    Warning (((char *)GETMESSAGE(60, 1, "Insufficient memory for session geometry item")));
	    return;

	}

	pTmpSessionGeom->flags = mask;
	pTmpSessionGeom->clientX = X;
	pTmpSessionGeom->clientY = Y;
	pTmpSessionGeom->clientWidth = width;
	pTmpSessionGeom->clientHeight = height;

	pSD->pDtSessionItems[count].sessionGeom = pTmpSessionGeom;
    }
    
} /* END OF FUNCTION  ParseSessionGeometry */


/*************************************<->*************************************
 *
 * void
 * ParseSessionWorkspaces (pSD, count, string)
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void ParseSessionWorkspaces (WmScreenData *pSD,  int count,
			     unsigned char *string)

{

    /*
     * Allocate space for the workspaces string
     */

    if ((pSD->pDtSessionItems[count].workspaces =
         (String)XtMalloc ((unsigned int) (strlen((char *)string) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(60, 2, "Insufficient memory for workspaces list in sesssion item")));
        return;

    }

    strcpy(pSD->pDtSessionItems[count].workspaces, (char *)string);
    
} /* END OF FUNCTION ParseSessionWorkspaces */



/*************************************<->*************************************
 *
 * void
 * ParseSessionCommand (pSD, count, string)
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void ParseSessionCommand (WmScreenData *pSD,  int count,
			  unsigned char **commandString)
{
#define ARG_AMT	100
    int xindex;
    unsigned char **argv;
    int  argc = 0;
    int  iSizeArgv;
    
    unsigned char *string;
    
    argv = (unsigned char **) XtMalloc (ARG_AMT * sizeof(char *));
    iSizeArgv = ARG_AMT;
    
    while ((string = GetSmartSMString (commandString)) != NULL)
    {
        /*
         * Get pointers to strings in command line and count them
         */
        argv[argc] = string;
        argc ++;

	if (argc >= iSizeArgv)
	{
	    iSizeArgv += ARG_AMT;
	    argv = (unsigned char **) 
		   XtRealloc ((char *)argv, (iSizeArgv * sizeof(char *)));
	}
    }
    if ((pSD->pDtSessionItems[count].commandArgv =
         (char **)XtMalloc ((argc) * sizeof(char * ))) == NULL)
    {
        /*
         * Allocate space for saved argv
         */
	
        Warning (((char *)GETMESSAGE(60, 3, "Insufficient memory for commandArgv array")));
    }
    else
    {
        pSD->pDtSessionItems[count].commandArgc = argc;
        for (xindex = 0; xindex < argc ; xindex++)
        {
            if ((pSD->pDtSessionItems[count].commandArgv[xindex] =
                 (String) XtMalloc
                 ((unsigned int) (strlen((char *)argv[xindex]) + 1))) == NULL)
            {
                /*
                 * Allocate space for the next command segment.
                 */
                Warning (((char *)GETMESSAGE(60, 4, "Insufficient memory for commandArgv item")));
            }
            else
            {
                strcpy(pSD->pDtSessionItems[count].commandArgv[xindex],
                       (char *)argv[xindex]);
            }
        }
    }

    XtFree ((char *) argv);
    
} /* END OF FUNCTION ParseSessionCommand */



/*************************************<->*************************************
 *
 * void
 * ParseSessionHost (pSD, count, string)
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
void ParseSessionHost (WmScreenData *pSD,  int count,
			     unsigned char *string)

{

    /*
     * Allocate space for the workspaces string
     */

    if ((pSD->pDtSessionItems[count].clientMachine =
         (String)XtMalloc ((unsigned int) (strlen((char *)string) + 1))) == 
	NULL)
    {
        Warning (((char *)GETMESSAGE(60, 38, 
		"Insufficient memory for host name in sesssion item")));
        return;
    }

    strcpy(pSD->pDtSessionItems[count].clientMachine, (char *)string);
    
} /* END OF FUNCTION ParseSessionHost */



/*************************************<->*************************************
 *
 *  GetSessionHintsInfo (pSD, numItems)
 *
 *  Description:
 *  -----------
 *  Inputs:
 *  ------
 *  Outputs:
 *  -------
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
Boolean GetSessionHintsInfo (WmScreenData *pSD, long numItems)

{
   
    if ((pSD->pDtSessionItems =
	 (DtSessionItem *)XtMalloc (numItems * sizeof (DtSessionItem)))
        == NULL)
    {
        Warning (((char *)GETMESSAGE(60, 5, "Insufficient memory for Dt Session Hints")));
        return(False);
    }
    
    memset ((char *)pSD->pDtSessionItems, NULL,
	    numItems * sizeof (DtSessionItem));

    return(True);
    
    
} /* END OF FUNCTION  GetSessionHintsInfo  */ 




/*************************************<->*************************************
 *
 *  PeekAhead (currentChar, currentLev)
 *
 *
 *  Description:
 *  -----------
 *  Returns a new level value if this is a new nesting level of quoted string
 *  Otherwise it returns a zero
 *
 *
 *  Inputs:
 *  ------
 *  currentChar = current position in the string
 *  currentLev = current level of nesting
 *
 * 
 *  Outputs:
 *  -------
 *  Returns either a new level of nesting or zero if the character is copied in
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int PeekAhead(unsigned char *currentChar,
		       unsigned int currentLev)


{
    Boolean		done = False;
    unsigned int 	tmpLev = 1;
#ifndef NO_MULTIBYTE
    unsigned int	chlen;

    while (((chlen = mblen ((char *)currentChar, MB_CUR_MAX)) > 0) &&
	   (chlen == 1) && ((*currentChar == '"') || (*currentChar == '\\'))
	   && (done == False))
    {
	currentChar++;

	if(((chlen = mblen ((char *)currentChar, MB_CUR_MAX)) > 0) && (chlen == 1) &&
	   ((*currentChar == '"') || (*currentChar == '\\')))
	{
	    tmpLev++;
	    if(*currentChar == '"')
	    {
		done = True;
	    }
	    else
	    {
		currentChar++;
	    }
	}
    }
#else
    while((*currentChar != NULL) && (done == False) &&
	  ((*currentChar == '"') || (*currentChar == '\\')))
    {
	currentChar++;
	if((*currentChar != NULL) &&
	   ((*currentChar == '"') || (*currentChar == '\\')))
	{
	    tmpLev++;
	    if(*currentChar == '"')
	    {
		done = True;
	    }
	    else
	    {
		currentChar++;
	    }
	}
    }
#endif /*NO_MULTIBYTE*/

    /*
     * Figure out if this is truly a new level of nesting - else ignore it
     * This section probably could do some error checking and return -1
	 * If so, change type of routine from unsigned int to int
     */
    if(done == True)
    {
	return(tmpLev);
    }
    else
    {
	return(0);
    }
} /* END OF FUNCTION PeekAhead */
    
    
#endif /* WSM */



#ifdef MOTIF_ONE_DOT_ONE
/*************************************<->*************************************
 *
 *  GetHomeDirName (fileName)
 *
 *  Description:
 *  -----------
 *  This function finds the "HOME" directory
 *
 *
 *  Inputs:
 *  ------
 *  fileName 
 *
 *  Outputs:
 *  -------
 *  fileName
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void GetHomeDirName(String  fileName)
{
        int uid;
        struct passwd *pw;
        char *ptr = NULL;

        if((ptr = getenv("HOME")) == NULL)
        {
            if((ptr = getenv("USER")) != NULL)
	    {
		pw = getpwnam(ptr);
	    }
            else
            {
                uid = getuid();
                pw = getpwuid(uid);
            }

            if (pw)
	    {
                ptr = pw->pw_dir;
	    }
            else
	    {
                ptr = "";
	    }
        }
        strcpy(fileName, ptr);
}
#endif


/*************************************<->*************************************
 *
 *  SyncModifierStrings (fileName)
 *
 *  Description:
 *  -----------
 *  This function updates modifierStrings table so that Mwm uses the correct
 *  modifier to keysym mapping.  Specifically, fix up the Alt and Meta bindings.
 *
 *  Inputs:
 *  ------
 *  fileName 
 *
 *  Outputs:
 *  -------
 *  fileName
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void SyncModifierStrings(void)
{
    XModifierKeymap *map;
    int i, j, k = 0;

    map = XGetModifierMapping (DISPLAY);

    for (i = 0; i < 8; i++)
    {
	for (j = 0; j < map->max_keypermod; j++)
	{
	    if (map->modifiermap[k])
	    {
		KeySym ks = XKeycodeToKeysym(DISPLAY, map->modifiermap[k], 0);
		char *nm = XKeysymToString(ks);

		/* Compare, ignoring the trailing '_L' or '_R' in keysym */
		if (nm && !strncmp("Alt", nm, 3))
		{
		    modifierStrings[ALT_INDEX].mask = (1<<i);
		}
		else if (nm && !strncmp("Meta", nm, 4))
		{
		    modifierStrings[META_INDEX].mask = (1<<i);
		}
	    }
	    k++;
	}
    }

    XFreeModifiermap(map);
}



/*************************************<->*************************************
 *
 *  ProcessWmFile ()
 *
 *
 *  Description:
 *  -----------
 *  This function reads the mwm resource description file and processes the
 *  resources that are described.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  pSD->buttonBindings = buttonBindings resource value
 *  wmGD.configFile = configuration file resource value
 *  pSD->keyBindings = keyBindings resource value
 *  wmGD.rootWindow = default root window of display
 *  HOME = environment variable for home directory
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.buttonSpecs = list of button binding specifications
 *  wmGD.keySpecs = list of key binding specification
 *  wmGD.menuSpecs = list of menu specifications
 * *wmGD.acceleratorMenuSpecs = initialized array of (MenuSpec *)
 *  wmGD.acceleratorMenuCount = 0
 *
 *
 *  Comments:
 *  --------
 * If there are more than MAXLINE characters on a line the excess characters
 * are truncated.
 * 
 *************************************<->***********************************/
#define MENU_SPEC	"menu"
#define BUTTON_SPEC	"buttons"
#define KEY_SPEC	"keys"
#ifdef PANELIST
#define FRONT_PANEL_SPEC DTWM_FP_PANEL_OLD
#define DROP_EFFECTS_SPEC   DTWM_FP_DROP_EFFECTS
#define PANEL_SPEC	DTWM_FP_PANEL
#define BOX_SPEC	DTWM_FP_BOX
#define CONTROL_SPEC	DTWM_FP_CONTROL
#define INCLUDE_SPEC	DTWM_FP_INCLUDE
#define ANIMATION_SPEC	DTWM_FP_ANIMATION
#define SWITCH_SPEC	DTWM_FP_SWITCH

void ProcessWmFile (WmScreenData *pSD, Boolean bNested)

#else /* PANELIST */
void ProcessWmFile (WmScreenData *pSD)
#endif /* PANELIST */
{
    unsigned char *lineP;
    unsigned char *string;
    unsigned int   n;
    MenuSpec      *menuSpec;
#ifdef PANELIST
    static Boolean conversionInProgress = False;
    Arg args[10];
    int argnum;

    if (!bNested)
    {
#endif /* PANELIST */

    /*
     * Initialize global data values that are set based on data in
     * the mwm resource description file.
     */

    pSD->buttonSpecs = NULL;
    pSD->keySpecs = NULL;
    pSD->menuSpecs = NULL;

#ifdef WSM
    /**** hhhhhhhhhhhh   ******/
    GetFunctionTableValues (&F_EXEC_INDEX, &F_NOP_INDEX, &F_ACTION_INDEX);
#endif /* WSM */
    /*
     * Find and parse the default system menu string, if it exists.
     */

    cfileP = NULL;
    linec = 0;
    if (((parseP = (unsigned char *) builtinSystemMenu) != NULL) && 
	 (GetNextLine () != NULL))
    {
	lineP = line;
        ParseMenuSet (pSD, lineP);
    }

    linec = 0;
    if (((parseP = (unsigned char *) builtinRootMenu) != NULL) && 
	 (GetNextLine () != NULL))
    {
	lineP = line;
        ParseMenuSet (pSD, lineP);
    }
#ifdef PANELIST
    if (wmGD.useFrontPanel &&  !wmGD.dtSD &&
	(XDefaultScreen (wmGD.display) == pSD->screen))
    {
        wmGD.dtSD = pSD;  /* only one per display */
    }
#endif /* PANELIST */

    /*
     * Find and associate a stream with the window manager resource 
     *   description file.
     */

    if ((cfileP = FopenConfigFile ()) == NULL)
    {
	if (!wmGD.useStandardBehavior)
	  Warning (((char *)GETMESSAGE(60, 6, "Cannot open configuration file")));
	return;
    }

#ifdef PANELIST 
    }  /* end if (!bNested) */
#endif /* PANELIST */
    /*
     * Parse the information in the configuration file.
     * If there are more than MAXLINE characters on a line the excess are
     *   truncated.
     */

    linec = 0;
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
	lineP = line;
	if ((*line == '!') || (string = GetString (&lineP)) == NULL)
	/* empty or comment line */
	{
	    continue;
	}

	ToLower (string);
	if (!strcmp ((char *)string, MENU_SPEC))
	{
	    ParseMenuSet (pSD, lineP);
	}
	else if (!strcmp ((char *) string, BUTTON_SPEC))
	{
	    ParseButtonSet (pSD, lineP);
	}
	else if (!strcmp ((char *) string, KEY_SPEC))
	{
	    ParseKeySet (pSD, lineP);
	}
#ifdef PANELIST
        else if (!strcmp ((char *)string, INCLUDE_SPEC))
        {
            ParseIncludeSet (pSD, lineP);
        }
#endif /* PANELIST */
    }

    fclose (cfileP);

    /*
     * Create and initialize the pSD->acceleratorMenuSpecs array.
     * This assumes we create pointers to MenuSpecs within ProcessWmFile().
     * Set pSD->acceleratorMenuCount to 0.
     */

    /* count the number of menu specifications */
    n = 0;
    menuSpec = pSD->menuSpecs;
    while (menuSpec)
    {
	n++;
	menuSpec = menuSpec->nextMenuSpec;
    }

    /* allocate the array and initialize to zeros */
    pSD->acceleratorMenuSpecs = NULL;
    if (n)
    {
        pSD->acceleratorMenuSpecs = 
	    (MenuSpec **) XtCalloc (n, sizeof (MenuSpec *));
        if (pSD->acceleratorMenuSpecs == NULL)
        {
            Warning (((char *)GETMESSAGE(60, 7, "Insufficient memory for menu accelerators")));
        }
    }
    pSD->acceleratorMenuCount = 0;
} /* END OF FUNCTION ProcessWmFile */

/**** This function stolen from Xt/Intrinsic.c ****/
/* The implementation of this routine is operating system dependent */

static char *ExtractLocaleName(lang)
    String	lang;
{

#ifdef hpux	 /* hpux-specific parsing of the locale string */
#define MAXLOCALE       64      /* buffer size of locale name */

    char           *start;
    char           *end;
    int             len;
    static char     buf[MAXLOCALE];

    /*  If lang has a substring ":<category>;", extract <category>
     *  from the first such occurrence as the locale name.
     */

    start = lang;
    if (start = strchr (lang, ':')) {
        start++;
        if (end = strchr (start, ';')) {
            len = end - start;
            strncpy(buf, start, len);
            *(buf + len) = '\0';
            lang = buf;
      }
    }
#endif	/* hpux */

    return lang;
}

#ifdef WSM
#define RC_CONFIG_SUBDIR		"/config/"
#define RC_DEFAULT_CONFIG_SUBDIR	"/config/C"
#endif /* WSM */

/*************************************<->*************************************
 *
 *  FopenConfigFile ()
 *
 *
 *  Description:
 *  -----------
 *  This function searches for, opens, and associates a stream with the mwm 
 *  resource description file,
 *
 *
 *  Inputs:
 *  ------
 *  wmGD.configFile = configuration file resource value.
 *  HOME = environment variable for home directory
 *
 *
 *  Outputs:
 *  -------
 *  Return = If successful, a pointer to the FILE structure associated with 
 *           the configuration file.  Otherwise, NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/
FILE *FopenConfigFile (void)
{

    char    *LANG, *LANGp;
    FILE    *fileP;

#ifndef MOTIF_ONE_DOT_ONE
    char *homeDir = XmeGetHomeDirName();
#endif
#ifdef PANELIST
    Boolean stackPushed;
#endif /* PANELIST */

    /*
     * Get the LANG environment variable
     * make copy since another call to getenv will blast the value.
     */
    LANGp = setlocale(LC_CTYPE, NULL);

    /*
     * setlocale not guaranteed to return $LANG -- extract
     * something usable.
     */
    LANGp = ExtractLocaleName (LANGp);

    if ((LANGp == NULL) || (strlen(LANGp) == 0))
      {
	 LANG = NULL;
      }
    else
      {
	 if ((LANG = (char *) XtMalloc(strlen(LANGp) +1)) == NULL)
	   {
	      PWarning (((char *)GETMESSAGE(60, 41, "Insufficient memory to get LANG environment variable.")));
	      return(NULL);
	   }

	 strcpy(LANG, LANGp);
      }


    /*
     * To get a name for the file first look at the value of the configFile
     * resource.  Interpret "~/.." as relative to the user's home directory.
     * Use the LANG variable if set and .mwmrc is in $HOME/$LANG/.mwmrc  
     */

#ifdef PANELIST
    if (pConfigStackTop && pConfigStackTop->tempName)
    {
	fileP = fopen (pConfigStackTop->tempName, "r");
	return (fileP);
    }
    stackPushed = (pConfigStackTop && (pConfigStackTop != pConfigStack));
    fileP = NULL;
    cfileName[0] = '\0';
#endif /* PANELIST */
    if ((wmGD.configFile != NULL) && (wmGD.configFile[0] != '\0'))
    /* pointer to nonNULL string */
    {
        if ((wmGD.configFile[0] == '~') && (wmGD.configFile[1] == '/'))
	/* handle "~/..." */
	{
#ifdef MOTIF_ONE_DOT_ONE
	    GetHomeDirName(cfileName);
#else
	    strcpy (cfileName, homeDir);
#endif
	    if (LANG != NULL)
	    {
		strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
		strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	    }
	    strncat(cfileName, &(wmGD.configFile[1]), MAXWMPATH-strlen(cfileName));
	    if ((fileP = fopen (cfileName, "r")) != NULL)
	    {
		if (LANG != NULL) { 
		    XtFree(LANG); 
		    LANG = NULL; 
		}
#ifndef PANELIST
		return (fileP);
#endif /* PANELIST */
	    }
	    else
	    {
		/* 
		 * Just try $HOME/.mwmrc
		 */
#ifdef MOTIF_ONE_DOT_ONE
		GetHomeDirName(cfileName);
#else
		strcpy (cfileName, homeDir);
#endif
		strncat(cfileName, &(wmGD.configFile[1]), 
			MAXWMPATH-strlen(cfileName));
		if ((fileP = fopen (cfileName, "r")) != NULL)
		{
		  if (LANG != NULL) {
		      XtFree(LANG);
		      LANG = NULL;
		  }
#ifndef PANELIST
		  return (fileP);
#endif /* PANELIST */
		}
	    }

	    
	}
	else
	/* relative to current directory or absolute */
	{
#ifdef PANELIST
	    char *pch;

            pch = (char *) GetNetworkFileName (wmGD.configFile);

	    if ((fileP = fopen (pch, "r")) != NULL)
	    {
	        strncpy (cfileName, pch, MAXWMPATH);
	    }
	    XtFree (pch);
  
  	  if ((fileP == NULL) && !stackPushed)
	  {
#endif  /* PANELIST  */
            if ((fileP = fopen (wmGD.configFile, "r")) != NULL)
	      {
		if (LANG != NULL) {
		    XtFree(LANG);
		    LANG = NULL;
		}
		return(fileP);
	      }
#ifdef PANELIST 
	  }
	  else if ((fileP == NULL) && stackPushed)
	  {
		strcpy (cfileName, wmGD.configFile);
	  }
#endif /* PANELIST */
	}
    }
#ifdef PANELIST 
  if ((fileP == NULL) && !stackPushed)
  {
#endif /* PANELIST */

    /*
     * The configFile resource didn't do it for us.
     * First try HOME_MWMRC, then try SYS_MWMRC .
     */

#define HOME_MWMRC "/.mwmrc"
#define SLASH_MWMRC "/system.mwmrc"

#ifdef MOTIF_ONE_DOT_ONE
    GetHomeDirName(cfileName);
#else
    strcpy (cfileName, homeDir);
#endif

#ifdef WSM
    if (MwmBehavior)
    {
	/*
	 *
	 *  Looking for $HOME/$LANG/.mwmrc
	 *  --or--if $LANG is NULL
	 *  Looking for $HOME/.mwmrc
	 *
	 */
	if (LANG != NULL)
	{
	    strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
	    strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	}
	strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));
    }
    else
    {
	/*
	 *
	 *  Looking for $HOME/.dt/$LANG/dtwmrc
	 *
	 *  --or--if $LANG is NULL--
	 *
	 *  Looking for $HOME/.dt/dtwmrc
	 *
	 */
	strncat(cfileName, "/.dt/", MAXWMPATH-strlen(cfileName));

	if (LANG != NULL)
	{
	    strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	}
	strncat(cfileName, LANG_DT_WMRC, MAXWMPATH - strlen(cfileName));
    }
#else /* WSM */
    if (LANG != NULL)
    {
	strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
	strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
    }
    strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));
#endif /* WSM */
    if ((fileP = fopen (cfileName, "r")) != NULL)
    {
        if (LANG != NULL) {
	    XtFree(LANG);
	    LANG = NULL;
	}
#ifndef PANELIST
        return (fileP);
#endif /* PANELIST */
    }
    else
    {
	/* 
	 * Just try $HOME/.mwmrc
	 */
#ifdef MOTIF_ONE_DOT_ONE
	GetHomeDirName(cfileName);
#else
    strcpy (cfileName, homeDir);
#endif
#ifdef WSM
	if (MwmBehavior)
	{
	    /* 
	     * Just try $HOME/.mwmrc
	     */
	    strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));
	}
	else
	{
	    /* 
	     * Just try $HOME/.dt/dtwmrc
	     */
	    strncat(cfileName, HOME_DT_WMRC, MAXWMPATH - strlen(cfileName));
	}
#else /* WSM */
        strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));
#endif /* WSM */
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  if (LANG != NULL) {
	      XtFree(LANG);
	      LANG = NULL;
	  }
#ifndef PANELIST
	  return (fileP);
#endif /* PANELIST */
	}
    }
#ifdef PANELIST
  }

#define DTLIBDIR  CDE_INSTALLATION_TOP
#define DTADMINDIR  CDE_CONFIGURATION_TOP
#define SLASH_DT_WMRC "/sys.dtwmrc"

  if ((fileP == NULL) && !stackPushed)
  {
    /* 
     * No home-based config file. Try the admin directory.
     */
    strcpy(cfileName, DTADMINDIR);
    strncat(cfileName, RC_CONFIG_SUBDIR, MAXWMPATH-strlen(cfileName));
    strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
    strncat(cfileName, SLASH_DT_WMRC, MAXWMPATH - strlen(cfileName));

    if (((fileP = fopen (cfileName, "r")) == NULL) && LANG && *LANG)
    {
	/* Try it with no LANG */
	strcpy(cfileName, DTADMINDIR);
	strncat(cfileName, RC_CONFIG_SUBDIR, MAXWMPATH-strlen(cfileName));
	strncat(cfileName, SLASH_DT_WMRC, MAXWMPATH - strlen(cfileName));
    }

    if ((fileP = fopen (cfileName, "r")) != NULL)
    {
      XtFree(LANG);
      LANG = NULL;
    }
  }

  if ((fileP == NULL) && !stackPushed)
  {
#endif /* PANELIST */

#ifndef MWMRCDIR
#define MWMRCDIR "/usr/lib/X11"
#endif
    if (LANG != NULL)
    {
#ifdef WSM
	if (MwmBehavior)
	{
	    strcpy(cfileName, MWMRCDIR);
	    strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
	    strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	    strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));
	}
	else
	{
	    strcpy(cfileName, DTLIBDIR);
	    strncat(cfileName, RC_CONFIG_SUBDIR, MAXWMPATH-strlen(cfileName));
	    strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	    strncat(cfileName, SLASH_DT_WMRC, MAXWMPATH - strlen(cfileName));
	}
#else /* WSM */
       /*
	* Try /$LANG/system.mwmrc within the install tree
	*/
	strcpy(cfileName, MWMRCDIR);
	strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
	strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));
#endif /* WSM */
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  XtFree(LANG);
	  LANG = NULL;
#ifndef PANELIST
	  return (fileP);
#endif /* PANELIST */
	}
    }

#ifdef PANELIST
    if ((fileP == NULL) && !stackPushed)
    {
#endif /* PANELIST */
#ifdef WSM
    if (MwmBehavior)
    {
	strcpy(cfileName, MWMRCDIR);
	strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));
#ifdef PANELIST
	fileP = fopen (cfileName, "r");
#else /* PANELIST */
	return (fopen (cfileName, "r"));
#endif /* PANELIST */
    }
    else
    {
	strcpy(cfileName, DTLIBDIR);
	strncat(cfileName, RC_DEFAULT_CONFIG_SUBDIR, 
					MAXWMPATH - strlen(cfileName));
	strncat(cfileName, SLASH_DT_WMRC, MAXWMPATH - strlen(cfileName));
#ifdef PANELIST
	fileP = fopen (cfileName, "r");
#else /* PANELIST */
	return (fopen (cfileName, "r"));
#endif /* PANELIST */
    }
#else /* WSM */
    /*
     * Try /system.mwmrc within the install tree
     */
    strcpy(cfileName, MWMRCDIR);
    strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));

    if (LANG != NULL) 
    {
       XtFree(LANG);
       LANG = NULL;
    }
#ifdef PANELIST
    strcpy(cfileName, cfileName);
    fileP = fopen (cfileName, "r");
#else /* PANELIST */
    return (fopen (cfileName, "r"));
#endif /* PANELIST */
#endif /* WSM */
#ifdef PANELIST
    }
  }

    if (!fileP)
    {
	char *pch;

	/*
	 * handle "<host>:<path>" form of file name
	 */
	pch = (char *) GetNetworkFileName (cfileName);
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	    strncpy (cfileName, pch, MAXWMPATH);
	    XtFree (pch);
	}

	/*
	 * Either not "<host>:<path>" form or there was a
	 * problem up above. This is the last attempt to 
	 * open something.
	 */
	if (!fileP)
	{
	    fileP = fopen (cfileName, "r");
	}
    }

    if (!pConfigStack)
    {
	ConfigStackInit (cfileName);
    }

    if (wmGD.cppCommand && *wmGD.cppCommand)
    {
	/*
	 *  Run the file through the C-preprocessor
	 */
	PreprocessConfigFile ();
	if (pConfigStackTop->cppName)
	{
	    /* open the result */
	    fileP = fopen (pConfigStackTop->cppName, "r");
	}
    }

    if (LANG != NULL) 
    {
	XtFree(LANG);
	LANG = NULL;
    }
    return (fileP);
#endif /* PANELIST */

} /* END OF FUNCTION FopenConfigFile */


/*************************************<->*************************************
 *
 *  SaveMenuAccelerators (pSD, newMenuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function saves the MenuSpec pointer in pSD->acceleratorMenuSpecs.
 *
 *
 *  Inputs:
 *  ------
 *  newMenuSpec = pointer to MenuSpec to be saved.
 *  pSD->acceleratorMenuSpecs = 
 *  pSD->acceleratorMenuCount = 
 *
 *
 *  Outputs:
 *  -------
 *  pSD->acceleratorMenuSpecs = possibly updated
 *  pSD->acceleratorMenuCount = possibly updated
 *
 *
 *  Comments:
 *  --------
 *  We assume only MenuSpecs created within ProcessWmFile() are to be saved.
 *  Otherwise, we may cause override the limits of pSD->acceleratorMenuSpecs.
 * 
 *************************************<->***********************************/

void SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec)
{
    MenuSpec  **pMenuSpec;

    pMenuSpec = pSD->acceleratorMenuSpecs;

    if (pMenuSpec == NULL) 
	return;

    while ((*pMenuSpec != NULL) && (*pMenuSpec != newMenuSpec))
    {
	pMenuSpec++;
    }

    if (*pMenuSpec == NULL)
    {
	*pMenuSpec = newMenuSpec;
        pSD->acceleratorMenuCount++;
    }

} /* END OF FUNCTION SaveMenuAccelerators */


/*************************************<->*************************************
 *
 *  ParseMenuSet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Menu pane specification found.  Parse the following syntax:
 *
 *          v
 *     Menu menu_pane_name
 *     {
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  lineP = pointer to menu name in line buffer
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *
 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->menuSpecs = list of menu specifications
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed menu specifications.
 *  This means custom menu specifications can be distinguished by NULL name.
 * 
 *************************************<->***********************************/

static void ParseMenuSet (WmScreenData *pSD, unsigned char *lineP)
{
    unsigned char     *string;
    MenuSpec *menuSpec;
    
    /*
     * If menu name is NULL then skip this pane specification.
     */

    if ((string = GetString (&lineP)) == NULL)
    {
        return;
    }

    /*
     * Allocate space for the menu specification structure.
     */

    if ((menuSpec = (MenuSpec *)XtMalloc (sizeof (MenuSpec))) == NULL)
    {
        PWarning (((char *)GETMESSAGE(60, 9, "Insufficient memory for menu")));
	return;
    }
    menuSpec->currentContext = 0;
    menuSpec->menuWidget = NULL;
    menuSpec->whichButton = SELECT_BUTTON;  /* Button1 selection default */
    menuSpec->menuItems = NULL;
    menuSpec->accelContext = 0;
    menuSpec->accelKeySpecs = NULL;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    menuSpec->exclusions = NULL;
    menuSpec->clientLocal = FALSE;
    menuSpec->commandID = 0;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    menuSpec->nextMenuSpec = NULL;

    /*
     * Allocate and fill space for the menu name.
     */

    if ((menuSpec->name = 
	 (String)XtMalloc ((unsigned int) (strlen ((char *)string) + 1))) 
	 == NULL)
    {
        PWarning (((char *)GETMESSAGE(60, 10, "Insufficient memory for menu")));
	XtFree ((char *)menuSpec);
	return;
    }
    strcpy (menuSpec->name, (char *)string);

    /* 
     * Add the empty structure to the head of the menu specification list.
     */

    menuSpec->nextMenuSpec = pSD->menuSpecs;
    pSD->menuSpecs = menuSpec;

    /*
     * Require leading '{' on the next line.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
	/* found '{' */
        {
            break;
        }

	/* not a '{' */
	PWarning (((char *)GETMESSAGE(60, 11, "Expected '{' after menu name")));
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Parse menu item specifications until "}" or EOF found.
     */

    menuSpec->menuItems = PARSE_MENU_ITEMS (pSD, menuSpec);

} /* END OF FUNCTION ParseMenuSet */


/*************************************<->*************************************
 *
 *  MenuItem *
 *  ParseMwmMenuStr (pSD, menuStr)
 *
 *
 *  Description:
 *  -----------
 *  This function parses a WMW_MENU string and returns a list of 
 *  MenuItems structures.  The string should have the syntax:
 *
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *
 *
 *  Inputs:
 *  ------
 *  line = (global) line buffer
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  Return = list of MenuItem structures or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

MenuItem *ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr)
{

    cfileP = NULL;
    linec = 0;
    parseP = menuStr;

    return (PARSE_MENU_ITEMS (pSD, NULL));

} /* END OF FUNCTION ParseMwmMenuStr */


/*************************************<->*************************************
 *
 *  static MenuItem *
 *  ParseMenuItems (pSD, menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  Parse menu item specifications:
 *
 *       label  [mnemonic]  [accelerator]  function
 *       label  [mnemonic]  [accelerator]  function
 *                 ...
 *       label  [mnemonic]  [accelerator]  function
 *     [}]
 *
 *
 *  Inputs:
 *  ------
 *  pSD    = pointer to screen data
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *
 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  Return = list of MenuItem structures or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static MenuItem *ParseMenuItems (WmScreenData *pSD
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
				 , MenuSpec *menuSpec
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
				)
{
    unsigned char *string;
    unsigned char *lineP;
    MenuItem      *firstMenuItem;
    MenuItem      *lastMenuItem;
    MenuItem      *menuItem;
    register int   ix;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    Boolean        use_separators = False;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    
    /*
     * Parse "label [mnemonic] [accelerator] function" or
     *       "<client command>[.<client command>]*"
     * lines until "}" or EOF found.
     */

    firstMenuItem = lastMenuItem = NULL;
    while ((GetNextLine () != NULL))
    {
	lineP = line;
	if ((*line == '!') || (*line == '#') || (string = GetString (&lineP)) == NULL)
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*string == '}')  /* finished with menu set. */
        {
	    break;
        }

	/*
	 * Allocate space for the menu item structure. 
	 */

        if ((menuItem = (MenuItem *)XtMalloc (sizeof (MenuItem))) == NULL)
	{
            PWarning (((char *)GETMESSAGE(60, 12, "Insufficient memory for menu item")));
            continue;
	}
	menuItem->nextMenuItem = NULL;
	menuItem->wmFunction = (WmFunction)NULL;
	menuItem->wmFuncArgs = NULL;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	menuItem->clientCommandName = NULL;
	menuItem->clientCommandID = 0;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	/*
	 * Is this a simple menu item label or is it a
	 * client command specification.
	 */

	if (IsClientCommand((String) string))
	{
	    if (!ParseClientCommand(&lineP, menuSpec, menuItem, string,
				    &use_separators))
	    {
		XtFree ((char *)menuItem);
		continue;
	    }

	    for (ix = 0; ix < WMFUNCTIONTABLESIZE - 1; ++ix)
	      if (functionTable[ix].wmFunction == F_InvokeCommand)
		break;

	    if (ix == WMFUNCTIONTABLESIZE - 1)
	    {
		ix = F_NOP_INDEX;
		menuItem->wmFunction = F_Nop;
	    }
	    else menuItem->wmFunction = F_InvokeCommand;
	}
	else /* It must be a menu item label */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	{
	    /*
	     * Parse the menu item label.
	     */
	    if (!ParseWmLabel (pSD, menuItem, string))
	    {
		XtFree ((char *)menuItem);
		continue;
	    }
	}

        /*
	 * Parse any menu function mnemonic.
	 */

	ParseWmMnemonic (&lineP, menuItem);

        /*
	 * Parse any menu function accelerator.
	 */

	if (!ParseWmAccelerator (&lineP, menuItem))
	{
	    FreeMenuItem (menuItem);
	    continue;
	}
	/*
	 * Parse the menu function name if this is not a client
	 * command. If it is a client command, then the wmFunction
	 * field should already be set, as well as the ix variable,
	 * but we do want to search for a menu item name that occupies
	 * the same place as the function does for normal menu items.
	 */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	if (menuItem->wmFunction != NULL)
	  ParseMenuItemName(&lineP, menuItem);
	else
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	  ix = ParseWmFunction (&lineP, CRS_MENU, &menuItem->wmFunction);

	/*
	 * Determine context sensitivity and applicability mask.
	 */

	menuItem->greyedContext = functionTable[ix].greyedContext;
	menuItem->mgtMask = functionTable[ix].mgtMask;
#ifdef PANELIST
        if ((menuItem->wmFunction == F_Toggle_Front_Panel) &&
	    ((wmGD.useFrontPanel == False) ||
	     (wmGD.dtSD != pSD)))
	{
	    /*
	     * disallow this function if there's no front
	     * panel on this screen.
	     */
	    menuItem->greyedContext |= (F_CONTEXT_ALL 		| 
				       F_SUBCONTEXT_IB_WICON 	| 
				       F_SUBCONTEXT_IB_IICON);
	}
#endif /* PANELIST */

        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, menuItem->wmFunction, &menuItem->wmFuncArgs))
        {
	    FreeMenuItem (menuItem);
	    continue;  /* skip this menu item */
        }

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	/*
	 * If we're working on the f.cci function, this will fix-up
	 * the menuItem entries so that it appears that we read-in
	 * an old-style client-command entry.  Eventually, the cci
	 * handling should be changed to make use of the wmFuncArgs.
	 * Note that if DEFAULT_NAME was specified as the label, it
	 * is first set to NULL.
	 * FixMenuItem needs menuSpec since this is when the EXCLUDE
	 * items are stored.
	 */
	if (ix == F_CCI_INDEX)
	  {
	    CCIEntryModifier mod = ((CCIFuncArg *)menuItem->wmFuncArgs)->mod;

	    /* first fix the label if needed. */
	    if (!strcmp(menuItem->label, CCI_USE_DEFAULT_NAME_TAG))
	      {
		XtFree(menuItem->label);
		menuItem->label = NULL;
	      }

	    FixMenuItem(menuSpec, menuItem);

	    if (mod == DELIMIT || mod == DELIMIT_CASCADE || mod == DELIMIT_INLINE)
	      use_separators = True;
	  }

	/*
	 * If this menu item is supposed to be wrapped in separators,
	 * then create a separator template before the menu item
	 */
	if (use_separators)
	{
	    MenuItem *separator = MakeSeparatorTemplate(TOP_SEPARATOR);
	    if (lastMenuItem != NULL) lastMenuItem->nextMenuItem = separator;
	    else 		      firstMenuItem = separator;
	    lastMenuItem = separator;
	}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

	/*
	 * Add this item to the menu specification.
	 */

	if (lastMenuItem != NULL)  /* not first */
	{
	    lastMenuItem->nextMenuItem = menuItem;
	}
	else
	{
	    firstMenuItem = menuItem;
	}
	lastMenuItem = menuItem;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	/* If this menu item is supposed to be wrapped in separators
	 * then create a separator template after the menu item
	 */
	if (use_separators)
	{
	    MenuItem *separator = MakeSeparatorTemplate(BOTTOM_SEPARATOR);
	    if (lastMenuItem != NULL) lastMenuItem->nextMenuItem = separator;
	    else 		      firstMenuItem = separator;
	    lastMenuItem = separator;
	}

	use_separators = FALSE;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    }

    return (firstMenuItem);

} /* END OF FUNCTION ParseMenuItems */



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  StoreExclusion (menuSpec, string)
 *
 *
 *  Description:
 *  -----------
 *  Store the exclusion string in the menuspec. The list of exclusion
 *  strings are used to determine whether an insertion should be disallowed.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = the menu specification structure
 *  string   = exclusion client command string
 *
 * 
 *  Outputs:
 *  -------
 *  Return   = nothing
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

static void StoreExclusion (MenuSpec *menuSpec, String string)
{
    MenuExclusion *exclusion;

    exclusion = (MenuExclusion *)XtMalloc(sizeof(MenuExclusion));
    exclusion->command_string = XtNewString(string);

    /* We don't care what order the exclusions are in so stick it
       at the head of the list because it is easier. */
    exclusion->nextExclusion = menuSpec->exclusions;
    menuSpec->exclusions = exclusion;
}


/*************************************<->*************************************
 *
 *  IsClientCommand (string)
 *
 *
 *  Description:
 *  -----------
 *  Determine whether the string is a client command by the prefix
 *  characters.
 *
 *
 *  Inputs:
 *  ------
 *  string   = possible client command string
 *
 * 
 *  Outputs:
 *  -------
 *  Return   = (Boolean) TRUE iff the string is a client command.
 *	       Otherwise, FALSE is returned.
 *
 *
 *  Comments:
 *  --------
 *  This function simply checks what the first two or three characters of
 *  the string are. If they match the beginning of a client command
 *  specification, then TRUE is returned. This function does no go on to
 *  parse the rest of the specification. The legal client command beginning
 *  characters are:
 *
 *		characters:	meaning:
 *		-----------------------------------------------
 *		    <		simple client command beginning
 *		  -><		forced cascade menu
 *		   =<		client command with separators
 *		   ~<		exclusion operator
 * 
 *  Assumes:
 *  --------
 *  There is no leading whitespace on the string
 *
 *************************************<->***********************************/

Boolean IsClientCommand (String string)
{
    if ((
#ifndef NO_MULTIBYTE
	 mblen ((char *)string, MB_CUR_MAX) == 1 &&
#endif
	 *string == '<') ||
	(strncmp(string, "-><", 3) == 0) ||
	(strncmp(string, "=<", 2) == 0)  ||
	(strncmp(string, "=><", 3) == 0) ||
	(strncmp(string, "~<", 2) == 0))
      return(TRUE);

    return(FALSE);
}


/*************************************<->*************************************
 *
 *  ParseClientCommand (linePP, menuSpec, menuitem, string, use_separators)
 *
 *
 *  Description:
 *  -----------
 *  Parse the string and whatever is left of the line to verify whether
 *  correct syntax was used for a client command. Store the client command
 *  string in the menuitem, unless it is an exclusion. If it is an
 *  exclusion, then store the exclusion string in the menuSpec and return
 *  FALSE to indicate that the menuitem is no longer needed.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *  string   = first token of client command
 *
 * 
 *  Outputs:
 *  -------
 *  Return   = (Boolean) TRUE iff the line is a valid client command 
 *	       that can used to match insertions.
 *	       Otherwise, FALSE is returned meaning that the client
 *	       command had incorrect syntax or it was an exclusion, in
 *	       which case any useful information was stored in the
 *	       menuSpec.
 *
 *
 *  Comments:
 *  --------
 *  This function parses the entire line to determine if correct
 *  syntax was used for the client command. We assume at this point
 *  that the line is a client command. We are just syntax checking.
 *  If the syntax is correct, the client command is stored in the
 *  menuitem structure, in the "label" field.
 *
 *  Valid syntax for a client command (single quoted characters are
 *  literals):
 *
 *  modifier  = { '->' | '=' | '~' }
 *  reference = '<' { name | '*' } '>'
 *  command   = [ modifier ] reference [ { modifier | '.' } reference ]*
 *  name      = alpha-numeric string, white space allowed
 *
 *  Assumes:
 *  --------
 *  There is no leading whitespace on the string argument
 *
 *************************************<->***********************************/

enum { PRS_NO_STATE, PRS_BEGIN, PRS_MODIFIER, PRS_REFERENCE,
       PRS_SEPARATOR, PRS_END, PRS_ERROR, PRS_MAX_STATES };

/* This table lists for each parse state, the legal states that can
   be moved to. Each list must end with a PRS_NO_STATE value to 
   terminate the list. */
static int cmd_parse_table[PRS_END][PRS_END] =
{
  /* PRS_NO_STATE */  { PRS_NO_STATE },
  /* PRS_BEGIN */     { PRS_MODIFIER, PRS_REFERENCE, PRS_NO_STATE },
  /* PRS_MODIFIER */  { PRS_REFERENCE, PRS_NO_STATE },
  /* PRS_REFERENCE */ { PRS_SEPARATOR, PRS_END, PRS_NO_STATE },
  /* PRS_SEPARATOR */ { PRS_REFERENCE, PRS_NO_STATE },
};

static Boolean ParseClientCommand (unsigned char **linePP, MenuSpec *menuSpec,
				   MenuItem *menuItem, unsigned char *string,
				   Boolean *use_separators)
{
    int token, linelen, i;
    int state = PRS_BEGIN;
    String stream, unchanged_stream, exclusion_text;
    Boolean return_val = FALSE;
    Boolean exclusion = FALSE; /* this will be set to TRUE if the client
				  command was parsed to be an exclusion
				  command. */

    /* Construct one input stream out of the string and the linePP that
       we were given. */
    linelen = strlen((char *)string) + strlen((char *)*linePP) + 1;
    if ((unchanged_stream = stream = (String)
	 XtMalloc((unsigned int)(sizeof(unsigned char) * linelen))) == NULL)
    {
	PWarning (((char *)GETMESSAGE(60, 42,
		    "Insufficient memory for menu item label")));
        return (FALSE);
    }
    strcpy(stream, (char *) string);
    strcat(stream, " ");
    strcat(stream, (char *) *linePP);

    for (;;)
    {
	token = PRS_NO_STATE;
	while (token == PRS_NO_STATE)
	{
#ifndef NO_MULTIBYTE
	    if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
	      token = PRS_ERROR;
	      continue;
	    }
#endif

	    switch (*stream)
	    {
	      case '\0':
	      case '\n':
		/* We've reached the end of the stream. Return the
		   PRS_END token. */
		token = PRS_END;
		break;
	      case '-':
		/* This should be a cascade-force modifier */
		++stream;
#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif
		if (*stream == '>')
		{
		    ++stream; token = PRS_MODIFIER;
		}
		else token = PRS_ERROR;
		break;
	      case '=':
		/* This is either a separators modifier or
		   a combination separators and cascade-force
		   modifier */
		++stream;
#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif
		if (*stream == '>') ++stream;
		token = PRS_MODIFIER;
		*use_separators = TRUE;
		break;
	      case '~':
		/* This is a exclude-command modifier */
		++stream; token = PRS_MODIFIER;
		exclusion = TRUE;
		/* Setup a pointer to the text following the ~ so
		   we can do matching later for exclusions. */
		exclusion_text = stream;
		break;
	      case '<':
		/* Skip the open bracket */
		++stream;

		/* This should be the beginning of a reference. First
		   skip any leading whitespace. */
#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif
		while (
#ifndef NO_MULTIBYTE
		       mblen ((char *)stream, MB_CUR_MAX) == 1 &&
#endif
		       (*stream == ' ' || *stream == '\t')) 
		  ++stream;

#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif
		/* Now check for a reference name wild card or a
		   full reference name */
		if (*stream == '*')
		  ++stream;
		else
		{
		    while (
#ifndef NO_MULTIBYTE
			   mblen ((char *)stream, MB_CUR_MAX) == 1 &&
#endif
			   (isalnum(*stream) || *stream == ' ' ||
			    *stream == '\t'  || *stream == '_' ))
		      ++stream;
		}
		
#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif

		/* Now skip past any trailing white space */
		while (
#ifndef NO_MULTIBYTE
		       mblen ((char *)stream, MB_CUR_MAX) == 1 &&
#endif
		       (*stream == ' ' || *stream == '\t'))
		  ++stream;

#ifndef NO_MULTIBYTE
		if (mblen ((char *)stream, MB_CUR_MAX) > 1) {
		  token = PRS_ERROR;
		  continue;
		}
#endif
		/* At this point, we should be looking at the close
		   of the reference */
		if (*stream == '>')
		{
		    token = PRS_REFERENCE;
		    ++stream;
		}
		else token = PRS_ERROR;
		break;
	      case '.':
		/* This is a reference separator */
		++stream; token = PRS_SEPARATOR;
		break;
	      case ' ':
	      case '\t':
		/* The only place white space is allowed as at the
		   beginning of the line, after all the client command
		   text and within the delimiters of a REFERENCE. We
		   are guaranteed not to have whitespace at the 
		   beginning of the line by the time this function is
		   called. Also, the REFERENCE parsing above handles
		   all white space internal to the client command. Therefore,
		   since we are seeing white space, we must be at the
		   end of the client command. */
		token = PRS_END;
		break;
	      default:
		token = PRS_ERROR;

	    } /* end switch (*stream) */
	} /* end while (token == PRS_NO_STATE) */

	/* If we got an error then just return an error */
	if (token == PRS_ERROR)
	{
	    return_val = FALSE; break;
	}
	
	/* Check whether the token we got is a valid transition */
	for (i = 0; cmd_parse_table[state][i] != PRS_NO_STATE; ++i)
	{
	    if (token == cmd_parse_table[state][i]) 
	    {
		/* It is a valid transition, so break out of the loop */
		break;
	    }
        }
	
	/* If i is not indexing the NO_STATE value in the parse_table,
	   then the parse succeeded. Check if the new state is PRS_END.
	   If so then we are done. If the state isn't the same as the
	   current token, then we hit a parse error. */
	if (cmd_parse_table[state][i] != PRS_NO_STATE)
	{
	    if (token == PRS_END)
	    {
		return_val = TRUE;
		break;
	    }
	}
	else 
	{
	    /* parse error */
	    return_val = FALSE;
	    break;
	}
	
	/* The transition was valid so make the transition by
	   setting the state to be the current token. */
	state = token; 

    } /* end for (;;) */

    /* If the return_val is TRUE, then the parse succeeded and we
       want to save the string we parsed into the label field of
       the menu item. */
    if (return_val == TRUE)
    {
	/* NULL terminate the string */
	*stream = '\0';

	/* Check whether this client command was an exclusion. If not,
	   then store the client command string in the menu item. */
	if (exclusion == TRUE)
	{
	    /* Since the command was an exclusion, store the command
	       string in the menuSpec and change the return value to
	       FALSE. */
	    StoreExclusion(menuSpec, exclusion_text);
	    return_val = FALSE;
	}
	else
	{
	    menuItem->label = XtNewString(unchanged_stream);
	    menuItem->labelType = XmSTRING;
	}
    }

    /* Free the string we allocated and return. */
    XtFree((char *)unchanged_stream);

    return(return_val);
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  ParseWmLabel (pSD, menuItem, string)
 *
 *
 *  Description:
 *  -----------
 *  Parse a menu label string.
 *
 *
 *  Inputs:
 *  ------
 *  pSD      = pointer to screen data
 *  menuItem = pointer to MenuItem structure
 *  string   = label string
 *
 * 
 *  Outputs:
 *  -------
 *  menuItem->label
 *  menuItem->labelType
 *  menuItem->labelBitmapCache
 *  Return   = boolean, FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 * We have two label types:  XmSTRING and XmPIXMAP
 * We allocate and fill the label field with string and set the type to 
 * XmSTRING.  If string = "@<bitmap_file>", and <bitmap_file> contains a
 * with which to build a label image we save the bitmap in the MenuItem 
 * structure and set the type to XmPIXMAP.
 * 
 *************************************<->***********************************/

static Boolean ParseWmLabel (WmScreenData *pSD, MenuItem *menuItem, 
			     unsigned char *string)
{

    /*
     * Allocate the label field and copy string.
     */

    if ((menuItem->label = (String)
        XtMalloc ((unsigned int)(strlen ((char *)string) + 1))) == NULL)
    {
        PWarning (((char *)GETMESSAGE(60, 13, "Insufficient memory for menu item")));
        return (FALSE);
    }

    strcpy (menuItem->label, (char *)string);
    menuItem->labelType = XmSTRING;

    if (*string == '@')
    /*
     * Here:  string  = "@<bitmap file>"
     * Try to find the label bitmap in the bitmap cache or read the label 
     * bitmap file.
     */
    {
        string++;  /* skip "@" */
#ifdef WSM
        if ((menuItem->labelBitmapIndex = GetBitmapIndex (pSD, 
					   (char *)string, True)) >= 0)
#else /* WSM */
        if ((menuItem->labelBitmapIndex = GetBitmapIndex (pSD, 
					       (char *)string)) >= 0)
#endif /* WSM */
	{
	    menuItem->labelType = XmPIXMAP;
	}
    }
    return (TRUE);

} /* END OF FUNCTION ParseWmLabel */



/*************************************<->*************************************
 *
 *  ParseWmMnemonic (linePP, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Parse an optional menu function mnemonic.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  menuItem->mnemonic = valid mnemonic character or NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void ParseWmMnemonic (unsigned char **linePP, MenuItem *menuItem)
{
    unsigned char *lineP = *linePP;
    unsigned char *mnemonic;

    /*
     * Skip leading white space.
     */
    ScanWhitespace (&lineP);
    menuItem->mnemonic = (KeySym)NULL;

    if (*lineP == '_')
    /* 
     * We have a mnemonic specification. 
     * Get the next string (we only use the first character).
     * If no string exists, the labelType is not XmSTRING, or does not contain 
     * the first character, then skip the string and return.
     * Otherwise, accept the first character as a mnemonic.
     */
    {
	KeySym ks;
        lineP++;
        mnemonic = GetString(&lineP);

#ifndef NO_MULTIBYTE
	if (menuItem->labelType == XmSTRING &&
	    mnemonic != NULL &&
	    (ks = XStringToKeysym((char *)mnemonic)) != NoSymbol &&
	    strchr(menuItem->label, (char)(ks & 0xff)) != NULL)
	{
	    menuItem->mnemonic = ks;
	}
#else
        if ((mnemonic != NULL) &&
            (*mnemonic != '\0') &&
            (menuItem->labelType == XmSTRING) &&
	    (strchr (menuItem->label, *mnemonic) != NULL))
        /* valid mnemonic */
        {
            menuItem->mnemonic = *mnemonic;
        }
#endif
	else
	{
            PWarning (((char *)GETMESSAGE(60, 14, "Invalid mnemonic specification")));
	}
    }

    *linePP = lineP;  /* consume any string */

} /* END OF FUNCTION ParseWmMnemonic */


/*************************************<->*************************************
 *
 *  ParseWmAccelerator (linePP, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Parse an optional menu function accelerator.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  menuItem->accelText = pointer to an accelerator string or NULL.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseWmAccelerator (unsigned char **linePP, MenuItem *menuItem)
{
    unsigned char *lineP;
    String        string;
    unsigned int  eventType;
    unsigned int  state;
    KeyCode       keycode;
    Boolean       status;

    menuItem->accelState = 0;
    menuItem->accelKeyCode = 0;
    menuItem->accelText = NULL;
    status = TRUE;

    /*
     * If linePP contains NULL, then abort.
     */
    if (*linePP == (unsigned char *) NULL) return(FALSE);

    /*
     * Skip leading white space.
     */
    ScanWhitespace (linePP);
    lineP = *linePP;

    /*
     * If the second character is not ".", and an accelerator specification 
     * exists, then process and save the specification string.
     */

    if ((*lineP != '\0') &&     /* something follows */
	(*lineP != '!')  &&     /* skip if we have the ! WmFunction */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	 /* skip label name for client command */
	((*lineP != '"') || (menuItem->wmFunction != F_InvokeCommand)) &&
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	(*lineP != 'f')  &&
	(*(lineP+1) != '.'))    /* skip if we have f.xxx WmFunction */
    {
        if (ParseKeyEvent(&lineP, &eventType, &keycode, &state))
        {
            if ((string = (String) XtMalloc 
		 ((unsigned int) (lineP - *linePP + 1))) == NULL)
            {
	        PWarning (((char *)GETMESSAGE(60, 15, "Insufficient memory for accelerator specification")));
                status = FALSE;
            }
	    else
	    /*
	     * Save the accelerator state and keycode.
	     * Process and save the accelerator text.
	     */
            {
	        ProcessAccelText (*linePP, lineP, (unsigned char *) string);
                menuItem->accelState = state;
                menuItem->accelKeyCode = keycode;
                menuItem->accelText = string;
            }
        }
	else
	{
	    PWarning (((char *)GETMESSAGE(60, 16, "Invalid accelerator specification")));
            status = FALSE;
	}

        *linePP = lineP;  /* consume the specification */
    }

    return (status);

} /* END OF FUNCTION ParseWmAccelerator */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  ParseMenuItemName (linePP, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Parse a user defined client command menu item
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  menuItem = pointer to MenuItem structure
 *
 * 
 *  Outputs:
 *  -------
 *  menuItem->label will have menu item name appended to it
 *
 *  Comments:
 *  --------
 *  This function attempts to find a menu item label string at the end
 *  of the client command specification line. A menu item label string
 *  must be delimited by double quotes. If found, the label string is
 *  appended to the menuItem->label field, after being reallocated to
 *  accommodate the new space requirement.
 * 
 *************************************<->***********************************/

static void ParseMenuItemName (unsigned char **linePP, MenuItem *menuItem)
{
    unsigned char *lineP, *endquote;
#ifndef NO_MULTIBYTE
    int chlen;
#endif

    /* Skip past any whitespace */
    ScanWhitespace (linePP);
    lineP = *linePP;

    /* Look for a double quote */
    if (
#ifndef NO_MULTIBYTE
	mblen ((char *)lineP, MB_CUR_MAX) == 1 &&
#endif
	*lineP == '"')
    {
	/* Move past the first quote. */
	++lineP;

	endquote = lineP;

	/* Search for closing quote */
#ifndef NO_MULTIBYTE
	while (*endquote != '\0' &&
	       (chlen = mblen ((char *)endquote, MB_CUR_MAX)) > 0 && 
	       (chlen > 1 || *endquote != '"'))
	{
	    /* If we ran off the end of the line, then just abort. Bad
	       syntax. */
	    if ((chlen == 1 && *endquote == '\n') || *endquote == '\0') return;
	    endquote += chlen;
	}
	if (chlen < 0) return; /* invalid character */
#else
	while (*endquote != '\0' && *endquote != '"') {
	  if (*endquote == '\n' || *endquote == '\0') return;
	  endquote++;
	}
#endif

	/* Well, we have a valid menu item name. Store it in the 
	   client command name field. Don't include the double quotes. */
	menuItem->clientCommandName =
	  XtMalloc(sizeof(char) * (endquote - lineP) + 1);
	strncpy(menuItem->clientCommandName, (char *) lineP,
		endquote - lineP);
	menuItem->clientCommandName[strlen(menuItem->clientCommandName)+1] = '\0';
    }
    else
    {
	/* If there was no double quote, then just advance to the end
	   of the line. */
#ifndef NO_MULTIBYTE
	while (*lineP != '\0' && 
	       ((chlen = mblen ((char *)lineP, MB_CUR_MAX)) > 1 ||
		*lineP != '\n'))
	  lineP += chlen > 0 ? chlen : 1;
#else
	while (*lineP != '\0' && *lineP != '\n')
	  lineP++;
#endif
	*linePP = lineP;
    }
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  int
 *  ParseWmFunction (linePP, res_spec, pWmFunction)
 *
 *
 *  Description:
 *  -----------
 *  Parse a button, key, or menu function name and return its function table
 *  index.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  res_spec = resource specification type (key, button, or menu).
 *  pWmFunction = pointer to window manager function destination.
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pWmFunction = pointer to parsed window manager function.
 *  Return = function table index of parsed function.
 *
 *
 *  Comments:
 *  --------
 *  Uses F_Nop if the function name or resource type is invalid.
 * 
 *************************************<->***********************************/

int ParseWmFunction (unsigned char **linePP, unsigned int res_spec, 
			    WmFunction *pWmFunction)
{
    unsigned char *lineP = *linePP;
    unsigned char *string;
    register int  low, mid, high, cmp;

    /*
     * Skip leading white space.
     */
    ScanWhitespace (&lineP);

    /* 
     * Have function string (may be NULL or a comment).
     * Handle the special case of '!' 
     */

    if (*lineP == '!')
    {
	*linePP = ++lineP;
	*pWmFunction = F_Exec;
	return (F_EXEC_INDEX);
    }

    /*
     * Identify the function corresponding to the specified name.
     * Try binary search of the window manager function parse table.
     * Assume f.nop if the function and resource type cannot be matched.
     * This handles NULL and comment strings, bad function names, and functions
     *   in inappropriate resource sets.
     */
    string = GetString (&lineP);
    *linePP = lineP;

    if (string != NULL)
    {
        ToLower (string);
	low = 0;
        high = WMFUNCTIONTABLESIZE - 1;

        while (low <= high)
        {
	    mid = (low + high)/2;
            cmp = strcmp (functionTable[mid].funcName, (char *)string);

            if (!cmp)
	    /*
	     * Function name match 
	     * Require proper resource type for the function.
	     */
	    {
	        if (res_spec & functionTable[mid].resource)
	        {
		    *pWmFunction = functionTable[mid].wmFunction;
	            return (mid);
                }

	        /* invalid resource:  use F_Nop */
  	        break;
            }

	    /*
	     * Function name mismatch 
	     */
            if (cmp > 0)
	    {
	        high = mid - 1;
	    }
	    else
	    {
	        low = mid + 1;
	    }
        }
    }

    /* 
     * Not found:  assume f.nop
     */
    *pWmFunction = F_Nop;
    return (F_NOP_INDEX);

} /* END OF FUNCTION ParseWmFunction */


/*************************************<->*************************************
 *
 *  ParseWmFuncMaybeStrArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function with a null or string argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function (not used).
 *  pArgs = pointer to argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument string.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 *  Only used to parse arguments for F_Lower, F_Raise, and F_Raise_Lower.
 *  If it is used for any other function, be sure to change FreeMenuItem ()
 *  accordingly.
 * 
 *************************************<->***********************************/

#ifndef PANELIST
static
#endif 
Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs)
{
    unsigned char *string = *linePP;
    unsigned int  len;

    ScanWhitespace (&string);
/*
    if (*lineP == '-')
    {
	*linePP = ++lineP;
	return (ParseWmFuncStrArg (linePP, wmFunction, pArgs));
    }
*/
#ifdef PANELIST
#if 0
    else if (*lineP == '"' && *(lineP+1) == '-')
    {
	/* kill off '-' */
	strcpy ((char *) (lineP+1), (char *) (lineP+2));
	return (ParseWmFuncStrArg (linePP, wmFunction, pArgs));
    }
#endif
#endif /* PANELIST */
    if ((len = strlen ((char *)string)) != 0)
    {
	if ((*pArgs = (String)XtMalloc (len + 1)) == NULL)
	{
            PWarning (((char *)GETMESSAGE(60, 17, "Insufficient memory")));
	    return (FALSE);
	}
	strcpy (*pArgs, (char *)string);
	return (TRUE);
    }
    else
    /* Do ParseWmFuncNoArg () */
    {
        *pArgs = NULL;
        return (TRUE);
    }

} /* END OF FUNCTION ParseWmFuncMaybeStrArg */


/*************************************<->*************************************
 *
 *  ParseWmFuncNoArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function null argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function (not used).
 *  pArgs = pointer to argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = unchanged
 *  pArgs    = NULL
 *  Return   = TRUE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseWmFuncNoArg (unsigned char **linePP, WmFunction wmFunction,
				 String *pArgs)
{

    *pArgs = NULL;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncNoArg */


/*************************************<->*************************************
 *
 *  ParseWmFuncStrArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function string argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the argument string is intended.
 *  pArgs = pointer to argument string destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument string.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 *  Insures that an argument for F_Exec() ends in '&' .
 *  Only used to parse arguments for F_Exec, F_Menu, F_Lower, F_Raise, 
 *  F_Raise_Lower, and F_Screen.  If it is used for any other function, be
 *  sure to change FreeMenuItem () accordingly.
 * 
 *************************************<->***********************************/

#ifndef PANELIST
static
#endif
Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs)
{
    unsigned char *string;
    unsigned int  len;
#ifndef NO_MULTIBYTE
    char *p;
    wchar_t last;
    char delim;
    wchar_t wdelim;
    int lastlen;
#endif

    if ((string = GetString (linePP)) != NULL)
    /* nonNULL string argument */
    {
        len = strlen ((char *)string);
        if ((*pArgs = (String)XtMalloc (len + 2)) == NULL)
        {
            PWarning (((char *)GETMESSAGE(60, 17, "Insufficient memory")));
	    return (FALSE);
        }
        strcpy (*pArgs, (char *)string);

        /*
         *  Insure that an argument for F_Exec ends in '&' .
         */

#ifndef NO_MULTIBYTE
	if ((wmFunction == F_Exec))
	{
	    lastlen = 0;
	    p = *pArgs;
	    while (*p &&
		   ((len = mblen(p, MB_CUR_MAX)) > 0))
	    {
		mbtowc(&last, p, MB_CUR_MAX);
		lastlen = len;
		p += len;
	    }
	    delim = '&';
	    mbtowc(&wdelim, &delim, MB_CUR_MAX);
	    if (lastlen == 1 && last != wdelim)
	    {
		*p++ = '&';
		*p   = '\0';
	    }
	}
#else
        if ((wmFunction == F_Exec) && ((*pArgs)[len - 1] != '&'))
        {
	    (*pArgs)[len] = '&';
	    (*pArgs)[len + 1] = '\0';
        }
#endif
    }
    else
    /* NULL string argument */
    {
        *pArgs = NULL;
    }

    return (TRUE);

} /* END OF FUNCTION ParseWmFuncStrArg */


/*************************************<->*************************************
 *
 *  FreeMenuItem (menuItem)
 *
 *
 *  Description:
 *  -----------
 *  This procedure destroys a MenuItem structure.
 *
 *
 *  Inputs:
 *  ------
 *  menuItem = to be destroyed.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  Assumes that ParseWmFuncStrArg () has parsed a menu item's function
 *  argument only for F_Exec, F_Menu, F_Lower, F_Raise, F_Raise_Lower, and
 *  F_Screen. If it is used for other functions, be sure to include them here!
 * 
 *************************************<->***********************************/

void FreeMenuItem (MenuItem *menuItem)
{
    if (menuItem->label != NULL)
    {
        XtFree ((char *)menuItem->label);
    }

    if (menuItem->accelText != NULL)
    {
	XtFree ((char *)menuItem->accelText);
    }

    /*
     * If menuItem->wmFuncArgs is nonNULL, we assume that it is a string that
     * was malloc'ed in ParseWmFuncStrArg () and we free it now.
     */
    if ((menuItem->wmFuncArgs != NULL) &&
        ((menuItem->wmFunction == F_Exec)  || 
         (menuItem->wmFunction == F_Menu)  || 
         (menuItem->wmFunction == F_Lower) || 
         (menuItem->wmFunction == F_Raise) || 
	 (menuItem->wmFunction == F_Raise_Lower) ||
	 (menuItem->wmFunction == F_Screen)))
    {
	XtFree ((char *)menuItem->wmFuncArgs);
    }

    if (menuItem->clientCommandName != NULL)
    {
	XtFree ((char *) menuItem->clientCommandName);
    }

    XtFree ((char *)menuItem);

} /* END OF FUNCTION FreeMenuItem */



/*************************************<->*************************************
 *
 *  ParseWmFuncGrpArg (linePP, wmFunction, pGroup)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function group argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the group argument is intended.
 *  pGroup = pointer to group argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pGroup    = pointer to parsed group argument.
 *  Return   = FALSE iff invalid group argument.
 *
 *
 *  Comments:
 *  --------
 *  The only valid nonNULL arguments are "icon", "window", and "transient".
 * 
 *************************************<->***********************************/

static Boolean ParseWmFuncGrpArg (unsigned char **linePP, 
				  WmFunction wmFunction, GroupArg *pGroup)
{
    unsigned char  *lineP = *linePP;
    unsigned char  *startP;
    unsigned char   grpStr[MAX_GROUP_STRLEN+1];
    int    len;


    /*
     * Parse groups while each is followed by "|".
     */

    *pGroup = 0;
    while (1)
    {
        /* 
	 * Skip whitespace and find next group string.
	 */

        ScanWhitespace (&lineP);
	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* Group missing => use default or complain */
	{
	    if (*pGroup)
	    {
                PWarning (((char *)GETMESSAGE(60, 18, "Missing group specification")));
                return (FALSE);
	    }
	    else
	    {
                *pGroup = F_GROUP_DEFAULT;
		break;
	    }
        }

	/*
	 * Found a group string; compare it with valid groups.
	 */

        len = min (lineP - startP, MAX_GROUP_STRLEN);
        (void) strncpy ((char *)grpStr, (char *)startP, len);
        grpStr[len] = '\0';
        ToLower (grpStr);

        if (!strcmp ("icon", (char *)grpStr))
        {
            *pGroup |= F_GROUP_ICON;
        }
        else if (!strcmp ("window", (char *)grpStr))
        {
            *pGroup |= F_GROUP_WINDOW;
        }
        else if (!strcmp ("transient", (char *)grpStr))
        {
            *pGroup |= F_GROUP_TRANSIENT;
        }
        else 
        /* Unknown group name */
        {
            PWarning (((char *)GETMESSAGE(60, 19, "Invalid group specification")));
            return (FALSE);
        }

        /*
	 *  Continue processing until the line is exhausted.
	 *  Skip any '|' .
	 */

        ScanWhitespace (&lineP);

        if (lineP == NULL || *lineP == '\0')
	{
	    break; 
        }
        else if (*lineP == '|')
	{
            lineP++;
        }
    }

    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncGrpArg */



/*************************************<->*************************************
 *
 *  ParseWmFuncNbrArg (linePP, wmFunction, pNumber)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager function number argument.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function 
 *  pNumber = pointer to number argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pNumber  = pointer to parsed number argument.
 *  Return   = FALSE iff invalid number argument.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseWmFuncNbrArg (unsigned char **linePP, 
				  WmFunction wmFunction, 
				  unsigned long *pNumber)
{
    int  val;

    val = StrToNum (GetString (linePP));
    if (val == -1)
    {
        PWarning (((char *)GETMESSAGE(60, 20, "Invalid number specification")));
        *pNumber = 0;
        return (FALSE);
    }

    *pNumber = val;
    return (TRUE);

} /* END OF FUNCTION ParseWmFuncNbrArg */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  ParseWmFuncCCIArgs (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a Client-Command entry's arguments.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the argument string is intended.
 *  pArgs = pointer to argument string destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument string.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseWmFuncCCIArgs (unsigned char **linePP, 
				   WmFunction wmFunction, String *pArgs)
{
  /*
   * Format:
   *    cci_func_args:
   *         cci_entry
   *         modifier cci_entry_list
   *
   *    cci_entry_list:
   *         cci_entry
   *         cci_entry . cci_entry
   *
   *    cci_entry:
   *         '<' cci_label '>'
   *
   *    cci_label:
   *         any combination of alpha and '_'
   */

  CCIEntryModifier  mod;
  CCIFuncArg      *cciArg;
  unsigned char   *string;


  cciArg = XtNew(CCIFuncArg);

  if ((string = GetString(linePP)) == NULL)
    {
      /* Error - no data for f.cci command. cci_entry_list is required. */
      fprintf(stderr, "Incorrect format for f.cci command.\n");
      return (FALSE);
    }
  else
    {
      /* check if no modifier was specified. */
      if (string[0] == '<')
	{
	  cciArg->mod      = NONE;
	  cciArg->cciEntry = XtNewString((char*)string);
	}
      else
	{
	  if (! GetCCIModifier((String)string, &mod))
	    {
	      cciArg->mod      = NONE;
	      cciArg->cciEntry = XtNewString("");
	    }
	  else
	    {
	      cciArg->mod = mod;

	      if ((string = GetString(linePP)) == NULL)
		{
		  /* Found a modifier, but there's no cci_entry_list. */
		  fprintf(stderr, "Incorrect format for f.cci command.\n");
		  return(FALSE);
		}
	      else
		{
		  cciArg->cciEntry = XtNewString((char*)string);
		}
	    }
	}

      *pArgs = (String)cciArg;
    }

  return(TRUE);
} /* END OF FUNCTION ParseWmFuncCCIArgs */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  ParseButtonStr ()
 *
 *
 *  Description:
 *  -----------
 *  This function parses a button set specification string:
 *
 *     bindings_name
 *     {
 *       button   context   function
 *       button   context   function
 *                 ...
 *       button   context   function
 *     }
 *
 *
 *
 *  Inputs:
 *  ------
 *  pSD->buttonBindings = buttonBindings resource value
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  pSD->buttonSpecs = list of button binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  The button set specification name must match pSD->buttonBindings.
 * 
 *************************************<->***********************************/

void ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr)
{
    unsigned char *lineP;

    cfileP = NULL;
    linec = 0;
    if (((parseP = buttonStr) != NULL) && (GetNextLine () != NULL))
    {
	lineP = line;
        ParseButtonSet (pSD, lineP);
    }

} /* END OF FUNCTION ParseButtonStr */


/*************************************<->*************************************
 *
 *  ParseButtonSet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Button set specification found.  Parse the following syntax:
 *
 *             v
 *     Buttons bindings_name
 *     {
 *       button   context   function
 *       button   context   function
 *                 ...
 *       button   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line =  (global) line buffer
 *  lineP = pointer to current character in line buffer
 *  pSD->buttonBindings = buttonBindings resource value
 *
 * 
 *  Outputs:
 *  -------
 *  lineP = pointer to current character in line buffer
 *  pSD->buttonSpecs = list of button binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed button binding set and sets with names that don't match
 *  the buttonBindings resource.
 *  Skips bad button binding specifications.
 * 
 *************************************<->***********************************/

static void ParseButtonSet (WmScreenData *pSD, unsigned char *lineP)
{
    unsigned char *string;
    ButtonSpec    *buttonSpec;
    ButtonSpec    *lastButtonSpec;
    int            ix;
    
    /*
     * Parse the button set bindings from the configuration file.
     * If either the button set name or buttonBindings resource is NULL or 
     *   they don't match, then skip this button specification.
     */

    if (((string = GetString (&lineP)) == NULL) ||
        (pSD->buttonBindings == NULL) ||
        strcmp ((char *)string, pSD->buttonBindings))
    {
        return;
    }

    /* 
     * Require leading '{' on the next line.
     */
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
        /* found '{' */
        {
            break;
        }

        /* not a '{' */
	PWarning (((char *)GETMESSAGE(60, 21, "Expected '{' after button set name")));
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Prepare to accumulate button bindings by finding the end of
     *   the button specification list.
     * lastButtonSpec will be NULL only if no prior bindings exist.
     */

    lastButtonSpec = pSD->buttonSpecs;
    if (lastButtonSpec != NULL)
    {
        while (lastButtonSpec->nextButtonSpec != NULL)
        {
            lastButtonSpec = (lastButtonSpec->nextButtonSpec);
        }
    }

    /*
     * Parse "button context function"  until "}" or EOF found.
     * Skips bad button binding specifications.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
	lineP = line;
	ScanWhitespace(&lineP);
	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*lineP == '}')  /* finished with button set */
        {
	    break;
        }

	/*
	 * Allocate space for the button binding specification. 
	 */
        if ((buttonSpec = (ButtonSpec *)XtMalloc (sizeof (ButtonSpec))) == NULL)
	{
            PWarning (((char *)GETMESSAGE(60, 22, "Insufficient memory for button specification")));
            continue;
	}
	buttonSpec->wmFunction = (WmFunction)NULL;
	buttonSpec->wmFuncArgs = NULL;
	buttonSpec->nextButtonSpec = NULL;

	/*
	 * Parse the button specification "button".
	 */
	lineP = line;
	if (!ParseBtnEvent(&lineP,
			   &buttonSpec->eventType,
			   &buttonSpec->button,
	                   &buttonSpec->state,
	                   &buttonSpec->click))
	{
            PWarning (((char *)GETMESSAGE(60, 23, "Invalid button specification")));
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}


	/*
	 * Parse the button context.
	 */
	if (!ParseContext(&lineP, &buttonSpec->context, 
			  &buttonSpec->subContext))
	{
            PWarning (((char *)GETMESSAGE(60, 24, "Invalid button context")));
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}

        /*
	 * Parse the button function and any arguments.
	 */

	ix = ParseWmFunction (&lineP, CRS_BUTTON, &buttonSpec->wmFunction);

	/*
	 * remove any subContexts that don't apply to this function
	 */

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_IICON) &&
	    (buttonSpec->subContext & F_SUBCONTEXT_IB_IICON))
	{
	    buttonSpec->subContext &= ~F_SUBCONTEXT_IB_IICON;
	}

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_WICON) &&
	    (buttonSpec->subContext & F_SUBCONTEXT_IB_WICON))
	{
	    buttonSpec->subContext &= ~F_SUBCONTEXT_IB_WICON;
	}

	/*
	 * Map Button3 menus to BMenu virtual button
	 */
        if (buttonSpec->button == Button3 && 
	   (buttonSpec->wmFunction == F_Menu ||
	    buttonSpec->wmFunction == F_Post_SMenu)) {

	    buttonSpec->button = wmGD.bMenuButton;
	}

        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, buttonSpec->wmFunction, &buttonSpec->wmFuncArgs))
        {
	    XtFree ((char *)buttonSpec);
	    continue;  /* skip this button specification */
	}

        /* 
	 * Add the button specification to the button specification list.
	 */
        if (lastButtonSpec != NULL)
	/* a prior specification exists */
        {
            lastButtonSpec->nextButtonSpec = buttonSpec;
        }
	else
        {
            pSD->buttonSpecs = buttonSpec;
        }
        lastButtonSpec = buttonSpec;
    }

} /* END OF FUNCTION ParseButtonSet */



/*************************************<->*************************************
 *
 *  ParseContext (linePP, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  Parses a general context string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  context =    context field value
 *  subContext = subContext field value
 *  Return = (Boolean) true iff valid context string
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseContext (unsigned char **linePP, Context *context, 
			     Context *subContext)
{
    unsigned char   *lineP = *linePP;
    unsigned char   *startP;
    unsigned char    ctxStr[MAX_CONTEXT_STRLEN+1];
    int     len;

    /*
     * Parse contexts while each is followed by "|".
     */

    *context = 0;
    *subContext = 0;
    while (1) {

        /* 
	 * Skip whitespace and find next context string.
	 */
        ScanWhitespace (&lineP);
	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* ERROR: Context missing */
	{
           return (FALSE);
        }

	/*
	 * Found nonNULL string; compare it with valid contexts.
	 */

        len = min(lineP - startP, MAX_CONTEXT_STRLEN);
        (void) strncpy ((char *)ctxStr, (char *)startP, len);
        ctxStr[len] = '\0';
        ToLower (ctxStr);

        if (!strcmp ("root", (char *)ctxStr))
        {
            *context |= F_CONTEXT_ROOT;
	    *subContext |= F_SUBCONTEXT_R_ALL;
        }
        else if (!strcmp ("icon", (char *)ctxStr))
        {
            *context |= (F_CONTEXT_ICON        |
			 F_CONTEXT_ICONBOX     |
			 F_SUBCONTEXT_IB_IICON | 
			 F_SUBCONTEXT_IB_WICON );
	    *subContext |= (F_SUBCONTEXT_I_ALL    | 
			    F_SUBCONTEXT_IB_IICON | 
			    F_SUBCONTEXT_IB_WICON );
        }
        else if (!strcmp ("window", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_ALL;
        }
        else if (!strcmp ("frame", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_FRAME;
        }
        else if (!strcmp ("title", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_TITLE;
        }
        else if (!strcmp ("border", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_BORDER;
        }
        else if (!strcmp ("app", (char *)ctxStr))
        {
            *context |= F_CONTEXT_WINDOW;
	    *subContext |= F_SUBCONTEXT_W_APP;
        }
#ifdef WSM
        else if (!strcmp ("ifkey", (char *)ctxStr))
        {
            *context |= F_CONTEXT_IFKEY;
        }
#endif /* WSM */
        else 
        /* Unknown context name */
        {
           return (FALSE);
        }

        /* continue only if followed by '|' */
        ScanWhitespace (&lineP);
        if (*lineP != '|')
	{
	    break; 
        }
        lineP++;
    }

    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseContext */


/*************************************<->*************************************
 *
 *  ParseKeyStr ()
 *
 *
 *  Description:
 *  -----------
 *  This function parses a key set specification string:
 *
 *     bindings_name
 *     {
 *        key   context   function
 *        key   context   function
 *                     ...
 *        key   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  pSD->keyBindings = keyBindings resource value
 *  functionTable = window manager function parse table
 *
 * 
 *  Outputs:
 *  -------
 *  pSD->keySpecs = list of key binding specification
 *
 *
 *  Comments:
 *  --------
 *  The key set specification name must match pSD->keyBindings.
 * 
 *************************************<->***********************************/

void
ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr)
{
    unsigned char *lineP;

    cfileP = NULL;
    linec = 0;
    if (((parseP = keyStr) != NULL) && (GetNextLine () != NULL))
    {
	lineP = line;
        ParseKeySet (pSD, lineP);
    }

} /* END OF FUNCTION ParseKeyStr */



/*************************************<->*************************************
 *
 *  ParseKeySet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *  Key set specification found.  Parse the following syntax:
 *
 *          v
 *     Keys bindings_name
 *     {
 *        key   context   function
 *        key   context   function
 *                     ...
 *        key   context   function
 *     }
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line =  (global) line buffer
 *  lineP = pointer to current character in line buffer
 *  pSD->keyBindings = keyBindings resource value
 *
 * 
 *  Outputs:
 *  -------
 *  lineP = pointer to current character in line buffer
 *  pSD->keySpecs = list of key binding specifications.
 *
 *
 *  Comments:
 *  --------
 *  Skips unnamed key binding set and sets with names that don't match the 
 *  keyBindings resource.
 *  Skips bad key binding specifications.
 * 
 *************************************<->***********************************/

static void ParseKeySet (WmScreenData *pSD, unsigned char *lineP)
{
    unsigned char         *string;
    KeySpec      *keySpec;
    KeySpec      *lastKeySpec;
    unsigned int  eventType;
    int           ix;
#ifdef WSM
    Boolean 	bBadKey;
#endif /* WSM */
    
    /*
     * Parse the key set bindings from the configuration file.
     * If either the key set name or keyBindings resource is NULL or they
     *   don't match then skip this key specification.
     */

    if (((string = GetString (&lineP)) == NULL) ||
        (pSD->keyBindings == NULL) ||
        strcmp ((char *)string, pSD->keyBindings))
    {
        return;
    }

    /*
     * Require leading '{' on next line.
     */
    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
	/* found '{' */
        {
            break;
        }

	/* not a '{' */
	PWarning (((char *)GETMESSAGE(60, 25, "Expected '{' after key set name")));
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Prepare to accumulate key bindings by finding the end of
     *   the key specification list.
     * lastKeySpec will be NULL only if no prior bindings exist.
     */

    lastKeySpec = pSD->keySpecs;
    if (lastKeySpec != NULL)
    {
        while (lastKeySpec->nextKeySpec != NULL)
        {
            lastKeySpec = (lastKeySpec->nextKeySpec);
        }
    }

    /*
     * Parse "key context function"  until "}" or EOF found.
     * Skip bad key bindings.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
	lineP = line;
	ScanWhitespace (&lineP);
	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*lineP == '}')  /* finished with key set */
        {
	    break;
        }

	/*
	 * Allocate space for the key specification.
	 */
        if ((keySpec = (KeySpec *)XtMalloc (sizeof (KeySpec))) == NULL)
	{
            PWarning (((char *)GETMESSAGE(60, 26, "Insufficient memory for key specification")));
            continue;
	}

	keySpec->wmFunction = (WmFunction)NULL;
	keySpec->wmFuncArgs = NULL;
	keySpec->nextKeySpec = NULL;

	/*
	 * Parse the key specification.
	 */
#ifdef WSM
	bBadKey = False;
#endif /* WSM */
	if (!ParseKeyEvent(&lineP,
			   &eventType,
			   &keySpec->keycode,
	                   &keySpec->state))
	{
#ifdef WSM
	    bBadKey = True;
#else /* WSM */
            PWarning (((char *)GETMESSAGE(60, 27, "Invalid key specification")));
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
#endif /* WSM */
	}

	/*
	 * Parse the key context.
	 *   Here lineP points to the candidate context string.
	 */

	if (!ParseContext(&lineP, &keySpec->context, 
			  &keySpec->subContext))
	{
#ifdef WSM
	    if (bBadKey)
		PWarning (((char *)GETMESSAGE(60, 27, "Invalid key specification")));
#endif /* WSM */
            PWarning (((char *)GETMESSAGE(60, 28, "Invalid key context")));
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}
#ifdef WSM
	if (bBadKey)
	{
	    /*
	     * Don't print an error message if this is a "hardware
	     * available" binding.
	     */
	    if (!(keySpec->context & F_CONTEXT_IFKEY))
		PWarning (((char *)GETMESSAGE(60, 27, "Invalid key specification")));
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}

	/*
	 * This flag is only used for parsing, clear it so the
	 * rest of the program doesn't see it.
	 */
	keySpec->context &= ~F_CONTEXT_IFKEY;
#endif /* WSM */


        /*
	 * Parse the key function and any arguments.
	 */

	ix = ParseWmFunction (&lineP, CRS_KEY, &keySpec->wmFunction);
	
	/*
	 * remove any subContexts that don't apply to this function
	 */
	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_IICON) &&
	    (keySpec->subContext & F_SUBCONTEXT_IB_IICON))
	{
	    keySpec->subContext &= ~F_SUBCONTEXT_IB_IICON;
	}

	if ((functionTable[ix].greyedContext & F_SUBCONTEXT_IB_WICON) &&
	    (keySpec->subContext & F_SUBCONTEXT_IB_WICON))
	{
	    keySpec->subContext &= ~F_SUBCONTEXT_IB_WICON;
	}

        /* 
	 * Apply the function argument parser.
	 */
        if (!(*(functionTable [ix].parseProc)) 
		   (&lineP, keySpec->wmFunction, &keySpec->wmFuncArgs))
        {
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}

        /* 
	 * Add the key specification to the key specification list. 
	 */
        if (lastKeySpec != NULL)
	/* a prior specification exists */
        {
            lastKeySpec->nextKeySpec = keySpec;
        }
	else
        {
            pSD->keySpecs = keySpec;
        }
        lastKeySpec = keySpec;
    }

} /* END OF FUNCTION ParseKeySet */

#ifndef WSM

/*************************************<->*************************************
 *
 *  GetNextLine ()
 *
 *
 *  Description:
 *  -----------
 *  Returns the next line from an fopened configuration file or a newline-
 *  embedded configuration string.
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *
 * 
 *  Outputs:
 *  -------
 *  line =    (global) next line 
 *  linec =   (global) line count incremented
 *  parseP =  (global) parse string pointer incremented
 *  Return =  line or NULL if file or string is exhausted.
 *
 *
 *  Comments:
 *  --------
 *  If there are more than MAXLINE characters on a line in the file cfileP the
 *  excess are truncated.  
 *  Assumes the line buffer is long enough for any parse string line.
 * 
 *************************************<->***********************************/

unsigned char *
GetNextLine (void)
{
    register unsigned char	*string;
    int				len;

#ifndef NO_MULTIBYTE
    int   chlen;
    wchar_t last;
    wchar_t wdelim;
    char delim;
    int lastlen;
#endif

    if (cfileP != NULL)
    /* read fopened file */
    {
	if ((string = (unsigned char *) 
		      fgets ((char *)line, MAXLINE, cfileP)) != NULL)
	{
#ifndef NO_MULTIBYTE

	    lastlen = 0;
	    while (*string &&
		   ((len = mblen((char *)string, MB_CUR_MAX)) > 0))
	    {
		mbtowc(&last, (char *)string, MB_CUR_MAX);
		lastlen = len;
		string += len;
	    }
	    delim = '\\';
	    mbtowc(&wdelim, &delim, MB_CUR_MAX);
	    if (lastlen == 1 && last == wdelim)
	    {
		do
		{
		    if (!fgets((char *)string, MAXLINE - (string - line), cfileP))
			break;

		    lastlen = 0;
		    while (*string &&
			   ((len = mblen((char *)string, MB_CUR_MAX)) > 0))
		    {
			mbtowc(&last, (char *)string, MB_CUR_MAX);
			lastlen = len;
			string += len;
		    }
		    linec++;
		}
		while (lastlen == 1 && last == wdelim);
	    }
	    string = line;
#else
	    len = strlen((char *)string) - 2;
	    if ((len > 0) && string[len] == '\\')
	    {
		do {
		    string = &string[len];
		    if (fgets((char *)string, 
		 	      MAXLINE - (string-line), cfileP) == NULL)
		       break;
		    len = strlen((char *)string) - 2;
		    linec++;
		} while ((len >= 0) && string[len] == '\\');
		string = line;
	    }
#endif
	}
    }
    else if ((parseP != NULL) && (*parseP != '\0'))
    /* read parse string */
    {
	string = line;
#ifndef NO_MULTIBYTE
#ifdef FIX_1127
	chlen = mblen((char *)parseP, MB_CUR_MAX);
	if(chlen==-1) string = NULL;
#endif

	while ((*parseP != '\0') &&
               ((chlen = mblen ((char *)parseP, MB_CUR_MAX)) > 0) &&
	       (*parseP != '\n'))
	/* copy all but NULL and newlines to line buffer */
	{
	    while (chlen--)
	    {
	        *(string++) = *(parseP++);
	    }
        }
#else
	while ((*parseP != '\0') && (*parseP != '\n'))
	/* copy all but end-of-line and newlines to line buffer */
	{
	    *(string++) = *(parseP++);
    }
#endif
	if (string)
	    *string = '\0';
	if (*parseP == '\n')
	{
	    parseP++;
	}
    }
    else
    {
	string = NULL;
    }

    linec++;
    return (string);

} /* END OF FUNCTION GetNextLine */
#endif /* WSM */

#ifndef PANELIST

#ifdef WSM
/*************************************<->*************************************
 *
 *  GetStringC (linePP, SmBehavior)
 *
 *
 *  Description:
 *  -----------
 *  Returns the next quoted or whitespace-terminated nonquoted string in the
 *  line buffer.
 *  Additional functionality added to GetString in that anything in a
 *  quoted string is considered sacred and nothing will be stripped from
 *  the middle of a quoted string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 *  SmBehavior = flag that enables parsing session manager hints
 *               if True.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  Return =  string 
 *
 *
 *  Comments:
 *  --------
 *  May alter the line buffer contents.
 *  Handles quoted strings and characters, removing trailing whitespace from
 *  quoted strings.
 *  Returns NULL string if the line is empty or is a comment.
 *  Code stolen from dtmwm.
 * 
 *************************************<->***********************************/
#else /* WSM */
/*************************************<->*************************************
 *
 *  GetString (linePP)
 *
 *
 *  Description:
 *  -----------
 *  Returns the next quoted or whitespace-terminated nonquoted string in the
 *  line buffer.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  pointer to current line buffer pointer.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =  pointer to revised line buffer pointer.
 *  Return =  string 
 *
 *
 *  Comments:
 *  --------
 *  May alter the line buffer contents.
 *  Handles quoted strings and characters, removing trailing whitespace from
 *  quoted strings.
 *  Returns NULL string if the line is empty or is a comment.
 * 
 *************************************<->***********************************/
#endif /* WSM */

#ifdef WSM
unsigned char *GetStringC (unsigned char **linePP, Boolean SmBehavior)
#else /* WSM */
unsigned char *GetString (unsigned char **linePP)
#endif /* WSM */
{
    unsigned char *lineP = *linePP;
    unsigned char *endP;
    unsigned char *curP;
    unsigned char *lnwsP;
#ifdef WSM
    unsigned int  level = 0, checkLev, i, quoteLevel[10];
#endif /* WSM */
#ifndef NO_MULTIBYTE
    int            chlen;

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /*
     * Return NULL if line is empty, a comment, or invalid.
     */
#ifdef WSM
    if (
	*lineP == '\0' ||
	((chlen = mblen ((char *)lineP, MB_CUR_MAX)) < 1) ||
        ((chlen == 1) && ((*lineP == '!') || 
			  ((!SmBehavior) && (*lineP == '#'))))
       )
#else /* WSM */
    if (
	*lineP == '\0' ||
	((chlen = mblen ((char *)lineP, MB_CUR_MAX)) < 1) ||
        ((chlen > 0) && ((*lineP == '!') || (*lineP == '#')))
       )
#endif /* WSM */
    {
        *linePP = lineP;
        return (NULL);
    }

    if ((chlen == 1) && (*lineP == '"'))
    /* Quoted string */
    {
#ifdef WSM
	quoteLevel[level] = 1;	
#endif /* WSM */
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
	       ((chlen > 1) || (*curP != '"')))
	/* Haven't found matching quote yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0))
	    /* character quote:
	     * copy first byte of quoted nonNULL character down.
	     * point curP to next byte
	     */
	    {
#ifdef WSM
		if (SmBehavior)
		{
		    /*
		     * Check to see if this is a quoted quote - if it is
		     * strip off a level - if not - it's sacred leave it alone
		     */
		    checkLev = PeekAhead((curP - 1), quoteLevel[level]);
		    if(checkLev > 0)
		    {
			if(quoteLevel[level] <= checkLev)
			{
			    level--;
			}
			else
			{
			    level++;
			    quoteLevel[level] = checkLev;
			}
			
			for(i = 0;i < (checkLev - 2);i++)
			{
			    *endP++ = *curP++;curP++;
			}
			*endP = *curP++;
		    }
		}
		else 
		{
#endif /* WSM */
		*endP = *curP++;
#ifdef WSM
		}
#endif /* WSM */
            }

	    if (chlen == 1)
	    /* Singlebyte character:  character copy finished. */
	    {
	        if (isspace (*endP))
	        /* whitespace character:  leave lnwsP unchanged. */
	        {
	            endP++;
	        }
	        else
	        /* non-whitespace character:  point lnwsP to it. */
	        {
	            lnwsP = endP++;
	        }
	    }
	    else if (chlen > 1)
	    /* Multibyte (nonwhitespace) character:  point lnwsP to it.
	     * Finish character byte copy.
	     */
	    {
	        lnwsP = endP++;
		while (--chlen)
		{
		    *endP++ = *curP++;
		    lnwsP++;
		}
	    }
        }
#else

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

#ifdef WSM
    /* Return NULL if line is empty, whitespace, or begins with a comment. */
    if ((lineP == NULL || *lineP == '\0') ||
	(!SmBehavior && (*lineP == '#')))
#else /* WSM */
    /* Return NULL if line is empty, whitespace, or begins with a comment. */
    if ((lineP == NULL) || (*lineP == '\0') || (*lineP == '#'))
#endif /* WSM */
    {
        *linePP = lineP;
        return (NULL);
    }

    if (*lineP == '"')
    /* Quoted string */
    {
#ifdef WSM
	quoteLevel[level] = 1;	
#endif /* WSM */
	/*
	 * Start beyond double quote and find the end of the quoted string.
	 * '\' quotes the next character.
	 * Otherwise,  matching double quote or NULL terminates the string.
	 *
	 * We use lnwsP to point to the last non-whitespace character in the
	 * quoted string.  When we have found the end of the quoted string,
	 * increment lnwsP and if lnwsP < endP, write NULL into *lnwsP.
	 * This removes any trailing whitespace without overwriting the 
	 * matching quote, needed later.  If the quoted string was all 
	 * whitespace, then this will write a NULL at the beginning of the 
	 * string that will be returned -- OK.
	 */
	lnwsP = lineP++;                /* lnwsP points to first '"' */
	curP = endP = lineP;            /* other pointers point beyond */

        while ((*endP = *curP) && (*endP != '"'))
	/* haven't found matching quote yet */
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != '\0'))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
#ifdef WSM
		if (SmBehavior)
		{
		    /*
		     * Check to see if this is a quoted quote - if it is
		     * strip off a level - if not - it's sacred leave it alone
		     */
		    checkLev = PeekAhead((curP - 1), quoteLevel[level]);
		    if(checkLev > 0)
		    {
			if(quoteLevel[level] <= checkLev)
			{
			    level--;
			}
			else
			{
			    level++;
			    quoteLevel[level] = checkLev;
			}
			
			for(i = 0;i < (checkLev - 2);i++)
			{
			    *endP++ = *curP++;curP++;
			}
			*endP = *curP++;
		    }
		}
		else 
		{
#endif /* WSM */
		*endP = *curP++;
#ifdef WSM
		}
#endif /* WSM */
            }
	    if (isspace (*endP))
	    /* whitespace character:  leave lnwsP unchanged. */
	    {
	        endP++;
	    }
	    else
	    /* non-whitespace character:  point lnwsP to it. */
	    {
	        lnwsP = endP++;
	    }
        }
#endif

	/*
	 *  Found matching quote or NULL.  
	 *  NULL out any trailing whitespace.
	 */

	lnwsP++;
	if (lnwsP < endP)
        {
	    *lnwsP = '\0';
        }
    }

    else
    /* Unquoted string */
    {
        /* 
	 * Find the end of the nonquoted string.
	 * '\' quotes the next character.
	 * Otherwise,  whitespace, end-of-line, or '#' terminates the string.
	 */
        curP = endP = lineP;

#ifndef NO_MULTIBYTE
#ifdef WSM
        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
               ((chlen > 1) || (!isspace (*curP) && 
			        (SmBehavior || (*curP != '#')))))
#else /* WSM */
        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
               ((chlen > 1) || (!isspace (*curP) && (*curP != '#'))))
#endif /* WSM */
	/* Haven't found whitespace or '#' yet.
	 * First byte of next character has been copied to endP.
	 */
        {
	    curP++;
	    if ((chlen == 1) && (*endP == '\\') && 
		((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0))
	    /* character quote:
	     * copy first byte of quoted nonNULL character down.
	     * point curP to next byte
	     */
	    {
		*endP = *curP++;
            }
	    endP++;
	    if (chlen > 1)
	    /* Multibyte character:  finish character copy. */
	    {
		while (--chlen)
		{
		    *endP++ = *curP++;
		}
	    }
        }
#else
#ifdef WSM
        while ((*endP = *curP) && !isspace (*endP) && 
					(SmBehavior || (*endP != '#')))
#else /* WSM */
        while ((*endP = *curP) && !isspace (*endP) && (*endP != '#'))
#endif /* WSM */
        {
	    /* point curP to next character */
	    curP++;
	    if ((*endP == '\\') && (*curP != '\0'))
	    /* shift quoted nonNULL character down and curP ahead */
	    {
		*endP = *curP++;
            }
	    endP++;
        }
#endif
    }

    /*
     * Three cases for *endP:
     *   '#' --> write NULL over # and point to NULL
     *   whitespace or
     *     matching quote -> write end-of-line over char and point beyond
     *   NULL -> point to NULL 
     */

#ifdef WSM
    if (!SmBehavior && (*endP == '#'))
#else /* WSM */
    if (*endP == '#')
#endif /* WSM */
    {
	*endP = '\0';       /* write '\0' over '#' */
	*linePP = endP;     /* point to '\0' */
    }
    else if (*endP != '\0')
    {
	*endP = '\0';       /* write NULL over terminator */
	*linePP = ++curP;   /* point beyond terminator */
    }
    else
    {
	*linePP = endP;
    }
    return ((unsigned char *)lineP);

} /* END OF FUNCTION GetString */
#endif /* PANELIST */



/*************************************<->*************************************
 *
 *  ParseBtnEvent (linePP, eventType, button, state, fClick)
 *
 *
 *  Description:
 *  -----------
 *  Parse a button event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer
 *  buttonEvents =    (global) button event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  button =    parsed button number
 *  state =     composite modifier mask
 *  fClick =    is click?
 *
 *  Return = (Boolean) true iff valid button event specification
 * 
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

Boolean ParseBtnEvent (unsigned char  **linePP,
		       unsigned int *eventType,
		       unsigned int *button,
		       unsigned int *state,
		       Boolean      *fClick)
{
    if (!ParseEvent (linePP, buttonEvents, eventType, button, state, fClick))
    {
       return (FALSE);
    }

    /* 
     * The following is a fix for an X11 deficiency in regards to 
     * modifiers in grabs.
     */
    if (*eventType == ButtonRelease)
    {
	/* the button that is going up will always be in the modifiers... */
	*state |= buttonModifierMasks[*button];
    }

    return (TRUE);

} /* END OF FUNCTION ParseBtnEvent */



/*************************************<->*************************************
 *
 *  ParseKeyEvent (linePP, eventType, keyCode, state)
 *
 *
 *  Description:
 *  -----------
 *  Parse a key event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer
 *  keyEvents =       (global) key event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  keyCode =   parsed KeyCode
 *  state =     composite modifier mask
 *
 *  Return = (Boolean) true iff valid key event specification
 * 
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state)


{
    Boolean      fClick;
    unsigned int keySym = 0;

    if (!ParseEvent (linePP, keyEvents, eventType, &keySym, state, &fClick))
    {
       return (FALSE);
    }

    /* 
     * Here keySym is a KeySym.  Convert it to a KeyCode.
     * KeyCode will be set to 0 if keySym is not defined for any KeyCode
     *  (e.g. 0x001).
     */

    *keyCode = XKeysymToKeycode(DISPLAY, (KeySym) keySym);

    if (*keyCode == 0)
    {
        if (keySym == XK_F9)
        {
	    keySym = XK_KP_F1;
        }
        else if (keySym == XK_F10)
        {
	    keySym = XK_KP_F2;
        }
        else if (keySym == XK_F11)
        {
	    keySym = XK_KP_F3;
        }
        else if (keySym == XK_F12)
        {
	    keySym = XK_KP_F4;
        }
        *keyCode = XKeysymToKeycode(DISPLAY, (KeySym) keySym);
    }

    return (*keyCode != 0);

} /* END OF FUNCTION ParseKeyEvent */


/*************************************<->*************************************
 *
 *  ParseEvent (linePP, table, eventType, detail, state, fClick)
 *
 *
 *  Description:
 *  -----------
 *  Parse an event specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =          pointer to current line buffer pointer.
 *  table =           event parse table
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  eventType = type of event
 *  detail =    dependent upon parse table detail procedure and closure
 *  state =     composite modifier mask
 *  fClick =    click flag
 *
 *  Return = (Boolean) true iff valid event specification
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseEvent (unsigned char **linePP, EventTableEntry *table,
			   unsigned int *eventType, unsigned int *detail,
			   unsigned int *state, Boolean *fClick)
{
    unsigned char    *lineP = *linePP;
    Cardinal ix;
    Boolean  status;
 
    /* Parse the modifiers */
    if (!ParseModifiers (&lineP, state) || *lineP != '<')
    {
       return (FALSE);
    }
    lineP++;  /* skip '<' */

    /* Parse the event type */
    if (!ParseEventType (&lineP, table, eventType, &ix) || *lineP != '>') 
    {
       return (FALSE);
    }
    lineP++;  /* skip '>' */

    /*
     *  Compute detail and fClick.
     *  Status will be False for a invalid KeySym name.
     */
    status = (*(table[ix].parseProc))(&lineP, table[ix].closure, detail);
    *fClick = table[ix].fClick;

    if (status)
    {
        *linePP = lineP;
    }
    return (status);

} /* END OF FUNCTION ParseEvent */



/*************************************<->*************************************
 *
 *  ParseModifiers(linePP, state)
 *
 *
 *  Description:
 *  -----------
 *  Parses a modifier specification.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer.
 *  modifierStrings = (global) modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP =    pointer to revised line buffer pointer.
 *  state  = composite modifier mask
 *  Return = (Boolean) true iff valid modifier name
 *
 *
 *  Comments:
 *  --------
 *  If successful, will be followed by NULL or '<'.
 * 
 *************************************<->***********************************/

static Boolean ParseModifiers(unsigned char **linePP, unsigned int *state)
{
    unsigned char         *lineP = *linePP;
    unsigned char         *startP;
    unsigned char          modStr[MAX_MODIFIER_STRLEN+1];
    Boolean       fNot;
    unsigned int  maskBit;
    int           len;

    *state = 0;
 
    /*
     * Parse modifiers until the event specifier is encountered.
     */

    ScanWhitespace (&lineP);
    while ((*lineP != '\0') && (*lineP != '<'))
    {
        if (*lineP == '~') 
	{
            fNot = TRUE;
            lineP++;
        }
	else 
	{
            fNot = FALSE;
	}

	startP = lineP;
        ScanAlphanumeric (&lineP);
        if (startP == lineP)
        /* ERROR: Modifier or '<' missing */
	{
            return (FALSE);
        }
        len = min(lineP - startP, MAX_MODIFIER_STRLEN);
        (void) strncpy ((char *)modStr, (char *)startP, len);
        modStr[len] = '\0';

        if (!LookupModifier (modStr, &maskBit))
        /* Unknown modifier name */
        {
            return (FALSE);
        }

	if (fNot) 
	{
            *state &= ~maskBit;
        }
	else 
	{
            *state |= maskBit;
        }
        ScanWhitespace(&lineP);
    }

    *linePP = lineP;
    return (TRUE);  /* must have '<' or NULL following */

} /* END OF FUNCTION ParseModifiers */


/*************************************<->*************************************
 *
 *  LookupModifier (name, valueP)
 *
 *
 *  Description:
 *  -----------
 *  Return the modifier mask for the provided modifier name.
 *
 *
 *  Inputs:
 *  ------
 *  name = modifier string
 *  modifierStrings = modifier string/mask table
 *
 * 
 *  Outputs:
 *  -------
 *  valueP = modifier mask
 *  Return = (Boolean) true iff valid modifier name
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean LookupModifier (unsigned char *name, unsigned int *valueP)
{
    register int i;

    if (name != NULL)
    {
        ToLower (name);
        for (i=0; modifierStrings[i].name != NULL; i++)
	{
	    if (!strcmp (modifierStrings[i].name, (char *)name))
            {
	        *valueP = modifierStrings[i].mask;
	        return (TRUE);
            }
        }
    }

    return (FALSE);

} /* END OF FUNCTION LookupModifier */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  GetCCIModifier (modString, mod)
 *
 *
 *  Description:
 *  -----------
 *  Return the cci modifier corresponding to the specified string
 *
 *
 *  Inputs:
 *  ------
 *  modString = cci modifier string; may be null
 *
 * 
 *  Outputs:
 *  -------
 *  mod    = cci modifier.
 *  Return = (Boolean) true iff valid modifier string
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean GetCCIModifier (String modString, CCIEntryModifier *mod)
{
  CCIEntryModifier i;


  if (modString != NULL)
    {
      ToLower ((unsigned char *)modString);
      for (i=NONE; i<=EXCLUDE; i++)
	{
	  if (!strcmp (CCIEntryModifierNames[i], modString))
            {
	      *mod = i;
	      return (TRUE);
            }
        }
    }

    return (FALSE);

} /* END OF FUNCTION GetCCIModifier */


/*************************************<->*************************************
 *
 *  FixMenuItem (menuSpec, menuItem)
 *
 *
 *  Description:
 *  -----------
 *  Fix-up the menuItem so that it appears an old-style cci command was
 *  read from the .mwmrc file
 *
 *
 *  Inputs:
 *  ------
 *  menuItem = the menu item structure
 *  menuSpec = the menu specification structure
 *
 * 
 *  Outputs:
 *  -------
 *  menuItem = the fixed-up menuitem
 *  menuSpec = the fixed-up menu specification structure if EXCLUDE found
 *  Return   = nothing
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

static void FixMenuItem (MenuSpec *menuSpec, MenuItem *menuItem)
{
  String      tmp;
  CCIFuncArg *cciArg;
  

  if (menuItem == NULL)
    return;

  cciArg = (CCIFuncArg *)menuItem->wmFuncArgs;

  menuItem->clientCommandName = menuItem->label;
  menuItem->label = cciArg->cciEntry;

  /*
   * Fix-up the label to handle the modifier.
   */

  switch (cciArg->mod)
    {
    case NONE:
      break;

    case INLINE:
      break;

    case CASCADE:
      /* -> */
      tmp = (String) XtMalloc(strlen(menuItem->label) + 3);
      sprintf(tmp, "->%s", menuItem->label);
      XtFree(menuItem->label);
      menuItem->label = tmp;
      break;

    case DELIMIT:
      /* = */
      tmp = (String) XtMalloc(strlen(menuItem->label) + 2);
      sprintf(tmp, "=%s", menuItem->label);
      XtFree(menuItem->label);
      menuItem->label = tmp;
      break;

    case DELIMIT_INLINE:
      break;

    case DELIMIT_CASCADE:
      /* => */
      tmp = (String) XtMalloc(strlen(menuItem->label) + 3);
      sprintf(tmp, "=>%s", menuItem->label);
      XtFree(menuItem->label);
      menuItem->label = tmp;
      break;

    case EXCLUDE:
      /* ~ */
      StoreExclusion(menuSpec, menuItem->label);

      tmp = (String) XtMalloc(strlen(menuItem->label) + 2);
      sprintf(tmp, "~%s", menuItem->label);
      XtFree(menuItem->label);
      menuItem->label = tmp;
      break;
    }
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  ParseEventType(linePP, table, eventType, ix)
 *
 *
 *  Description:
 *  -----------
 *  Parses the event type string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer.
 *  table =  event parse table
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = pointer to revised line buffer pointer.
 *  eventType = type of event
 *  ix = table index for matched event
 *
 *  Return = (Boolean) true iff valid event
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseEventType (unsigned char **linePP, EventTableEntry *table,
			       unsigned int *eventType, Cardinal *ix)
{
    unsigned char *lineP = *linePP;
    unsigned char *startP = *linePP;
    unsigned char eventTypeStr[MAX_EVENTTYPE_STRLEN+1];
    register int  len;

    /* Parse out the event string */
    ScanAlphanumeric (&lineP);

    /*
     * Attempt to match the parsed event against our supported event set.
     */

    if (startP != lineP)
    {
        len = min (lineP - startP, MAX_EVENTTYPE_STRLEN);
        (void) strncpy ((char *)eventTypeStr, (char *)startP, len);
        eventTypeStr[len] = '\0';
        ToLower (eventTypeStr);

        for (len = 0; table[len].event != NULL; len++)
            if (!strcmp (table[len].event, (char *)eventTypeStr))
            {
               *ix = len;
               *eventType = table[*ix].eventType;
               *linePP = lineP;
               return (TRUE); 
            }
    }

    /* Unknown event specified */
    return (FALSE);

} /* END OF FUNCTION ParseEventType */


/*************************************<->*************************************
 *
 *  ParseImmed (linePP, closure, detail)
 *
 *
 *  Description:
 *  -----------
 *  Button event detail procedure.
 *
 *
 *  Inputs:
 *  ------
 *  linePP =  not used
 *  closure = table entry
 *
 * 
 *  Outputs:
 *  -------
 *  detail = pointer to closure
 *
 *  Return = TRUE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseImmed (unsigned char **linePP, unsigned int closure,
			   unsigned int  *detail)
{
    *detail = closure;
    return (TRUE);

} /* END OF FUNCTION ParseImmed */


/*************************************<->*************************************
 *
 *  ParseKeySym (linePP, closure, detail)
 *
 *
 *  Description:
 *  -----------
 *  Key event detail procedure.  Parses a KeySym string.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = pointer to current line buffer pointer
 *
 *  closure = not used.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = pointer to revised line buffer pointer
 *  detail = pointer to parsed KeySym
 *
 *  Return = (Boolean) true iff valid KeySym string
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static Boolean ParseKeySym (unsigned char **linePP, unsigned int closure,
			    unsigned int *detail)
{
    unsigned char *lineP = *linePP;
    unsigned char *startP;
    char           keySymName[MAX_KEYSYM_STRLEN+1];
    int            len;
#ifndef NO_MULTIBYTE
    int            chlen;
#endif

    ScanWhitespace (&lineP);
    startP = lineP;

#ifndef NO_MULTIBYTE
    while (*lineP &&
	   ((chlen = mblen ((char *)lineP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) ||
	   (!isspace (*lineP) && *lineP != ',' && *lineP != ':')))
    {
	/* Skip next character */
        lineP += chlen;
    }
#else
    while (*lineP && !isspace (*lineP) && *lineP != ',' && *lineP != ':' )
    {
	/* Skip next character */
        lineP++;
    }
#endif

    len = min (lineP - startP, MAX_KEYSYM_STRLEN);
    (void) strncpy (keySymName, (char *)startP, len);
    keySymName[len] = '\0';

#ifndef NO_MULTIBYTE
    if ((*detail = XStringToKeysym(keySymName)) == NoSymbol &&
	 (mblen (keySymName, MB_CUR_MAX) == 1))
#else
    if ((*detail = XStringToKeysym(keySymName)) == NoSymbol)
#endif
    {
        if (!isdigit (keySymName[0]) ||
            ((*detail = StrToNum ((unsigned char *)&keySymName[0])) == -1))
        {
            *detail = NoSymbol;
            return (FALSE);
        }
    }
    *linePP = lineP;
    return (TRUE);

} /* END OF FUNCTION ParseKeySym */



/*************************************<->*************************************
 *
 *  StrToNum(str)
 *
 *
 *  Description:
 *  -----------
 *  Converts a string to an unsigned hexadecimal, decimal, or octal integer.
 *
 *
 *  Inputs:
 *  ------
 *  str = character string
 *
 * 
 *  Outputs:
 *  -------
 *  Return = unsigned integer 
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static unsigned int StrToNum(unsigned char *str)
{
    unsigned char c;
    unsigned int  val = 0;

    if (*str == '0')
    {
	str++;
	if (*str == 'x' || *str == 'X')
	{
	    return (StrToHex(++str));
	}
	return (StrToOct(str));
    }

    while ((c = *str) != '\0')
    {
	if ('0' <= c && c <= '9')
	{
	    val = val*10+c-'0';
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToNum */



/*************************************<->*************************************
 *

 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static unsigned int StrToHex(unsigned char *str)
{
    unsigned char c;
    unsigned int  val = 0;

    while ((c = *str) != '\0')
    {
	if ('0' <= c && c <= '9')
	{
	    val = val*16+c-'0';
	}
	else if ('a' <= c && c <= 'f')
	{
	    val = val*16+c-'a'+10;
	}
	else if ('A' <= c && c <= 'F')
	{
	    val = val*16+c-'A'+10;
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToHex */



/*************************************<->*************************************
 *

 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static unsigned int StrToOct(unsigned char *str)
{
    unsigned char c;
    unsigned int  val = 0;

    while ((c = *str) != '\0')
    {
	if ('0' <= c && c <= '7')
	{
	    val = val*8+c-'0';
	}
	else
	{
	    return (-1);
	}
	str++;
    }

    return (val);

} /* END OF FUNCTION StrToOct */



/*************************************<->*************************************
 *
 *  ScanAlphanumeric (linePP)
 *
 *
 *  Description:
 *  -----------
 *  Scan string until a non-alphanumeric character is encountered.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = nonNULL pointer to current line buffer pointer
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = nonNULL pointer to revised line buffer pointer
 *
 *
 *  Comments:
 *  --------
 *  Assumes linePP is nonNULL
 * 
 *************************************<->***********************************/

void ScanAlphanumeric (unsigned char **linePP)
{
#ifndef NO_MULTIBYTE
    int            chlen;

    while (*linePP &&
	   ((chlen = mblen ((char *) *linePP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) || isalnum (**linePP)))
    {
        (*linePP) += chlen;
    }
#else
    while (*linePP && isalnum (**linePP))
    {
        (*linePP)++;
    }
#endif

} /* END OF FUNCTION ScanAlphanumeric */


#ifndef WSM

/*************************************<->*************************************
 *
 *  ScanWhitespace(linePP)
 *
 *
 *  Description:
 *  -----------
 *  Scan the string, skipping over all white space characters.
 *
 *
 *  Inputs:
 *  ------
 *  linePP = nonNULL pointer to current line buffer pointer
 *
 * 
 *  Outputs:
 *  -------
 *  linePP = nonNULL pointer to revised line buffer pointer
 *
 *
 *  Comments:
 *  --------
 *  Assumes linePP is nonNULL
 * 
 *************************************<->***********************************/

void ScanWhitespace(unsigned char  **linePP)
{
#ifndef NO_MULTIBYTE
    while (*linePP && (mblen ((char *)*linePP, MB_CUR_MAX) == 1) && isspace (**linePP))
#else
    while (*linePP && isspace (**linePP))
#endif
    {
        (*linePP)++;
    }

} /* END OF FUNCTION ScanWhitespace */
#endif /* not WSM */

#ifndef WSM

/*************************************<->*************************************
 *
 *  ToLower (string)
 *
 *
 *  Description:
 *  -----------
 *  Lower all characters in a string.
 *
 *
 *  Inputs:
 *  ------
 *  string = NULL-terminated character string or NULL
 *
 * 
 *  Outputs:
 *  -------
 *  string = NULL-terminated lower case character string or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void ToLower (unsigned char  *string)
{
    unsigned char *pch = string;
#ifndef NO_MULTIBYTE
    int            chlen;

    while (*pch && ((chlen = mblen ((char *)pch, MB_CUR_MAX)) > 0))
    {
        if ((chlen == 1) && (isupper (*pch)))
	{
	    *pch = tolower(*pch);
	}
	pch += chlen;
    }
#else
    while (*pch != '\0')
    {
        if (isupper (*pch))
	{
	    *pch = tolower(*pch);
	}
	pch++;
    }
#endif

} /* END OF FUNCTION ToLower */
#endif  /* WSM */


/*************************************<->*************************************
 *
 *  PWarning (message)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a resource description parse message to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  message = pointer to a message string
 *  cfileP  = (global) file pointer to fopened configuration file or NULL
 *  linec   = (global) line counter
 * 
 *************************************<->***********************************/

void
PWarning (char *message)
{

#ifdef WSM
    char pch[MAXWMPATH+1];
    String sMessage;
    char *pchFile;

    sMessage = XtNewString ((String) message);
    if (cfileP != NULL)
    {
	if (pConfigStackTop->fileName)
	{
	    pchFile = pConfigStackTop->fileName;
	}
	else 
	{
	    pchFile = wmGD.configFile;
	}

        sprintf (pch, pWarningStringFile,
                     GETMESSAGE(20,1,"Workspace Manager"), 
		     sMessage, linec, pchFile);
    }
    else
    {
        sprintf (pch, pWarningStringLine,
                     GETMESSAGE(20,1,"Workspace Manager"), 
		     sMessage, linec);
    }
    _DtSimpleError (wmGD.mwmName, DtIgnore, NULL, pch, NULL);
    XtFree (sMessage);
#else /* WSM */
    if (cfileP != NULL)
    {
        fprintf (stderr, ((char *)GETMESSAGE(60, 33, "%s: %s on line %d of configuration file %s\n")),
		 wmGD.mwmName, message, linec,
		 wmGD.configFile ? wmGD.configFile : cfileName);
    }
    else
    {
        fprintf (stderr, ((char *)GETMESSAGE(60, 34, "%s: %s on line %d of specification string\n")),
                     wmGD.mwmName, message, linec);
    }
    fflush (stderr);
#endif /* WSM */


} /* END OF FUNCTION PWarning */

#ifdef WSM
/*
 * Key substitution table entry
 */

typedef struct _keySubs
{
    unsigned char *	pchFrom;
    int   		lenFrom;
    unsigned char *	pchTo;
} KeySub;


/*************************************<->*************************************
 *
 *  InitKeySubs (ppKeySubs, pNumKeySubs)
 *
 *
 *  Description:
 *  -----------
 *  Initialize key label substitutions used in acclerator text
 *
 *
 *  Inputs:
 *  ------
 *  ppKeySubs	= ptr to key substitution table ptr
 *  pNumKeySubs	= ptr to number of key substitutions
 *
 * 
 *  Outputs:
 *  -------
 *  *ppKeySubs		= ptr to key substitution table 
 *  *pNumKeySubs	= number of substitutions found
 *
 *  Comments:
 *  --------
 *  *ppKeySubs is allocated with XtMalloc in a complicated way.
 *  If this ever needs to be freed, a function to free it needs to
 *  be written.
 * 
 *************************************<->***********************************/

static void InitKeySubs (
    KeySub	**ppKeySubs,
    int		*pNumKeySubs)
{
    int numKS;
    KeySub *pKS;
    KeySub *pKSret;
    unsigned char *pch0;
    unsigned char *pch1;
    int len;
#ifndef NO_MULTIBYTE
    int		chlen;
#endif 

    pch0 = (unsigned char *)GETMESSAGE(60, 40, "");

    if ((pch0 == NULL) || (*pch0 == '\0'))
    {
	*ppKeySubs = NULL;
	*pNumKeySubs = 0;
	return;
    }

    pKSret = NULL;
    numKS = 0;

    while (*pch0 != '\0')
    {
	ScanWhitespace (&pch0);
	if (*pch0 == '\0') break;

	/* 
	 * allocate space for next key sub 
	 */
	if (pKSret == NULL)
	{
	    pKSret = (KeySub *) XtMalloc (1*sizeof(KeySub));
	}
	else
	{
	    pKSret = (KeySub *) XtRealloc ((char *)pKSret, 
					   (numKS+1)*sizeof(KeySub));
	}
	pKS = &pKSret[numKS];

	/* get "from" string */
	pch1 = pch0;
#ifndef NO_MULTIBYTE
        while (*pch1 && ((chlen = mblen ((char *)pch1, MB_CUR_MAX)) > 0))
	{
	    if ((chlen == 1) && (*pch1 == ' '))
	    {
		break;
	    }
	    pch1 += chlen;
	}
#else /* NO_MULTIBYTE */
	while (*pch1 && (*pch1 != ' ')) pch1++;
#endif /* NO_MULTIBYTE */
	pKS->lenFrom = pch1 - pch0;
	if (pKS->lenFrom < 1) 
	{
	    /*
	     * no "from" string
	     */
	    break;
	}
	pKS->pchFrom = (unsigned char *) XtMalloc (1+pKS->lenFrom);
	memcpy (pKS->pchFrom, pch0, pKS->lenFrom);
	pKS->pchFrom[pKS->lenFrom] = '\0';

	/* get "to" string */
	ScanWhitespace (&pch1);
	pch0 = pch1;

#ifndef NO_MULTIBYTE
        while (*pch1 && ((chlen = mblen ((char *)pch1, MB_CUR_MAX)) > 0))
	{
	    if ((chlen == 1) && (*pch1 == ' '))
	    {
		break;
	    }
	    pch1 += chlen;
	}
#else /* NO_MULTIBYTE */
	while (*pch1 && (*pch1 != ' ')) pch1++;
#endif /* NO_MULTIBYTE */

	len = pch1 - pch0;
	if (len < 1)
	{
	    /*
	     * Invalid format, "from" string with no "to" string.
	     */
	    break;
	}
	pKS->pchTo = (unsigned char *) XtMalloc (1+len);
	memcpy (pKS->pchTo, pch0, len);
	pKS->pchTo[len] = '\0';

	/* advance cursor */
	pch0 = pch1;

	/* got one, bump the counter */
	numKS++; 
    }

    *ppKeySubs = pKSret;
    *pNumKeySubs = numKS;
}

#endif /* WSM */

/*************************************<->*************************************
 *
 *  ProcessAccelText (startP, endP, destP)
 *
 *
 *  Description:
 *  -----------
 *  Process accelerator text and copy into string.
 *
 *
 *  Inputs:
 *  ------
 *  startP = pointer to start of valid accelerator specification
 *  endP =   pointer past end of accelerator specification
 *  destP =  pointer to destination buffer
 *
 * 
 *  Outputs:
 *  -------
 *  Destination buffer has processed accelerator text.
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void ProcessAccelText (unsigned char *startP, unsigned char *endP,
			      unsigned char *destP)
{
#ifndef NO_MULTIBYTE
    int   chlen;
#endif
#ifdef WSM
    static Boolean	bAccelInit = False;
    static KeySub	*pKeySub;
    static int		numKeySubs;
    unsigned char *	pchFirst;
    unsigned char *	pchSub;
    int			lenSub;
    int			i;

    if (!bAccelInit)
    {
	InitKeySubs (&pKeySub, &numKeySubs);
	bAccelInit = True;
    }
#endif /* WSM */

    /*
     * Copy modifiers
     */

    ScanWhitespace (&startP);

    while (*startP != '<')
    {
        if (*startP == '~') 
	{
            *destP++ = *startP++;
        }
#ifdef WSM
	pchFirst = startP;
#endif /* WSM */

#ifndef NO_MULTIBYTE
        while (*startP &&
	       (((chlen = mblen ((char *)startP, MB_CUR_MAX)) > 1)
		|| isalnum (*startP)))
        {
	    while (chlen--)
	    {
#ifdef WSM
	        startP++;

#else /* WSM */
	        *destP++ = *startP++;
#endif /* WSM */
	    }
	}
#else
        while (isalnum (*startP))
        {
#ifdef WSM
	        startP++;
#else /* WSM */
	    *destP++ = *startP++;
#endif /* WSM */
	}
#endif
#ifdef WSM
	/* find substitution */
	pchSub = NULL;
	lenSub = 0;

	for (i=0; i<numKeySubs; i++)
	{
	    if ((pKeySub[i].lenFrom == startP-pchFirst) &&
		(!strncasecmp ((char *)pKeySub[i].pchFrom, (char *)pchFirst, 
				pKeySub[i].lenFrom)))
	    {
		pchSub = pKeySub[i].pchTo;
		lenSub = strlen((char *)pchSub);
		break;
	    }

	}

        if ((pchSub != NULL) && (lenSub > 0))
	{
	    memcpy (destP, pchSub, lenSub);
	    destP += lenSub;
	}
	else
	{
	    memcpy (destP, pchFirst, startP-pchFirst);
	    destP += startP-pchFirst;
	}
#endif /* WSM */
	*destP++ = '+';

        ScanWhitespace (&startP);
    }

    /*
     * Skip the key event type.
     */
    startP++;  /* skip '<' */
    while (*startP != '>')
    {
#ifndef NO_MULTIBYTE
        startP += mblen ((char *)startP, MB_CUR_MAX);
#else
        startP++;
#endif
    }
    startP++;  /* skip '>' */

    /*
     * Copy the KeySym string.
     */

    ScanWhitespace (&startP);
    while (startP != endP)
    {
        *destP++ = *startP++;
    }
    *destP = '\0';

} /* END OF FUNCTION ProcessAccelText */



/*************************************<->*************************************
 *
 *  ProcessCommandLine (argc, argv)
 *
 *
 *  Description:
 *  -----------
 *  This function looks for and processes mwm options in the command line
 *
 *  Inputs:
 *  ------
 *  argc =   argument count.
 *  argv =   argument vector.
 *
 * 
 *  Outputs:
 *  -------
 *  Changes global data to based on command line options recognized
 *
 *
 *************************************<->***********************************/
#define SCREENS_OPT		"-screens"
#define MULTI_SCREEN_OPT	"-multiscreen"

void ProcessCommandLine (int argc,  char *argv[])
{
    unsigned char *string;
    int argnum;
    unsigned char *lineP;

    for (argnum = 1; argnum < argc; argnum++)
    {
        lineP = (unsigned char *) argv[argnum];
        if ((string = GetString (&lineP)) == NULL) 
        /* empty or comment line */
	{
            continue;
        }
	if (!strcmp((char *)string, MULTI_SCREEN_OPT))
	{
	    wmGD.multiScreen = True;
	    wmGD.numScreens = ScreenCount (DISPLAY);
        }
	else if (!strcmp((char *)string, SCREENS_OPT))
	{
	    argnum++;		/* skip to next arg */
            ParseScreensArgument (argc, argv, &argnum, lineP);
        }
    }

} /* END OF FUNCTION ProcessCommandLine */


/*************************************<->*************************************
 *
 *  ParseScreensArgument (argc, argv, pArgnum, lineP)
 *
 *
 *  Description:
 *  -----------
 *  This function processes the ``-screens'' command line argument
 *
 *  Inputs:
 *  ------
 *  argc =   argument count.
 *  argv =   argument vector.
 *  pArgnum = pointer to argument number where processing left off
 *  lineP  = pointer into argv[*pArgnum] where processing left off
 *
 * 
 *  Outputs:
 *  -------
 *  Changes global data to based on command line options recognized
 *      + wmGD.screenNames
 *      + wmGD.numScreens
 *  Assumes default screenNames are already in place
 *
 *************************************<->***********************************/

static void ParseScreensArgument (int argc, char *argv[], int *pArgnum,
				  unsigned char *lineP)
{
    unsigned char *string;
    int sNum = 0;
    int lastLen;
    int nameCount = 0;

    for (; (*pArgnum < argc) && (sNum < ScreenCount(DISPLAY)); 
					    (*pArgnum)++, sNum++)
    {
	lineP = (unsigned char *)argv[*pArgnum];
	if (*argv[*pArgnum] == '"')
	{
	    /*
	     * if Quote, use GetString to strip it
	     */
	    if ((string = GetString (&lineP)) == NULL) 
		/* empty or comment line */
	    {
		continue;
	    }
	}
	else 
	{
	    string = (unsigned char *)argv[*pArgnum];
	    if (*string == '-')
	    {
		/* another option, end of screens names */
		break;
	    }
	}
	
	if (!(wmGD.screenNames[sNum] = (unsigned char *) 
	      XtRealloc ((char*)wmGD.screenNames[sNum], 
		         1 + strlen((char *)string))))
	{
	    Warning (((char *)GETMESSAGE(60, 31, "Insufficient memory for screen names")));
	    ExitWM(WM_ERROR_EXIT_VALUE);
	}
	else 
	{
	    strcpy((char *)wmGD.screenNames[sNum], (char *)string);
	    nameCount++;
	}
    }

    (*pArgnum)--;

    /*
     * remaining screens (if any) get first name specified 
     */
    if (nameCount > 0)
    {
	lastLen = 1 + strlen((char *)wmGD.screenNames[0]);
	for (; sNum < ScreenCount(DISPLAY); sNum++)
	{
	    if (!(wmGD.screenNames[sNum] = (unsigned char *) 
		XtRealloc ((char*)wmGD.screenNames[sNum], lastLen)))
	    {
		Warning (((char *)GETMESSAGE(60, 32, "Insufficient memory for screen names")));
		ExitWM(WM_ERROR_EXIT_VALUE);
	    }
	    else 
	    {
		strcpy((char *)wmGD.screenNames[sNum], 
		       (char *)wmGD.screenNames[0]);
	    }
	}
    }

} /* END OF FUNCTION ParseScreensArgument */


/*************************************<->*************************************
 *
 *  ProcessMotifBindings ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used retrieve the motif input bindings
 *  and put them into a property on the root window.
 *
 *
 *************************************<->***********************************/
void ProcessMotifBindings (void)
{
    char           fileName[MAXWMPATH+1];
    char	  *bindings = NULL;
#ifndef MOTIF_ONE_DOT_ONE
    char	  *homeDir = XmeGetHomeDirName();
#else
    FILE          *fileP;
#endif

    /*
     *  Look in the user's home directory for .motifbind
     */

#ifdef MOTIF_ONE_DOT_ONE
    GetHomeDirName(fileName);
#else
    strcpy (fileName, homeDir);
#endif
    strncat(fileName, "/", MAXWMPATH-strlen(fileName));
    strncat(fileName, MOTIF_BINDINGS_FILE, MAXWMPATH-strlen(fileName));

#ifdef MOTIF_ONE_DOT_ONE
    if ((fileP = fopen (fileName, "r")) != NULL)
    {
        unsigned char   buffer[MBBSIZ];
        int             count;
        Boolean         first = True;
        int             mode = PropModeReplace;
        Window          propWindow;

        /*
         * Get the atom for the property.
         */
        wmGD.xa_MOTIF_BINDINGS =
                XInternAtom (DISPLAY, _XA_MOTIF_BINDINGS, False);

        /*
         * The property goes on the root window of screen zero
         */
        propWindow = RootWindow(DISPLAY, 0);

        /*
         * Copy file contents to property on root window of screen 0.
         */
        while ( (count=fread((char *) &buffer[0], 1, MBBSIZ, fileP)) > 0)
        {
            XChangeProperty (DISPLAY, propWindow, wmGD.xa_MOTIF_BINDINGS,
                                XA_STRING, 8, mode,
                                &buffer[0], count);

            if (first)
            {
                first = False;
                mode = PropModeAppend;
            }
        }
    }

#else
    XDeleteProperty (DISPLAY, RootWindow (DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_BINDINGS", False));
    XDeleteProperty (DISPLAY, RootWindow (DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_DEFAULT_BINDINGS", False));

    if (_XmVirtKeysLoadFileBindings (fileName, &bindings) == True) {
	XChangeProperty (DISPLAY, RootWindow(DISPLAY, 0),
		XInternAtom (DISPLAY, "_MOTIF_BINDINGS", False),
		XA_STRING, 8, PropModeReplace,
		(unsigned char *)bindings, strlen(bindings));
    }
    else {
	_XmVirtKeysLoadFallbackBindings (DISPLAY, &bindings);
    }
    XtFree (bindings);
#endif
} /* END OF FUNCTION ProcessMotifBindings */

#ifdef PANELIST

/*************************************<->*************************************
 *
 *  void
 *  ParseWmFunctionArg (linePP, ix, wmFunc, ppArg, sClientName)
 *
 *
 *  Description:
 *  -----------
 *  Parse the function arguments for a window manager function.
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to line buffer pointer (contains string arg).
 *  ix = window manager function index (returned by ParseWmFunction)
 *  pWmFunction = pointer to window manager function destination.
 *  ppArg = ptr to argument pointer.
 *  sClientName = string name of client 
 *
 *
 *  Outputs:
 *  -------
 *  *ppArg = arg to pass to window manager function when invoking.
 *  Return = true on succes, false on some kind of parse error
 *
 *
 *  Comments:
 *  --------
 *  functionTable (window manager function table) is indexed with ix
 *  to get parsing info.
 *
 *  This function may malloc memory for the returned arg.
 *
 *  The sClientName is needed for starting some hpterm-based push_recall
 *  clients. It needs to be passed into the action so the hpterm gets
 *  named appropriately.
 * 
 *************************************<->***********************************/

Boolean
ParseWmFunctionArg (
		unsigned char **linePP,
		int ix, 
		WmFunction wmFunc, 
		void **ppArg,
		String sClientName,
		String sTitle)
{
    unsigned char *lineP = *linePP;
    Boolean bValidArg = True;
    unsigned char *str = NULL;

    /*
     * If this is (possibly) a string argument, put it 
     * in quotes so that it will be parsed properly.
     */
    if ((functionTable[ix].parseProc == ParseWmFuncStrArg) ||
        (functionTable[ix].parseProc == ParseWmFuncMaybeStrArg))
    {
	if (lineP && *lineP != '"')
	{
	    /*
	     * not in quotes, repackage it, escaping the appropriate
	     * characters.
	     */
	    str = _DtWmParseMakeQuotedString (lineP);
	    if (str)
	    {
		lineP = str;
	    }
	}
    }

    /* 
     * Apply the function argument parser.
     */
    if ((functionTable[ix].wmFunction != wmFunc) ||
	!(*(functionTable [ix].parseProc)) (&lineP, wmFunc, ppArg))
    {
	bValidArg = False;
    }

    /*
     * Add the exec parms if this is an f.action
     */
    if ((wmFunc == F_Action) && ppArg && *ppArg)
    {
	WmActionArg *pAP = (WmActionArg *) *ppArg;
	int totLen = 0;

	/*
	 * allocate more than enough string space to copy 
	 * strings and intervening spaces.
	 */
	if (sClientName && *sClientName)
	{
	    /* length of: "name=" + sClientName + NULL */
	    totLen += 5 + strlen(sClientName) + 1;
	}
	if (sTitle && *sTitle)
	{
	    /* length of: "," + "title=" + sTitle + NULL */
	    totLen += 1 + 6 + strlen(sTitle) + 1;
	}

	if (totLen > 0)
	{
	    pAP->szExecParms = (String) XtMalloc (totLen);
	    /* start with empty string */
	    pAP->szExecParms[0] = '\0'; 

	    if (sClientName && *sClientName)
	    {
		strcat (pAP->szExecParms, "name=");
		strcat (pAP->szExecParms, sClientName);
	    }
	    if (sTitle && *sTitle)
	    {
		if (pAP->szExecParms[0] != '\0')
		{
		    strcat (pAP->szExecParms, ",");
		}
		strcat (pAP->szExecParms, "title=");
		strcat (pAP->szExecParms, sTitle);
	    }
	}
    }

    if (str)
    {
	XtFree ((char *) str);
    }

    return (bValidArg);

} /* END OF FUNCTION ParseWmFunctionArg */


/*************************************<->*************************************
 *
 *  SystemCmd (pchCmd)
 *
 *
 *  Description:
 *  -----------
 *  This function fiddles with our signal handling and calls the
 *  system() function to invoke a unix command.
 *
 *
 *  Inputs:
 *  ------
 *  pchCmd = string with the command we want to exec.
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  The system() command is touchy about the SIGCLD behavior. Restore
 *  the default SIGCLD handler during the time we run system().
 * 
 *************************************<->***********************************/

void
SystemCmd (char *pchCmd)
{
    struct sigaction sa;
    struct sigaction osa;

    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;

    (void) sigaction (SIGCLD, &sa, &osa);

    system (pchCmd);

    (void) sigaction (SIGCLD, &osa, (struct sigaction *) 0);
}



/*************************************<->*************************************
 *
 *  DeleteTempConfigFileIfAny ()
 *
 *
 *  Description:
 *  -----------
 *  This function deletes the temporary config file used to process 
 *  old dtwmrc syntax.
 *
 *
 *  Inputs:
 *  ------
 *
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void
DeleteTempConfigFileIfAny (void)
{
    char pchCmd[MAXWMPATH+1];

    if (pConfigStackTop->tempName)
    {
	strcpy (pchCmd, "/bin/rm ");
	strcat (pchCmd, pConfigStackTop->tempName);
	SystemCmd (pchCmd);
	XtFree ((char *) pConfigStackTop->tempName);
	pConfigStackTop->tempName = NULL;
    }
    if (pConfigStackTop->cppName)
    {
	strcpy (pchCmd, "/bin/rm ");
	strcat (pchCmd, pConfigStackTop->cppName);
	SystemCmd (pchCmd);
	XtFree ((char *) pConfigStackTop->cppName);
	pConfigStackTop->cppName = NULL;
    }
}


/*************************************<->*************************************
 *
 *  ParseIncludeSet (pSD, lineP)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  cfileP = (global) file pointer to fopened configuration file or NULL
 *  lineP = pointer to line buffer
 *  line   = (global) line buffer
 *  linec  = (global) line count
 *  parseP = (global) parse string pointer if cfileP == NULL
 *  pSD->rootWindow = default root window of display
 * 
 *  Outputs:
 *  -------
 *  linec  = (global) line count incremented
 *  parseP = (global) parse string pointer if cfileP == NULL
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void ParseIncludeSet (WmScreenData *pSD, unsigned char *lineP) 
{
    unsigned char     *string;
    unsigned char     *pchName;

    /*
     * Require leading '{' on the next line.
     */

    while ((GetNextLine () != NULL))  /* not EOF nor read error */
    {
        lineP = line;
	ScanWhitespace(&lineP);

	if ((lineP == NULL) || (*line == '!') || (*lineP == '\0') || (*lineP == '#'))
	/* ignore empty or comment line */
        {
            continue;
        }

        if (*lineP == '{')
	/* found '{' */
        {
            break;
        }

	/* not a '{' */
	PWarning (((char *)GETMESSAGE(60, 37, "Expected '{'")));
        return;
    }

    /*
     * Found leading "{" or EOF.
     * Parse include files until "}" or EOF found.
     */
    while ((GetNextLine () != NULL))
    {
	lineP = line;
	if ((*line == '!') || (string = GetString (&lineP)) == NULL)
	/* ignore empty or comment lines */
        {
            continue;
        }
        if (*string == '}')  /* finished with set. */
        {
	    break;
        }
	pchName = _DtWmParseFilenameExpand (string);
	if (pchName && ConfigStackPush (pchName))
	{
	    ProcessWmFile (pSD, True /* nested */);
	    ConfigStackPop ();
	    XtFree ((char *) pchName);
	}

    }

}



/*************************************<->*************************************
 *
 *  ConfigStackInit (pchFileName)
 *
 *
 *  Description:
 *  -----------
 *  Initializes the config file processing stack
 *
 *  Inputs:
 *  ------
 *  pchFileName = name of new file to start parsing
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void ConfigStackInit (char *pchFileName)
{

    pConfigStack  = XtNew (ConfigFileStackEntry);

    if (pConfigStack)
    {
	pConfigStackTop = pConfigStack;
	pConfigStackTop->fileName = XtNewString (pchFileName);
	pConfigStackTop->tempName = NULL;
	pConfigStackTop->cppName = NULL;
	pConfigStackTop->offset = 0;
	pConfigStackTop->pWmPB = wmGD.pWmPB;
	pConfigStackTop->wmgdConfigFile = wmGD.configFile;
	pConfigStackTop->pIncluder = NULL;

    }
    else
    {
	sprintf ((char *)wmGD.tmpBuffer,
	    (char *)GETMESSAGE(60,36,"Insufficient memory to process included file: %s"), 
	    pchFileName);
	Warning ((char *)wmGD.tmpBuffer);
    }
}


/*************************************<->*************************************
 *
 *  ConfigStackPush (pchFileName)
 *
 *
 *  Description:
 *  -----------
 *  Open an included config file
 *
 *  Inputs:
 *  ------
 *  pchFileName = name of new file to start parsing
 *  wmGD.pWmPB = global parse buffer (pickle this)
 * 
 *  Outputs:
 *  -------
 *  wmGD.pWmPB = global parse buffer (new one for new file)
 *  return = FILE * to open file or NULL
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static FILE *
ConfigStackPush (unsigned char *pchFileName)
{
    ConfigFileStackEntry *pEntry;
    FILE *pNewFile = NULL;

    pEntry = XtNew (ConfigFileStackEntry);
    if (pEntry)
    {
	/* save off state of current config file */
	pConfigStackTop->offset = ftell (cfileP);
	pConfigStackTop->pWmPB = wmGD.pWmPB;
	fclose (cfileP);

	/* set up state of new config file */
	pEntry->fileName = XtNewString ((char *)pchFileName);
	pEntry->tempName = NULL;
	pEntry->cppName = NULL;
	pEntry->wmgdConfigFile = (String) pEntry->fileName;

	/* set globals for new config file */
	wmGD.pWmPB = _DtWmParseNewBuf ();
	wmGD.configFile = pEntry->wmgdConfigFile;

	/* put new entry onto stack */
	pEntry->pIncluder = pConfigStackTop;
	pConfigStackTop = pEntry;

	/* open the file */
	pNewFile = cfileP = FopenConfigFile();

	if (!pNewFile)
	{
	    /* file open failed! back out */
	    ConfigStackPop ();
	}
    }
    else
    {
	sprintf ((char *)wmGD.tmpBuffer,
	    (char *)GETMESSAGE(60,36,"Insufficient memory to process included file: %s"), 
	    pchFileName);
	Warning ((char *)wmGD.tmpBuffer);
    }

    return (pNewFile);
}



/*************************************<->*************************************
 *
 *  ConfigStackPop ()
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pchFileName = name of new file to start parsing
 *  wmGD.pWmPB = global parse buffer (pickle this)
 * 
 *  Outputs:
 *  -------
 *  wmGD.pWmPB = global parse buffer (new one for new file)
 *
 *
 *  Comments:
 *  --------
 *  assumes cfileP is closed already
 * 
 *************************************<->***********************************/

static void ConfigStackPop (void)
{
    Boolean error = False;
    ConfigFileStackEntry *pPrev;
    char pchCmd[MAXWMPATH+1];

    if (pConfigStackTop != pConfigStack)
    {
	pPrev = pConfigStackTop->pIncluder;

	_DtWmParseDestroyBuf (wmGD.pWmPB);
	if (pConfigStackTop->tempName)
	{
	    XtFree (pConfigStackTop->tempName);
	}
	if (pConfigStackTop->cppName)
	{
	    strcpy (pchCmd, "/bin/rm "); 
	    strcat (pchCmd, pConfigStackTop->cppName); 
	    SystemCmd (pchCmd);
	    XtFree ((char *) pConfigStackTop->cppName);
	    pConfigStackTop->cppName = NULL;
	}
	if (pConfigStackTop->fileName)
	{
	    XtFree (pConfigStackTop->fileName);
	}

	wmGD.pWmPB = pPrev->pWmPB;
	wmGD.configFile = pPrev->wmgdConfigFile;
	if (pPrev->tempName)
	{
	    cfileP = fopen (pPrev->tempName, "r");
	}
	else if (pPrev->cppName)
	{
	    cfileP = fopen (pPrev->cppName, "r");
	}
	else
	{
	    cfileP = fopen (pPrev->fileName, "r");
	}
	if (cfileP) 
	{
	    fseek (cfileP, pPrev->offset, 0);
	}
	else
	{
	    char msg[MAXWMPATH+1];

	    sprintf(msg, ((char *)GETMESSAGE(60, 39, 
			    "Could not reopen configuration file %s")),
		pPrev->fileName);
	    Warning (msg);
	}

	XtFree ((char *)pConfigStackTop);
	pConfigStackTop = pPrev;
    }
}


/*************************************<->*************************************
 *
 *  ParseWmFuncActionArg (linePP, wmFunction, pArgs)
 *
 *
 *  Description:
 *  -----------
 *  Parses a window manager f.action argument
 *
 *
 *  Inputs:
 *  ------
 *  linePP   = pointer to current line buffer pointer.
 *  wmFunction = function for which the argument string is intended.
 *  pArgs = pointer to argument destination.
 *
 * 
 *  Outputs:
 *  -------
 *  linePP   = pointer to revised line buffer pointer.
 *  pArgs    = pointer to parsed argument.
 *  Return   = FALSE iff insufficient memory
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean ParseWmFuncActionArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs)
{
#define WM_ACTION_ARG_INCREMENT 5
#define WM_ACTION_ARG_PAD	256
    unsigned char *string;
    char *pch;
    WmActionArg *pAP;
    int iArgSz;

    pAP = XtNew (WmActionArg);
    if (pAP && (string = GetString (linePP)) != NULL)
    {
	/* Got action name */
	pAP->actionName = XtNewString ((char *) string);

	/* Get action arguments, if any */
	if (pAP->aap = (DtActionArg *) 
		XtMalloc (WM_ACTION_ARG_INCREMENT * sizeof (DtActionArg)))
	{
	    iArgSz = WM_ACTION_ARG_INCREMENT;
	    pAP->numArgs = 0;

	    while ((string = GetString (linePP)) != NULL)
	    {
	       if (pAP->aap[pAP->numArgs].u.file.name = (char *)
		       XtMalloc(1 + strlen((char *)string)))
	       {
		   pAP->aap[pAP->numArgs].argClass = DtACTION_FILE;

		   /* format the action argument */
		   pch = pAP->aap[pAP->numArgs].u.file.name;

		   /* 
		    * Expand environment variable
		    */
		   if (string[0] == '$')
		   {
		       string = (unsigned char *) getenv ((char *)&string[1]);
		       if (!string)
		       {
			   break;
		       }
		       else
		       {
			   /*
			    * Make sure there's room for the new
			    * string.
			    */
			   pch = (char *) 
			     XtRealloc (pch, (1+strlen((char *)string)));
			   pAP->aap[pAP->numArgs].u.file.name = pch;
		       }
		   }

		   /* !!! No host name processing is done!!! */

		   strcpy (pch, (char *)string);

		   pAP->numArgs++;
		   if (pAP->numArgs == iArgSz)
		   {
		       /* need to increase our array space */
		       iArgSz += WM_ACTION_ARG_INCREMENT;
		       pAP->aap = (DtActionArg *) 
			   XtRealloc((char *)pAP->aap,
			             (iArgSz * sizeof (DtActionArg)));
		       if (!pAP->aap)
		       {
			   break; /* out of memory */
		       }
		   }
	       }
	       else
	       {
		   break; /* out of memory */
	       }
	    }

	}
	pAP->szExecParms = NULL;
        *pArgs = (String) pAP;
    }
    else
    /* NULL string argument */
    {
        *pArgs = NULL;
    }

    return (TRUE);

} /* END OF FUNCTION ParseWmFuncActionArg */


#endif /* PANELIST */
#ifdef WSM

/*************************************<->*************************************
 *
 *  PreprocessConfigFile (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function runs the configuration file through the C
 *  preprocessor
 *
 *
 *  Inputs:
 *  ------
 *  pSD = ptr to screen data
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void
PreprocessConfigFile (void)
{
#define CPP_NAME_SIZE	((L_tmpnam)+1)
    char pchCmd[MAXWMPATH+1];

    if (wmGD.cppCommand && *wmGD.cppCommand)
    {
	/*
	 * Generate a temp file name.
	 */
	pConfigStackTop->cppName = XtMalloc (CPP_NAME_SIZE * sizeof(char));
	if (pConfigStackTop->cppName)
	{
	    (void) tmpnam (pConfigStackTop->cppName);

	    /*
	     * Build up the command line.
	     */
	    strcpy (pchCmd, wmGD.cppCommand);
	    strcat (pchCmd, " ");
	    strcat (pchCmd, pConfigStackTop->fileName);
	    strcat (pchCmd, " ");
	    strcat (pchCmd, pConfigStackTop->cppName);

	    /*
	     * Run the config file through the converter program
	     * and send the output to a temp file. 
	     */
	    SystemCmd (pchCmd);
	}
    }
}


/*************************************<->*************************************
 *
 *  GetNetworkFileName (char *pchFile)
 *
 *
 *  Description:
 *  -----------
 *  This function returns a local representation for a network
 *  file. 
 *
 *
 *  Inputs:
 *  ------
 *  pchFile	- pointer to file name of form [<host>:]<path>
 *
 *  Return:
 *  -------
 *  String	- ptr to allocated string of local file name. If input
 *		  is not a network file, the a copy of pchFile is returned.
 *
 *
 *  Comments:
 *  --------
 *  returned file name should be freed with XtFree().
 * 
 *************************************<->***********************************/

static String
GetNetworkFileName (char *pchFile)
{
    char *pch;
    char *host_part; 
    char *file_part; 
    char *netfile;
    char *sName = NULL;
    char *pchName = NULL;
    int count;
    char **pchTok;
    String sReturn = NULL;
    char *homeDir;
    int len;


    pch = strchr (pchFile, ':');

    if (pch)
    {
	/*
	 * Expand special chars and find matching file.
	 */
	pchTok = (char **)shellscan (pchFile, &count, 0);

	if ((count == 1) || (count == 2))
	{
	    /* one match found */
	    host_part = pchTok[0];
	}
	else if (count > 2)
	{
	    /* several matches found, pick one */
	    host_part = pchTok[1];
	}
	else
	{
	    host_part = NULL;
	}

	if (host_part != NULL)
	{
	    pch = strchr (host_part, ':');
	    if (pch)
	    {
		/* 
		 * copy the string so we don't munge data
		 * inside shellscan 
		 */
		host_part = sName = XtNewString ((String) host_part);
		pch = strchr (sName, ':');

		/*
		 * separate the host and file parts of the
		 * file name
		 */
		*pch = '\0';
		file_part = pch+1;
	    }
	    else
	    {
		/*
		 * The colon went away. Hmm...
		 */
		file_part = host_part;
		host_part = NULL;
	    }

	    if (
#ifndef NO_MULTIBYTE
	        (mblen(file_part, MB_CUR_MAX) == 1) && 
		(mblen(file_part+1, MB_CUR_MAX) == 1) &&
#endif
		(*file_part == '~') &&
		(*(file_part+1) == '/'))
	    {
		/*
		 * Replace '~' with $HOME
		 */
		homeDir = XmeGetHomeDirName();
		len = strlen (host_part) + 1 +
		      strlen (homeDir) + strlen (file_part) + 1;
		pch = (char *) XtMalloc (len);
		strcpy (pch, sName);
		host_part = pch;
		pch += strlen (pch) + 1;
		strcpy (pch, homeDir);
		strcat (pch, file_part+1);
		file_part = pch;
		XtFree (sName);
		sName = host_part;
	    }
	}
	else
	{
	    /*
	     * shellscan had a problem with the file name.
	     * just operate on the name as-is.
	     * temporarily replace ':' with a NULL 
	     */
	    host_part = sName = XtNewString ((String) pchFile);
	    pch = strchr (sName, ':');
	    *pch = '\0';
	    host_part = pchFile;
	    file_part = pch+1;
	}

	/* convert to canonical host/file name */
	netfile = (char *) 
		    tt_host_file_netfile (host_part, file_part);
	if (tt_pointer_error (netfile) == TT_OK)
	{
	    /* convert to local file name equivalent */
	    pchName = tt_netfile_file (netfile);

	    if (tt_pointer_error (pchName) == TT_OK)
	    {
		sReturn = XtNewString ((String) pchName);
		tt_free ((char *)pchName);
	    }
	    tt_free (netfile);
	}

	if (sName)
	{
	    XtFree ((char *)sName);
	}
    }

    if (sReturn == NULL)
    {
	if (
#ifndef NO_MULTIBYTE
	    (mblen(pchFile, MB_CUR_MAX) == 1) && 
	    (mblen(pchFile+1, MB_CUR_MAX) == 1) &&
#endif
	    (*pchFile == '~') &&
	    (*(pchFile+1) == '/'))
	{
	    /*
	     * Replace '~' with $HOME
	     */
	    homeDir = XmeGetHomeDirName();
	    len = strlen (homeDir) + strlen (pchFile) + 1;
	    sReturn = (char *) XtMalloc (len);
	    strcpy (sReturn, homeDir);
	    strcat (sReturn, pchFile+1);
	}
	else
	{
	    sReturn = XtNewString ((String) pchFile);
	}
    }

    return (sReturn);
}
/****************************   eof    ***************************/
#endif /* WSM */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  SetGreyedContextAndMgtMask (menuItem, wmFunction)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the greyed context and management mask
 *  for a menu item based on the menu function passed in.
 *
 *  Inputs:
 *  ------
 *  menuItem   = the menu item to be set up 
 *  wmFunction = the menu function to find in the function table
 *		 to determine how to set up the relevant fields
 * 
 *  Outputs:
 *  -------
 *  The menuItem will have its greyed context and management mask fields
 *  set appropriately. If the given function cannot be found, the fields
 *  will be set to the appropriate values as if the function were F_Nop.
 *  Return = True if the function could be found. False otherwise.
 *
 *************************************<->***********************************/

Boolean SetGreyedContextAndMgtMask (MenuItem *menuItem,
 				    WmFunction wmFunction)
{
    int ix;

    for (ix = 0; ix < WMFUNCTIONTABLESIZE - 1; ++ix)
    {
	if (functionTable[ix].wmFunction == wmFunction)
	{
	    /* Success! The function was found. Set up the
	       values and get the heck out of here. */
	    menuItem->greyedContext = functionTable[ix].greyedContext;
	    menuItem->mgtMask = functionTable[ix].mgtMask;
	    return(True);
	}
    }

    /* We couldn't find the given command in the function table.
       Set up the values as if the F_Nop function were found
       and return False. */
    menuItem->greyedContext = functionTable[F_NOP_INDEX].greyedContext;
    menuItem->mgtMask = functionTable[F_NOP_INDEX].mgtMask;
    return(False);
}



/*************************************<->*************************************
 *
 *  MakeSeparatorTemplate ()
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/
static
MenuItem *MakeSeparatorTemplate (int position)
{
    MenuItem *item;

    item = (MenuItem *)XtMalloc(sizeof(MenuItem));

    /* We use the labelType to determine where this separator is positioned
       relative to the client command(s) it is surrounding, i.e. TOP or
       BOTTOM */
    item->labelType = position;
    /* Make it look like a client command: */
    item->label = XtNewString("<label-template>");
    item->labelBitmapIndex = -1;
    item->mnemonic = (KeySym) 0;
    item->accelState = 0;
    item->accelKeyCode = (KeyCode) 0;
    item->accelText = (String) NULL;
    item->wmFunction = (WmFunction) F_Separator;
    item->wmFuncArgs = (String) NULL;
    item->greyedContext = 0;
    item->mgtMask = 0;
    item->clientCommandName = (String) NULL;
    item->nextMenuItem = (MenuItem *) NULL;

    return(item);
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
