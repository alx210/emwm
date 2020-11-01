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
static char rcsid[] = "$TOG: WmManage.c /main/11 1998/01/12 16:45:48 cshi $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1992, 1993 HEWLETT-PACKARD COMPANY 
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
#ifdef PANELIST
#include "WmPanelP.h"	/* typedef needed in WmManage.h */
#include <Dt/Message.h>
#include "WmIPC.h"
#endif /* PANELIST */
#include "WmManage.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmProtocol.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#ifdef WSM
#include "WmPresence.h"
#include "WmWrkspace.h"
#endif /* WSM */
#include "WmXSMP.h"



/*
 * Function Declarations:
 */

#ifdef PANELIST

Boolean IsEmbeddedClient (
    ClientData *pCD, 
    WmFpEmbeddedClientData **ppECD);
Boolean ManageEmbeddedClient ( 
    ClientData *pCD, 
    WmFpEmbeddedClientData *pECD,
    long manageFlags);
Boolean IsPushRecallClient (
    ClientData *pCD, 
    WmFpPushRecallClientData **ppPRCD);
static void HandleSubstructEvents(
        Widget w,
        caddr_t ptr,
        XEvent *event );
Boolean UpdateEmbeddedClientsProperty(
        WmScreenData *pSD );
static void ForceSubpanelWMState(Window win);
static void ReManageWindow (ClientData *pCD);
static void CheckPushRecallClient (ClientData *pCD);

#endif /* PANELIST */


/*
 * Global Variables:
 */



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
#ifdef WSM
    int nAncillaries, iAnc;
    Window *pAncillaryWindows, *pWin1;
    WmWorkspaceData *pWS0;
#endif /* WSM */
    unsigned int     nclients;
    ClientData *pcd = NULL;
    PropWMState *wmStateProp;
    Boolean manageOnRestart;
    int i,j;
    long manageFlags;

#ifdef WSM
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

#endif /* WSM */

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
#endif

	for (i = 0; i < nclients; i++)
	{
	    /* determine if the client window should be managed by wm */
#ifdef WSM
            if (InWindowList (clients[i], pAncillaryWindows, nAncillaries))
            {
		/* don't manage ancillary window manager windows */
                continue;
	    }
#else /* WSM */
            if ((clients[i] == XtWindow (pSD->screenTopLevelW)) ||
		(clients[i] == XtWindow (pSD->pActiveWS->workspaceTopLevelW)) ||
		(clients[i] == pSD->activeIconTextWin))
            {
		/* don't manage ancillary window manager windows */
                continue;
	    }
#endif /* WSM */
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

#ifdef WSM
    if (pAncillaryWindows)
    {
	XtFree ((char *) pAncillaryWindows);
    }
#endif /* WSM  */

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
#ifdef WSM
    WmWorkspaceData *pwsi;
#endif /* WSM */
#ifdef PANELIST 
    WmFpEmbeddedClientData *pECD;
#endif /* PANELIST */

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


#ifdef PANELIST
    /*
     *  Handle case of transients that derive from embedded clients.
     */
    if (wmGD.dtSD && (wmGD.dtSD == pCD->pSD))
    {
	if (pCD->transientLeader && pCD->transientLeader->pECD)
	{
	    WmPanelistObject  pPanelist;
	    ClientData *pCDfp = NULL;

	    pPanelist = (WmPanelistObject) pCD->pSD->wPanelist;
	    (void) XFindContext (DISPLAY, XtWindow(O_Shell (pPanelist)),
		      wmGD.windowContextType, (caddr_t *)&pCDfp);

	    pCD->transientLeader = pCDfp;
	}
    }

    if (IsEmbeddedClient (pCD, &pECD))
    {
	/*
	 * This client is embedded in the front panel 
	 */
	
	if (ManageEmbeddedClient(pCD, pECD, manageFlags))
	{
	    /*
	     *   ...then we've embedded it in the front
	     *   panel--no further processing required.
	     */
#ifdef WSM
	    if (smAckState == SM_START_ACK)
	    {
		SendClientMsg( wmGD.dtSmWindow, 
			      (long) wmGD.xa_DT_SM_WM_PROTOCOL,
			      (long) wmGD.xa_DT_WM_WINDOW_ACK,
			      CurrentTime, NULL, 0);
	    }
#endif /* WSM */
	    return;
	}
    }
    
    /*
     *  Handle case of transients that derive from embedded clients.
     *  !!!!
     */
#if 0
    if (pCD->transientLeader && pCD->transientLeader->pAccessPanel)
    {
        pCD->transientLeader = 
	    pCD->transientLeader->pAccessPanel->pCD_accessPanel;
    }
#endif 
#endif /* PANELIST */
#ifdef WSM
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
#endif /* WSM */
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
	(wmGD.positionIsFrame) && 
	((pCD->clientOffset.x != 0) ||
	 (pCD->clientOffset.y != 0)))
    { 
	SendClientOffsetMessage (pCD);
    }

    /*
     * Make an icon for the client window if it is not a valid transient
     * window.
     */

