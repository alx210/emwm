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
static char rcsid[] = "$TOG: WmFunction.c /main/19 1998/04/20 13:00:48 mgreess $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993, 1994 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#define FIX_1350    1

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include <sys/types.h>
#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#else
extern int putenv();
extern char *getenv();
extern int atoi();
extern pid_t wait();

#ifndef PORT_NOVFORK
extern pid_t vfork();
#endif /* PORT_NOVFORK */
#endif
#ifndef WSM
#include <signal.h>
#endif
#include <stdio.h>
#include <X11/Xos.h>
#include "WmICCC.h"
#ifdef WSM
#include "WmWrkspace.h"  /* for ClientInWorkspace() test */
#include <Dt/EnvControlP.h>  /* for restoring **environ before an exec() */
#include "WmResNames.h"
#include <Dt/Message.h>
#include <Dt/Help.h>
#endif /* WSM */
#ifdef PANELIST
#include <Dt/DtStrDefs.h>
#include "WmPanelP.h"
#include "WmSignal.h"
#endif /* PANELIST */

/*
 * include extern functions
 */
#include "WmFunction.h"
#include "WmCEvent.h"
#ifdef WSM
#include "WmHelp.h"
#endif /* WSM */
#include "WmCDInfo.h"
#include "WmColormap.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#ifdef WSM
#include "WmIPC.h"
#endif /* WSM */
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#ifdef WSM
#include "WmPresence.h"
#endif /* WSM */
#include "WmProperty.h"
#include "WmProtocol.h"
#include "WmResParse.h"
#include "WmWinConf.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#include "WmXSMP.h"

#include <Xm/RowColumnP.h> /* for MS_LastManagedMenuTime */
extern XmMenuState _XmGetMenuState();

static unsigned int GetEventInverseMask(XEvent *event);

#ifdef WSM

#if (defined(USL) || defined(__uxp__) || defined(linux)) && !defined(_NFILE)
#define _NFILE FOPEN_MAX
#endif
#define CLOSE_FILES_ON_EXEC() \
{int ifx; for (ifx=3; ifx < _NFILE; ifx++) (void) fcntl (ifx, F_SETFD, 1);}

#endif /* WSM */
/*
 * Global Variables:
 */

/*
 * The 'dirty' variables are used to keep track of the transient window
 * that has been lowered via "f.lower freeFamily".
 */
static ClientData *dirtyStackEntry = NULL;
static ClientData *dirtyLeader = NULL;

#ifdef WSM

/***********************<->*************************************
 *
 *  F_Action (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for invoking actions.
 *
 *
 *  Inputs:
 *  ------
 *  args = action function and arguments
 *
 *  pCD = pointer to the ClientData for the whole front panel
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *  Comments:
 *  -------
 *  The pCD->controlWindow is a temporary area used solely for
 *  this function invocation--it stores the pCW of the control
 *  that want the function to happen.
 ******************************<->***********************************/

Boolean
F_Action (String actionName, ClientData *pCD, XEvent *event)
{

    WmActionArg *pAP = (WmActionArg *) actionName;

    /* make sure the command runs on the right display. */
    if (wmGD.pActiveSD->displayString)
    {
	putenv(wmGD.pActiveSD->displayString);
    }

    if (wmGD.dtSD)
    {
	WmFrontPanelSetBusy (True);
    }

    
    DtActionInvoke (wmGD.pActiveSD->screenTopLevelW1,
		    pAP->actionName, pAP->aap, pAP->numArgs,
                    pAP->szExecParms, NULL, NULL, 1, NULL, NULL);
    
    /*
     * Restore original DISPLAY environment variable 
     */
    if(wmGD.pActiveSD->displayString && wmGD.displayString)
    {
	putenv(wmGD.displayString);
    }

    return (True);
    
} /* END OF FUNCTION F_Action */
#endif /* WSM */


/******************************<->*************************************
 *
 *  F_Beep (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for beeping.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 ******************************<->***********************************/

Boolean F_Beep (String args, ClientData *pCD, XEvent *event)
{

    /* !!! what is a good value for percent (the second arg) !!! */
    XBell (DISPLAY, 0);

    return (True);

} /* END OF FUNCTION F_Beep */



/*
 * Handle Special case where the dirty window is the top most
 * transient window.  When this is the case, raising the window
 * that was on top (the window just below the dirty window) will
 * fail because Mwm stack database is out of sync.  So the solution
 * is to restack the dirty transient relative to the second to the
 * top transient.  This function is used to support freeFamily stacking.
 */
ClientData * FindSecondToTopTransient (pcd)
ClientData *pcd;
{
    ClientData *pcdNext;
    static ClientData *second;

    pcdNext = pcd->transientChildren;
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    if (!pcdNext->transientChildren->transientChildren)
	    {
		second = pcdNext;
	    }
	    FindSecondToTopTransient (pcdNext);
	}
	pcdNext = pcdNext->transientSiblings;
	if (pcdNext && !pcdNext->transientSiblings)
	{
	    second = pcdNext;
	}
    }

    return (second);

} /* END OF FUNCTION */



Boolean ForceLowerWindow (ClientData *pcd)
{
#if 0
    Window stackWindow;
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
#endif
    XWindowChanges changes;
    Boolean restack = False;
#ifdef WSM
    Window stackWindow;
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
    unsigned int mask;
    ClientListEntry 	*pCLE;

    /* 
     * Find lowest window in this workspace. We'll stack this transient
     * below it.
     */
    pCLE = pSD->lastClient;
    stackWindow = None;
    mask = CWStackMode;
    while (pCLE != NULL)
    {
	if ((pCLE->pCD != pcd) &&
	    (ClientInWorkspace (ACTIVE_WS, pCLE->pCD)))
	{
	    if ((pCLE->type == MINIMIZED_STATE) &&
		(pCLE->pCD->clientState == MINIMIZED_STATE))
	    {
		stackWindow = ICON_FRAME_WIN(pCLE->pCD);
	    }
	    else if ((pCLE->type == NORMAL_STATE) &&
		     ((pCLE->pCD->clientState == NORMAL_STATE) ||
		      (pCLE->pCD->clientState == MAXIMIZED_STATE)))
	    {
		stackWindow = pCLE->pCD->clientFrameWin;
	    }

	    if (stackWindow != None)
	    {
		mask |= CWSibling;
		changes.sibling = stackWindow;
		break;
	    }
	}
	if (stackWindow == None)
	{
	    pCLE = pCLE->prevSibling;
	}
    }
#endif /* WSM */
#if 0
    if (pSD->lastClient->type == MINIMIZED_STATE)
    {
	stackWindow = ICON_FRAME_WIN(pSD->lastClient->pCD);
    }
    else
    {
	stackWindow = pSD->lastClient->pCD->clientFrameWin;
    }
#endif

    changes.stack_mode = Below;
#ifdef WSM
    if (mask)
    {
	XConfigureWindow (DISPLAY, pcd->clientFrameWin, mask, &changes);
    }
#else /* WSM */
    XConfigureWindow (DISPLAY, pcd->clientFrameWin, CWStackMode,
		      &changes);
#endif /* WSM */

    return (restack);
}



/*************************************<->*************************************
 *
 *  F_Lower (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for bottoming a client window
 *  or icon.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

Boolean F_Lower (String args, ClientData *pCD, XEvent *event)
{
    ClientListEntry *pEntry;
    ClientListEntry *pNextEntry;
    ClientListEntry *pStackEntry;
    String string = args;
    int flags = STACK_NORMAL;
#ifdef WSM
    WmWorkspaceData *pWS = ACTIVE_WS;
#endif /* WSM */

    if (string)
    {
	/* process '-client' argument */
	if (string[0] == '-')
	{
	    string = &string[1];
	    string = (String) GetString ((unsigned char **) &string);

	    pStackEntry = NULL;
	    pNextEntry = ACTIVE_PSD->lastClient;
	    while (pNextEntry &&
		   (pEntry = FindClientNameMatch (pNextEntry, False,
							string,	F_GROUP_ALL)))
	    {
		pNextEntry = pEntry->prevSibling;
#ifdef WSM
	        if (ClientInWorkspace (pWS, pEntry->pCD))
	        {
#endif /* WSM */
		Do_Lower (pEntry->pCD, pStackEntry, STACK_NORMAL);
		pStackEntry = pEntry;
#ifdef WSM
	        }
#endif /* WSM */
	    }
	}
	/* process family stacking stuff */
	else if (*string)
	{
	    unsigned int  slen, len, index;

	    slen = strlen(args) - 2;		/* subtract '\n' and NULL */
	    for (index = 0; index < slen; string = &args[index+1])
	    {
		if ((string = (String) GetString ((unsigned char **) &string)) == NULL)
		   break;
		len = strlen(string);
		if (!strcmp(string,"within"))
		{
		    flags |= STACK_WITHIN_FAMILY;
		}
		else if (!strcmp(string,"freeFamily"))
		{
		    flags |= STACK_FREE_FAMILY;
		}
		index += len;
	    }
#ifdef WSM
	    if (ClientInWorkspace (pWS, pCD))
	    {
#endif /* WSM */
	    Do_Lower (pCD, (ClientListEntry *) NULL, flags);
#ifdef WSM
	    }
#endif /* WSM */
	}
    }
    else if (pCD)
    {
#ifdef WSM
	    if (ClientInWorkspace (pWS, pCD))
	    {
#endif /* WSM */
	Do_Lower (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
#ifdef WSM
	    }
#endif /* WSM */
    }

    /*
     * If caused by button press, event may ALSO cause focus to be
     * passed to this client - prepare to disable focusAutoRaise.
     */
    if (pCD && event && (event->type == ButtonPress))
      pCD->focusAutoRaiseDisablePending = True;

    wmGD.passButtonsCheck = False;
    return (True);

} /* END OF FUNCTION F_Lower */



/*************************************<->*************************************
 *
 *  Do_Lower (pCD, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for lowering the client window
 *  so that it does not obscure any other window above the stack entry
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data of the window (or icon) to be lowered.
 * 
 *  pStackEntry = pointer to client list entry for window that is to be 
 *	below the lowered window (if NULL, window is lowered to the bottom
 *	of the	stack).
 *
 *************************************<->***********************************/

void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags)
{
    Boolean restackTransients;
    ClientData *pcdLeader;
    WmWorkspaceData *pWS = ACTIVE_WS;
#ifdef WSM
    Boolean bLeaderRestacked;
#endif /* WSM */

#ifdef PANELIST
    if (pCD->pECD)
    {
	/*
	 * Window has been reparented into the front panel. 
	 * Don't follow through on window stacking change.
	 */
	return;
    }
    else 
#else /* PANELIST */
#endif /* PANELIST */
#ifdef WSM
    if (ClientInWorkspace(pWS, pCD)  && 
	(!pStackEntry || ClientInWorkspace (pWS, pStackEntry->pCD)))
    {
	/* 
	 * Both clients are in the current workspace. Set
	 * client indices so that the access macros work.
	 */
	SetClientWsIndex (pCD);
	if (pStackEntry)
	{
	    SetClientWsIndex (pStackEntry->pCD);
	}
    }
    else
    {
	/*
	 * One or both of the clients are not in the current workspace
	 * Do nothing.
	 */
	return;
    }
#endif /* WSM */

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    if ((pcdLeader->clientState == MINIMIZED_STATE) && !P_ICON_BOX(pcdLeader))
    {
        /*
         * If a dirtyStackEntry exists, return it to its original place 
         * in the stack (for all stacking types)
         */
        if (dirtyStackEntry)
        {
            if (dirtyStackEntry->transientChildren ||
                dirtyStackEntry->transientLeader)
                RestackTransients (dirtyStackEntry);
            dirtyStackEntry = NULL;
            dirtyLeader = NULL;
	}

	/*
	 * Only restack the icon if it is not currently lowered.
	 */

	if (pStackEntry)
	{
	    if (pStackEntry->prevSibling != &pcdLeader->iconEntry)
	    {
		StackWindow (pWS, &pcdLeader->iconEntry, True /*above*/,
		    pStackEntry);
		MoveEntryInList (pWS, &pcdLeader->iconEntry, True /*above*/,
		    pStackEntry);
	    }
	}
	else
	{
	    if (ACTIVE_PSD->lastClient != &pcdLeader->iconEntry)
	    {
		StackWindow (pWS, &pcdLeader->iconEntry, 
			     False /*on bottom*/, (ClientListEntry *) NULL);
		MoveEntryInList (pWS, &pcdLeader->iconEntry, 
			     False /*on bottom*/, (ClientListEntry *) NULL);
	    }
	}
    }
    else /* NORMAL_STATE, MAXIMIZED_STATE, adoption */
    {
#ifdef WSM
        /*
	 * Handle restacking of primary/secondary windows
	 * within the transient window tree.
	 */
        bLeaderRestacked = False;
	if ((pcdLeader->transientChildren) &&
	    (!pcdLeader->secondariesOnTop) &&
	    (!wmGD.bSuspendSecondaryRestack))
	{
	    if (pCD == pcdLeader)
	    {
		/*
		 * Lower requested on the leader itself, insure it's
		 * at the bottom.
		 */
		bLeaderRestacked = BumpPrimaryToBottom (pcdLeader);
	    }
	    else if (pCD->transientChildren)
	    {
		/*
		 * Lower requested on the leader of a subtree. Insure
		 * that this subtree leader is at the bottom of the
		 * subtree.
		 */
		bLeaderRestacked = BumpPrimaryToBottom (pCD);
	    }
	    else if (pCD->transientLeader)
	    {
		ClientData *pcdLdr;

		/*
		 * Lower requested on a transient. Insure all the
		 * subtree leaders up to the top are at the bottom
		 * of their respective transient subtrees.
		 */
		for (pcdLdr = pCD->transientLeader;
			pcdLdr; 
				pcdLdr = pcdLdr->transientLeader)
		{
		    bLeaderRestacked |= BumpPrimaryToBottom (pcdLdr);
		}
	    }

	}
#endif /* WSM */
	/*
	 * If this is a transient window then put it below its
	 * sibling transient windows.
	 */

	restackTransients = False;
	if (pCD->transientLeader)
	{

	    /*
	     * If freeFamily stacking, then put dirty transient window
	     * (if any) back in place before force lowering current window
	     * to the bottom of the global window stack.  Then return.
	     */

	    if (flags & STACK_FREE_FAMILY)
	    {
		/* Restore dirty transient if not current window. */
		if ((dirtyStackEntry) &&
		    (dirtyStackEntry != pCD))
		{
		    RestackTransients (dirtyStackEntry);
		}

		dirtyStackEntry = pCD;
		dirtyLeader = pcdLeader;

		ForceLowerWindow (pCD);
		return;
	    }

	    /*
	     * Reach here only if NOT doing a f.lower freeFamily (see
	     * return; statement above).  Put current transient below
	     * its sibling transient windows.
	     */
	    restackTransients = PutTransientBelowSiblings (pCD);
	}

	/*
	 * If doing a regular f.lower and you have a dirty window, then
	 * clean up dirty transient window.
	 */

	if (dirtyStackEntry)
	{
	    /* 
	     * If lowering a window in the same family as the dirty
	     * transient window, then just restack before lowering.
	     * Else, restore the dirty transient in place before
	     * lowering the current window.  Clear dirtyStack.
	     */
	    if (dirtyLeader == pcdLeader)
	    {
		restackTransients = True;
	    }
	    else
	    {
		RestackTransients (dirtyStackEntry);
	    }

	    dirtyStackEntry = NULL;
	}

	/*
	 * Only restack the window or transient window tree if it is
	 * not currently lowered and the window is not a system
	 * modal window.
	 */

	if (pStackEntry)
	{
	    if ((pStackEntry->prevSibling != &pcdLeader->clientEntry) &&
		!(wmGD.systemModalActive &&
		  (pcdLeader == wmGD.systemModalClient)))
	    {
	        StackWindow (pWS, &pcdLeader->clientEntry, True /*above*/,
		    pStackEntry);
		MoveEntryInList (pWS, &pcdLeader->clientEntry, True /*above*/,
		    pStackEntry);
	    }
#ifdef WSM
	    else if ((restackTransients) || (bLeaderRestacked))
#else /* WSM */
	    else if (restackTransients)
#endif /* WSM */
	    {
		RestackTransients (pCD);
	    }
	}
	else
	{
	    if ((pWS->pSD->lastClient != &pcdLeader->clientEntry) &&
		!(wmGD.systemModalActive &&
		  (pcdLeader == wmGD.systemModalClient)) &&
		!(flags & STACK_WITHIN_FAMILY))
	    {
	        StackWindow (pWS, &pcdLeader->clientEntry, False /*on bottom*/,
		    (ClientListEntry *) NULL);
		MoveEntryInList (pWS, &pcdLeader->clientEntry,
		    False /*on bottom*/, (ClientListEntry *) NULL);
	    }
#ifdef WSM
	    else if ((restackTransients) || (bLeaderRestacked))
#else /* WSM */
	    else if (restackTransients)
#endif /* WSM */
	    {
		RestackTransients (pCD);
	    }
	}
    }

} /* END OF FUNCTION Do_Lower */



