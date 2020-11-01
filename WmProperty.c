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
static char rcsid[] = "$TOG: WmProperty.c /main/7 1997/12/02 10:00:00 bill $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"
#include <stdio.h>
#ifdef WSM
#include <Dt/WsmP.h>
#include <X11/Xatom.h>
#endif /* WSM */

/*
 * include extern functions
 */

#include "WmColormap.h"
#include "WmError.h"
#include "WmResParse.h"



/*
 * Function Declarations:
 */

#include "WmProperty.h"


/*
 * Global Variables:
 */

static SizeHints sizeHints;



/*************************************<->*************************************
 *
 *  SizeHints *
 *  GetNormalHints (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function replaces the XGetNormalHints Xlib function.  This function
 *  gets the information in the WM_NORMAL_HINTS property on the client window.
 *  The property encoding can be any of the supported versions (R2, R3+).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = (client)
 *
 * 
 *  Outputs:
 *  -------
 *  Return = A pointer to a filled out SizeHints structure is returned.
 *           Default values are set if the WM_NORMAL_HINTS property could
 *           not be retrieved.
 *
 *************************************<->***********************************/

SizeHints * 
GetNormalHints(
        ClientData *pCD )

{
    PropSizeHints *property = NULL;
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    unsigned long nitems;


    /*
     * Retrieve the property data.
     *
     *     ICCC_R2 version:  nitems = PROP_SIZE_HINTS_ELEMENTS - 3
     *     ICCC_CURRENT version: nitems = PROP_SIZE_HINTS_ELEMENTS
     */

#ifdef WSM
    if ((!HasProperty (pCD, XA_WM_NORMAL_HINTS)) ||
	((Success != XGetWindowProperty (DISPLAY, pCD->client,
			XA_WM_NORMAL_HINTS, 0L, (long)PROP_SIZE_HINTS_ELEMENTS,
			False, XA_WM_SIZE_HINTS, &actualType, &actualFormat,
			&nitems, &leftover, (unsigned char **)&property)) ||
	  (actualType != XA_WM_SIZE_HINTS) ||
	  (nitems < (PROP_SIZE_HINTS_ELEMENTS - 3)) ||
	  (actualFormat != 32)))
#else /* WSM */
    if ((Success != XGetWindowProperty (DISPLAY, pCD->client,
			XA_WM_NORMAL_HINTS, 0L, (long)PROP_SIZE_HINTS_ELEMENTS,
			False, XA_WM_SIZE_HINTS, &actualType, &actualFormat,
			&nitems, &leftover, (unsigned char **)&property)) ||
	 (actualType != XA_WM_SIZE_HINTS) ||
	 (nitems < (PROP_SIZE_HINTS_ELEMENTS - 3)) ||
	 (actualFormat != 32))
#endif /* WSM */
    {
	/*
	 * Indicate no property values were retrieved:
	 */

	sizeHints.icccVersion = ICCC_UNKNOWN;
	sizeHints.flags = 0;
    }
    else
    {
	/*
	 * Parse the hint values out of the property data:
	 */

	sizeHints.flags = property->flags;
	sizeHints.x = property->x;
	sizeHints.y = property->y;
	sizeHints.width = property->width;
	sizeHints.height = property->height;
	sizeHints.min_width = property->minWidth;
	sizeHints.min_height = property->minHeight;
	sizeHints.max_width = property->maxWidth;
	sizeHints.max_height = property->maxHeight;
	sizeHints.width_inc = property->widthInc;
	sizeHints.height_inc = property->heightInc;
	sizeHints.min_aspect.x = (int)property->minAspectX;
	sizeHints.min_aspect.y = (int)property->minAspectY;
	sizeHints.max_aspect.x = (int)property->maxAspectX;
	sizeHints.max_aspect.y = (int)property->maxAspectY;


	if (nitems == (PROP_SIZE_HINTS_ELEMENTS - 3))
	{
	    /*
	     *  This is ICCC_R2.
	     */

	    sizeHints.icccVersion = ICCC_R2;
	}
	else
	{
	    /*
	     *  This is ICCC_CURRENT.
	     */

	    sizeHints.icccVersion = ICCC_CURRENT;
	    sizeHints.base_width = property->baseWidth;
	    sizeHints.base_height = property->baseHeight;
	    sizeHints.win_gravity = property->winGravity;
	}
    }


    /*
     * Free the property data buffer:
     */

    if (property)
    {
	XFree ((char *)property);
    }


    /*
     * Return the hints values:
     */

    return (&sizeHints);


} /* END OF FUNCTION GetNormalHints */



/*************************************<->*************************************
 *
 *  ProcessWmProtocols (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function reads and processes the WM_PROTOCOLS property that is
 *  associated with a client window.
 *
 *  ICCC_COMPLIANT check added to allow older clients to work, for now...
 *  eventually, this code should be removed.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = (clientProtocols, clientProtocolCount, protocolFlags)
 *
 *************************************<->***********************************/

void ProcessWmProtocols (ClientData *pCD)
{
    int rValue;
    Atom *property = NULL;
#ifndef ICCC_COMPLIANT
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    unsigned long nitems;
#else
    int nitems;
#endif /* ICCC_COMPLIANT */
    int i;


    if (pCD->clientProtocols)
    {
	XtFree ((char *)pCD->clientProtocols);
	pCD->clientProtocols = NULL;
    }
    pCD->clientProtocolCount = 0;
    pCD->protocolFlags = 0;


    /*
     * Read the WM_PROTOCOLS property.
     */

#ifndef ICCC_COMPLIANT
#ifdef WSM
    if (!HasProperty (pCD, wmGD.xa_WM_PROTOCOLS))
	rValue = -1;
    else
#endif /* WSM */
    rValue = XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa_WM_PROTOCOLS, 0L,
		 (long)MAX_CLIENT_PROTOCOL_COUNT, False, AnyPropertyType,
		 &actualType, &actualFormat, &nitems, &leftover,
		 (unsigned char **)&property);


    if ((rValue != Success) || (actualType == None) || (actualFormat != 32))