#ifdef WSM
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
	    if (pwsi = GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID))
	    {

		if ((pCD->pSD->useIconBox && 
                     !(manageFlags & MANAGEW_WM_CLIENTS) &&
		     !(pCD->clientFlags & FRONT_PANEL_BOX)) || (i == 0))
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
#else /* WSM */
    if ((pCD->clientFunctions & MWM_FUNC_MINIMIZE) &&
        (pCD->transientLeader == NULL) && 
	  !MakeIcon (pCD->pSD->pActiveWS, pCD))
    {
	/*
	 * Error in making an icon for the client window; clean up the wm
	 * resources; do not manage the client window.
	 */

	UnManageWindow (pCD);
	return;
    }
#endif /* WSM */


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
#ifndef WSM
    if (pCD->iconFrameWin)
    {
	XSaveContext (DISPLAY, pCD->iconFrameWin, wmGD.windowContextType,
	    (caddr_t)pCD);
    }
#endif /* WSM */

    if (pCD->clientCmapCount > 0)
    {
	for (i = 0; i < pCD->clientCmapCount; i++)
	{
	    if (pCD->cmapWindows[i] != pCD->client)
	    {
#ifndef	IBM_169380
		AddColormapWindowReference(pCD, pCD->cmapWindows[i]);
#else
	        XSaveContext (DISPLAY, pCD->cmapWindows[i],
		    wmGD.windowContextType, (caddr_t)pCD);
#endif
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

#ifndef WSM
    if (pCD->iconWindow && pCD->iconFrameWin)
    {
	XGrabButton (DISPLAY, AnyButton, AnyModifier, pCD->iconFrameWin, True,
	    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
	    GrabModeAsync, GrabModeAsync, None, wmGD.workspaceCursor);
    }
#endif /* WSM */

    /*
     * Setup key binding handling for system menu accelerators.
     */

    if (pCD->systemMenuSpec &&
        (pCD->systemMenuSpec->accelKeySpecs))
    {
	SetupKeyBindings (pCD->systemMenuSpec->accelKeySpecs,
			  pCD->clientFrameWin, GrabModeSync, F_CONTEXT_ALL);
#ifdef WSM
	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if (!pCD->pWsList[i].pIconBox && pCD->pWsList[i].iconFrameWin)
	    {
		SetupKeyBindings (pCD->systemMenuSpec->accelKeySpecs,
			      pCD->pWsList[i].iconFrameWin, GrabModeSync, 
			      F_CONTEXT_ALL);
	    }
	}
#else /* WSM */
	if (!pCD->pIconBox && pCD->iconFrameWin)
	{
	    SetupKeyBindings (pCD->systemMenuSpec->accelKeySpecs,
			      pCD->iconFrameWin, GrabModeSync, F_CONTEXT_ALL);
	}
#endif /* WSM */
    }

#ifdef WSM
  for (i = 0; i < pCD->numInhabited; i++)
  {
    if (!pCD->pWsList[i].pIconBox && pCD->pWsList[i].iconFrameWin)
#else /* WSM */
    if (!pCD->pIconBox && pCD->iconFrameWin)
#endif /* WSM */
    {
	static int iconKeySpec = 1;
	static int iconAccelSpec = 1;

        if ((iconKeySpec != 0) && KEY_SPECS(pCD))
        {
#ifdef WSM
	    iconKeySpec = SetupKeyBindings (KEY_SPECS(pCD), 
				pCD->pWsList[i].iconFrameWin,
				GrabModeSync, F_CONTEXT_ICON);
#else /* WSM */
	    iconKeySpec = SetupKeyBindings (KEY_SPECS(pCD), pCD->iconFrameWin,
				GrabModeSync, F_CONTEXT_ICON);
#endif /* WSM */
        }

        if ((iconAccelSpec != 0) && ACCELERATOR_MENU_COUNT(pCD))
        {
	    int n;

	    iconAccelSpec = 0;
	    for (n= 0; n < pSD->acceleratorMenuCount; n++)
	    {
#ifdef WSM
	        iconAccelSpec += SetupKeyBindings (
			    ACCELERATOR_MENU_SPECS(pCD)[n]->accelKeySpecs,
			    pCD->pWsList[i].iconFrameWin, GrabModeSync,
			    F_CONTEXT_ICON);
#else /* WSM */
	        iconAccelSpec += SetupKeyBindings (
			    ACCELERATOR_MENU_SPECS(pCD)[n]->accelKeySpecs,
			    pCD->iconFrameWin, GrabModeSync,
			    F_CONTEXT_ICON);
#endif /* WSM */
	    }
	}
    }
#ifdef WSM
  }
#endif /* WSM */


    /*
     * Setup keyboard focus handling if policy is "explicit".
     */

    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
    {
	DoExplicitSelectGrab (pCD->clientBaseWin);
    }

#ifdef WSM
    UpdateWorkspacePresenceProperty(pCD);
#endif /* WSM */


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
#ifdef WSM
    if (!ClientInWorkspace (pSD->pActiveWS, pCD))
    {
	initialState |= UNSEEN_STATE;
    }
#endif /* WSM */
    pCD->clientState = WITHDRAWN_STATE;
    pCD->clientFlags &= ~WM_INITIALIZATION;

#ifdef WSM
    /* 
     * Add to stacking list using the client's zero'th workspace
     * instead of the current one because it may not be in 
     * the current one.
     */
    AddClientToList (GetWorkspaceData (pSD, pCD->pWsList[0].wsID),
	pCD, True /*on top*/);
#else /* WSM */
    AddClientToList (pSD->pActiveWS, pCD, True /*on top*/);
#endif /* WSM */
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
#ifdef WSM
          !(pCD->clientState & UNSEEN_STATE) &&
#endif /* WSM */
	  (pCD->inputFocusModel ||
	   (pCD->protocolFlags & PROTOCOL_WM_TAKE_FOCUS)))))
    {
	Do_Focus_Key (pCD, GetTimestamp() , ALWAYS_SET_FOCUS);
    }
    else if ((pCD->inputMode == MWM_INPUT_SYSTEM_MODAL) ||
	     (wmGD.keyboardFocus && IS_APP_MODALIZED(wmGD.keyboardFocus)))
    {
	Do_Focus_Key ((ClientData *)NULL, GetTimestamp() , ALWAYS_SET_FOCUS);
    }

#ifdef WSM
    if (smAckState == SM_START_ACK)
    {
	SendClientMsg( wmGD.dtSmWindow, (long) wmGD.xa_DT_SM_WM_PROTOCOL,
		      (long) wmGD.xa_DT_WM_WINDOW_ACK,
		      CurrentTime, NULL, 0);
    }

    /*
     * Free the initial property list. This will force
     * reads of properties that change after the initial
     * management (see HasProperty() function.)
     */
    DiscardInitialPropertyList (pCD);

#endif /* WSM */
#ifdef PANELIST
    CheckPushRecallClient (pCD);