/*************************************<->*************************************
 *
 *  F_CircleDown (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving the client window
 *  on top of stack to the bottom.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event)
{
    unsigned long types;
    unsigned long windowType;
    ClientListEntry *pNextEntry;
    ClientData *pcdNext;


    /*
     * Go down through the client list looking for a window of an
     * appropriate type that is obscuring lower windows.
     */

    types = (unsigned long)args;
    pNextEntry = ACTIVE_PSD->clientList;
    while (pNextEntry)
    {
	/*
	 * Only check out the window if it is onscreen.
	 */

	pcdNext = pNextEntry->pCD;
	if (((pNextEntry->type == NORMAL_STATE) &&
	     (pcdNext->clientState != MINIMIZED_STATE)) ||
	    ((pNextEntry->type == MINIMIZED_STATE) &&
	     (pcdNext->clientState == MINIMIZED_STATE)))
	{
	    if (pcdNext->clientState == MINIMIZED_STATE)
	    {
		windowType = F_GROUP_ICON;
	    }
	    else
	    {
		windowType = F_GROUP_WINDOW;
		if (pcdNext->transientLeader || pcdNext->transientChildren)
		{
		    windowType |= F_GROUP_TRANSIENT;
		}
	    }
	    if (types & windowType)
	    {
		if (CheckIfClientObscuringAny (pcdNext))
		{
		    /*
		     * This window (or window tree) is obscuring another window
		     * on the screen.  Lower the window.
		     */

#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
		    F_Lower (NULL, pcdNext, (XEvent *) NULL);
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
		    break;
		}
	    }
	}
	pNextEntry = pNextEntry->nextSibling;
    }

    return (True);

} /* END OF FUNCTION F_Circle_Down */



/*************************************<->*************************************
 *
 *  F_Circle_Up (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving the client window
 *  on the bottom of the stack to the top.
 *
 *
 *  Inputs:
 *  ------
 *  args = function arguments (specified in .mwmrc file)
 *
 *  pCD = pointer to the client data for the client window to which the
 *        function is to be applied
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event)
{
    unsigned long types;
    unsigned long windowType;
    ClientListEntry *pNextEntry;
    ClientData *pcdNext;


    /*
     * Go up through the client list looking for a window of an
     * appropriate type that is obscured by higher windows.
     */

    types = (unsigned long)args;
    pNextEntry = ACTIVE_PSD->lastClient;
    while (pNextEntry)
    {
	/*
	 * Only check out the window if it is onscreen.
	 */

	pcdNext = pNextEntry->pCD;
	if (((pNextEntry->type == NORMAL_STATE) &&
	     (pcdNext->clientState != MINIMIZED_STATE)) ||
	    ((pNextEntry->type == MINIMIZED_STATE) &&
	     (pcdNext->clientState == MINIMIZED_STATE)))
	{
	    if (pcdNext->clientState == MINIMIZED_STATE)
	    {
		windowType = F_GROUP_ICON;
	    }
	    else
	    {
		windowType = F_GROUP_WINDOW;
		if (pcdNext->transientLeader || pcdNext->transientChildren)
		{
		    windowType |= F_GROUP_TRANSIENT;
		}
	    }
	    if (types & windowType)
	    {
		if (CheckIfClientObscuredByAny (pcdNext))
		{
		    /*
		     * This window (or window tree) is obscured by another
		     * window on the screen.  Raise the window.
		     */

#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
		    F_Raise (NULL, pcdNext, (XEvent *) NULL);
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
		    break;
		}
	    }
	}
	pNextEntry = pNextEntry->prevSibling;
    }

    return (True);


} /* END OF FUNCTION F_Circle_Up */



/*************************************<->*************************************
 *
 *  F_Focus_Color (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the colormap
 *  focus to a client window or reinstalling the default colormap.
 *
 *************************************<->***********************************/

Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event)
{

    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_EXPLICIT)
    {
        if (pCD)
        {
	    /*
	     * The window selected for the colormap focus is a top-level client
	     * window.  If there are subwindow colormaps then determine if the
	     * selection was in one of the subwindows.
	     */

	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		/* !!! colormap for client supplied icon window !!! */
		pCD = NULL;
	    }
        }

        SetColormapFocus (ACTIVE_PSD, pCD);
    }

    return (True);

} /* END OF FUNCTION F_Focus_Color */



/*************************************<->*************************************
 *
 *  F_Exec (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for executing a command
 *  (with /bin/sh).
 *
 *************************************<->***********************************/

Boolean F_Exec (String args, ClientData *pCD, XEvent *event)
{
    int   status;
    int   pid;
    int   w;
#ifndef WSM
    void (*intStat) ();
    void (*quitStat) ();
    void (*chldStat) ();
#endif /* WSM */
    char *shell;
    char *shellname;


    /* make sure the f.exec command runs on the right display. */
    if (wmGD.pActiveSD->displayString)
      {
	putenv(wmGD.pActiveSD->displayString);
      }
    
#ifdef PANELIST
    if (wmGD.dtSD)
    {
	/*
	 * Start the busy indicator, waiting for a pushbutton window
	 * for the given duration
	 */
       WmFrontPanelSetBusy (True);
    }
#endif /* PANELIST */

#ifndef WSM
    /* For now use only for non WSM code.  May integrate to WSM later. */
    /*
     * Disable SIGCHLD while we wait for this child to exit, otherwise
     * we will go into the ChildProcSignalHandler and will never get the
     * correct child pid in the while loop below. pgw@ixi 30-5-95 IBIS 20585.
     *
     * Moved before fork() to avoid race condition.
     * Change handler to SIG_DFL, SIG_IGN causes bug. paulsh@sequent 31-08-95.
     */
    chldStat = (void (*)())signal (SIGCHLD, SIG_DFL); 
#endif

    /*
     * Fork a process to exec a shell to run the specified command:
     */

#ifdef PORT_NOVFORK
    if ((pid = fork ()) == 0)
#else
    if ((pid = vfork ()) == 0)
#endif
    {

#ifndef NO_SETPGRP
#if defined(SVR4) || defined(__OSF1__) || defined(__osf__) || defined(_POSIX_JOB_CONTROL)
	setsid();
#else
#ifdef SYSV
	setpgrp();
#else
	int tpid;

	tpid = getpid();
	setpgrp(tpid, tpid);
#endif /* SYSV */
#endif /* SVR4 */
#endif /* NO_SETPGRP */
#ifdef WSM 
	/*
	 * Clean up window manager resources.
	 * The X file descriptor should be automatically closed.
	 */

	/*
	 * Fix up signal handling.
	 */
	RestoreDefaultSignalHandlers ();

	/*
	 * Fix up the child application's environment NOT to
	 * inherit the XFILESEARCHPATH, XBMLANGPATH, NLSPATH, etc.
	 * used by dtwm.
	 */
	 _DtEnvControl(DT_ENV_RESTORE_PRE_DT);

	 CLOSE_FILES_ON_EXEC();
#endif /* WSM */

	/*
	 * Exec the command using $MWMSHELL if set or 
	 * $SHELL if set and $MWMSHELL not set or sh.
	 */

        if (((shell = getenv ("MWMSHELL")) != NULL) ||
	    ((shell = getenv ("SHELL")) != NULL))

	{
	    shellname = strrchr (shell, '/');
	    if (shellname == NULL)
	    {
		/*
		If the shell pathname obtained from SHELL or MWMSHELL does not
		have a "/" in the path and if the user expects this shell to be
		obtained using the PATH variable rather than the current
		directory, then we must call execlp and not execl
		*/
		shellname = shell;
		execlp (shell, shellname, "-c", args, 0);
	    }
	    else
	    {
		shellname++;
		execl (shell, shellname, "-c", args, 0);
	    }
	}

	/*
	 * There is no SHELL environment variable or the first execl failed.
	 * Try /bin/sh .
	 */
#ifdef SVR4
        execl ("/usr/bin/sh", "sh", "-c", args, 0);
#else
        execl ("/bin/sh", "sh", "-c", args, 0);
#endif


	/*
	 * Error - command could not be exec'ed.
	 */

	_exit (127);
    }

    else if (pid == -1)
      return(True);

    /*
     * Have the window manager wait for the shell to complete.
     */

#ifndef WSM
    intStat = (void (*)())signal (SIGINT, SIG_IGN);
    quitStat = (void (*)())signal (SIGQUIT, SIG_IGN);
#endif /* WSM */

#ifdef WSM
    /*
     * Don't need to wait because WSM sets SIGCLD handler
     */
#else /* WSM */
    while ((w = wait (&status)) != pid && (w != -1));

    /*
     * Currently the only purpose for setting the local variable "status"
     * here is as a convenience for setting break points or querying the
     * value of "status" when using a debugger.
     * Otherwise the value of "status" is unused beyond this point.
     */
    if (w == -1)
    {
	status = -1;
    }
#endif /* WSM */

#ifndef WSM
    signal (SIGINT, intStat);
    signal (SIGQUIT, quitStat);
    signal (SIGCHLD, chldStat);
#endif /* WSM */

    /*
     * Restore original DISPLAY environment variable value
     * so a restart will start on the same screen
     */

    if(wmGD.pActiveSD->displayString &&
       wmGD.displayString)
    {
	putenv(wmGD.displayString);
    }


    return (True);


} /* END OF FUNCTION F_Exec */



/*************************************<->*************************************
 *
 *  F_Quit_Mwm (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for terminating the window
 *  manager.
 *
 *************************************<->***********************************/

Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event)
{
    if (wmGD.showFeedback & WM_SHOW_FB_QUIT)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
    }
    else
    {
	Do_Quit_Mwm(False);
    }
    
    return (False);

} /* END OF FUNCTION F_Quit_Mwm */



/*************************************<->*************************************
 *
 *  Do_Quit_Mwm (diedOnRestart)
 *
 *
 *  Description:
 *  -----------
 *  Callback to do the f.quit_mwm function.
 *
 *************************************<->***********************************/

