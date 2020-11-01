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
 * Motif Release 1.2
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmCPlace.c /main/5 1996/08/09 15:18:04 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */
#include "WmGlobal.h"
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

/* absolute value macro */
#ifndef ABS
#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#endif

#define GRAB_MASK (KeyPressMask | ButtonPressMask | ButtonReleaseMask |\
		   PointerMotionMask)
#define PGRAB_MASK (ButtonPressMask | ButtonReleaseMask |\
		    PointerMotionMask | PointerMotionHintMask)

#define NOFRZ_GRAB_MASK (KeyPressMask | ButtonPressMask |\
			 ButtonReleaseMask)
#define NOFRZ_PGRAB_MASK (ButtonPressMask | ButtonReleaseMask)
		    	  
/*
 * include extern functions
 */
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmFeedback.h"
#include "WmWinConf.h"

/*
 * Global Variables:
 */
static int placeX;
static int placeY;
static unsigned int placeWidth;
static unsigned int placeHeight;

static Boolean placeResize;
static Boolean placementDone;
static int placePointerX;
static int placePointerY;
static int placeKeyMultiplier;

static int placeOffsetX;
static int placeOffsetY;



/*************************************<->*************************************
 *
 *  SetupPlacement (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Perform the initialization for interactive placement
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
 * o sets up global data and puts initial display on the screen
 * 
 *************************************<->***********************************/
void SetupPlacement (ClientData *pcd)
{
    int cX, cY, junk;
    Window junk_win;

    /*
     * Restore the state of the last "depressed" frame gadget
     */

    if (wmGD.gadgetClient && wmGD.gadgetDepressed)
    {
	PopGadgetOut(wmGD.gadgetClient, wmGD.gadgetDepressed);

    }
	
    /* get offset of frame origin from window origin */
    placeOffsetX = pcd->clientOffset.x;
    placeOffsetY = pcd->clientOffset.y;

    XQueryPointer (DISPLAY, ACTIVE_ROOT, 
		   &junk_win, &junk_win,
		   &cX, &cY, &junk, &junk, (unsigned int *)&junk);

    /* convert to frame coordinates */
    placePointerX = placeX = cX;
    placePointerY = placeY = cY;
    placeWidth = pcd->clientWidth;
    placeHeight = pcd->clientHeight;
    ClientToFrame (pcd, &cX, &cY, &placeWidth, &placeHeight);

    /* in "position" mode to start with */
    placeResize = FALSE;
    wmGD.preMove = FALSE;

    /* normal window being dealt with, not icon */
    wmGD.movingIcon = FALSE;

    if (wmGD.showFeedback & WM_SHOW_FB_PLACEMENT)
    {
        DoFeedback (pcd, placeX, placeY, placeWidth, placeHeight, 
		    (FB_SIZE | FB_POSITION), placeResize);
    }

    /* set up initial visual feedback */
    MoveOutline (placeX, placeY, placeWidth, placeHeight);

} /* END OF FUNCTION SetupPlacement  */


/*************************************<->*************************************
 *
 *  IsRepeatedKeyEvent (dpy, pEvent, pOldEvent)
 *
 *
 *  Description:
 *  -----------
 *  Returns TRUE if the event passed in is a repeat of the key event
 *  indicated in pOldEvent
 *
 *
 *  Inputs:
 *  ------
 *  dpy		- display
 *  pEvent	- pointer to this event
 *  pOldEvent	- pointer to previous event (cast to the correct type)
 *
 * 
 *  Outputs:
 *  -------
 *  IsRepeatedKeyEvent	- True if the events are "the same," 
 *			  False otherwise.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Bool IsRepeatedKeyEvent (Display *dpy, XEvent *pEvent, char *pOldEvent)
{
    XEvent *pOld = (XEvent *) pOldEvent;

    if ((pEvent->type == KeyPress) &&			/* key press event */
	(pEvent->type == pOld->type) &&			/* same event type */
	(pEvent->xkey.keycode == pOld->xkey.keycode) &&	/* same key code */
	(pEvent->xkey.state == pOld->xkey.state) &&	/* same modifiers */
	(pEvent->xkey.window == pOld->xkey.window) &&	/* same window */
	(pEvent->xkey.root == pOld->xkey.root))		/* same root */
    {
	return (True);
    }
    else 
    { 
	return (False);
    }
}


/*************************************<->*************************************
 *
 *  StartInteractiveSizing (pcd, time)
 *
 *
 *  Description:
 *  -----------
 *  Switch from "position" mode to "resize" mode
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  time	- time stamp of event for pointer regrab
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
void StartInteractiveSizing (ClientData *pcd, Time time)
{
    unsigned int gmask;

    /* regrab pointer to change cursor */
    gmask = (wmGD.freezeOnConfig)? PGRAB_MASK : NOFRZ_PGRAB_MASK;
    XChangeActivePointerGrab (DISPLAY, gmask, 
			      wmGD.sizePlacementCursor, time);

    /* put cursor at lower-right corner */
    if (wmGD.enableWarp)
    {
	XWarpPointer (DISPLAY, None, ACTIVE_ROOT, 0, 0, 0, 0, 
		  (int) (placeX+placeWidth-1), (int) (placeY+placeHeight-1));
    }

    /*
     * Don't go into resize mode if resize is turned off.
     */
    if (pcd->clientFunctions & MWM_FUNC_RESIZE)
    {
	/* update flags */
	placeResize = TRUE;
	wmGD.preMove = FALSE;
    }
}


