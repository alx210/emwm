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
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmResource.h"
#include "WmWinConf.h"
#include "WmXmP.h"

#include <Xm/VirtKeysP.h>

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <ctype.h>

#include <X11/Xlocale.h>
#include <stdlib.h>

#ifdef MOTIF_ONE_DOT_ONE
#include <stdio.h>
#include <pwd.h>
#else
#include <Xm/XmP.h>             /* for XmeGetHomeDirName */
#endif
#include <signal.h>

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

# define PARSE_MENU_ITEMS(pSD, mSpec) ParseMenuItems(pSD)

/*
 * include extern functions
 */
#include "WmResParse.h"
#include "WmWrkspace.h"
#include "WmError.h"
#include "WmFunction.h"
#include "WmImage.h"
#include "WmXSMP.h"

#ifdef MOTIF_ONE_DOT_ONE
extern char   *getenv ();
#endif

/*
 * Global Variables And Tables:
 */
static char cfileName[MAXWMPATH+1];
#ifndef NO_MESSAGE_CATALOG
char * pWarningStringFile;
char * pWarningStringLine;
#else
char pWarningStringFile[] = "%s: %s on line %d of configuration file %s\n";
char pWarningStringLine[] = "%s: %s on line %d of specification string\n";
#endif

static FILE *cfileP = NULL;   /* fopen'ed configuration file or NULL */
static unsigned char  line[MAXLINE+1]; /* line buffer */
static int   linec = 0;       /* line counter for parser */
static unsigned char *parseP = NULL;   /* pointer to parse string */



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
	{"super",	Mod4Mask},
    {"lock",	LockMask},
    {"mod1",	Mod1Mask},
    {"mod2",	Mod2Mask},
    {"mod3",	Mod3Mask},
    {"mod4",	Mod4Mask},
    {"mod5",	Mod5Mask},
    {NULL,      0},
};

#define ALT_INDEX 3
#define META_INDEX 4
#define SUPER_INDEX 5

typedef Boolean (*EventTableParseProcT)
	(unsigned char**, unsigned int, unsigned int*);

typedef struct {
   char         *event;
   unsigned int  eventType;
   EventTableParseProcT parseProc;
   unsigned int  closure;
   Boolean       fClick;
} EventTableEntry;

#ifdef MOTIF_ONE_DOT_ONE
void GetHomeDirName(String  fileName);
#endif
FILE *FopenConfigFile (void);
void SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
static void ParseMenuSet (WmScreenData *pSD, unsigned char *lineP);
MenuItem *ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
static MenuItem *ParseMenuItems (WmScreenData *pSD);
static Boolean ParseWmLabel (WmScreenData *pSD, MenuItem *menuItem, 
			     unsigned char *string);
static void ParseWmMnemonic (unsigned char **linePP, MenuItem *menuItem);
static Boolean ParseWmAccelerator (unsigned char **linePP, MenuItem *menuItem);
int ParseWmFunction (unsigned char **linePP, unsigned int res_spec, 
			    WmFunction *pWmFunction);
static Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
static Boolean ParseWmFuncNoArg (unsigned char **linePP, WmFunction wmFunction,
				 String *pArgs);
static Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
void FreeMenuItem (MenuItem *menuItem);
static Boolean ParseWmFuncGrpArg (unsigned char **linePP, 
				  WmFunction wmFunction, GroupArg *pGroup);
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
static char *ExtractLocaleName(String);
static Boolean ParseWmFuncNbrArg (unsigned char **linePP, 
	WmFunction wmFunction, unsigned long *pNumber);

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
    { NULL, 		0, 				NULL, 			0, 		0}
};


static EventTableEntry keyEvents[] = {

    {"key",         KeyPress,    ParseKeySym,    0,  FALSE},
    { NULL, 		0, 			NULL, 			0, 	0}
};

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
typedef Boolean (*FunctionTableParseProcT)
	(unsigned char**, WmFunction, void*);

typedef struct {
   char         * funcName;
   Context        greyedContext;
   unsigned int   resource;
   long           mgtMask;
   WmFunction     wmFunction;
   FunctionTableParseProcT parseProc;
} FunctionTableEntry;


/*
 * NOTE: New functions MUST be added in ALPHABETICAL order.  A binary search
 *       is used to find the correct function name.
 */

