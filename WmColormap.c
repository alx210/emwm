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
static char rcsid[] = "$XConsortium: WmColormap.c /main/5 1996/10/30 11:14:44 drk $"
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

#include "WmColormap.h"
#include "WmKeyFocus.h"

static Bool ProcessEvents(Display *dpy, XEvent *Event, char *c_pCD);


/* Global variables */
	static unsigned long firstRequest, lastRequest;


/*************************************<->*************************************
 *
 *  InitWorkspaceColormap ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the default workspace colormap and prepares for
 *  workspace colormap processing.
 *
 *
 *  Inputs:
 *  -------
 *  pSD = ptr to screen data
 * 
 *  Outputs:
 *  -------
 *  wmGD = (workspaceColormap)
 * 
 *************************************<->***********************************/

void InitWorkspaceColormap (WmScreenData *pSD)
{
    /*
     * Setup the default (workspace) colormap:
     * !!! this should be made more general to get the colormap for the !!!
     * !!! workspace (root) and then track colormap changes             !!!
     */

    pSD->workspaceColormap = DefaultColormap (DISPLAY, pSD->screen);

} /* END OF FUNCTION InitWorkspaceColormap */



/*************************************<->*************************************
 *
 *  InitColormapFocus (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function prepares for managing the colormap focus and sets the
 *  initial colormap focus (if the focus policy is "keyboard" - i.e. the
 *  colormap focus tracks the keyboard focus) the initial colormap
 *  installation is done in InitKeyboardFocus.
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to screen data
 *
 *  Outputs:
 *  -------
 *  *pSD = (colormapFocus)
 * 
 *************************************<->***********************************/

void InitColormapFocus (WmScreenData *pSD)
{
    ClientData *pCD;
    Boolean sameScreen;


    /*
     * Set up the initial colormap focus.  If the colormapFocusPolicy is
     * "keyboard" or it is "pointer" and the keyboard input focus policy
     * is "pointer" then set up the initial colormap focus when the
     * initial keyboard input focus is set up.
     */

    pSD->colormapFocus = NULL;

    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
    {
	if (wmGD.keyboardFocusPolicy != KEYBOARD_FOCUS_POINTER)
	{
	    if ((pCD = GetClientUnderPointer (&sameScreen)) != NULL)
	    {
	        SetColormapFocus (pSD, pCD);
	    }
	    else
	    {
	        WmInstallColormap (pSD, pSD->workspaceColormap);
	    }
	}
    }
    else
    {
	WmInstallColormap (pSD, pSD->workspaceColormap);
    }

} /* END OF FUNCTION InitColormapFocus */



#ifndef OLD_COLORMAP
/*************************************<->*************************************
 *
 *  ForceColormapFocus (pSD, pCD)
 *
 *
 *  Description:
 *  -----------
 * ForceColormapFocus is the working part of the original SetColormapFocus.
 * This function is used to unconditionally set the colormap focus to a
 * particular client window or to clear the colormap focus (set focus to
 * the root window).
 *
 * The reason is to permit focus to be dtrced.  We need to do this because
 * we can already have colormap focus, but still need to set the colormaps.
 * Examples of when this occurs are:
 *
 *	* after the window manager itself has forced a colormap,
 *	  as happens when it draws transients in the overlay planes.
 *	* when WM_COLORMAP_WINDOWS changes.
 *	* when a ColormapNotify (new) event is received.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to Screen Data
 *  pCD = pointer to client data (clientColormap ...)
 *
 *************************************<->***********************************/

void ForceColormapFocus (WmScreenData *pSD, ClientData *pCD)
{
    if (pCD && ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE)))
    {
	pSD->colormapFocus = pCD;
#ifndef OLD_COLORMAP /* colormaps */
	ProcessColormapList (pSD, pCD);
#else /* OSF original */
	WmInstallColormap (pSD, pCD->clientColormap);
#endif
    }
    else
    {
	/*
	 * The default colormap is installed for minimized windows that have
	 * the colormap focus.
	 * !!! should colormaps be installed for icons with client      !!!
	 * !!! icon windows?  should the client colormap be installed ? !!!
	 */

	pSD->colormapFocus = NULL;
	WmInstallColormap (pSD, pSD->workspaceColormap);
    }

} /* END OF FUNCTION ForceColormapFocus */
#endif



/*************************************<->*************************************
 *
 *  SetColormapFocus (pSD, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to set the colormap focus to a particular client
 *  window or to clear the colormap focus (set focus to the root window).
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to Screen Data
 *  pCD = pointer to client data (clientColormap ...)
 *
 *************************************<->***********************************/

