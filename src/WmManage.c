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
/*
 * include extern functions
 */
#include "WmCDecor.h"
#include "WmCEvent.h"
#include "WmColormap.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFunction.h"
#include "WmGraphics.h"
#include "WmIDecor.h"
#include "WmIconBox.h"
#include "WmImage.h"
#include "WmKeyFocus.h"
#include "WmManage.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmProtocol.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#include "WmPresence.h"
#include "WmWrkspace.h"
#include "WmXSMP.h"


/*************************************<->*************************************
 *
 *  AdoptInitialClients (pSD)
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to screen data
 *
 *
 *  Description:
 *  -----------
 *  This function is called to find client windows that were mapped prior to 
 *  starting (or restarting) the window manager.  These windows are included
 *  in the set of windows managed by the window manager.
 *
 *************************************<->***********************************/

void AdoptInitialClients (WmScreenData *pSD)
{
    Window  root;
    Window  parent;
    Window *clients;
    int nAncillaries, iAnc;
    Window *pAncillaryWindows, *pWin1;
    WmWorkspaceData *pWS0;
    unsigned int     nclients;
    ClientData *pcd = NULL;
    PropWMState *wmStateProp;
    Boolean manageOnRestart;
    int i,j;
    long manageFlags;

    /* 
     * Generate list of ancillary windows (not to be managed)
     */
    nAncillaries = 2 + pSD->numWorkspaces;
    pAncillaryWindows = (Window *) XtMalloc (sizeof(Window)*(nAncillaries));
    if (!pAncillaryWindows)
    {
	Warning (((char *)GETMESSAGE(46, 1, "Insufficient memory to adopt initial clients")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }
    pWS0 = pSD->pWS;
    pWin1 = pAncillaryWindows;
    for (iAnc = 0; iAnc < pSD->numWorkspaces; iAnc++)
    {
	*pWin1 = XtWindow((pWS0)->workspaceTopLevelW);
	pWin1++;
	pWS0++;
    }
    *pWin1++ = XtWindow (pSD->screenTopLevelW);
    *pWin1 = pSD->activeIconTextWin;


    /*
     * Look for mapped top-level windows and start managing them:
     */

    if (XQueryTree (DISPLAY, pSD->rootWindow, &root, &parent, &clients,
	    &nclients))
    {
#ifndef DONT_FILTER_ICON_WINDOWS
	/*
	 * Filter out icon windows so they don't get managed as a client
	 * window.  Icon windows will be process in SetupClientIconWindow().
	 */
	XWMHints *tmphint;

	for (i = 0; i < nclients; i++) {
	    if (clients[i]) {
		if ((tmphint = XGetWMHints (DISPLAY, clients[i])) != NULL) {
		    if (tmphint->flags & IconWindowHint) {
			for (j = 0; j < nclients; j++) {
			    if (clients[j] == tmphint->icon_window) {
				clients[j] = None;
				break;
			    }
			}
		    }
		    XFree ((char *) tmphint);
		}
	    }
	}
#endif /* DONT_FILTER_ICON_WINDOWS */

	for (i = 0; i < nclients; i++)
	{
	    /* determine if the client window should be managed by wm */

        if (InWindowList (clients[i], pAncillaryWindows, nAncillaries))
        {
		/* don't manage ancillary window manager windows */
                continue;
	    }
	    if (!XFindContext (DISPLAY, clients[i], wmGD.windowContextType,
	        (caddr_t *)&pcd)) 
	    {
		/* don't manage a window we've already established a 
		   context for (e.g. icon windows) */
		continue;
	    }
	    if (!WmGetWindowAttributes (clients[i]))
            {
		/* can't access the window; ignore it */
		continue;
            }
	    /* window attributes are put into the global cache */

	    /*
	     * Get the window WM_STATE property value to determine the
	     * initial window state if the wm is being restarted.
	     */

	    manageFlags = MANAGEW_WM_STARTUP;
	    manageOnRestart = True;

	    if (wmGD.wmRestarted)
	    {
		manageFlags |= MANAGEW_WM_RESTART;
		if ((wmStateProp = GetWMState (clients[i])) != NULL)
		{
		    if (wmStateProp->state == IconicState)
		    {
			manageFlags |= MANAGEW_WM_RESTART_ICON;
		    }
		    else if (wmStateProp->state != NormalState)
		    {
			manageOnRestart = False;
		    }
		    XFree ((char *)wmStateProp);
		}
		else 
		{
		    manageOnRestart = False;
		}
	    }

	    /*
	     * Don't manage any override_redirect windows (mapped or not).
	     * Manage an unmapped window if it has a WM_STATE property
	     *   and it is not Withdrawn.
	     * Manage any window that is mapped.
	     */

	    if ((wmGD.windowAttributes.override_redirect != True) &&
		((wmGD.wmRestarted && manageOnRestart) ||
		 (wmGD.windowAttributes.map_state != IsUnmapped)))
	    {
		ManageWindow (pSD, clients[i], manageFlags);
	    }
	}

	if (nclients)
	{
	    XFree ((char *)clients);
	}
    }

    if (pAncillaryWindows)
    {
	XtFree ((char *) pAncillaryWindows);
    }

} /* END OF FUNCTION AdoptInitialClients */



/*************************************<->*************************************
 *
 *  ManageWindow (pSD, clientWindow, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This is the highlevel function that is used to include a window in
 *  the set of windows that are managed by the window manager.  The window
 *  gets reparented and decorated, gets an icon, is setup for window
 *  management event handling, etc.  Client windows that are controlled
 *  by the window manager (e.g., the icon box) are also managed with
 *  this function.
 *
 *
 *  Inputs:
 *  ------
 *  clientWindow = window of the client that we should manage
 *
 *  manageFlags	= additional control information 
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = initialized client data
 *
 *************************************<->***********************************/

void 
ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags)
{
    ClientData *pCD;
    int initialState;
    int i;
    Boolean sendConfigNotify;
    WmWorkspaceData *pwsi;

    /*
     * Get client information including window attributes and window
     * property values.  Use this information to determine how the window
     * is to be managed.
     */

    if (!(pCD = GetClientInfo (pSD, clientWindow, manageFlags)))
    {
	/* error getting client info; do not manage the client window */
	return;
    }

    if (pCD->inputMode == MWM_INPUT_SYSTEM_MODAL)
    {
	/*
	 * Put system modal windows in all workspaces to
	 * avoid the race condition of the window coming up
	 * just as the user switches workspaces.
	 */
	pCD->dtwmFunctions |= DtWM_FUNCTION_OCCUPY_WS;
	F_AddToAllWorkspaces(0, pCD, 0);
	pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
    }

    if (manageFlags & MANAGEW_WM_RESTART)
    {
	if (manageFlags & MANAGEW_WM_RESTART_ICON)
	{
	    pCD->clientState = MINIMIZED_STATE;
	}
	else
	{
	    pCD->clientState = NORMAL_STATE;
	}
    }


    /*
     * Setup the initial placement of the client window.  Do interactive
     * placement if configured.
     */

    sendConfigNotify = InitClientPlacement (pCD, manageFlags);


    /*
     * Make a window frame for the client window and reparent the client
     * window.
     */

    if (!FrameWindow (pCD))
    {
	/*
	 * Error in framing the window; clean up the wm resources made
	 * up to this point for the client window. Do not manage the
	 * client window.
	 */

	UnManageWindow (pCD);
	return;
    }

    /*
     * Send config notify if the client's been moved/resized
     */
    if (sendConfigNotify)
    {
	SendConfigureNotify (pCD);
    }

    /*
     * Send client offset message if:
     *
     *   1. The client is interested.
     *   2. The position we report to the user is not the client's real
     *      position.
     *   3. There is a client offset to report.
     */
    if ((pCD->protocolFlags & PROTOCOL_MWM_OFFSET) &&
	((pCD->clientOffset.x != 0) || (pCD->clientOffset.y != 0)))
    { 
	SendClientOffsetMessage (pCD);
    }

    /*
     * Make an icon for the client window if it is not a valid transient
     * window.
     */
    if ((pCD->clientFunctions & MWM_FUNC_MINIMIZE) &&
	(pCD->transientLeader == NULL))
    {
	/* 
	 * Make icons frames 
	 * Only make one icon frame for root icons.
	 * Make one per workspace for icon box icons.
	 */
	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if( (pwsi = GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID)) )
	    {

		if ((pCD->pSD->useIconBox && 
                     !(manageFlags & MANAGEW_WM_CLIENTS)) || (i == 0))
		{
		    /*
		     *   Make icon inside an icon box for non-root case
		     */
		    if (!MakeIcon (pwsi, pCD)) 
		    { 
			/*
			 * Error in making an icon for the client window; 
			 * clean up the wm resources; do not manage the 
			 * client window.
			 */

			UnManageWindow (pCD);
			return;
		    }
		    else 
		    {
			XSaveContext (DISPLAY, pCD->pWsList[i].iconFrameWin, 
				wmGD.windowContextType, (caddr_t)pCD);

			if (pCD->iconWindow && pCD->pWsList[i].iconFrameWin)
			{
			    XGrabButton (DISPLAY, AnyButton, AnyModifier, 
				pCD->pWsList[i].iconFrameWin, True,
				ButtonPressMask|ButtonReleaseMask|
				    ButtonMotionMask,
				GrabModeAsync, GrabModeAsync, None, 
				wmGD.workspaceCursor);
			}
		    }
		}
		else 
		{
		    /* 
		     *  Make root icons for a client
		     */
 		    if ((pCD->clientFunctions & MWM_FUNC_MINIMIZE) &&
 			(pCD->transientLeader == NULL))
 		    {
 			if ((i == 0) &&
 			    (!MakeIcon (pwsi, pCD)))
 			{
 			    /*
 			     * Error in making an icon for the client 
 			     * window; clean up the wm resources; do 
 			     * not manage the client window.
 			     */
 
 			    UnManageWindow (pCD);
 			    return;
 			}
 			else
 			{
 			    /* copy root icon frame reference to other 
 			     * workspaces
			     */
 			    pCD->pWsList[i].iconFrameWin = 
 				    pCD->pWsList[0].iconFrameWin;
 			}
 		    }
		}
	    }
	}
    }

    /*
     * Register window contexts to facilitate event handling:
     */

    XSaveContext (DISPLAY, pCD->clientFrameWin, wmGD.windowContextType,
	(caddr_t)pCD);

    XSaveContext (DISPLAY, pCD->clientBaseWin, wmGD.windowContextType,
	(caddr_t)pCD);

    if (DECOUPLE_TITLE_APPEARANCE(pCD) && pCD->clientTitleWin)
    {
	/* 
	 * handle exposures on title bar if it has its own appearance
	 */
	XSaveContext (DISPLAY, pCD->clientTitleWin, wmGD.windowContextType,
	    (caddr_t)pCD);
    }

    if (pCD->clientCmapCount > 0)
    {
	for (i = 0; i < pCD->clientCmapCount; i++)
	{
	    if (pCD->cmapWindows[i] != pCD->client)
	    {
			AddColormapWindowReference(pCD, pCD->cmapWindows[i]);
	    }
	}
    }

    pCD->clientFlags |= CLIENT_CONTEXT_SAVED;


    /*
     * Setup button binding handling for actions that apply to the client
     * window.
     */

    if (BUTTON_SPECS(pCD))
    {
	SetupCButtonBindings (pCD->clientBaseWin, BUTTON_SPECS(pCD));
    }

    /*
     * Setup key binding handling for system menu accelerators.
     */

    if (pCD->systemMenuSpec &&
        (pCD->systemMenuSpec->accelKeySpecs))
    {
	SetupKeyBindings (pCD->systemMenuSpec->accelKeySpecs,
			  pCD->clientFrameWin, GrabModeSync, F_CONTEXT_ALL);

	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if (!pCD->pWsList[i].pIconBox && pCD->pWsList[i].iconFrameWin)
	    {
		SetupKeyBindings (pCD->systemMenuSpec->accelKeySpecs,
			      pCD->pWsList[i].iconFrameWin, GrabModeSync, 
			      F_CONTEXT_ALL);
	    }
	}
    }

  for (i = 0; i < pCD->numInhabited; i++)
  {
    if (!pCD->pWsList[i].pIconBox && pCD->pWsList[i].iconFrameWin)
    {
	static int iconKeySpec = 1;
	static int iconAccelSpec = 1;

        if ((iconKeySpec != 0) && KEY_SPECS(pCD))
        {
	    iconKeySpec = SetupKeyBindings (KEY_SPECS(pCD), 
				pCD->pWsList[i].iconFrameWin,
				GrabModeSync, F_CONTEXT_ICON);
        }

        if ((iconAccelSpec != 0) && ACCELERATOR_MENU_COUNT(pCD))
        {
	    int n;

	    iconAccelSpec = 0;
	    for (n= 0; n < pSD->acceleratorMenuCount; n++)
	    {
	        iconAccelSpec += SetupKeyBindings (
			    ACCELERATOR_MENU_SPECS(pCD)[n]->accelKeySpecs,
			    pCD->pWsList[i].iconFrameWin, GrabModeSync,
			    F_CONTEXT_ICON);
	    }
	}
    }
  }

    /*
     * Setup keyboard focus handling if policy is "explicit".
     */

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	DoExplicitSelectGrab (pCD->clientBaseWin);
    }

    UpdateWorkspacePresenceProperty(pCD);


    /*
     * Make sure the client window has been reparented ...
     */

    if (!(manageFlags & MANAGEW_WM_CLIENTS))
    {
        XSync (DISPLAY, False);

        if (pCD->clientFlags & CLIENT_DESTROYED)
        {
	    UnManageWindow (pCD);
	    return;
        }
    }

    /*
     * Setup the initial display state for the client window:
     */

    initialState = pCD->clientState;

    if (!ClientInWorkspace (pSD->pActiveWS, pCD))
    {
	initialState |= UNSEEN_STATE;
    }

    pCD->clientState = WITHDRAWN_STATE;
    pCD->clientFlags &= ~WM_INITIALIZATION;

    /* 
     * Add to stacking list using the client's zero'th workspace
     * instead of the current one because it may not be in 
     * the current one.
     */
    AddClientToList (GetWorkspaceData (pSD, pCD->pWsList[0].wsID),
	pCD, True /*on top*/);
    SetClientState (pCD, initialState, GetTimestamp());

    /*
     * Set the keyboard input focus to the newly managed window if appropriate:
     * - focus is automatically set only if the focus policy is explicit
     * - if there is a system modal window active then set the focus only
     *   if the new window is in the system modal heirarchy
     * - focus is automatically set if startupKeyFocus is selected or
     *   the new window is a system modal window or the current focus window
     *   has the new window as an application modal subordinate
     * - don't automatically set the focus if the window is minimized or
     *   is a window that generally doesn't take input
     */

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	((pCD->inputMode == MWM_INPUT_SYSTEM_MODAL) ||
	 ((!wmGD.systemModalActive ||
	   (wmGD.systemModalClient == FindTransientTreeLeader (pCD))) &&
	  (wmGD.startupKeyFocus ||
	   (wmGD.keyboardFocus && (IS_APP_MODALIZED(wmGD.keyboardFocus)))) &&
	  !(manageFlags &
	    (MANAGEW_WM_STARTUP | MANAGEW_WM_RESTART | MANAGEW_WM_CLIENTS)) &&
	  (pCD->clientState != MINIMIZED_STATE) &&
	  !(pCD->clientState & UNSEEN_STATE) && (pCD->inputFocusModel ||
	   (pCD->protocolFlags & PROTOCOL_WM_TAKE_FOCUS)))))
    {
	Do_Focus_Key (pCD, GetTimestamp() , ALWAYS_SET_FOCUS);
    }
    else if ((pCD->inputMode == MWM_INPUT_SYSTEM_MODAL) ||
	     (wmGD.keyboardFocus && IS_APP_MODALIZED(wmGD.keyboardFocus)))
    {
	Do_Focus_Key ((ClientData *)NULL, GetTimestamp() , ALWAYS_SET_FOCUS);
    }

    /*
     * Free the initial property list. This will force
     * reads of properties that change after the initial
     * management (see HasProperty() function.)
     */
    DiscardInitialPropertyList (pCD);

} /* END OF FUNCTION ManageWindow */



