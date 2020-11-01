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
static char rcsid[] = "$XConsortium: WmEvent.c /main/7 1996/11/20 15:27:47 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993, 1994 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/*
 * Included Files:
 */

#include "WmGlobal.h"
/*
 * include extern functions
 */
#include "WmEvent.h"
#ifdef WSM
#include "WmBackdrop.h"
#endif /* WSM */
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmCEvent.h"
#include "WmColormap.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmMenu.h"
#ifdef WSM
#include "WmICCC.h"
#include "WmProperty.h"
#endif /* WSM */
#include "WmWinInfo.h"
#include "WmWinState.h"
#ifdef PANELIST
#include "WmResNames.h"
#include "WmResParse.h"
#include "WmParse.h"
#include "WmParseP.h"
#endif /* PANELIST */

#include <Xm/RowColumnP.h> /* for MS_LastManagedMenuTime */
extern XmMenuState _XmGetMenuState();

#ifdef WSM
/*
 * FUNCTION PARSER TABLE
 */

typedef struct {
   char         * funcName;
   Context        greyedContext;
   unsigned int   resource;
   long           mgtMask;
   WmFunction     wmFunction;
   Boolean       (*parseProc)();
} FunctionTableEntry;

#endif /* WSM */


/*
 * Global Variables:
 */

extern unsigned int buttonModifierMasks[];
#ifdef WSM
int smAckState = SM_UNITIALIZED;
extern FunctionTableEntry functionTable[];
extern int F_NOP_INDEX;
#endif /* WSM */

#ifndef MOTIF_ONE_DOT_ONE
#include <Xm/MenuShellP.h>
#endif



/*************************************<->*************************************
 *
 *  InitEventHandling ()
 *
 *
 *  Description:
 *  -----------
 *  This function initializes window manager event handling in preparation
 *  for managing client windows.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD = (keySpecs)
 *
 *************************************<->***********************************/

void InitEventHandling (void)
{
    WmScreenData *pSD;
    XSetWindowAttributes setAttributes;
    unsigned long base_mask;
    unsigned int n, scr;


    /*
     * Prepare to get root (workspace) window events that are used to
     * manage client windows.  Setup accelerator event processing.
     */

    base_mask = SubstructureRedirectMask | FocusChangeMask;

    /* handle entry of root window */
    base_mask |= EnterWindowMask | LeaveWindowMask;

    for (scr=0; scr<wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

        if (pSD->managed) 
        {
	    setAttributes.event_mask = base_mask;

	    if (pSD->buttonBindings)
	    {
		/*
		 * The desktop menu and button bindings for window 
		 * manager functions use button press and button 
		 * release events.
		 */
		setAttributes.event_mask |= 
		    (ButtonPressMask | ButtonReleaseMask);
	    }

            XChangeWindowAttributes (DISPLAY, pSD->rootWindow, 
                CWEventMask, &setAttributes);


	    /*
	     * Setup event handling for "accelerated" window management 
	     * functions done with key bindings.
	     */

            if (pSD->keySpecs)
            {
        	SetupKeyBindings (pSD->keySpecs, pSD->rootWindow, 
		    GrabModeSync, F_CONTEXT_ALL);
            }
        
	    if (pSD->acceleratorMenuCount)
	    {
		for (n = 0; n < pSD->acceleratorMenuCount; n++)
		{
		SetupKeyBindings (
		    pSD->acceleratorMenuSpecs[n]->accelKeySpecs,
		    pSD->rootWindow, GrabModeSync, F_CONTEXT_ALL);
		}
	    }
	} /* end if (managed) */
    }  /* end for (all screens) */
} /* END OF FUNCTION InitEventHandling */


/*************************************<->*************************************
 *
 *  _WmGrabMasks (modifiers, pnum_masks)
 *
 *
 *  Description:
 *  -----------
 *  This function returns a set of grab masks to use that effectively
 *  filters out the effects of locking modifiers. Redundant masks
 *  are removed.
 *
 *
 *  Inputs:
 *  ------
 *  modifiers		- keymask of modifiers
 *
 *  Outputs:
 *  ------
 *  *pnum_masks		- number of masks returned
 *
 *  Return:
 *  -------
 *  pointer to a NULL-terminated list of modifier masks. This memory is
 *  statically allocated and reused. Do no free or modify. Make a copy
 *  if you need to keep it. 
 *
 *************************************<->***********************************/

static unsigned int *
_WmGrabMasks ( unsigned int modifiers, int *pnum_masks )
{
    static unsigned int	*pRetMasks = NULL;
    static int len_ret_masks = 0;
    int num_masks;
    int num_ret_masks;
    int i,j;
    unsigned int mask;

    /* count the number of masks in the lock sequence */
    for (num_masks=0; wmGD.pLockMaskSequence[num_masks] != 0; num_masks++);

    /* insure we have enough space for our returned masks */
    if ((pRetMasks == NULL) || (len_ret_masks < num_masks+2))
    {
	if (pRetMasks != NULL) 
			XtFree ((char *)pRetMasks);

	len_ret_masks = num_masks+2;
	pRetMasks = (unsigned int *) 
			XtCalloc (len_ret_masks, sizeof(unsigned int));
    }

    /* fill up the array of masks we return */
    num_ret_masks = 0;
    for (i=0; i<num_masks; i++)
    {
	/* combine with this set of locking mods */
	mask = (modifiers | wmGD.pLockMaskSequence[i]);

	/* skip if exact match */
	if (mask == modifiers) continue;

	/* add this mask to the list if not already there */
        for (j=0; j<num_ret_masks; j++)
	{
	    if (mask == pRetMasks[j])
		break;
	}
	if (j >= num_ret_masks)
	{
	    /* we don't have this mask yet, add it */
	    pRetMasks[num_ret_masks] = mask;
	    num_ret_masks++;
	}
    }

    /*
     * Add the original mask to the list at the end
     */
    pRetMasks[num_ret_masks++] = modifiers;
    pRetMasks[num_ret_masks] = 0;		/* terminator */

    *pnum_masks = num_ret_masks;

    return (pRetMasks);
}


/*************************************<->*************************************
 *
 *  WmGrabKey (display, keycode, modifiers, grab_window, owner_events,
 *		pointer_mode, keyboard_mode)
 *
 *
 *  Description:
 *  -----------
 *  This function does several grabs on a key to make sure the
 *  key is grabbed irrespective of the state of locking modifiers
 *  It is a wrapper for XGrabKey, so the parameters are all the
 *  same.
 *
 *
 *  Inputs:
 *  ------
 *  display		- X server connection
 *  keycode		- keycode to grab
 *  modifiers		- keymask of modifiers
 *  grab_window		- window to do grab on
 *  owner_events	- does app receive events normally?
 *  pointer_mode	- pointer event processing during grab
 *  keyboard_mode	- keyboard event processing during grab
 *  wmGD.pLockMaskSequence	- extra modifier masks to grab with
 *
 *  Return:
 *  -------
 *  The function is asynchronous.
 *
 *************************************<->***********************************/

void
WmGrabKey (
	Display		*display,
	int		keycode,
	unsigned int	modifiers,
	Window		grab_window,
	Bool		owner_events,
	int		pointer_mode,
	int		keyboard_mode 
	  )
{
    unsigned int	*pGrabMasks;
    int			i, num_masks;

    pGrabMasks = _WmGrabMasks (modifiers, &num_masks);

    for (i=0; i<num_masks; i++, pGrabMasks++)
    {
	XGrabKey (display, keycode, *pGrabMasks, grab_window,
	        owner_events, pointer_mode, keyboard_mode);
    }
}


/*************************************<->*************************************
 *
 *  WmGrabButton (display, button, modifiers, grab_window, owner_events,
 *		event_mask, pointer_mode, keyboard_mode, confine_to, cursor)
 *
 *
 *  Description:
 *  -----------
 *  This function does several grabs on a button to make sure the
 *  button is grabbed irrespective of the state of locking modifiers
 *  It is a wrapper for XGrabButton, so the parameters are all the
 *  same.
 *
 *
 *  Inputs:
 *  ------
 *  display		- X server connection 
 *  button		- button to grab
 *  modifiers		- keymask of modifiers
 *  grab_window		- window to do grab on
 *  event_mask		- event mask in effect during grab
 *  owner_events	- does app receive events normally?
 *  pointer_mode	- pointer event processing during grab
 *  keyboard_mode	- keyboard event processing during grab
 *  confine_to		- window to confine the pointer to
 *  cursor		- cursor to be displayed during grab
 *  wmGD.pLockMaskSequence	- extra modifier masks to grab with
 *
 *  Return:
 *  -------
 *  The function is asynchronous.
 *
 *************************************<->***********************************/

