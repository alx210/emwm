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
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmCEvent.c /main/10 1996/08/09 15:05:39 rswiston $"
#endif
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"

#include <X11/Xatom.h>

/*
 * include extern functions
 */
#include "WmCEvent.h"
#include "WmCDecor.h"
#include "WmColormap.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmKeyFocus.h"
#ifdef PANELIST
#include "WmPanelP.h"
#endif /* PANELIST */
#include "WmManage.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmProtocol.h"
#include "WmWinConf.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */


/*
 * Global Variables:
 */

extern unsigned int buttonModifierMasks[];


/*************************************<->*************************************
 *
 *  SetupCButtonBindings (window, buttonSpecs)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the event handling necessary to support user
 *  specified button bindings for window manager functions that apply to
 *  the client window.
 *
 *
 *  Inputs:
 *  ------
 *  window = grab window id
 *
 *  buttonSpecs = list of button bindings for window manager functions
 *
 *************************************<->***********************************/

void SetupCButtonBindings (Window window, ButtonSpec *buttonSpecs)
{
    ButtonSpec *buttonSpec;
    unsigned int eventMask;
    unsigned int grabState;


    /*
     * If the context of the button binding includes "window" do button
     * grabs to get the button events that invoke window manger functions.
     * !!! don't do redundant grabs !!!
     */

    buttonSpec = buttonSpecs;
    while (buttonSpec)
    {
	if ((buttonSpec->context & F_CONTEXT_WINDOW) &&
	    (buttonSpec->subContext & F_SUBCONTEXT_W_CLIENT))
	{
	    eventMask = ButtonMotionMask | ButtonReleaseMask;

	    if (buttonSpec->eventType == ButtonRelease)
	    {
		/*
		 * Don't include the button down in the grab state.
		 */

		grabState = buttonSpec->state &
				~(buttonModifierMasks[buttonSpec->button]);
	    }
	    else
	    {
		grabState = buttonSpec->state;
	    }

	    WmGrabButton (DISPLAY, buttonSpec->button, grabState,
	        window, False, eventMask, GrabModeSync,
	        GrabModeAsync, None, None);
	}
	/*
	 * If the window context is not "window" a general grab is not
	 * necessary.
	 */

	buttonSpec = buttonSpec->nextButtonSpec;
    }

} /* END OF FUNCTION SetupCButtonBindings */



/*************************************<->*************************************
 *
 *  WmDispatchClientEvent (event)
 *
 *
 *  Description:
 *  -----------
 *  This function detects and dispatches events that are reported to a client
 *  frame or icon window that are not widget-related (i.e. they would not be
 *  dispatched by the Xtk intrinsics).
 *
 *
 *  Inputs:
 *  ------
 *  event = This is an X event that has been retrieved by XtNextEvent.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

Boolean WmDispatchClientEvent (XEvent *event)
{
    ClientData * pCD = NULL;
#ifndef	IBM_169380
    ClientData **cmap_window_data = NULL;
#endif
    Boolean dispatchEvent = False;

    /*
     * Detect and dispatch non-widget events that have been reported to
     * an icon or a client window frame.
     */

#ifndef IBM_169380
    if ((XFindContext (DISPLAY, event->xany.window, wmGD.windowContextType,
            (caddr_t *)&pCD)) &&
        (XFindContext (DISPLAY, event->xany.window, wmGD.cmapWindowContextType,
            (caddr_t *)&cmap_window_data)))
#else
    if (XFindContext (DISPLAY, event->xany.window, wmGD.windowContextType,
	    (caddr_t *)&pCD))
#endif
    {
	/*
	 *  Set active screen if we're not sure. 
	 */
	if (wmGD.queryScreen)
	    DetermineActiveScreen (event);

	/*
	 * Handle events on windows that are made by mwm for
	 * non-client-specific functions.  Also handle "leftover"
	 * events on windows that used to be managed by mwm
	 * (e.g. ConfigureRequest events).
	 */

	return (HandleEventsOnSpecialWindows (event));
    }

#ifndef IBM_169380
    if (cmap_window_data)
    /*
     * Event is on a subwindow that is specified in one or more toplevel
     * window's WM_COLORMAP_WINDOWS property.  (Most likely this is a
     * ColormapNotify event.)  It could have more than one pCD associated
     * with it, so we have to choose one.  If one of the pCD's currently has
     * the Colormap Focus, then let's use that one.  Otherwise, just use
     * the 1st one.
     */
    {
        int j;
        for (j = 0; cmap_window_data[j]; j++)
        {
            if (ACTIVE_PSD->colormapFocus == cmap_window_data[j])
            {
                pCD = cmap_window_data[j];
                break;
            }
        }
        /*
         * None of the pCD's in the list have Colormap Focus.  So, just
         * set pCD to the 1st one in the list.
         */
        if (!pCD)
            pCD = cmap_window_data[0];
    }
#endif

    /*
     *  Set active screen if this is not a FocusOut event.
     *  We don't need to set it on focus out AND we use
     *  (SCREEN_FOR_CLIENT(pCD) != ACTIVE_SCREEN) in
     *  in HandleCFocusOut to determine if a new colormap needs
     *  to be installed.
     */

    if (!(event->type == FocusOut))
    {
	SetActiveScreen (PSD_FOR_CLIENT(pCD));
    }
#ifdef WSM
    /* Get workspace specific client data */
    SetClientWsIndex (pCD);
#endif /* WSM */

    /*
     * Handle events on top-level client windows.
     */

    if (event->xany.window == pCD->client)
    {
	return (HandleEventsOnClientWindow (pCD, event));
    }

    /*
     * Handle events on windows created by mwm (for icons and client window
     * frames) and on non-top-level client windows (e.g., colormap
     * windows).
     */

    switch (event->type)
    {
	case ButtonPress:
	{
	    dispatchEvent = HandleCButtonPress (pCD, (XButtonEvent *)event);
	    break;
	}

	case ButtonRelease:
	{
	    if (wmGD.menuActive)
	    {
		dispatchEvent = True; /* have the toolkit dispatch the event */
	    }
	    else
	    {
		HandleCButtonRelease (pCD, (XButtonEvent *)event);
	    }
	    break;
	}

	case KeyPress:
	{
	    dispatchEvent = HandleCKeyPress (pCD, (XKeyEvent *)event);
	    break;
	}

	case MotionNotify:
	{
	    HandleCMotionNotify (pCD, (XMotionEvent *)event);
	    break;
	}

	case Expose:
	{
	    /* 
	     * If multiple expose events, wait for last one.
	     */

	    if (event->xexpose.count == 0) 
	    {
		if (event->xexpose.window == ICON_FRAME_WIN(pCD))
		{
		    IconExposureProc (pCD, True);
		    if (P_ICON_BOX(pCD))
		    {
			dispatchEvent = True;
		    }
		}
		else if (event->xexpose.window == 
			     pCD->pSD->activeIconTextWin)
		{
		    PaintActiveIconText (pCD, FALSE);
		}
		else if (!(pCD->clientFlags & CLIENT_DESTROYED))
		{
		    if ((event->xexpose.window == pCD->clientFrameWin) ||
			 (event->xexpose.window == pCD->clientTitleWin))
		    {
			FrameExposureProc (pCD);
		    }
		    if (event->xexpose.window == pCD->clientBaseWin)
		    {
			BaseWinExposureProc (pCD);
		    }
		}
#ifdef PANELIST
		else if (pCD->clientFlags & FRONT_PANEL_BOX)
		{
	        /*
		 *
		 *  Then this client is the shell for the 
		 *  front panel and we want the toolkit to repaint
		 *  it.
		 *
		 */
		    dispatchEvent = True;
		}
#endif /* PANELIST */
	    }
	    break;
	}

	case EnterNotify:
	{
	    HandleCEnterNotify (pCD, (XEnterWindowEvent *)event);
	    break;
	}

	case LeaveNotify:
	{
	    HandleCLeaveNotify (pCD, (XLeaveWindowEvent *)event);
	    break;
	}

	case FocusIn:
	{
	    dispatchEvent = HandleCFocusIn (pCD, (XFocusChangeEvent *)event);
	    break;
	}

	case FocusOut:
	{
	    dispatchEvent = HandleCFocusOut (pCD, (XFocusChangeEvent *)event);
	    break;
	}

	case DestroyNotify:
	{
	    if (((XDestroyWindowEvent *)event)->window == pCD->client)
	    {
	        pCD->clientFlags |= CLIENT_DESTROYED;
	        UnManageWindow (pCD);
	    }
	    break;
	}

	case UnmapNotify:
	{
	    /*
	     * This event is generated when a managed  client window is 
	     * unmapped by the client or when the window manager unmaps the
	     * client window; check the wmMapCount to determine if this is
	     * the result of a window manager unmap. If this is a client
	     * unmap then the window is to be withdrawn from window manager
	     * control.
	     */

	    if (((XUnmapEvent *)event)->window == pCD->client)
	    {
	        if (pCD->wmUnmapCount)
	        {
		    pCD->wmUnmapCount--;
	        }
	        else
	        {
		    UnManageWindow (pCD);
	        }
            }
	    break;
	}

	case MapRequest:
	{
	    /*
	     * This is a request to change the state of the client window from
	     * iconic (minimized) to normal.
	     */
#ifdef WSM
            if (!ClientInWorkspace (ACTIVE_WS, pCD))
	    {
		if (pCD->absentMapBehavior == AMAP_BEHAVIOR_IGNORE)
		{
		    SetClientState (pCD, NORMAL_STATE|UNSEEN_STATE, 
				    GetTimestamp ());
		}
		else
		{
		    HonorAbsentMapBehavior(pCD);
		    SetClientState (pCD, NORMAL_STATE, GetTimestamp ());
		}
	    }
	    else
	    {
		SetClientState (pCD, NORMAL_STATE, GetTimestamp ());
	    }
#else /* WSM */
	    SetClientState (pCD, NORMAL_STATE, GetTimestamp ());
#endif /* WSM */
	    break;
	}

	case ConfigureRequest:
	{
	    HandleCConfigureRequest (pCD, (XConfigureRequestEvent *)event);
	    break;
	}

	case ColormapNotify:
	{
	    /*
	     * Process changes to client window colormaps:
	     */

	    HandleCColormapNotify (pCD, (XColormapEvent *)event);
	    break;
	}

	case ClientMessage:
	{
	    /*
	     * Handle client message events.
	     */

	    HandleClientMessage (pCD, (XClientMessageEvent *)event);
	    break;
	}
	case ReparentNotify:
	{
	    if ((((XReparentEvent *)event)->window == pCD->client) &&
		(((XReparentEvent *)event)->parent != pCD->clientBaseWin))
	    {
		/*
		 * The window was reparented away from the frame.
		 * Unmanage to clean up the now empty frame.
		 *
		 * Note: We get here when the reparent is done while
		 * the client is unmapped (e.g. iconified). Otherwise
		 * the reparent will generate an UnmapNotify which
		 * will also cause us to unmanage the client.
		 */
		UnManageWindow (pCD);
	      }
	    break;
	}
    } /* end of event.type switch */


    return (dispatchEvent);


} /* END OF FUNCTION WmDispatchClientEvent */



