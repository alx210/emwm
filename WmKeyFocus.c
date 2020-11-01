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
static char rcsid[] = "$XConsortium: WmKeyFocus.c /main/5 1996/05/17 12:53:16 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
/*
 * include extern functions
 */
#include "WmKeyFocus.h"
#include "WmCDecor.h"
#include "WmColormap.h"
#include "WmEvent.h"
#include "WmCEvent.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmProtocol.h"
#include "WmWinInfo.h"
#include "WmWinList.h"



/*
 * Global Variables:
 */

static Boolean removeSelectGrab = True;



/*************************************<->*************************************
 *
 *  InitKeyboardFocus ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets the keyboard input focus to a client window or icon
 *  when the window manager starts up.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD = (keyboardFocusPolicy, colormapFocusPolicy)
 *
 *************************************<->***********************************/

void InitKeyboardFocus (void)
{
    ClientData *pCD;
    Boolean sameScreen;
    Boolean focusSet = False;
    int scr;
    int junk;
    Window junk_win, root_returned;
    int  currentX, currentY;


    /*
     * Set the keyboard focus based on the keyboard focus policy.
     */

    wmGD.keyboardFocus = NULL;
    wmGD.nextKeyboardFocus = NULL;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    wmGD.Screens[scr].focusPriority = 0;

	    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	    {
		/*
		 * Set the keyboard focus to the window that 
		 * currently contains the pointer.
		 */

		pCD = GetClientUnderPointer (&sameScreen);

		if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
		{
		    /*
		     * Do some colormap installation that has been 
		     * deferred from the InitColormapFocus routine.
		     */

		    SetColormapFocus (ACTIVE_PSD, pCD);
		}

		if (pCD)
		{
		    Do_Focus_Key (pCD, GetTimestamp (), ALWAYS_SET_FOCUS);
		    focusSet = True;
		}
	    }
	    else
	    {
		ButtonSpec *buttonSpec;
		
		/*
		 * Prepare to do explicit selection button grabs.
		 */

		buttonSpec = wmGD.Screens[scr].buttonSpecs;
		while (buttonSpec)
		{
		    if ((buttonSpec->button == FOCUS_SELECT_BUTTON) &&
			(buttonSpec->context & F_CONTEXT_WINDOW) &&
			(buttonSpec->subContext & F_SUBCONTEXT_W_CLIENT))
		    {
			if (buttonSpec->state == 0)
			{
			    removeSelectGrab = False;
			}
		    }
		    buttonSpec = buttonSpec->nextButtonSpec;
		}
	    }
	}
    }


    if (!focusSet)
    {
        /*
         * This is keyboard focus policy is either "explicit" or it it 
	 * "pointer"
         * and there is no window under the pointer.  No window currently has
         * the keyboard input focus.  Set the keyboard focus to the window
         * manager default (non-client) OR to the last client with focus.
	 *
	 * In Mwm 1.1.4 and later, calling Do_Focus_Key with NULL will try
	 * to find a 'reasonable' window to put focus.  This means that on
	 * startup and restarts, a Mwm window will have focus!  Yeah!
         */

	/* 
	 * Set Active Screen First 
	 */
	if (XQueryPointer(DISPLAY, DefaultRootWindow(DISPLAY), 
			  &root_returned, &junk_win,
			  &currentX, &currentY, 
			  &junk, &junk, (unsigned int *)&junk))
	{
	    for (scr = 0; scr < wmGD.numScreens; scr++)
	    {
		if (wmGD.Screens[scr].managed && 
		    wmGD.Screens[scr].rootWindow == root_returned)
		{
		    SetActiveScreen(&(wmGD.Screens[scr]));
		    break;
		}
	    }
	}

        Do_Focus_Key ((ClientData *)NULL, CurrentTime, ALWAYS_SET_FOCUS);
    }

} /* END OF FUNCTION InitKeyboardFocus */



/*************************************<->*************************************
 *
 *  SetKeyboardFocus (pCD, focusFlags)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to handle a client window getting the input
 *  focus (as the RESULT of an XSetInput call - probably done by 
 *  Do_Focus_Key).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for window that is to get the focus
 *
 *  focusFlags = flags that indicate focus change details
 *	{REFRESH_LAST_FOCUS}
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (keyboardFocus)
 * 
 *************************************<->***********************************/