void
WmGrabButton (
	Display		*display,
	unsigned int	button,
	unsigned int	modifiers,
	Window		grab_window,
	unsigned int	event_mask,
	Bool		owner_events,
	int		pointer_mode,
	int		keyboard_mode,
	Window		confine_to,
	Cursor		cursor
	  )
{
    unsigned int	*pGrabMasks;
    int			i, num_masks;

    pGrabMasks = _WmGrabMasks (modifiers, &num_masks);

    for (i=0; i<num_masks; i++, pGrabMasks++)
    {
	XGrabButton (display, button, *pGrabMasks, grab_window, event_mask,
	        owner_events, pointer_mode, keyboard_mode, confine_to,
		cursor);
    }
}


/*************************************<->*************************************
 *
 *  WmUngrabButton (display, button, modifiers, grab_window)
 *
 *
 *  Description:
 *  -----------
 *  This function is the complement of WmGrabButton. It does several 
 *  ungrabs on a button to undo the set of grabs done to ignore
 *  the state of locking modifiers.
 *
 *  It is a wrapper for XUngrabButton, so the parameters are all the
 *  same.
 *
 *
 *  Inputs:
 *  ------
 *  display		- X server connection 
 *  button		- button to grab
 *  modifiers		- keymask of modifiers
 *  grab_window		- window to do grab on
 *  wmGD.pLockMaskSequence	- extra modifier masks to grab with
 *
 *  Return:
 *  -------
 *  The function is asynchronous.
 *
 *************************************<->***********************************/

void
WmUngrabButton (
	Display		*display,
	unsigned int	button,
	unsigned int	modifiers,
	Window		grab_window 
	  )
{
    unsigned int	*pGrabMasks;
    int			i, num_masks;

    pGrabMasks = _WmGrabMasks (modifiers, &num_masks);

    for (i=0; i<num_masks; i++, pGrabMasks++)
    {
	XUngrabButton (display, button, *pGrabMasks, grab_window);
    }
}


/*************************************<->*************************************
 *
 *  SetupKeyBindings (keySpecs, grabWindow, keyboardMode, context)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the event handling necessary to support user
 *  specified key bindings for window manager functions.
 *
 *
 *  Inputs:
 *  ------
 *  keySpecs = list of key bindings for window manager functions.
 *
 *  grabWindow = window that is to be associated with the passive key grab.
 *
 *  keyboardMode = indicates keyboard mode for grab.
 *
 *  context = context of key binding to set
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = number of key bindings set
 *
 *************************************<->***********************************/

int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, int keyboardMode, long context)
{
    KeySpec *keySpec;
    int setCount = 0;
    Boolean iconContext;


    /*
     * Use key grabs to get the keys that invoke window manger functions.
     */

    iconContext = (context == F_CONTEXT_ICON);
    keySpec = keySpecs;
    while (keySpec)
    {
#ifdef OLD_CODE
	if (((keySpec->context == F_CONTEXT_ICON) && iconContext) ||
	    ((keySpec->context != F_CONTEXT_ICON) && !iconContext))
#endif
	if (((F_CONTEXT_ICON == (keySpec->context ^
				 (F_CONTEXT_ICONBOX     |
				  F_SUBCONTEXT_IB_IICON |
				  F_SUBCONTEXT_IB_WICON))) &&
             iconContext) ||
            ((F_CONTEXT_ICON != (keySpec->context ^
				 (F_CONTEXT_ICONBOX     |
				  F_SUBCONTEXT_IB_IICON |
				  F_SUBCONTEXT_IB_WICON))) &&
             !iconContext))
	{

	    WmGrabKey (DISPLAY, keySpec->keycode, keySpec->state, grabWindow,
	        False, GrabModeAsync, keyboardMode);

	    setCount++;
	}

	keySpec = keySpec->nextKeySpec;
    }

    return (setCount);

} /* END OF FUNCTION SetupKeyBindings */



/*************************************<->*************************************
 *
 *  WmDispatchMenuEvent (event)
 *
 *
 *  Description:
 *  -----------
 *  This function detects and processes events that affect menu behavior that
 *  are NOT dispatched (processed) by the toolkit.  The events may cause the 
 *  menu to be unposted, may trigger hotspot processing, or may represent 
 *  menu accelerators.  This processing is generally done when the system 
 *  menu is posted in "sticky" mode.
 *
 *
 *  Inputs:
 *  ------
 *  event = This is an X event that has been retrieved by XtNextEvent.
 *  wmGD.menuActive == nonNULL
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

Boolean WmDispatchMenuEvent (XButtonEvent *event)
{
    ClientData *pCD = wmGD.menuClient;
    Boolean     doXtDispatchEvent = True;
    Boolean     checkContext;
    Context     context = 0;
     /* For fixing the bug CR 5227 */
     XKeyEvent *keyEvent;
     KeySpec   *keySpecs;
     MenuButton   *menuBtnPtr;


    if (event->type == KeyPress)
    {
        if (wmGD.menuActive->accelKeySpecs)
        {
	   /*
	    * Check to see if the KeyPress is a menu accelerator
	    * (don't require context match for system menu accelerators).
	    * If so, the active menu will be unposted and the KeyPress event 
	    * will not be sent on to the toolkit.
	    */

  	    checkContext = (!pCD || (pCD->systemMenuSpec != wmGD.menuActive));
	    if (checkContext)
	    {
                if (pCD)
                {
            	    if (pCD->clientState == MINIMIZED_STATE)
            	    {
            	        context = F_CONTEXT_ICON;
            	    }
            	    else if (pCD->clientState == NORMAL_STATE)
            	    {
            	        context = F_CONTEXT_NORMAL;
            	    }
            	    else
            	    {
            	        context = F_CONTEXT_MAXIMIZE;
            	    }
                }
                else
                {
            	    context = F_CONTEXT_ROOT;
                }
	    }
	    /* Begin fixing CR 5227 */
	    keySpecs = wmGD.menuActive->accelKeySpecs;
	    keyEvent = (XKeyEvent *)event;
	    menuBtnPtr = wmGD.menuActive->menuButtons + 
	                 (wmGD.menuActive->menuButtonSize - 1);
  
	    while (keySpecs)
	      {
                if ((keyEvent->keycode == keySpecs->keycode) &&
		    ((keyEvent->state == keySpecs->state) ||
		    (NOLOCKMOD(keyEvent->state) == keySpecs->state))
                   && ((!checkContext) || (context & keySpecs->context)))
                 {
                    doXtDispatchEvent =  
                            XtIsSensitive(menuBtnPtr->buttonWidget);
                    break;
                 } 
                 keySpecs = keySpecs->nextKeySpec;
                 menuBtnPtr--;
                } 
     
	    doXtDispatchEvent = doXtDispatchEvent &&
		HandleKeyPress ((XKeyEvent *)event, 
				wmGD.menuActive->accelKeySpecs,
				checkContext, context, 
				TRUE, (ClientData *)NULL);
        }

        if (wmGD.menuActive && wmGD.menuUnpostKeySpec)
        {
	    if ((wmGD.menuUnpostKeySpec->keycode == event->button) &&
		((wmGD.menuUnpostKeySpec->state == event->state) ||
		 (wmGD.menuUnpostKeySpec->state == NOLOCKMOD(event->state))))
	    {
	        /*
	         * This is an alternate key for unposting a menu from the
	         * keyboard (in addition to [ESC]).
	         */

	        UnpostMenu (wmGD.menuActive);
	        doXtDispatchEvent = False;
	    }
        }
#ifdef ROOT_ICON_MENU
        if (wmGD.menuActive && wmGD.F_NextKeySpec)
        {
            if (((wmGD.F_NextKeySpec->state == event->state) ||
                 (wmGD.F_NextKeySpec->state == NOLOCKMOD(event->state))) &&
                (wmGD.F_NextKeySpec->keycode == event->button))
            {
                /*
                 * This is a key spec to traverse to the next window
                 * via the keyboard.
                 */
		
                UnpostMenu (wmGD.menuActive);
                doXtDispatchEvent = False;
            }
        }
        if (wmGD.menuActive && wmGD.F_PrevKeySpec)
        {
            if (((wmGD.F_PrevKeySpec->state == event->state) ||
                 (wmGD.F_PrevKeySpec->state == NOLOCKMOD(event->state))) &&
                (wmGD.F_PrevKeySpec->keycode == event->button))
            {
                /*
                 * This is a key spec to traverse to the previous window
                 * via the keyboard.
                 */
		
                UnpostMenu (wmGD.menuActive);
                doXtDispatchEvent = False;
            }
        }
#endif /*  ROOT_ICON_MENU */
    }

    else if (wmGD.checkHotspot &&
	     ((event->type == ButtonPress) || 
	      (event->type == ButtonRelease)) &&
	     (event->x_root >= wmGD.hotspotRectangle.x) &&
	     (event->y_root >= wmGD.hotspotRectangle.y) &&
	     (event->x_root < (wmGD.hotspotRectangle.x + 
	                       (short) wmGD.hotspotRectangle.width)) &&
	     (event->y_root < (wmGD.hotspotRectangle.y + 
	                       (short) wmGD.hotspotRectangle.height))&&
#ifdef WSM
	     (pCD || 
	      (wmGD.rootButtonClick && wmGD.clickData.clickPending)))