/*************************************<->*************************************
 *
 *  UnManageWindow (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a top-level client window and it's transients
 *  from the set of windows that is managed by the window manager.  
 *
 *
 *  Inputs:
 *  ------
 *  pCD 	- pointer to client data of window to unmanage
 *
 *************************************<->***********************************/

void UnManageWindow (ClientData *pCD)
{
    /*
     * Withdraw all the transient children of this window.
     */

    if (pCD->transientChildren != NULL) 
    {
	WithdrawTransientChildren (pCD);
    }


    /*
     * If this is a transient window, then delete it from the leader's
     * list of transients.
     */

    if (pCD->transientLeader)
    {
	DeleteTransient (pCD);

        /* If this was a modal dialog box, then replay the event. */
        if ( wmGD.replayEnterEvent )
          {
            XPutBackEvent( DISPLAY, (XEvent*)&wmGD.savedEnterEvent );
            /* Reset event flag to false */
            wmGD.replayEnterEvent = False;
          }
    }


    /*
     * Withdraw this window
     */

    WithdrawWindow (pCD);

} /* END OF FUNCTION UnManageWindow */



/*************************************<->*************************************
 *
 *  WithdrawTransientChildren (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function withdraws all transient children of the specified window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of the leader of the transient tree.
 * 
 *************************************<->***********************************/