void SetKeyboardFocus (ClientData *pCD, long focusFlags)
{
    ClientData *currentFocus;

    
    /*
     * Don't set the keyboard input focus if it is already set to
     * the client window.
     */

    if (wmGD.keyboardFocus == pCD)
    {
	return;
    }
    currentFocus = wmGD.keyboardFocus;
    ACTIVE_PSD->focusPriority++;


    /*
     * If the keyboard input focus policy is "explicit" then reset the
     * selection button event handling.
     */

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	/*
	 * Reset explicit focus selection event tracking on the last focus
	 * window (reset the passive grab on the focus button).
	 */

	if (currentFocus)
	{
	    ResetExplicitSelectHandling (currentFocus);
	    wmGD.keyboardFocus = NULL;
	}
	
	if (pCD && ((pCD->clientState == NORMAL_STATE) ||
		    (pCD->clientState == MAXIMIZED_STATE)))
	{
	    /*
	     * The focus is to be set to a client window (not the root).
	     * Stop explicit focus selection event tracking on the new focus
	     * window.
	     */

	    if (removeSelectGrab)
	    {
	        WmUngrabButton (DISPLAY, FOCUS_SELECT_BUTTON, 0,
		    pCD->clientBaseWin);
	    }
        }
    }
    
    wmGD.keyboardFocus = pCD;


    /*
     * Do focus auto raise if specified.
     */

    if (pCD && pCD->focusAutoRaise)
    {
	if (wmGD.autoRaiseDelay &&
	    (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER))
	{
	    AddWmTimer (TIMER_RAISE, (unsigned long)wmGD.autoRaiseDelay,
		pCD);
	}
	else
	{
	    Boolean sameScreen;

	    if (((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
		 (!pCD->focusAutoRaiseDisabled)) ||
		((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) &&
		 (pCD == GetClientUnderPointer (&sameScreen))))
	    {
	        Do_Raise (pCD, (ClientListEntry *)NULL, STACK_NORMAL);
	    }
	}
    }


    /*
     * Clear the focus indication if it is set for a client window or icon.
     */

    if (currentFocus)
    {
	ClearFocusIndication (currentFocus,
	    ((focusFlags & REFRESH_LAST_FOCUS) ? True : False));
    }


    /*
     * Install the client window colormap if the colormap focus policy is
     * "keyboard".
     */

    if ((wmGD.colormapFocusPolicy == CMAP_FOCUS_KEYBOARD) &&
	(!(focusFlags & SCREEN_SWITCH_FOCUS)))
    {
	SetColormapFocus (ACTIVE_PSD, pCD);
    }


    /*
     * Set the focus window or icon visual indication.
     */

    if (pCD)
    {
	pCD->focusPriority = ACTIVE_PSD->focusPriority;
	SetFocusIndication (pCD);
    }

} /* END OF FUNCTION SetKeyboardFocus */



/*************************************<->*************************************
 *
 *  ResetExplicitSelectHandling (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function resets the selection button event handling for a client
 *  window or icon.  This applies only if the keyboard focus policy is
 *  "explicit".
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for window that has focus handling reset
 *
 *************************************<->***********************************/

void ResetExplicitSelectHandling (ClientData *pCD)
{
#ifdef WSM
    Boolean bUnseen;

    bUnseen = (pCD->clientState & UNSEEN_STATE) ? True : False;
    if (bUnseen)
	pCD->clientState &= ~UNSEEN_STATE;

#endif /* WSM */
    if ((pCD->clientState == NORMAL_STATE) ||
	(pCD->clientState == MAXIMIZED_STATE))
    {
	/*
	 * A client window was selected.
	 */

	DoExplicitSelectGrab (pCD->clientBaseWin);
    }
    else if (pCD->clientState == MINIMIZED_STATE)
    {
	/*
	 * An icon was selected.
	 */

	/* !!! grab reset if client icon window? !!! */
    }
#ifdef WSM

    if (bUnseen)
	pCD->clientState |= UNSEEN_STATE;
#endif /* WSM */
    

} /* END OF FUNCTION ResetExplicitSelectHandling */    



/*************************************<->*************************************
 *
 *  DoExplicitSelectGrab (window)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to do a grab button on the specified window.  The
 *  grab is intended to catch the keyboard focus select button.
 *
 *
 *  Inputs:
 *  ------
 *  widow = grab widow for the select button
 * 
 *************************************<->***********************************/