void SetColormapFocus (WmScreenData *pSD, ClientData *pCD)
{
    if (pCD == pSD->colormapFocus)
    {
	/*
	 * The focus is already set to the right place.
	 */

	return;
    }
#ifndef OLD_COLORMAP
    ForceColormapFocus (pSD, pCD);
#else /* OSF original */

    if (pCD && ((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE)))
    {
	pSD->colormapFocus = pCD;
#ifndef OLD_COLORMAP /* colormaps */
	ProcessColormapList (pSD, pCD);
#else /* OSF original */
	WmInstallColormap (pSD, pCD->clientColormap);
#endif
    }
    else
    {
	/*
	 * The default colormap is installed for minimized windows that have
	 * the colormap focus.
	 * !!! should colormaps be installed for icons with client      !!!
	 * !!! icon windows?  should the client colormap be installed ? !!!
	 */

	pSD->colormapFocus = NULL;
	WmInstallColormap (pSD, pSD->workspaceColormap);
    }
#endif

} /* END OF FUNCTION SetColormapFocus */



/*************************************<->*************************************
 *
 *  WmInstallColormap (pSD, colormap)
 *
 *
 *  Description:
 *  -----------
 *  This function installs colormaps for the window manager.  It trys to be
 *  intelligent and avoid unnecessary installations.  It assumes that no
 *  other program is installing colormaps.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = ptr to screen data
 *  colormap = the id for the colormap to be installed
 *
 *************************************<->***********************************/

void WmInstallColormap (WmScreenData *pSD, Colormap colormap)
{
    /*
     * !!! this could be generalized to work better for systems that !!!
     * !!! support multiple installed colormaps                      !!!
     */

    if (colormap != pSD->lastInstalledColormap)
    {
	XInstallColormap (DISPLAY, colormap);
	pSD->lastInstalledColormap = colormap;
    }

} /* END OF FUNCTION WmInstallColormap */



/*************************************<->*************************************
 *
 *  ResetColormapData (pCD, pWindows, count)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to release old colormap data (contexts, malloc'ed
 *  space).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (cmapWindows ...)
 *
 *  pWindows = new list of colormap windows
 *
 *  count = number of windows in new colormap windows list
 *
 *************************************<->***********************************/

void ResetColormapData (ClientData *pCD, Window *pWindows, int count)
{
    int i;


    if (pCD->clientCmapCount)
    {
	if (count == 0)
	{
	    /* reset the client colormap to the toplevel window colormap */
	    for (i = 0; i < pCD->clientCmapCount; i++)
	    {
		if (pCD->cmapWindows[i] == pCD->client)
		{
		    pCD->clientColormap = pCD->clientCmapList[i];
		    break;
		}
	    }
	}

	/*
	 * Free up old contexts.
	 */

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

	/*
	 * Free up old colormap data.
	 */

	XtFree ((char *)(pCD->cmapWindows));
	XtFree ((char *)(pCD->clientCmapList));
	pCD->clientCmapCount = 0;
#ifndef OLD_COLORMAP /* colormap */
	XtFree ((char  *)(pCD->clientCmapFlags));
	pCD->clientCmapFlags = 0;		/* DEBUG: */
	pCD->clientCmapFlagsInitialized = 0;
#endif
    }

    if (count)
    {
	/*
	 * Set new contexts.
	 */

	for (i = 0; i < count; i++)
	{
	    if (pWindows[i] != pCD->client)
	    {
#ifndef	IBM_169380
		AddColormapWindowReference(pCD, pWindows[i]);
#else
		XSaveContext (DISPLAY, pWindows[i], wmGD.windowContextType,
		    (caddr_t)pCD);
#endif
	    }
	}
    }

} /* END OF FUNCTION ResetColormapData */

#ifndef IBM_169380
/*************************************<->*************************************
 *
 *  AddColormapWindowReference (pCD, window)
 *
 *  Description:
 *  -----------
 *  This function is used to update (or create, if necessary) the structure
 *  that keeps track of all references to a Window from a toplevel window
 *  WM_COLORMAP_DATA property.
 *
 *************************************<->***********************************/

void AddColormapWindowReference (ClientData *pCD, Window window)
{
    ClientData          **cmap_window_data;
    Boolean             context_exists;
    int                 i;
    ClientData          **new_cmap_window_data;

    context_exists = (!XFindContext (DISPLAY, window,
                        wmGD.cmapWindowContextType,
                        (XPointer *) &cmap_window_data));
    if (context_exists)
    {
        for (i = 0; cmap_window_data[i] != NULL; i++)
        {
            if (cmap_window_data[i] == pCD)
            {
                /* Reference already exists - return */
                return;
            }
        }
        new_cmap_window_data = (ClientData **)
                                XtMalloc((i + 2 ) * sizeof(ClientData *));
        memcpy((void *)new_cmap_window_data,(void *)cmap_window_data,
                        (i + 1) * sizeof(ClientData *));
        XtFree((char *) cmap_window_data);
        XDeleteContext(DISPLAY, window, wmGD.cmapWindowContextType);
    }
    else
    {
        i = 0;
        new_cmap_window_data = (ClientData **)
                                XtMalloc(2 * sizeof(ClientData *));
    }
    new_cmap_window_data[i] = pCD;
    new_cmap_window_data[i + 1] = NULL;
    XSaveContext (DISPLAY, window, wmGD.cmapWindowContextType,
                        (caddr_t)new_cmap_window_data);
}