void WithdrawTransientChildren (ClientData *pCD)
{
    ClientData *pcdNext;
    ClientData *pcdThis;


    pcdNext = pCD->transientChildren;
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    WithdrawTransientChildren (pcdNext);
	}
	pcdThis = pcdNext;
	pcdNext = pcdThis->transientSiblings;
	DeleteTransient(pcdThis);
	WithdrawWindow (pcdThis);
    }

} /* END OF FUNCTION WithdrawTransientChildren */



/*************************************<->*************************************
 *
 *  WithdrawWindow (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a top-level client window from the set of windows
 *  that is managed by the window manager.  All window manager resources
 *  associtated with the client window are freed up (possibly cached for
 *  reuse).  Any custom system menu is destroyed.
 *
 *
 *  Inputs:
 *  ------
 *  pCD 	- pointer to client data of window to withdraw
 * 
 *************************************<->***********************************/

void WithdrawWindow (ClientData *pCD)
{
    int x;
    int y;
    int i;
    XWindowChanges xwc;


    /*
     * Put the client window into a withdrawn state:
     *
     * - remove the icon/client window from the screen
     * - make sure the input focus no longer is associted with the window
     * - free the icon placement (if necessary)
     */
    SetClientWsIndex (pCD);

    if (!(pCD->clientFlags & WM_INITIALIZATION))
    {
	if (!pCD->transientLeader)
	{
	    DeleteClientFromList (pCD->pSD->pActiveWS, pCD);
	}
	ResetWithdrawnFocii (pCD);
	if (pCD->clientState & MINIMIZED_STATE)
	{
	    if (wmGD.iconAutoPlace && (!(P_ICON_BOX(pCD))))
	    {
		WsClientData *pWsc;
		int j;

		/* 
		 * Clean up icon placement data in all inhabited
		 * workspaces
		 */
		for (j = 0; j< pCD->numInhabited; j++)
		{
			pWsc = &(pCD->pWsList[j]);

			if (pWsc->iconPlace != NO_ICON_PLACE)
			{
				pWsc->IPData->placeList[pWsc->iconPlace].pCD = NULL;
			}
		}
	    }
	    if (ICON_FRAME_WIN(pCD))
	    {
		XUnmapWindow (DISPLAY, ICON_FRAME_WIN(pCD));
	    }
	    XFlush (DISPLAY);
	}
	else if ((pCD->clientState == NORMAL_STATE) ||
		 (pCD->clientState == MAXIMIZED_STATE))
	{
	    XUnmapWindow (DISPLAY, pCD->clientFrameWin);
	    XFlush (DISPLAY);
	}
    }
    /* 
     * Clean up the workspace presence dialog if it's
     * connected to this client.
     */
    if ((pCD->pSD->presence.shellW) &&
	(pCD->pSD->presence.pCDforClient == pCD))
    {
	if (pCD->pSD->presence.onScreen)
	{
	    HidePresenceBox (pCD->pSD, True);
	}
	pCD->pSD->presence.pCDforClient = NULL;
    }

    /*
     * Check to see if the window is being unmanaged because the window
     * was destroyed.
     */

    if (!(pCD->clientFlags & CLIENT_DESTROYED))
    {
	XEvent eventReturn;

	if (XCheckTypedWindowEvent (DISPLAY, pCD->clientBaseWin, DestroyNotify,
	        &eventReturn))
	{
	    pCD->clientFlags |= CLIENT_DESTROYED;
	}
    }


    /*
     * Reparent the client window back to root if the window has been
     * reparented by the window manager.  Remove the window from the
     * window managers save-set if necessary.
     */

    if ((pCD->clientFlags & CLIENT_REPARENTED) &&
        !(pCD->clientFlags & CLIENT_DESTROYED))
    {
	SetWMState (pCD->client, WithdrawnSTATE, 
		pCD->pWsList[0].iconFrameWin);

	if (pCD->maxConfig)
	{
	    x = pCD->maxX;
	    y = pCD->maxY;
	}
	else
	{
		int xoff, yoff;
		
		CalculateGravityOffset (pCD, &xoff, &yoff);
		x = pCD->clientX - xoff;
		y = pCD->clientY - yoff;
	}

	XUnmapWindow (DISPLAY, pCD->client);
	XReparentWindow (DISPLAY, pCD->client, ROOT_FOR_CLIENT(pCD), x, y);

	/* give the window back it's X border */
	xwc.border_width = pCD->xBorderWidth;
	XConfigureWindow(DISPLAY, pCD->client, CWBorderWidth, &xwc);

	if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED))
	{
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
	    XReparentWindow (DISPLAY, pCD->iconWindow, ROOT_FOR_CLIENT(pCD), 
			     pCD->pWsList[0].iconX, pCD->pWsList[0].iconY);
	}
    }


    if ((pCD->clientFlags & CLIENT_IN_SAVE_SET) &&
        !(pCD->clientFlags & CLIENT_DESTROYED))
    {
	XRemoveFromSaveSet (DISPLAY, pCD->client);

	if (pCD->iconWindow && (pCD->clientFlags & ICON_IN_SAVE_SET))
	{
	    XRemoveFromSaveSet (DISPLAY, pCD->iconWindow);
	}
    }

    /*
     * Free a custom system menu if one was created.
     */

    FreeCustomMenuSpec (pCD->systemMenuSpec);

    /*
     * Free the client window frame:
     */

    if (pCD->clientFrameWin)
    {
	FreeClientFrame (pCD);
    }


    /*
     * Free the icon associated with the client window:
     */

    if (PIXMAP_IS_VALID( pCD->iconPixmap )) 
    {
	XFreePixmap (DISPLAY, pCD->iconPixmap);
    }

    if ((pCD->numInhabited > 0) && ICON_FRAME_WIN(pCD))
    {
        FreeIcon (pCD);
    }


    /*
     * Free up the client protocol list:
     */

    if (pCD->clientProtocols)
    {
	XtFree ((char *)pCD->clientProtocols);
    }


    /*
     * Free up the mwm messages list:
     */

    if (pCD->mwmMessages)
    {
	XtFree ((char *)pCD->mwmMessages);
    }


    /*
     * Delete client window manager timers:
     */

    DeleteClientWmTimers (pCD);


    /*
     * Free up window context associations.  
     */
    DeleteClientContext (pCD);


    /* 
     * Count backward for efficiency  --  
     *     removes from end of list.
     */
    for (i = pCD->numInhabited - 1; i >= 0; i--)
    {
	TakeClientOutOfWorkspace (
	    GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID),
	    pCD);
    }

    /*
     * Free up window manager resources:
     */

    if (!(pCD->clientFlags & CLIENT_WM_CLIENTS))
    {
        if (pCD->clientName)
        {
	    XFree ((char *) (pCD->clientName));
        }
	if (pCD->clientClass)
	{
	    XFree ((char *) (pCD->clientClass));
	}
    }

    if ((pCD->clientFlags & CLIENT_HINTS_TITLE) && pCD->clientTitle)
    {
	XmStringFree (pCD->clientTitle);
    }

    if ((pCD->iconFlags & ICON_HINTS_TITLE) && pCD->iconTitle)
    {
	XmStringFree (pCD->iconTitle);
    }

    if (pCD->clientCmapCount > 0)
    {
	for (i = 0; i < pCD->clientCmapCount; i++)
	{
	    if (pCD->cmapWindows[i] != pCD->client)
	    {
			RemoveColormapWindowReference(pCD, pCD->cmapWindows[i]);
	    }
	}
	XtFree ((char *) (pCD->cmapWindows));
	XtFree ((char *) (pCD->clientCmapList));
	XtFree ((char  *) (pCD->clientCmapFlags));
    }

    /*
     * Insure list of initial properties has been freed.
     */
    DiscardInitialPropertyList (pCD);

    /* 
     * free up list of workspace specific data
     */
    if ((pCD)->pWsList)
    {
	XtFree ((char *) (pCD->pWsList));
    }

    /*
     * free up workspace hints
     */
    if (pCD->pWorkspaceHints)
    {
	XtFree ((char *)pCD->pWorkspaceHints);
    }

	/*
	 * Free up EWMH data
	 */
	if(pCD->ewmhClientTitle) XmStringFree(pCD->ewmhClientTitle);
	if(pCD->ewmhIconTitle) XmStringFree(pCD->ewmhIconTitle);
	if(pCD->ewmhIconPixmap) XFreePixmap(DISPLAY,pCD->ewmhIconPixmap);

    if (pCD->smClientID)
	XFree (pCD->smClientID);

    /*
     * Clean up references to this data before we free it.
     */
    if (wmGD.menuClient == pCD) {
	wmGD.menuClient = NULL;
    }

    if (wmGD.gadgetClient == pCD) {
	wmGD.gadgetClient = NULL;
	wmGD.gadgetDepressed = 0;
    }

    if (wmGD.clickData.pCD == pCD) {
	wmGD.clickData.pCD = NULL;
    }

    if (wmGD.nextKeyboardFocus == pCD)
	wmGD.nextKeyboardFocus = NULL;
    if (wmGD.keyboardFocus == pCD)
	wmGD.keyboardFocus = NULL;

