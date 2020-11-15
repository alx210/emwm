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
#include "WmICCC.h"
#include "WmProtocol.h"


/*
 * include extern functions
 */

#include "WmCDecor.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#include "WmManage.h"
#include "WmProperty.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */


/*
 * Function Declarations:
 */

#include "WmWinState.h"
static void UnmapClients (ClientData *pCD, unsigned int event_mask);
static void SetupWindowStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);



/*
 * Global Variables:
 */
extern int firstTime;


/******************************<->*************************************
 *
 *  SetClientState (pCD, newState, setTime)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to change the state of a client window (between
 *  withdrawn, normal, minimized, maximized).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = This is a pointer to the window data for the window that
 *        is to have its state changed. The fields that are used
 *        are clientState, ...
 *
 *  newState = This is the state that the client window is to be changed to.
 *
 *  setTime = timestamp for state setting operations
 *
 * 
 *  Outputs:
 *  -------
 *  pCD.clientState = new client state
 *
 ******************************<->***********************************/

void SetClientState (ClientData *pCD, int newState, Time setTime)
{
	SetClientStateWithEventMask(pCD, newState, setTime, (unsigned int)0);
} /* END OF FUNCTION SetClientState */

void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask)
{
    ClientData *pcdLeader;
    int currentState;
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);
#ifdef WSM
    Boolean notShowing = (newState & UNSEEN_STATE);
#endif /* WSM */

    currentState = pCD->clientState;
    if (currentState == newState)
    {
	/* no change in state */
	return;
    }


    /*
     * Undo the old state and setup the new state.  If this is a transient
     * window then insure that it is put in a state that is compatible
     * with its transient leader (e.g., it cannot be minimized separately).
     */

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;
#ifdef WSM
    SetClientWsIndex (pCD);