FunctionTableEntry functionTable[] = {
    {"f.beep",		0,
			CRS_ANY,
			0,
			F_Beep,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.circle_down",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Down,
			(FunctionTableParseProcT)ParseWmFuncGrpArg},
    {"f.circle_up",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Circle_Up,
			(FunctionTableParseProcT)ParseWmFuncGrpArg},
    {"f.create_workspace", 0,
			CRS_ANY,
			0,
			F_CreateWorkspace,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.delete_workspace", 0,
			CRS_ANY,
			0,
			F_DeleteWorkspace,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.exec",		0,
			CRS_ANY,
			0,
			F_Exec,
			(FunctionTableParseProcT)ParseWmFuncStrArg},
    {"f.focus_color",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Color,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.focus_key",	F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Focus_Key,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.goto_workspace", 0,
			CRS_ANY,
			0,
			F_Goto_Workspace,
			(FunctionTableParseProcT)ParseWmFuncStrArg},
    {"f.kill",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_CLOSE,
			F_Kill,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.lower",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Lower,
			(FunctionTableParseProcT)ParseWmFuncMaybeStrArg},
    {"f.marquee_selection",	
			F_CONTEXT_WINDOW|F_CONTEXT_ICON|F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			0,
			F_Marquee_Selection,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.maximize",	F_CONTEXT_ROOT|F_CONTEXT_MAXIMIZE|
	                               F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_MAXIMIZE,
			F_Maximize,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.menu",		0,
			CRS_ANY,
			0,
			F_Menu,
			(FunctionTableParseProcT)ParseWmFuncStrArg},
    {"f.minimize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			MWM_FUNC_MINIMIZE,
			F_Minimize,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.move",		F_CONTEXT_ROOT,
			CRS_ANY,
			MWM_FUNC_MOVE,
			F_Move,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.next_cmap",	0,
			CRS_ANY,
			0,
			F_Next_Cmap,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.next_key",	0,
			CRS_ANY,
			0,
			F_Next_Key,
			(FunctionTableParseProcT)ParseWmFuncGrpArg},
    {"f.next_workspace",	0,
			CRS_ANY,
			0,
			F_Next_Workspace,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.nop",	        F_CONTEXT_ROOT|F_CONTEXT_ICON|F_CONTEXT_WINDOW|
	                    F_SUBCONTEXT_IB_WICON | F_SUBCONTEXT_IB_IICON,
			CRS_ANY,
			0,
			F_Nop,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.normalize",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Normalize,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.normalize_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Normalize_And_Raise,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.occupy_all", F_CONTEXT_ICONBOX|F_CONTEXT_ROOT,
			CRS_ANY,
			WSM_FUNCTION_OCCUPY_WS,
 			F_AddToAllWorkspaces,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.pack_icons",	0,
			CRS_ANY,
			0,
			F_Pack_Icons,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.pass_keys",	0,
			CRS_ANY,
			0,
			F_Pass_Key,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.post_wmenu",	0,
			CRS_BUTTON|CRS_KEY,
			0,
			F_Post_SMenu,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.prev_cmap",	0,
			CRS_ANY,
			0,
			F_Prev_Cmap,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.prev_key",	0,
			CRS_ANY,
			0,
			F_Prev_Key,
			(FunctionTableParseProcT)ParseWmFuncGrpArg},
    {"f.prev_workspace",	0,
			CRS_ANY,
			0,
			F_Prev_Workspace,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.quit_mwm",	0,
			CRS_ANY,
			0,
			F_Quit_Mwm,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.quit",	0,
			CRS_ANY,
			0,
			F_Quit_Mwm,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.raise",		F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise,
			(FunctionTableParseProcT)ParseWmFuncMaybeStrArg},
    {"f.raise_lower",	F_CONTEXT_ROOT |
    				F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Raise_Lower,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.refresh",	0,
			CRS_ANY,
			0,
			F_Refresh,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.refresh_win",	F_CONTEXT_ICON|F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Refresh_Win,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.remove",	F_CONTEXT_ROOT,
			CRS_ANY,
			WSM_FUNCTION_OCCUPY_WS,
			F_Remove,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.resize",	F_CONTEXT_ICON|F_CONTEXT_ROOT|
                                 F_SUBCONTEXT_IB_IICON|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			MWM_FUNC_RESIZE,
			F_Resize,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.restart",	0,
			CRS_ANY,
			0,
			F_Restart,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.restore",	F_CONTEXT_ROOT|F_CONTEXT_NORMAL|F_SUBCONTEXT_IB_WICON,
			CRS_ANY,
			0,
			F_Restore,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.restore_and_raise",
	                F_CONTEXT_ROOT|F_CONTEXT_NORMAL,
			CRS_ANY,
			0,
			F_Restore_And_Raise,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.screen",	0,
			CRS_ANY,
			0,
			F_Screen,
			(FunctionTableParseProcT)ParseWmFuncStrArg},
    {"f.send_msg", F_CONTEXT_ROOT,
			CRS_ANY,
			0,
			F_Send_Msg,
			(FunctionTableParseProcT)ParseWmFuncNbrArg},
    {"f.separator",	0,
			CRS_MENU,
			0,
			F_Separator,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.set_behavior",	0,
			CRS_ANY,
			0,
			F_Set_Behavior,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.set_context",	0,
			CRS_ANY,
			0,
			F_Set_Context,
			(FunctionTableParseProcT)ParseWmFuncNbrArg},
    {"f.title",		0,
			CRS_MENU,
			0,
			F_Title,
			(FunctionTableParseProcT)ParseWmFuncNoArg},
    {"f.workspace_presence", F_CONTEXT_ROOT|F_CONTEXT_ICONBOX|
                        F_SUBCONTEXT_IB_WICON,
 			CRS_ANY,
			WSM_FUNCTION_OCCUPY_WS,
 			F_Workspace_Presence,
            (FunctionTableParseProcT)ParseWmFuncNoArg},
#if defined(DEBUG)
    {"f.zz_debug",	0,
			CRS_ANY,
			0,
			F_ZZ_Debug,
            (FunctionTableParseProcT)ParseWmFuncStrArg},
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
int F_ACTION_INDEX;
int F_EXEC_INDEX;
int F_NOP_INDEX;


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
 *  FindSessionMatch(commandArgc, commandArgv, pCD, pSD, pWorkSpaceList,
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
 *  FindSessionMatch - returns True if a match for this client
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
Boolean FindSessionMatch(int commandArgc, char **commandArgv,
			    ClientData *pCD, WmScreenData *pSD,
			    char **pWorkSpaceList, char *clientMachine)

{
    int count;
    int relCount;
    int argNum;
    SessionGeom *sessionGeom;


    for (count = 0; count < pSD->totalSessionItems; count++)
    {
	if (!pSD->pSessionItems[count].processed &&
            pSD->pSessionItems[count].commandArgc == commandArgc)
	{
	    if ((clientMachine) &&
		(pSD->pSessionItems[count].clientMachine) &&
		(strcmp(clientMachine, 
			pSD->pSessionItems[count].clientMachine)))
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
                          pSD->pSessionItems[count].commandArgv[argNum]))

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
		
                pSD->pSessionItems[count].processed = True;
                pSD->remainingSessionItems --;
                pCD->clientFlags |= SM_LAUNCHED;
		
		/*
		 * Free strings malloc'd for commandArgv for this item 
		 */
		
		for (relCount = 0; relCount < commandArgc; relCount++)
		{
		    XtFree(pSD->pSessionItems[count].commandArgv[relCount]);
		}
		XtFree((char *)pSD->pSessionItems[count].commandArgv);

                if(pSD->pSessionItems[count].clientState)
                {
                    pCD->clientState =
                        pSD->pSessionItems[count].clientState;
		    pCD->clientFlags |= SM_CLIENT_STATE;
                }
		
                if(pSD->pSessionItems[count].sessionGeom)
                {
                    sessionGeom = pSD->pSessionItems[count].sessionGeom;
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
		    
		    XtFree((char *)pSD->pSessionItems[count].sessionGeom); 
                }

                if(pSD->pSessionItems[count].clientMachine)
                {
		    /*
		     * Free clientMachine malloc'd space for this item 
		     */
		    
		    XtFree((char *)
			   pSD->pSessionItems[count].clientMachine); 
		    pSD->pSessionItems[count].clientMachine = NULL;
                }
		
		
                if(pSD->pSessionItems[count].workspaces)
                {
		    /*
		     * The caller is responsible for freeing this
		     * data.
		     */
		    *pWorkSpaceList = pSD->pSessionItems[count].workspaces;
                }


		if(pSD->remainingSessionItems == 0)
		{
		    /*
		     * Free the whole pSD->pSessionItems structure 
		     */
		    XtFree((char *)pSD->pSessionItems);
		}
		
		return (True);
            }

        } /* not processed and argc's are the same */

    } /* for */
    
    return (False);
    
} /* END OF FUNCTION FindSessionMatch */






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
	pSD->pSessionItems[count].clientState = NORMAL_STATE;
    }
    else if(!strcmp((char *)string, "IconicState"))
    {
	pSD->pSessionItems[count].clientState = MINIMIZED_STATE;
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

	pSD->pSessionItems[count].sessionGeom = pTmpSessionGeom;
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

    if ((pSD->pSessionItems[count].workspaces =
         (String)XtMalloc ((unsigned int) (strlen((char *)string) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(60, 2, "Insufficient memory for workspaces list in sesssion item")));
        return;

    }

    strcpy(pSD->pSessionItems[count].workspaces, (char *)string);
    
} /* END OF FUNCTION ParseSessionWorkspaces */


#ifdef CDE_COMPAT
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
    if ((pSD->pSessionItems[count].commandArgv =
         (char **)XtMalloc ((argc) * sizeof(char * ))) == NULL)
    {
        /*
         * Allocate space for saved argv
         */
	
        Warning (((char *)GETMESSAGE(60, 3, "Insufficient memory for commandArgv array")));
    }
    else
    {
        pSD->pSessionItems[count].commandArgc = argc;
        for (xindex = 0; xindex < argc ; xindex++)
        {
            if ((pSD->pSessionItems[count].commandArgv[xindex] =
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
                strcpy(pSD->pSessionItems[count].commandArgv[xindex],
                       (char *)argv[xindex]);
            }
        }
    }

    XtFree ((char *) argv);
    
} /* END OF FUNCTION ParseSessionCommand */
#endif /* CDE_COMPAT */


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

    if ((pSD->pSessionItems[count].clientMachine =
         (String)XtMalloc ((unsigned int) (strlen((char *)string) + 1))) == 
	NULL)
    {
        Warning (((char *)GETMESSAGE(60, 38, 
		"Insufficient memory for host name in sesssion item")));
        return;
    }

    strcpy(pSD->pSessionItems[count].clientMachine, (char *)string);
    
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
   
    if ((pSD->pSessionItems =
	 (WmSessionItem *)XtMalloc (numItems * sizeof (WmSessionItem)))
        == NULL)
    {
        Warning (((char *)GETMESSAGE(60, 5, "Insufficient memory for WM Session Hints")));
        return(False);
    }
    
    memset ((char *)pSD->pSessionItems, 0,
	    numItems * sizeof (WmSessionItem));

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
		KeySym ks = XkbKeycodeToKeysym(DISPLAY, map->modifiermap[k], 0, 0);
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
		else if (nm && !strncmp("Super", nm, 5))
		{
		    modifierStrings[SUPER_INDEX].mask = (1<<i);
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
void ProcessWmFile (WmScreenData *pSD)
{
    unsigned char *lineP;
    unsigned char *string;
    unsigned int   n;
    MenuSpec      *menuSpec;

    /*
     * Initialize global data values that are set based on data in
     * the mwm resource description file.
     */

    pSD->buttonSpecs = NULL;
    pSD->keySpecs = NULL;
    pSD->menuSpecs = NULL;

    /**** hhhhhhhhhhhh   ******/
    GetFunctionTableValues (&F_EXEC_INDEX, &F_NOP_INDEX, &F_ACTION_INDEX);

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

static char *ExtractLocaleName(String lang)
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
		return (fileP);
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
		  return (fileP);
		}
	    }

	    
	}
	else
	/* relative to current directory or absolute */
	{
      if ((fileP = fopen (wmGD.configFile, "r")) != NULL)
      {
		if (LANG != NULL) {
		    XtFree(LANG);
		    LANG = NULL;
		}
		return(fileP);
      }
	}
    }

    /*
     * The configFile resource didn't do it for us.
     * First try HOME_MWMRC, then try SYS_MWMRC .
     */