/*
 * Fix for 5325 - Delete reference by dirty stack 
 */
    ClearDirtyStackEntry(pCD);

    XtFree ((char *)pCD);


} /* END OF FUNCTION WithdrawWindow */



/*************************************<->*************************************
 *
 *  DeleteClientContext (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes the client from the X context manager
 *  
 *
 *  Inputs:
 *  ------
 *  pCD 	- pointer to client data 
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void DeleteClientContext (ClientData *pCD)
{
    /*
     * Free up window context associations.  The context for the client
     * window is always set if there is a client data structure.
     */

    XDeleteContext (DISPLAY, pCD->client, wmGD.windowContextType);
    if (pCD->clientFlags & CLIENT_CONTEXT_SAVED)
    {
	XDeleteContext (DISPLAY, pCD->clientFrameWin, wmGD.windowContextType);
	XDeleteContext (DISPLAY, pCD->clientBaseWin, wmGD.windowContextType);
	if (DECOUPLE_TITLE_APPEARANCE(pCD))
	{
	    XDeleteContext (DISPLAY, pCD->clientTitleWin,
		wmGD.windowContextType);
	}
	if (ICON_FRAME_WIN(pCD)) 
	{
		int k;

	    for (k=0; k < pCD->numInhabited; k++)
	    {
		XDeleteContext (DISPLAY, pCD->pWsList[k].iconFrameWin, 
				 wmGD.windowContextType);
	    }
	}
	pCD->clientFlags &= ~CLIENT_CONTEXT_SAVED;
    }

} /* END OF FUNCTION DeleteClientContext */