#endif /* WSM */

    if (pCD->transientLeader)
    {
	if ((pcdLeader->clientState == MINIMIZED_STATE) &&
	    (newState != WITHDRAWN_STATE))
	{
	    newState = MINIMIZED_STATE;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	}
	else if ((newState == MINIMIZED_STATE) &&
		 (pcdLeader->clientState != MINIMIZED_STATE))
	{
	    if (currentState == WITHDRAWN_STATE)
	    {
		newState = NORMAL_STATE;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	    }
	    else
	    {
		newState = currentState;
#ifdef WSM
	    if (notShowing)
	    {
		newState |= UNSEEN_STATE;
	    }
#endif /* WSM */
	    }
	}
	if (newState == currentState)
	{
	    return;
	}
    }

    switch (newState)
    {

#ifdef WSM
        case UNSEEN_STATE | WITHDRAWN_STATE:
#else
	case WITHDRAWN_STATE:
#endif /* WSM */
	{
	    /*
	     * Free window manager resources (frame and icon).  The
	     * WM_STATE property is set in WithdrawWindow.
	     */

	    UnManageWindow (pCD);
	    break;
	}

	case NORMAL_STATE:
	case MAXIMIZED_STATE:
	{
	    SetupWindowStateWithEventMask (pCD, newState, setTime, event_mask);
#ifdef WSM
	    XMapWindow (DISPLAY, pCD->client);
	    XMapWindow (DISPLAY, pCD->clientFrameWin);
#endif /* WSM */
	    break;
	}

	case MINIMIZED_STATE:
	{
	    Boolean clientHasFocus;

	    /*
	     * Transient windows are minimized with the rest of the transient
	     * tree, including the transient leader.
	     */

	    if ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE))
	    {
		if ((wmGD.keyboardFocus == pCD) ||
		    (pCD->transientChildren && wmGD.keyboardFocus &&
		     (pCD == FindTransientTreeLeader (wmGD.keyboardFocus))))
		{
		    clientHasFocus = True;
		}
		else
		{
		    clientHasFocus = False;
		}

		if (clientHasFocus ||
		  ((wmGD.nextKeyboardFocus == pCD) ||
		   (pCD->transientChildren && wmGD.keyboardFocus &&
		    (pCD == FindTransientTreeLeader (wmGD.nextKeyboardFocus)))))
	    	{
		    /*
		     * Give up the keyboard focus when minimized (including
		     * the case in which an associated transient window has
		     * the focus).  Immediately remove the focus indication
		     * from the window being minimized.
		     */

		    if (wmGD.autoKeyFocus &&
			(wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
		    {
			AutoResetKeyFocus (pcdLeader, setTime);
		    }
		    else
		    {
		        Do_Focus_Key (NULL, setTime, 
				ALWAYS_SET_FOCUS | WORKSPACE_IF_NULL);
		    }

		    if (clientHasFocus)
		    {
			SetKeyboardFocus (NULL, 0);
		    }
		}

		/* unmap main client and all transients */
		UnmapClients (pCD, event_mask);
	    }

	    /*
	     * Display the icon for the minimized client.
	     */

	    if (ICON_FRAME_WIN(pCD)) 
	    {
#ifdef WSM
		if (pCD->clientState & UNSEEN_STATE)
		{
		    if (pCD->iconWindow)
		    {
			XMapWindow (DISPLAY, pCD->iconWindow);
		    }
		    XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
		}

		ShowAllIconsForMinimizedClient (pCD);
#else /* WSM */
		ShowIconForMinimizedClient (pSD->pActiveWS, pCD);
#endif /* WSM */
	    }

	    SetClientWMState (pCD, IconicState, MINIMIZED_STATE);

	    if ((pSD->useIconBox) && P_ICON_BOX(pCD))
	    {
		if ((pCD->clientFlags & ICON_BOX) && ACTIVE_ICON_TEXT_WIN)
		{
		    /*
		     * Hide active icon text window and reparent it to
		     * root
		     */
		    HideActiveIconText((WmScreenData *)NULL);
		    pSD->activeLabelParent = ACTIVE_ROOT;
		    XReparentWindow(DISPLAY, ACTIVE_ICON_TEXT_WIN , 
				ACTIVE_ROOT, 0, 0 );
		}
		if (ICON_FRAME_WIN(pCD))
		{
		    /* 
		     * force icon appearance in icon box to change 
		     */
		    IconExposureProc (pCD, True);
		}
	    }
	    break;
	}

#ifdef WSM 

        case UNSEEN_STATE | NORMAL_STATE:
        case UNSEEN_STATE | MAXIMIZED_STATE:
        case UNSEEN_STATE | MINIMIZED_STATE:
	{
	    if (wmGD.keyboardFocus == pCD)
	    {
		/*
		 * Give up the keyboard focus 
		 */
		Do_Focus_Key ((ClientData *)NULL, 
			CurrentTime, ALWAYS_SET_FOCUS);
		SetKeyboardFocus (NULL, 0);
	    }

	    if (!(pCD->clientState & UNSEEN_STATE) &&
		 (((pCD->clientState & ~UNSEEN_STATE) == NORMAL_STATE) ||
		  ((pCD->clientState & ~UNSEEN_STATE) == MAXIMIZED_STATE)))
	    {
		/* unmap main client and all transients */
		UnmapClients (pcdLeader, event_mask);

	    }
      
	    if (pCD->clientFrameWin) 
	    {
		if (!P_ICON_BOX(pCD))
		{
		    if (ICON_FRAME_WIN(pCD))
		    {
			XUnmapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
		    }
		    if (pCD->iconWindow)
			XUnmapWindow (DISPLAY, pCD->iconWindow);
		} 
	    }

	    switch (newState & ~UNSEEN_STATE)
	    {
	    case MINIMIZED_STATE:
		SetClientWMState (pCD, IconicState, newState);
		break;

	    case NORMAL_STATE:
	    case MAXIMIZED_STATE:
	    default:
		SetClientWMState (pCD, NormalState, newState);
		break;
	    }
        }
	break;
#endif /* WSM */
    }

} /* END OF FUNCTION SetClientStateWithEventMask */



/*************************************<->*************************************
 *
 *  SetupWindowStateWithEventMask (pCD, newState, setTime, event_mask)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to setup a client window in the Normal or Maximized
 *  state.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = This is a pointer to the window data for the window that
 *        is to have its state changed.
 *
 *  newState = This is the state that the client window is to be changed to.
 *
 *  setTime = timestamp for state setting operations
 *
 *  event_mask = what to grab to prevent stray events going somewhere
 * 
 *  Outputs:
 *  -------
 *  pCD.clientState = new client state
 *
 *************************************<->***********************************/