#else
#ifdef WSM
    if (!HasProperty (pCD, wmGD.xa_WM_PROTOCOLS))
	rValue = -1;
    else
#endif /* WSM */
    rValue = XGetWMProtocols (DISPLAY, pCD->client, 
		 (Atom **)&property, &nitems);

    if (0 == rValue) 
#endif /* ICCC_COMPLIANT */
    {
	/*
	 * WM_PROTOCOLS does not exist or it is an invalid type or size.
	 */

	pCD->clientProtocols = NULL;
    }
    else
    {
        if (!(pCD->clientProtocols = (Atom *)XtMalloc (nitems * sizeof (Atom))))
        {
	    /* unable to allocate space */
	    Warning (((char *)GETMESSAGE(54, 1, "Insufficient memory for window management data")));
        }
	else
	{
	    /*
	     * Save the protocols in the client data and look for predefined
	     * protocols.
	     */

    	    pCD->clientProtocolCount = nitems;

    	    for (i = 0; i < nitems; i++)
    	    {
		pCD->clientProtocols[i] = property[i];
		if (property[i] == wmGD.xa_WM_SAVE_YOURSELF)
		{
		    pCD->protocolFlags |= PROTOCOL_WM_SAVE_YOURSELF;
		}
		else if (property[i] == wmGD.xa_WM_TAKE_FOCUS)
		{
		    pCD->protocolFlags |= PROTOCOL_WM_TAKE_FOCUS;
		}
		else if (property[i] == wmGD.xa_WM_DELETE_WINDOW)
		{
		    pCD->protocolFlags |= PROTOCOL_WM_DELETE_WINDOW;
		}
		else if (property[i] == wmGD.xa_MWM_MESSAGES)
		{
		    pCD->protocolFlags |= PROTOCOL_MWM_MESSAGES;
		}
    	    }
	}
    }


    if (property)
    {
	XFree ((char *)property);
    }


} /* END OF FUNCTION ProcessWmProtocols */



/*************************************<->*************************************
 *
 *  ProcessMwmMessages (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function reads and processes the _MWM_MESSAGES property that is
 *  associated with a client window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = (mwmMessagesCount, mwmMessages)
 *
 *************************************<->***********************************/

void ProcessMwmMessages (ClientData *pCD)
{
    int rValue;
    long *property = NULL;
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    unsigned long nitems;
    int i;


    if (pCD->mwmMessages)
    {
	XtFree ((char *)pCD->mwmMessages);
	pCD->mwmMessages = NULL;
    }
    pCD->mwmMessagesCount = 0;


    /*
     * Read the _MWM_MESSAGES property.
     */

#ifdef WSM
    if (!HasProperty (pCD, wmGD.xa_MWM_MESSAGES))
        rValue = ~Success;
    else
#endif /* WSM */
    rValue = XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa_MWM_MESSAGES, 0L,
		 (long)MAX_MWM_MESSAGES_COUNT, False, AnyPropertyType,
		 &actualType, &actualFormat, &nitems, &leftover,
		 (unsigned char **)&property);


    if ((rValue != Success) || (actualType == None) || (actualFormat != 32)
	|| (nitems == 0))
    {
	/*
	 * _MWM_MESSAGES does not exist or it is an invalid type.
	 */

	pCD->mwmMessages = NULL;
    }
    else
    {
        if (!(pCD->mwmMessages = (long *)XtMalloc (nitems * sizeof (long))))
        {
	    /* unable to allocate space */
	    Warning (((char *)GETMESSAGE(54, 2, "Insufficient memory for window management data")));
        }
	else
	{
	    /*
	     * Save the protocols in the client data and look for predefined
	     * protocols.
	     */

    	    pCD->mwmMessagesCount = nitems;

    	    for (i = 0; i < nitems; i++)
    	    {
		if ((pCD->mwmMessages[i] = property[i]) == wmGD.xa_MWM_OFFSET)
		{
		    pCD->protocolFlags |= PROTOCOL_MWM_OFFSET;
		}
    	    }
	}
    }


    if (property)
    {
	XFree ((char *)property);
    }


} /* END OF FUNCTION ProcessMwmMessages */



/*************************************<->*************************************
 *
 *  SetMwmInfo (propWindow, flags, wmWindow)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _MOTIF_WM_INFO property on the specified (usually
 *  the root) window.
 *
 *
 *  Inputs:
 *  ------
 *  propWindow = window on which the _MOTIF_WM_INFO property is to be set
 *
 *  flags = motifWmInfo.flags value
 *
 *  wmWindow = motifWmInfo.wmWindow value
 *
 * 
 *  Outputs:
 *  -------
 *  _MWM_INFO = this property is set on the specified window
 *
 *************************************<->***********************************/

void SetMwmInfo (Window propWindow, long flags, Window wmWindow)
{
    PropMwmInfo property;


    property.flags = flags;
    property.wmWindow = wmWindow;

    XChangeProperty (DISPLAY, propWindow, wmGD.xa_MWM_INFO, wmGD.xa_MWM_INFO,
	32, PropModeReplace, (unsigned char *)&property,
	PROP_MWM_INFO_ELEMENTS);

} /* END OF FUNCTION SetMwmInfo */

#ifdef WSM

/*************************************<->*************************************
 *
 *  SetMwmSaveSessionInfo (wmWindow)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the WM_SAVE_YOURSELF property on the wm window
 *
 *
 *  Inputs:
 *  ------
 *  wmWindow = motifWmInfo.wmWindow
 *
 * 
 *  Outputs:
 *  -------
 *  WM_SAVE_YOURSELF = this property is set on the wm window
 *
 *************************************<->***********************************/

