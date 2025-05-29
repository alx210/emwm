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

#include "WmError.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#include "WmWinInfo.h"
#include <Xm/TransferP.h>

/*
 * Function Declarations:
 */

#include "WmProtocol.h"


/*************************************<->*************************************
 *
 *  SetupWmICCC ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the window manager handling of the inter-client
 *  communications conventions.
 *
 *
 *  Outputs:
 *  -------
 *  (wmGD) = Atoms id's are setup.
 *
 *************************************<->***********************************/

void SetupWmICCC (void)
{
    enum { 
	   XA_WM_STATE, XA_WM_PROTOCOLS, XA_WM_CHANGE_STATE,
	   XA_WM_SAVE_YOURSELF, XA_WM_DELETE_WINDOW,
	   XA_WM_COLORMAP_WINDOWS, XA_WM_TAKE_FOCUS, XA_MWM_HINTS,
	   XA_MWM_MENU, XA_MWM_MESSAGES, XA_MOTIF_WM_OFFSET,
	   XA_COMPOUND_TEXT, NUM_ATOMS };

    static char *atom_names[] = {
	   _XA_WM_STATE, _XA_WM_PROTOCOLS, _XA_WM_CHANGE_STATE,
	   _XA_WM_SAVE_YOURSELF, _XA_WM_DELETE_WINDOW,
	   _XA_WM_COLORMAP_WINDOWS, _XA_WM_TAKE_FOCUS, _XA_MWM_HINTS,
	   _XA_MWM_MENU, _XA_MWM_MESSAGES, _XA_MOTIF_WM_OFFSET,
	   "COMPOUND_TEXT"
    };

    XIconSize sizeList;
    int scr;
    Atom atoms[XtNumber(atom_names)];

    /*
     * Make atoms that are required by the ICCC and mwm.  The atom for
     * _MOTIF_WM_INFO is intern'ed in ProcessMotifWmInfo.
     */
    XInternAtoms(DISPLAY, atom_names, XtNumber(atom_names), False, atoms);

    wmGD.xa_WM_STATE			= atoms[XA_WM_STATE];
    wmGD.xa_WM_PROTOCOLS		= atoms[XA_WM_PROTOCOLS];
    wmGD.xa_WM_CHANGE_STATE		= atoms[XA_WM_CHANGE_STATE];
    wmGD.xa_WM_SAVE_YOURSELF		= atoms[XA_WM_SAVE_YOURSELF];
    wmGD.xa_WM_DELETE_WINDOW		= atoms[XA_WM_DELETE_WINDOW];
    wmGD.xa_WM_COLORMAP_WINDOWS		= atoms[XA_WM_COLORMAP_WINDOWS];
    wmGD.xa_WM_TAKE_FOCUS		= atoms[XA_WM_TAKE_FOCUS];
    wmGD.xa_MWM_HINTS			= atoms[XA_MWM_HINTS];
    wmGD.xa_MWM_MENU			= atoms[XA_MWM_MENU];
    wmGD.xa_MWM_MESSAGES		= atoms[XA_MWM_MESSAGES];
    wmGD.xa_MWM_OFFSET			= atoms[XA_MOTIF_WM_OFFSET];
    wmGD.xa_COMPOUND_TEXT = atoms[XA_COMPOUND_TEXT];

    /*
     * Setup the icon size property on the root window.
     */

    sizeList.width_inc = 1;
    sizeList.height_inc = 1;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    sizeList.min_width = wmGD.Screens[scr].iconImageMinimum.width;
	    sizeList.min_height = wmGD.Screens[scr].iconImageMinimum.height;
	    sizeList.max_width = wmGD.Screens[scr].iconImageMaximum.width;
	    sizeList.max_height = wmGD.Screens[scr].iconImageMaximum.height;

	    XSetIconSizes (DISPLAY, wmGD.Screens[scr].rootWindow, 
		&sizeList, 1);
	}
    }

} /* END OF FUNCTION SetupWmICCC */



/*************************************<->*************************************
 *
 *  SendConfigureNotify (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a synthetic ConfigureNotify event when
 *  a client window is reconfigured in certain ways (e.g., the window is
 *  moved without being resized).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (window id and client size data)
 *
 *************************************<->***********************************/