static void SetupWindowStateWithEventMask (ClientData *pCD, int newState, 
	Time setTime, unsigned int event_mask)
{
    int currentState;
#ifdef WSM
    int wsI, iplace;
    WmWorkspaceData *pWS_i;
#else /* WSM */
    WmWorkspaceData *pWS = PSD_FOR_CLIENT(pCD)->pActiveWS;
#endif /* WSM */
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);

    currentState = pCD->clientState;

    /*
     * A transient window is not restored or maximized if the transient leader
     * is minimized.
     */

    if (newState == NORMAL_STATE)
    {
	if (pCD->maxConfig == True)
	{
	    /*
	     * The configuration function uses maxConfig to determine
	     * what the current configuration is (and then resets
	     * maxConfig) and uses the state paramenter to determine
	     * what the new configuration is.
	     */

	    ConfigureNewState (pCD); 
	}
    }
    else /* MAXIMIZED_STATE */
    {
	if (pCD->maxConfig == False)
	{
	    ConfigureNewState (pCD); 
        }
    }

    if (currentState == MINIMIZED_STATE)
    {
	Boolean clearIconFocus;

	/*
	 * give up keyboard focus 
	 */

	if ((wmGD.keyboardFocus == pCD) ||
	    (wmGD.nextKeyboardFocus == pCD))
	{
	    Do_Focus_Key (NULL, setTime, ALWAYS_SET_FOCUS | WORKSPACE_IF_NULL);
	}

	if (wmGD.keyboardFocus == pCD)
	{
	    clearIconFocus = True;
	}
	else
	{
	    clearIconFocus = False;
	}

	/*
	 * The wm icon frame window and the client icon window
	 * (if it is being used) are mapped and the client window and
	 * client frame are unmapped.
	 */

	if (ICON_FRAME_WIN(pCD))
	{
	    if (pSD->useIconBox && P_ICON_BOX(pCD) && 
		!(pCD->clientFlags & ICON_BOX))
	    {
	        ShowClientIconState(pCD, newState);
	    }
	    else 
	    {
		Boolean doGrab = False;
    		if (event_mask)
		doGrab = (Success == XGrabPointer 
			(DISPLAY, DefaultRootWindow(DISPLAY),
			False, event_mask, GrabModeAsync, GrabModeAsync,
			None, None, CurrentTime));
	        XUnmapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
	        if (pCD->iconWindow)
	        {
		    XUnmapWindow (DISPLAY, pCD->iconWindow);
	        }
    		if (event_mask && doGrab)
		{
			XEvent event;
			XMaskEvent(DISPLAY, event_mask, &event);
			XUngrabPointer(DISPLAY,CurrentTime);
		}
#ifdef WSM
	        if (wmGD.iconAutoPlace) 
	        {
                    for (wsI = 0; wsI < pCD->numInhabited; wsI++)
		    {
			iplace = pCD->pWsList[wsI].iconPlace;
			if (iplace != NO_ICON_PLACE)
			{
			    pWS_i = GetWorkspaceData (pCD->pSD,
						pCD->pWsList[wsI].wsID);
			    pWS_i->IPData.placeList[iplace].pCD = 
				    NULL;
			}
		    }
	        }
#else /* WSM */
	        if ((wmGD.iconAutoPlace) && (ICON_PLACE(pCD) != NO_ICON_PLACE))
	        {
		    pWS->IPData.placeList[ICON_PLACE(pCD)].pCD = 
			NULL;
	        }
#endif /* WSM */
	    }

	    if (clearIconFocus)
	    {
		ClearFocusIndication (pCD, False /*no refresh*/);
		wmGD.keyboardFocus = NULL;
	    }
	}
    }
    if ((currentState != NORMAL_STATE) && (currentState != MAXIMIZED_STATE))
    {
	/*
	 * Note that maximized state is considered a NormalState in
	 * the ICCC.  SetClientWMState also sets the state in the
	 * client data.
	 */

	if (currentState == MINIMIZED_STATE)
	{
	    /*
	     * Raise the window(s) when they are deiconified.
	     */

	    pCD->clientState = newState;
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = True;
#endif /* WSM */
	    F_Raise (NULL, pCD, NULL);
#ifdef WSM
		    wmGD.bSuspendSecondaryRestack = False;
#endif /* WSM */
	}

	if ( (!(pCD->clientFlags & ICON_BOX)) || 
	     ((pCD->clientFlags & ICON_BOX) && (!(firstTime))) )
	{
	    MapClientWindows (pCD);
	}


	/*
	 * Set the WM_STATE property of the window and any associated
	 * transients, along with the clientState value.  The call
	 * is made with an indication of NORMAL_STATE to insure
	 * that transient window clientState values are setup
	 * correctly.  The top-level window clientState is set later.
	 */

	SetClientWMState (pCD, NormalState, NORMAL_STATE);
    }
    pCD->clientState = newState;

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	(currentState == MINIMIZED_STATE) && wmGD.deiconifyKeyFocus)
    {
	ClientData *pcdFocus;

	pcdFocus = FindTransientFocus (pCD);
	if (pcdFocus)
	{
	    Do_Focus_Key (pcdFocus, setTime, ALWAYS_SET_FOCUS);
	}
    }

    if ( pSD->useIconBox &&  P_ICON_BOX(pCD) &&
	 (!(pCD->clientFlags & ICON_BOX)) && (ICON_FRAME_WIN(pCD)))
    {
	/* 
	 * force icon appearance in icon box to change 
	 */

	IconExposureProc (pCD, True);
    }

} /* END OF FUNCTION SetupWindowStateWithEventMask */