/*************************************<->*************************************
 *
 *  HandleEventsOnSpecialWindows (pEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handles events on special window manager windows and "leftover" events
 *  from destroyed client window frames.
 *
 *
 *  Inputs:
 *  ------
 *  pEvent = pointer to an XEvent structure
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

Boolean HandleEventsOnSpecialWindows (XEvent *pEvent)
{
    Boolean dispatchEvent = True;
#ifdef WSM
    WmScreenData *pSD;
#endif /* WSM */


    /*
     * The window is not a root window or a client frame window.  Check for
     * a special window manager window.  Have the toolkit dispatch the event
     * if the event is not on a special window.
     */

    if (pEvent->xany.window == ACTIVE_ROOT)
    {
	if (pEvent->type == FocusIn)
	{
	    SetKeyboardFocus ((ClientData *) NULL, REFRESH_LAST_FOCUS);
	}
    }
    else if (pEvent->xany.window == ACTIVE_PSD->feedbackWin)
    {
	if (pEvent->type == Expose)
	{
	    if (pEvent->xexpose.count == 0)
	    {
	        PaintFeedbackWindow(ACTIVE_PSD);
	    }
	}
	dispatchEvent = False; /* don't have the toolkit dispatch the event */
    }
    else if (pEvent->xany.window == ACTIVE_PSD->inputScreenWindow)
    {
	if (pEvent->type == ButtonPress)
	{
	    F_Beep (NULL, (ClientData *) NULL, (XEvent *) NULL);
	}
	else if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) &&
		 (pEvent->type == EnterNotify))
	{
	    HandleWsEnterNotify ((XEnterWindowEvent *)pEvent);
	}
	dispatchEvent = False; /* don't have the toolkit dispatch the event */
    }
#ifdef  WSM
    else if (!XFindContext (DISPLAY, pEvent->xany.window,
		    wmGD.mwmWindowContextType, (caddr_t *)&pSD))
    {
	if ((pEvent->type == PropertyNotify) &&
	    (pEvent->xproperty.atom == wmGD.xa_DT_WM_REQUEST) &&
	    (pEvent->xproperty.state == PropertyNewValue))
	{
	    HandleDtWmRequest (pSD, pEvent);
	}
	if (pEvent->type == ClientMessage)
	{
	    HandleDtWmClientMessage ((XClientMessageEvent *)pEvent);
	}
    }
#endif /* WSM */
    else
    {
	/*
	 * Events may come in for a client frame base window that no
	 * longer exists (the client window was just unmanaged but the
	 * the client did some action before the un-reparenting was
	 * actually done).  Confirm that this is the case and then
	 * handle the request as if it came in as a root window event.
	 */

	switch (pEvent->type)
	{
	    case ConfigureRequest:
	    {
		if (GetParentWindow (pEvent->xconfigurerequest.window) ==
		    ACTIVE_ROOT)
		{
		    /*
		     * This is an event for a client base window that
		     * no longer exists.  Handle the event as if it is a
		     * root window event.
		     */

		    dispatchEvent =  WmDispatchWsEvent (pEvent);
		}
		break;
	    }

	    case MapRequest:
	    {
		if (GetParentWindow (pEvent->xmaprequest.window) ==
		    ACTIVE_ROOT)
		{
		    /*
		     * This is an event for a client base window that
		     * no longer exists.  Handle the event as if it is a
		     * root window event.
		     */

		    dispatchEvent = WmDispatchWsEvent (pEvent);
		}
		break;
	    }
	}
    }

    return (dispatchEvent);

} /* END OF FUNCTION HandleEventsOnSpecialWindows */



/*************************************<->*************************************
 *
 *  HandleEventsOnClientWindow (pCD, pEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handles events on a client top-level window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  pEvent = pointer to an XEvent structure
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

Boolean HandleEventsOnClientWindow (ClientData *pCD, XEvent *pEvent)
{
    Boolean doXtDispatchEvent = True;

#ifndef NO_SHAPE
    if (pEvent->type == (wmGD.shapeEventBase+ShapeNotify))
    {
        HandleCShapeNotify (pCD, (XShapeEvent *)pEvent);
    }
    else
#endif /* NO_SHAPE */
    switch (pEvent->type)
    {
	case ColormapNotify:
	{
	    /*
	     * Process changes to top-level client window colormaps:
	     */

	    HandleCColormapNotify (pCD, (XColormapEvent *)pEvent);
	    doXtDispatchEvent = False;
	    break;
	}

	case PropertyNotify:
	{
	    /*
	     * Process property changes on managed client windows:
	     */
		
	    HandleCPropertyNotify (pCD, (XPropertyEvent *)pEvent);
	    doXtDispatchEvent = False;
	    break;
	}

	case ClientMessage:
	{
	    /*
	     * Handle client message events.
	     */

	    HandleClientMessage (pCD, (XClientMessageEvent *)pEvent);
	    break;
	}

    }

    return (doXtDispatchEvent);


} /* END OF FUNCTION HandleEventsOnClientWindow */



/*************************************<->*************************************
 *
 *  HandleCPropertyNotify (pCD, propertyEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function handles propertyNotify events (indicating window property
 *  changes) that are reported to the client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the client window that got the event
 *
 *  propertyEvent = propertyNotify event that was received
 *
 *************************************<->***********************************/