void SetMwmSaveSessionInfo (Window wmWindow)
{

    Atom  property;
    property = wmGD.xa_WM_SAVE_YOURSELF;
    
    XChangeProperty (DISPLAY, wmWindow, 
		     wmGD.xa_WM_PROTOCOLS, XA_ATOM,
		     32, PropModeReplace,
		     (unsigned char *) &property, 1);
    SetWMState(wmWindow, NORMAL_STATE, 0);
    
} /* END OF FUNCTION SetMwmSaveSessionInfo */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  GetWMState (window)
 *
 *
 *  Description:
 *  -----------
 *  This function gets the WM_STATE property on a client top-level
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  window = client window from which the WM_STATE property is to be retrieved
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = a pointer to the WM_STATE property value (NULL if not defined)
 *
 *
 *  Comments:
 *  --------
 *  This function will eventually be superceded when WM_STATE support is
 *  added to the official X release.
 * 
 *************************************<->***********************************/

PropWMState *
GetWMState(
        Window window )
{
    int ret_val;
    PropWMState *property = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;


    ret_val = XGetWindowProperty (DISPLAY, window, wmGD.xa_WM_STATE, 
		  0L, PROP_WM_STATE_ELEMENTS,
		  False, wmGD.xa_WM_STATE, 
		  &actual_type, &actual_format, 
		  &nitems, &leftover, (unsigned char **)&property);

    if (!((ret_val == Success) && (actual_type == wmGD.xa_WM_STATE) &&
         (nitems == PROP_WM_STATE_ELEMENTS)))
    {
        /*
         * The property could not be retrieved or is not correctly set up.
         */

        if (property)
        {
	    XFree ((char *)property);
	    property = NULL;
        }
    }

    return (property);

} /* END OF FUNCTION GetWMState */



/*************************************<->*************************************
 *
 *  SetWMState (window, state, icon)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the WM_STATE property on a client top-level
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  window = client window on which the WM_STATE property is to be set
 *
 *  state = state of the client application
 *
 *  icon = window manager's icon window
 *
 * 
 *  Outputs:
 *  -------
 *  WM_STATE = this property is set on the client window
 *
 *
 *  Comments:
 *  --------
 *  This function will eventually be superceded when WM_STATE support is
 *  added to the official X release.
 * 
 *************************************<->***********************************/

void SetWMState (Window window, int state, Window icon)
{
    PropWMState property;


    property.state = state;
    property.icon = icon;

    XChangeProperty (DISPLAY, window, wmGD.xa_WM_STATE, wmGD.xa_WM_STATE, 32,
	PropModeReplace, (unsigned char *)&property, PROP_WM_STATE_ELEMENTS);

} /* END OF FUNCTION SetWMState */



/*************************************<->*************************************
 *
 *  PropMwmHints *
 *  GetMwmHints (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function reads any _MWM_HINTS property that is associated with a 
 *  client window.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  RETURN = ptr to mwm hints property, or NULL ptr if failure
 *
 *************************************<->***********************************/

PropMwmHints *
GetMwmHints(
        ClientData *pCD )

{
    int ret_val;
    PropMwmHints *property = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;


#ifdef WSM
    if (!HasProperty(pCD, wmGD.xa_MWM_HINTS))
	ret_val = ~Success;
    else
#endif /* WSM */
    ret_val = XGetWindowProperty (DISPLAY, pCD->client, wmGD.xa_MWM_HINTS, 
		  0L, PROP_MWM_HINTS_ELEMENTS,
		  False, wmGD.xa_MWM_HINTS, 
		  &actual_type, &actual_format, 
		  &nitems, &leftover, (unsigned char **)&property);

    /*
     * Retrieve the property data.
     *
     *     Motif 1.1.n clients:	nitems	= PROP_MWM_HINTS_ELEMENTS
     *     Motif 1.2 clients:	nitems 	= PROP_MWM_HINTS_ELEMENTS + 2
     *
     * NOTES:  We don't need to check (nitems == PROP_MWM_HINTS_ELEMENTS)
     *         since...
     *
     * If running Motif 1.1.n client with Mwm 1.2, then ignore extra elements
     *    since property.flags won't have extra elements set.
     *
     * If running Motif 1.2 client with Mwm 1.1.n, then ignore extra elements
     *    since Mwm 1.1.n won't try to access the extra elements.
     */

    if ((ret_val == Success) && (actual_type == wmGD.xa_MWM_HINTS))
    {
	return (property);			/* indicate success */
    }


    /*
     * The property could not be retrieved or is not correctly set up.
     */

    if (property)
    {
	XFree ((char *)property);
    }

    return (NULL);			/* indicate failure */


} /* END OF FUNCTION GetMwmHints */



/*************************************<->*************************************
 *
 *  PropMwmInfo *
 *  GetMwmInfo (rootWindowOfScreen)
 *
 *
 *  Description:
 *  -----------
 *  This function reads the _MOTIF_WM_INFO property from the root window if
 *  it is setup.
 * 
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data 
 *
 *  Outputs:
 *  -------
 *  RETURN = ptr to motif wm info property, or NULL ptr if no property
 *
 *************************************<->***********************************/

PropMwmInfo *GetMwmInfo (Window rootWindowOfScreen)
{
    int ret_val;
    PropMwmInfo *property = NULL;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long leftover;


    ret_val = XGetWindowProperty (DISPLAY, rootWindowOfScreen,
                                     wmGD.xa_MWM_INFO,
                                     0L, PROP_MWM_INFO_ELEMENTS,
                                     False, wmGD.xa_MWM_INFO,
                                     &actual_type, &actual_format,
                                     &nitems, &leftover,
                                     (unsigned char **)&property);

    if ((ret_val == Success) && (actual_type == wmGD.xa_MWM_INFO) &&
        (nitems == PROP_MWM_INFO_ELEMENTS)) 
    {
	return (property);			/* indicate success */
    }


    /*
     * The property could not be retrieved or is not correctly set up.
     */

    if (property)
    {
	XFree ((char *)property);
    }

    return (NULL);			/* indicate failure */


} /* END OF FUNCTION GetMwmInfo */