/*************************************<->*************************************
 *
 *  ConfigureNewState (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Configure the window to a new state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This is only good for going between NORMAL and MAXIMIZED state.
 * 
 *************************************<->***********************************/

void ConfigureNewState (ClientData *pcd)
{
    if (pcd->maxConfig)
    {
	pcd->maxConfig = FALSE;
	RegenerateClientFrame(pcd);
	XResizeWindow (DISPLAY, pcd->client,
			   (unsigned int) pcd->clientWidth, 
			   (unsigned int) pcd->clientHeight);
    }
    else
    {
	XResizeWindow (DISPLAY, pcd->client,
			   (unsigned int) pcd->maxWidth, 
			   (unsigned int) pcd->maxHeight);
	pcd->maxConfig = TRUE;
	RegenerateClientFrame(pcd);
    }
    SendConfigureNotify (pcd);

    /*
     * Force repaint if size doesn't change to update frame appearance.
     */

    if ((pcd->clientWidth == pcd->maxWidth) &&
        (pcd->clientHeight == pcd->maxHeight))
    {
	FrameExposureProc (pcd);
    }

} /* END OF FUNCTION ConfigureNewState */



/*************************************<->*************************************
 *
 *  UnmapClients (pCD, event_mask)
 *
 *
 *  Description:
 *  -----------
 *  Unmap the window(s).  The indicated client may be the head of a transient
 *  tree - if it is unmap all windows in the transient tree.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of window(s) to be unmapped
 *  event_mask = what to grab to prevent stray events going somewhere. Our
 *	passive grab has just been activated -- but it is dropped when the
 * 	window is unmapped and the ButtonRelease event can go to the window
 *	now exposed. Avoid this by grabbing the ButtonRelease before the unmap
 *	and swallowing it.
 *	Also done for icon being unmapped.
 *
 *************************************<->***********************************/

static void UnmapClients (ClientData *pCD, unsigned int event_mask)
{
    ClientData *pNext;
    Boolean doGrab = False;

    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* unmap all children first */
	if (pNext->transientChildren)
	    UnmapClients (pNext, (unsigned int) 0);

	/* then unmap all siblings at this level */
	XUnmapWindow (DISPLAY, pNext->clientFrameWin);
	XUnmapWindow (DISPLAY, pNext->client);
	pNext->wmUnmapCount++;
	pNext = pNext->transientSiblings;
    }

    if (event_mask)
	doGrab = (Success == XGrabPointer (DISPLAY, DefaultRootWindow(DISPLAY),
		False, event_mask, GrabModeAsync, GrabModeAsync,
		None, None, CurrentTime));
    /* unmap this primary window */
    XUnmapWindow (DISPLAY, pCD->clientFrameWin); 
    XUnmapWindow (DISPLAY, pCD->client);
    if (event_mask && doGrab)
	{
	XEvent event;
	XMaskEvent(DISPLAY, event_mask, &event);
	XUngrabPointer(DISPLAY,CurrentTime);
	}
    pCD->wmUnmapCount++;

} /* END OF FUNCTION UnmapClients */