/*************************************<->*************************************
 *
 *  RemoveColormapWindowReference (pCD, window)
 *
 *  Description:
 *  -----------
 *  This function is used to update (or delete, if necessary) the structure
 *  that keeps track of all references to a Window from a toplevel window
 *  WM_COLORMAP_DATA property.
 *
 *************************************<->***********************************/

void RemoveColormapWindowReference (ClientData *pCD, Window window)
{
    ClientData  **cmap_window_data;
    Boolean     context_exists;
    int         i;
    int         reference_idx = -1;
    ClientData  **new_cmap_window_data;

    context_exists = (!XFindContext (DISPLAY, window,
                        wmGD.cmapWindowContextType,
                        (XPointer *) &cmap_window_data));
    if (context_exists)
    {
        for (i = 0; cmap_window_data[i] != NULL; i++)
        {
            if (cmap_window_data[i] == pCD)
                reference_idx = i;
        }
        if (reference_idx < 0)
            return;

        if (i > 1)
        {
        int     j,idx;

            new_cmap_window_data = (ClientData **)
                                        XtMalloc(i * sizeof(ClientData *));
            idx = 0;
            for (j = 0; cmap_window_data[j] != NULL; j++)
            {
                if (j != reference_idx)
                {
                    new_cmap_window_data[idx] = cmap_window_data[j];
                    idx++;
                }
            }
            new_cmap_window_data[idx] = NULL;
        }
        XtFree((char *) cmap_window_data);
        XDeleteContext(DISPLAY, window, wmGD.cmapWindowContextType);
        if (i > 1)
        {
            XSaveContext (DISPLAY, window,
                        wmGD.cmapWindowContextType,
                        (caddr_t)new_cmap_window_data);
        }
    }
}
#endif	/* IBM_169380 */

/*******************************************************************************
 **
 ** The rest of this module contains the SGI-added colormap handling.
 **
 ** mwm 1.1.3 didn't even try to deal with multiple colormaps, except to rotate
 ** them.  We need to see that all of the colormaps from WM_COLORMAP_WINDOWS
 ** are installed when a window gets colormap focus.
 **
 ** The general idea is to keep track of which colormaps bounce which other
 ** ones, so we only flash the first time (usually not even then).
 **
 ** The conflict record of a window is cleared whenever:
 **	* WM_COLORMAP_WINDOWS property changes
 **	* ColormapNotify for a new colormap happens
 **	* windows are rotated (prev_cmap, next_cmap)
 ** This is because with a changed colormap list, we need to recalculate
 ** which ones get bounced out during a full colormap installation.
 **
 ** We don't just lift the twm code because, after carefully looking over
 ** the twm code, it appears to have some problems of its own.  In
 ** particular, it assumes that if a given colormap displaces another one
 ** once, it will always do so.  This isn't necessarily so for a multiple
 ** hardware colormaps machine.
 **
 ** We still need to add code to keep track of which color maps are really
 ** installed at any one time.  The current code is ready for this, but it has
 ** not yet been done.  Then we could do two things:
 **
 **	* refrain from installing a colormap if it is already installed
 **
 **	* have a way to restore all installed colormaps after the window
 **	  manager overwrites with it's own.
 **
 ******************************************************************************/


void
ProcessColormapList (WmScreenData *pSD, ClientData *pCD)