/*************************************<->*************************************
 *
 *  HandlePlacementKeyEvent (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Handle key presses during interactive placement
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to key press event
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void HandlePlacementKeyEvent (ClientData *pcd, XKeyEvent *pev)
{
    XEvent KeyEvent;
    KeySym keysym;
    Boolean control, valid;
    int big_inc;
    int tmpX = 0;
    int tmpY = 0;
    int warpX, warpY, newX, newY;
    int keyPlaceX = placeX;
    int keyPlaceY = placeY;
    unsigned int keyPlaceWidth = placeWidth;
    unsigned int keyPlaceHeight = placeHeight;

    /* filter out repeating keys */
    placeKeyMultiplier = 1;
    if (pev->type == KeyPress)
    {
	while (placeKeyMultiplier <= 10 && 
		  XCheckIfEvent (DISPLAY, &KeyEvent, IsRepeatedKeyEvent, 
		  (char *) pev))
	{
	      placeKeyMultiplier++;
	}
    }

    /* convert event data to useful key data */
    keysym = XKeycodeToKeysym (DISPLAY, pev->keycode, 0);
    control = (pev->state & ControlMask) != 0;
    big_inc = DisplayWidth(DISPLAY, ACTIVE_PSD->screen) / 20;

    /* interpret key data */
    valid = FALSE;
    switch (keysym) 
    {
	case XK_Left:
	    tmpX = (control) ? (-big_inc) : -1;
	    valid = TRUE;
	    break;

	case XK_Up:
	    tmpY = (control) ? (-big_inc) : -1;
	    valid = TRUE;
	    break;

	case XK_Right:
	    tmpX = (control) ? (big_inc) : 1;
	    valid = TRUE;
	    break;

	case XK_Down:
	    tmpY = (control) ? (big_inc) : 1;
	    valid = TRUE;
	    break;

	case XK_space:
	    StartInteractiveSizing(pcd, pev->time);
	    break;

	case XK_Return:
	    placementDone = TRUE;	/* global "done" flag */
	    break;

	default:
	    break;
    }


    /* if a valid key was pressed, then react to it */
    if (valid) {
	tmpX *= placeKeyMultiplier;
	tmpY *= placeKeyMultiplier;

	if (placeResize)
	{
	    keyPlaceWidth += tmpX;		/* change size of outline */
	    keyPlaceHeight += tmpY;

	    FixFrameValues(pcd, &keyPlaceX, &keyPlaceY, &keyPlaceWidth, 
		           &keyPlaceHeight, placeResize);

	    warpX = keyPlaceX+keyPlaceWidth-1;
	    warpY = keyPlaceY+keyPlaceHeight-1;

	    SetPointerPosition (warpX, warpY, &newX, &newY);

	    if ((warpX == newX) && (warpY == newY))
	    {
		placeWidth = keyPlaceWidth;
		placeHeight = keyPlaceHeight;
	    }
	    else 
	    {
		placeWidth = newX - keyPlaceX + 1;
		placeHeight = newY - keyPlaceY + 1;
	    }
	}
	else 
	{
	    keyPlaceX += tmpX;		/* change position of outline */
	    keyPlaceY += tmpY;

	    FixFrameValues(pcd, &keyPlaceX, &keyPlaceY, &keyPlaceWidth, 
		           &keyPlaceHeight, placeResize);

	    warpX = keyPlaceX;
	    warpY = keyPlaceY;

	    SetPointerPosition (warpX, warpY, &newX, &newY);

	    placeX = newX;
	    placeY = newY;
	}
	placePointerX = newX;
	placePointerY = newY;
    }
    FixFrameValues (pcd, &placeX, &placeY, &placeWidth, &placeHeight,
		    placeResize);

    MoveOutline (placeX, placeY, placeWidth, placeHeight);

    if (wmGD.showFeedback & WM_SHOW_FB_PLACEMENT)
    {
	DoFeedback (pcd, placeX, placeY, placeWidth, placeHeight, 
	            0, placeResize);
    }
} /* END OF FUNCTION HandlePlacementKeyEvent */