#else /* WSM */
	     pCD)
#endif /* WSM */
    {
	/*   ^^^
	 * Added check for NULL pCD in the above condition.
	 * We should never get here with a NULL pCD, but, 
	 * sometimes our UnmapCallback for a menu does not
	 * get called, so..., we get to this point because
	 * wmGD.menuActive is not cleared, but, wmGD.menuClient 
	 * is set to NULL when we unmanage the client window.
	 */
	
	/*
	 * The event triggers hotspot processing for the system menu button
	 * or an icon.
	 */

	if (event->type == ButtonRelease)
	{
#ifdef WSM
          if (pCD)
	  {
#endif /* WSM */
	    /*
	     * The system menu is posted from a system menu button or an
	     * icon.  By doing a button release over the system menu button
	     * or icon the system menu that is posted is put into keyboard
	     * traversal mode.
	     */

	    ProcessClickBRelease (event, pCD, wmGD.clickData.context,
		wmGD.clickData.subContext);

	    if (wmGD.clickData.context == F_SUBCONTEXT_W_SYSTEM)
	    {
		PopGadgetOut (pCD, FRAME_SYSTEM);
	    }
#ifdef MOTIF_ONE_DOT_ONE
	    TraversalOn (pCD->systemMenuSpec);
	    doXtDispatchEvent = False;
#else
 	    _XmGetMenuState(XtParent(pCD->systemMenuSpec->menuWidget))
		->MS_LastManagedMenuTime = ((XButtonEvent *)event)->time;
	    doXtDispatchEvent = True;
#endif
#ifdef WSM
          }
	  else if ((!wmGD.clickData.pCD) && 
	      (((XButtonEvent *)event)->button == wmGD.clickData.button) &&
	      ((((XButtonEvent *)event)->state == 
				wmGD.clickData.releaseState) ||
	       (NOLOCKMOD(((XButtonEvent *)event)->state) == 
				wmGD.clickData.releaseState)))
	  {
	      /*
	       * This is a button release over the root. Check for
	       * root menu click and keep the menu up in a sticky
	       * fashion.
	       */
		Time timeDiff;

		/* 
		 * Check click time 
		 */
		 if (((XButtonEvent *)event)->time > wmGD.clickData.time)
		 {
		   timeDiff = 
		     ((XButtonEvent *)event)->time - wmGD.clickData.time;
		 }
		 else
		 {
		   timeDiff = 
		     ~wmGD.clickData.time + ((XButtonEvent *)event)->time + 1;
		 }

		 if (timeDiff < wmGD.doubleClickTime)
		 {
#ifdef MOTIF_ONE_DOT_ONE
		   TraversalOn (wmGD.menuActive);
		   doXtDispatchEvent = False;
#else
		   _XmGetMenuState (XtParent(wmGD.menuActive->menuWidget))
		       ->MS_LastManagedMenuTime =
			   ((XButtonEvent *)event)->time;
		   doXtDispatchEvent = True;
#endif
		 }
	    wmGD.clickData.clickPending = False;
	    }
#endif /* WSM */
	}
	else
	{
	    /*
	     * A button press over a system menu button or an icon when the
	     * system menu is posted indicates that a double-click action is
	     * to be done if appropriate and the menu is to be taken
	     * out of traversal mode (done by the menu widget).
	     */

	    ProcessClickBPress (event, pCD, wmGD.clickData.context,
				wmGD.clickData.subContext);

	    if (wmGD.clickData.subContext == F_SUBCONTEXT_W_SYSTEM)
	    {
		PushGadgetIn (pCD, FRAME_SYSTEM);
	    }

	    if (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_W_SYSTEM)
	    {
		if (wmGD.systemButtonClick2 &&
		    (pCD->clientFunctions & MWM_FUNC_CLOSE))
		{
		    /*
		     * Close the client window.  Cancel other system menu
		     * button actions.
		     */

		    UnpostMenu (pCD->systemMenuSpec);
		    F_Kill (NULL, pCD, (XEvent *) event);
		    doXtDispatchEvent = False;
		}
	    }
	    else
	    if (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_I_ALL)
	    {
		/*
		 * Normalize the icon.
		 */
		int newState;

		UnpostMenu (pCD->systemMenuSpec);
		if (pCD->maxConfig)
		{
		    newState = MAXIMIZED_STATE;
		}
		else
		{
		    newState = NORMAL_STATE;
		}

		SetClientState (pCD, newState, event->time);
		wmGD.clickData.clickPending = False;
		wmGD.clickData.doubleClickPending = False;
		doXtDispatchEvent = False;
	    }
	    else
	    if ((wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_IICON)||
		(wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_WICON))
            {
                /*
                 * Raise the Window and Normalize
                 */
		
                UnpostMenu (pCD->systemMenuSpec);
		F_Restore_And_Raise ((String)NULL, pCD, (XEvent *)NULL);
/*		F_Normalize_And_Raise ((String)NULL, pCD, (XEvent *)NULL);
*/                doXtDispatchEvent = False;
            }

	    /*
	     * Else no special button press processing; have the toolkit
	     * dispatch the event to the menu widgets.
	     */
	}
    }

    return (doXtDispatchEvent);


} /* END OF FUNCTION WmDispatchMenuEvent */



/*************************************<->*************************************
 *
 *  WmDispatchWsEvent (event)
 *
 *
 *  Description:
 *  -----------
 *  This function detects and dispatches events that are reported to the root
 *  (workspace) window and that are not widget-related (i.e. they would not be
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

Boolean WmDispatchWsEvent (XEvent *event)
{
    ClientData *pCD;
    Boolean dispatchEvent = False;
    WmScreenData *pSD;


    /*
     * Detect and dispatch non-widget events that have been reported to
     * the root window.
     */

    switch (event->type)
    {
	case KeyPress:
	{
	    /*
	     * The key press is to initiate some window management
	     * function (e.g., shuffle the client windows).
	     */

	    dispatchEvent = HandleWsKeyPress ((XKeyEvent *)event);
	    break;
	}

	case ButtonPress:
	{
	    /*
	     * The button press is to initiate some window management
	     * function (e.g., pop up the desktop menu).
	     */

	    if (wmGD.menuActive)
	    {
		dispatchEvent = True; /* have the toolkit dispatch the event */
	    }
	    else
	    {
		HandleWsButtonPress ((XButtonEvent *)event);
	    }
	    break;
	}

	case ButtonRelease:
	{
	    /*
	     * The button release may do some window management
	     * function.
	     */

	    if (wmGD.menuActive)
	    {
		dispatchEvent = True; /* have the toolkit dispatch the event */
	    }
	    else
	    {
		HandleWsButtonRelease ((XButtonEvent *)event);
	    }
	    break;
	}

	case UnmapNotify:
	{
	  /* BEGIN CR 5183 */
	  if ( (!XFindContext (DISPLAY, event->xunmap.window,
			       wmGD.windowContextType,
			       (XPointer *)&pCD)
		)
	      && (((XUnmapEvent *)event)->window == pCD->client)
	      )
	  /* END CR 5183 */
	    {
		/*
		 * This is a synthetic UnmapNotity used to withdraw a client
		 * window form window manager control.
		 */

		UnManageWindow (pCD);
	    }
	    break;
	}

	case EnterNotify:
	{
	    HandleWsEnterNotify ((XEnterWindowEvent *)event);
	    break;
	}

	case LeaveNotify:
	{
	    HandleWsLeaveNotify ((XLeaveWindowEvent *)event);
	    break;
	}

	case ConfigureRequest:
	{
	    HandleWsConfigureRequest ((XConfigureRequestEvent *)event);
	    break;
	}

	case MapRequest:
	{
	    /*
	     * Determine if the window is already being managed:
	     */

	    if ((XFindContext (DISPLAY, event->xmaprequest.window,
		    wmGD.windowContextType, (caddr_t *)&pCD)) &&
		(pSD = GetScreenForWindow (event->xmaprequest.window)))
	    {
	        /*
                 * The window is not yet managed and it's parented to a 
		 * screen/root window that we manage. Start to manage the
		 * new window.  Management details are dependent on the
		 * type of the window.  For a typical top-level application
		 * window reparent the window to a window frame, add it to
		 * the wm saveset, ...
                 */

                ManageWindow (pSD, event->xmaprequest.window, MANAGEW_NORMAL);
	    }
	    /* else ...
	     * The context information on the window WAS found.
	     * The window is already managed by the window manager
	     * so this is redundant request to have the client
	     * window managed.
	     */

	    break;
	}

	case FocusIn:
	{
	    HandleWsFocusIn ((XFocusInEvent *)event);
	    break;
	}

	case FocusOut:
	{
	    break;
	}

    } /* end of event.type switch */


    return (dispatchEvent);

} /* END OF FUNCTION WmDispatchWsEvent */