void Do_Quit_Mwm (Boolean diedOnRestart)
{
    int scr;
    ClientListEntry *pNextEntry;


    /*
     * Close the X connection to get all the X resources cleaned up.
     * !!! maybe windows should be reparented / rebordered  before closing? !!!
     * !!! clean up the _MOTIF_WM_INFO property on the root window !!!
     */


    if (DISPLAY)
    {
        XSetInputFocus(DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
	for (scr = 0; scr < wmGD.numScreens; scr++)
	{
	    if (wmGD.Screens[scr].managed)
	    {
#ifdef WSM
		SaveResources(&wmGD.Screens[scr]);
#endif /* WSM */
		pNextEntry = wmGD.Screens[scr].lastClient;
		while (pNextEntry)
		{
		    if (pNextEntry->type == NORMAL_STATE)
		    {
			if (!(pNextEntry->pCD->clientFlags & 
			      CLIENT_WM_CLIENTS))
			{
			    ReBorderClient (pNextEntry->pCD, diedOnRestart);
			}
		    }
		    pNextEntry = pNextEntry->prevSibling;
		}
#if defined(PANELIST)
	        UnParentControls (&wmGD.Screens[scr], False);
#endif /* PANELIST */

#ifndef WSM
		XDeleteProperty(DISPLAY, wmGD.Screens[scr].rootWindow,
				wmGD.xa_MWM_INFO);
#endif /* WSM */
	    }
	}
#ifdef WSM
	/* shut down the messaging connection */
	dtCloseIPC();
#endif /* WSM */
	ResignFromSM();
        XSync (DISPLAY, False);
        XCloseDisplay (DISPLAY);
    }
    
    if(diedOnRestart)
    {
	exit (WM_ERROR_EXIT_VALUE);
    }
    else
    {
	exit (0);
    }

} /* END OF FUNCTION Do_Quit_Mwm */


/*************************************<->*************************************
 *
 *  ReBorderClient (pCD, reMapClient)
 *
 *
 *  Description:
 *  -----------
 *  Restores X border for client window and reparents the
 *  window back to the root.
 *
 *
 *  Inputs:
 *  -------
 *  pCD = pointer to the client data for the window to be re-bordered.
 *
 *************************************<->***********************************/

void ReBorderClient (ClientData *pCD, Boolean reMapClient)
{
    int x, y;
    int xoff, yoff;
    XWindowChanges windowChanges;

    while (pCD)
    {
        if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED) &&
	    (!(reMapClient)))
        {
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
#ifdef WSM
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->pWsList->iconX, 
		pCD->pWsList->iconY);
#else /* WSM */
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->iconX, pCD->iconY);
#endif /* WSM */
        }

	if (!(reMapClient))
	{
	    if (pCD->maxConfig)
	    {
		x = pCD->maxX;
		y = pCD->maxY;
	    }
	    else
	    {
		if(wmGD.positionIsFrame)
		{
		    CalculateGravityOffset (pCD, &xoff, &yoff);
		    x = pCD->clientX - xoff;
		    y = pCD->clientY - yoff;
		}
		else
		{
		    x = pCD->clientX;
		    y = pCD->clientY;
		}
	    }
	    XUnmapWindow(DISPLAY, pCD->clientFrameWin);
	    XReparentWindow (DISPLAY, pCD->client, 
			     ROOT_FOR_CLIENT(pCD), x, y);
	}
	else
	{
	    XMapWindow(wmGD.display, pCD->client);
	}

	if (pCD->transientChildren)
	{
	    ReBorderClient (pCD->transientChildren, reMapClient);
	}

	if (!(reMapClient))
	{
	    /*
	     * restore X border
	     */
	    windowChanges.x = x;
	    windowChanges.y = y;
	    windowChanges.border_width = pCD->xBorderWidth;
	    XConfigureWindow (DISPLAY, pCD->client, 
			      CWBorderWidth | CWX | CWY, &windowChanges);
	}

	if (pCD->transientLeader)
	{
	    pCD = pCD->transientSiblings;
	}
	else
	{
	    pCD = NULL;
	}
    }

} /* END OF FUNCTION ReBorderClient */



/*************************************<->*************************************
 *
 *  F_Focus_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  focus to a particular client window.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) focus flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event)
{
    long focusFlags = (long)args;


    if (pCD && (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
    {
	Do_Focus_Key (pCD, GetFunctionTimestamp ((XButtonEvent *)event),
	    (focusFlags | ALWAYS_SET_FOCUS));
    }

    return (True);

} /* END OF FUNCTION F_Focus_Key */


/*************************************<->*************************************
 *
 *  FindSomeReasonableClient
 *
 *  Description:
 *  -----------
 *  Find a client, any client to set the focus to, return client or NULL.
 *  This code is ripped off from AutoResetKeyFocus(). 
 *  
 *************************************<->***********************************/

static Window FindSomeReasonableClient(void)
{
   ClientData *pcdNoFocus=NULL;

    ClientListEntry *pNextEntry;
    ClientData *pCD;
    ClientData *pcdLastFocus = (ClientData *) NULL;
    ClientData *pcdFocus;
    Window focusWindow = (Window) NULL;

    /*
     * Scan through the list of clients to find a window to get the focus.
     */

    pNextEntry = ACTIVE_PSD->clientList;

    while (pNextEntry)
    {
	pCD = pNextEntry->pCD;
	if (!wmGD.systemModalActive ||
	    (wmGD.systemModalClient == pCD))
	{
	    if ((pNextEntry->type != MINIMIZED_STATE) &&
	        (pCD->clientState != MINIMIZED_STATE) &&
#ifdef WSM
                (ClientInWorkspace (ACTIVE_WS, pCD)) &&
#endif /* WSM */
	        (pCD != pcdNoFocus))
	    {
	        if (pCD->transientChildren)
	        {
		    pcdFocus = FindLastTransientTreeFocus (pCD, pcdNoFocus);
	        }
	        else
	        {
		    pcdFocus = pCD;
	        }
	        if (pcdFocus &&
		    ((pcdLastFocus == NULL) ||
		     (pcdFocus->focusPriority > pcdLastFocus->focusPriority)))
	        {
		    pcdLastFocus = pcdFocus;
	        }
	    }
	}
	pNextEntry = pNextEntry->nextSibling;
    }

    /*
     * Set the focus window if one is found
     */

#ifdef WSM
    if (pcdLastFocus && 
	ClientInWorkspace (ACTIVE_WS, pcdLastFocus))
#else /* WSM */
    if (pcdLastFocus)
#endif /* WSM */
      focusWindow = pcdLastFocus->client;

    /*
     * If a client window could not be found, then just put focus
     * on any icon.
     */

    if (focusWindow == (Window) NULL)
    {
	pNextEntry = ACTIVE_PSD->clientList;

	while (pNextEntry)
	{
	    pCD = pNextEntry->pCD;

#ifdef WSM
          if (ClientInWorkspace (ACTIVE_WS, pCD))
	  {
#endif /* WSM */
	    if ((pNextEntry->type == MINIMIZED_STATE) ||
	        (pCD->clientState == MINIMIZED_STATE))
	    {
		focusWindow = ICON_FRAME_WIN(pCD);
		break;
	    }
#ifdef WSM
	  }
#endif /* WSM */
	    pNextEntry = pNextEntry->nextSibling;
	}
    }

    return (focusWindow);

} /* END OF FUNCTION FindSomeReasonableClient */




/*************************************<->*************************************
 *
 *  Do_Focus_Key (pCD, focusTime, flags)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to set the focus to a window.  The focus indication
 *  is not changed until the FocusIn event is received.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data
 *
 *  focusTime = focus change time
 *
 *  flags = wm focus change flags
 *
 *************************************<->***********************************/

void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags)
{
    ClientData *pcdFocus;
    Window focusWindow;


    /* Clear the replay flag */
    wmGD.replayEnterEvent = False;

    pcdFocus = pCD;
#ifdef WSM
    /* 
     * Make sure the client is in the current workspace
     */
    if ((pCD) &&
	(ClientInWorkspace (ACTIVE_WS, pCD)))
#else /* WSM */
    if (pCD)
#endif /* WSM */
    {
	if (pCD->clientState == MINIMIZED_STATE)
	{
	    focusWindow = ICON_FRAME_WIN(pCD);
	}
	else if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
	{
	    /*
	     * Set the keyboard focus to the indicated client window.
	     * If the window has an application modal subordinate then
	     * set the input focus to that window if the focus isn't
	     * already owned by a subordinate.
	     */

	    if (IS_APP_MODALIZED(pCD))
	    {
		ClientData *pcdFocusLeader,*currFocusLeader;

		/*
		 * Handle case where a modal window exists when Mwm starts up.
		 * wmGD.keyboardFocus is NULL, give focus to the modal dialog.
		 */

	        if (wmGD.keyboardFocus)
		{
		    currFocusLeader = wmGD.keyboardFocus->transientLeader;
		}
		else
		{
		    currFocusLeader = (ClientData *) NULL;
		}

		/*
		 * Find focus leader for pCD
		 */

		pcdFocusLeader = pCD;
		while (pcdFocusLeader->transientLeader &&
		       (pcdFocusLeader != currFocusLeader))
		{
		    pcdFocusLeader = pcdFocusLeader->transientLeader;
		}

		if (pcdFocusLeader == currFocusLeader)
		{
		    pcdFocus = wmGD.keyboardFocus;
		    flags = 0;
		}
		else
		{
		    pcdFocus = FindTransientFocus (pcdFocusLeader);
		}
	    }

	    /*
	     * !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  
	     * We must look at why FindTransientFocus is
	     * returning a NULL pcd.  The old code simply set
	     *	focusWindow = pcdFocus->client;
	     * !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  !!!  
	     *
	     * 11/26/96 rswiston - In tracking down CDExc22816, we
	     * discovered that pCD could get tricked into thinking
	     * it had modal transients when in fact all its transients
	     * had been withdrawn (fixed in WithdrawTransientChildren()).
	     * As a result, FindTransientFocus() returns wmGD.keyboardFocus;
	     * if nobody has the focus, FindTransientFocus() returns NULL.
	     */
	    if (pcdFocus)
	    {
		focusWindow = pcdFocus->client;
	    }
	    else
	    {
		focusWindow = (wmGD.keyboardFocus) ?
		    wmGD.keyboardFocus->client : ACTIVE_PSD->wmWorkspaceWin;
	    }
	}
	else
	{
	    /*
	     * If the focus policy is "pointer" don't set the focus to a
	     * window if it has an application modal subordinate.
	     */

	    if (IS_APP_MODALIZED(pCD))
	    {
		pcdFocus = NULL;
		focusWindow = ACTIVE_PSD->wmWorkspaceWin;

                /* Replay this later when the modal window is removed. */
                wmGD.replayEnterEvent = True;
	    }
	    else
	    {
		focusWindow = pcdFocus->client;
	    }
	}
    }
    else
    {
	/*
	 * Set up the default (non client specific) keyboard input focus.
	 */

	if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	{
	    focusWindow = PointerRoot;
	}
	else
	{
	    /*
	     * The WORKSPACE_IF_NULL flag is used to prevent client
	     * windows from flashing when deiconifying a client.
	     */

	    if (WORKSPACE_IF_NULL & flags)
	    {
	    	focusWindow = ACTIVE_PSD->wmWorkspaceWin;
	    }
	    else
	    {
		/* find some reasonable client so that focus is not lost */

		focusWindow = FindSomeReasonableClient();
		if (focusWindow == (Window)NULL)
		{
		    focusWindow = ACTIVE_PSD->wmWorkspaceWin;
		}
	    }
	}
    }

    if ((pcdFocus != wmGD.keyboardFocus) || (flags & ALWAYS_SET_FOCUS))
    {
        if (pcdFocus)
	{
	    /*
	     * Set the focus and/or send a take focus client message.  This
	     * is not done if a client area button press was done to set
	     * set the focus and the window is a globally active input
	     * style window (See ICCCM).
	     */

	    if ( (flags & CLIENT_AREA_FOCUS)                           &&
		 (pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS)    &&
		! pcdFocus->inputFocusModel                            &&
		 (pcdFocus == pCD)                                     &&
		 (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
                !(pcdFocus->clientState == MINIMIZED_STATE)
	       )
	    {
	      /*
	       * We get here if:
	       * 1. User clicked in the client area   AND
	       * 2. Input model is Globally Active    AND
	       * 3. Keyboard focus policy is explicit
	       */

	      /* this window has WM_TAKE_FOCUS set and InputField false. */
	      /* just send a message.                                    */
	      SendClientMsg (pcdFocus->client, 
			     (long) wmGD.xa_WM_PROTOCOLS,
			     (long) wmGD.xa_WM_TAKE_FOCUS, 
			     focusTime, NULL, 0);
	    }
	    else
	    {
	        if ((pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS) &&
                    !(pcdFocus->clientState == MINIMIZED_STATE))
	        {
		  /*
		   * Locally Active Input Model - Send a take focus message to the client.
		   */

		  SendClientMsg (pcdFocus->client, 
				 (long) wmGD.xa_WM_PROTOCOLS,
				 (long) wmGD.xa_WM_TAKE_FOCUS, 
				 focusTime, NULL, 0);
	        }

		/*
		 * Don't set the input focus if the window has input_field set
		 * to False or has expressed an interest in WM_TAKE_FOCUS
		 * (ie. 'No Input', 'Globally Active', or 'Locally Active'),
		 * and the user click in the client area.  If the user clicks
		 * on the titlebar or traverses to this window via f.next_key,
		 * set the focus so that the user can access the window menu
		 * and accelerators.
		 */

		if ( wmGD.enforceKeyFocus       ||  /* res - default == True. */
		     (flags & ALWAYS_SET_FOCUS) ||
		    !(flags & CLIENT_AREA_FOCUS)||  /* clicked on frame? */
		     pcdFocus->inputFocusModel  ||  /* Pass.|Glob. Active */
		     (pcdFocus->clientState == MINIMIZED_STATE)
		   )
		{
		  if ( !(flags & CLIENT_AREA_FOCUS) &&
		       !pcdFocus->inputFocusModel &&
                       !(pcdFocus->clientState == MINIMIZED_STATE))
		  {
		   /* the window doesn't want the focus - set it to the frame */
		   /* user clicked on the frame but we don't want the focus */
		   /* set it to the client's frame */
		   XSetInputFocus (DISPLAY, pcdFocus->clientBaseWin,
				RevertToPointerRoot, CurrentTime);
		  }
#ifndef FIX_1350
                  else if ( !(flags & CLIENT_AREA_FOCUS)                   &&
		       !(pcdFocus->protocolFlags & PROTOCOL_WM_TAKE_FOCUS) &&
		        pcdFocus->inputFocusModel
		     )
		  {
		    XSetInputFocus (DISPLAY, focusWindow,
				    RevertToPointerRoot, CurrentTime);
		  }
#endif
                  else
		  {
		    XSetInputFocus (DISPLAY, focusWindow,
				      RevertToParent, CurrentTime);
		  }
		}
		else
		{
		  /*
		   * We've decided that the window shouldn't get the focus,
		   * so don't change the focus.
		   */
		  pcdFocus = wmGD.nextKeyboardFocus;
		}
	    }
	}
	else
	{
	    XSetInputFocus (DISPLAY, focusWindow, RevertToPointerRoot,
					focusTime);
	}

	wmGD.nextKeyboardFocus = pcdFocus;
    }


} /* END OF FUNCTION Do_Focus_Key */


#ifdef WSM

/***********************<->*************************************
 *
 *  F_Goto_Workspace (args, pCD, event)
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for switching
 *  to another workspace by name.
 *
 *  Inputs:
 *  ------
 *  args = action function and arguments
 *
 *  pCD = pointer to the ClientData
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *  Outputs:
 *  -------
 *  Always False
 *
 *  Comments:
 *  -------
 ******************************<->***********************************/
Boolean
F_Goto_Workspace (String args, ClientData *pCD, XEvent *event)
{
    WmScreenData *pSD = ACTIVE_PSD;
    int iwsx;
    XmString xms;

    /* 
     * Compare argument against both resource name 
     * and workspace title, take the first match.
     */
    xms = XmStringCreate (args, XmFONTLIST_DEFAULT_TAG);
    for (iwsx = 0; iwsx < pSD->numWorkspaces; iwsx++)
    {
	if (!strcmp(pSD->pWS[iwsx].name, args) ||
	    XmStringCompare (xms, pSD->pWS[iwsx].title))
	{
	    break;
	}
    }

    XmStringFree (xms);

    /* check bounds */
    if (iwsx >= pSD->numWorkspaces)
    {
	Warning (((char *)GETMESSAGE(26, 4, 
		"Invalid workspace name specified for f.goto_workspace")));
    }
    else
    {
	ChangeToWorkspace (&pSD->pWS[iwsx]);

    }

    return (False);
}  /* END OF FUNCTION F_Goto_Workspace */

#endif /* WSM */

#ifdef WSM

/******************************<->*************************************
 *
 *  Boolean F_Help (String args, ClientData *pCD, XEvent *event)
 *
 *  Description:
 *  -----------
 *  Invoke help on the workspace manager
 *
 *  Inputs:
 *  ------
 *  args - incoming values
 *  pCD  - associated client data structure
 *  event - what triggered this call
 * 
 *  Outputs:
 *  -------
 *  Return - True if the call occurs; false otherwise.
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
Boolean
F_Help (String args, ClientData *pCD, XEvent *event)
{
#ifdef PANELIST
    Boolean rval;

    rval = WmDtHelp(args);
    return (rval);
#endif /* PANELIST */    
    

}  /* END OF FUNCTION F_Help */