/*************************************<->*************************************
 *
 *  SetClientWMState (pCD, wmState, mwmState)
 *
 *
 *  Description:
 *  -----------
 *  Set a new window manage state for a client window or a tree of transient
 *  client windows.
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *
 *  wmState = new state for WM_STATE property
 *
 *  mwmState = mwm client state
 *
 *************************************<->***********************************/

void SetClientWMState (ClientData *pCD, int wmState, int mwmState)
{
    ClientData *pNext;
#ifdef WSM
    Boolean bToUnseen;

    bToUnseen = (mwmState & UNSEEN_STATE) != 0;
    mwmState &= ~UNSEEN_STATE;
#endif /* WSM */

#ifdef WSM
    SetClientWsIndex (pCD);
#endif /* WSM */
    pNext = pCD->transientChildren;
    while (pNext)
    {
	if (pNext->transientChildren)
	{
	    SetClientWMState (pNext, wmState, mwmState);
	}

#ifdef WSM
        SetClientWsIndex (pNext);
#endif /* WSM */
	SetWMState (pNext->client, wmState, ICON_FRAME_WIN(pNext));
	if (pNext->maxConfig && mwmState == NORMAL_STATE)
	{
	    pNext->clientState = MAXIMIZED_STATE;
	}
#ifdef WSM
	else if (!pNext->maxConfig && mwmState == MAXIMIZED_STATE)
	{
	    pNext->clientState = NORMAL_STATE;
	}
#endif /* WSM */
	else
	{
	    pNext->clientState = mwmState;
	}
#ifdef WSM
	if (bToUnseen)
	    pNext->clientState |= UNSEEN_STATE;
#endif /* WSM */
	pNext = pNext->transientSiblings;
    }

    SetWMState (pCD->client, wmState, ICON_FRAME_WIN(pCD));
    pCD->clientState = mwmState;
#ifdef WSM
    if (bToUnseen)
	pCD->clientState |= UNSEEN_STATE;
#endif /* WSM */

} /* END OF FUNCTION SetClientWMState */


/*************************************<->*************************************
 *
 *  MapClientWindows (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Maps the window.  If this is a transient tree then all the windows in
 *  the transient tree are mapped.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 * 
 *************************************<->***********************************/

void MapClientWindows (ClientData *pCD)
{
    ClientData *pNext;


    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* map all transient children first */
	if (pNext->transientChildren)
	{
	    MapClientWindows (pNext);
	}

	/* then map all siblings at this level */
	XMapWindow (DISPLAY, pNext->client);
	XMapWindow (DISPLAY, pNext->clientFrameWin);

	pNext = pNext->transientSiblings;
    }

    /* map the primary window */
    XMapWindow (DISPLAY, pCD->client);
    XMapWindow (DISPLAY, pCD->clientFrameWin);

} /* END OF FUNCTION MapClientWindows */



/*************************************<->*************************************
 *
 *  ShowIconForMinimizedClient (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function shows the icon for the specified client.  If the icon
 *  is in an icon box then the "minimized" icon is displayed.  If the icon
 *  is on the root window it is mapped.
 * 
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD	= pointer to  client data
 *
 *************************************<->***********************************/

void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD)
{
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);

    /*
     * Handle auto-placement for root icons (icons not in an icon
     * box).
     */
    if (wmGD.iconAutoPlace && !P_ICON_BOX(pCD))
    {
        if ((ICON_PLACE(pCD) == NO_ICON_PLACE) ||
	    ((pWS->IPData.placeList[ICON_PLACE(pCD)].pCD) &&
	     (pWS->IPData.placeList[ICON_PLACE(pCD)].pCD != pCD)))
        {
            /*
             * Icon place not defined or occupied by another client,
	     * find a free place to put the icon.
             */

	    if ((ICON_PLACE(pCD) = GetNextIconPlace (&pWS->IPData)) 
		== NO_ICON_PLACE)
	    {
	        ICON_PLACE(pCD) = 
		    CvtIconPositionToPlace (&pWS->IPData,
							 pCD->clientX,
			               	                 pCD->clientY);
	    }
	    CvtIconPlaceToPosition (&pWS->IPData, ICON_PLACE(pCD), 
				    &ICON_X(pCD), &ICON_Y(pCD));

#ifndef WSM
	    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pCD), 
		ICON_X(pCD), ICON_Y(pCD));