/*************************************<->*************************************
 *
 *  ProcessWmColormapWindows (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves and processes the WM_COLORMAP_WINDOWS client
 *  window property.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = (cmapWindows, clientCmapList, clientCmapCount, clientCmapIndex)
 *
 *************************************<->***********************************/

void ProcessWmColormapWindows (ClientData *pCD)
{
    int rValue;
    Window *property = NULL;
    Atom actualType;
    int actualFormat;
    unsigned long leftover;
    unsigned long nitems;
    int i;
    Window *pWindows;
    Colormap *pColormaps;
    int colormapCount;
    XWindowAttributes wAttributes;
    ClientData *pcd;
    XSetWindowAttributes sAttributes;
#ifndef OLD_COLORMAP /* colormaps */
    int *pCmapFlags;
#endif


    /*
     * pCD->clientCmapCount and pCD->clientCmapIndex are initialized in
     * WmWinInfo.c.
     */

    /*
     * Read the WM_COLORMAP_WINDOWS property.
     */

    rValue = XGetWindowProperty (DISPLAY, pCD->client,
		 wmGD.xa_WM_COLORMAP_WINDOWS, 0L,
		 (long)MAX_COLORMAP_WINDOWS_COUNT, False, AnyPropertyType,
		 &actualType, &actualFormat, &nitems, &leftover,
		 (unsigned char **)&property);


    if ((rValue == Success) && (actualType != None) && (actualFormat == 32) &&
	(nitems > 0))
    {
	/*
	 * WM_COLORMAP_WINDOWS exists and is a valid type.
	 */

        if (!(pWindows = (Window *)XtMalloc ((nitems * sizeof (Window)) + 1)) ||
            !(pColormaps = (Colormap *)XtMalloc ((nitems*sizeof(Colormap)) + 1)))
        {
	    /* unable to allocate space */
	    Warning (((char *)GETMESSAGE(54, 3, "Insufficient memory for window management data")));
	    if (pWindows)
	    {
		XtFree ((char *)pWindows);
	    }
        }
#ifndef OLD_COLORMAP /* colormap */
	/* Is the above OSF code a bug -- allocates one extra byte, rather */
	/* than one extra element, for the top window if needed? */
	else if ( ! (pCmapFlags = (int *)XtCalloc(nitems+1,sizeof(int)))) {
			/* unable to allocate space */
			Warning (((char *)GETMESSAGE(54, 4, "Insufficient memory for window manager flags")));
			XtFree ((char *)pWindows); XtFree ((char *)pColormaps);
	}
#endif
	else
	{
	    /*
	     * Check to see if the top-level client window is in the list.
	     * If it is not then add it to the head of the list.
	     */

    	    for (i = 0; i < nitems; i++)
	    {
		if (property[i] == pCD->client)
		{
		    break;
		}
	    }

	    colormapCount = 0;
	    if (i == nitems)
	    {
		/* add the client window to the colormap window list */
		pWindows[0] = pCD->client;
		pColormaps[0] = FindColormap (pCD, pCD->client);
		colormapCount++;
	    }

	    sAttributes.event_mask = (ColormapChangeMask);
    	    for (i = 0; i < nitems; i++)
    	    {
		if ((pColormaps[colormapCount] = 
		     FindColormap (pCD, property[i])) != None)
		{
		    pWindows[colormapCount] = property[i];
		    colormapCount++;
		}
		else if (XFindContext (DISPLAY, property[i],
		             wmGD.windowContextType, (caddr_t *)&pcd))
		{
		    /*
		     * The window is not a top level window or a window that
		     * is already being tracked for colormap changes.
		     * Track colormap attribute changes.
		     */

		    XChangeWindowAttributes (DISPLAY, property[i], CWEventMask,
			&sAttributes);
		

		    if (XGetWindowAttributes (DISPLAY, property[i],
			    &wAttributes))
		    {
			pWindows[colormapCount] = property[i];
			pColormaps[colormapCount] = wAttributes.colormap;
		        colormapCount++;
		    }
		}
	    }

	    /*
	     * Free up the old colormap window data if it has been set.  Set
	     * new window contexts.
	     */

	    ResetColormapData (pCD, pWindows, colormapCount);
		

	    /*
	     * Set the colormap window data.
	     */

	    pCD->clientColormap = pColormaps[0];
	    if (colormapCount > 1)
	    {
		/*
		 * The top level window and at least one other window is in
		 * the colormap windows list.
		 */

		pCD->clientCmapCount = colormapCount;
		pCD->cmapWindows = pWindows;
		pCD->clientCmapList = pColormaps;
		pCD->clientCmapIndex = 0;
#ifndef OLD_COLORMAP /* colormap */
		pCD->clientCmapFlags = pCmapFlags;
#endif
	    }
	    else
	    {
		/*
		 * Only the top level window is being tracked for colormap
		 * data.
		 */

		pCD->clientCmapCount = 0;
		XtFree ((char *)pWindows);
		XtFree ((char *)pColormaps);
#ifndef OLD_COLORMAP /* colormap */
		XtFree((char *)pCmapFlags);
#endif
	    }
	}
    }


    if (property)
    {
	XFree ((char *)property);
    }


} /* END OF FUNCTION ProcessWmColormapWindows */