void DoExplicitSelectGrab (Window window)
{

    WmGrabButton (DISPLAY, FOCUS_SELECT_BUTTON, 0, window,
	False, ButtonReleaseMask, GrabModeSync, GrabModeSync, None,
	None);

} /* END OF FUNCTION DoExplicitSelectGrab */



/*************************************<->*************************************
 *
 *  SetFocusIndication (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function changes the client window or icon decoration to have it
 *  indicate that the window or icon has the keyboard input focus.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for window/icon that is getting the focus
 *
 * 
 *************************************<->***********************************/

void SetFocusIndication (ClientData *pCD)
{
    ClientData *saveCD;

    /* 
     * Set the "focus" to pCD to insure correct display of the frame 
     * This is necessary because the called routines get GCs based
     * on the current keyboard focus.
     */
    saveCD = wmGD.keyboardFocus;
    wmGD.keyboardFocus = pCD;

    if ((pCD->clientState == NORMAL_STATE) ||
	(pCD->clientState == MAXIMIZED_STATE))
    {
	/*
	 * A client window has the input focus.
	 */

	ShowActiveClientFrame (pCD);
    }
    else if (pCD->clientState == MINIMIZED_STATE)
    {
	/*
	 * An icon has the input focus.
	 */

	ShowActiveIcon (pCD);
    }

    /* restore old keyboard focus */
    wmGD.keyboardFocus = saveCD;

} /* END OF FUNCTION SetFocusIndication */



/*************************************<->*************************************
 *
 *  ClearFocusIndication (pCD, refresh)
 *
 *
 *  Description:
 *  -----------
 *  This function changes the client window or icon decoration to have it
 *  indicate that the window or icon does not have the keyboard input focus
 *  (i.e. it is inactive).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for window/icon that is losing the focus
 *
 *  refresh = True if window/icon frame is to redrawn
 *
 *************************************<->***********************************/

void ClearFocusIndication (ClientData *pCD, Boolean refresh)
{
    ClientData *saveCD;
#ifdef WSM
    Boolean bUnseen;
#endif /* WSM */

    /* 
     * Set the "focus" to NULL to insure correct display of the frame 
     * This is necessary because the called routines get GCs based
     * on the current keyboard focus.
     */

    saveCD = wmGD.keyboardFocus;
    wmGD.keyboardFocus = NULL;
#ifdef WSM
    bUnseen = (pCD->clientState & UNSEEN_STATE) ? True : False;
    if (bUnseen)
	pCD->clientState &= ~UNSEEN_STATE;
#endif /* WSM */

    if ((pCD->clientState == NORMAL_STATE) ||
	(pCD->clientState == MAXIMIZED_STATE))
    {
	/*
	 * A client window no longer has the input focus.
	 */

	ShowInactiveClientFrame (pCD);
    }
    else if (pCD->clientState == MINIMIZED_STATE)
    {
	/*
	 * An icon no longer has the input focus.
	 */

	ShowInactiveIcon (pCD, refresh);
    }

#ifdef WSM
    if (bUnseen) 
	pCD->clientState |= UNSEEN_STATE;
#endif /* WSM */

    /* restore old keyboard focus */
    wmGD.keyboardFocus = saveCD;

} /* END OF FUNCTION ClearFocusIndication */



/*************************************<->*************************************
 *
 *  GetClientUnderPointer (pSameScreen)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the managed client window or icon that is under
 *  the pointer.
 *
 *
 *  Outputs:
 *  -------
 *  pSameScreen = pointer to flag that indicates if pointer is on the wm screen
 *
 *  Return = client data pointer for the client window / icon under the
 *           mouse cursor
 *        
 *************************************<->***********************************/

ClientData *GetClientUnderPointer (pSameScreen)
    Boolean *pSameScreen;

{
    Window root;
    Window child;
    int rootX;
    int rootY;
    int winX;
    int winY;
    unsigned int mask;
    ClientData *pCD;


    if ((*pSameScreen = XQueryPointer (DISPLAY, ACTIVE_ROOT, &root, &child,
			   &rootX, &rootY, &winX, &winY, &mask)) != False)
    {
	if (child && 
	    !XFindContext (DISPLAY, child, wmGD.windowContextType,
		 (caddr_t *)&pCD))
	{
	    /*
	     * There is a client window or icon under the pointer.
	     */

	    return (pCD);
	}
    }

    return (NULL);

} /* END OF FUNCTION GetClientUnderPointer */