/******************************<->*************************************
 *
 *  Boolean F_Help_Mode (String args, ClientData *pCD, XEvent *event)
 *
 *  Description:
 *  -----------
 *  Invoke item help on the frontpanel
 *
 *  Inputs:
 *  ------
 *  args - NULL
 *  pCD  - associated client data structure ??
 *  event - what triggered this call
 * 
 *  Outputs:
 *  -------
 *  Return - True if the call occurs; false otherwise.
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
Boolean
F_Help_Mode (String args, ClientData *pCD, XEvent *event)
{
#ifdef PANELIST
    /*
     * Help mode event processing interferes
     * with slide up windows. Don't continue
     * if windows are sliding.
     */
    if (wmGD.iSlideUpsInProgress == 0)
    {
	(void) WmDtHelpMode();
    }
    return (False);
#endif /* PANELIST */    

}  /* END OF FUNCTION F_Help_Mode */

#endif /* WSM */

/******************************<->*************************************
 *
 *  F_Next_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  input focus to the next window in the set of managed windows.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event)
{
#ifdef ROOT_ICON_MENU
    Boolean focused = False;
#endif /*  ROOT_ICON_MENU */
    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
#ifdef ROOT_ICON_MENU
	focused = 
#endif /*  ROOT_ICON_MENU */
	FocusNextWindow ((unsigned long)args,
			 GetFunctionTimestamp ((XButtonEvent *)event));
#ifdef ROOT_ICON_MENU
        if (focused && wmGD.iconClick &&
            event && event->type == KeyPress &&
            wmGD.nextKeyboardFocus &&
            wmGD.nextKeyboardFocus->clientState == MINIMIZED_STATE &&
            !P_ICON_BOX(wmGD.nextKeyboardFocus))
        {
            /*
             * Post system menu from the icon
             */
            F_Post_SMenu (args, wmGD.nextKeyboardFocus, event);
            return (False);
        }
#endif /*  ROOT_ICON_MENU */
    }

    return (True);

} /* END OF FUNCTION F_Next_Key */



/*************************************<->*************************************
 *
 *  F_Prev_Cmap (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler installing the previous
 *  colormap in the list of client window colormaps.
 *
 *************************************<->***********************************/

Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event)
{
    if (pCD == NULL)
    {
	pCD = ACTIVE_PSD->colormapFocus;
    }

    if (pCD && (pCD->clientCmapCount > 0) &&
        ((pCD->clientState == NORMAL_STATE) ||
	 (pCD->clientState == MAXIMIZED_STATE)))
    {
	if (--(pCD->clientCmapIndex) < 0)
	{
	    pCD->clientCmapIndex = pCD->clientCmapCount - 1;
	}
	pCD->clientColormap = pCD->clientCmapList[pCD->clientCmapIndex];
	if (ACTIVE_PSD->colormapFocus == pCD)
	{
#ifndef OLD_COLORMAP /* colormap */
	    /*
	     * We just re-ordered the colormaps list,
	     * so we need to re-run the whole thing.
	     */
	    pCD->clientCmapFlagsInitialized = 0;
	    ProcessColormapList (ACTIVE_PSD, pCD);
#else /* OSF original */
	    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
#endif
	}
    }

    return (True);

} /* END OF FUNCTION F_Prev_Cmap */



/*************************************<->*************************************
 *
 *  F_Prev_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for setting the keyboard
 *  input focus to the previous window in the set of managed windows.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event)
{
#ifdef ROOT_ICON_MENU
    Boolean focused = False;
#endif /*  ROOT_ICON_MENU */
    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
#ifdef ROOT_ICON_MENU
	focused = 
#endif /*  ROOT_ICON_MENU */
	FocusPrevWindow ((unsigned long)args,
			    GetFunctionTimestamp ((XButtonEvent *)event));
#ifdef ROOT_ICON_MENU
        if (focused && wmGD.iconClick &&
            event && event->type == KeyPress &&
            wmGD.nextKeyboardFocus &&
            wmGD.nextKeyboardFocus->clientState == MINIMIZED_STATE &&
            !P_ICON_BOX(wmGD.nextKeyboardFocus))
        {
            /*
             * Post system menu from the icon
             */
            F_Post_SMenu (args, wmGD.nextKeyboardFocus, event);
            return (False);
        }
#endif /*  ROOT_ICON_MENU */

    }

    return (True);

} /* END OF FUNCTION F_Prev_Key */

#ifdef PANELIST

/***********************<->*************************************
 *
 *  F_Post_FpMenu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting
 *  the Front Panel window menu.
 *
 *  Inputs:
 *  ------
 *  args = arguments (none)
 *
 *  pCD = pointer to the FP ClientData
 *
 *  event = X button press that invoked the function
 *
 ******************************<->***********************************/

Boolean
F_Post_FpMenu (String args, ClientData *pCD, XEvent *event)
{
    static MenuSpec *fpMenuSpec = (MenuSpec *)NULL;

    if (event->type != ButtonPress)
	return False;

    if (!fpMenuSpec)
    {
	WmScreenData *pSD = (pCD) ? PSD_FOR_CLIENT(pCD) : ACTIVE_PSD;
	MenuSpec *oldSpec;
	Widget tmpWidget;
	char *newMenuName;

	newMenuName = pCD ? pCD->systemMenu : "DtPanelMenu";

	for (oldSpec = pSD->menuSpecs;
	     oldSpec != (MenuSpec *)NULL;
	     oldSpec = oldSpec->nextMenuSpec)
	{
	    if (oldSpec->name && (strcmp(oldSpec->name, newMenuName) == 0))
		break;
	}
	if (!oldSpec)
	    return False;

	fpMenuSpec = DuplicateMenuSpec(oldSpec);

	/*
	 * TEMPORARILY modify pSD so the new menu will be
	 * created on DISPLAY1 instead of DISPLAY.
	 */
	fpMenuSpec->nextMenuSpec = pSD->menuSpecs;
	pSD->menuSpecs = fpMenuSpec;
	tmpWidget = pSD->screenTopLevelW;
	pSD->screenTopLevelW = pSD->screenTopLevelW1;

	(void)MAKE_MENU (pSD, pCD, newMenuName,
			 F_CONTEXT_NORMAL, F_CONTEXT_NORMAL,
			 (MenuItem *) NULL, FALSE);

	/* Restore pSD */
	pSD->screenTopLevelW = tmpWidget;
	pSD->menuSpecs = fpMenuSpec->nextMenuSpec;
    }

    PostMenu (fpMenuSpec, pCD, event->xbutton.x_root, event->xbutton.y_root,
	      event->xbutton.button, F_CONTEXT_NORMAL, POST_AT_XY, event);

    _XmGetMenuState(XtParent(fpMenuSpec->menuWidget))
	->MS_LastManagedMenuTime = event->xbutton.time;

    return False;
}


/***********************<->*************************************
 *
 *  F_Push_Recall (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for invoking/topping
 *  push_recall clients.
 *
 *
 *  Inputs:
 *  ------
 *  args = arguments
 *
 *  pCD = pointer to the ClientData 
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *  Comments:
 *  -------
 ******************************<->***********************************/

Boolean
F_Push_Recall (String args, ClientData *pCD, XEvent *event)
{
    WmPushRecallArg *pPRP;
    WmScreenData *pSD;
    WmFpPushRecallClientData *pPRCD;

    pPRP = (WmPushRecallArg *) args;
    pSD = (pCD) ? PSD_FOR_CLIENT(pCD) : ACTIVE_PSD;

    if (pPRP->ixReg  < pSD->numPushRecallClients)
    {
	/* get slot for this client */
	pPRCD = &(pSD->pPRCD[pPRP->ixReg]);

	/*
	 * If the client is already running, then top it in this workspace,
	 * else invoke the function to start it.
	 */
	if (pPRCD->pCD)
	{
	    /* Client is managed already. */ 
	    if (!(ClientInWorkspace (pSD->pActiveWS, pPRCD->pCD)))
	    {
		WorkspaceID *wsRemoveList;
		int sizeRemoveList;

		/* 
		 * Move client to current workspace 
		 */
	        wsRemoveList = GetListOfOccupiedWorkspaces (pPRCD->pCD, 
					&sizeRemoveList);
		RemoveClientFromWorkspaces (pPRCD->pCD, wsRemoveList,
                                        sizeRemoveList);
		XtFree ((char *)wsRemoveList);
		AddClientToWorkspaces (pPRCD->pCD, &(pSD->pActiveWS->id), 1);
		SetClientWsIndex(pPRCD->pCD);
		SetClientState(pPRCD->pCD, 
	            pPRCD->pCD->clientState & ~UNSEEN_STATE, CurrentTime);
	    }

	    /* Make this client visible */
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
	    F_Normalize_And_Raise (NULL, pPRCD->pCD, event);
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
	}
	else 
	{
	    struct timeval tvNow;
	    struct timezone tz;
	    Boolean bWaiting = False;

	    if (pPRCD->tvTimeout.tv_sec != 0)
	    {
		gettimeofday (&tvNow, &tz);

		if ((pPRCD->tvTimeout.tv_sec > tvNow.tv_sec) ||
		    ((pPRCD->tvTimeout.tv_sec == tvNow.tv_sec) &&
		     (pPRCD->tvTimeout.tv_usec > tvNow.tv_usec)))
		{
		    /* still waiting for client to start */
		    bWaiting = True;
		}
	    }

	    if (!bWaiting)
	    {
		long clientTimeout;
		Arg al[5];
		int ac;
		WmPanelistObject  pPanelist;

		pPanelist = (WmPanelistObject) pSD->wPanelist;

		/* invoke the function to start the client */
		pPRP->wmFunc ( pPRP->pArgs, pCD, event);

		if (pPanelist && panel.busy_light_data)
		{
		    /* set timeout value */
		    ac = 0;
		    XtSetArg (al[ac], 
			XmNclientTimeoutInterval, &clientTimeout);	ac++; 
		    XtGetValues (panel.busy_light_data->icon, (ArgList)al, ac);
		}

		/*
		 * ClientTimeout is in milliseconds, timeval values
		 * are in seconds and microseconds.
		 */
		gettimeofday (&(pPRCD->tvTimeout), &tz);

		pPRCD->tvTimeout.tv_sec += clientTimeout / 1000;
		pPRCD->tvTimeout.tv_usec += 
		    (clientTimeout % 1000) * 1000;

		pPRCD->tvTimeout.tv_sec += pPRCD->tvTimeout.tv_usec / 1000000;
		pPRCD->tvTimeout.tv_usec %= 1000000;
	    }
	}
    }

    return (True);

} /* END OF FUNCTION F_Push_Recall */
#endif /* PANELIST */


/*************************************<->*************************************
 *
 *  F_Pass_Key (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a function stub for the f.pass_key window manager function.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *************************************<->***********************************/

Boolean F_Pass_Key (args, pCD, event)
    String args;
    ClientData *pCD;
    XEvent *event;

{
    if (wmGD.passKeysActive)
    {
	/*
	 * Get out of pass keys mode.
	 */

	wmGD.passKeysActive = False;
	wmGD.passKeysKeySpec = NULL;
    }
    else
    {
	/*
	 * Get into pass keys mode.
	 */

	wmGD.passKeysActive = True;
    }

    return (False);

} /* END OF FUNCTION F_Pass_Key */



/*************************************<->*************************************
 *
 *  F_Maximize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for maximizing a client
 *  window.
 *
 *************************************<->***********************************/

Boolean F_Maximize (String args, ClientData *pCD, XEvent *event)
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_MAXIMIZE))
    {
	SetClientStateWithEventMask (pCD, MAXIMIZED_STATE,
	    GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
    }

    return (False);

} /* END OF FUNCTION F_Maximize */



/*************************************<->*************************************
 *
 *  F_Menu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting a menu.
 *  This function can only be invoked by a key or button event.
 *   wmGD.menuUnpostKeySpec is assumed set appropriately; it will be set to
 *     NULL when the menu is unposted.
 *
 *************************************<->***********************************/