/*************************************<->*************************************
 *
 *  HandleWsKeyPress (keyEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes KeyPress events that are reported to the root
 *  window.  These events are generally associated with accelerators.
 *
 *
 *  Inputs:
 *  ------
 *  keyEvent = pointer to a key press event on the root window.
 *
 *  Output:
 *  ------
 *  RETURN = True is the event is to be dispatched by XtDispatch.
 *
 *************************************<->***********************************/

Boolean HandleWsKeyPress (XKeyEvent *keyEvent)
{
    Boolean      dispatchEvent = False;
    Boolean      checkKeyEvent = True;
    unsigned int n;
    Context      context;

    if (wmGD.menuActive)
    {
	/*
	 *  The active menu accelerators have been checked and keyEvent was
	 *  not one of them.  We will check for pass keys mode and then 
	 *  have the toolkit dispatch the event, without searching any other 
	 *  key or accelerator specification list.
	 */

	dispatchEvent = True;
	checkKeyEvent = False;
    }

    /*
     * If pass keys is active then only check for getting out of the
     * pass keys mode.  Unfreeze the keyboard and replay the key if
     * pass keys is active.
     */

    if (wmGD.passKeysActive)
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


    /*
     * Search through the key specification list and the menu 
     * accelerator lists until these lists are exhausted or
     * the event is handled.
     */

    if (checkKeyEvent)
    {
        if (wmGD.keyboardFocus)
        {
	    if (wmGD.keyboardFocus->clientState == MINIMIZED_STATE)
	    {
	        context = F_CONTEXT_ICON;
	    }
	    else if (wmGD.keyboardFocus->clientState == NORMAL_STATE)
	    {
	        context = F_CONTEXT_NORMAL;
	    }
	    else
	    {
	        context = F_CONTEXT_MAXIMIZE;
	    }
        }
        else
        {
	    context = F_CONTEXT_ROOT;
        }

        if (HandleKeyPress (keyEvent, ACTIVE_PSD->keySpecs, 
	                    TRUE, context, FALSE, (ClientData *)NULL) &&
	    ACTIVE_PSD->acceleratorMenuCount)
	{
	    for (n = 0; ((keyEvent->keycode != 0) &&
			 (n < ACTIVE_PSD->acceleratorMenuCount)); n++)
	    {
	        if (!HandleKeyPress (keyEvent,
		     ACTIVE_PSD->acceleratorMenuSpecs[n]->accelKeySpecs, 
				 TRUE, context, TRUE,(ClientData *)NULL))
	        {
		    break;
		}
	    }
        }

	/*
	 * Fix for CR 3117 - Do the XAllowEvents after calling HandleKeyPress so that
	 *                   keys meant for an application can be sent to it.
	 */
	XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);
	/*
	 * End Fix for CR 3117
	 */
    }

    return (dispatchEvent);

} /* END OF FUNCTION HandleWsKeyPress */



/*************************************<->*************************************
 *
 *  HandleKeyPress (keyEvent, keySpecs, checkContext, context, onlyFirst, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies window manager functions that are triggered by
 *  a KeyPress event.  The window manager functions are done if appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  keyEvent = pointer to a key press event on the root window
 *  keySpecs = pointer to a key specification list to search
 *  checkContext = TRUE iff the context must match the keySpec context.
 *  context = context to match keySpec context.
 *  onlyFirst = TRUE iff key processing should stop with the first match.
 *
 *  Output:
 *  ------
 *  RETURN = False if key binding processing should be terminated; True if
 *	key binding processing can continue
 *
 *************************************<->***********************************/

Boolean HandleKeyPress (XKeyEvent *keyEvent, 
			KeySpec *keySpecs, 
			Boolean checkContext, 
			Context context, 
			Boolean onlyFirst, 
			ClientData *pCD)
{
  Boolean     processKey = True;
  ClientData *functionClient;
  Boolean   haveRootBinding = False;
  Boolean   haveWindowBinding = False;

  /*
   * Search for matching key specification.
   */

  while (processKey && keySpecs)
    {
      if (((keyEvent->state == keySpecs->state) ||
           (NOLOCKMOD(keyEvent->state) == keySpecs->state)) &&
	  (keyEvent->keycode == keySpecs->keycode))
	{
	  if ((!checkContext) || (context & keySpecs->context))
	    {
	      /*
	       * A matching key binding has been found.
	       * Determine the client to which the key binding function is to
	       *   apply.
	       * Unpost any active menu and specify that no futher key binding 
	       *   processing should be done.
	       * Do the function associated with the matching key binding.
	       * Stop if onlyFirst == TRUE
	       */

	      if (pCD)
		{
		  functionClient = pCD;
		}
	      else
		{
		  functionClient = wmGD.keyboardFocus;
		}

	      if (wmGD.menuActive)
		{
		  functionClient = wmGD.menuClient;  /* might not have focus! */
		  UnpostMenu (wmGD.menuActive);
		  processKey = False;
		}
	      else if (onlyFirst)
		{
		  processKey = False;
		}

	      if ((keySpecs->wmFunction == F_Menu) ||
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
		  (keySpecs->wmFunction == F_Post_RMenu) ||
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
		  (keySpecs->wmFunction == F_Post_SMenu))
		{
		  wmGD.menuUnpostKeySpec = keySpecs;  /* menu unpost key spec */
		}
	      else if (keySpecs->wmFunction == F_Pass_Key)
		{
		  wmGD.passKeysKeySpec = keySpecs;
		}
#ifdef ROOT_ICON_MENU
              else if (keySpecs->wmFunction == F_Next_Key)
		{
                  wmGD.F_NextKeySpec = keySpecs;
		}
              else if (keySpecs->wmFunction == F_Prev_Key)
		{
                  wmGD.F_PrevKeySpec = keySpecs;
		}
#endif /* ROOT_ICON_MENU */
	      if (!(keySpecs->wmFunction (keySpecs->wmFuncArgs,
					  functionClient, keyEvent)))
		{
		  /*
		   * The window manager function return indicates that further
		   * key binding processing should not be done.
		   */

		  processKey = False;
		}
	      /*
	       * Note that for key bindings, frame, title, border, and app contexts
	       * are equivalent to the window context. This is NOT the same as for
	       * button bindings.
	       */
	      if ((context & (F_CONTEXT_WINDOW)))
		haveWindowBinding = True;
	    }
	  /* Fix for 3117 -- If the keypress looks as if it had been intended
	   *                 for the application, send it back.
	   */
	  
	  else if ((context & (F_CONTEXT_WINDOW)) &&
		   (keySpecs->context & F_CONTEXT_ROOT))
	    {
	      haveRootBinding = True;
	    }
	}
	keySpecs = keySpecs->nextKeySpec;

      }

    if (haveRootBinding && (!haveWindowBinding) )
      {
	XAllowEvents (DISPLAY, ReplayKeyboard, CurrentTime);
      }

    return (processKey);


} /* END OF FUNCTION HandleKeyPress */



/*************************************<->*************************************
 *
 *  HandleWsButtonPress (buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies button events that are associated with window
 *  manager functions.  Window manager functions are done if appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event on the root window
 *
 *************************************<->***********************************/

void HandleWsButtonPress (XButtonEvent *buttonEvent)
{
    ClientData *pCD;
    Context context;
    int partContext;
    Context subContext;


    /*
     * Determine if the top-level window that contains the pointer is a
     * client managed by the window manager (there may be no window under
     * the pointer or it may be an "override-redirect" window).
     */

    if ((buttonEvent->subwindow == None) ||
	(XFindContext (DISPLAY, buttonEvent->subwindow, wmGD.windowContextType,
	     (caddr_t *)&pCD)))
    {
	/* no managed window under the pointer */
	pCD = NULL;
    }
    

    /*
     * Look through the window manager function button binding list for
     * matches with the event:
     */

    IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
    subContext = (1L << partContext);

    ProcessClickBPress (buttonEvent, pCD, context, subContext);

    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) && pCD)
    {
	/*
	 * Button bindings have been processed, now check for bindings that
	 * are associated with the built-in semantics of the window frame
	 * decorations.
	 */

	CheckButtonPressBuiltin (buttonEvent, context, subContext, partContext,
	    pCD);
    }
    /*
     * Else skip built-in processing due to execution of a function that
     * does on-going event processing or that has changed the client state
     * (e.g., f.move or f.minimize).
     */


} /* END OF FUNCTION HandleWsButtonPress */