/*************************************<->*************************************
 *
 *  FocusNextWindow (type, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to change the focus to the next window in the 
 *  window stacking order.  The next focus window must be of the specified
 *  type(s).  If the focus traversal cannot be done because there is not
 *  an object of the specified type (accepting focus) then don't change the
 *  focus (!!!should the focus be unset in this case!!!).
 *
 *
 *  Inputs:
 *  ------
 *  type = type of objects to change the focus to
 *
 *  focusTime = timestamp to be used for setting the input focus
 *
 *************************************<->***********************************/

Boolean FocusNextWindow (unsigned long type, Time focusTime)
{
    ClientListEntry *pCurrentEntry;
    ClientListEntry *pNextEntry;
    Boolean focused = False;
    ClientData *pCD;


    /*
     * Identify the window or icon that currently has the focus and start
     * traversing to the next object.
     */

    if (type & F_GROUP_TRANSIENT)
    {
	/*
	 * Move the keyboard input focus around in a transient tree.
	 */

	focused = FocusNextTransient (wmGD.keyboardFocus, type, False,
				      focusTime);
    }

    if (!focused)
    {
	if (wmGD.systemModalActive)
	{
	    focused = True;
	}
        else if (wmGD.keyboardFocus)
        {
	    if (wmGD.keyboardFocus->transientLeader)
	    {
		pCD = FindTransientTreeLeader (wmGD.keyboardFocus);
	    }
	    else
	    {
		pCD = wmGD.keyboardFocus;
	    }

	    if (pCD->clientState == MINIMIZED_STATE)
	    {
		pCurrentEntry = &pCD->iconEntry;
	    }
	    else
	    {
		pCurrentEntry = &pCD->clientEntry;
	    }

	    pNextEntry = pCurrentEntry->nextSibling;
	    if (!pNextEntry)
	    {
		pNextEntry = ACTIVE_PSD->clientList;
	    }
    	}
    	else
    	{
		pCurrentEntry = ACTIVE_PSD->clientList;
		pNextEntry = pCurrentEntry;
    	}
    }


    while (!focused && pNextEntry)
    {
	focused = CheckForKeyFocus (pNextEntry, type, True /*next*/, focusTime);
	if (!focused)
	{
	    pNextEntry = pNextEntry->nextSibling;
        }
    }

    if (!focused)
    {
	pNextEntry = ACTIVE_PSD->clientList;
	while ((pNextEntry != pCurrentEntry) && !focused)
	{
	    focused = CheckForKeyFocus (pNextEntry, type, True/*next*/,
					focusTime);
	    if (!focused)
	    {
		pNextEntry = pNextEntry->nextSibling;
	    }
	}
    }

    return (focused);

} /* END OF FUNCTION FocusNextWindow */



/*************************************<->*************************************
 *
 *  FocusNextTransient (pCD, type, initiate, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to determine if another window in a transient
 *  tree should get the input focus.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the client window that has the focus
 *
 *  type = type of objects to change the focus to
 *
 *  initiate = set True if transient focus traversal is to be initiated;
 *	set to False if transient focus traversal is to be continued
 *
 *  focusTime = timestamp to be used to set the input focus
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the focus window has been identified and the focus
 *	has been set (or is already set)
 *
 *************************************<->***********************************/