#endif /* PANELIST */

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
#ifdef PANELIST
    if (pCD->pECD)
    {
	WmFpEmbeddedClientData *pECD;

	pECD = (WmFpEmbeddedClientData *) pCD->pECD;

	XtRemoveEventHandler(XtWindowToWidget (DISPLAY1, pECD->winParent),
		(SubstructureRedirectMask | SubstructureNotifyMask),
	        False,
		(XtEventHandler)HandleSubstructEvents,
		(XtPointer)(pCD));

	pECD->pCD = NULL;
	UpdateEmbeddedClientsProperty (pCD->pSD);
    }

    if (pCD->pPRCD)
    {
	WmFpPushRecallClientData *pPRCD;
	int j;

	pPRCD = (WmFpPushRecallClientData *) pCD->pSD->pPRCD;

	for (j = 0; 
		 j < pCD->pSD->numPushRecallClients; 
			 j++, pPRCD++)
	{
	    /*
	     * Clean out all slots used by this client.
	     */
	    if ((!strcmp ((char *)pCD->clientName, 
			 (char *)(pPRCD->pchResName))) &&
		(pPRCD->pCD == pCD))
	    {
		pPRCD->pCD = NULL;
	    }
	}
	pCD->pPRCD = NULL;
    }
#endif /* PANELIST */
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
#ifdef WSM
    SetClientWsIndex (pCD);
#endif /* WSM */

    if (!(pCD->clientFlags & WM_INITIALIZATION))
    {
	if (!pCD->transientLeader)
	{
	    DeleteClientFromList (pCD->pSD->pActiveWS, pCD);
	}
	ResetWithdrawnFocii (pCD);
	if (pCD->clientState & MINIMIZED_STATE)
	{
#ifdef WSM
	    if (wmGD.iconAutoPlace && (!(P_ICON_BOX(pCD))))
	    {
		WmWorkspaceData *pWsTmp;
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
			if (pWsTmp=GetWorkspaceData(pCD->pSD, pWsc->wsID))
			{
			  pWsTmp->IPData.placeList[pWsc->iconPlace].pCD 
			      = NULL;
			}
		    }
		}
	    }
#else /* WSM */
	    if (wmGD.iconAutoPlace && (!(P_ICON_BOX(pCD))))
	    {
		if (ICON_PLACE(pCD) != NO_ICON_PLACE)
		{
		pCD->pSD->pActiveWS->IPData.placeList[ICON_PLACE(pCD)].pCD 
		    = NULL;
		}
	    }
#endif /* WSM */
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
#ifdef WSM
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
#endif /* WSM */

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
#ifdef WSM
	SetWMState (pCD->client, WithdrawnSTATE, 
		pCD->pWsList[0].iconFrameWin);
#else /* WSM */
	SetWMState (pCD->client, WithdrawnSTATE, ICON_FRAME_WIN(pCD));
#endif /* WSM */

	if (pCD->maxConfig)
	{
	    x = pCD->maxX;
	    y = pCD->maxY;
	}
	else
	{
	    int xoff, yoff;
	    
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

	XUnmapWindow (DISPLAY, pCD->client);
	XReparentWindow (DISPLAY, pCD->client, ROOT_FOR_CLIENT(pCD), x, y);

	/* give the window back it's X border */
	xwc.border_width = pCD->xBorderWidth;
	XConfigureWindow(DISPLAY, pCD->client, CWBorderWidth, &xwc);

	if (pCD->iconWindow && (pCD->clientFlags & ICON_REPARENTED))
	{
	    XUnmapWindow (DISPLAY, pCD->iconWindow);
#ifdef WSM
	    XReparentWindow (DISPLAY, pCD->iconWindow, ROOT_FOR_CLIENT(pCD), 
			     pCD->pWsList[0].iconX, pCD->pWsList[0].iconY);
#else /* WSM */
	    XReparentWindow (DISPLAY, pCD->iconWindow, ROOT_FOR_CLIENT(pCD), 
			     ICON_X(pCD), ICON_Y(pCD));
#endif /* WSM */
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

#ifdef WSM
    if ((pCD->numInhabited > 0) && ICON_FRAME_WIN(pCD))
#else /* WSM */
    if (ICON_FRAME_WIN(pCD))
#endif /* WSM */
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


#ifdef WSM
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
#endif /* WSM */

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
#ifndef	IBM_169380
		RemoveColormapWindowReference(pCD, pCD->cmapWindows[i]);
#else
		XDeleteContext (DISPLAY, pCD->cmapWindows[i],
		    wmGD.windowContextType);
#endif
	    }
	}
	XtFree ((char *) (pCD->cmapWindows));
	XtFree ((char *) (pCD->clientCmapList));
#ifndef OLD_COLORMAP /* colormap */
	XtFree ((char  *) (pCD->clientCmapFlags));
#endif
    }

#ifdef WSM
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
#endif /* WSM */

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
#ifdef WSM
            int k;

	    for (k=0; k < pCD->numInhabited; k++)
	    {
		XDeleteContext (DISPLAY, pCD->pWsList[k].iconFrameWin, 
				 wmGD.windowContextType);
	    }
#else /* WSM */
	    XDeleteContext (DISPLAY, pCD->iconFrameWin, 
	                     wmGD.windowContextType);
#endif /* WSM */
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
#ifdef WSM
    WmWorkspaceData *pWsTmp;
    int i;
#endif /* WSM */

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