/*************************************<->*************************************
 *
 *  HandleWsButtonRelease (buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies button release events that are associated with
 *  window manager functions.  Window manager functions are done if
 *  appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button release event
 * 
 *************************************<->***********************************/

void HandleWsButtonRelease (XButtonEvent *buttonEvent)
{
    ClientData *pCD;
    Context context;
    int  partContext;
    Context subContext;


    /*
     * Determine if the top-level window that contains the pointer is a
     * client managed by the window manager (there may be no window under
     * the pointer or it may be an "override-redirect" window).
     */

    if ((buttonEvent->subwindow == None) ||
	(XFindContext (DISPLAY, buttonEvent->subwindow, wmGD.windowContextType,
	     (caddr_t *)&pCD)))
    {
	/* no managed window under the pointer */
	pCD = NULL;
    }
    

    /*
     * Look for a builtin function that may be done by this event.
     */

    IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
    subContext = (1L << partContext);

    ProcessClickBRelease (buttonEvent, pCD, context, subContext);

    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) && pCD)
    {
	/*
	 * Button bindings have been processed, now check for bindings that
	 * are associated with the built-in semantics of the window frame
	 * decorations.
	 */

	CheckButtonReleaseBuiltin (buttonEvent, context, subContext, pCD);
    }
    /*
     * Else skip built-in processing due to execution of a function that
     * does on-going event processing or that has changed the client state
     * (e.g., f.move or f.minimize).
     */


} /* END OF FUNCTION HandleWsButtonRelease */



/*************************************<->*************************************
 *
 *  CheckForButtonAction (buttonEvent, context, subContext, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a button event is to do a button binding
 *  action.  The action is done if specified.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = a button event handled by the window manager
 *
 *  context = button event context (root, icon, window)
 *
 *  subContext = button event subContext (title, system button, etc.)
 *
 *  pCD = a pointer to client data that is associated with the button event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True then further button binding processing can be done;
 *      if false then a state change function, menu function, or
 *      configuration function is ongoing and further button binding
 *      processing should not be done.
 *
 *
 *************************************<->***********************************/

Boolean CheckForButtonAction (XButtonEvent *buttonEvent, Context context, Context subContext, ClientData *pCD)
{
    ButtonSpec *buttonSpec;

    /*
     * Look through the window manager function button binding list for
     * matches with the event:
     */

    buttonSpec = ACTIVE_PSD->buttonSpecs;
    while (buttonSpec)
    {
	if ((buttonEvent->button == buttonSpec->button) &&
	    ((buttonEvent->state == buttonSpec->state) ||
	     (NOLOCKMOD(buttonEvent->state) == buttonSpec->state)))
	{
	    /*
	     * See if the event context matches the binding context.
	     */

	    if ((buttonEvent->type == buttonSpec->eventType) &&
	        (context & buttonSpec->context) &&
		(subContext & buttonSpec->subContext))
	    {

		/*
		 * For click type bindings check for a match between the
		 * event context and the click / double-click context.
		 */

	        if (buttonEvent->type == ButtonRelease)
	        {
		    /*
		     * Clicks occur on button releases.  A button release
		     * binding is always treated as a click binding.
		     */

		    if ((buttonSpec->subContext | wmGD.clickData.clickContext)
			 != buttonSpec->subContext)
		    {
		        /* click binding and event contexts do not match */
			buttonSpec = buttonSpec->nextButtonSpec;
		        continue;
		    }
		    /* else there is a click match */
	        }
		else if (buttonSpec->click && (buttonEvent->type==ButtonPress))
		{
		    /*
		     * Double-clicks occur on button presses.
		     */

		    if ((buttonSpec->subContext |
					wmGD.clickData.doubleClickContext)
			!= buttonSpec->subContext)
		    {
			/* click binding and event contexts do not match */
			buttonSpec = buttonSpec->nextButtonSpec;
			continue;
		    }
		    else
		    {
		        /*
			 * The is a double-click match.  Don't do any click
			 * or double-click matches for the following button
			 * press and release.
			 */

			wmGD.clickData.clickPending = False;
			wmGD.clickData.doubleClickPending = False;
		    }
		}

	        if (!(buttonSpec->wmFunction (buttonSpec->wmFuncArgs, pCD,
					      buttonEvent)))
		{
		    /*
		     * The window manager function return indicates that
		     * further button binding processing should not be done.
		     */

		    return (False);
		}
	    }
	}
	buttonSpec = buttonSpec->nextButtonSpec;
    }

    return (True);


} /* END OF FUNCTION CheckForButtonAction */



/*************************************<->*************************************
 *
 *  IdentifyEventContext (event, pCD, pContext, pPartContext)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the context in which an event occured.  The 
 *  pointer position is used to identify the context if the event is a
 *  button event.  If the context and the window state are incompatible
 *  (e.g., the context is window and the window is minimized) then the
 *  context is reset to 0 (none).
 *
 *
 *  Inputs:
 *  ------
 *  event = find the context of this X event
 *
 *  pCD = client data (maybe NULL) that the event is associated with
 *
 * 
 *  Outputs:
 *  -------
 *  pContext = event context
 *
 *  pPartContext = part (e.g, frame) context associated with the event
 *
 *************************************<->***********************************/

void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, Context *pContext, int *pPartContext)
{
    Boolean eventOnRoot;
    Window actionWindow;
    int clientX;
    int clientY;
    int framePart;


    eventOnRoot = (event->window == ACTIVE_ROOT) ? 
				True : False;

    if (pCD)
    {
	actionWindow = (eventOnRoot) ? event->subwindow : event->window;
	if (actionWindow == pCD->clientFrameWin)
	{
	    *pContext = F_CONTEXT_WINDOW;

	    if (eventOnRoot)
	    {
	        clientX = event->x -
			  (pCD->maxConfig ? pCD->maxX : pCD->clientX) +
			  pCD->clientOffset.x;
	        clientY = event->y -
			  (pCD->maxConfig ? pCD->maxY : pCD->clientY) +
			  pCD->clientOffset.y;
	    }
	    else
	    {
		clientX = event->x;
		clientY = event->y;
	    }
	    framePart = IdentifyFramePart (pCD, clientX, clientY);
	    *pPartContext = framePart;
	}
	else if (actionWindow == pCD->clientBaseWin)
	{
	    *pContext = F_CONTEXT_WINDOW;
	    *pPartContext = FRAME_CLIENT;
	}
	else if ((actionWindow == ICON_FRAME_WIN(pCD)) ||
		 (actionWindow == ACTIVE_PSD->activeIconTextWin))
	{
	    if (P_ICON_BOX(pCD))
	    {
	        *pContext = F_CONTEXT_ICONBOX;
		if (pCD->clientState == MINIMIZED_STATE)
		{
		    *pPartContext = ICONBOX_PART_IICON;
		}
		else
		{
		    *pPartContext = ICONBOX_PART_WICON;
		}
	    }
	    else
	    {
	        *pContext = F_CONTEXT_ICON;
	        *pPartContext = ICON_PART_ALL;
	    }
	}
	else
	{
	    *pContext = F_CONTEXT_ROOT;
	    *pPartContext = ROOT_PART_ALL;
	}

	/*
	 * Check for an incompatible context and window state.
	 */

	if (((*pContext & F_CONTEXT_WINDOW) &&
	     (pCD->clientState != NORMAL_STATE) &&
	     (pCD->clientState != MAXIMIZED_STATE)) ||
	    ((*pContext & F_CONTEXT_ICON) &&
	     (pCD->clientState != MINIMIZED_STATE)))
	{
	    *pContext = F_CONTEXT_NONE;
	}
    }
    else
    {
	*pContext = F_CONTEXT_ROOT;
	*pPartContext = ROOT_PART_ALL;
    }


} /* END OF FUNCTION IdentifyEventContext */



/*************************************<->*************************************
 *
 *  ProcessClickBPress (buttonEvent, pCD, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  This function checks for a double-click match and saves state information
 *  to do click and double-click processing.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event
 *
 *  pCD = pointer to client data (identifies client window)
 *
 *  context = root/window/icon context for the event
 *
 *  subContext = subcontext for the event (title, system button, etc.)
 *
 * 
 *  Outputs:
 *  -------
 *  (wmGD.clickData) = click processing information
 *
 *  (wmGD.clickData.doubleClickContext) = set if double click occured
 *
 *************************************<->***********************************/