void HandleCPropertyNotify (ClientData *pCD, XPropertyEvent *propertyEvent)
{

    switch (propertyEvent->atom)
    {
        case XA_WM_HINTS:
	{
	    ProcessWmHints (pCD, FALSE /*not first time*/);
	    break;
	}
	
        case XA_WM_NORMAL_HINTS:
	{
	    ProcessWmNormalHints (pCD, FALSE /*not first time*/, 0);
	    break;
	}
	
        case XA_WM_NAME:
	{
	    ProcessWmWindowTitle (pCD, FALSE /*not first time*/);
	    break;
	}
	
        case XA_WM_ICON_NAME:
	{
	    ProcessWmIconTitle (pCD, FALSE /*not first time*/);
	    break;
	}
	
        case XA_WM_CLASS:
	{

	    break;
	}
	
        case XA_WM_COMMAND:
	{
	    if (pCD->clientFlags & CLIENT_TERMINATING)
	    {
		DeleteClientWmTimers (pCD);
		XKillClient (DISPLAY, pCD->client);
	    }
	    break;
	}

        case XA_WM_TRANSIENT_FOR:
        {
	    /*
	     * here we handle the special case of dialogs that are
	     * mapped before the windows they are transient for are
	     * mapped.  Xm handles this case by waiting for the
	     * transient_for window to appear before setting the
	     * WM_TRANSIENT_FOR property on the dialog.  Mwm has to
	     * notice this property change and re-organize things
	     * so the dialog is treated as a transient window.
	     *
	     * Note that we also handle the case of the WM_TRANSIENT_FOR
	     * property being removed. 
	     */
	    DeleteClientFromList (pCD->pSD->pActiveWS, pCD);
	    ProcessWmTransientFor(pCD);
	    AddClientToList(pCD->pSD->pActiveWS, pCD, True);
	    if (pCD->transientLeader != NULL)
		StackTransientWindow(pCD);
	    break;
	}
	
	default:
	{
	    if (propertyEvent->atom == wmGD.xa_WM_PROTOCOLS)
	    {
		ProcessWmProtocols (pCD);
	    }
#ifdef WSM
	    else if (propertyEvent->atom == wmGD.xa_DT_WORKSPACE_HINTS)
	    {
		(void) ProcessWorkspaceHints (pCD);
	    }
#endif /* WSM */
	    else if (propertyEvent->atom == wmGD.xa_MWM_MESSAGES)
	    {
		if (pCD->protocolFlags & PROTOCOL_MWM_MESSAGES)
		{
		    ProcessMwmMessages (pCD);
		}
	    }
	    else if (propertyEvent->atom == wmGD.xa_SM_CLIENT_ID)
	    {
		ProcessSmClientID(pCD);
	    }
	    else if (propertyEvent->atom == wmGD.xa_WMSAVE_HINT)
	    {
		ProcessWmSaveHint(pCD);
	    }
	    else if (propertyEvent->atom == wmGD.xa_WM_COLORMAP_WINDOWS)
	    {
		if (propertyEvent->state == PropertyNewValue)
		{
		    ProcessWmColormapWindows (pCD);
		}
		else
		{
		    /* property was deleted */
		    ResetColormapData (pCD, NULL, 0);
		}

		if ((ACTIVE_PSD->colormapFocus == pCD) &&
		    ((pCD->clientState == NORMAL_STATE) ||
		     (pCD->clientState == MAXIMIZED_STATE)))
		{
		    /*
		     * The client window has the colormap focus, install the
		     * colormap.
		     */
#ifndef OLD_COLORMAP /* colormap */
		    /*
		     * We just changed the colormaps list,
		     * so we need to re-run the whole thing.
		     */
		    pCD->clientCmapFlagsInitialized = 0;
		    ProcessColormapList (ACTIVE_PSD, pCD);
#else /* OSF original */
		    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
#endif
		}
	    }
	    break;
	}
    }

} /* END OF FUNCTION HandleCPropertyNotify */



/*************************************<->*************************************
 *
 *  HandleCButtonPress (pCD, buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a button
 *  press event on the client window (including frame) or icon.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (identifies client window)
 *
 *  buttonEvent = ButtonPress event on client window
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the event should be dispatched by XtDispatchEvent
 *
 *************************************<->***********************************/

Boolean HandleCButtonPress (ClientData *pCD, XButtonEvent *buttonEvent)
{
    Boolean dispatchEvent = False;
    Boolean replayEvent = True;
    Context context;
    int partContext;
    Context subContext;
    static Time baseWinTime = 0;
    static unsigned int baseWinButton = 0;

    wmGD.passButtonsCheck = True;

    /*
     * Find out the event context and process the event accordingly.
     * If the event is due to a key focus selection grab or an application
     * modal grab then handle the grab (only these types of grabs are
     * done on the client window frame base window)..
     */

    if (wmGD.menuActive)
    {
	dispatchEvent = True;	/* have the toolkit dispatch the event */
    }
    else
    {
	IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
	subContext = (1L << partContext);

        if (buttonEvent->window == pCD->clientBaseWin)
        {
            /* save time of event caught by base window grab */
            baseWinTime = buttonEvent->time;
            baseWinButton = buttonEvent->button;
	}
 
        /*
         * If this event was caught by the base window grab and
         * replayed, then don't reprocess if caught by the frame
         * window. (Replayed events have the same time.)
         */
        if (!((buttonEvent->window == pCD->clientFrameWin) &&
              (buttonEvent->button == baseWinButton) &&
              (buttonEvent->time == baseWinTime)))
	{

#ifndef MOTIF_ONE_DOT_ONE
	    /*
	     * Motif 1.2, ignore replayed events UNPOST_AND_REPLAY events
	     * generated from the menu system (time stamps are exactly
	     * the same for the replayed event)
	     */

	    if (wmGD.clickData.time == buttonEvent->time)
	    {
		dispatchEvent = False;
	    }
	    else
	    {
		ProcessClickBPress (buttonEvent, pCD, context, subContext);
	    }
#else
	    ProcessClickBPress (buttonEvent, pCD, context, subContext);
#endif

	    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) 
		&& pCD)
	    {
		/*
		 * Button bindings have been processed, now check for bindings
		 * that associated with the built-in semantics of the window
		 * frame decorations.
		 */

		CheckButtonPressBuiltin (buttonEvent, context, subContext,
		    partContext, pCD);

		/*
		 * For case where button action causes lower, but
		 * builtin causes focus - disable auto raise until
		 * we receive focusIn or focusOut.
		 */
		pCD->focusAutoRaiseDisablePending = False;
	    }
	    else
	    {
	       /*
		* Else skip built-in processing due to execution of a function
		* that does on-going event processing or that has changed the
		* client state (e.g., f.move or f.minimize).
		*/

		replayEvent = False;
	    }
	}
    }

    if (buttonEvent->window == pCD->clientBaseWin)
    {
	ProcessButtonGrabOnClient (pCD, buttonEvent, replayEvent);
    }

    return (dispatchEvent);


} /* END OF FUNCTION HandleCButtonPress */



/*************************************<->*************************************
 *
 *  ProcessButtonGrabOnClient (pCD, buttonEvent, replayEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function handles an activated button grab on the client window
 *  frame base window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of window associated with the grab
 *
 *  buttonEvent = ButtonPress event on client window
 *
 *  replayEvent = True if event should be replayed
 *
 *************************************<->***********************************/

void ProcessButtonGrabOnClient (ClientData *pCD, XButtonEvent *buttonEvent, Boolean replayEvent)
{
    ButtonSpec *buttonSpec;
    Boolean passButton;



    if ((buttonEvent->button == SELECT_BUTTON) && 
	((buttonEvent->state == 0) ||
	 (NOLOCKMOD(buttonEvent->state) == 0)))
    {
	passButton = wmGD.passSelectButton;
    }
    else
    {
	passButton = wmGD.passButtons;
    }

    if (IS_APP_MODALIZED(pCD) || !passButton)
    {
	replayEvent = False;
    }
    else if (replayEvent)
    {
	/*
	 * Replay the event as long as there is not another button binding
	 * for the button release.
	 */

	buttonSpec = ACTIVE_PSD->buttonSpecs;
	while (buttonSpec)
	{
	    if ((buttonSpec->eventType == ButtonRelease) &&
		((buttonEvent->state == buttonSpec->state) ||
		 (NOLOCKMOD(buttonEvent->state) == buttonSpec->state)) &&
		(buttonEvent->button == buttonSpec->button))
	    {
		replayEvent = False;
		break;
	    }

	    buttonSpec = buttonSpec->nextButtonSpec;
	}
    }

    if (replayEvent && wmGD.passButtonsCheck)
    {
	XAllowEvents (DISPLAY, ReplayPointer, CurrentTime);
    }
    else
    {
	if (IS_APP_MODALIZED(pCD))
	{
	    /*
	     * The grab is done on a window that has an application modal
	     * secondary window.  Beep to indicate no client processing of
	     * the event.
	     */

	    F_Beep (NULL, pCD, (XEvent *) NULL);
	}

	XAllowEvents (DISPLAY, AsyncPointer, CurrentTime);
    }
    XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);

} /* END OF FUNCTION ProcessButtonGrabOnClient */



/*************************************<->*************************************
 *
 *  CheckButtonPressBuiltin (buttonEvent, context, subContext, partContext, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a built-in window manager function
 *  has been selected.  If yes, then the function is done.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to button event
 *
 *  context = button event context (root, icon, window)
 *
 *  subContext = button event subcontext (title, system button, ...)
 *
 *  partContext = part context within a window manager component
 *
 *************************************<->***********************************/