#ifdef WSM
    if ((pCD->pSD->useIconBox) && pCD->pWsList[0].pIconBox)
    {
	/* 
	 * We're using icon boxes and it's in at least one ...
	 * Delete from all workspaces we live in
	 */
	for (i = 0; i< pCD->numInhabited; i++)
	{
	    if (pWsTmp = GetWorkspaceData(pCD->pSD, pCD->pWsList[i].wsID))
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
#else /* WSM */
    if (pCD->pSD->useIconBox && P_ICON_BOX(pCD))
    {
	DeleteIconFromBox (pCD->pSD->pActiveWS->pIconBox, pCD);
    }
    else
    {
	XDestroyWindow (DISPLAY, pCD->iconFrameWin);
    }
#endif /* WSM */

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
#ifdef WSM
    int i;
#endif /* WSM */
    ClientData *pCD = NULL;

    /*
     * Get the dialog shell window client data.
     */

    if (XFindContext (DISPLAY, XtWindow (XtParent (dialogboxW)),
		      wmGD.windowContextType, (caddr_t *)&pCD))
      return;

    XtUnmanageChild (dialogboxW);
    DeleteClientFromList (ACTIVE_WS, pCD);
#ifdef WSM
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
#endif /* WSM */
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

#ifdef WSM
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
#endif /* WSM */

    if (pSD->clientList)
    {
      StackWindow (pSD->pActiveWS, &pCD->clientEntry,
                    TRUE, (ClientListEntry *) NULL);
    }
    AddClientToList (pSD->pActiveWS, pCD, True /*on top*/);
    XMapWindow (DISPLAY, pCD->clientFrameWin);
    XtManageChild (dialogboxW);

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
    {
	Do_Focus_Key (pCD, GetTimestamp() , ALWAYS_SET_FOCUS);
    }


} /* END OF FUNCTION ReManageDialog */

#ifdef PANELIST

/*************************************<->*************************************
 *
 *  RegisterEmbeddedClients (wPanelist, pECD, count)
 *
 *
 *  Description:
 *  -----------
 *  This function registers a list of clients to be embedded in the
 *  front panel subsystem.
 *
 *
 *  Inputs:
 *  ------
 *  wPanelist = front panel object (widget)
 *  pECD = pointer to list of data for clients to embed
 *  count = number of elements in the list
 *
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void
RegisterEmbeddedClients (
	Widget wPanelist, 
	WmFpEmbeddedClientList pECD, 
	int count)
{
    WmScreenData *pSD;
    int i;

    for (i= 0; i < wmGD.numScreens; i++)
    {
	pSD = &(wmGD.Screens[i]);

	if (pSD->managed)
	{
	   if (pSD->wPanelist == wPanelist)
	       break;
	}
    }

    if (i < wmGD.numScreens)
    {
	pSD->pECD = (struct _WmFpEmbeddedClientData *) pECD;
	pSD->numEmbeddedClients = count;
    }
#ifdef DEBUG
    else
    {
	fprintf (stderr, "Couldn't match wPanelist to screen data\n");
    }
#endif /* DEBUG */

} /* END OF FUNCTION RegisterEmbeddedClients */


#define LTT_INCREMENT  16
/*************************************<->*************************************
 *
 *  ListTransientSubtree (pCD, ppWins, pSize, pCount)
 *
 *
 *  Description:
 *  -----------
 *  This function returns the list of windows in a transient window
 *  tree.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data of a window.
 *  ppWins	- address of a pointer to a list of windows 
 *		  (this must be in the heap -- XtMalloc).
 *  pSize	- address of variable with size of list
 *  pCount	- address of variable with number of windows in list
 * 
 *  Outputs:
 *  -------
 *  *ppWins	- may point to a new area of memory if list grows
 *  *pSize	- if list has to grow, this may be bigger
 *  *pCount	- number of windows in the list
 *
 *  Comments
 *  --------
 *  The list should be freed when done with XtFree().
 *
 *************************************<->***********************************/

static void
ListTransientSubtree (
	ClientData *pCD,
	Window **ppWins,
	int *pSize,
	int *pCount)
{
    /*
     * Check size
     */
    if (*pCount == *pSize) 
    {
	*pSize += LTT_INCREMENT;
	*ppWins = (Window *) 
		XtRealloc ((char *) *ppWins, (*pSize * sizeof(Window)));
    }
    /*
     * Add this window to the list
     */
    (*ppWins)[*pCount] = pCD->client;
    *pCount += 1;

    /*
     * Add siblings
     */
    if (pCD->transientSiblings)
	ListTransientSubtree (pCD->transientSiblings, ppWins, pSize, pCount);

    /*
     * Add children
     */
    if (pCD->transientChildren)
	ListTransientSubtree (pCD->transientChildren, ppWins, pSize, pCount);

    
} /* END OF FUNCTION ListTransientSubtree */



/*************************************<->*************************************
 *
 *  ListTransientTree (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function returns the list of windows in a transient window
 *  tree.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data of a primary window.
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments
 *  --------
 *  The list should be freed when done with XtFree().
 *
 *************************************<->***********************************/

static Window *
ListTransientTree (
	ClientData *pCD)
{
    Window *pWins;
    int count;
    int iSize;

    /*
     * Initial allocation
     */
    iSize = LTT_INCREMENT;
    pWins = (Window *) XtMalloc (iSize * sizeof(Window));
    count = 0;

    /*
     * Add this window to the list
     */
    ListTransientSubtree (pCD, &pWins, &iSize, &count);

    /*
     * Add terminator to end of window list
     */
    if (count == iSize) 
    {
	iSize += LTT_INCREMENT;
	pWins = (Window *) 
		XtRealloc ((char *)pWins, (iSize * sizeof(Window)));
    }
    pWins[count++] = None;

    /*
     * Return the list of windows found
     */
    return (pWins);
    
} /* END OF FUNCTION ListTransientTree */


/*************************************<->*************************************
 *
 *  ReManageWindow (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function unmanages and remanages a window and it's associated
 *  transients.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data of a primary window.
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments
 *  --------
 *  The pointer pCD is invalid after calling this function -- a
 *  side-effect of unmanaging the client before remanaging it.
 *
 *************************************<->***********************************/