/*************************************<->*************************************
 *
 *  ResetWitdrawnFocii (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function resets the various types of focus if they are set to a
 *  window being withdrawn.
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pCD 	- pointer to client data 
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void ResetWithdrawnFocii (ClientData *pCD)
{
    if ((wmGD.keyboardFocus == pCD) ||
	/* BEGIN fix for CDExc21090 */
	((wmGD.keyboardFocus == (ClientData *)NULL) &&
	 (wmGD.nextKeyboardFocus == pCD) &&
	 (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)))
	/* END fix for CDExc21090 */
    {
	if (wmGD.autoKeyFocus &&
	    (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
	{
	    /* local hack: if we've already received a map for a new
	    ** focus window, be sure to use wmGD.nextKeyboardFocus; otherwise 
	    ** AutoResetKeyFocus chooses an essentially arbitrary window to 
	    ** set focus to. 
	    */
	    if (wmGD.nextKeyboardFocus == pCD)
		    AutoResetKeyFocus (pCD, GetTimestamp());
	    else
	        Do_Focus_Key ((ClientData *)wmGD.nextKeyboardFocus, 
				GetTimestamp(), ALWAYS_SET_FOCUS);
	}
	else
	{
	    /*
	     * Set the focus to the default state if the focus is not in
	     * the process of being set (i.e. a FocusIn event will be 
	     * comming along shortly.
	     */

	    if (wmGD.nextKeyboardFocus == wmGD.keyboardFocus)
	    {
	        Do_Focus_Key ((ClientData *)NULL, GetTimestamp(),
		    ALWAYS_SET_FOCUS | WORKSPACE_IF_NULL);
	    }
	}
	SetKeyboardFocus ((ClientData *)NULL, 0);
    }

    if (((pCD->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL) ||
         (pCD->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL)) &&
	(wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER))
    {
	/*
	 * Repair the focus if an application modal dialog went 
	 * away. We may not see an enter event and have the focus
	 * set to the wrong place.
	 */
	RepairFocus ();
    }

    if (wmGD.nextKeyboardFocus == pCD)
    {
	wmGD.nextKeyboardFocus = NULL;
    }

    if (ACTIVE_PSD->colormapFocus == pCD)
    {
	SetColormapFocus (ACTIVE_PSD, (ClientData *)NULL);
    }

} /* END OF FUNCTION ResetWithdrawnFocii */



/*************************************<->*************************************
 *
 *  FreeClientFrame (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function frees up frame windows and associated resources.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 *************************************<->***********************************/

void FreeClientFrame (ClientData *pCD)
{
    if (pCD->pclientTopShadows) {
	FreeRList (pCD->pclientTopShadows);
	pCD->pclientTopShadows = NULL;
    }
    if (pCD->pclientBottomShadows) {
	FreeRList (pCD->pclientBottomShadows);
	pCD->pclientBottomShadows = NULL;
    }
    if (pCD->pclientTitleTopShadows) {
	FreeRList (pCD->pclientTitleTopShadows);
	pCD->pclientTitleTopShadows = NULL;
    }
    if (pCD->pclientTitleBottomShadows) {
	FreeRList (pCD->pclientTitleBottomShadows);
	pCD->pclientTitleBottomShadows = NULL;
    }
    if (pCD->pclientMatteTopShadows) {
	FreeRList (pCD->pclientMatteTopShadows);
	pCD->pclientMatteTopShadows = NULL;
    }
    if (pCD->pclientMatteBottomShadows) {
	FreeRList (pCD->pclientMatteBottomShadows);
	pCD->pclientMatteBottomShadows = NULL;
    }
    if (pCD->pTitleGadgets) {
	XtFree ((char *)pCD->pTitleGadgets);
	pCD->pTitleGadgets = NULL;
	pCD->cTitleGadgets = 0;
    }
    if (pCD->pResizeGadgets) {
	XtFree ((char *)pCD->pResizeGadgets);
	pCD->pResizeGadgets = NULL;
    }

    /* destroy frame window & all children */
    XDestroyWindow (DISPLAY, pCD->clientFrameWin);

} /* END OF FUNCTION FreeClientFrame */



/*************************************<->*************************************
 *
 *  FreeIcon (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function frees up icon windows and associated resources.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 * 
 *************************************<->***********************************/

void FreeIcon (ClientData *pCD)
{
    WmWorkspaceData *pWsTmp;
    int i;

    if (pCD->piconTopShadows) {
	FreeRList (pCD->piconTopShadows);
	pCD->piconTopShadows = NULL;
    }
    if (pCD->piconBottomShadows) {
	FreeRList (pCD->piconBottomShadows);
	pCD->piconBottomShadows = NULL;
    }

    /* 
     * destroy frame window & all children 
     */

    if ((pCD->pSD->useIconBox) && pCD->pWsList[0].pIconBox)
    {
	/* 
	 * We're using icon boxes and it's in at least one ...
	 * Delete from all workspaces we live in
	 */
	for (i = 0; i< pCD->numInhabited; i++)
	{
	    if( (pWsTmp = GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID)) )
	    {
		DeleteIconFromBox (pWsTmp->pIconBox, pCD);
	    }
	}
    }
    else
    {
	/* only one window, so destroying its first reference will
	 * clean it up adequately
	 */
	if (pCD->pWsList[0].iconFrameWin)
	{
	    XDestroyWindow (DISPLAY, pCD->pWsList[0].iconFrameWin);
	}
    }
} /* END OF FUNCTION FreeIcon */