Boolean FocusNextTransient (ClientData *pCD, unsigned long type, Boolean initiate, Time focusTime)
{
    Boolean focused = False;
    unsigned long startAt;
    ClientData *pcdLeader;
    ClientData *pcdLowerLeader;
    ClientData *pcdFocus;


    if (initiate && !(type & F_GROUP_TRANSIENT))
    {
	/*
	 * If in a transient tree focus on the last transient window that
	 * had the focus.
	 */

	if (pCD->transientChildren)
	{
	    pcdFocus = FindLastTransientTreeFocus (pCD, (ClientData *)NULL);
	    if (pcdFocus != wmGD.keyboardFocus)
	    {
		pcdLeader = FindTransientTreeLeader (pcdFocus);
		if (wmGD.keyboardFocus && wmGD.keyboardFocus->focusAutoRaise &&
		    (wmGD.keyboardFocus != pcdLeader))
	        {
		    pcdLowerLeader =
				FindTransientTreeLeader (wmGD.keyboardFocus);
		    if (pcdLowerLeader == pcdLeader)
		    {
			if (PutTransientBelowSiblings (wmGD.keyboardFocus))
			{
			    RestackTransients (pcdLeader);
			}
		    }
		    else
		    {
		        F_Lower (NULL, wmGD.keyboardFocus, (XEvent *) NULL);
		    }
	        }
	        Do_Focus_Key (pcdFocus, focusTime, ALWAYS_SET_FOCUS);
	    }
	    focused = True;
	}
	else
	{
	    focused = False;
	}
    }
    else if (pCD && (pCD->clientState != MINIMIZED_STATE) &&
	     (pCD->transientLeader || pCD->transientChildren))
    {
	startAt = (initiate) ? (ACTIVE_PSD->clientCounter + 1) : 
	    pCD->clientID;
	pcdLeader = FindTransientTreeLeader (pCD);
	pcdFocus = FindNextTFocusInSeq (pcdLeader, startAt);
	if ((pcdFocus == NULL) && (type == F_GROUP_TRANSIENT))
	{
	    /*
	     * Wrap around and find a focus window.
	     */

	    pcdFocus = FindNextTFocusInSeq (pcdLeader,
			   (unsigned long) (ACTIVE_PSD->clientCounter + 1));
	}
	if (pcdFocus)
	{
	    if (pcdFocus != wmGD.keyboardFocus)
	    {
	        if (wmGD.keyboardFocus && wmGD.keyboardFocus->focusAutoRaise &&
		    (wmGD.keyboardFocus != pcdLeader))
	        {
		    pcdLowerLeader =
				FindTransientTreeLeader (wmGD.keyboardFocus);
		    if (pcdLowerLeader == pcdLeader)
		    {
			if (PutTransientBelowSiblings (wmGD.keyboardFocus))
			{
			    RestackTransients (pcdLeader);
			}
		    }
		    else
		    {
		        F_Lower (NULL, wmGD.keyboardFocus, (XEvent *)NULL);
		    }
	        }
	        Do_Focus_Key (pcdFocus, focusTime, ALWAYS_SET_FOCUS);
	    }
	    focused = True;
	}
    }
    else
    {
	if (type == F_GROUP_TRANSIENT)
	{
	    /*
	     * Focus only within a transient tree.  In this case the current
	     * or prospective focus is not within a transient tree so leave
	     * the focus where it is.
	     */

	    focused = True;
	}
    }

    return (focused);

} /* END OF FUNCTION FocusNextTransient */



/*************************************<->*************************************
 *
 *  FindLastTransientTreeFocus (pCD, pcdNoFocus)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to scan a transient tree for the last window in
 *  the tree that had the focus.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the transient tree (or subtree)
 *	leader.
 *
 *  pcdNoFocus = pointer to the client data for a client window that is not
 *	to get the input focus (NULL if no client window restriction).
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data of the window that last had the
 *	focus.
 *
 *************************************<->***********************************/

ClientData *FindLastTransientTreeFocus (pCD, pcdNoFocus)
    ClientData *pCD;
    ClientData *pcdNoFocus;

{
    ClientData *pcdNext;
    ClientData *pcdFocus;
    ClientData *pcdLastFocus = NULL;


    pcdNext = pCD->transientChildren;
    while (pcdNext)
    {
	pcdFocus = FindLastTransientTreeFocus (pcdNext, pcdNoFocus);
	if (pcdFocus &&
	    (!IS_APP_MODALIZED(pcdFocus)) &&
	    ((pcdLastFocus == NULL) ||
	     (pcdFocus->focusPriority > pcdLastFocus->focusPriority)))
	{
	    pcdLastFocus = pcdFocus;
	}
	pcdNext = pcdNext->transientSiblings;
    }

    if ((!IS_APP_MODALIZED(pCD)) &&
	((pcdLastFocus == NULL) ||
	 (pCD->focusPriority > pcdLastFocus->focusPriority)))
    {
	pcdLastFocus = pCD;
    }

    return (pcdLastFocus);


} /* END OF FUNCTION FindLastTransientTreeFocus */



/*************************************<->*************************************
 *
 *  FindNextTFocusInSeq (pCD, startAt)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to scan a transient tree for the next window that
 *  can accept the focus.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the transient tree (or subtree)
 *	leader.
 *
 *  startAt = focus window should have a lower id then this client id
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data of the window that should get the
 *	focus.
 *
 *************************************<->***********************************/

ClientData *FindNextTFocusInSeq (pCD, startAt)
    ClientData *pCD;
    unsigned long startAt;