/*************************************<->*************************************
 *
 *  FindColormap (pCD, window)
 *
 *
 *  Description:
 *  -----------
 *  This function checks colormap information that is currently saved in
 *  the client data for the colormap of the specified window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  window = get the colormap id for this window
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = colormap id for window (NULL if no colormap information)
 *
 *************************************<->***********************************/

Colormap FindColormap (ClientData *pCD, Window window)
{
    Colormap colormap = (Colormap)0;
    int i;


    if (pCD->clientCmapCount == 0)
    {
	/*
	 * If the colormap count is 0 there is no list of colormaps and
	 * clientColormap is the colormap of the top-level window.
	 */

	if (window == pCD->client)
	{
	    colormap = pCD->clientColormap;
	}
    }
    else
    {
	for (i = 0; i < pCD->clientCmapCount; i++)
	{
	    if (pCD->cmapWindows[i] == window)
	    {
		colormap = pCD->clientCmapList[i];
		break;
	    }
	}
    }

    return (colormap);

} /* END OF FUNCTION FindColormap */



/*************************************<->*************************************
 *
 *  GetMwmMenuItems (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function reads and processes any _MWM_MENU property that is
 *  associated with a client window and returns a list of MenuItem structures
 *  specified by the property, or NULL.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  Return = MenuItem list or NULL.
 *
 *************************************<->***********************************/

MenuItem *
GetMwmMenuItems(
        ClientData *pCD )
{
    int rValue;
    XTextProperty textProperty;
    MenuItem *menuItems;

    /*
     * Read the _MWM_MENU property.
     */

    textProperty.value = (unsigned char *)NULL;
    rValue = XGetTextProperty(DISPLAY, pCD->client, &textProperty,
			      wmGD.xa_MWM_MENU);
    if ((rValue == 0) || (textProperty.value == (unsigned char *)NULL))
    /* _MWM_MENU does not exist or it is an invalid type.  */
    {
	menuItems = NULL;
    }

    else
    /* parse the property string */
    {
	char **textList;
	int nItems;

	if (XmbTextPropertyToTextList(DISPLAY, &textProperty,
				      &textList, &nItems) != Success)
	{
	    menuItems = NULL;
	}
	else
	{
	    if(textList)
	    {
		menuItems = ParseMwmMenuStr (PSD_FOR_CLIENT(pCD),
					 (unsigned char *)textList[0]);
		XFreeStringList(textList);
	    }
	    else
		menuItems = NULL;
	}

	XFree((void *)textProperty.value);
    }

    return (menuItems);

} /* END OF FUNCTION GetMwmMenuItems */

#ifdef WSM


/*************************************<->*************************************
 *
 *  GetWorkspaceHints (display, window, ppWsAtoms, pCount, pbAll)
 *
 *
 *  Description:
 *  -----------
 *  Get the contents of the WM_COMMAND property on a window
 *
 *
 *  Inputs:
 *  ------
 *  display	- X display 
 *  window	- window to get hints from
 *  ppWsAtoms	- pointer to a list of workspace atoms (to be returned)
 *  pCount	- ptr to a number of atoms (to be returned)
 *  pbAll	- ptr to a boolean (to be returned)
 *
 *
 *  Returns:
 *  --------
 *  Success if suceeded, otherwise failure code.
 * 
 *
 *  Outputs:
 *  -------
 *  *ppWsAtoms 	- list of workspace atoms
 *  *pCount	- number of atoms in *ppWsAtoms
 *  *pbAll	- True if should put in all workspaces
 *
 *
 *  Comments:
 *  --------
 *  The caller must XtFree *ppWsAtoms when done!!!
 * 
 *************************************<->***********************************/

Status GetWorkspaceHints (Display *display, Window window, 
			  Atom **ppWsAtoms, unsigned int *pCount,
			  Boolean *pbAll)
{
    int rcode;
    DtWorkspaceHints *pWsHints;
    Atom *paWs;

    rcode = _DtWsmGetWorkspaceHints(display, window, &pWsHints);
    if (rcode == Success)
    {
	if (pWsHints->flags & DT_WORKSPACE_HINTS_WORKSPACES)
	{
	    paWs = (Atom *) 
		XtMalloc (pWsHints->numWorkspaces * sizeof(Atom));
	    memcpy (paWs, 
		    pWsHints->pWorkspaces, 
		    (pWsHints->numWorkspaces * sizeof(Atom)));

	    *pCount = pWsHints->numWorkspaces;
	    *ppWsAtoms = paWs;
	}
	else
	{
	    *pCount = 0;
	    *ppWsAtoms = NULL;
	}

	if ((pWsHints->flags & DT_WORKSPACE_HINTS_WSFLAGS) &&
	    (pWsHints->wsflags & DT_WORKSPACE_FLAGS_OCCUPY_ALL))
	{
	    *pbAll = True;
	}
	else
	{
	    *pbAll = False;
	}


	_DtWsmFreeWorkspaceHints (pWsHints);
    }
	
    return(rcode);

} /* END OF FUNCTION GetWorkspaceHints */


/*************************************<->*************************************
 *
 *  SetEmbeddedClientsProperty (propWindow, pEmbeddedClients, 
 *  				cEmbeddedCLients)
 *
 *
 *  Description:
 *  -----------
 *  This function writes the _DT_WORKSPACE_EMBEDDED_CLIENTS property
 *
 *
 *  Inputs:
 *  ------
 *  propWindow = window on which the property is to be written
 *  pEmbeddedClients = pointer to data (array of window IDs)
 *  cEmbeddedClients = number of window IDs in the array
 *
 *************************************<->***********************************/

void SetEmbeddedClientsProperty (Window propWindow, 
    Window *pEmbeddedClients, unsigned long cEmbeddedClients)
{
    XChangeProperty (DISPLAY, propWindow, wmGD.xa_DT_EMBEDDED_CLIENTS, 
	wmGD.xa_DT_EMBEDDED_CLIENTS,
	32, PropModeReplace, (unsigned char *)pEmbeddedClients,
	cEmbeddedClients);

} /* END OF FUNCTION SetEmbeddedClientsProperty */