static void
ReManageWindow (
	ClientData *pCD)
{
    long manageFlags = MANAGEW_NORMAL;
    Window *pWins, *pW;
    WmScreenData *pSD;

    /*
     * Get the list of windows in the transient window tree.
     */
    pWins = ListTransientTree (pCD);

    pSD = pCD->pSD;

    /*
     * Unmanage this window and associated transients
     */
    UnManageWindow (pCD);

    /*** pCD is no longer a valid pointer!!! ***/
    
    /*
     * Remanage this window and its secondaries
     */
    pW = pWins;
    while (*pW != None)
    {
	ManageWindow (pSD, *pW, manageFlags);
	pW++;
    }

    XtFree ((char *) pWins);

} /* END OF FUNCTION ReManageWindow */



/*************************************<->*************************************
 *
 *  ScanForEmbeddedClients (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function scans the managed windows and identifies those that
 *  should be embedded clients in the front panel
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data.
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void
ScanForEmbeddedClients (
	WmScreenData *pSD)
{
    ClientData *pCD;
    ClientListEntry *pCLE;
    WmFpEmbeddedClientData *pECD;
    Boolean bReset;
    long manageFlags = 0L;
    Window *pWins, *pW;

    /*
     *  Search through all the windows we're managing right now to
     *  see if any should be embedded in a front/sub panel.
     */
    pCLE = pSD->clientList;
    bReset = False;

    while (pCLE != NULL)
    {
	/*
	 * See if this is an previously unrecognized embedded client
	 */
	pCD = pCLE->pCD;

	if ((pCD->pECD == NULL ) && IsEmbeddedClient (pCD, &pECD))
	{
	    /*
	     * Remanage this window and associated transients
	     */
	    ReManageWindow (pCD);
	    /*
	     * At this point pCD is no longer valid and the
	     * pSD->clientList has been changed.
	     */
	    bReset = True;
	}
	
	/*
	 * Test for exit condition 
	 */
	if (pCLE == pSD->lastClient) 
	{
	    /*
	     * Gone all the way through the list without finding
	     * anything -- time to quit
	     */
	    break;
	}
	else if (bReset)
	{
	    /*
	     * Remanaging a client restructures the client list.
	     * Start over at the beginning.
	     */
	    bReset = False;
	    pCLE = pSD->clientList;
	}
	else
	{
	    /*
	     * Move to next client.
	     */
	    pCLE = pCLE->nextSibling;
	}
    }

} /* END OF FUNCTION ScanForEmbeddedClients */


/*************************************<->*************************************
 *
 *  IsEmbeddedClient (pCD, ppECD)
 *
 *
 *  Description:
 *  -----------
 *  This function tests a a client to see if it should be embedded
 *  in the front panel.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = ptr to Client Data
 *  ppECD = ptr to returned embedded client data ptr
 *
 * 
 *  Outputs:
 *  -------
 *  *ppECD = ptr to embedded client data
 *
 *************************************<->***********************************/

Boolean

IsEmbeddedClient (ClientData *pCD, WmFpEmbeddedClientData **ppECD)

{
    WmScreenData *pSD;
    int i;
    Boolean bFoundMatch = False;
    WmFpEmbeddedClientData *pECD;

    pSD = pCD->pSD;
    pECD = (WmFpEmbeddedClientData *) pSD->pECD;

    for (i = 0; i < pSD->numEmbeddedClients && !bFoundMatch; i++, pECD++)
    {
	/*
	 * It's an embedded client if 
	 *     the resource name matches a slot and 
	 *     it's not a subpanel and
	 *     the slot isn't already filled.
	 */
	if ((!strcmp ((char *)pCD->clientName, 
		      (char *)(pECD->pchResName))) &&
	    (!(pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)) &&
	    (!pECD->pCD))
	{
	    *ppECD = pECD;
	    bFoundMatch = True;
	}
    }
    return (bFoundMatch);

} /* END OF FUNCTION IsEmbeddedClient */


/******************************<->*************************************
 *
 *  ManageEmbeddedClient (pCD, pECD, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This is the function that is used to setup a client window
 *  in the front panel.
 *
 *  Inputs:
 *  ------
 *  pCD = initialized client data, including window of client that
 *        we want to manage.
 *  pECD = ptr to embedded client entry for this client
 *
 *  manageFlags	= additional control information 
 * 
 *  Outputs:
 *  -------
 *  Returns False if normal client processing needs to be done.
 *
 *  Returns True if this client has been embedded directly in the
 *  front panel and is NOT to be managed as a normal top level
 *  window--no further processing required.
 ******************************<->***********************************/
Boolean

ManageEmbeddedClient (
    ClientData *pCD, 
    WmFpEmbeddedClientData *pECD,
    long manageFlags)