void SendConfigureNotify (ClientData *pCD)
{
    XConfigureEvent notifyEvent;


    /*
     * Send a synthetic ConfigureNotify message:
     */

    notifyEvent.type = ConfigureNotify;
    notifyEvent.display = DISPLAY;
    notifyEvent.event = pCD->client;
    notifyEvent.window = pCD->client;
	if(pCD->fullScreen)
	{
	notifyEvent.x = pCD->fullScreenX;
	notifyEvent.y = pCD->fullScreenY;
	notifyEvent.width = pCD->fullScreenWidth;
	notifyEvent.height = pCD->fullScreenHeight;
	}
	else if (pCD->maxConfig)
    {
	notifyEvent.x = pCD->maxX;
	notifyEvent.y = pCD->maxY;
	notifyEvent.width = pCD->maxWidth;
	notifyEvent.height = pCD->maxHeight;
    }
    else
    {
	notifyEvent.x = pCD->clientX;
	notifyEvent.y = pCD->clientY;
	notifyEvent.width = pCD->clientWidth;
	notifyEvent.height = pCD->clientHeight;
    }
    notifyEvent.border_width = 0;
    notifyEvent.above = None;
    notifyEvent.override_redirect = False;

    XSendEvent (DISPLAY, pCD->client, False, StructureNotifyMask,
	(XEvent *)&notifyEvent);


} /* END OF FUNCTION SendConfigureNotify */



/*************************************<->*************************************
 *
 *  SendClientOffsetMessage (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message containing the offset
 *  between the window position reported to the user and the actual
 *  window position of the client over the root.
 *
 *  This can be used by clients that map and unmap windows to help them
 *  work with the window manager to place the window in the same location
 *  when remapped. 
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (frame geometry info)
 *
 *************************************<->***********************************/

void SendClientOffsetMessage (ClientData *pCD)
{
    long borderWidth = (long)pCD->xBorderWidth;
    long offsetX = pCD->clientOffset.x;
    long offsetY = pCD->clientOffset.y;
      
    XClientMessageEvent clientMsgEvent;

    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = pCD->client;
    clientMsgEvent.message_type = wmGD.xa_MWM_MESSAGES;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = wmGD.xa_MWM_OFFSET;

    /*
     * Use window gravity to allow the user to specify the window
     * position on the screen  without having to know the dimensions
     * of the decoration that mwm is adding.
     */
    
    switch (pCD->windowGravity)
    {
      case NorthWestGravity:
      default:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case NorthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = offsetY;
	    break;
	}
	
      case EastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case SouthEastGravity:
	{
	    clientMsgEvent.data.l[1] = -(offsetX - (2 * borderWidth));
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthGravity:
	{
	    clientMsgEvent.data.l[1] = borderWidth;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case SouthWestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = -(offsetX - (2 * borderWidth));
	    break;
	}
	
      case WestGravity:
	{
	    clientMsgEvent.data.l[1] = offsetX;
	    clientMsgEvent.data.l[2] = borderWidth + (offsetY - offsetX)/2;
	    break;
	}
	
      case CenterGravity:
	{
	    clientMsgEvent.data.l[2] = (offsetY - offsetX)/2;
	    break;
	}
    }

    XSendEvent (DISPLAY, pCD->client, False, NoEventMask,
	(XEvent *)&clientMsgEvent);


} /* END OF FUNCTION SendClientOffsetMessage */


/*************************************<->*************************************
 *
 *  SendClientMsg (window, type, data0, time, pData, dataLen)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to send a client message event that to a client
 *  window.  The message may be sent as part of a protocol arranged for by
 *  the client with the WM_PROTOCOLS property.
 *
 *
 *  Inputs:
 *  ------
 *  window = destination window for the client message event
 *
 *  type = client message type
 *
 *  data0 = data0 value in the client message
 *
 *  time = timestamp to be used in the event
 *
 *  pData = pointer to data to be used in the event
 *
 *  dataLen = len of data (in 32 bit units)
 *
 *************************************<->***********************************/

void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen)
{
    XClientMessageEvent clientMsgEvent;
    int i;


    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = window;
    clientMsgEvent.message_type = type;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = data0;
    clientMsgEvent.data.l[1] = (long)time;
    if (pData)
    {
	/*
	 * Fill in the rest of the ClientMessage event (that holds up to
	 * 5 words of data).
	 */

        if (dataLen > 3)
        {
	    dataLen = 3;
        }
        for (i = 2; i < (2 + dataLen); i++)
        {
	    clientMsgEvent.data.l[i] = pData[i];
        }
    }
    
    
    XSendEvent (DISPLAY, window, False, NoEventMask,
	(XEvent *)&clientMsgEvent);
    XFlush(DISPLAY);


} /* END OF FUNCTION SendClientMsg */