{
    ClientData *pcdNextFocus = NULL;
    ClientData *pcdNext;
    ClientData *pcdFocus;


    pcdNext = pCD->transientChildren;
    while (pcdNext)
    {
	pcdFocus = FindNextTFocusInSeq (pcdNext, startAt);
	if (pcdFocus)
	{
	    if ((pcdNextFocus == NULL) ||
		(pcdFocus->clientID > pcdNextFocus->clientID))
	    {
		pcdNextFocus = pcdFocus;
	    }
	}
	pcdNext = pcdNext->transientSiblings;
    }

    if ((pcdNextFocus == NULL) ||
	(pCD->clientID > pcdNextFocus->clientID))
    {
	if ((!IS_APP_MODALIZED(pCD)) && (pCD->clientID < startAt))
	{
	    pcdNextFocus = pCD;
	}
    }

    return (pcdNextFocus);


} /* END OF FUNCTION FindNextTFocusInSeq */



/*************************************<->*************************************
 *
 *  FocusPrevWindow (type, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to change the focus to the previous window in the 
 *  window stacking order.  The next focus window must be of the specified
 *  type(s).  If the focus traversal cannot be done because there is not
 *  an object of the specified type (accepting focus) then don't change the
 *  focus (!!!should the focus be unset in this case!!!).
 *
 *
 *  Inputs:
 *  ------
 *  type = type of objects to change the focus to
 *
 *  focusTime = timestamp to be used to set the input focus
 *
 *************************************<->***********************************/

Boolean FocusPrevWindow (unsigned long type, Time focusTime)
{
    ClientListEntry *pCurrentEntry;
    ClientListEntry *pNextEntry;
    Boolean focused = False;
    ClientData *pCD;


    /*
     * Identify the window or icon that currently has the focus and start
     * traversing to the previous object.
     */

    if (type & F_GROUP_TRANSIENT)
    {
	/*
	 * Move the keyboard input focus around in a transient tree.
	 */

	focused = FocusPrevTransient (wmGD.keyboardFocus, type, False,
				      focusTime);
    }
    
    if (!focused)
    {
	if (wmGD.systemModalActive)
	{
	    focused = True;
	}
        else if (wmGD.keyboardFocus)
        {
	    if (wmGD.keyboardFocus->transientLeader)
	    {
		pCD = FindTransientTreeLeader (wmGD.keyboardFocus);
	    }
	    else
	    {
		pCD = wmGD.keyboardFocus;
	    }

	    if (pCD->clientState == MINIMIZED_STATE)
	    {
	        pCurrentEntry = &pCD->iconEntry;
	    }
	    else
	    {
	        pCurrentEntry = &pCD->clientEntry;
	    }

	    pNextEntry = pCurrentEntry->prevSibling;
	    if (!pNextEntry)
	    {
	        pNextEntry = ACTIVE_PSD->lastClient;
	    }
        }
        else
        {
	    pCurrentEntry = ACTIVE_PSD->lastClient;
	    pNextEntry = pCurrentEntry;
        }
    }


    while (!focused && pNextEntry)
    {
	focused = CheckForKeyFocus (pNextEntry, type, False /*previous*/,
				    focusTime);
	if (!focused)
	{
	    pNextEntry = pNextEntry->prevSibling;
        }
    }

    if (!focused)
    {
	pNextEntry = ACTIVE_PSD->lastClient;
	while ((pNextEntry != pCurrentEntry) && !focused)
	{
	    focused = CheckForKeyFocus (pNextEntry, type, False/*previous*/,
					focusTime);
	    if (!focused)
	    {
		pNextEntry = pNextEntry->prevSibling;
	    }
	}
    }

    return (focused);

} /* END OF FUNCTION FocusPrevWindow */



/*************************************<->*************************************
 *
 *  FocusPrevTransient (pCD, type, initiate, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to determine if another (previous) window in a
 *  transient tree should get the input focus.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the client window that has the focus
 *
 *  type = type of objects to change the focus to
 *
 *  initiate = set True if transient focus traversal is to be initiated;
 *	set to False if transient focus traversal is to be continued
 *
 *  focusTime = timestamp to be used to set the input focus
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the focus window has been identified and the focus
 *	has been set (or is already set)
 *
 *************************************<->***********************************/