void CheckButtonPressBuiltin (XButtonEvent *buttonEvent, Context context, Context subContext, int partContext, ClientData *pCD)
{
    /*
     * All builtin button bindings are based on button 1 with no
     * modifiers. (Ignore locking modifiers)
     */

    if (((buttonEvent->button != SELECT_BUTTON)  && 
	 (buttonEvent->button != DMANIP_BUTTON)) || 
	   NOLOCKMOD(buttonEvent->state))
    {
	return;
    }


    /*
     * Process the builtin button bindings based on the window manager
     * component that was selected.
     */

    if (context & F_CONTEXT_ICON)
    {
	HandleIconButtonPress (pCD, buttonEvent);
    }
    else if (context & F_CONTEXT_ICONBOX)
    {
	HandleIconBoxButtonPress (pCD, buttonEvent, subContext);
    }
    else if (context & F_CONTEXT_WINDOW)
    {
	/*
	 * A client window frame component was selected.
	 */

	/*
	 * If the keyboard focus policy is explicit then all window frame
	 * components set the keyboard input focus when selected.
	 */

	if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
	{
	    /* If we've just done f.lower, disable focusAutoRaise. */
	    if (pCD && pCD->focusAutoRaiseDisablePending)
	      pCD->focusAutoRaiseDisabled = True;

	    Do_Focus_Key (pCD, buttonEvent->time,
		(long)((partContext == FRAME_CLIENT) ? CLIENT_AREA_FOCUS : 0));
	}


	/*
         * Process the builtin button bindings based on the client window
	 * frame component that was selected.
	 */

	if ((buttonEvent->button == SELECT_BUTTON) && 
	    (subContext == F_SUBCONTEXT_W_SYSTEM))
	{
	    int flags = 0;

            /*
	     * System menu button component:
             * SELECT_BUTTON Press - post the system menu.
	     * SELECT_BUTTON double-click - close the window.
	     */

	    PushGadgetIn (pCD, partContext);

	    if ((wmGD.clickData.doubleClickContext == F_SUBCONTEXT_W_SYSTEM) &&
	        wmGD.systemButtonClick2 &&
	        (pCD->clientFunctions & MWM_FUNC_CLOSE))
	    {
	        /*
	         * Close the client window.  Don't do any of the other
	         * system menu button actions.
	         */

		wmGD.clickData.clickPending = False;
		wmGD.clickData.doubleClickPending = False;
		F_Kill (NULL, pCD, (XEvent *) buttonEvent);
		return;
	    }

	    if (pCD->clientState == NORMAL_STATE)
	    {
		context = F_CONTEXT_NORMAL;
	    }
	    else if (pCD->clientState == MAXIMIZED_STATE)
	    {
	        context = F_CONTEXT_MAXIMIZE;
	    }
	    else
	    {
	        context = F_CONTEXT_ICON;
	    }

	    /*
	     * Set up for "sticky" menu processing if specified.
	     */
	    if (wmGD.systemButtonClick)
	    {
		wmGD.checkHotspot = True;
		flags |= POST_STICKY;
	    }

            pCD->grabContext = context;

	    PostMenu (pCD->systemMenuSpec, pCD, 0, 0, SELECT_BUTTON, 
		      context, flags, (XEvent *)buttonEvent);

	}
	else if (subContext == F_SUBCONTEXT_W_TITLE)
	{
            /*
	     * Title component:
             * SELECT_BUTTON  or DMANIP_BUTTON Press - 
	     *               start looking for a move.
             */

	    PushGadgetIn (pCD, partContext);

/*
 * Fix for 5075 - Check to make sure that MWM_FUNC_MOVE is set in the
 *                clientFunctions.  This is necessary because the title
 *                bar is added based on a number of decorations even if
 *                the resources or the user has specifically requested
 *                that "move" not be one of them.
 */
            if (pCD->clientFunctions & MWM_FUNC_MOVE)
            {
	      wmGD.preMove = True;
	      wmGD.preMoveX = buttonEvent->x_root;
	      wmGD.preMoveY = buttonEvent->y_root;
	      wmGD.configButton = buttonEvent->button;
	      wmGD.configAction = MOVE_CLIENT;
            }
/*
 * End fix 5075
 */

	}
	else if (subContext & F_SUBCONTEXT_W_RBORDER)
	{
            /*
	     * Resize border handle components:
             * SELECT_BUTTON or DMANIP_BUTTON Press - 
	     *              start looking for a resize.
             */

	    wmGD.preMove = True;
	    wmGD.preMoveX = buttonEvent->x_root;
	    wmGD.preMoveY = buttonEvent->y_root;
	    wmGD.configButton = buttonEvent->button;
	    wmGD.configAction = RESIZE_CLIENT;
	    wmGD.configPart = partContext;
	    wmGD.configSet = True;
	}
	else if ((buttonEvent->button == SELECT_BUTTON) &&
            (subContext & (F_SUBCONTEXT_W_MINIMIZE|F_SUBCONTEXT_W_MAXIMIZE)))
	{
            /*
	     * Minimize and maximize button components:
             * SELECT_BUTTON Press - start of a click.
             */

	    PushGadgetIn (pCD, partContext);
	}
	   
	/*
	 * Other components: no action
	 */
    }

} /* END OF FUNCTION CheckButtonPressBuiltin */



/*************************************<->*************************************
 *
 *  HandleIconButtonPress (pCD, buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function handles builtin functions in the icon context.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of the icon that received the button event
 *
 *  buttonEvent = pointer to the button event that occurred
 *
 *************************************<->***********************************/

void HandleIconButtonPress (ClientData *pCD, XButtonEvent *buttonEvent)
{
    int newState;

    /*
     * Do icon component button press actions:
     * Button 1 press - set the keyboard input focus if policy is explicit
     * Button 1 double-click - normalize the icon
     */

    if (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_I_ALL)
    {
        /*
         * A double-click was done, normalize the icon.
         */

	if (pCD->maxConfig)
	{
	    newState = MAXIMIZED_STATE;
	}
	else
	{
	    newState = NORMAL_STATE;
	}

        SetClientState (pCD, newState, buttonEvent->time);
	wmGD.clickData.clickPending = False;
	wmGD.clickData.doubleClickPending = False;
    }
    else
    {
        /*
         * This is a regular button press (it may be the start of a 
         * double-click).  Set the focus and top the icon if appropriate.
         */

        if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
        {
	    Do_Focus_Key (pCD, buttonEvent->time, ALWAYS_SET_FOCUS);
        }


        /*
         * Indicate that a move may be starting; wait for button motion
         * events before moving the icon.
         */

        wmGD.preMove = True;
        wmGD.preMoveX = buttonEvent->x_root;
        wmGD.preMoveY = buttonEvent->y_root;
        wmGD.configButton = buttonEvent->button;
        wmGD.configAction = MOVE_CLIENT;
    }


} /* END OF FUNCTION HandleIconButtonPress */



/*************************************<->*************************************
 *
 *  HandleIconBoxButtonPress (pCD, buttonEvent, subContext)
 *
 *
 *  Description:
 *  -----------
 *  This function handles builtin functions in the iconbox context.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data of the icon that received the button event
 *
 *  buttonEvent = pointer to the button event that occurred
 *
 *  subContext = context id of event location inside icon box
 *
 *************************************<->***********************************/

void HandleIconBoxButtonPress (ClientData *pCD, XButtonEvent *buttonEvent, Context subContext)
{

    /*
     * Do iconbox icon component button press actions:
     * Button 1 press - select the icon
     * Button 1 double-click - normalize the icon or raise the window
     */

    if ((wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_IICON) ||
       (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_WICON))
    {
	F_Restore_And_Raise ((String)NULL, pCD, (XEvent *)NULL);
    }
    else if ((subContext == F_SUBCONTEXT_IB_IICON) ||
	     (subContext == F_SUBCONTEXT_IB_WICON))
    {
	/*
	 * Indicate that a move may be starting; wait for button motion
	 * events before moving the icon.
	 */

	wmGD.preMove = True;
	wmGD.preMoveX = buttonEvent->x_root;
	wmGD.preMoveY = buttonEvent->y_root;
	wmGD.configButton = buttonEvent->button;
	wmGD.configAction = MOVE_CLIENT;
    }

    /*
     * Do icon box icon actions:
     * Button 1 press - select the icon in the icon box
     */

    /*
     * XmProcessTraversal will move the selection cursor to the
     * widget that was "boinked" with the mouse
     */

    if ((P_ICON_BOX(pCD)->pCD_iconBox == wmGD.keyboardFocus) ||
	(P_ICON_BOX(pCD)->pCD_iconBox == wmGD.nextKeyboardFocus))
    {
	XmProcessTraversal (XtWindowToWidget(DISPLAY, ICON_FRAME_WIN(pCD)), 
			    XmTRAVERSE_CURRENT);
    }


} /* END OF FUNCTION HandleIconBoxButtonPress */



/*************************************<->*************************************
 *
 *  HandleCButtonRelease (pCD, buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a button
 *  release event on the client window (including frame) or icon.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for the window/icon that got the event
 *
 *  buttonEvent = pointer to the button event that occurred
 *
 *  Comments:
 *  ---------
 *  Skip builtin processing if move or resize button actions were started
 *  due to button-up bindings.
 *
 *************************************<->***********************************/