/*************************************<->*************************************
 *
 *  WithdrawDialog (dialogboxW)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a DialogBox widget "client" from the set of windows 
 *  that are managed by the window manager.
 *
 *
 *  Inputs:
 *  ------
 *  dialogboxW = DialogBox widget to withdraw.
 * 
 *  Comments:
 *  --------
 *  Does not maintain the WM_STATE property on the dialog "client".
 * 
 *************************************<->***********************************/

void WithdrawDialog (Widget dialogboxW)
{
    int i;
    ClientData *pCD = NULL;

    /*
     * Get the dialog shell window client data.
     */

    if (XFindContext (DISPLAY, XtWindow (XtParent (dialogboxW)),
		      wmGD.windowContextType, (caddr_t *)&pCD))
      return;

    XtUnmanageChild (dialogboxW);
    DeleteClientFromList (ACTIVE_WS, pCD);
    /* TakeClientOutOfWorkspace (ACTIVE_WS, pCD); */

    /* 
     * Count backward for efficiency  --  
     *     removes from end of list.
     */
    for (i = pCD->numInhabited - 1; i >= 0; i--)
    {
	TakeClientOutOfWorkspace (
	    GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID),
	    pCD);
    }
    ResetWithdrawnFocii (pCD);
    XUnmapWindow (DISPLAY, pCD->clientFrameWin);

} /* END OF FUNCTION WithdrawDialog */