Boolean FocusPrevTransient (ClientData *pCD, unsigned long type, Boolean initiate, Time focusTime)
{
    Boolean focused = False;
    unsigned long startAt;
    ClientData *pcdLeader;
    ClientData *pcdFocus;


    if (initiate && !(type & F_GROUP_TRANSIENT))
    {
	/*
	 * If in a transient tree focus on the last transient window that
	 * had the focus.
	 */

	if (pCD->transientChildren)
	{
	    pcdFocus = FindLastTransientTreeFocus (pCD, (ClientData *)NULL);
	    if (pcdFocus != wmGD.keyboardFocus)
	    {
	        Do_Focus_Key (pcdFocus, focusTime, ALWAYS_SET_FOCUS);
	    }
	    focused = True;
	}
	else
	{
	    focused = False;
	}
    }
    else if (pCD && (pCD->clientState != MINIMIZED_STATE) &&
	     (pCD->transientLeader || pCD->transientChildren))
    {
	startAt = (initiate) ? 0 : pCD->clientID;
	pcdLeader = FindTransientTreeLeader (pCD);
	pcdFocus = FindPrevTFocusInSeq (pcdLeader, startAt);
	if ((pcdFocus == NULL) && (type == F_GROUP_TRANSIENT))
	{
	    /*
	     * Wrap around and find a focus window.
	     */

	    pcdFocus = FindPrevTFocusInSeq (pcdLeader, 0);
	}
	if (pcdFocus)
	{
	    if (pcdFocus != wmGD.keyboardFocus)
	    {
	        Do_Focus_Key (pcdFocus, focusTime, ALWAYS_SET_FOCUS);
	    }
	    focused = True;
	}
    }
    else
    {
	if (type == F_GROUP_TRANSIENT)
	{
	    /*
	     * Focus only within a transient tree.  In this case the current
	     * or prospective focus is not within a transient tree so leave
	     * the focus where it is.
	     */

	    focused = True;
	}
    }

    return (focused);

} /* END OF FUNCTION FocusPrevTransient */



/*************************************<->*************************************
 *
 *  FindPrevTFocusInSeq (pCD, startAt)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to scan a transient tree for the previous window that
 *  can accept the focus.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the transient tree (or subtree)
 *	leader.
 *
 *  startAt = focus window should have a higher id then this client id
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data of the window that should get the
 *	focus.
 *
 *************************************<->***********************************/

ClientData *FindPrevTFocusInSeq (pCD, startAt)
    ClientData *pCD;
    unsigned long startAt;

{
    ClientData *pcdNextFocus = NULL;
    ClientData *pcdNext;
    ClientData *pcdFocus;


    pcdNext = pCD->transientChildren;
    while (pcdNext)
    {
	pcdFocus = FindPrevTFocusInSeq (pcdNext, startAt);
	if (pcdFocus)
	{
	    if ((pcdNextFocus == NULL) ||
		(pcdFocus->clientID < pcdNextFocus->clientID))
	    {
		pcdNextFocus = pcdFocus;
	    }
	}
	pcdNext = pcdNext->transientSiblings;
    }

    if ((pcdNextFocus == NULL) ||
	(pCD->clientID < pcdNextFocus->clientID))
    {
	if (!(IS_APP_MODALIZED(pCD)) && (pCD->clientID > startAt))
	{
	    pcdNextFocus = pCD;
	}
    }

    return (pcdNextFocus);


} /* END OF FUNCTION FindPrevTFocusInSeq */



/*************************************<->*************************************
 *
 *  CheckForKeyFocus (pNextEntry, type, focusNext, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to determine if a window is a worthy candidate for
 *  getting the input focus (it is on-screen and is of the desired type).
 *  If it is, the window gets the keyboard input focus.
 *
 *
 *  Inputs:
 *  ------
 *  pNextEntry = the client list entry to be checked
 *
 *  type = the desired type of the focus window
 *
 *  focusNext = if true then focus the next window in the window stack
 *
 *  focusTime = timestamp to be used to set the input focus
 *
 * 
 *  Outputs:
 *  -------
 *  Return = True if the window gets the keyboard input focus otherwise False
 *
 *************************************<->***********************************/