{
	register int i;
	XEvent event;


    /*
     * If there is no client, return.  This can happen when the root gets focus.
     */
	if (!pCD) return;

    /*
     * If the window does not have colormap focus, return.  We only install
     * colormaps for windows with focus.  We'll get another chance when the
     * window does get focus.
     */
	if (pCD != pSD->colormapFocus) return;

    /*
     * If window is iconified, return.
     */
	if (   (pCD->clientState != NORMAL_STATE) 
	    && (pCD->clientState != MAXIMIZED_STATE)
	   ) return;

    /*
     * If the list doesn't exist, or has just a single item, no conflicts
     * exist -- just go ahead and install the indicated colormap.
     */
	if (pCD->clientCmapCount == 0) {
		WmInstallColormap (pSD, pCD->clientColormap);
		return;
	}
	if (pCD->clientCmapCount == 1) {
		WmInstallColormap (pSD, pCD->clientCmapList[0]);
		return;
	}

    /*
     * If the list has already been initialized, we just need to do installs.
     * Separate out these loops for performance, and because it isn't nice
     * to grab the server unnecessarily.
     *
     * This code should also check for already-installed, once we put in that
     * capability.
     */
	if (pCD->clientCmapFlagsInitialized) {

	    /* Do the part between the index and zero */
		for (i=pCD->clientCmapIndex; --i>=0; ) {
			if (pCD->clientCmapFlags[i] == ColormapInstalled) {
				WmInstallColormap (pSD, pCD->clientCmapList[i]);
			   }
		};
	
	    /* Do the part from the end of the list to the index */
		for (i=pCD->clientCmapCount; --i>= pCD->clientCmapIndex; ) {
			if (pCD->clientCmapFlags[i] == ColormapInstalled) {
				WmInstallColormap (pSD, pCD->clientCmapList[i]);
			}
		}

	    /**/
		return;
	}

    /*
     * If we get this far, the list has not yet been initialized.
     *
     * Stabilize the input queue -- the issue is that we need to know
     * which colormap notify install and uninstall events are ours.
     */
	XGrabServer (DISPLAY);	/* Ensure no one else's events for awhile */
	XSync (DISPLAY, FALSE);	/* Let pending events settle */
	firstRequest = NextRequest (DISPLAY); /* First one that can be ours */

    /*
     * Install the colormaps from last to first -- first is the "highest
     * priority".  "First" is pCD->clientCmapIndex.
     *
     * If the list has not been proocessed before, we need to unconditionally
     * install each colormap.  Colormap flashing is possible this once.
     *
     * If the list has already been processed once, all conflict checking
     * was done then.  All we need to do this time is to install the colormaps
     * we know we need.
     */

	/* Do the part between the index and zero */
	for (i=pCD->clientCmapIndex; --i>=0; ) {
		WmInstallColormap (pSD, pCD->clientCmapList[i]);
		pCD->clientCmapFlags[i] = ColormapInstalled;
	};

	/* Do the part from the end of the list to the index */
	for (i=pCD->clientCmapCount; --i>= pCD->clientCmapIndex; ) {
		WmInstallColormap (pSD, pCD->clientCmapList[i]);
		pCD->clientCmapFlags[i] = ColormapInstalled;
	}

    /*
     * Stabilize the input queue again -- the issue is that we need to know
     * which colormap notify install and uninstall events we caused.
     */
	XSync (DISPLAY, FALSE);			/* Let pending events settle */
	lastRequest = NextRequest (DISPLAY);	/* Last one that can be ours */
	XUngrabServer (DISPLAY);		/* Let others use it again */

    /* Process the install & uninstall events */
	XCheckIfEvent (DISPLAY, (XEvent *) &event, ProcessEvents, (char *)pCD);

    /* Set that the list has been processed once */
	pCD->clientCmapFlagsInitialized = True;
}


/*
 * Look over the queue for install and uninstall events on colormap/window
 * combinations we care about.  We don't actually disturb the queue, so
 * events can be delivered in undisturbed order to the normal event handling
 * routines.
 *
 * For each appropriate install/uninstall ColormapNotify event that is queued:
 *	   *) if uninstall event
 *		*) Set the conflict flag for this colormap window
 *	   else if install event
 *		*) Clear the conflict flag for this colormap window
 */
static Bool
ProcessEvents(Display *dpy, XEvent *Event, char *c_pCD)
{
	int i;
	XColormapEvent *pEvent = (XColormapEvent *) Event;
	ClientData *pCD = (ClientData *) c_pCD;

	if (   (pEvent->type == ColormapNotify)
	    && (pEvent->serial >= firstRequest)
	    && (pEvent->serial <  lastRequest)
	    && (pEvent->colormap != None)
	    && (!pEvent->new)
	   ) {
		switch (pEvent->state) {
		case ColormapInstalled:
			for (i=0; i<pCD->clientCmapCount; i++) {
				if (  (pCD->clientCmapList[i]==pEvent->colormap)
				    &&(pCD->cmapWindows[i]==pEvent->window)
				   ) {
					pCD->clientCmapFlags[i]
						= ColormapInstalled;
					break;
				}
			}
			break;
		case ColormapUninstalled:
			for (i=0; i<pCD->clientCmapCount; i++) {
				if (  (pCD->clientCmapList[i]==pEvent->colormap)
				    &&(pCD->cmapWindows[i]==pEvent->window)
				   ) {
					pCD->clientCmapFlags[i]
						= ColormapUninstalled;
					break;
				}
			}
			break;
		default:		/* Should never get here */
			break;
		}
	}

    /*
     * Always return false:
     *	* so that we get to search the entire queue -- it isn't very long
     *	* so all events remain on the queue to be handled normally elsewhere
     */
	return False;	/* Always, so no events are lost from the queue */
}