/*************************************<->*************************************
 *
 *  HandlePlacementButtonEvent (pev)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *
 *  Inputs:
 *  ------
 *  pev		- pointer to button event
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void HandlePlacementButtonEvent (XButtonEvent *pev)
{
    /*
     * Only listen to button 1
     */
    if (pev->button == 1)
    {
	/* 
	 * Complete interactive placement on button release
	 */
	if (pev->type == ButtonRelease) 
	{
	    placementDone = TRUE;		/* global done flag */
	}

	else if (pev->type == ButtonPress) 
	{

	/*
	 * Button press, go to pre-resize mode
	 */
	    wmGD.preMoveX = pev->x_root;
	    wmGD.preMoveY = pev->y_root;
	    wmGD.preMove = TRUE;
	}
    }
}



/*************************************<->*************************************
 *
 *  HandlePlacementMotionEvent (pcd, pev)
 *
 *
 *  Description:
 *  -----------
 *  Handles pointer motion events during interactive placement
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pev		- pointer to mouse motion event
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

void HandlePlacementMotionEvent (ClientData *pcd, XMotionEvent *pev)
{
    int diffx, diffy;

    /*
     * If in pre-resize mode, check for motion crossing threshhold before 
     * switching modes
     */
    if (wmGD.preMove) {
	diffx = pev->x_root - wmGD.preMoveX;
	diffy = pev->y_root - wmGD.preMoveY;
	if ((ABS(diffx) > wmGD.moveThreshold) ||
	    (ABS(diffy) > wmGD.moveThreshold))
	{
	    StartInteractiveSizing(pcd, pev->time);
	}
	return; 
    }

    if (placeResize) {
	/*
	 * Track lower right corner
	 */
	if (pev->x_root > placeX)
	    placeWidth = pev->x_root - placeX + 1;
	if (pev->y_root > placeY)
	    placeHeight = pev->y_root - placeY + 1;
    }
    else {
	/*
	 * track window position
	 */
	placeX = pev->x_root;
	placeY = pev->y_root;
    }

    placePointerX = pev->x_root;
    placePointerY = pev->y_root;

    FixFrameValues (pcd, &placeX, &placeY, &placeWidth, &placeHeight,
		    placeResize);

    MoveOutline (placeX, placeY, placeWidth, placeHeight);

    if (wmGD.showFeedback & WM_SHOW_FB_PLACEMENT)
    {
	DoFeedback (pcd, placeX, placeY, placeWidth, placeHeight, 
	            0, placeResize);
    }
} /* END OF FUNCTION HandlePlacementMotionEvent */



/*************************************<->*************************************
 *
 *  DoPlacement (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Gets window configuration from the user via pointer/keyboard interaction
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pcd		- clientX, clientY, clientWidth, and clientHeight members
 *		  could be changed
 *
 *  Comments:
 *  --------
 *  We try to be careful only to remove events that we need from the 
 *  event queue while we're in our own event processing loop.
 * 
 *************************************<->***********************************/
void DoPlacement (ClientData *pcd)
{
    XEvent event;

    /*
     * Initialization
     */
    SetupPlacement (pcd);

    /*
     * Process events
     */
    placementDone = FALSE;
    while (!placementDone)
    {
	GetConfigEvent (DISPLAY, ACTIVE_ROOT, GRAB_MASK,
		placePointerX, placePointerY, placeX, placeY, 
		placeWidth, placeHeight, &event);

	switch (event.type) {
	    case KeyPress:
		HandlePlacementKeyEvent(pcd, (XKeyEvent *)&event);
		break;

	    case ButtonPress:
		HandlePlacementButtonEvent((XButtonEvent *)&event);
		break;

	    case ButtonRelease:
		HandlePlacementButtonEvent((XButtonEvent *)&event);
		break;

	    case MotionNotify:
		HandlePlacementMotionEvent(pcd, (XMotionEvent *)&event);
		break;

	}
    }
    
    /* copy back the configuration information */
    pcd->clientX = placeX + placeOffsetX;
    pcd->clientY = placeY + placeOffsetY; 
    pcd->clientWidth = placeWidth - 2*placeOffsetX;
    pcd->clientHeight = placeHeight - placeOffsetX - placeOffsetY;

    /* clean up */
    MoveOutline (0,0,0,0);
    HideFeedbackWindow(pcd->pSD);

} /* END OF FUNCTION DoPlacement  */



/*************************************<->*************************************
 *
 *  PlaceWindowInteractively(pcd)
 *
 *
 *  Description:
 *  -----------
 *  Gets the clients size and position information interactively from the
 *  user.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
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
void PlaceWindowInteractively (ClientData *pcd)
{
    unsigned int gmask;

    gmask = (wmGD.freezeOnConfig)? PGRAB_MASK : NOFRZ_PGRAB_MASK;

    /*
     * Return if config is in progress or if grabs fail
     */
    if (!DoGrabs (ACTIVE_ROOT, wmGD.movePlacementCursor, 
	gmask, CurrentTime, pcd, True))
    {
	return;
    }

    /*
     *  Get the size and position of the window from the user.
     *  Do our own event processing until a button-up event occurs
     */
    DoPlacement(pcd);

    /*
     * Finish up and return the data
     */
    UndoGrabs();
    wmGD.preMove = FALSE;
}