/*************************************<->*************************************
 *
 *  ReManageDialog (pSD, dialogboxW)
 *
 *
 *  Description:
 *  -----------
 *  This function remanages a DialogBox "client" that was unmanaged via 
 *  WithdrawDialog ().
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  dialogboxW = DialogBox widget to remanage.
 *
 * 
 *  Outputs:
 *  -------
 *  Does not maintain the WM_STATE property on the dialog "client".
 *
 *************************************<->***********************************/

void ReManageDialog (WmScreenData *pSD, Widget dialogboxW)
{
    ClientData *pCD = NULL;

    /*
     * Get the dialog shell window client data.
     */

    if (XFindContext (DISPLAY, XtWindow (XtParent (dialogboxW)),
		      wmGD.windowContextType, (caddr_t *)&pCD))
      return;

    /*
     * The order is important here:
     */

    /*
     * Put system modal windows in all workspaces to
     * avoid the race condition of the window coming up
     * just as the user switches workspaces OR when
     * the window is up and a user switces workspaces
     * with a key binding.  We may want to eventually short 
     * circuit F_Functions any time there is a modal
     * window up, but for now, we will just make sure
     * the modal window appears in all workspaces 
     */

    pCD->dtwmFunctions |= DtWM_FUNCTION_OCCUPY_WS;
    F_AddToAllWorkspaces(0, pCD, 0);
    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;

    if (pSD->clientList)
    {
      StackWindow (pSD->pActiveWS, &pCD->clientEntry,
                    TRUE, (ClientListEntry *) NULL);
    }
    AddClientToList (pSD->pActiveWS, pCD, True /*on top*/);
    XMapWindow (DISPLAY, pCD->clientFrameWin);
    XtManageChild (dialogboxW);

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	Do_Focus_Key (pCD, GetTimestamp() , ALWAYS_SET_FOCUS);
    }


} /* END OF FUNCTION ReManageDialog */