#endif /* WSM */

        }

        pWS->IPData.placeList[ICON_PLACE(pCD)].pCD = pCD;
    }

#ifdef WSM
    /*
     * If icon on root window and this workspace is active, the
     * make sure it's in the right place.
     */
    if ((pWS == pSD->pActiveWS) && !P_ICON_BOX(pCD))
    {
	XMoveWindow (DISPLAY, ICON_FRAME_WIN(pCD), 
	    ICON_X(pCD), ICON_Y(pCD));
    }
#endif /* WSM */
    if (pCD->iconWindow)
    {
        XMapWindow (DISPLAY, pCD->iconWindow);
    }

    if ((pSD->useIconBox) && P_ICON_BOX(pCD))
    {
        ShowClientIconState (pCD, MINIMIZED_STATE );
    }
    else
    {
	XWindowChanges windowChanges;

	/*
	 * Map the icon on the screen at the appropriate place in the 
	 * window stack.
	 */

	if (wmGD.lowerOnIconify)
	{
	    if ((&pCD->iconEntry != pSD->lastClient) &&
		(pSD->lastClient))
	    {
		if (pSD->lastClient->type == MINIMIZED_STATE)
		{
		    windowChanges.sibling = 
			ICON_FRAME_WIN(pSD->lastClient->pCD);
		}
		else
		{
		    windowChanges.sibling =
			pSD->lastClient->pCD->clientFrameWin;
		}
		windowChanges.stack_mode = Below;
		XConfigureWindow (DISPLAY, ICON_FRAME_WIN(pCD),
		    		  (CWSibling | CWStackMode), &windowChanges);
		MoveEntryInList (pWS, &pCD->iconEntry, 
		    False /*on bottom*/, NULL);
	    }
	}
	else
	{
	    windowChanges.sibling = pCD->clientFrameWin;
	    windowChanges.stack_mode = Below;
	    XConfigureWindow (DISPLAY, ICON_FRAME_WIN(pCD),
	    		      (CWSibling | CWStackMode), &windowChanges);
	    MoveEntryInList (pWS, &pCD->iconEntry, False /*below*/,
			     &pCD->clientEntry);
	}

#ifdef WSM
        if (pWS == pSD->pActiveWS)
	{
	    XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
	}
#else /* WSM */
	XMapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
#endif /* WSM */
    }

} /* END OF FUNCTION ShowIconForMinimizedClient */

#ifdef WSM

/*************************************<->*************************************
 *
 *  ShowAllIconsForMinimizedClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function places icons in all the workspaces for the minimized
 *  client. Since there is only one clientState per client (not per
 *  workspace), this loops over all workspace in which the client
 *  resides and places an icon in each.
 * 
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to  client data
 *
 *  Comments:
 *  ---------
 *  This operates by setting up the currentWsc index for each workspace
 *  and calling ShowIconForMinimizedClient, which makes heavy use of
 *  the macros that use the currentWsc index.
 *
 *************************************<->***********************************/

void ShowAllIconsForMinimizedClient (ClientData *pCD)
{
    int saveWsc = pCD->currentWsc;
    int tmpWsc;
    WmWorkspaceData *pWS;

    for (tmpWsc = 0; tmpWsc < pCD->numInhabited; tmpWsc++)
    {
	pCD->currentWsc = tmpWsc;
	pWS = GetWorkspaceData (PSD_FOR_CLIENT(pCD),
				    pCD->pWsList[tmpWsc].wsID);
	ShowIconForMinimizedClient(pWS, pCD);
    }
    
    pCD->currentWsc = saveWsc;

} /* END OF FUNCTION ShowAllIconsForMinimizedClient */
#endif /* WSM */