void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, Context context, Context subContext)
{
    Time timeDiff;
    Boolean passButton;


    /*
     * Check for a double-click.  If a double click has occurred then
     * save the double-click context.
     */

    wmGD.clickData.doubleClickContext = F_SUBCONTEXT_NONE;
    if (wmGD.clickData.doubleClickPending &&
	(buttonEvent->button == wmGD.clickData.button) &&
        ((buttonEvent->state == wmGD.clickData.state) ||
         (NOLOCKMOD(buttonEvent->state) == wmGD.clickData.state)) &&
	(pCD == wmGD.clickData.pCD) &&
	(context == wmGD.clickData.context))
    {
	/*
	 * Check the time between button release events.
	 */

	if (buttonEvent->time > wmGD.clickData.time)
	{
	    timeDiff = buttonEvent->time - wmGD.clickData.time;
	}
	else
	{
	    timeDiff = ~wmGD.clickData.time + buttonEvent->time + 1;
	}

	if (timeDiff < wmGD.doubleClickTime)
	{
	    /*
	     * A double-click has been done; save the context.
	     */

	    wmGD.clickData.doubleClickContext = subContext |
						wmGD.clickData.subContext;
	}
    }


    /*
     * Save state data for click checking.  If a button binding match
     * occurs for a double-click then clear out the clickData (don't
     * do any click/double-click matches for the following button press
     * and release).  If the button press is done on the client area and
     * is used to set the focus to the window then don't use it in
     * setting up clickData.
     */

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

    if (!(pCD && (buttonEvent->window == pCD->clientBaseWin) && passButton))
    {
        wmGD.clickData.button = buttonEvent->button;
        wmGD.clickData.state = buttonEvent->state;
        /* add in event button mask (this will show up in the button release */
        wmGD.clickData.releaseState = buttonEvent->state |
				    buttonModifierMasks[buttonEvent->button];
        wmGD.clickData.pCD = pCD;
        wmGD.clickData.context = context;
        wmGD.clickData.subContext = subContext;
        wmGD.clickData.time = buttonEvent->time;
        wmGD.clickData.clickPending = True;
        wmGD.clickData.doubleClickPending = True;
#ifdef WSM
        wmGD.clickData.bReplayed = wmGD.bReplayedButton;
#endif /* WSM */
    }


} /* END OF FUNCTION ProcessClickBPress */



/*************************************<->*************************************
 *
 *  ProcessClickBRelease (buttonEvent, pCD, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a "click" was done.  The button release
 *  completes a click if there is a click pending and the button release
 *  context is the same as the button press context.  Configuration or
 *  menu activity cancels a pending click.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event
 *
 *  pCD = pointer to client data (identifies client window)
 *
 *  context = root/window/icon context for the event
 *
 *  subContext = window subcontext for the event (title, system button, etc.)
 *
 *  (wmGD.clickData) = click processing information
 *
 * 
 *  Outputs:
 *  -------
 *  (wmGD.clickData) = click processing information
 *
 *  (wmGD.clickData.clickContext) = set if click occured
 * 
 *************************************<->***********************************/

void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, Context context, Context subContext)
{

    /*
     * Restore the state of the last "depressed" frame gadget
     */

    if (pCD && (wmGD.gadgetClient == pCD) && (pCD->decorFlags))
    {
	PopGadgetOut(pCD, wmGD.gadgetDepressed);
    }
	

    /*
     * Check to see if a click has been done.
     */

    wmGD.clickData.clickContext = F_SUBCONTEXT_NONE;
    if (wmGD.clickData.clickPending &&
	(buttonEvent->button == wmGD.clickData.button) &&
	(buttonEvent->state == wmGD.clickData.releaseState) &&
	(pCD == wmGD.clickData.pCD) &&
	(context == wmGD.clickData.context))
    {
	wmGD.clickData.clickContext = subContext | wmGD.clickData.subContext;
	/* !!! check for double click time? !!! */
    }
    else
    {
	wmGD.clickData.doubleClickPending = False;
    }

    wmGD.clickData.clickPending = False;


} /* END OF FUNCTION ProcessClickBRelease */


#ifdef WSM

/*************************************<->*************************************
 *
 *  HandleDtWmClientMessage (clientEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function handles client message events that are sent to the 
 *  wm window.  The window manager action that is taken depends on the
 *  message_type of the event.
 *
 *
 *  Inputs:
 *  ------
 *  clientEvent = pointer to a client message event on the wm window
 * 
 *************************************<->***********************************/

void HandleDtWmClientMessage (XClientMessageEvent *clientEvent)
{
    int scr;
    /*
     * Process the client message event based on the message_type.
     */
    
    if (clientEvent->message_type == wmGD.xa_DT_SM_WM_PROTOCOL)
    {
	if (clientEvent->data.l[0] == wmGD.xa_DT_SM_START_ACK_WINDOWS)
	{
	    smAckState = SM_START_ACK;
	}
	else if (clientEvent->data.l[0] == wmGD.xa_DT_SM_STOP_ACK_WINDOWS)
	{
	    smAckState = SM_STOP_ACK;
	}
    }

    if (clientEvent->message_type == wmGD.xa_WM_PROTOCOLS)
    {
	if (clientEvent->data.l[0] == wmGD.xa_WM_SAVE_YOURSELF)
	{
	    for (scr = 0; scr < wmGD.numScreens; scr++)
	    {
		if (wmGD.Screens[scr].managed)
		{
		    /*
		     * Write out current workspace, frontpanel 
		     * position and iconbox position and size.
		     */
		    SaveResources(&wmGD.Screens[scr]);
		}
	    } /*  for loop */
	    XSetCommand(DISPLAY, wmGD.commandWindow, 0, 0);

	} /* WM_SAVE_YOURSELF */     
    } /* WM_PROTOCOLS */
} /* END OF FUNCTION HandleDtWmClientMessage */


/*************************************<->*************************************
 *
 *  HandleDtWmRequest (pSD, pev)
 *
 *
 *  Description:
 *  -----------
 *  This function processes _DT_WM_REQUESTs that come in from 
 *  other clients
 *
 *
 *  Inputs:
 *  ------
 *  pSD - pointer to screen data
 *  pev - pointer to the triggering event (PropertyNotify)
 *
 *  Comments:
 *  ---------
 *  This reuses the global parse buffer. It assumes that no parsing 
 *  is in progress. All parsing of the config file must be completed 
 *  before we call this routine. 
 *
 *
 *************************************<->***********************************/

void 
HandleDtWmRequest (WmScreenData *pSD, XEvent *pev)
{
    Boolean more = True;
    char *pchReq = NULL;
    String sRequest = NULL;
    unsigned char *lineP;
    int iFuncIndex;
    WmFunction   wmFunction;
    String       wmFuncArgs;
    ClientData	 *pCD;
    Context	 ctxDisallowed;
    DtWmpParseBuf wmPB;

    /*
     * Save state of global parse buffer
     */
    memcpy (&wmPB, wmGD.pWmPB, sizeof(DtWmpParseBuf));

    while (more)
    {
	GetDtWmRequest (pSD, &pchReq, &more);

	if (pchReq)
	{
	    pCD = NULL;
	    ctxDisallowed = F_CONTEXT_ROOT;
	    if (wmGD.requestContextWin != (Window) 0L)
	    {
		if (!XFindContext (DISPLAY, wmGD.requestContextWin, 
					wmGD.windowContextType,
					(caddr_t *)&pCD))
		{
		    /* 
		     * A valid client window was specified
		     * in a previous F_Set_Context request.
		     * Remove the restriction to root-only context.
		     */
		    ctxDisallowed = F_CONTEXT_NONE;
		}
	    }
	    sRequest = XtNewString (pchReq);
	    _DtWmParseSetLine (wmGD.pWmPB, (unsigned char *)sRequest);
	    lineP = wmGD.pWmPB->pchLine;
            iFuncIndex = ParseWmFunction (&lineP, CRS_BUTTON|CRS_KEY, 
		&wmFunction);

	    if (iFuncIndex != F_NOP_INDEX)
	    {
		if (functionTable[iFuncIndex].greyedContext & ctxDisallowed)
		{
		    /* 
		     * Sorry, we have to disallow this function request
		     * based on context problems.
		     */
		    XtFree ((char *)sRequest);
		    sRequest = NULL;
		    break;
		}

		/*
		 * Apply the function argument parser.
		 */
		if ((*(functionTable [iFuncIndex].parseProc))
			   (&lineP, wmFunction, &wmFuncArgs))
		{
		    /* 
		     * Found it in the function table! 
		     * Apply the function.
		     */
		    wmFunction (wmFuncArgs, pCD, NULL);

		    /*
		     * Free up allocated args, if any
		     */
		    if (wmFuncArgs)
		    {
			if ((functionTable[iFuncIndex].parseProc ==
			    		ParseWmFuncStrArg) ||
			    (functionTable[iFuncIndex].parseProc ==
					ParseWmFuncMaybeStrArg))
			{
			    XtFree ((char *)wmFuncArgs);
			}
			else if ((functionTable[iFuncIndex].parseProc ==
			    		ParseWmFuncActionArg))
			{
			    WmActionArg *pAP = (WmActionArg *) wmFuncArgs;

			    if (pAP->actionName)
				XtFree ((char *) pAP->actionName);
			    if (pAP->szExecParms)
				XtFree ((char *) pAP->szExecParms);
			    while (pAP->numArgs > 0)
			    {
				XtFree ((char *)
				    pAP->aap[--(pAP->numArgs)].u.file.name);
			    }
			    XtFree ((char *) pAP);
			}
		    }
		}
	    }
	    else if (!strncmp (pchReq, DTWM_REQ_CHANGE_BACKDROP,
			strlen(DTWM_REQ_CHANGE_BACKDROP)))
	    {
		Pixmap pixmap = None;
		char *pch;
		char *pchFile = NULL;

		/* skip function name */
		pch = pchReq;
		(void) strtok (pch, " ");

		/* get path name */
		pch = strtok (NULL, " ");
		if (pch)
		{
		    pchFile = (char *) XtMalloc (1+strlen(pch));
		}
		else
		{
		    Warning (((char *)GETMESSAGE(32, 3, "Missing path name for backdrop change request.")));

		}
		if (pchFile)
		{
		    strcpy (pchFile, pch);

		    /* get pixmap id */
		    pch = strtok (NULL, " ");
		    if (pch) 
		    {
			sscanf (pch, "%lx", &pixmap);  
			SetNewBackdrop (ACTIVE_WS, pixmap, (String)pchFile);
		    }
		    else 
		    {
			Warning (((char *)GETMESSAGE(32, 4, "Missing pixmap id for backdrop change request.")));
		    }
		    XtFree (pchFile);
		}
		else
		{
		    Warning (((char *)GETMESSAGE(32, 2, "Insufficient memory to handle backdrop change.")));
		}
	    }
	    if (sRequest)
	    {
		XtFree ((char *) sRequest);
	    }
	    XtFree (pchReq);
	}
    }

    /*
     * Restore state of global parse buffer
     */
    memcpy (wmGD.pWmPB, &wmPB, sizeof(DtWmpParseBuf));

} /* END OF FUNCTION HandleDtWmRequest */