Boolean CheckForKeyFocus (ClientListEntry *pNextEntry, unsigned long type, Boolean focusNext, Time focusTime)
{
    ClientData *pCD = pNextEntry->pCD;
    unsigned long windowType;
    Boolean focused = False;


    /*
     * First check for focusing within a transient tree.
     */


    /*
     * Make sure the window is being displayed and is of the specified type.
     */

    if (((pNextEntry->type == NORMAL_STATE) &&
#ifdef WSM
         (!(pCD->clientState & UNSEEN_STATE)) &&
#endif /* WSM */
	 (pCD->clientState != MINIMIZED_STATE)) ||
	((pNextEntry->type == MINIMIZED_STATE) &&
	 (pCD->clientState == MINIMIZED_STATE)))
    {
	if (pCD->clientState == MINIMIZED_STATE)
	{
	    windowType = F_GROUP_ICON;
	}
	else
	{
	    if (focusNext)
	    {
		focused = FocusNextTransient (pCD, type, True, focusTime);
	    }
	    else
	    {
		focused = FocusPrevTransient (pCD, type, True, focusTime);
	    }
	    windowType = F_GROUP_WINDOW;
	    if (pCD->transientLeader || pCD->transientChildren)
	    {
		windowType |= F_GROUP_TRANSIENT;
	    }
	}

	if (!focused && (type & windowType))
	{
	    focused = True;
	    if (focusNext && wmGD.keyboardFocus &&
		wmGD.keyboardFocus->focusAutoRaise)
	    {
		F_Lower (NULL, wmGD.keyboardFocus, (XEvent *)NULL);
	    }
	    Do_Focus_Key (pCD, focusTime, ALWAYS_SET_FOCUS);
	}
    }

    return (focused);

} /* END OF FUNCTION CheckForKeyFocus */



/*************************************<->*************************************
 *
 *  RepairFocus ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets the keyboard and colormap focus to a client 
 *  window or icon when the window manager is recovering from a 
 *  configuration action.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Comments:
 *  --------
 *  o we only need to repair keyboard focus policy is "pointer"
 * 
 *************************************<->***********************************/

void RepairFocus (void)
{
    ClientData *pCD;
    Boolean sameScreen;
    XEvent event;


    /*
     * Repair the keyboard and colormap focus based on the policies
     */

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
        (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER))
    {
	/*
	 * Move old enter and leave events and then get the window that
	 * the pointer is currently in.
	 */

	XSync (DISPLAY, False);
	while (XCheckMaskEvent (DISPLAY, EnterWindowMask | LeaveWindowMask,
		   &event))
	{
	}

	pCD = GetClientUnderPointer (&sameScreen);

	/*
         * Set the keyboard focus to the window that currently contains the
         * pointer.
         */

	if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	{
	    /*
	     * This will also set colormap focus if it is CMAP_FOCUS_KEYBOARD.
	     */

	    Do_Focus_Key (pCD, CurrentTime, ALWAYS_SET_FOCUS);
	}
	else if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	{
	    SetColormapFocus (ACTIVE_PSD, pCD);
	}
    }

} /* END OF FUNCTION RepairFocus */



/*************************************<->*************************************
 *
 *  AutoResetKeyFocus (pcdFocus, focusTime)
 *
 *
 *  Description:
 *  -----------
 *  This function resets the keyboard input focus when a window with the
 *  focus is withdrawn or iconified.  The focus is set to the last window
 *  that had the focus.  The focus is not set to an icon.
 *
 *
 *  Inputs:
 *  ------
 *  pcdFocus = pointer to the client data of the window with the focus or
 *	the leader of the transient tree that contains the focus window;
 *	the focus should not be set to the pcdFocus window or subordinates.
 *
 *  focusTime = timestamp to be used in setting the keyboard input focus.
 * 
 *************************************<->***********************************/

void AutoResetKeyFocus (ClientData *pcdNoFocus, Time focusTime)
{
    ClientListEntry *pNextEntry;
    ClientData *pCD;
    ClientData *pcdLastFocus = NULL;
    ClientData *pcdFocus;


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
	        (!(pCD->clientState & UNSEEN_STATE)) &&
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
     * Set the focus if there is a window that is a good candidate for
     * getting the focus.
     */

    if (pcdLastFocus)
    {
	Do_Focus_Key (pcdLastFocus, focusTime, ALWAYS_SET_FOCUS);
    }
    else
    {
	/*
	 * !!! Immediately set the focus indication !!!
	 */

	Do_Focus_Key ((ClientData *)NULL, focusTime, ALWAYS_SET_FOCUS);
    }

} /* END OF FUNCTION AutoResetKeyFocus */