#ifdef HP_VUE

/*************************************<->*************************************
 *
 *  SetWorkspaceInfo (propWindow, pWsInfo, cInfo)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _DT_WORKSPACE_INFO property
 *
 *
 *  Inputs:
 *  ------
 *  propWindow = window on which the _DT_WORKSPACE_INFO property is to be set
 *  pWsInfo =  pointer to workspace info data
 *  cInfo = size of workspace info data
 * 
 *
 *************************************<->***********************************/

void SetWorkspaceInfo (Window propWindow, WorkspaceInfo *pWsInfo, unsigned long cInfo)
{
    XChangeProperty (DISPLAY, propWindow, wmGD.xa_DT_WORKSPACE_INFO, 
	wmGD.xa_DT_WORKSPACE_INFO,
	32, PropModeReplace, (unsigned char *)pWsInfo,
	(cInfo * sizeof(WorkspaceInfo))/sizeof(long));

} /* END OF FUNCTION SetWorkspaceInfo */
#endif /* HP_VUE */


/*************************************<->*************************************
 *
 *  SetWorkspaceListProperty (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _DT_WORKSPACE_LIST property
 *
 *
 *  Inputs:
 *  ------
 *  pSD = ptr to screen data
 * 
 *
 *************************************<->***********************************/

void
SetWorkspaceListProperty (WmScreenData *pSD)
{
    WmWorkspaceData *pws;
    Atom *pWsList;
    int count;

    pWsList = (Atom *) 
	XtMalloc (pSD->numWorkspaces * sizeof(Atom));

    pws = pSD->pWS;
    for (count = 0; count < pSD->numWorkspaces; count++)
    {
	pWsList[count] = pws->id;
	pws++;
    }

    XChangeProperty (DISPLAY, pSD->wmWorkspaceWin, 
        wmGD.xa_DT_WORKSPACE_LIST, 
	XA_ATOM,
	32, PropModeReplace, (unsigned char *)pWsList,
	(pSD->numWorkspaces * sizeof(Atom))/sizeof(long));

    XtFree ((char *) pWsList);

} /* END OF FUNCTION SetWorkspaceListProperty */


/*************************************<->*************************************
 *
 *  SetCurrentWorkspaceProperty (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _DT_WORKSPACE_CURRENT property
 *
 *
 *  Inputs:
 *  ------
 *  pSD = ptr to screen data
 * 
 *
 *************************************<->***********************************/

void
SetCurrentWorkspaceProperty (WmScreenData *pSD)
{
    Atom aCurrent;

    aCurrent = pSD->pActiveWS->id;

    XChangeProperty (DISPLAY, pSD->wmWorkspaceWin, 
        wmGD.xa_DT_WORKSPACE_CURRENT, 
	XA_ATOM,
	32, PropModeReplace, (unsigned char *)&aCurrent,
	(sizeof(Atom))/sizeof(long));

    XSync (DISPLAY, False);     /* XFlush didn't work here, why? */

} /* END OF FUNCTION SetCurrentWorkspaceProperty */


/*************************************<->*************************************
 *
 *  SetWorkspaceInfoProperty (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _DT_WORKSPACE_INFO_<name> property
 *  for a particular workspace
 *
 *
 *  Inputs:
 *  ------
 *  pWS = ptr to workspace data
 * 
 *
 *************************************<->***********************************/

void
SetWorkspaceInfoProperty (WmWorkspaceData *pWS)
{
    char *pch;
    Atom aProperty;
    String sTitle;
    char **ppchList;
    int iNumStrings;
    int count, iwin;
    int i, ix;
    Status status;
    XTextProperty tp;
#define WIP_NUMBER_SIZE		16

    /*
     * Construct our property name
     */
    pch = WorkspacePropertyName (pWS);

    aProperty = XmInternAtom (DISPLAY, pch, FALSE);

    XtFree ((char *) pch);

    /*
     * Determine the number of strings in our vector. One for each of
     *
     *     workspace title
     *     pixel set id
     *     backdrop background
     *     backdrop foreground
     *     backdrop name
     *     number of backdrop windows
     *     list of backdrop windows
     */
    iNumStrings =  6;	/* number of fields minus backdrop window(s) */
    count = 1;		/* number of backdrop windows */
    iNumStrings += count;

    /* allocate string vector */
    ppchList = (char **) XtMalloc (iNumStrings * sizeof (char *));
    pch = (char *) XtMalloc (iNumStrings * WIP_NUMBER_SIZE * sizeof(char));
    
    i = 0;

    /* Convert workspace title to ascii */
    sTitle = (String) WmXmStringToString (pWS->title);
    ppchList[i++] = (char *) sTitle;

    /*  Pixel set id */
    ix = (i * WIP_NUMBER_SIZE);
    sprintf (&pch[ix], "%d", pWS->backdrop.colorSet);
    ppchList[i++] = &pch[ix];

    /* backdrop background */
    ix = (i * WIP_NUMBER_SIZE);
    sprintf (&pch[ix], "0x%lx", pWS->backdrop.background);
    ppchList[i++] = &pch[ix];

    /* backdrop foreground */
    ix = (i * WIP_NUMBER_SIZE);
    sprintf (&pch[ix], "0x%lx", pWS->backdrop.foreground);
    ppchList[i++] = &pch[ix];

    /* backdrop name */
    ix = (i * WIP_NUMBER_SIZE);
    sprintf (&pch[ix], "0x%lx", pWS->backdrop.nameAtom);
    ppchList[i++] = &pch[ix];

    /* number of backdrop windows */
    ix = (i * WIP_NUMBER_SIZE);
    if ((pWS->backdrop.window == None))
    {
	strcpy (&pch[ix], "0");
    }
    else
    {
	sprintf (&pch[ix], "%d", count);
    }
    ppchList[i++] = &pch[ix];

    /* backdrop windows */
    /* 
     * One or zero backdrop windows 
     * (NULL written if zero)
     */
    ix = (i * WIP_NUMBER_SIZE);
    sprintf (&pch[ix], "0x%lx", pWS->backdrop.window);
    ppchList[i++] = &pch[ix];

    /*
     * Write out the property
     */
    status = XmbTextListToTextProperty (DISPLAY, ppchList, iNumStrings,
					    XStdICCTextStyle, &tp);
    if ((status == Success) || (status > 0))
    {
        /*
         * Complete or partial conversion
         */
        XSetTextProperty (DISPLAY, pWS->pSD->wmWorkspaceWin, &tp, aProperty);
        XFree (tp.value);
    }

    XtFree ((char *) ppchList);
    XtFree (pch);
    if (sTitle) XtFree ((char *)sTitle);

} /* END OF FUNCTION SetWorkspaceInfoProperty */