Boolean F_Menu (String args, ClientData *pCD, XEvent *event)
{
    MenuSpec    *menuSpec;
    Context      menuContext;
    unsigned int button;
    int          x;
    int		 y;
    long         flags = POST_AT_XY;
    WmScreenData *pSD;


    if (event && 
	((event->type == ButtonPress) || (event->type == ButtonRelease)))
    {
        button = event->xbutton.button;
	x = event->xbutton.x_root;
	y = event->xbutton.y_root;
        if (event->type == ButtonRelease)
	{
	    flags |= POST_TRAVERSAL_ON;
	}
#ifdef WSM
	/*
	 * Root menu, if posted with button press, then 
	 * set up to handle root menu click to make the menu
	 * sticky.
	 */
	else if (wmGD.rootButtonClick && (event->type == ButtonPress))
	{
	    if (wmGD.bReplayedButton)
	    {
		/* This button was replayed, it most likely dismissed
		   a previous sticky menu, don't post a menu here */
		return (False);
	    }
	    wmGD.checkHotspot = True;
	    wmGD.hotspotRectangle.x = x - wmGD.moveThreshold/2;
	    wmGD.hotspotRectangle.y = y - wmGD.moveThreshold/2;
	    wmGD.hotspotRectangle.width = wmGD.moveThreshold;
	    wmGD.hotspotRectangle.height = wmGD.moveThreshold;
	}
#endif /* WSM */
    }
    else if (event && 
	((event->type == KeyPress) || (event->type == KeyRelease)))
    {
        button = NoButton;
	x = event->xkey.x_root;
	y = event->xkey.y_root;
    }
    else
    {
	/*
	 * A button or key event must be used to post a menu using this 
	 * function.
	 */

	return (False);
    }

    if (pCD)
    {
	if (pCD->clientState == NORMAL_STATE)
	{
	    menuContext = F_CONTEXT_NORMAL;
	}
	else if (pCD->clientState == MAXIMIZED_STATE)
	{
	    menuContext = F_CONTEXT_MAXIMIZE;
	}
	else 
	{
	    menuContext = F_CONTEXT_ICON;
	}
	if (P_ICON_BOX(pCD) &&
            event->xany.window == ICON_FRAME_WIN(pCD))
	{
	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		menuContext = F_SUBCONTEXT_IB_IICON;
	    }
	    else
	    {
		menuContext = F_SUBCONTEXT_IB_WICON;
	    }
	}
    }
    else
    {
	menuContext = F_CONTEXT_ROOT;
    }


    /* We do not add this MenuSpec to wmGD.acceleratorMenuSpecs.
     * This should have been done in MakeWmFunctionResources().
     */

    pSD = (pCD) ? PSD_FOR_CLIENT(pCD) : ACTIVE_PSD;
    if ((menuSpec = MAKE_MENU (pSD, pCD, args, menuContext, 
			      menuContext, (MenuItem *) NULL, FALSE)) != NULL)
    {
        PostMenu (menuSpec, pCD, x, y, button, menuContext, flags, event);
    }

    return (False);

} /* END OF FUNCTION F_Menu */


/*************************************<->*************************************
 *
 *  F_Minimize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for minimizing a client
 *  window.
 *
 *************************************<->***********************************/

Boolean F_Minimize (String args, ClientData *pCD, XEvent *event)
{
    ClientData *pcdLeader;


    if (pCD)
    {
	/*
	 * If the window is a transient then minimize the entire transient
	 * tree including the transient leader.
	 */
	
	pcdLeader = (pCD->transientLeader) ?
					FindTransientTreeLeader (pCD) : pCD;
	if (pcdLeader->clientFunctions & MWM_FUNC_MINIMIZE)
	{
	    SetClientStateWithEventMask (pCD, MINIMIZED_STATE,
		GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
	}
    }

    return (False);

} /* END OF FUNCTION F_Minimize */



/*************************************<->*************************************
 *
 *  F_Move (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for moving a client window
 *  or icon.
 *
 *************************************<->***********************************/

Boolean F_Move (String args, ClientData *pCD, XEvent *event)
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_MOVE))
    {
	StartClientMove (pCD, event);
	HandleClientFrameMove (pCD, event);
    }

    return (False);

} /* END OF FUNCTION F_Move */



/*************************************<->*************************************
 *
 *  F_Next_Cmap (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler installing the next
 *  colormap in the list of client window colormaps.
 *
 *************************************<->***********************************/

Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event)
{
    if (pCD == NULL)
    {
	pCD = ACTIVE_PSD->colormapFocus;
    }

    if (pCD && (pCD->clientCmapCount > 0) &&
        ((pCD->clientState == NORMAL_STATE) ||
	 (pCD->clientState == MAXIMIZED_STATE)))
    {
	if (++(pCD->clientCmapIndex) >= pCD->clientCmapCount)
	{
	    pCD->clientCmapIndex = 0;
	}
	pCD->clientColormap = pCD->clientCmapList[pCD->clientCmapIndex];
	if (ACTIVE_PSD->colormapFocus == pCD)
	{
#ifndef OLD_COLORMAP /* colormap */
	    /*
	     * We just re-ordered the colormaps list,
	     * so we need to re-run the whole thing.
	     */
	    pCD->clientCmapFlagsInitialized = 0;
	    ProcessColormapList (ACTIVE_PSD, pCD);
#else /* OSF original */
	    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
#endif
	}
    }

    return (True);

} /* END OF FUNCTION F_Next_Cmap */



/*************************************<->*************************************
 *
 *  F_Nop (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for doing nothing.
 *
 *************************************<->***********************************/

Boolean F_Nop (String args, ClientData *pCD, XEvent *event)
{

    return (True);

} /* END OF FUNCTION F_Nop */



/*************************************<->*************************************
 *
 *  F_Normalize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state.
 *
 *************************************<->***********************************/

Boolean F_Normalize (String args, ClientData *pCD, XEvent *event)
{

    if (pCD)
    {
	SetClientStateWithEventMask (pCD, NORMAL_STATE,
	    GetFunctionTimestamp ((XButtonEvent *)event),
		GetEventInverseMask(event));
    }

    return (False);

} /* END OF FUNCTION F_Normalize */



/*************************************<->*************************************
 *
 *  F_Normalize_And_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state and raising it from and icon.
 *
 *************************************<->***********************************/

Boolean F_Normalize_And_Raise (String args, ClientData *pCD, XEvent *event)
{
#ifdef PANELIST
    WmScreenData 	*pSD;
    WmWorkspaceData 	*pWS;

    if (args)
    {
	if (pCD) 
	    pSD = PSD_FOR_CLIENT (pCD);
	else
	    pSD = ACTIVE_PSD;

	pWS =  pSD->pActiveWS;

	if (pSD->useIconBox && 
	    wmGD.useFrontPanel && 
	    pSD->iconBoxControl &&
	    (!strcmp(args, WmNiconBox)))
	{
	    /* 
	     * There's an icon box in the front panel and this is a 
	     * request to pop up the icon box.
	     */
	    IconBoxPopUp (pWS, True);
	    return (False);
	}
    }
#endif /* PANELIST */
    if (pCD)
    {
        if (pCD->clientState == MINIMIZED_STATE)
        {
            /* normalize window  */
            SetClientStateWithEventMask (pCD, NORMAL_STATE,
                          (Time)
                          (event
                           ? GetFunctionTimestamp ((XButtonEvent *)event)
                           : GetTimestamp ()),
			GetEventInverseMask(event));
        }
        else
        {
	    /* Make sure we are in NORMAL_STATE */
	    SetClientStateWithEventMask (pCD, NORMAL_STATE,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));

            /* Raise the window and set the keyboard focus to the window */
#ifdef WSM
	    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
            F_Raise (NULL, pCD, (XEvent *)NULL);
#ifdef WSM
	    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
	    if (wmGD.raiseKeyFocus)
	    {
		F_Focus_Key (NULL, pCD,
			     (event 
			      ? ((XEvent *)event)
			      : ((XEvent *)NULL)));
	    }
        }
	wmGD.clickData.clickPending = False;
	wmGD.clickData.doubleClickPending = False;
    }

    return (False);

} /* END OF FUNCTION F_Normalize_And_Raise */



/*************************************<->*************************************
 *
 *  F_Restore (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state.
 *
 *************************************<->***********************************/

Boolean F_Restore (String args, ClientData *pCD, XEvent *event)
{
    int newState;

    if (pCD)
    {
	/*
	 * If current state is MAXIMIZED state then just go to NORMAL state,
	 * otherwise (you are in MINIMIZED state) return to previous state.
	 */

	if (pCD->clientState == MAXIMIZED_STATE)
	{
	    SetClientStateWithEventMask (pCD, NORMAL_STATE,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));
	}
	else
	{
	    if (pCD->maxConfig)
	    {
		newState = MAXIMIZED_STATE;
	    }
	    else
	    {
		newState = NORMAL_STATE;
	    }

	    SetClientStateWithEventMask (pCD, newState,
			    GetFunctionTimestamp ((XButtonEvent *)event),
				GetEventInverseMask(event));
	}
    }

    return (False);

} /* END OF FUNCTION F_Restore */



/*************************************<->*************************************
 *
 *  F_Restore_And_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for putting a client window
 *  in the normal state and raising it from and icon.
 *
 *************************************<->***********************************/

Boolean F_Restore_And_Raise (String args, ClientData *pCD, XEvent *event)
{
    int newState;
    
    if (pCD)
    {
        if (pCD->clientState == MINIMIZED_STATE)
        {
            /* Restore window  */
	    if (pCD->maxConfig)
	    {
		newState = MAXIMIZED_STATE;
	    }
	    else
	    {
		newState = NORMAL_STATE;
	    }

            SetClientStateWithEventMask (pCD, newState,
                          (Time)
                          (event
                           ? GetFunctionTimestamp ((XButtonEvent *)event)
                           : GetTimestamp ()),
			GetEventInverseMask(event));
        }
        else
        {
	    /* Make sure we restore the window first */
	    F_Restore (NULL, pCD, event);

            /* Raise the window and set the keyboard focus to the window */
#ifdef WSM
	    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
            F_Raise (NULL, pCD, (XEvent *)NULL);
#ifdef WSM
	    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
	    if (wmGD.raiseKeyFocus)
	    {
		F_Focus_Key (NULL, pCD,
			     (event 
			      ? ((XEvent *)event)
			      : ((XEvent *)NULL)));
	    }
        }
	wmGD.clickData.clickPending = False;
	wmGD.clickData.doubleClickPending = False;
    }

    return (False);

} /* END OF FUNCTION F_Restore_And_Raise */



/*************************************<->*************************************
 *
 *  F_Pack_Icons (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for packing icons in the
 *  icon box or on the desktop.
 *
 *************************************<->***********************************/

Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event)
{
    
    IconBoxData *pIBD;

    if (ACTIVE_PSD->useIconBox)
    {
	pIBD = ACTIVE_WS->pIconBox;
	if (pCD)
	{
	    while (pCD != pIBD->pCD_iconBox)
	    {
		if (pIBD->pNextIconBox)
		{
		    pIBD = pIBD->pNextIconBox;
		}
		else
		{
		    pIBD = NULL;
		    break;
		}
	    }
	}
	if (pIBD)
	{
	    PackIconBox (pIBD, False, False, 0, 0);
	}
	else
	{
	   PackRootIcons ();
	}
    }
    else
    {
	PackRootIcons ();
    }

    return (True);


} /* END OF FUNCTION F_Pack_Icons */


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  F_Post_RMenu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting the
 *  root window menu.
 *  This function can only be invoked by a key event.
 *
 *************************************<->***********************************/

Boolean F_Post_RMenu (String args, ClientData *pCD, XEvent *event)
{
    MenuSpec    *rootMenu;
    unsigned int button = NoButton;
    int          x, y;
    long         flags = POST_AT_XY;
    Window       rwin, cwin;
    int          winx, winy;
    unsigned int mask;


    if ((event->type == KeyPress) || (event->type == KeyRelease))
      {

	/* Find the root menu spec */
	for (rootMenu = ACTIVE_PSD->menuSpecs;
	     rootMenu != (MenuSpec *) NULL;
	     rootMenu = rootMenu->nextMenuSpec)
	  {
	    if (strcmp(rootMenu->name, ACTIVE_PSD->rootMenu) == 0)
	      break;
	  }
    
	/* If we couldn't find the root menu, then do nothing. */
	if (rootMenu == (MenuSpec *) NULL)
	  return (False);

	else
    	  {
	    XQueryPointer(DISPLAY, ACTIVE_ROOT,
			  &rwin, &cwin, &x, &y, &winx, &winy, &mask);

	    PostMenu (rootMenu, NULL, x, y, NoButton, F_CONTEXT_ROOT,
		      flags, event);
	  }
      }

    return (False);

} /* END OF FUNCTION F_Post_RMenu */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  F_Post_SMenu (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for posting the system menu
 *  for the specified client.
 *  This function can only be invoked by a key or button event.
 *  wmGD.menuUnpostKeySpec is assumed set appropriately; it will be set to
 *    NULL when the menu is unposted.
 *
 *************************************<->***********************************/

Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event)
{
    Context menuContext;


    /*
     * An event must be used to post the system menu using this function.
     */

    if (event && pCD && pCD->systemMenuSpec)
    {
        /*
	 * Determine whether the keyboard is posting the menu and post
	 * the menu at an appropriate place.
         */

	if (pCD->clientState == NORMAL_STATE)
	{
	    menuContext = F_CONTEXT_NORMAL;
	}
	else if (pCD->clientState == MAXIMIZED_STATE)
	{
	    menuContext = F_CONTEXT_MAXIMIZE;
	}
	else 
	{
	    menuContext = F_CONTEXT_ICON;
	}
	if (P_ICON_BOX(pCD) &&
            event->xany.window == ICON_FRAME_WIN(pCD))
	{
	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		menuContext = F_SUBCONTEXT_IB_IICON;
	    }
	    else
	    {
		menuContext = F_SUBCONTEXT_IB_WICON;
	    }
	}

	if ((event->type == KeyPress) || (event->type == KeyRelease))
	{
	    /*
	     * Set up for "sticky" menu processing if specified.
	     */

	    if (pCD->clientState == MINIMIZED_STATE ||
		menuContext == (F_SUBCONTEXT_IB_IICON | F_SUBCONTEXT_IB_WICON))
	    {
		if (wmGD.iconClick)
		{
		    wmGD.checkHotspot = True;
		}
	    }
	    else if (wmGD.systemButtonClick && (pCD->decor & MWM_DECOR_MENU))
	    {
		wmGD.checkHotspot = True;
	    }

	    PostMenu (pCD->systemMenuSpec, pCD, 0, 0, NoButton, menuContext,
		      0, event);
	}
	else if (event->type == ButtonPress)
	{
#ifdef WSM
	    /*
	     * Root menu, if posted with button press, then 
	     * set up to handle root menu click to make the menu
	     * sticky.
	     */
	    if (wmGD.rootButtonClick && (!wmGD.checkHotspot))
	    {
		wmGD.checkHotspot = True;
		wmGD.hotspotRectangle.x = 
			    event->xbutton.x_root - wmGD.moveThreshold/2;
		wmGD.hotspotRectangle.y = 
			    event->xbutton.y_root - wmGD.moveThreshold/2;
		wmGD.hotspotRectangle.width = wmGD.moveThreshold;
		wmGD.hotspotRectangle.height = wmGD.moveThreshold;
	    }
#endif /* WSM */
	    PostMenu (pCD->systemMenuSpec, pCD, 
		event->xbutton.x_root, event->xbutton.y_root,
	  	event->xbutton.button, menuContext, POST_AT_XY, event);
	}
	else if (event->type == ButtonRelease)
	{
	    PostMenu (pCD->systemMenuSpec, pCD, 
		event->xbutton.x_root, event->xbutton.y_root,
	  	event->xbutton.button, menuContext,
		POST_AT_XY | POST_TRAVERSAL_ON, event);
	}
    }

    return (False);

} /* END OF FUNCTION F_PostSMenu */