void HandleCButtonRelease (ClientData *pCD, XButtonEvent *buttonEvent)
{
    Context context;
    Context subContext;
    int partContext;


    /*
     * Find out whether the event was on the client window frame or the icon
     * and process the event accordingly.
     */

    IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
    subContext = (1L << partContext);

    ProcessClickBRelease (buttonEvent, pCD, context, subContext);

    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) && pCD)
    {
	/*
	 * Button bindings have been processed, now check for bindings
	 * that associated with the built-in semantics of the window
	 * frame decorations.
	 */

        CheckButtonReleaseBuiltin (buttonEvent, context, subContext, pCD);
    }
    /*
     * Else skip built-in processing due to execution of a function that
     * does on-going event processing or that has changed the client state
     * (e.g., f.move or f.minimize).
     */


    /* clear preMove state */
    wmGD.preMove = False;


} /* END OF FUNCTION HandleCButtonRelease */



/*************************************<->*************************************
 *
 *  HandleCKeyPress (pCD, keyEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a key
 *  press event on the client window (including frame) or icon.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for the window/icon that got the event
 *
 *  keyEvent = pointer to the key event that occurred
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the event should be dispatched by XtDispatchEvent
 *
 *************************************<->***********************************/

Boolean HandleCKeyPress (ClientData *pCD, XKeyEvent *keyEvent)
{
    Boolean dispatchEvent = False;
    Boolean checkKeyEvent = True;


    if (wmGD.menuActive)
    {
	/*
	 * The active menu accelerators have been checked and keyEvent was
	 * not one of them.  We will check for an iconbox icon widget key and
	 * for pass keys mode and then have the toolkit dispatch the event, 
	 * without rechecking the client accelerator list.
	 */

	dispatchEvent = True;
	checkKeyEvent = False;
    }

    /*
     * If pass keys is active then only check for getting out of the pass
     * keys mode if the event is on the client frame or icon frame window.
     * Unfreeze the keyboard and replay the key if pass keys is active.
     */

    if (((keyEvent->window == ICON_FRAME_WIN(pCD)) ||
	 (keyEvent->window == pCD->pSD->activeIconTextWin)) &&
	P_ICON_BOX(pCD))
    {
	/*
	 * This is a non-grabbed key that is intended for the icon widget
	 * in the iconbox.
	 */

	dispatchEvent = True; /* have the toolkit dispatch the event */
	checkKeyEvent = False;
	if (keyEvent->window == pCD->pSD->activeIconTextWin)
	{
	    /*
	     * The event is really for the icon, not the active
	     * label, so ... correct the window id 
	     */

	    keyEvent->window = ICON_FRAME_WIN(pCD);
	}
    }
    else if (wmGD.passKeysActive)
    {
	if (wmGD.passKeysKeySpec &&
	    ((wmGD.passKeysKeySpec->state == keyEvent->state) ||
	     (wmGD.passKeysKeySpec->state == NOLOCKMOD(keyEvent->state))) &&
	    (wmGD.passKeysKeySpec->keycode == keyEvent->keycode))
	{
	    /*
	     * Get out of the pass keys mode.
	     */

	    F_Pass_Key (NULL, (ClientData *) NULL, (XEvent *) NULL);
	    XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);
	}
	else
	{
	    XAllowEvents (DISPLAY, ReplayKeyboard, CurrentTime);
	}
	checkKeyEvent = False;
    }
    else
    {
	XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);
    }


    /*
     * Check for a "general" key binding that has been set only for the
     * icon context.  These key bindings are set with the keyBinding
     * resource or as accelerators in icon context menus.
     */

    if (checkKeyEvent && (keyEvent->window == ICON_FRAME_WIN(pCD)))
    {
	if ((checkKeyEvent = HandleKeyPress (keyEvent, 
					     ACTIVE_PSD->keySpecs, True,
					     F_CONTEXT_ICON, False,
					     (ClientData *)NULL))
	    && ACTIVE_PSD->acceleratorMenuCount)
	{
	    int n;

	    for (n = 0; ((keyEvent->keycode != 0) &&
			 (n < ACTIVE_PSD->acceleratorMenuCount)); n++)
	    {
		if (!HandleKeyPress (keyEvent,
		           ACTIVE_PSD->acceleratorMenuSpecs[n]->accelKeySpecs,
			   True, F_CONTEXT_ICON, True,(ClientData *)NULL))
		{
		    checkKeyEvent = False;
		    break;
		}
	    }
	}
    }

    /*
     * Check for a key binding that has been set as an accelerator in the
     * system menu.  We only do the first accelerator found.
     */

    if (checkKeyEvent && pCD->systemMenuSpec &&
        (pCD->systemMenuSpec->accelKeySpecs))
    {
	HandleKeyPress (keyEvent, pCD->systemMenuSpec->accelKeySpecs,
			FALSE, 0, TRUE,(ClientData *)NULL );
    }

    return (dispatchEvent);

} /* END OF FUNCTION HandleCKeyPress */



/*************************************<->*************************************
 *
 *  CheckButtonReleaseBuiltin (buttonEvent, context, subContext, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a built-in window manager function
 *  has been activated as a result of a button release. If yes, then the
 *  associated function is done.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button release event
 *
 *  context = button event context (root, icon, window)
 *
 *  subContext = button event subcontext (title, system button, ...)
 *
 *  pCD = pointer to client data for the window/icon that got the event
 *
 *************************************<->***********************************/

void CheckButtonReleaseBuiltin (XButtonEvent *buttonEvent, Context context, Context subContext, ClientData *pCD)
{
    /*
     * All builtin button buindings are based on button 1 with no modifiers.
     * (Ignore locking modifiers).
     *
     * Test the event for a ``button up'' transition on buttons we are
     * interested in.
     */

    if (!((buttonEvent->button == SELECT_BUTTON) &&
	  (NOLOCKMOD(buttonEvent->state) == SELECT_BUTTON_MASK)) &&
	!((buttonEvent->button == DMANIP_BUTTON) &&
	  (NOLOCKMOD(buttonEvent->state) == DMANIP_BUTTON_MASK)))
    {
	return;
    }


    /*
     * Process the builtin button bindings based on the window manager
     * component that was selected.
     */

    if ((buttonEvent->button == SELECT_BUTTON) &&
	(context & F_CONTEXT_ICON))
    {
	/*
	 * Do the icon component button release actions:
	 * SELECT_BUTTON click - post the system menu if specified.
	 */

	if (wmGD.iconClick &&
	    (wmGD.clickData.clickContext == F_SUBCONTEXT_I_ALL))
	{
	    wmGD.checkHotspot = True;

	    /*
	     * Post the system menu with traversal on (Button 1 should be
	     * used to manipulate the menu).
	     */
	    pCD->grabContext = F_CONTEXT_ICON;
	    PostMenu (pCD->systemMenuSpec, pCD, 0, 0, NoButton, 
		      F_CONTEXT_ICON, POST_STICKY, (XEvent *)buttonEvent);
	}
    }
/* post menu from icon in iconbox */
    else if ((buttonEvent->button == SELECT_BUTTON) &&
	     (context & F_CONTEXT_ICONBOX))
    {
        if ((wmGD.iconClick)  &&
            (((pCD->clientState == MINIMIZED_STATE)  &&
	      (wmGD.clickData.clickContext == F_SUBCONTEXT_IB_IICON)) ||
	     (wmGD.clickData.clickContext == F_SUBCONTEXT_IB_WICON))  )
        {
            wmGD.checkHotspot = True;
	    
            /*
             * Post the system menu with traversal on (Button 1 should be
             * used to manipulate the menu.
             */
            if ((wmGD.clickData.clickContext == F_SUBCONTEXT_IB_IICON) &&
                (pCD->clientState == MINIMIZED_STATE))
            {
		pCD->grabContext = F_SUBCONTEXT_IB_IICON;
                PostMenu (pCD->systemMenuSpec, pCD, 0, 0, NoButton,
                          F_SUBCONTEXT_IB_IICON, POST_STICKY, (XEvent *)buttonEvent);
            }
            else
            {
		pCD->grabContext = F_SUBCONTEXT_IB_WICON;
                PostMenu (pCD->systemMenuSpec, pCD, 0, 0, NoButton,
                          F_SUBCONTEXT_IB_WICON, POST_STICKY, (XEvent *)buttonEvent);
            }
        }
    }
/* end of post menu from icon in iconbox */
    else if (context & F_CONTEXT_WINDOW)
    {
	/*
	 * The button release is on a client window frame component.
	 */

	if ((buttonEvent->button == SELECT_BUTTON) &&
	    (subContext == F_SUBCONTEXT_W_MINIMIZE))
	{
	    /*
	     * Minimize button:
	     * Button 1 click - minimize the window.
	     */

	    if (wmGD.clickData.clickContext == F_SUBCONTEXT_W_MINIMIZE)
	    {
		SetClientState (pCD, MINIMIZED_STATE, buttonEvent->time);
	    }
	}
	else if ((buttonEvent->button == SELECT_BUTTON) &&
	         (subContext == F_SUBCONTEXT_W_MAXIMIZE))
	{
	    /*
	     * Maximize button:
	     * Button 1 click - maximize the window.
	     */

	    if (wmGD.clickData.clickContext == F_SUBCONTEXT_W_MAXIMIZE)
	    {
		if (pCD->clientState == NORMAL_STATE)
		{
	            SetClientState (pCD, MAXIMIZED_STATE, buttonEvent->time);
		}
		else
		{
		    SetClientState (pCD, NORMAL_STATE, buttonEvent->time);
		}
	    }
	}
    }


    /*
     * Clear the pre-configuration info that supports the move threshold.
     */

    wmGD.preMove = False;


} /* END OF FUNCTION CheckButtonReleaseBuiltin */



