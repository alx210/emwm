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
 * Motif Release 1.2.1
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: WmMain.c /main/8 1998/04/20 13:01:09 mgreess $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#include <X11/Xlocale.h>
#ifdef WSM
#include <Dt/Message.h>
#include <Dt/EnvControlP.h>
#endif /* WSM */
/*
 * include extern functions
 */

#include "WmCEvent.h"
#include "WmEvent.h"
#include "WmInitWs.h"
#ifdef WSM
#include "WmIPC.h"
#include "WmBackdrop.h"
#endif /* WSM */


/*
 * Function Declarations:
 */
#ifdef WSM
int WmReturnIdentity (int argc, char *argv[], char *environ[]);
#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : \
(IsBackdropWindow (ACTIVE_PSD, (w))))
#else /* WSM */
#define ManagedRoot(w) (!XFindContext (DISPLAY, (w), wmGD.screenContextType, \
(caddr_t *)&pSD) ? (SetActiveScreen (pSD), True) : False)
#endif /* WSM */

WmScreenData *pSD;

/*
 * Global Variables:
 */

WmGlobalData wmGD;
#ifndef NO_MESSAGE_CATALOG
NlsStrings wmNLS;
#endif
#ifdef WSM
int WmIdentity;
#endif /* WSM */



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

int
main (int argc, char *argv [], char *environ [])
{
    XEvent	event;
    Boolean	dispatchEvent;

    setlocale(LC_ALL, "");

#ifndef NO_MULTIBYTE
#ifdef WSM
    /*
     * Set up environment variables for this HP DT client
     */
    _DtEnvControl(DT_ENV_SET);

    /*
     * Force LANG lookup early. 
     * (Front end may change $LANG to 'C' as part
     *  of string space reduction optimization.)
     */
     {
	 char * foo = ((char *)GETMESSAGE(44, 1, ""));
     }
#endif /* WSM */
    XtSetLanguageProc (NULL, (XtLanguageProc)NULL, NULL);
#endif
#ifdef WSM
    /*  
     * Get Identity
     */
    WmIdentity = WmReturnIdentity(argc, argv, environ);
#endif /* WSM */

    /*
     * Initialize the workspace:
     */

    InitWmGlobal (argc, argv, environ);
#ifdef WSM

    /*
     * Set up PATH variable if it must run as standalone command
     * invoker
     */
    if (wmGD.dtLite)
    {
	_DtEnvControl(DT_ENV_SET_BIN);
    }
#endif /* WSM */
    
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

#ifdef WSM
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
#endif /* WSM */
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

#ifdef WSM
/******************************<->*************************************
 *
 *  WmReturnIdentity (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This function checks the last component of the (path)name
 *  contained in argv[0] and makes a global decision as to whether
 *  it should fetch resources as mwm or dtwm.
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 ******************************<->***********************************/

int WmReturnIdentity ( int argc, char *argv[], char *environ[]) 
{
	char *tempString;
	char *origPtr;

	/* assume it's dtwm until proven differently */

 	int retVal = DT_MWM;

	if (!(tempString = 
	      (char *)(XtMalloc ((unsigned int)(strlen (argv[0]) + 1)))))
	{
		Warning(((char *)GETMESSAGE(44, 2, "Insufficient memory for name of window manager")));
		exit(WM_ERROR_EXIT_VALUE);
	}

	origPtr = tempString;

	if (strrchr(argv[0], '/'))
	{
		
		strcpy(tempString, (strrchr(argv[0], '/')));

		tempString++;
	}
	else
		strcpy(tempString, argv[0]);

	if (!(strcmp(tempString, WM_RESOURCE_NAME)))
	/*
	 *
	 *   If it's explicity "mwm", then set our identity anew.
	 *
	 */
	{
		retVal = MWM;
	}

	XtFree((char *)origPtr);

	return(retVal);

} /* END OF FUNCTION WmReturnIdentity */
#endif /* WSM */
#ifdef WSM
/*************************     eof   ******************************/
#endif /* WSM */