#endif /* WSM */


/*************************************<->*************************************
 *
 *  HandleWsEnterNotify (enterEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes EnterNotify events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  enterEvent = pointer to an enter notify event on the root window.
 *
 *************************************<->***********************************/

void HandleWsEnterNotify (XEnterWindowEvent *enterEvent)
{
    WmScreenData *pSD;

    /*
     * If the pointer entered a screen that we manage, then set the
     * new active screen.
     */
    if (wmGD.queryScreen &&
	(!XFindContext (DISPLAY, enterEvent->window, wmGD.screenContextType,
	    (caddr_t *)&pSD)))
    {
	SetActiveScreen (pSD);
    }

    /*
     * The root window was entered; do focus processing
     * if necessary:
     */
    

    if (!wmGD.menuActive &&
	((enterEvent->mode == NotifyNormal) ||
	 (enterEvent->mode == NotifyUngrab) ||
	 (enterEvent->mode == NotifyWhileGrabbed)))
    {
        if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	{
	    Do_Focus_Key ((ClientData *) NULL, enterEvent->time, 
			ALWAYS_SET_FOCUS);
	}
	else if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	         ((enterEvent->detail == NotifyNonlinearVirtual) ||
	          (enterEvent->detail == NotifyNonlinear)) &&
		 (wmGD.keyboardFocus == NULL) &&
		 enterEvent->focus)
	{
	    /*
	     * Reset the explicit selection focus to the workspace
	     * window.
	     */

	    Do_Focus_Key ((ClientData *) NULL, enterEvent->time, 
	    		ALWAYS_SET_FOCUS);
	}

        if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	{
	    SetColormapFocus (ACTIVE_PSD, (ClientData *) NULL);
	}
    }

} /* END OF FUNCTION HandleWsEnterNotify */



/*************************************<->*************************************
 *
 *  HandleWsLeaveNotify (leaveEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes LeaveNotify events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  enterEvent = pointer to an leave notify event on the root window.
 *
 *************************************<->***********************************/

void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent)
{
    WmScreenData *pSD;

    /*
     * The root window was exited; do focus processing
     * if necessary:
     */

    if (!wmGD.menuActive &&
	((leaveEvent->detail == NotifyNonlinear) ||
	(leaveEvent->detail == NotifyNonlinearVirtual)))
    {
	/*
	 * The pointer has moved to another screen.  Fix the
	 * focus on the screen controlled by the window manager.
	 */

        if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
            (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER))
	{
	    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	    {
	        Do_Focus_Key ((ClientData *) NULL, leaveEvent->time,
		    (SCREEN_SWITCH_FOCUS | ALWAYS_SET_FOCUS));
	    }
	    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	    {
	        SetColormapFocus (ACTIVE_PSD, (ClientData *) NULL);
	    }
	}

	/*  Set new active screen */

	if (!XFindContext (DISPLAY, leaveEvent->root, wmGD.screenContextType,
	    (caddr_t *)&pSD))
	{
	    /* moved to another screen we manage! */
	    SetActiveScreen (pSD);
	}
	else
	{
	    /* off onto an unmanaged screen */
	    wmGD.queryScreen = True;

	    /* set input focus to pointer root */
	    XSetInputFocus (DISPLAY, PointerRoot, 
		RevertToPointerRoot, leaveEvent->time);
	}
    }
} /* END OF FUNCTION HandleWsLeaveNotify */



/*************************************<->*************************************
 *
 *  HandleWsConfigureRequest (focusEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes ConfigureRequest events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  focusEvent = pointer to a configure request event on the root window.
 *
 *************************************<->***********************************/

void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent)
{
    ClientData *pCD;
    XConfigureEvent notifyEvent;
    Boolean configChanged;
    XWindowChanges values;


    /*
     * A window that is a child of the root window is being
     * configured.  Either it is an un-managed window or it is a
     * managed window that did the configuration before it was
     * reparented.
     */

    if (XFindContext (DISPLAY, configureEvent->window, wmGD.windowContextType,
	    (caddr_t *)&pCD))
    {
	/*
	 * Get window attribute information; this is used later on
	 * to decide if a synthetic ConfigureNotify event should
	 * be send to the client.
	 */

	if (WmGetWindowAttributes (configureEvent->window))
	{
	    configChanged =
		(wmGD.windowAttributes.x != configureEvent->x) ||
		(wmGD.windowAttributes.y != configureEvent->y) ||
		(wmGD.windowAttributes.width != configureEvent->width) ||
		(wmGD.windowAttributes.height != configureEvent->height) ||
		(wmGD.windowAttributes.border_width !=
				       configureEvent->border_width) ||
		(configureEvent->value_mask & (CWSibling|CWStackMode));

            /*
             * The window is not (yet) managed.  Do the window
	     * configuration.
             */

	    if (configChanged)
	    {
	        values.x = configureEvent->x;
	        values.y = configureEvent->y;
	        values.width = configureEvent->width;
	        values.height = configureEvent->height;
	        values.border_width = configureEvent->border_width;
	        values.sibling = configureEvent->above;
	        values.stack_mode = configureEvent->detail;
	        XConfigureWindow (DISPLAY, configureEvent->window,
	            (unsigned int) (configureEvent->value_mask), &values);
	    }

	    /*
	     * Some clients expect a ConfigureNotify event even if the
	     * XConfigureWindow call has NO effect.  Send a synthetic
	     * ConfigureNotify event just to be sure.
	     */

	    if (!configChanged)
	    {
	        notifyEvent.type = ConfigureNotify;
	        notifyEvent.display = DISPLAY;
	        notifyEvent.event = configureEvent->window;
	        notifyEvent.window = configureEvent->window;
	        notifyEvent.x = configureEvent->x;
	        notifyEvent.y = configureEvent->y;
	        notifyEvent.width = configureEvent->width;
	        notifyEvent.height = configureEvent->height;
	        notifyEvent.border_width = configureEvent->border_width;
	        notifyEvent.above = None;
	        notifyEvent.override_redirect = False;

	        XSendEvent (DISPLAY, configureEvent->window, False,
	            StructureNotifyMask, (XEvent *)&notifyEvent);
            }
        }
    }
    else
    {
        /*
         * The context information on the window WAS found.
         * The window is already managed by the window manager
         * so this is a configuration request that was made before
         * the window was reparented.
         */

	HandleCConfigureRequest (pCD, configureEvent);
    }

} /* END OF FUNCTION HandleWsConfigureRequest */



/*************************************<->*************************************
 *
 *  HandleWsFocusIn (focusEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes FocusIn events that are reported to the root
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  focusEvent = pointer to a focus in event on the root window.
 *
 *************************************<->***********************************/