{
    int wsIndex;
    int i;
    XWindowChanges windowChanges;
    unsigned int mask;
    WmFpPushRecallClientData *pPRCD;

    if (!pECD || !pCD)
    {
	return (False);
    }
   
    /*
     * Add to all workspaces
     */
    pCD->dtwmFunctions |= DtWM_FUNCTION_OCCUPY_WS;

    F_AddToAllWorkspaces(0, pCD, 0);

    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;

    /*
     * Set client list entries 
     * (in a list by itself)
     */
    pCD->clientEntry.type = NORMAL_STATE;
    pCD->clientEntry.pCD = pCD;
    pCD->clientEntry.nextSibling = NULL;
    pCD->clientEntry.prevSibling = NULL;

    pCD->iconEntry.type = MINIMIZED_STATE;
    pCD->iconEntry.pCD = pCD;
    pCD->iconEntry.nextSibling = NULL;
    pCD->iconEntry.prevSibling = NULL;

    /*
     *  Save context for event processing.
     *
     */

    XSaveContext (DISPLAY, pCD->client, wmGD.windowContextType, 
		    (caddr_t)pCD);

    if (!(pCD->clientFlags & CLIENT_WM_CLIENTS))
    {
	XChangeSaveSet (DISPLAY, pCD->client, SetModeInsert);
	XChangeSaveSet (DISPLAY1, pCD->client, SetModeInsert);
	pCD->clientFlags |= CLIENT_IN_SAVE_SET;
    }
    if (!(manageFlags & MANAGEW_WM_CLIENTS))
    {
	XSync (DISPLAY1, False);

	if (pCD->clientFlags & CLIENT_DESTROYED)
	{
	    UnManageWindow (pCD);
	    return (True); 
	}
    }

    XtAddEventHandler(XtWindowToWidget (DISPLAY1, pECD->winParent),
		(SubstructureRedirectMask | SubstructureNotifyMask),
	        False,
		(XtEventHandler)HandleSubstructEvents,
		(XtPointer)(pCD));

    /* 
     * Fill in more client data
     */
    pCD->clientX = pECD->x;
    pCD->clientY = pECD->y;
    pCD->clientWidth = pECD->width;
    pCD->clientHeight = pECD->height;

    pCD->clientFrameWin = 0;
    pCD->clientBaseWin = pECD->winParent;

    pECD->pCD = pCD;
    pCD->pECD = (void *) pECD;

#ifdef WSM
    SetClientWsIndex(pCD);
#endif
    SetClientWMState (pCD, NormalState, NORMAL_STATE);

    /*
     * Set state on subpanel in case it never gets mapped
     * to prevent session manager from finding an embedded
     * client on its own.
     */
    ForceSubpanelWMState (pECD->winParent);

    XReparentWindow (DISPLAY1, pCD->client, 
		     pECD->winParent,
		     pECD->x, pECD->y);
    pCD->clientFlags |= CLIENT_REPARENTED;

    windowChanges.width = pECD->width;
    windowChanges.height = pECD->height;
    windowChanges.border_width = 0;
    mask = (CWWidth | CWHeight | CWBorderWidth);

    XConfigureWindow (DISPLAY1, pCD->client, mask, &windowChanges);

    XMapWindow (DISPLAY1, pCD->client);
    if (pCD->iconWindow)
    {
	XUnmapWindow (DISPLAY, pCD->iconWindow);
    }

    UpdateEmbeddedClientsProperty (pCD->pSD);

    SendConfigureNotify (pCD);

    if (IsPushRecallClient (pCD, &pPRCD))
    {
	/*
	 * There should only be one instance of this
	 * client started from a front panel button.
	 */
	pPRCD->pCD = pCD;
	pCD->pPRCD = (void *) pPRCD;
    }

    WmStopWaiting(); 

    return(True); /* successful embedation */

} /* END OF FUNCTION ManageEmbeddedClient */


/******************************<->*************************************
 *
 *  ReparentEmbeddedClient (pECD, newControl, newWin, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pECD = ptr to embedded client entry for this client
 *  newControl = widget for new "parent" widget
 *  newWin = window ID of window that this embedded client will be
 *           a parent of. This is needed in case the control is a
 *           gadget.
 *  x = x-coord position within newWin where UL corner of the embedded
 *      client will go.
 *  y = y-coord as described above.
 *  width = desired width of embedded client in this new location
 *  height = desired height as above.
 *
 * 
 *  Outputs:
 *  -------
 *  Returns False if embedded client was is not moved to the new
 *  location.
 *
 *  Returns True if this client has been reparented to the new
 *  control.
 ******************************<->***********************************/
Boolean

ReparentEmbeddedClient (
    WmFpEmbeddedClientData *pECD,
    Widget newControl,
    Window newWin,
    int x, 
    int y,
    unsigned int width, 
    unsigned int height
    )

{
    int wsIndex;
    int i;
    XWindowChanges windowChanges;
    unsigned int mask;
    WmFpPushRecallClientData *pPRCD;
    ClientData *pCD;

    /*
     * If we have bogus data or if we're asked
     * to reparent to our current parent, then just
     * say no.
     */
    if ((pECD == NULL) || 
	(pECD->pCD == NULL) ||
	(pECD->winParent == newWin))
    {
	return (False);
    }
    pCD=pECD->pCD;

    /*
     * Need event handler on new parent window?
     */
    if (newWin != pECD->winParent)
    {
	XtRemoveEventHandler(XtWindowToWidget (DISPLAY1, pECD->winParent),
		(SubstructureRedirectMask | SubstructureNotifyMask),
	        False,
		(XtEventHandler)HandleSubstructEvents,
		(XtPointer)(pCD));

	XtAddEventHandler(XtWindowToWidget (DISPLAY1, newWin),
		(SubstructureRedirectMask | SubstructureNotifyMask),
	        False,
		(XtEventHandler)HandleSubstructEvents,
		(XtPointer)(pCD));
    }

    /* 
     * Update embedding and client data
     */
    pECD->wControl = newControl;
    pECD->winParent = newWin;
    pCD->clientX = pECD->x = x;
    pCD->clientY = pECD->y = y;
    pCD->clientWidth = pECD->width = width;
    pCD->clientHeight = pECD->height = height;
    pCD->clientBaseWin = pECD->winParent;

    /*
     * Set state on subpanel in case it never gets mapped
     * to prevent session manager from finding an embedded
     * client on its own.
     */
    ForceSubpanelWMState (pECD->winParent);

    /*
     * Do the actual reparent
     */
    XReparentWindow (DISPLAY1, pCD->client, 
		     pECD->winParent,
		     pECD->x, pECD->y);

    /*
     * Configure the embedded client
     */
    windowChanges.width = pECD->width;
    windowChanges.height = pECD->height;
    windowChanges.border_width = 0;
    mask = (CWWidth | CWHeight | CWBorderWidth);

    XConfigureWindow (DISPLAY1, pCD->client, mask, &windowChanges);

    XMapWindow (DISPLAY1, pCD->client);

    UpdateEmbeddedClientsProperty (pCD->pSD);

    SendConfigureNotify (pCD);

    return(True); /* successful reparent */

} /* END OF FUNCTION ReparentEmbeddedClient */