/*************************************<->*************************************
 *
 *  F_Kill (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for terminating a client.
 *  Essentially the client connection is shut down.
 *
 *************************************<->***********************************/

Boolean F_Kill (String args, ClientData *pCD, XEvent *event)
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_CLOSE))
    {
	Boolean do_delete_window =
		pCD->protocolFlags & PROTOCOL_WM_DELETE_WINDOW;
	Boolean do_save_yourself =
		pCD->protocolFlags & PROTOCOL_WM_SAVE_YOURSELF;

#ifdef PANELIST
	if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)
	{
	    Widget 		wPanel;
	    
	    /*
	     * Get the widget for the subpanel
	     * (Should be only child of the shell!)
	     */
	    wPanel = WmPanelistWindowToSubpanel (DISPLAY1, pCD->client);
            if (wPanel)
	    {
		SlideSubpanelBackIn (pCD, wPanel);
	    }
	    return (False);
	}
        if (pCD->clientFlags & ICON_BOX)
	{
	    /*
	     * When the front panel is used with the icon box,
	     * "Close" hides the icon box into the front panel.
	     */
	    if ((wmGD.useFrontPanel) &&
		(pCD->pSD->iconBoxControl) &&
		(IconBoxShowing(pCD->pSD->pActiveWS)))
	    {
		IconBoxPopUp (pCD->pSD->pActiveWS, False);
	    }
	}
	else
#endif /* PANELIST */
	if (!do_delete_window && !do_save_yourself)
	{
	    XKillClient (DISPLAY, pCD->client);
	}
	else
	{
	    if (do_delete_window)
 	    {
		/*
		 * The client wants to be notified, not killed.
		 */

		SendClientMsg (pCD->client, (long) wmGD.xa_WM_PROTOCOLS,
		    (long) wmGD.xa_WM_DELETE_WINDOW, CurrentTime, NULL, 0);
	    }
#ifdef WSM
	    /*
	     * HP does not want to send a client message for both
	     * delete_window AND save_yourself.  The current OSF
	     * patch did just that.  This "else if" returns dtwm
	     * to the behavior of dt 2.01
	     */
            else if (do_save_yourself)
#else /* WSM */
	    if (do_save_yourself)
#endif /* WSM */
	    {
		/*
		 * Send a WM_SAVE_YOURSELF message and wait for a change to
		 * the WM_COMMAND property.
		 * !!! button and key input should be kept from the window !!!
		 */

		if (AddWmTimer (TIMER_QUIT, 
		    (unsigned long) wmGD.quitTimeout, pCD))
		{
		    SendClientMsg (pCD->client, (long) wmGD.xa_WM_PROTOCOLS,
			(long) wmGD.xa_WM_SAVE_YOURSELF, CurrentTime, NULL, 0);

		    pCD->clientFlags |= CLIENT_TERMINATING;
		}
		else
		{
		    XKillClient (DISPLAY, pCD->client);
		}
	    }
	}
    }

    return (False);

} /* END OF FUNCTION F_Kill */

#ifdef WSM

/*************************************<->*************************************
 *
 *  F_Marquee_Selection (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for selecting 
 *  non-window manager objects on the root window.
 *
 *************************************<->***********************************/

Boolean F_Marquee_Selection (String args, ClientData *pCD, XEvent *event)
{
    if (!pCD)
    {
	/*
	 * This function only valid in root context
	 */
	StartMarqueeSelect (ACTIVE_PSD, event);
	HandleMarqueeSelect (ACTIVE_PSD, event);
    }

    return (False);

} /* END OF FUNCTION F_Marquee_Selection */

/*************************************<->*************************************
 *
 *  RefreshByClearing (win)
 *
 *
 *  Description:
 *  -----------
 *  Recursively refresh this window and its children by doing
 *  XClearAreas
 *
 *************************************<->***********************************/
static void
RefreshByClearing (Window win)
{
    Status status;
    int i;
    Window root, parent;
    unsigned int nchildren;
    Window *winChildren;

    /* clear this window */
    XClearArea(DISPLAY, win, 0, 0, 0, 0, True);

    /* find any children and clear them, too */
    status = XQueryTree(DISPLAY, win, &root, &parent, &winChildren,
		    &nchildren);
    if (status != 0)
    {
	/* recurse for each child window */
	for (i=0; i<nchildren; ++i) 
        {
	    RefreshByClearing(winChildren[i]);
	}

	/* clean up */
	if (nchildren > 0) 
	    XFree((char *)winChildren);
    }
}
#endif /* WSM */


/*************************************<->*************************************
 *
 *  F_Refresh (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for causing all windows
 *  in the workspace to be redrawn.
 *
 *************************************<->***********************************/

Boolean F_Refresh (String args, ClientData *pCD, XEvent *event)
{
    Window win;

#ifdef WSM
    if (wmGD.refreshByClearing)
    {
	RefreshByClearing (ACTIVE_ROOT);
    }
    else
    {
#endif /* WSM */
			 /* default background_pixmap is None */
    win = XCreateWindow (DISPLAY,
			 ACTIVE_ROOT, 0, 0,
	                 (unsigned int) DisplayWidth (DISPLAY, 
			     ACTIVE_SCREEN),
	                 (unsigned int) DisplayHeight (DISPLAY, 
			     ACTIVE_SCREEN),
	                 0, 
                         0,
	                 InputOutput,
                         CopyFromParent,
	                 0, 
			 (XSetWindowAttributes *)NULL);   

    XMapWindow (DISPLAY, win);
    XDestroyWindow (DISPLAY, win);
#ifdef WSM 
    }
#endif /* WSM */
    XFlush (DISPLAY);

    return (True);

} /* END OF FUNCTION F_Refresh */



/*************************************<->*************************************
 *
 *  F_Resize (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for resizing a client window.
 *
 *************************************<->***********************************/

Boolean F_Resize (String args, ClientData *pCD, XEvent *event)
{
    if (pCD && (pCD->clientFunctions & MWM_FUNC_RESIZE) &&
	((pCD->clientState == NORMAL_STATE) ||
					(pCD->clientState == MAXIMIZED_STATE)))
    {
	StartClientResize (pCD, event);
	HandleClientFrameResize (pCD, event);
    }

    return (False);

} /* END OF FUNCTION F_Resize */



/*************************************<->*************************************
 *
 *  F_Restart (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for restarting the window
 *  manager.
 *
 *************************************<->***********************************/

Boolean F_Restart (String args, ClientData *pCD, XEvent *event)
{
#ifdef WSM
    if (args && *args && !strcmp (args, DTWM_REQP_NO_CONFIRM))
    {
	RestartWm (MWM_INFO_STARTUP_CUSTOM);
    }
    else
#endif /* WSM */
    if (wmGD.showFeedback & WM_SHOW_FB_RESTART)
    {
	ConfirmAction (ACTIVE_PSD, RESTART_ACTION);
    }
    else
    {
	RestartWm (MWM_INFO_STARTUP_CUSTOM);
    }
    return (False);

} /* END OF FUNCTION F_Restart */



/*************************************<->*************************************
 *
 *  Do_Restart (dummy)
 *
 *
 *  Description:
 *  -----------
 *  Callback function for restarting the window manager.
 *
 *************************************<->***********************************/

void Do_Restart (Boolean dummy)
{
    RestartWm (MWM_INFO_STARTUP_CUSTOM);

} /* END OF FUNCTION Do_Restart */



/*************************************<->*************************************
 *
 *  RestartWm (startupFlags)
 *
 *
 *  Description:
 *  -----------
 *  Actually restarts the window manager.
 *
 *
 *  Inputs:
 *  ------
 *  startupFlags = flags to be put into the Wm_INFO property for restart.
 *
 *************************************<->***********************************/

void RestartWm (long startupFlags)
{
    ClientListEntry *pNextEntry;
    int scr;


    for (scr=0; scr<wmGD.numScreens; scr++)
    {
	if(wmGD.Screens[scr].managed)
	{
	    
	    /*
	     * Set up the _MOTIF_WM_INFO property on the root window 
	     * to indicate a restart.
	     */
	    
	    SetMwmInfo (wmGD.Screens[scr].rootWindow, startupFlags, 0);
#ifdef WSM
	    SaveResources(&wmGD.Screens[scr]);
#endif /* WSM */
	    /*
	     * Unmap client windows and reparent them to the root window.
	     */
	    
	    pNextEntry = wmGD.Screens[scr].lastClient;
	    while (pNextEntry)
	    {
		if (pNextEntry->type == NORMAL_STATE)
		{
		    if (pNextEntry->pCD->clientFlags & CLIENT_WM_CLIENTS)
		    {
			if (pNextEntry->pCD->clientState != MINIMIZED_STATE)
			{
			    XUnmapWindow (DISPLAY, 
					  pNextEntry->pCD->clientFrameWin);
			}
		    }
		    else
		    {
			DeFrameClient (pNextEntry->pCD);
		    }
		}
		pNextEntry = pNextEntry->prevSibling;
	    }
#if defined(PANELIST)
	    UnParentControls (&wmGD.Screens[scr], True);
#endif /* PANELIST */
	}
	
    }
    
#ifdef WSM
    /* shut down the messaging connection */
    dtCloseIPC();
#endif /* WSM */
    ResignFromSM();

    /*
     * This fixes restart problem when going from explicit focus to
     * pointer focus.  Window under pointer was not getting focus indication
     * until pointer was moved to new window, or out of and into the
     * window.
     */

    XSetInputFocus (DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync (DISPLAY, False);

#ifdef WSM
    CLOSE_FILES_ON_EXEC();
    _DtEnvControl(DT_ENV_RESTORE_PRE_DT); 
#endif /* WSM */
    /*
     * Restart the window manager with the initial arguments plus
     * the restart settings.
     */

    execvp (*(wmGD.argv), wmGD.argv);

#ifdef WSM
    Warning (((char *)GETMESSAGE(26, 1, 
"The window manager restart failed. The window manager program could not \
be found or could not be executed.")));
#else /* WSM */
    Warning ("Cannot restart the window manager");
#endif /* WSM */
    Do_Quit_Mwm (True);



} /* END OF FUNCTION RestartWm */


/*************************************<->*************************************
 *
 *  DeFrameClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Unmaps a client window (and client icon window) and reparents the
 *  window back to the root.
 *
 *
 *  Inputs:
 *  -------
 *  pCD = pointer to the client data for the window to be de-framed.
 *
 *************************************<->***********************************/

void DeFrameClient (ClientData *pCD)
{
    int x, y;
    int xoff, yoff;
    XWindowChanges windowChanges;

    while (pCD)
    {
        if (pCD->clientState != MINIMIZED_STATE)
        {
	    XUnmapWindow (DISPLAY, pCD->clientFrameWin);
        }

        if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED))
        {
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
	    XRemoveFromSaveSet (DISPLAY, pCD->iconWindow);
#ifdef WSM
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->pWsList->iconX, 
		pCD->pWsList->iconY);
#else /* WSM */
	    XReparentWindow (DISPLAY, pCD->iconWindow, 
		ROOT_FOR_CLIENT(pCD), pCD->iconX, pCD->iconY);
#endif /* WSM */
        }

        if (pCD->maxConfig)
        {
	    x = pCD->maxX;
	    y = pCD->maxY;
        }
        else
        {
	    if(wmGD.positionIsFrame)
	    {
		CalculateGravityOffset (pCD, &xoff, &yoff);
		x = pCD->clientX - xoff;
		y = pCD->clientY - yoff;
	    }
	    else
	    {
		x = pCD->clientX;
		y = pCD->clientY;
	    }
        }

#ifndef UNMAP_ON_RESTART
	if (pCD->clientState == MINIMIZED_STATE)
	{
	    XUnmapWindow (DISPLAY, pCD->client);
	}
#else
	XUnmapWindow (DISPLAY, pCD->client);
#endif
	XRemoveFromSaveSet (DISPLAY, pCD->client);
        XReparentWindow (DISPLAY, pCD->client, 
	    ROOT_FOR_CLIENT(pCD), x, y);

	if (pCD->transientChildren)
	{
	    DeFrameClient (pCD->transientChildren);
	}

	/*
	 * restore X border
	 */
	windowChanges.x = x;
	windowChanges.y = y;
	windowChanges.border_width = pCD->xBorderWidth;
	XConfigureWindow (DISPLAY, pCD->client, CWBorderWidth | CWX | CWY,
			  &windowChanges);

	if (pCD->transientLeader)
	{
	    pCD = pCD->transientSiblings;
	}
	else
	{
	    pCD = NULL;
	}
    }

} /* END OF FUNCTION DeFrameClient */

#if defined(PANELIST)

/******************************<->*************************************
 *
 *  F_Toggle_Front_Panel (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for toggling the
 *  front panel off and on.
 ******************************<->***********************************/

Boolean
F_Toggle_Front_Panel (String args, ClientData *pCD, XEvent *event)
{

    WmPanelistObject  pPanelist;

    if (pCD)
    {
        pPanelist = (WmPanelistObject) pCD->pSD->wPanelist;
    }
    else
    {
        pPanelist = (WmPanelistObject) ACTIVE_PSD->wPanelist;
    }

    pCD = NULL;
    if (pPanelist)
    {
	(void) XFindContext (DISPLAY, XtWindow(O_Shell(pPanelist)),
			    wmGD.windowContextType, (caddr_t *)&pCD);
    }


    if (pCD)
    {
	if (pCD->clientState & MINIMIZED_STATE)
	{
	    SetClientState (pCD, NORMAL_STATE, 
		    GetFunctionTimestamp ((XButtonEvent *)event));
	}
	else
	{
	    SetClientState (pCD, MINIMIZED_STATE, 
		    GetFunctionTimestamp ((XButtonEvent *)event));
	}
    }

    return(True);
} /* END OF FUNCTION F_Toggle_Front_Panel */