void HandleWsFocusIn (XFocusInEvent *focusEvent)
{
    ClientData *pCD;
    Boolean sameScreen;

    /*
     * This code is used to handle the case of the focus being
     * set to pointer root (either explicitly by some client, by the window
     * manager or as a result of a "revert to" action).
     * It also handles the case where the focus is manipulated by a window
     * manager on another screen (in this case let the other window manager
     * control the focus). Reset the focus to a client window if appropriate.
     */

    if (((focusEvent->mode == NotifyNormal) ||
	 (focusEvent->mode == NotifyUngrab)) &&
	((focusEvent->detail == NotifyPointerRoot) ||
	 (focusEvent->detail == NotifyDetailNone) ||
	 (focusEvent->detail == NotifyInferior)))
    {
	/*
	 * Fix the keyboard focus if it should be set to a particular client.
	 */

        pCD = GetClientUnderPointer (&sameScreen);
	if (wmGD.keyboardFocus && (focusEvent->detail != NotifyInferior))
	{
	    if (sameScreen)
	    {
		/*
		 * Assume that the focus still belongs to the screen
		 * controlled by mwm.  Repair the focus if the client
		 * is still active.
		 */

		if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
		{
		    Do_Focus_Key (wmGD.keyboardFocus, GetTimestamp (),
			ALWAYS_SET_FOCUS);
		}
		else
		{
		    if (pCD || (focusEvent->detail == NotifyDetailNone))
		    {
			/* !!! check for redundant focus setting !!! */
	        	Do_Focus_Key (pCD, GetTimestamp (), ALWAYS_SET_FOCUS);
		    }
		}
		SetKeyboardFocus ((ClientData *) NULL, REFRESH_LAST_FOCUS);
	    }
	    else
	    {
	        /*
		 * Assume that the focus is now controlled by a
		 * window manager on another screen.  Clear the
		 * focus locally.
	         */

		SetKeyboardFocus ((ClientData *) NULL, REFRESH_LAST_FOCUS);
	    }
        }
        else
        {
	    /*
	     * No client window currently has the focus.  If the pointer
	     * is on the mwm-controlled screen set the focus to
	     * the window management window if the focus is explicit.
	     */

	    if (sameScreen)
	    {
		if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
		{
		    if (((focusEvent->detail == NotifyInferior) ||
			 (focusEvent->detail == NotifyPointerRoot)) &&
			(wmGD.keyboardFocus != wmGD.nextKeyboardFocus))
		    {
			/*
			 * Window that had the focus went away.  Try to
			 * reset the window to the next keyboard focus
			 * client window if there is one.
			 */

		        Do_Focus_Key (wmGD.nextKeyboardFocus, GetTimestamp (),
			    ALWAYS_SET_FOCUS);
		    }
		    else
		    {
		        /* Re: CR 4896                                          */
                        /* The previous version would pass NULL widget to this  */
                        /* this routine.  This doesn't seem to make sense. NULL */
                        /* has been replaced by pCD which seems to fix the icon */
                        /* focus problem.                                       */
                        /* Another related patch is made in WmCEvent.c.         */
		        Do_Focus_Key ((ClientData *) pCD, GetTimestamp(), 
					ALWAYS_SET_FOCUS);
		    }
		}
		else /*KEYBOARD_FOCUS_POINTER*/
		{
		    if (pCD || focusEvent->detail != NotifyPointerRoot)
		    {
		        Do_Focus_Key (pCD, GetTimestamp (), ALWAYS_SET_FOCUS);
		    }
		}
	    }
        }
    }

} /* END OF FUNCTION HandleWsFocusIn */



/*************************************<->*************************************
 *
 *  GetTimestamp ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to provide a timestamp for use with X calls that
 *  require a timestamp (and a timestamp is not available from a prior
 *  X event).
 *
 *
 *  Outputs:
 *  -------
 *  Return = a timestamp value
 *
 *  Comment: 
 *  --------
 *  This costs a server round-trip
 *
 *************************************<->***********************************/

Time GetTimestamp (void)
{
    Time timestamp;
    WmScreenData *pSD = ACTIVE_PSD;
    XEvent event;
    long property;

    /*
     * Do zero-length append to our own WM_STATE
     */
    XChangeProperty (DISPLAY, pSD->wmWorkspaceWin, wmGD.xa_WM_STATE, 
	wmGD.xa_WM_STATE, 32, PropModeAppend, 
	(unsigned char *)&property, 0);

    /*
     * Pick up the property notify event
     */
    XSync (DISPLAY, False);
    if (XCheckWindowEvent (DISPLAY, pSD->wmWorkspaceWin, 
			    PropertyChangeMask, &event))
    {
	if (event.type == PropertyNotify)
	{
	    timestamp = event.xproperty.time;
	}
	else
	{
	    /* not sure what happened here ... use CurrentTime */
	    timestamp = CurrentTime; 
	}
	if ((event.type != PropertyNotify) ||
	    (event.xproperty.atom != wmGD.xa_WM_STATE))
	{
	    /* 
	     * This wasn't the event we caused, put it back for
	     * later processing. We'll keep the timestamp, though.
	     */
	    XPutBackEvent (DISPLAY, &event);
	}
    }
    else
    {
	/* Hmm... didn't get the prop notify, fall back to current time */
	timestamp = CurrentTime; 
    }

    return (timestamp);

} /* END OF FUNCTION GetTimestamp */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  LastTime ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to provide a timestamp for use with X calls that
 *  require a timestamp. It returns the last timestamp processed if one
 *  exists or it generates a new one.
 *
 *
 *  Inputs:
 *  ------
 *  none
 * 
 *  Outputs:
 *  -------
 *  Return = a timestamp value - NOT CurrentTime
 *
 *************************************<->***********************************/

Time LastTime ()
{
  Time evTime;

  if (! (evTime = XtLastTimestampProcessed(DISPLAY)) )
    evTime = GetTimestamp();

  return (evTime);
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  PullExposureEvents ()
 *
 *
 *  Description:
 *  -----------
 *  Pull in and process all outstanding exposure events 
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Useful for cleaning up display after menu popdown
 * 
 *************************************<->***********************************/
void PullExposureEvents (void)
{
    XEvent	event;
    Boolean	dispatchEvent;

    /* 
     * Force the exposure events into the queue
     */
    XSync (DISPLAY, False);
#ifdef WSM
    XSync (DISPLAY1, False);
#endif /* WSM */
    /*
     * Selectively extract the exposure events
     */
#ifdef WSM
    while (XCheckMaskEvent (DISPLAY, 
	       ExposureMask|VisibilityChangeMask, &event) ||
	   XCheckMaskEvent (DISPLAY1, 
	       ExposureMask|VisibilityChangeMask, &event))
#else /* WSM */
    while (XCheckMaskEvent (DISPLAY, ExposureMask, &event))
#endif /* WSM */
    {
        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 */

#ifdef WSM
      switch (event.type)
      {
       case Expose:
#endif /* WSM */
	if (event.xany.window == ACTIVE_ROOT)
	{
	    dispatchEvent = WmDispatchWsEvent (&event);
	}
	else
	{
	    dispatchEvent = WmDispatchClientEvent (&event);
	}
#ifdef WSM
       default:
	dispatchEvent = True;
      }
#endif /* WSM */

	if (dispatchEvent)
	{
	    /*
	     * Dispatch widget related event:
	     */

	    XtDispatchEvent (&event);
	}
    }

} /* END OF FUNCTION PullExposureEvents */

#ifdef WSM

/*************************************<->*************************************
 *
 *  ReplayedButtonEvent ()
 *
 *
 *  Description:
 *  -----------
 *  Compare to button events to see if it's one event that's been
 *  replayed.
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  return	= True if event is replayed.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Boolean
ReplayedButtonEvent (
    XButtonEvent *pevB1,
    XButtonEvent *pevB2)
{
    Boolean rval = False;

    if ( (pevB1->type  		== pevB2->type) &&
	 (pevB1->send_event 	== pevB2->send_event) &&
	 (pevB1->display 	== pevB2->display) &&
	 (pevB1->window 	== pevB2->window) &&
	 (pevB1->root 		== pevB2->root) &&
	 (pevB1->subwindow 	== pevB2->subwindow) &&
	 (pevB1->time 		== pevB2->time) &&
	 (pevB1->x 		== pevB2->x) &&
	 (pevB1->y 		== pevB2->y) &&
	 (pevB1->x_root 	== pevB2->x_root) &&
	 (pevB1->y_root 	== pevB2->y_root) &&
	 (pevB1->state 		== pevB2->state) &&
	 (pevB1->button		== pevB2->button) &&
	 (pevB1->same_screen	== pevB2->same_screen) 
       )
    {
	rval = True;
    }

    return (rval);
}
#endif /* WSM */