/*************************************<->*************************************
 *
 *  ForceSubpanelWMState (win)
 *
 *
 *  Description:
 *  -----------
 *  This function forces a WM_STATE property on a subpanel window
 *  so that the session manager doesn't save multiple copies
 *  of embedded clients for subpanels that never get mapped.
 *
 *
 *  Inputs:
 *  ------
 *  win = window ID of a subpanel window (not necessarily the top level!)
 *
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

static void
ForceSubpanelWMState (Window win)
{
    ClientData *pCD = NULL;
    Window root, parent;
    Window *children = NULL;
    unsigned int numChildren;
    PropWMState *wmStateProp;
    Boolean bDone = False;

    while (!bDone)
    {
	if (!XQueryTree (DISPLAY, win, &root, &parent, 
		&children, &numChildren))
	{
	    break;
	}

	if (!XFindContext(DISPLAY, win, wmGD.windowContextType, 
	    (caddr_t *)&pCD))
	{
	    /*
	     * Only continue if we're not already managing this subpanel.
	     */
	    bDone = True;
	}
	else if (parent == root)
	{
	    if (wmStateProp = GetWMState (win))
	    {
		/*
		 * Already has a WM_STATE.
		 */
		XFree ((char *)wmStateProp);
	    }
	    else
	    {
		/*
		 * Add a dummy WM_STATE to foil the session manager
		 * search.
		 */
		SetWMState (win, WITHDRAWN_STATE, 0);
	    }
	    bDone = True;
	}
	else 
	{
	    /* continue ascent up to root */
	    win = parent;
	}

	XFree ((char *) children);
    }

} /* END OF FUNCTION ForceSubpanelWMState */


/*************************************<->*************************************
 *
 *  RegisterPushRecallClients (wPanelist, pPRCD, count)
 *
 *
 *  Description:
 *  -----------
 *  This function registers a list of push_recallclients for the 
 *  front panel subsystem. 
 *
 *
 *  Inputs:
 *  ------
 *  wPanelist = front panel object (widget)
 *  pPRCD = pointer to list of data for clients
 *  count = number of elements in the list
 *
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void
RegisterPushRecallClients (
	Widget wPanelist, 
	WmFpPushRecallClientList pPRCD, 
	int count)
{
    WmScreenData *pSD;
    int i;

    for (i= 0; i < wmGD.numScreens; i++)
    {
	pSD = &(wmGD.Screens[i]);

	if (pSD->managed)
	{
	   if (pSD->wPanelist == wPanelist)
	       break;
	}
    }

    if (i < wmGD.numScreens)
    {
	pSD->pPRCD = (struct _WmFpPushRecallClientData *) pPRCD;
	pSD->numPushRecallClients = count;
    }
#ifdef DEBUG
    else
    {
	fprintf (stderr, "Couldn't match wPanelist to screen data\n");
    }
#endif /* DEBUG */

    for (i = 0; i < pSD->numPushRecallClients ; i++, pPRCD++)
    {
	/*
	 * Initialize data in each slot
	 */
	pPRCD->tvTimeout.tv_sec = 0;
    }

} /* END OF FUNCTION RegisterPushRecallClients */


/*************************************<->*************************************
 *
 *  IsPushRecallClient (pCD, ppPRCD)
 *
 *
 *  Description:
 *  -----------
 *  This function tests a a client to see if it should be embedded
 *  in the front panel.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = ptr to Client Data
 *  ppPRCD = ptr to returned embedded client data ptr
 *
 * 
 *  Outputs:
 *  -------
 *  *ppPRCD = ptr to embedded client data
 *
 *************************************<->***********************************/

Boolean

IsPushRecallClient (ClientData *pCD, WmFpPushRecallClientData **ppPRCD)

{
    WmScreenData *pSD;
    int i;
    Boolean bFoundMatch = False;
    WmFpPushRecallClientData *pPRCD;

    pSD = pCD->pSD;
    pPRCD = (WmFpPushRecallClientData *) pSD->pPRCD;

    for (i = 0; i < pSD->numPushRecallClients && !bFoundMatch; i++, pPRCD++)
    {
	/*
	 * It's a push_recall client if the resource name matches
	 * a slot and the slot isn't already filled.
	 */
	if ((!strcmp ((char *)pCD->clientName, 
		     (char *)(pPRCD->pchResName))) &&
	    (!pPRCD->pCD))
	{
	    *ppPRCD = pPRCD;
	    bFoundMatch = True;
	}
    }
    return (bFoundMatch);

} /* END OF FUNCTION IsPushRecallClient */


/*************************************<->*************************************
 *
 *  ScanForPushRecallClients (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function scans the managed windows and identifies those that
 *  should be push recall clients of the front panel
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data.
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void
ScanForPushRecallClients (
	WmScreenData *pSD)
{
    ClientData *pCD;
    ClientListEntry *pCLE;
    WmFpPushRecallClientData *pPRCD;

    /*
     *  Search through all the windows we're managing right now 
     */
    pCLE = pSD->clientList;

    while (pCLE != NULL)
    {
	/*
	 * See if this is an previously unrecognized push recall client
	 */
	pCD = pCLE->pCD;

	if ((pCD->pPRCD == NULL ) && IsPushRecallClient (pCD, &pPRCD))
	{
	    CheckPushRecallClient (pCD);
	}
	
	/*
	 * Test for exit condition 
	 */
	if (pCLE == pSD->lastClient) 
	{
	    /*
	     * Gone all the way through the list without finding
	     * anything -- time to quit
	     */
	    break;
	}
	else
	{
	    /*
	     * Move to next client.
	     */
	    pCLE = pCLE->nextSibling;
	}
    }

} /* END OF FUNCTION ScanForPushRecallClients */


/******************************<->*************************************
 *
 *  static void CheckPushRecallClient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Checks a client against the list of push recall clients to see
 *  if there are any matches. All matches are marked.
 *
 *  Inputs:
 *  ------
 *  pCD - pointer to the Client Data structure
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *
 ******************************<->***********************************/
static void 
CheckPushRecallClient(
        ClientData *pCD)
{
    WmFpPushRecallClientData *pPRCD;

    while (IsPushRecallClient (pCD, &pPRCD))
    {
	/*
	 * There should only be one instance of this
	 * client started from a front panel button.
	 */
	pPRCD->pCD = pCD;
	pPRCD->tvTimeout.tv_sec = 0;
	if (!pCD->pPRCD)
	    pCD->pPRCD = (void *) pPRCD;
    }
}