/******************************<->*************************************
 *
 *  Boolean F_Version (String args, ClientData *pCD, XEvent *event)
 *
 *  Description:
 *  -----------
 *  Invoke the help on version dialogue.
 *
 *  Inputs:
 *  ------
 *  args - incoming values
 *  pCD  - associated client data structure
 *  event - what triggered this call
 * 
 *  Outputs:
 *  -------
 *  Return - True if the call occurs; false otherwise.
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
Boolean
F_Version (String args, ClientData *pCD, XEvent *event)
{

    WmPanelistObject  pPanelist;

    if (pCD)
    {
        pPanelist = (WmPanelistObject) pCD->pSD->wPanelist;
    }
    else
    {
        pPanelist = (WmPanelistObject) ACTIVE_PSD->wPanelist;
    }

    if (pPanelist)
    {
	WmDtHelpOnVersion (O_Shell (pPanelist));
    }

    return (True);

}  /* END OF FUNCTION F_Version */
#endif /* PANELIST */


/******************************<->*************************************
 *
 *  F_Send_Msg (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for sending a client
 *  message event to a client window.
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) message id
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 ******************************<->***********************************/

Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event)
{
    register int i;


    if (pCD && pCD->mwmMessagesCount)
    {
	/*
	 * A message id must be made "active" by being included in the
	 * _MWM_MESSAGES property before the associated message can be sent.
	 */

	for (i = 0; i < pCD->mwmMessagesCount; i++)
	{
	    if (pCD->mwmMessages[i] == (long)args)
	    {
		SendClientMsg (pCD->client, (long) wmGD.xa_MWM_MESSAGES, 
		    (long)args, CurrentTime, NULL, 0);
		return (True);
	    }
	}
    }

    return (True);

} /* END OF FUNCTION F_Send_Msg */



/*************************************<->*************************************
 *
 *  F_Separator (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a placeholder function; it should never be called.
 *
 *************************************<->***********************************/

Boolean F_Separator (String args, ClientData *pCD, XEvent *event)
{

    return (True);

} /* END OF FUNCTION F_Separator */


Boolean ForceRaiseWindow (ClientData *pcd)
{
#if 0
    Window stackWindow;
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
#endif
    XWindowChanges changes;
    Boolean restack = False;

#if 0
    if (pSD->clientList->type == MINIMIZED_STATE)
    {
	stackWindow = ICON_FRAME_WIN(pSD->clientList->pCD);
    }
    else
    {
	stackWindow = pSD->clientList->pCD->clientFrameWin;
    }
#endif

    /*
     * Windows did not raise on regular f.raise because the raise was
     * not relative to another window (methinks).
     */
    changes.stack_mode = Above;
    XConfigureWindow (DISPLAY, pcd->clientFrameWin, CWStackMode,
		      &changes);

    return (restack);
}



/*************************************<->*************************************
 *
 *  F_Raise (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for topping the client window
 *  so that it is unobscured.
 *
 *************************************<->***********************************/

Boolean F_Raise (String args, ClientData *pCD, XEvent *event)
{
    ClientListEntry *pEntry;
    ClientListEntry *pNextEntry;
    ClientListEntry *pStackEntry;
    String string = args;
    int flags = STACK_NORMAL;
#ifdef WSM
	WmWorkspaceData *pWS = ACTIVE_WS;
#endif /* WSM */

    if (string)
    {
	/* process '-client' argument */
	if (string[0] == '-')
	{
	    string = &string[1];
	    string = (String) GetString ((unsigned char **) &string);

	    pStackEntry = NULL;
	    pNextEntry = ACTIVE_PSD->clientList;
	    while (pNextEntry &&
		   (pEntry = FindClientNameMatch (pNextEntry, True, string,
						  F_GROUP_ALL)))
	    {
		pNextEntry = pEntry->nextSibling;
#ifdef WSM
	        if (ClientInWorkspace (pWS, pEntry->pCD))
		{
#endif /* WSM */
		Do_Raise (pEntry->pCD, pStackEntry, STACK_NORMAL);
		pStackEntry = pEntry;
#ifdef WSM
	        }
#endif /* WSM */
	    }
	}
	/* process family stacking stuff */
	else if (*string)
	{
	    unsigned int  slen, len, index;

	    slen = strlen(args) - 2;		/* subtract '\n' and NULL */
	    for (index = 0; index < slen; string = &args[index+1])
	    {
		if ((string = (String) GetString ((unsigned char **) &string)) == NULL)
		   break;
		len = strlen(string);
		if (!strcmp(string,"within"))
		{
		    flags |= STACK_WITHIN_FAMILY;
		}
		else if (!strcmp(string,"freeFamily"))
		{
		    flags |= STACK_FREE_FAMILY;
		}
		index += len;
	    }
#ifdef WSM
	    if (ClientInWorkspace (pWS, pCD))
	    {
#endif /* WSM */
	    Do_Raise (pCD, (ClientListEntry *) NULL, flags);
#ifdef WSM
	    }
#endif /* WSM */
	}
    }
    else if (pCD)
    {
#ifdef WSM
	if (ClientInWorkspace (pWS, pCD))
	{
#endif /* WSM */
	Do_Raise (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
#ifdef WSM
	}
#endif /* WSM */
    }

    return (True);

} /* END OF FUNCTION F_Raise */



/*************************************<->*************************************
 *
 *  Do_Raise (pCD, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for topping the client window
 *  so that it is unobscured.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data of the window (or icon) to be raised.
 * 
 *  pStackEntry = pointer to client list entry for window that is to be 
 *	above the raised window (if NULL window is raised to the top of the
 *	stack).
 *
 *************************************<->***********************************/

void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags)
{
    Boolean restackTransients;
    ClientData *pcdLeader;
    WmWorkspaceData *pWS = ACTIVE_WS;
#ifdef WSM
    Boolean bLeaderRestacked;
#endif /* WSM */

#ifdef PANELIST
    if (pCD->pECD)
    {
	/*
	 * Window has been reparented into the front panel. 
	 * Don't follow through on window stacking change.
	 */
	return;
    }
    else 
#else /* PANELIST */
#endif /* PANELIST */
#ifdef WSM
    if (ClientInWorkspace(pWS, pCD)  && 
	(!pStackEntry || ClientInWorkspace (pWS, pStackEntry->pCD)))
    {
	/* 
	 * Both clients are in the current workspace. Set
	 * client indices so that the access macros work.
	 */
	SetClientWsIndex (pCD);
	if (pStackEntry)
	{
	    SetClientWsIndex (pStackEntry->pCD);
	}
    }
    else
    {
	/*
	 * One or both of the clients are not in the current workspace
	 * Do nothing.
	 */
	return;
    }
#endif /* WSM */

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    if (wmGD.systemModalActive && (pcdLeader != wmGD.systemModalClient))
    {
	/*
	 * Don't raise the window above the system modal window.
	 */
    }
    else if ((pcdLeader->clientState == MINIMIZED_STATE) &&
	     !P_ICON_BOX(pcdLeader))
    {
        /*
         * If a dirtyStackEntry exists, return it to its original place 
         * in the stack (for all stacking types)
         */
        if (dirtyStackEntry)
        {
            if (dirtyStackEntry->transientChildren ||
                dirtyStackEntry->transientLeader)
                RestackTransients (dirtyStackEntry);
            dirtyStackEntry = NULL;
            dirtyLeader = NULL;
	}

	/*
	 * Only restack the icon if it is not currently raised.
	 */

	if (pStackEntry)
	{
	    if (pStackEntry->nextSibling != &pcdLeader->iconEntry)
	    {
	        StackWindow (pWS, &pcdLeader->iconEntry, False /*below*/,
			     pStackEntry);
	        MoveEntryInList (pWS, &pcdLeader->iconEntry, False /*below*/,
				 pStackEntry);
	    }
	}
	else
	{
	    if (ACTIVE_PSD->clientList != &pcdLeader->iconEntry)
	    {
	        StackWindow (pWS, &pcdLeader->iconEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
	        MoveEntryInList (pWS, &pcdLeader->iconEntry, 
		    True /*on top*/, (ClientListEntry *) NULL);
	    }
	}
    }
    else /* NORMAL_STATE, MAXIMIZED_STATE, adoption */
    {
#ifdef WSM
        /*
	 * Handle restacking of primary/secondary windows
	 * within the transient window tree. Don't raise this
	 * window above any modal transients.
	 */
        bLeaderRestacked = False;
	if ((pcdLeader->transientChildren) &&
	    (!pCD->secondariesOnTop) &&
	    (!wmGD.bSuspendSecondaryRestack) &&
	    (!IS_APP_MODALIZED(pCD)))
	{
	    if (pCD != pcdLeader)
	    {
		/* 
		 * This is not the transient leader, make sure
		 * the transient leader isn't on top.
		 * (Brute force solution)
		 */
		bLeaderRestacked = NormalizeTransientTreeStacking (pcdLeader);

		if (pCD->transientChildren)
		{
		    /*
		     * This isn't the overall leader of the transient 
		     * tree, but it does have transient's of its own.
		     * Move it to the top of its own transient sub-tree.
		     */
		    bLeaderRestacked |= BumpPrimaryToTop (pCD);
		}
	    }
	    else 
	    {
		/*
		 * This is the transient leader, move it to the
		 * top.
		 */
		bLeaderRestacked = BumpPrimaryToTop (pcdLeader);
	    }

	}
#endif /* WSM */
	/*
	 * If this is a transient window then put it on top of its
	 * sibling transient windows.
	 */

	restackTransients = False;


/*
 * Fix for 5325 - The following code has been reorganized to cause the
 *                action of F_Raise to match the current documentation.
 *                The new algorithm is as follows:
 *
 *                if (dirtyStackEntry)
 *                  restore dirty tree
 *                if (not withinFamily)
 *                  bring window group to the top of global stack
 *                if (freeFamily)
 *                  raise the requested window to top of family
 *                else
 *                  raise requested window to top of siblings
 *                if (need to restack windows)
 *                  restack windows
 *                return
 */
        /*
         * If a dirtyStackEntry exists, return it to its original place 
         * in the stack (for all stacking types)
         */
        if (dirtyStackEntry)
        {
            /*
             * Check to make sure that the dirty pCD has either transient
             * children or a transient leader.  If not, do not restore
             * the transients.
             */
            if (dirtyStackEntry->transientChildren ||
                dirtyStackEntry->transientLeader)
                RestackTransients (dirtyStackEntry);
            dirtyStackEntry = NULL;
            dirtyLeader = NULL;
	}

        /*
         * If the flags do not indicate "within", raise the window family
         * to the top of the window stack. If the window is the primary,
         * raise it to the top regardless of the flags.
         */
        if (!pCD->transientLeader || !(flags & STACK_WITHIN_FAMILY))
        {
            if (pStackEntry)
            {
                if (pStackEntry->nextSibling != &pcdLeader->clientEntry)
                {
                    StackWindow (pWS, &pcdLeader->clientEntry, 
                        False /*below*/, pStackEntry);
                    MoveEntryInList (pWS, &pcdLeader->clientEntry, 
                        False /*below*/, pStackEntry);
                }
            }
            else
            {
                if (ACTIVE_PSD->clientList != &pcdLeader->clientEntry)
                {
                    StackWindow (pWS, &pcdLeader->clientEntry,
                        True /*on top*/, (ClientListEntry *) NULL);
                    MoveEntryInList (pWS, &pcdLeader->clientEntry,
                        True /*on top*/, (ClientListEntry *) NULL);
                }
            }
        }
  
        /*
         * If freeFamily stacking is requested, check to make sure that
         * the window has either a transientChild or Leader.  This will
         * guarantee that windows that form their own family are not
         * labelled as dirty (what's to dirty it up?).  If it has either,
         * raise the window to the top of the family stack.
         */
        if ((flags & STACK_FREE_FAMILY) &&
            (pCD->transientLeader || pCD->transientChildren))
        {
            dirtyStackEntry = pCD;
	    dirtyLeader = pcdLeader;
  
            restackTransients = ForceRaiseWindow (pCD);
        }
  
        /*
         * If withinFamily stacking is requested, put the current transient
         * on top of its sibling transient windows.
         */
        else
        {
  	    restackTransients = PutTransientOnTop (pCD);
  	}
  
        /* At this point, if doing a regular f.raise the window family has
         * already been brought to the top of the stack, so nothing further
         * needs to be done for it.
         */
  
        /* Restack the transients if needed */
  
#ifdef WSM
        if ((restackTransients) || (bLeaderRestacked))
#else /* WSM */
        if (restackTransients)
#endif /* WSM */
        {
	    RestackTransients (pCD);
  	}
    }

} /* END OF FUNCTION Do_Raise */



/*************************************<->*************************************
 *
 *  F_Raise_Lower (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This window manager function tops an obscured window or icon and bottoms 
 *  a window or icon that is on top of the window stack.
 *
 *************************************<->***********************************/

Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event)
{
    ClientData *pcdLeader;

    if (pCD)
    {
	pcdLeader = (pCD->transientLeader) ?
					FindTransientTreeLeader (pCD) : pCD;

	/*
	 * Treat a raise/lower on a window in a transient tree as if it is
	 * a raise/lower for the whole tree.
	 */

	if (CheckIfClientObscuredByAny (pcdLeader))
	{
	    /*
	     * The window is obscured by another window, raise the window.
	     */

#ifdef WSM
	    F_Raise (NULL, pCD, (XEvent *)NULL);
#else /* WSM */
	    F_Raise (NULL, pcdLeader, (XEvent *)NULL);
#endif /* WSM */
	}
	else if (CheckIfClientObscuringAny (pcdLeader) &&
	        !(wmGD.systemModalActive &&
	         (pcdLeader == wmGD.systemModalClient)))
	{
	    /*
             * The window is obscuring another window and is
             * not system modal, lower the window.
	     */

	    F_Lower (NULL, pcdLeader, (XEvent *)NULL);
#ifdef WSM
	    if ((pcdLeader->secondariesOnTop == False) &&
		(pCD->transientLeader != NULL) &&
		(!IS_APP_MODALIZED(pcdLeader)))
	    {
		/* Push transient below primary */
		(void) BumpPrimaryToTop (pcdLeader);
	        RestackTransients (pcdLeader);
	    }
#endif /* WSM */
	}
#ifdef WSM
        else if ((pcdLeader->secondariesOnTop == False) &&
		 (pcdLeader->transientChildren != NULL) &&
		 (!wmGD.systemModalActive) &&
		 (!IS_APP_MODALIZED(pcdLeader)))
	{
	    if (LeaderOnTop(pcdLeader))
	    {
		/* Push primary below transient */
		(void) BumpPrimaryToBottom (pcdLeader);
	        RestackTransients (pcdLeader);
	    }
	    else
	    {
		F_Raise (NULL, pCD, (XEvent *)NULL);
		/* Push transient below primary */
		(void) BumpPrimaryToTop (pcdLeader);
	        RestackTransients (pcdLeader);
	    }
	}
#endif /* WSM */
    }

    return (True);

} /* END OF FUNCTION F_Raise_Lower */



