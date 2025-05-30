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

#include <X11/Xlocale.h>

/*
 * include extern functions
 */

#include "WmCEvent.h"
#include "WmEvent.h"
#include "WmInitWs.h"
#include "WmBackdrop.h"


/*
 * Function Declarations:
 */
#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : \
(IsBackdropWindow (ACTIVE_PSD, (w))))

WmScreenData *pSD;

/*
 * Global Variables:
 */

WmGlobalData wmGD;
#ifndef NO_MESSAGE_CATALOG
NlsStrings wmNLS;
#endif



/*************************************<->*************************************
 *
 *  main (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This is the main window manager function.  It calls window manager
 *  initializtion functions and has the main event processing loop.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 *************************************<->***********************************/

int main (int argc, char *argv [], char *environ [])
{
    XEvent	event;
    Boolean	dispatchEvent;

    setlocale(LC_ALL, "");

    XtSetLanguageProc (NULL, (XtLanguageProc)NULL, NULL);

    /*
     * Initialize the workspace:
     */

    InitWmGlobal (argc, argv, environ);
    
    /*
     * MAIN EVENT HANDLING LOOP:
     */

    for (;;)
    {
        XtAppNextEvent (wmGD.mwmAppContext, &event);


        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 * The lock modifier is "filtered" out for window manager processing.
	 */

	wmGD.attributesWindow = 0L;


	if ((event.type == ButtonPress) || 
	    (event.type == ButtonRelease))
	{
	    if ((wmGD.evLastButton.button != 0) &&
		ReplayedButtonEvent (&(wmGD.evLastButton), 
				     &(event.xbutton)))
	    {
		wmGD.bReplayedButton = True;
	    }
	    else
	    {
		/* save this button for next comparison */
		memcpy (&wmGD.evLastButton, &event, sizeof (XButtonEvent));
		wmGD.bReplayedButton = False;
	    }
	}

	dispatchEvent = True;
	if (wmGD.menuActive)
	{
	    /*
	     * Do special menu event preprocessing.
	     */

	    if (wmGD.checkHotspot || wmGD.menuUnpostKeySpec ||
		wmGD.menuActive->accelKeySpecs)
	    {
	        dispatchEvent = WmDispatchMenuEvent ((XButtonEvent *) &event);
	    }
	}

	if (dispatchEvent)
	{
	    if (ManagedRoot(event.xany.window))
	    {
	        dispatchEvent = WmDispatchWsEvent (&event);
	    }
	    else
	    {
	        dispatchEvent = WmDispatchClientEvent (&event);
	    }

	    if (dispatchEvent)
	    {
                /*
                 * Dispatch widget related event:
                 */

                XtDispatchEvent (&event);
	    }
	}
    }

} /* END OF FUNCTION main */


void PrintVersionInfo(void)
{
	fprintf(stdout, "%s - Version %d.%d (Motif %d.%d.%d)\n\n",
		MWM_NAME, MWM_VERSION, MWM_REVISION,
		XmVERSION, XmREVISION, XmUPDATE_LEVEL);
}