/*************************************<->*************************************
 *
 *  AddWmTimer (timerType, timerInterval, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets a window manager timer of the specified type.
 *
 *
 *  Inputs:
 *  ------
 *  timerType = type of timer to be set
 *
 *  timerInterval = length of timeout in ms
 *
 *  pCD = pointer to client data associated with the timer
 *
 *  return = True if timer could be set
 *
 *************************************<->***********************************/

Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD)
{
    WmTimer *pWmTimer;


    if (!(pWmTimer = (WmTimer *)XtMalloc (sizeof (WmTimer))))
    {
	Warning (((char *)GETMESSAGE(56, 1, "Insufficient memory for window manager data")));
	return (False);
    }

    /* !!! handle for XtAppAddTimeOut error !!! */
    pWmTimer->timerId = XtAppAddTimeOut (wmGD.mwmAppContext, 
			    timerInterval, (XtTimerCallbackProc)TimeoutProc, (caddr_t)pCD);
    pWmTimer->timerCD = pCD;
    pWmTimer->timerType = timerType;
    pWmTimer->nextWmTimer = wmGD.wmTimers;
    wmGD.wmTimers = pWmTimer;

    return(True);

} /* END OF FUNCTION AddWmTimer */



/*************************************<->*************************************
 *
 *  DeleteClientWmTimers (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes all window manager timers that are associated with
 *  the specified client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for client whose timers are to be deleted
 *
 *  wmGD = (wmTimers)
 *
 *************************************<->***********************************/

void DeleteClientWmTimers (ClientData *pCD)
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;
    WmTimer *pRemoveTimer;


    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerCD == pCD)
	{
	    if (pPrevTimer)
	    {
		pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	    }
	    else
	    {
		wmGD.wmTimers = pWmTimer->nextWmTimer;
	    }
	    pRemoveTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	    XtRemoveTimeOut (pRemoveTimer->timerId);
	    XtFree ((char *)pRemoveTimer);
	}
	else
	{
	    pPrevTimer = pWmTimer;
	    pWmTimer = pWmTimer->nextWmTimer;
	}
    }


} /* END OF FUNCTION DeleteClientWmTimers */



/*************************************<->*************************************
 *
 *  TimeoutProc (client_data, id)
 *
 *
 *  Description:
 *  -----------
 *  This function is an Xtk timeout handler.  It is used to handle various
 *  window manager timers (i.e. WM_SAVE_YOURSELF quit timeout).
 *
 *
 *  Inputs:
 *  ------
 *  client_data = pointer to window manager client data
 *
 *  id = Xtk timer id
 *
 *************************************<->***********************************/

void TimeoutProc (caddr_t client_data, XtIntervalId *id)
{
    WmTimer *pPrevTimer;
    WmTimer *pWmTimer;

    
    /*
     * Find out if the timer still needs to be serviced.
     */

    pPrevTimer = NULL;
    pWmTimer = wmGD.wmTimers;
    while (pWmTimer)
    {
	if (pWmTimer->timerId == *id)
	{
	    break;
	}
	pPrevTimer = pWmTimer;
	pWmTimer = pWmTimer->nextWmTimer;
    }

    if (pWmTimer)
    {
	/*
	 * Do the timer related action.
	 */

	switch (pWmTimer->timerType)
	{
	    case TIMER_QUIT:
	    {
		XKillClient (DISPLAY, pWmTimer->timerCD->client);
		break;
	    }

	    case TIMER_RAISE:
	    {
		Boolean sameScreen;

		if ((wmGD.keyboardFocus == pWmTimer->timerCD) &&
		    (pWmTimer->timerCD->focusPriority == 
			(PSD_FOR_CLIENT(pWmTimer->timerCD))->focusPriority) &&
		    (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) &&
		    (pWmTimer->timerCD == GetClientUnderPointer(&sameScreen)))
		{
		    Do_Raise (pWmTimer->timerCD, (ClientListEntry *)NULL, STACK_NORMAL);
		}
		break;
	    }
	}


	/*
	 * Remove the timer from the wm timer list.
	 */

	if (pPrevTimer)
	{
	    pPrevTimer->nextWmTimer = pWmTimer->nextWmTimer;
	}
	else
	{
	    wmGD.wmTimers = pWmTimer->nextWmTimer;
	}
	XtFree ((char *)pWmTimer);
    }

    /*
     * Free up the timer.
     */

    XtRemoveTimeOut (*id);


} /* END OF FUNCTION TimeoutProc */