/*************************************<->*************************************
 *
 *  F_Refresh_Win (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for causing a client window
 *  to redisplay itself.
 *
 *************************************<->***********************************/

Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event)
{
    Window win;
    unsigned int w, h;

    if (pCD && ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE)))
    {
        if (pCD->clientState == NORMAL_STATE)
	{
	    w = (unsigned int) pCD->clientWidth;
	    h = (unsigned int) pCD->clientHeight;
	}
	else
	{
	    w = (unsigned int) pCD->maxWidth;
	    h = (unsigned int) pCD->maxHeight;
	}

#ifdef WSM
        if (wmGD.refreshByClearing)
	{
	    RefreshByClearing (pCD->clientFrameWin);
	}
	else
	{
#endif /* WSM */
			 /* default background_pixmap is None */
        win = XCreateWindow (DISPLAY,
		         pCD->clientBaseWin,
		         pCD->matteWidth,
		         pCD->matteWidth,
		         w, h,
	                 0, 
	                 0,
	                 InputOutput,
                         CopyFromParent,
	                 0, 
			 (XSetWindowAttributes *)NULL);  

        XMapWindow (DISPLAY, win);
        XDestroyWindow (DISPLAY, win);
#ifdef WSM
	}
#endif /* WSM */
        XFlush (DISPLAY);
    }

    return (True);

} /* END OF FUNCTION F_Refresh_Win */



/*************************************<->*************************************
 *
 *  F_Set_Behavior (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to switch the window manager configuration between
 *  the built-in configuration (for CXI behavior) and the user's custom
 *  configuration.
 *
 *************************************<->***********************************/

Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event)
{
    /*
     * Go system modal in starting to do the set behavior.
     */

    /* !!! grab the server and the pointer !!! */


    /*
     * Confirm that a set_behavior should be done.
     * Execute restart if so.
     */

    if (wmGD.showFeedback & WM_SHOW_FB_BEHAVIOR)
    {
	ConfirmAction (ACTIVE_PSD, (wmGD.useStandardBehavior) ?
		       CUSTOM_BEHAVIOR_ACTION : DEFAULT_BEHAVIOR_ACTION);
    }
    else
    {
	RestartWm ((long) ((wmGD.useStandardBehavior) ?
			MWM_INFO_STARTUP_CUSTOM : MWM_INFO_STARTUP_STANDARD));
    }
    return (False);

} /* END OF FUNCTION F_Set_Behavior */



/*************************************<->*************************************
 *
 *  Do_Set_Behavior (dummy)
 *
 *
 *  Description:
 *  -----------
 *  Callback to do the f.set_behavior function.
 *
 *************************************<->***********************************/

void Do_Set_Behavior (Boolean dummy)
{
    RestartWm ((long) ((wmGD.useStandardBehavior) ?
			MWM_INFO_STARTUP_CUSTOM : MWM_INFO_STARTUP_STANDARD));

} /* END OF FUNCTION Do_Set_Behavior */

#ifdef WSM

/*************************************<->*************************************
 *
 *  F_Set_Context (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to set a client context for subsequent
 *  WM_REQUESTs
 *
 *************************************<->***********************************/

Boolean F_Set_Context (String args, ClientData *pCD, XEvent *event)
{
 
    wmGD.requestContextWin = (Window) args;
    return (True);

} /* END OF FUNCTION F_Set_Context */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  F_Title (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is a placeholder function; it should never be called.
 *
 *************************************<->***********************************/

Boolean F_Title (String args, ClientData *pCD, XEvent *event)
{

    return (True);

} /* END OF FUNCTION F_Title */



/******************************<->*************************************
 *
 *  F_Screen (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function handler for warping to screens
 *
 *
 *  Inputs:
 *  ------
 *  args = (immediate value) window type flags
 *
 *  pCD = pointer to the client data
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *  NOTE: May want to consider tracking changes in screen because in
 *	  managing a new window (ie. in ManageWindow()).
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *************************************<->***********************************/

Boolean F_Screen (String args, ClientData *pCD, XEvent *event)
{
    Window dumwin;
    int x, y, dumint;
    unsigned int dummask;
    WmScreenData *newscr = NULL;
    int scr, inc;
    static int PreviousScreen = -1;
    char pch[80];
    

    if (PreviousScreen == -1)
    {
	PreviousScreen = DefaultScreen(DISPLAY);
    }

    if (strcmp (args, "next") == 0)
    {
	scr = ACTIVE_PSD->screen + 1;
	inc = 1;
    }
    else if (strcmp (args, "prev") == 0)
    {
	scr = ACTIVE_PSD->screen - 1;
	inc = -1;
    }
    else if (strcmp (args, "back") == 0)
    {
	scr = PreviousScreen;
	inc = 0;
    }
    else
    {
	scr = atoi (args);
	inc = 0;
    }

    while (!newscr) {
					/* wrap around */
	if (scr < 0) 
	  scr = wmGD.numScreens - 1;
	else if (scr >= wmGD.numScreens)
	  scr = 0;

	newscr = &(wmGD.Screens[scr]);
	if (!wmGD.Screens[scr].managed) { /* make sure screen is managed */
	    if (inc) {			/* walk around the list */
		scr += inc;
		continue;
	    }
	    sprintf(pch, 
		    "Unable to warp to unmanaged screen %d\n", scr);
	    Warning (&pch[0]);
	    XBell (DISPLAY, 0);
	    return (False);
	}
    }

    if (ACTIVE_PSD->screen == scr) return (False);  /* already on that screen */

    PreviousScreen = ACTIVE_PSD->screen;
    XQueryPointer (DISPLAY, ACTIVE_ROOT, &dumwin, &dumwin, &x, &y,
		   &dumint, &dumint, &dummask);

    XWarpPointer (DISPLAY, None, newscr->rootWindow, 0, 0, 0, 0, x, y);

    if (newscr && (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
    {
	/*
	 * Set the ACTIVE_PSD to the new screen so that Do_Focus_Key can 
	 * uses the new screen instead of the old screen.  Then call
	 * Do_Focus_Key with a NULL pCD to find a reasonable client to
	 * set focus to.
	 */
	SetActiveScreen (newscr);
        Do_Focus_Key (NULL, GetFunctionTimestamp ((XButtonEvent *)event),
		      ALWAYS_SET_FOCUS);
    }

    return (False);
}



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  F_InvokeCommand (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager function for invoking client inserted menu
 *  commands.
 *
 *************************************<->***********************************/

Boolean F_InvokeCommand (String args, ClientData *pCD, XEvent *event)
{
    CARD32 commandID, clientWindow;
    Atom notifySelection;

    if (args == (String) NULL) return(FALSE);

    if (sscanf(args, "%d %d %ld", &commandID, &clientWindow,
	       &notifySelection) != 3)
      return(FALSE);

    SendInvokeMessage(commandID,
		      (pCD == (ClientData *) NULL ? 0 : pCD->client),
		      notifySelection,
		      LastTime());

    return (True);
} /* END OF FUNCTION F_InvokeCommand */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  GetFunctionTimestamp (pEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to extract a timestamp from a key or button event.
 *  If the event passed in is not a key or button event then a timestamp
 *  is generated.
 *
 *
 *  Inputs:
 *  ------
 *  event = pointer to an X event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = a timestamp
 *
 *************************************<->***********************************/

Time GetFunctionTimestamp (XButtonEvent *pEvent)
{
    Time time;

    if (pEvent &&
	(((pEvent->type == ButtonPress) || (pEvent->type == ButtonRelease)) ||
	 ((pEvent->type == KeyPress) || (pEvent->type == KeyRelease))))
    {
	time = pEvent->time;
    }
    else
    {
	time = GetTimestamp ();
    }

    return (time);

} /* END OF FUNCTION GetFunctionTimestamp */


/*
** name the event mask we need for a grab in order to find the matching 
** event for an event; right now handle only button-presses
*/
static unsigned int GetEventInverseMask(XEvent *event)
{
	if ((XEvent*)NULL == event)
		return 0;
	if (ButtonPress == event->type)
		return ButtonReleaseMask;	/* detail ? */
	/*
	expansion further here
	*/
	else 
		return 0;
}



/*************************************<->*************************************
 *
 *  ClearDirtyStackEntry (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to clear the static dirtyStackEntry structure and
 *  the dirtyLeader static variable when a pCD is destroyed.  This 
 *  guarantees that freed memory will not be accessed.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to clientData being freed
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = void
 *
 *************************************<->***********************************/

void ClearDirtyStackEntry (ClientData *pCD)
{
  if (pCD == dirtyStackEntry)
    {
      dirtyStackEntry = NULL;
      dirtyLeader = NULL;
    }
}
#if defined(DEBUG) && defined(WSM)

/***********************<->*************************************
 *
 *  F_ZZ_Debug (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This is the window manager debug (multi) function
 *
 *
 *  Inputs:
 *  ------
 *  args = arguments
 *
 *  pCD = pointer to the ClientData for the whole front panel
 *
 *  event = X event that invoked the function (key, button, or menu/NULL)
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = if True then further button binding/function processing can
 *           be done for the event that caused this function to be called.
 *
 *  Comments:
 *  -------
 *  Argument 1 determines the debug function to execute:
 *
 *  Valid arguments:
 *
 *      "color_server_info"  - dump out color server info
 *
 ******************************<->***********************************/

Boolean
F_ZZ_Debug (String subFcn, ClientData *pCD, XEvent *event)
{
    /* Only do something is sub function is specified */

    if (subFcn)
    {
	if (!(strcmp(subFcn, "dump_resources")))
	{
	    int scr;
	    char szRes[80];

	    for (scr=0; scr<wmGD.numScreens; scr++)
	    {
		sprintf (szRes, "/tmp/dtwm.resources.%d", scr);

		XrmPutFileDatabase(XtScreenDatabase(
				       XScreenOfDisplay(DISPLAY, scr)), 
				   szRes);
	    }
	}
    }
    return (True);
}
#endif /* DEBUG */

#ifdef WSM

/*************************************<->*************************************
 *
 *  F_Next_Workspace (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function switches to the next workspace in the list
 *
 *************************************<->***********************************/

Boolean F_Next_Workspace (String args, ClientData *pCD, XEvent *event)
{
    WmScreenData *pSD = ACTIVE_PSD;
    int iwsx;

    for (iwsx = 0; iwsx < pSD->numWorkspaces; iwsx++)
    {
	if (pSD->pWS[iwsx].id == pSD->pActiveWS->id)
	{
	    iwsx++;
	    break;
	}
    }

    /* check bounds and wrap */
    if (iwsx >= pSD->numWorkspaces)
	iwsx = 0;

    ChangeToWorkspace (&pSD->pWS[iwsx]);


    return (False);

} /* END OF FUNCTION F_Next_Workspace */


/*************************************<->*************************************
 *
 *  F_Prev_Workspace (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function switches to the previous workspace in the list
 *
 *************************************<->***********************************/

Boolean F_Prev_Workspace (String args, ClientData *pCD, XEvent *event)
{
    WmScreenData *pSD = ACTIVE_PSD;
    int iwsx;

    for (iwsx = 0; iwsx < pSD->numWorkspaces; iwsx++)
    {
	if (pSD->pWS[iwsx].id == pSD->pActiveWS->id)
	{
	    iwsx--;
	    break;
	}
    }

    /* check bounds and wrap */
    if (iwsx < 0)
	iwsx = pSD->numWorkspaces - 1;

    ChangeToWorkspace (&pSD->pWS[iwsx]);


    return (False);

} /* END OF FUNCTION F_Prev_Workspace */



/*************************************<->*************************************
 *
 *  F_Workspace_Presence (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  This function pops up the workspace presence dialog box
 *
 *************************************<->***********************************/

Boolean F_Workspace_Presence (String args, ClientData *pCD, XEvent *event)
{
    Context wsContext = (Context)NULL;

    if (pCD && (pCD->dtwmFunctions & DtWM_FUNCTION_OCCUPY_WS))
    {
	if (pCD->clientState == NORMAL_STATE)
	{
	    wsContext = F_CONTEXT_NORMAL;
	}
	else if (pCD->clientState == MAXIMIZED_STATE)
	{
	    wsContext = F_CONTEXT_MAXIMIZE;
	}
	else 
	{
	    wsContext = F_CONTEXT_ICON;
/*	    return (False); */
	}
	ShowPresenceBox (pCD, wsContext);
    }
    return (False);

} /* END OF FUNCTION F_Workspace_Presence */

#ifdef DEBUG
void
DumpWindowList ()
{
    WmScreenData *pSD = (ACTIVE_WS)->pSD;
    ClientListEntry 	*pCLE;

    fprintf (stdout, "Window stacking (bottom to top)\n");
    pCLE = pSD->lastClient;
    while (pCLE)
    {
	if (ClientInWorkspace (ACTIVE_WS, pCLE->pCD))
	    fprintf (stdout, "* ");
	else
	    fprintf (stdout, "  ");

	fprintf (stdout, "%08lx\t%s\n", 
	    pCLE->pCD->client,
	    pCLE->pCD->clientName);

	pCLE = pCLE->prevSibling;
    }
}
#endif /* DEBUG */

#endif /* WSM */