/*************************************<->*************************************
 *
 *  HandleCMotionNotify (pCD, motionEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a motion
 *  notify event on the client window (including frame) or icon.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the window/icon that got the motion
 *
 *  motionEvent = pointer to the motion event
 *
 *************************************<->***********************************/

void HandleCMotionNotify (ClientData *pCD, XMotionEvent *motionEvent)
{
    int diffX;
    int diffY;


    /*
     * Do pre-move processing (to support the move threshold) if appropriate:
     */

    if (wmGD.preMove)
    {
	diffX = motionEvent->x_root - wmGD.preMoveX;
	if (diffX < 0) diffX = -diffX;
	diffY = motionEvent->y_root - wmGD.preMoveY;
	if (diffY < 0) diffY = -diffY;


	if ((diffX >= wmGD.moveThreshold) || (diffY >= wmGD.moveThreshold)) 
	{
	    /*
	     * The move threshold has been exceded; start the config action.
	     */

	    wmGD.clickData.clickPending = False;
	    wmGD.clickData.doubleClickPending = False;
	    wmGD.preMove = False;

	    if (wmGD.configAction == MOVE_CLIENT)
	    {
		HandleClientFrameMove (pCD, (XEvent *) motionEvent);
	    }
	    else if (wmGD.configAction == RESIZE_CLIENT)
	    {
		HandleClientFrameResize (pCD, (XEvent *) motionEvent);
	    }
	}
    }

} /* END OF FUNCTION HandleCMotionNotify */



/*************************************<->*************************************
 *
 *  HandleCEnterNotify (pCD, enterEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with an enter
 *  window event on the client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the window/icon that was entered
 *
 *  enterEvent = pointer to the enter event
 *
 *************************************<->***********************************/