#define HOME_MWMRC "/.emwmrc"
#define SLASH_MWMRC "/system.emwmrc"

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
    strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));

    if ((fileP = fopen (cfileName, "r")) != NULL)
    {
        if (LANG != NULL) {
	    XtFree(LANG);
	    LANG = NULL;
		}
        return (fileP);
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

    strncat(cfileName, HOME_MWMRC, MAXWMPATH - strlen(cfileName));
	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  if (LANG != NULL) {
	      XtFree(LANG);
	      LANG = NULL;
	  }
	  return (fileP);
	}
    }

#ifndef RCDIR
#define RCDIR "/usr/lib/X11"
#endif
    if (LANG != NULL)
    {
   /*
	* Try /$LANG/system.mwmrc within the install tree
	*/
	strcpy(cfileName, RCDIR);
	strncat(cfileName, "/", MAXWMPATH-strlen(cfileName));
	strncat(cfileName, LANG, MAXWMPATH-strlen(cfileName));
	strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));

	if ((fileP = fopen (cfileName, "r")) != NULL)
	{
	  XtFree(LANG);
	  LANG = NULL;
	  return (fileP);
	}
    }
    /*
     * Try /system.mwmrc within the install tree
     */
    strcpy(cfileName, RCDIR);
    strncat(cfileName, SLASH_MWMRC, MAXWMPATH - strlen(cfileName));

    if (LANG != NULL) 
    {
       XtFree(LANG);
       LANG = NULL;
    }
    return (fopen (cfileName, "r"));

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