/******************************<->*************************************
 *
 *  static void HandleSubstructEvents (Widget w, caddr_t pCD, XEvent *event)
 *
 *
 *  Description:
 *  -----------
 *  Causes death of embedded clients to run through UnManageWindow()
 *  for proper cleanup.
 *
 *  Note there is one of these event handlers instantiated for
 *  each live client window in the front panel.  Hence, for each
 *  live client window death, each of the event handlers gets called
 *  once.  We need to ensure that we've got the right pCD before
 *  calling UnManageWindow() on it.
 *
 *
 *  Inputs:
 *  ------
 *  w   - not used
 *  pCD - pointer to the Client Data structure
 *  event - we only care about UnMapNotify
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  This routine is called LOTS of times, for all types of events.
 *
 ******************************<->***********************************/
static void 
HandleSubstructEvents(
        Widget w,
        caddr_t ptr,
        XEvent *event )
{
	struct _ClientData *pCD = (struct _ClientData *)ptr;

        switch (event->type)
	{
	    case UnmapNotify:
	    {
		if (pCD->client == event->xunmap.window)
		{
		    UnManageWindow (pCD);
		}
		break;
	    }
	}
} /* END OF FUNCTION HandleSubstructEvents */


/*******************************<->*************************************
 *
 *  UpdateEmbeddedClientsProperty (pSD)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD - pointer to the screen data
 * 
 *  Outputs:
 *  -------
 *  True if successful, False otherwise.
 *
 *  Comments:
 *  --------
 *  The _DT_WORKSPACE_EMBEDDED_CLIENTS property on the 
 *  root window for the screen will be updated to reflect the
 *  current contents of the front panel
 *
 * 
 ******************************<->***********************************/
Boolean 
UpdateEmbeddedClientsProperty(
        WmScreenData *pSD )
{
    unsigned int numClients = 0;
    Window *pClients = NULL;
    Boolean rval = True;
    int i;
    WmFpEmbeddedClientData *pECD;

    pECD = (WmFpEmbeddedClientData *) pSD->pECD;

    for (i = 0; i < pSD->numEmbeddedClients; i++, pECD++)
    {
	if (pECD->pCD)
	{
	    numClients += 1;

	    if (!pClients) 
	    {
		pClients = (Window *) XtMalloc (sizeof(Window));
	    }
	    else
	    {
		pClients = (Window *) XtRealloc ((char *)pClients,
		    (numClients * (sizeof(Window))));
	    }

	    if (!pClients)
	    {
		Warning (
((char *)GETMESSAGE(4, 17, "Insufficient memory to write _DT_WORKSPACE_EMBEDDED_CLIENTS."))
			);
		rval = False;
		break;
	    }
	    else
	    {
		pClients[numClients-1] = pECD->pCD->client;
	    }
	}
    }

    SetEmbeddedClientsProperty (pSD->rootWindow, pClients,
	numClients);

    if (pClients != NULL)
    {
	XtFree ((char *)pClients);
    }

    return (rval);
} /* END OF FUNCTION UpdateEmbeddedClientsProperty */



/*******************************<->*************************************
 *
 *  UnParentControls (pSD, unmap)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD - pointer to the screen data
 *  unmap - if True, then unmap the windows after reparenting to root
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  Reparents clients embedded in the front panel back to the 
 *  root window
 * 
 ******************************<->***********************************/
void 
UnParentControls(
        WmScreenData *pSD,
        Boolean unmap )
{
    int i;
    ClientData *pCD;
    WmFpEmbeddedClientData *pECD;
    
    if (pSD && pSD->managed)
    {
	pECD = (WmFpEmbeddedClientData *) pSD->pECD;
	for (i = 0; i < pSD->numEmbeddedClients; i++, pECD++)
	{
	    pCD = pECD->pCD;

	    if (pCD)
	    {
		if ((pCD->clientFlags & CLIENT_IN_SAVE_SET) &&
		    !(pCD->clientFlags & CLIENT_DESTROYED))
		{
		    XRemoveFromSaveSet (DISPLAY, pCD->client);
		    XRemoveFromSaveSet (DISPLAY1, pCD->client);
		}
		
		XReparentWindow (DISPLAY,
				 pCD->client,
				 pSD->rootWindow,
				 pCD->clientX,
				 pCD->clientY);
		if (unmap)
		{
		    XUnmapWindow (DISPLAY, pCD->client);
		    if (pCD->iconWindow)
		    {
			if (pCD->clientFlags & ICON_IN_SAVE_SET)
			{
			    XRemoveFromSaveSet (DISPLAY, pCD->iconWindow);
			    pCD->clientFlags &= ~ICON_IN_SAVE_SET;
			}
			XUnmapWindow (DISPLAY, pCD->iconWindow);
		    }
		}
	    }
	}
    }
    
} /* END OF FUNCTION UnParentControl */



/*************************************<->*************************************
 *
 *  RegisterIconBoxControl (wPanelist)
 *
 *
 *  Description:
 *  -----------
 *  This function registers the icon box control in a front panel
 *
 *
 *  Inputs:
 *  ------
 *  wPanelist = front panel object (widget)
 *
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void
RegisterIconBoxControl (Widget wPanelist)
{
    WmScreenData *pSD;
    int i;

    for (i= 0; i < wmGD.numScreens; i++)
    {
	pSD = &(wmGD.Screens[i]);

	if (pSD->managed)
	{
	   if (pSD->wPanelist == wPanelist)
	       break;
	}
    }

    if (i < wmGD.numScreens)
    {
	pSD->iconBoxControl = True;
	pSD->useIconBox = True;
    }
#ifdef DEBUG
    else
    {
	fprintf (stderr, "Couldn't match wPanelist to screen data\n");
    }
#endif /* DEBUG */

} /* END OF FUNCTION RegisterIconBoxControl */

#endif /* PANELIST */