void HandleCEnterNotify (ClientData *pCD, XEnterWindowEvent *enterEvent)
{
    XEvent          report;
    Boolean	    MatchFound;
    Window	    enterWindow;

    /*
     * If a client is being configured don't change the keyboard input
     * focus.  The input focus is "fixed" after the configuration has been
     * completed.
     */

    if (pCD->clientState == MINIMIZED_STATE)
    {
	enterWindow = ICON_FRAME_WIN(pCD);
    }
    else
    {
	enterWindow = pCD->clientFrameWin;
    }

    MatchFound = XCheckTypedWindowEvent(DISPLAY, enterWindow,
				    LeaveNotify, &report);

    /*
     * NOTE: Handle focus change for when user clicks button in the
     * process of moving focus the matching event will be NotifyGrab.
     *
     * IF (((no_match) ||
     *      (another window has focus and button grabbed)) &&
     *     pointer_mode)
     */

    if ((enterEvent->detail != NotifyInferior) &&
	(((!MatchFound || (report.xcrossing.detail == NotifyInferior)) &&
	  ((enterEvent->mode == NotifyNormal) ||
	   (enterEvent->mode == NotifyUngrab)) &&
	  !wmGD.menuActive) ||

	 (wmGD.keyboardFocus &&
	  (wmGD.keyboardFocus->clientFrameWin != enterWindow) &&
	  (enterEvent->mode == NotifyGrab))) &&

	((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
	 (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)))
    {
	/* 
	 * Make sure that EnterNotify is applicable; don't do anything if
	 * the window is minimized (not currently visible) or the event is
	 * associated with an icon in the icon box.
	 */

	if (!(((enterEvent->window == pCD->clientFrameWin) &&
	      (pCD->clientState == MINIMIZED_STATE)) ||
	     (((enterEvent->window == ICON_FRAME_WIN(pCD)) && 
	       P_ICON_BOX(pCD)) ||
	      (enterEvent->window == pCD->pSD->activeIconTextWin))))

	{
	    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	    {
		/*
		 * Set the focus only if the window does not currently have
		 * or if another window is in the process of getting the
		 * focus (this check avoids redundant focus setting).
		 */

		if ((pCD != wmGD.keyboardFocus) ||
		    (pCD != wmGD.nextKeyboardFocus))
		{

	            Do_Focus_Key (pCD, enterEvent->time, ALWAYS_SET_FOCUS);

                    /* Does the event need to be replayed for modalized windows ? */
                    if ( wmGD.replayEnterEvent )
                        /* Yes, save the event. */
                        memcpy( &wmGD.savedEnterEvent, enterEvent, 
                                sizeof( XEnterWindowEvent ) );


/*
 * The original code counted on getting a focus out event as a result
 * of setting the input focus in Do_Focus_key.  That would cause
 * SetkeyboardFocus to get called.  Unfortunately, you cannot depend on
 * getting a focus out.  You may have already had focus yourself.
 *
 * This bug can be produced by:
 *	* bring up a menu and leave it posted
 *	* move to a different window and click
 *	* the menu comes unposted, the new window has input focus, but no
 *	  client active decorations are changed.
 */
#ifdef SGI_FOCUS_PATCH
		    SetKeyboardFocus (pCD, REFRESH_LAST_FOCUS);
#endif
		}
	    }
	    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	    {
	        SetColormapFocus (ACTIVE_PSD, pCD);
	    }
	}
    }

} /* END OF FUNCTION HandleCEnterNotify */




/*************************************<->*************************************
 *
 *  HandleCLeaveNotify (pCD, leaveEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with an leave
 *  window event on the client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the window/icon that was leaveed
 *
 *  leaveEvent = pointer to the leave event
 *
 *************************************<->***********************************/

void HandleCLeaveNotify (ClientData *pCD, XLeaveWindowEvent *leaveEvent)
{
    XEvent          report;
    Window	    leaveWindow;

    if (pCD->clientState == MINIMIZED_STATE)
    {
	leaveWindow = ICON_FRAME_WIN(pCD);
    }
    else
    {
	leaveWindow = pCD->clientFrameWin;
    }

    /*
     * Don't remove enterEvents when user double clicks on an icon in
     * an iconbox.  Otherwise the window that gets normalized will get
     * matching enter events and not get the focus.
     */
    if (P_ICON_BOX(pCD) &&
	(P_ICON_BOX(pCD)->pCD_iconBox != wmGD.keyboardFocus) &&
	(P_ICON_BOX(pCD)->pCD_iconBox != wmGD.nextKeyboardFocus))
    {
	XCheckTypedWindowEvent(DISPLAY, leaveWindow, EnterNotify, &report);
    }

} /* END OF FUNCTION HandleCLeaveNotify */




/*************************************<->*************************************
 *
 *  HandleCFocusIn (pCD, focusChangeEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a focus
 *  in event.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the window/icon that was entered
 *
 *  enterEvent = pointer to the focus in event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if event is to be dispatched by the toolkit
 *
 *************************************<->***********************************/

Boolean HandleCFocusIn (ClientData *pCD, XFocusChangeEvent *focusChangeEvent)
{
    Boolean setupNextFocus;
    Boolean doXtDispatchEvent = False;

    /*
     * Ignore the event if it is for a window that is no longer viewable.
     * This is the case for a client window FocusIn event that is being
     * processed for a window that has been minimized.
     */

    if ((focusChangeEvent->window == ICON_FRAME_WIN(pCD)) && 
	P_ICON_BOX(pCD))
    {
	doXtDispatchEvent = True;
    }
    else if (((focusChangeEvent->mode == NotifyNormal) ||
	     (focusChangeEvent->mode == NotifyWhileGrabbed)) &&
	    !((focusChangeEvent->window == pCD->clientBaseWin) &&
	      (pCD->clientState == MINIMIZED_STATE)) &&
	    !((focusChangeEvent->window == ICON_FRAME_WIN(pCD)) &&
	      (pCD->clientState != MINIMIZED_STATE)))
    {
	setupNextFocus = (wmGD.keyboardFocus == wmGD.nextKeyboardFocus);

	if (wmGD.keyboardFocus != pCD)
	{
	    if ((focusChangeEvent->detail == NotifyNonlinear) ||
	        (focusChangeEvent->detail == NotifyNonlinearVirtual))
	    {
	        SetKeyboardFocus (pCD, REFRESH_LAST_FOCUS);
		if (setupNextFocus)
		{
		    wmGD.nextKeyboardFocus = wmGD.keyboardFocus;
		}
	    }
	    /* Re: CR 4896                                                  */
	    /* this part added to try and fix the %#$!@!!&* focus bug.      */
            /* this seems to solve most of the problem.  This still leaves  */
            /* times when clicking on an icon toggles the focus back to the */
            /* the previous focus window.                                   */
            /* Another patch was added to WmEvent.c to fix that problem.    */
	    else
	    {
	        SetKeyboardFocus (pCD, REFRESH_LAST_FOCUS);
		if (setupNextFocus) wmGD.nextKeyboardFocus = wmGD.keyboardFocus;
	    }
	}
	else if ((focusChangeEvent->detail == NotifyInferior) &&
		 (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT))
	{
	    /*
	     * The client window was withdrawn (unmapped or destroyed).
	     * Reset the focus.
	     * !!! pointer focus !!!
	     */

	    if (wmGD.autoKeyFocus)
	    {
		/* !!! fix this up to handle transient windows !!! */
		AutoResetKeyFocus (wmGD.keyboardFocus, GetTimestamp ());
	    }
	    else
	    {
		Do_Focus_Key ((ClientData *) NULL, GetTimestamp (), 
			ALWAYS_SET_FOCUS);
	    }
	}
    }

    pCD->focusAutoRaiseDisabled = False;

    return (doXtDispatchEvent);

} /* END OF FUNCTION HandleCFocusIn */



/*************************************<->*************************************
 *
 *  HandleCFocusOut (pCD, focusChangeEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a focus
 *  out event that applies to a client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data for the window/icon that was entered
 *
 *  enterEvent = pointer to the focus out event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if event is to be dispatched by the toolkit
 *
 *************************************<->***********************************/

Boolean HandleCFocusOut (ClientData *pCD, XFocusChangeEvent *focusChangeEvent)
{
    Boolean doXtDispatchEvent = False;
    long focusFlags = REFRESH_LAST_FOCUS ;

    pCD->focusAutoRaiseDisabled = False;

    /*
     * Ignore the event if it is for a window that is no longer viewable.
     * This is the case for a client window FocusOut event that is being
     * processed for a window that has been minimized. Also, ignore focus
     * out events for clients that aren't on the current screen.
     */

    if (((focusChangeEvent->window == ICON_FRAME_WIN(pCD)) && 
	 P_ICON_BOX(pCD)) ||
	(SCREEN_FOR_CLIENT(pCD) != ACTIVE_SCREEN))
    {
	doXtDispatchEvent = True;
    }
    else if ((wmGD.keyboardFocus == pCD) &&
	     (focusChangeEvent->mode == NotifyNormal) &&
	     ((focusChangeEvent->detail == NotifyNonlinear) ||
	      (focusChangeEvent->detail == NotifyNonlinearVirtual)) &&
	     !((focusChangeEvent->window == pCD->clientBaseWin) &&
	       (pCD->clientState == MINIMIZED_STATE)) &&
	     !((focusChangeEvent->window == ICON_FRAME_WIN(pCD)) &&
	       (pCD->clientState != MINIMIZED_STATE)))
    {
	/*
	 * The keyboard focus was shifted to another window, maybe on
	 * another screen.  Clear the focus indication and reset focus
	 * handling for the client window.
	 */

	/*
	 * use SCREEN_SWITCH_FOCUS in SetKeyboardFocus to
	 * not call SetColormapFocus if we are moveing
	 * to another screen
	 */
	if (SCREEN_FOR_CLIENT(pCD) != ACTIVE_SCREEN)
	{
	    focusFlags |= SCREEN_SWITCH_FOCUS;
	}
	SetKeyboardFocus ((ClientData *) NULL, focusFlags);
	if (wmGD.nextKeyboardFocus == pCD)
	{
	    wmGD.nextKeyboardFocus = NULL;
	}
    }

    return (doXtDispatchEvent);

} /* END OF FUNCTION HandleCFocusOut */



/*************************************<->*************************************
 *
 *  HandleCConfigureRequest (pCD, configureRequest)
 *
 *
 *  Description:
 *  -----------
 *  This functions handles ConfigureRequest events that are for client windows.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  configureRequest = a pointer to a ConfigureRequest event
 *
 *************************************<->***********************************/

void HandleCConfigureRequest (ClientData *pCD, XConfigureRequestEvent *configureRequest)
{
    unsigned int mask = configureRequest->value_mask;
    int stackMode = configureRequest->detail;
    unsigned int changeMask;
    ClientData *pcdLeader;
    ClientData *pcdSibling;
    ClientListEntry *pStackEntry;


    /*
     * Call ProcessNewConfiguration to handle window moving and resizing.
     * Send ConfigureNotify event (based on ICCCM conventions).
     * Then process the request for stacking.
     */

    if ((configureRequest->window == pCD->client) &&
	(mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth)))
    {
	if (pCD->maxConfig) {
	    ProcessNewConfiguration (pCD,
		(mask & CWX) ? configureRequest->x : pCD->maxX,
		(mask & CWY) ? configureRequest->y : pCD->maxY,
		(unsigned int) ((mask & CWWidth) ? 
		    configureRequest->width : pCD->maxWidth),
		(unsigned int) ((mask & CWHeight) ? 
		    configureRequest->height : pCD->maxHeight),
		True /*client request*/);
	}
	else {
	    int xOff, yOff;

	    /* CDExc21094 - ProcessNewConfiguration() offsets the */
	    /* x and y positions passed in; in order to keep them */
	    /* the same, we offset them in the opposite direction. */
	    if (wmGD.positionIsFrame)
	    {
		xOff = pCD->clientOffset.x;
		yOff = pCD->clientOffset.y;
	    }
	    else
	    {
		xOff = yOff = 0;
	    }

	    ProcessNewConfiguration (pCD,
		(mask & CWX) ? configureRequest->x : pCD->clientX - xOff,
		(mask & CWY) ? configureRequest->y : pCD->clientY - yOff,
		(unsigned int) ((mask & CWWidth) ? 
		    configureRequest->width : pCD->clientWidth),
		(unsigned int) ((mask & CWHeight) ? 
		    configureRequest->height : pCD->clientHeight),
		True /*client request*/);
	}
    }

    if (mask & CWStackMode)
    {
	changeMask = mask & (CWSibling | CWStackMode);
	if (changeMask & CWSibling)
	{
	    if (XFindContext (DISPLAY, configureRequest->above,
		    wmGD.windowContextType, (caddr_t *)&pcdSibling))
	    {
		changeMask &= ~CWSibling;
	    }
	    else
	    {
		/*
		 * For client requests only primary windows can be
		 * restacked relative to one another.
		 */

		pcdLeader = FindTransientTreeLeader (pCD);
		pcdSibling = FindTransientTreeLeader (pcdSibling);
		if (pcdLeader == pcdSibling)
		{
		    changeMask &= ~CWSibling;
		}
		else
		{
		    pStackEntry = &pcdSibling->clientEntry;
		    if ((stackMode == Above) || (stackMode == TopIf))
		    {
			/* lower the window to just above the sibling */
			Do_Lower (pcdLeader, pStackEntry, STACK_NORMAL);
	    	    }
	    	    else if ((stackMode == Below) || (stackMode == BottomIf))
	    	    {
			/* raise the window to just below the sibling */
			Do_Raise (pcdLeader, pStackEntry, STACK_NORMAL);
	    	    }
	    	    else if (stackMode == Opposite)
	    	    {
			F_Raise_Lower (NULL, pCD, (XEvent *)configureRequest);
	    	    }
		}
	    }
	}

	if (!(changeMask & CWSibling))
	{
	    if ((stackMode == Above) || (stackMode == TopIf))
	    {
		Do_Raise (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
	    }
	    else if ((stackMode == Below) || (stackMode == BottomIf))
	    {
		Do_Lower (pCD, (ClientListEntry *) NULL, STACK_NORMAL);
	    }
	    else if (stackMode == Opposite)
	    {
		F_Raise_Lower (NULL, pCD, (XEvent *) configureRequest);
	    }
	}

	/* !!! should a synthetic ConfigureNotify be sent? !!! */
        if ((configureRequest->window == pCD->client) &&
	    !(mask & (CWX | CWY | CWWidth | CWHeight | CWBorderWidth)))
	{
	    SendConfigureNotify (pCD);
	}
    }


} /* END OF FUNCTION HandleCConfigureRequest */



/*************************************<->*************************************
 *
 *  HandleCColormapNotify (pCD, colorEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function does window management actions associated with a colormap
 *  notify event on the client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  colorEvent = a ColormapNotify event
 *
 *************************************<->***********************************/

void HandleCColormapNotify (ClientData *pCD, XColormapEvent *colorEvent)
{
    int i;
#ifndef	IBM_169380
    ClientData  **cmap_window_data;
#endif
    Boolean newClientColormap = False;


    /*
     * The colormap of the top-level client window or one of its subwindows
     * has been changed.
     */


    if (colorEvent->new)
    {
        /*
         * The colormap has been changed.
         */

        /*
         * !!! when the server ColormapNotify problem is fixed !!!
         * !!! use the colormap id from the event              !!!
         */
        if (WmGetWindowAttributes (colorEvent->window))
        {
	    colorEvent->colormap = wmGD.windowAttributes.colormap;
        }
	else
	{
	    return;
	}
        /*
         * !!! remove the above code when the problem is fixed  !!!
         */

	/*
	 * Identify the colormap that the window manager has associated
	 * with the window.
	 */

#ifndef IBM_169380
        if ((pCD->clientCmapCount == 0) && (colorEvent->window == pCD->client))
#endif
	if (pCD->clientCmapCount == 0)
	{
	    /* no subwindow colormaps; change top-level window colormap */
#ifdef  IBM_169380
	    if (colorEvent->window == pCD->client)
	    {
#endif
	        if (colorEvent->colormap == None)
	        {
	            /* use the workspace colormap */
	            pCD->clientColormap = 
			ACTIVE_PSD->workspaceColormap;
		}
		else
		{
	            pCD->clientColormap = colorEvent->colormap;
		}
		newClientColormap = True;
#ifdef  IBM_169380
	    }
#endif
	}

#ifndef	IBM_169380
        if (!XFindContext (DISPLAY, colorEvent->window,
            wmGD.cmapWindowContextType, (caddr_t *)&cmap_window_data))
        {
            /*
             * The WM_COLORMAP_WINDOWS property of a toplevel window may
             * specify colorEvent->window.  If so, we must update the
             * colormap information it holds in clientCmapList.
             */
            ClientData  *any_pCD;
            int         j;

            for (j = 0; cmap_window_data[j] != NULL; j++)
            {
                any_pCD = cmap_window_data[j];
                for (i = 0; i < any_pCD->clientCmapCount; i++)
                {
                    if (any_pCD->cmapWindows[i] == colorEvent->window)
                    {
                        if (colorEvent->colormap == None)
                        {
                            /* use the workspace colormap */
                            any_pCD->clientCmapList[i] =
                                ACTIVE_PSD->workspaceColormap;
                        }
                        else
                        {
                            any_pCD->clientCmapList[i] = colorEvent->colormap;
                        }
                        if (i == any_pCD->clientCmapIndex)
                        {
                            any_pCD->clientColormap =
                                any_pCD->clientCmapList[i];
                            if (any_pCD == pCD)
                            {
                                newClientColormap = True;
                            }
                        }
                        break;
                    }
                }
            }
        }
#else
	else
	{
	    /* there are subwindow colormaps */
	    for (i = 0; i < pCD->clientCmapCount; i++)
	    {
		if (pCD->cmapWindows[i] == colorEvent->window)
		{
		    if (colorEvent->colormap == None)
		    {
			/* use the workspace colormap */
			pCD->clientCmapList[i] = 
			    ACTIVE_PSD->workspaceColormap;
		    }
		    else
		    {
			pCD->clientCmapList[i] = colorEvent->colormap;
		    }
		    if (i == pCD->clientCmapIndex)
		    {
			newClientColormap = True;
			pCD->clientColormap = pCD->clientCmapList[i];
		    }
		    break;
		}
	    }
	}
#endif	/* IBM_169380 */

	if ((ACTIVE_PSD->colormapFocus == pCD) && newClientColormap &&
	    ((pCD->clientState == NORMAL_STATE) ||
	    (pCD->clientState == MAXIMIZED_STATE)))
	{
	    /*
	     * The client window has the colormap focus, install the
	     * colormap.
	     */

	    WmInstallColormap (ACTIVE_PSD, pCD->clientColormap);
	}
    }


} /* END OF FUNCTION HandleCColormapNotify */



/*************************************<->*************************************
 *
 *  HandleClientMessage (pCD, clientEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function handles client message events that are sent to the root
 *  window.  The window manager action that is taken depends on the
 *  message_type of the event.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  clientEvent = pointer to a client message event on the root window
 * 
 *************************************<->***********************************/

void HandleClientMessage (ClientData *pCD, XClientMessageEvent *clientEvent)
{
    unsigned int newState;

    /*
     * Process the client message event based on the message_type.
     */

    if (clientEvent->message_type == wmGD.xa_WM_CHANGE_STATE)
    {
	if ((clientEvent->data.l[0] == IconicState) &&
	    (pCD->clientFunctions & MWM_FUNC_MINIMIZE))
	{
	    newState = MINIMIZED_STATE;
	}
	else if (clientEvent->data.l[0] == NormalState)
	{
	    newState = NORMAL_STATE;
	}
#ifdef WSM
	if (!ClientInWorkspace (ACTIVE_WS, pCD))
	{
	    newState |= UNSEEN_STATE;
	}
#endif /* WSM */

	SetClientState (pCD, newState, GetTimestamp ());

    }

} /* END OF FUNCTION HandleClientMessage */


#ifndef NO_SHAPE

/*************************************<->*************************************
 *
 *  HandleCShapeNotify (pCD, shapeEvent)
 *
 *
 *  Description:
 *  -----------
 *  Handle a shape notify event on a client window. Keeps track of
 *  the shaped state of the client window and calls
 *  SetFrameShape() to reshape the frame accordingly.
 *
 *  Inputs:
 *  ------
 *  shapeEvent = pointer to a shape notify in event on the client window.
 *
 *************************************<->***********************************/
void
HandleCShapeNotify (ClientData *pCD,  XShapeEvent *shapeEvent)
{
    if (pCD)
    {
	if (shapeEvent->kind != ShapeBounding)
	{
	    return;
	}
	
	pCD->wShaped = shapeEvent->shaped;
	SetFrameShape (pCD);
    }
} /* END OF FUNCTION HandleCShapeNotify */
#endif /* NO_SHAPE */


/*************************************<->*************************************
 *
 *  GetParentWindow (window)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the parent window of the specified window.
 *
 *
 *  Inputs:
 *  ------
 *  window = find the parent of this window
 * 
 *  Outputs:
 *  -------
 *  Return = return the window id of the parent of the specified window
 * 
 *************************************<->***********************************/

Window GetParentWindow (Window window)
{
    Window root;
    Window parent;
    Window *children;
    unsigned int nchildren;


    if (XQueryTree (DISPLAY, window, &root, &parent, &children, &nchildren))
    {
	if (nchildren)
	{
	    XFree ((char *)children);
	}
    }
    else
    {
	parent = (Window)0L;
    }

    return (parent);


} /* END OF FUNCTION GetParentWindow */


/*************************************<->*************************************
 *
 *  DetermineActiveScreen (pEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function determines the currently active screen
 *
 *
 *  Inputs:
 *  ------
 *  pEvent = pointer to an event structure
 * 
 *  Outputs:
 *  -------
 *  ACTIVE_PSD =  set to point to the screen data for the currently
 *                active scree;
 *  wmGD.queryScreen =  set to False if we're sure about the ACTIVE_PSD
 *                      setting
 * 
 *************************************<->***********************************/

void DetermineActiveScreen (XEvent *pEvent)
{
    WmScreenData *pSD;

    switch (pEvent->type)
    {
	case NoExpose:
	case GraphicsExpose:
		break;		/* ignore these events */

        default:
		/*
		 * Get the screen that the event occurred on.
		 */
		pSD = GetScreenForWindow (pEvent->xany.window);

		if (pSD) 
		{
		    /*
		     * Set the ACTIVE_PSD to the event's screen to
		     * make sure the event gets handled correctly.
		     */
		    SetActiveScreen (pSD);
		}
		break;
    }

} /* END OF FUNCTION DetermineActiveScreen */


/*************************************<->*************************************
 *
 *  GetScreenForWindow (win)
 *
 *
 *  Description:
 *  -----------
 *  This function determines the screen for a window
 *
 *
 *  Inputs:
 *  ------
 *  win = window id
 * 
 *  Outputs:
 *  -------
 *  value of function = pointer to screen data (pSD) or NULL on failure
 * 
 *************************************<->***********************************/

WmScreenData * GetScreenForWindow (win)
    Window win;

{
    XWindowAttributes attribs;
    WmScreenData *pSD = NULL;


    /*
     * Get the screen that the event occurred on.
     */
    if (XGetWindowAttributes (DISPLAY, win, &attribs))
    {
	if (!XFindContext (DISPLAY, attribs.root, wmGD.screenContextType, 
			    (caddr_t *)&pSD))
	{
	    if (pSD && !pSD->screenTopLevelW)
	    {
		pSD = NULL;
	    }
	}
    }

    return (pSD);

} /* END OF FUNCTION GetScreenForWindow */