/*************************************<->*************************************
 *
 *  DeleteWorkspaceInfoProperty (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes a _DT_WORKSPACE_INFO_<name> property
 *  for a particular workspace
 *
 *
 *  Inputs:
 *  ------
 *  pWS = ptr to workspace data
 * 
 *
 *************************************<->***********************************/

void
DeleteWorkspaceInfoProperty (WmWorkspaceData *pWS)
{
    char *pch;
    Atom aProperty;

    /*
     * Get the atom for the workspace property.
     */
    pch = WorkspacePropertyName (pWS);

    aProperty = XmInternAtom (DISPLAY, pch, FALSE);

    XtFree ((char *) pch);

    /*
     * Do the property deletion
     */
    XDeleteProperty (DISPLAY, pWS->pSD->wmWorkspaceWin, aProperty);
    XFlush (DISPLAY);


} /* END OF FUNCTION DeleteWorkspaceInfoProperty */


/*************************************<->*************************************
 *
 *  WorkspacePropertyName (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function returns a string containing the property name for a 
 *  workspace.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = ptr to workspace data
 *
 *  Returns
 *  -------
 *  string containing the workspace property name (Free with XtFree)
 *
 *
 *************************************<->***********************************/

char *
WorkspacePropertyName (WmWorkspaceData *pWS)
{
    char *pch;
    char *pchName;
    int len;
    Atom aProperty;

    /*
     * Construct our property name
     */
    pchName = pWS->name;
    len = strlen(pchName) + strlen (_XA_DT_WORKSPACE_INFO) + 4;

    pch = (char *) XtMalloc (len);
    strcpy (pch, _XA_DT_WORKSPACE_INFO);
    strcat (pch, "_");
    strcat (pch, pchName);

    return (pch);

} /* END OF FUNCTION WorkspacePropertyName */


/*************************************<->*************************************
 *
 *  SetWorkspacePresence (propWindow, pWsPresence, cPresence)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the _DT_WORKSPACE_PRESENCE property
 *
 *
 *  Inputs:
 *  ------
 *  propWindow = window on which the _DT_WORKSPACE_PRESENCE property 
 *               is to be set
 *  pWsPresence =  pointer to workspace presence data
 *  cPresence = size of workspace presence data
 * 
 *
 *************************************<->***********************************/

void SetWorkspacePresence (Window propWindow, Atom *pWsPresence, unsigned long cPresence)
{
    XChangeProperty (DISPLAY, propWindow, wmGD.xa_DT_WORKSPACE_PRESENCE, 
	wmGD.xa_DT_WORKSPACE_PRESENCE, 32, PropModeReplace, 
	(unsigned char *)pWsPresence, cPresence);
    XFlush (DISPLAY);

} /* END OF FUNCTION SetWorkspacePresence */




/*************************************<->*************************************
 *
 *  GetDtSessionHints (pSD, sNum)
 *
 *
 *  Description:
 *  -----------
 *  This function reads and processes _DT_SESSION_HINTS property that is
 *  associated with the root window of each screen managed by dtwm
 *
 *
 *  Inputs:
 *  ------
 *
 *
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void GetDtSessionHints (WmScreenData *pSD, int sNum)

{

    int           rValue;
    char          *property = NULL;
    Atom          actualType;
    int           actualFormat;
    unsigned long leftover;
    unsigned long nitems;

    

    /*
     * Read the  property.
     */

    rValue = XGetWindowProperty (DISPLAY, pSD->rootWindow,
                                 wmGD.xa_DT_SESSION_HINTS, 0L,
                                 (long)1000000, False, AnyPropertyType,
                                 &actualType, &actualFormat, &nitems,
                                 &leftover, (unsigned char **)&property);


    if ((rValue != Success) || (actualType == None) || (actualFormat != 8))
        /* _DT_SESSION_HINTS does not exist or it is an invalid type.  */
    {
        pSD->pDtSessionItems = NULL;

    }

    else
        /* parse the property string */
    {
	ParseDtSessionHints (pSD, (unsigned char *)property);
    }


    if (property)
    {
        XFree ((char *)property);
    }
    
    /*
     * Delete the property so we don't see it if the user
     * restarts dtwm.
     */
#ifndef DEBUG_SESSION_HINTS
    XDeleteProperty (DISPLAY, pSD->rootWindow, wmGD.xa_DT_SESSION_HINTS);
#endif /* DEBUG_SESSION_HINTS */
} /* END OF FUNCTION GetDtSessionHints */