static MenuItem *ParseMenuItems (WmScreenData *pSD)
{
    unsigned char *string;
    unsigned char *lineP;
    MenuItem      *firstMenuItem;
    MenuItem      *lastMenuItem;
    MenuItem      *menuItem;
    register int   ix = 0;
    
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
	ix = ParseWmFunction (&lineP, CRS_MENU, &menuItem->wmFunction);

	/*
	 * Determine context sensitivity and applicability mask.
	 */

	menuItem->greyedContext = functionTable[ix].greyedContext;
	menuItem->mgtMask = functionTable[ix].mgtMask;
    /* 
	 * Apply the function argument parser.
	 */
	if (!(*(functionTable [ix].parseProc)) 
	   (&lineP, menuItem->wmFunction, &menuItem->wmFuncArgs))
	{
	FreeMenuItem (menuItem);
	continue;  /* skip this menu item */
	}

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

    }

    return (firstMenuItem);

} /* END OF FUNCTION ParseMenuItems */


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
        if ((menuItem->labelBitmapIndex = GetBitmapIndex (pSD, 
					   (char *)string, True)) >= 0)
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

	if (menuItem->labelType == XmSTRING &&
	    mnemonic != NULL &&
	    (ks = XStringToKeysym((char *)mnemonic)) != NoSymbol &&
	    strchr(menuItem->label, (char)(ks & 0xff)) != NULL)
	{
	    menuItem->mnemonic = ks;
	}
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

static Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
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

static Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs)
{
    unsigned char *string;
    unsigned int  len;
    char *p;
    wchar_t last;
    char delim;
    wchar_t wdelim;
    int lastlen;

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

	if (wmFunction == F_Exec)
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
        else if (!strcmp ("ifkey", (char *)ctxStr))
        {
            *context |= F_CONTEXT_IFKEY;
        }
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
    Boolean 	bBadKey;
    
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
	bBadKey = False;
	if (!ParseKeyEvent(&lineP, &eventType, &keySpec->keycode, &keySpec->state))
	{
	    bBadKey = True;
	}

	/*
	 * Parse the key context.
	 *   Here lineP points to the candidate context string.
	 */

	if (!ParseContext(&lineP, &keySpec->context, 
			  &keySpec->subContext))
	{
	    if (bBadKey)
		PWarning (((char *)GETMESSAGE(60, 27, "Invalid key specification")));
        PWarning (((char *)GETMESSAGE(60, 28, "Invalid key context")));
	    XtFree ((char *)keySpec);
	    continue;  /* skip this key specification */
	}

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
    int   chlen;
    wchar_t last;
    wchar_t wdelim;
    char delim;
    int lastlen;

    if (cfileP != NULL)
    /* read fopened file */
    {
	if ((string = (unsigned char *) 
		      fgets ((char *)line, MAXLINE, cfileP)) != NULL)
	{
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
	}
    }
    else if ((parseP != NULL) && (*parseP != '\0'))
    /* read parse string */
    {
	string = line;

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

unsigned char *GetString (unsigned char **linePP)
{
    unsigned char *lineP = *linePP;
    unsigned char *endP;
    unsigned char *curP;
    unsigned char *lnwsP;
    int            chlen;

    /* get rid of leading white space */
    ScanWhitespace (&lineP);

    /*
     * Return NULL if line is empty, a comment, or invalid.
     */
    if (
	*lineP == '\0' ||
	((chlen = mblen ((char *)lineP, MB_CUR_MAX)) < 1) ||
        ((chlen > 0) && ((*lineP == '!') || (*lineP == '#')))
       )
    {
        *linePP = lineP;
        return (NULL);
    }

    if ((chlen == 1) && (*lineP == '"'))
    /* Quoted string */
    {
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
		*endP = *curP++;
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

        while ((*endP = *curP) &&
               ((chlen = mblen ((char *)curP, MB_CUR_MAX)) > 0) &&
               ((chlen > 1) || (!isspace (*curP) && (*curP != '#'))))

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
    }

    /*
     * Three cases for *endP:
     *   '#' --> write NULL over # and point to NULL
     *   whitespace or
     *     matching quote -> write end-of-line over char and point beyond
     *   NULL -> point to NULL 
     */

    if (*endP == '#')
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
    int            chlen;

    ScanWhitespace (&lineP);
    startP = lineP;

    while (*lineP &&
	   ((chlen = mblen ((char *)lineP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) ||
	   (!isspace (*lineP) && *lineP != ',' && *lineP != ':')))
    {
	/* Skip next character */
        lineP += chlen;
    }

    len = min (lineP - startP, MAX_KEYSYM_STRLEN);
    (void) strncpy (keySymName, (char *)startP, len);
    keySymName[len] = '\0';

    if ((*detail = XStringToKeysym(keySymName)) == NoSymbol &&
	 (mblen (keySymName, MB_CUR_MAX) == 1))
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
    int            chlen;

    while (*linePP &&
	   ((chlen = mblen ((char *) *linePP, MB_CUR_MAX)) > 0) &&
           ((chlen > 1) || isalnum (**linePP)))
    {
        (*linePP) += chlen;
    }

} /* END OF FUNCTION ScanAlphanumeric */



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
    while (*linePP && (mblen ((char *)*linePP, MB_CUR_MAX) == 1) && isspace (**linePP))
    {
        (*linePP)++;
    }

} /* END OF FUNCTION ScanWhitespace */



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
    int            chlen;

    while (*pch && ((chlen = mblen ((char *)pch, MB_CUR_MAX)) > 0))
    {
        if ((chlen == 1) && (isupper (*pch)))
	{
	    *pch = tolower(*pch);
	}
	pch += chlen;
    }

} /* END OF FUNCTION ToLower */



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
} /* END OF FUNCTION PWarning */


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
    int		chlen;

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
    while (*pch1 && ((chlen = mblen ((char *)pch1, MB_CUR_MAX)) > 0))
	{
	    if ((chlen == 1) && (*pch1 == ' '))
	    {
		break;
	    }
	    pch1 += chlen;
	}
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

    while (*pch1 && ((chlen = mblen ((char *)pch1, MB_CUR_MAX)) > 0))
	{
	    if ((chlen == 1) && (*pch1 == ' '))
	    {
		break;
	    }
	    pch1 += chlen;
	}

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
    int   chlen;
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
	    pchFirst = startP;

        while (*startP &&
	       (((chlen = mblen ((char *)startP, MB_CUR_MAX)) > 1)
		|| isalnum (*startP)))
        {
	    while (chlen--)
	    {
	        startP++;
	    }
	}

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

	*destP++ = '+';

        ScanWhitespace (&startP);
    }

    /*
     * Skip the key event type.
     */
    startP++;  /* skip '<' */
    while (*startP != '>')
    {
        startP += mblen ((char *)startP, MB_CUR_MAX);
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
#define VERSION_OPT "-version"

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
	else if (!strcmp((char *)string, VERSION_OPT))
	{
		PrintVersionInfo();
		exit(0);
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

#ifdef UNUSED
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
#endif /* UNUSED */

/****************************   eof    ***************************/