/*************************************<->*************************************
 *
 *  GetDtWmRequest (pSD, pszReq, pmore)
 *
 *
 *  Description:
 *  -----------
 *  This function returns the next request
 *
 *
 *  Inputs:
 *  ------
 *  pSD - pointer to screen data
 *  psdReq - pointer to a char pointer
 *
 *
 *  Outputs:
 *  -------
 *  *pszReq	- pointer to null terminated string containing next
 *                request
 *  *pmore	- set to true if more data is left in the property
 *
 *  Comments:
 *  ---------
 *  The data for pszReq is allocated in here. The caller must free up
 *  this space using XtFree.
 *
 *
 *************************************<->***********************************/

void 
GetDtWmRequest (
		WmScreenData *pSD, 
		char **pszReq, 
		Boolean *pmore)

{

    int           rValue;
    char	 *chRequest = NULL;
    static char  *property = NULL;
    static int    iNext = -1;
    int 	  i;
    Atom          actualType;
    int           actualFormat;
    unsigned long leftover;
    static unsigned long nitems = 0;

    
    /*
     * We need to read the property again if we have no data left
     * over from last time;
     */
    if (property == NULL)
    {
#ifdef PARANOID
	/*
	 * Lock down the server to prevent changes to this
	 * property while we "edit" it.
	 */
	XGrabServer(DISPLAY);
#endif /* PARANOID */

	/*
	 * Read the property and delete it.
	 */
	rValue = XGetWindowProperty (DISPLAY, pSD->wmWorkspaceWin,
				     wmGD.xa_DT_WM_REQUEST, 0L,
				     (long)1000000, True, AnyPropertyType,
				     &actualType, &actualFormat, &nitems,
				     &leftover, (unsigned char **)&property);

#ifdef PARANOID
	/* Give the server back */ 
	XUngrabServer(DISPLAY);
#endif /* PARANOID */

	/* 
	 * Validate the property that we've read
	 */
	if ((rValue != Success) || 
	    (actualType == None) || 
	    (actualFormat != 8))
	{
	    /* The property does not exist or it is an invalid type. */
	    property = NULL;
	    iNext = -1;
	    nitems = 0;
	}
	else
	{
	    /* the property is fine, set the index of the next char. */
	    iNext = 0;
	}
    }


    /* 
     * If we've got something, then extract and return the next
     * request.
     */
    if (property && iNext >= 0)
    {
	int len = 0;

	for (i=iNext; i<nitems; i++)
	{
	    if (property [i] == '\0')
	    {
		break;
	    }
	}
	if (i>=nitems) i=nitems;

	len = i - iNext + 1 + ((property[i] == '\0') ? 0 : 1);

        chRequest = (char *) XtMalloc (len);
	if (chRequest == NULL)
	{
	    Warning (((char *)GETMESSAGE(54, 2, "Insufficient memory for window management data")));
	}
	else
	{
	    /* dequeue the request */
	    strncpy (chRequest, &property[iNext], len);
	    if (property[i] != '\0')
	    {
		chRequest[len-1]='\0';
	    }
	    iNext = i+1;
	}
	
	if (iNext >= nitems)
	{
	    /*
	     * Extracted the last request, free up the storage
	     * and reset for next time.
	     */
	    XFree ((char *)property);
	    iNext = -1;
	    property = NULL;
	}
    }
    
    *pmore = (property != NULL);
    *pszReq = chRequest;

} /* END OF FUNCTION GetDtWmRequest */


/*************************************<->*************************************
 *
 *  GetIntialPropertyList (ClientData *)
 *
 *
 *  Description:
 *  -----------
 *  Get the list of initial properties on the window
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD		- paInitialProperties member is updated
 *
 *
 *  Comments:
 *  --------
 *  The caller must XFree the paIntialialProperties member!
 * 
 *************************************<->***********************************/

void
GetInitialPropertyList (ClientData *pCD)
{
    Atom *paList;
    int iProps;

    paList = XListProperties (DISPLAY, pCD->client, &iProps);

    if (paList)
    {
	pCD->paInitialProperties = paList;
	pCD->numInitialProperties = iProps;
    }
    else
    {
	pCD->paInitialProperties = NULL;
	pCD->numInitialProperties = 0;
    }

} /* END OF FUNCTION GetInitialPropertyList */


/*************************************<->*************************************
 *
 *  DiscardIntialPropertyList (ClientData *)
 *
 *
 *  Description:
 *  -----------
 *  Tosses out the intial property list for a client, frees data
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD		- paInitialProperties member is updated
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void
DiscardInitialPropertyList (ClientData *pCD)
{
    if (pCD->paInitialProperties)
    {
	/*
	 * Free the initial property list. 
	 * (see HasProperty() function)
	 */
	XFree ((char *) pCD->paInitialProperties);
	pCD->paInitialProperties = NULL;
	pCD->numInitialProperties = 0;
    }
} /* END OF FUNCTION DiscardInitialPropertyList */


/*************************************<->*************************************
 *
 *  HasProperty (pCD, aProperty)
 *
 *
 *  Description:
 *  -----------
 *  Returns True if this client had this property when it was mapped
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  aProperty	- atom of property to test for
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- True if this property was on the initial list for this
 *		  client; False otherwise.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean
HasProperty (
	ClientData *	pCD,
	Atom		aProperty)
{
    Boolean 	bFound = False;
    Atom  *	paList;
    int		count;

    paList = pCD->paInitialProperties;

    if (paList)
    {
	count = pCD->numInitialProperties;
	while ((!bFound) && (count > 0))
	{
	    bFound = (*paList == aProperty);
	    count--;
	    paList++;
	}
    }
    else
    {
	/*
	 * The property list doesn't exist. Return
	 * True to force a read of this property. The most likely
	 * case is that this property was updated after the 
	 * window was managed and needs to be read.
	 */
	bFound = True;
    }
    return (bFound);

} /* END OF FUNCTION HasProperty */
#endif /* WSM */



