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
static char rcsid[] = "$TOG: WmWinInfo.c /main/18 1999/02/04 15:17:25 mgreess $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmICCC.h"
#include "WmResNames.h"

#define MWM_NEED_ICONBOX
#include "WmIBitmap.h"

#include <Xm/Xm.h>
#include <X11/Xlocale.h>
#ifdef PANELIST
#include "WmPanelP.h"
#endif /* PANELIST */

#define makemult(a, b) ((b==1) ? (a) : (((int)((a) / (b))) * (b)) )

/*
 * include extern functions
 */
#include "WmWinInfo.h"
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmCPlace.h"
#include "WmError.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmImage.h"
#include "WmManage.h"
#include "WmMenu.h"
#include "WmProperty.h" 
#include "WmResource.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */
#include "WmWinList.h"
#ifdef WSM
#include "WmPresence.h"
#endif /* WSM */
#include "WmXSMP.h"

#ifdef PANELIST
static void AdjustSlideOutGeometry (ClientData *pCD);
static void FixSubpanelEmbeddedClientGeometry (ClientData *pCD);
#endif /* PANELIST */

#ifndef NO_MESSAGE_CATALOG
# define LOCALE_MSG GETMESSAGE(70, 7, "[XmbTextPropertyToTextList]:\n     Locale (%.100s) not supported. (Check $LANG).")
#else
# define LOCALE_MSG "[XmbTextPropertyToTextList]:\n     Locale (%.100s) not supported. (Check $LANG)."
#endif

/*
 * Global Variables:
 */
#ifdef WSM
WmWorkspaceData *pIconBoxInitialWS;
#endif /* WSM */



/*************************************<->*************************************
 *
 *  GetClientInfo (pSD, clientWindow, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to initialize client window data based on the
 *  contents of client window properties and the client window configuration.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data for screen that client lives in
 *
 *  clientWindow = window id for the client window that is to be managed
 *
 *  manageFlags = flags that indicate wm state info
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to an initialized client data structure for the
 *           specified client window
 *
 *************************************<->***********************************/

ClientData * 
GetClientInfo (WmScreenData *pSD, Window clientWindow, long manageFlags)

{
    ClientData *pCD;
    XSetWindowAttributes sAttributes;


    /*
     * Allocate and initialize a client data structure:
     */

    if (!(pCD = (ClientData *)XtMalloc (sizeof (ClientData))))
    {
	/* unable to allocate space */
	Warning (((char *)GETMESSAGE(70, 1, "Insufficient memory for client data")));
	return (NULL);
    }

    
    /*
     * Initialize the data structure:
     */

    pCD->client = clientWindow;
    pCD->clientID = ++(pSD->clientCounter);
    pCD->clientFlags = WM_INITIALIZATION;
    pCD->iconFlags = 0;
#ifndef WSM
    pCD->pIconBox = NULL;
#endif /* WSM */
    pCD->thisIconBox = NULL;
#ifdef PANELIST
    pCD->pECD = NULL;
    pCD->pPRCD = NULL;
    pCD->pSOR = NULL;
#endif /* PANELIST */
    pCD->wmUnmapCount = 0;
    pCD->transientFor = (Window)0L;
    pCD->transientLeader = NULL;
    pCD->transientChildren = NULL;
    pCD->transientSiblings = NULL;
#ifdef WSM
    pCD->primaryStackPosition = 0;
#endif /* WSM */
    pCD->fullModalCount = 0;
    pCD->primaryModalCount = 0;
    pCD->focusPriority = 0;
    pCD->focusAutoRaiseDisabled = False;
    pCD->focusAutoRaiseDisablePending = False;

    pCD->clientClass = NULL;
    pCD->clientName = NULL;
    pCD->clientFrameWin = (Window)0L;
#ifndef WSM
    pCD->iconFrameWin = (Window)0L;
#endif /* WSM */
    pCD->iconWindow = (Window)0L;
    pCD->iconPixmap = (Pixmap)0L;
#ifndef WSM
    pCD->iconPlace = NO_ICON_PLACE;
#endif /* WSM */
    pCD->clientProtocols = NULL;
    pCD->clientProtocolCount = 0;
    pCD->mwmMessages = NULL;
    pCD->mwmMessagesCount = 0;
    pCD->clientCmapCount = 0;
    pCD->clientCmapIndex = 0;
    pCD->clientCmapFlagsInitialized = FALSE;
    pCD->systemMenuSpec = NULL;
#ifdef WSM
    pCD->putInAll = False;
    pCD->pWorkspaceHints = NULL;
    pCD->numInhabited = 0;
    pCD->pWsList = NULL;
    pCD->dtwmFunctions = DtWM_FUNCTION_OCCUPY_WS;
    pCD->dtwmBehaviors = 0L;
    pCD->paInitialProperties = NULL;
    pCD->numInitialProperties = 0;
#endif /* WSM */

    pCD->decorFlags = 0L;
    pCD->pTitleGadgets = NULL;
    pCD->cTitleGadgets = 0;
    pCD->pResizeGadgets = NULL;
    pCD->clientTitleWin = (Window)0L;
    pCD->pclientTopShadows = NULL;
    pCD->pclientBottomShadows = NULL;
    pCD->pclientTitleTopShadows = NULL;
    pCD->pclientTitleBottomShadows = NULL;
    pCD->pclientMatteTopShadows = NULL;
    pCD->pclientMatteBottomShadows = NULL;
    pCD->piconTopShadows = NULL;
    pCD->piconBottomShadows = NULL;
    pCD->internalBevel = (wmGD.frameStyle == WmSLAB) ? 0 : 
						FRAME_INTERNAL_SHADOW_WIDTH;
#ifndef NO_OL_COMPAT
    pCD->bPseudoTransient = False;
#endif /* NO_OL_COMPAT */

    pCD->maxWidth = pCD->maxWidthLimit = BIGSIZE;
    pCD->maxHeight = pCD->maxHeightLimit = BIGSIZE;
    pCD->maxConfig = FALSE;
    pCD->pSD = pSD;
    pCD->dataType = CLIENT_DATA_TYPE;
    pCD->window_status = 0L;

    pCD->clientEntry.nextSibling = NULL;
    pCD->clientEntry.prevSibling = NULL;
    pCD->clientEntry.pCD = NULL;

    pCD->smClientID = (String)NULL;

     /*
     * Do special processing for client windows that are controlled by
     * the window manager.
     */

    if (manageFlags & MANAGEW_WM_CLIENTS)
    {
#ifdef WSM
        WmWorkspaceData *pWS;

	if (manageFlags & MANAGEW_ICON_BOX)
	{
	    pWS = pIconBoxInitialWS;
	}
	else 
	{
	    pWS = pSD->pActiveWS;
	}
	return (GetWmClientInfo (pWS, pCD, manageFlags));
#else /* WSM */
	return (GetWmClientInfo (pSD->pActiveWS, pCD, manageFlags));
#endif /* WSM */
    }


    /*
     * Register the client window to facilitate event handling:
     */

    XSaveContext (DISPLAY, clientWindow, wmGD.windowContextType, (caddr_t)pCD);


    /*
     * Listen for property change events on the window so that we keep 
     * in sync with the hints.
     */
    sAttributes.event_mask = (PropertyChangeMask | ColormapChangeMask);    
    XChangeWindowAttributes (DISPLAY, pCD->client, CWEventMask,
        &sAttributes);

    /*
     * Get window configuration attributes.  WmGetWindowAttributes sets
     * up the global window attributes cache with the client window 
     * attributes.
     */

    if (!WmGetWindowAttributes (clientWindow))
    {
	/*
	 * Cannot get window attributes. Do not manage window.
	 * (error message within WmGetWindowAttributes)
	 */

	UnManageWindow (pCD);
	return (NULL);
    }
    pCD->xBorderWidth = wmGD.windowAttributes.border_width;

#ifdef WSM
    /*
     * Get the initial list of properties on this window. 
     * Save it to optimize subsequent property fetching.
     */
    GetInitialPropertyList (pCD);
#endif /* WSM */

    /*
     * Retrieve and process WM_CLASS hints client window property info:
     */

    ProcessWmClass (pCD);


    /*
     * Retrieve and process WM_TRANSIENT_FOR client window property info:
     */

    ProcessWmTransientFor (pCD);

    /*
     * Get client window resource data (from resources, .mwmrc):
     *  Moved prior to GetClientWorkspaceInfo() because the
     *  ignoreWMSaveHints resource may affect that function.
     */

    ProcessClientResources (pCD);

    /*
     * Retreive and process SM_CLIENT_ID client window property info
     * and WMSAVE_HINT client window property info:
     * must be done prior to calling GetClientWorkspaceInfo().
     */
    ProcessSmClientID (pCD);
    ProcessWmSaveHint (pCD);

#ifdef WSM
    /*
     *  Set client's workspace information.  NOTE: this also may
     *  set the geometry, initial state, etc.  For Sm-aware clients,
     *  this info will be in private DB; for older clients, it will
     *  be contained in the screen's pDtSessionItems.
     */
    if (!GetClientWorkspaceInfo (pCD, manageFlags))
    {
	XtFree ((char *)pCD);
	return (NULL);
    }

    /*
     *  Restore client's per-workspace icon information.
     */
    LoadClientIconPositions(pCD);

    /*
     * Retrieve and process _DT_WM_HINTS client window property
     * (results are used in ProcessMwmHints)
     */
    ProcessDtWmHints (pCD);
#else

    /*
     * For Sm-aware clients, retrieve geometry and initial state
     * from private DB.
     */
    FindClientDBMatch(pCD, (char **)NULL);

#endif /* WSM */

    /*
     * Retrieve and process M_CLIENT_DECOR client window property info:
     */

    ProcessMwmHints (pCD);


    /*
     * Retrieve and process WM_HINTS client window property info:
     */

    ProcessWmHints (pCD, True /*first time*/);


    /*
     * Set offset from frame of client window
     */

    SetClientOffset (pCD);


    /*
     * Retrieve and process WM_NORMAL_HINTS client window property info:
     * 
     */

    ProcessWmNormalHints (pCD, True /*first time*/, manageFlags);


    /*
     * Retrieve and process WM_NAME client window property info (this
     * property contains the window title NOT the window resource name):
     */

    ProcessWmWindowTitle (pCD, TRUE);


    /*
     * Retrieve and process WM_ICON_NAME client window property info:
     */

    ProcessWmIconTitle (pCD, TRUE);


    /*
     * Retrieve and process the WM_PROTOCOLS property.
     */

    ProcessWmProtocols (pCD);


    /*
     * If necessary retrieve and process the _MWM_MESSAGES property.
     */

    if (pCD->protocolFlags & PROTOCOL_MWM_MESSAGES)
    {
	ProcessMwmMessages (pCD);
    }


    /*
     * Make or find a system menu for the client.
     */

    if (pCD->systemMenu)
    {
	MakeSystemMenu (pCD);
    }
    else
    {
	pCD->systemMenuSpec = NULL;
    }


    /*
     * Setup the colormap data for the client window.  This includes
     * retrieving and processing client window properties that deal with
     * subwindow colormaps.
     */

    InitCColormapData (pCD);


    /* successful return */

    return (pCD);


} /* END OF FUNCTION GetClientInfo */



/*************************************<->*************************************
 *
 *  GetWmClientInfo (pWS, pCD, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to initialize client window data for a window
 *  that is controlled by the window manager (e.g., the icon box).  The 
 *  client window may get made in the process.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 *  pCD = pointer to client window data structure
 *
 *  manageFlags = flags that indicate wm state info
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to an initialized client data structure or NULL
 *           if the client data could not be initialized
 *
 *************************************<->***********************************/
ClientData * 
GetWmClientInfo (WmWorkspaceData *pWS,
			      ClientData * pCD,
			      long manageFlags)

{
    Pixmap 	iconBitmap;
#ifdef WSM
int i;
#endif /* WSM */

    /*
     * Set up the client class and name for resource retrieval.
     * Get client specific resource data (from resources, .mwmrc).
     */

    if (manageFlags & MANAGEW_ICON_BOX)
    {
	SetIconBoxInfo (pWS, pCD);
    }
    else if (manageFlags & MANAGEW_CONFIRM_BOX)
    {
	pCD->clientClass = WmCConfirmbox;
	pCD->clientName = WmNconfirmbox;
	pCD->iconImage = NULL;
	pCD->useClientIcon = False;
	pCD->focusAutoRaise = True;
	pCD->internalBevel = (wmGD.frameStyle == WmSLAB) ? 0 : 
						FRAME_INTERNAL_SHADOW_WIDTH;
	pCD->matteWidth = 0;
	pCD->maximumClientSize.width = 0;
	pCD->maximumClientSize.height = 0;
	pCD->systemMenu = NULL;
    }


    /*
     * Set up transient for data.
     */

    if (manageFlags & MANAGEW_ICON_BOX)
    {
    }


    /*
     * Set up WM_HINTS type information.
     */

    pCD->inputFocusModel = True;
    pCD->clientState = NORMAL_STATE;
    
    if (ICON_DECORATION(pCD) & ICON_IMAGE_PART)
    {
	if (manageFlags & MANAGEW_ICON_BOX)
	{
	    pCD->clientFlags |= ICON_BOX;
	}
        
	if (!pCD->useClientIcon && pCD->iconImage)
        {
	    /*
	     * Make a client supplied icon image.
	     * Do not use the default icon image if iconImage is not found.
	     */

	    pCD->iconPixmap = MakeNamedIconPixmap (pCD, pCD->iconImage);
        }

	if (!pCD->iconPixmap)
        {
	    /*
	     * Use a built-in icon image for the window manager client.
	     * The image may differ from the default icon image, depending on
	     * the particular client (eg the iconbox).
	     */

	    if (manageFlags & MANAGEW_ICON_BOX)
	    {
		/*
		 * Make a default iconBox icon image.
		 */

		iconBitmap = XCreateBitmapFromData (DISPLAY, 
				ROOT_FOR_CLIENT(pCD), (char *)iconBox_bits, 
				iconBox_width, iconBox_height);

		pCD->iconPixmap = MakeIconPixmap (pCD, 
				iconBitmap, (Pixmap)0L,
				iconBox_width, iconBox_height, 1);

	    }
        }
    }

#ifdef WSM
    /* 
     * Allocate initial workspace ID list 
     * fill with NULL IDs
     */
    if ((pCD->pWsList = (WsClientData *) 
	    XtMalloc(pCD->pSD->numWorkspaces * sizeof(WsClientData))) == NULL)
    {
	Warning (((char *)GETMESSAGE(70, 2, "Insufficient memory for client data")));
	return (NULL);
    }
    pCD->sizeWsList = pCD->pSD->numWorkspaces;
    for (i = 0; i < pCD->pSD->numWorkspaces; i++)
    {
	pCD->pWsList[i].wsID = NULL;
	pCD->pWsList[i].iconPlace = NO_ICON_PLACE;
	pCD->pWsList[i].iconX = 0;
	pCD->pWsList[i].iconY = 0;
	pCD->pWsList[i].iconFrameWin = (Window)0L;
	pCD->pWsList[i].pIconBox = NULL;
    }
    /* 
     * Internally managed clients must be specifically inserted
     * into workspaces the first time by calling
     * PutClientIntoWorkspace.
     */
    pCD->numInhabited = 0;
#else /* WSM */
    pCD->iconPlace = NO_ICON_PLACE;
    pCD->iconX = 0;
    pCD->iconY = 0;
#endif /* WSM */
    pCD->windowGroup = 0L;
#ifndef NO_OL_COMPAT
    pCD->bPseudoTransient = False;
#endif /* NO_OL_COMPAT */


    /*
     * Set up _MWM_HINTS data.
     */
    /*
     * Fix the client functions and decorations fields if they have
     * default resource values.
     */


    if (manageFlags & MANAGEW_CONFIRM_BOX)
    {
        pCD->clientFunctions = WM_FUNC_NONE;
        pCD->clientDecoration = WM_DECOR_BORDER;
    }
    else
    {
        if (pCD->clientFunctions & WM_FUNC_DEFAULT)
        {
	    pCD->clientFunctions = WM_FUNC_ALL;
        }

        if (pCD->clientDecoration & WM_DECOR_DEFAULT)
        {
	    pCD->clientDecoration = WM_DECOR_ALL;
        }

        if (manageFlags & MANAGEW_ICON_BOX)
        {
            pCD->clientFunctions &= ICON_BOX_FUNCTIONS;
#ifdef WSM
	    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
#endif /* WSM */
#ifdef PANELIST
	    if (wmGD.useFrontPanel && pCD->pSD->iconBoxControl) 
	    { 
		/*
		 * If there's a front panel button for the icon
		 * box, then use it to "hide" the box on "close"
		 */
		pCD->clientFunctions &= ~MWM_FUNC_MINIMIZE;
		pCD->clientFunctions |= MWM_FUNC_CLOSE;
	    }
#else /* PANELIST */
#endif /* PANELIST */
        }


        if (!(pCD->clientFunctions & MWM_FUNC_RESIZE))
        {
	    pCD->clientDecoration &= ~MWM_DECOR_RESIZEH;
        }

        if (!(pCD->clientFunctions & MWM_FUNC_MINIMIZE))
        {
	    pCD->clientDecoration &= ~MWM_DECOR_MINIMIZE;
        }

        if (!(pCD->clientFunctions & MWM_FUNC_MAXIMIZE))
        {
	    pCD->clientDecoration &= ~MWM_DECOR_MAXIMIZE;
        }
    }

    pCD->decor = pCD->clientDecoration;

    if (manageFlags & MANAGEW_ICON_BOX)
    {
        pCD->inputMode = MWM_INPUT_MODELESS;
    }
    else if (manageFlags & MANAGEW_CONFIRM_BOX)
    {
        pCD->inputMode = MWM_INPUT_SYSTEM_MODAL;
    }

    /*
     * Set up WM_NORMAL_HINTS data.
     */

    pCD->icccVersion = ICCC_CURRENT;
    pCD->sizeFlags = US_POSITION | US_SIZE;

    /* 
     * Any calls to create Window Manager clients should
     * return with the values for the following fields set.
     *  If it fails, it should free any space allocated and
     *  set pCD = NULL
     * 
     *  pCD->clientX =
     *  pCD->clientY =
     *  pCD->clientWidth =
     *  pCD->clientHeight =
     *  pCD->minWidth =
     *  pCD->minHeight =
     *  pCD->widthInc =
     *  pCD->heightInc =
     *  pCD->baseWidth =
     *  pCD->baseHeight =
     *  pCD->maxWidth =
     *  pCD->maxHeight =
     *  pCD->oldMaxWidth =
     *  pCD->oldMaxHeight =
     *
     *        AND PROBABLY SHOULD SET
     *         pCD->client = THE_WINDOW_THE_FUNCTION_CREATES
     */

    pCD->windowGravity = NorthWestGravity;

    /* 
     * Create IconBox window 
     */
    
    if (manageFlags & MANAGEW_ICON_BOX)
    {
        if (!MakeIconBox (pWS, pCD))
        {
            /*
             *  May want a more verbose message here 
             */

            Warning (((char *)GETMESSAGE(70, 3, "Couldn't make icon box")));
	    return (NULL);
        }
#ifdef WSM
	PutClientIntoWorkspace (pWS, pCD);
#endif /* WSM */
    }
    else if (manageFlags & MANAGEW_CONFIRM_BOX)
    {
	Window       root;
	unsigned int cbWidth, cbHeight;
	unsigned int depth;

        XGetGeometry (DISPLAY, pCD->client, &root,
		      &(pCD->clientX), &(pCD->clientY),
		      &cbWidth, &cbHeight, 
		      (unsigned int*)&(pCD->xBorderWidth), &depth);

        pCD->clientWidth = cbWidth;
        pCD->clientHeight = cbHeight;
	pCD->minWidth = pCD->baseWidth = pCD->maxWidth = pCD->clientWidth;
	pCD->minHeight = pCD->baseHeight = pCD->maxHeight = pCD->clientHeight;
	pCD->oldMaxWidth = pCD->maxWidth;
	pCD->oldMaxHeight = pCD->maxHeight;
        pCD->widthInc = 1;
        pCD->heightInc = 1;
        pCD->clientFlags |= CONFIRM_BOX;
#ifdef WSM
	PutClientIntoWorkspace (ACTIVE_WS, pCD);
#endif /* WSM */
    }

    /*
     * Set offset from frame of client window (need client size information).
     */

    SetFrameInfo (pCD);


    /*
     * Register the client window to facilitate event handling.
     */

    XSaveContext (DISPLAY, pCD->client, wmGD.windowContextType, (caddr_t)pCD);


    /*
     * Set up WM_PROTOCOLS data.
     */

    pCD->clientProtocolCount = 0;
    pCD->protocolFlags = 0;


    /*
     * Make the system menu.
     */

    if (manageFlags & MANAGEW_ICON_BOX)
    {
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	/** BEGIN FIX CR 6941 **/
	MenuItem *iconBoxMenuItems, *lastItem;

	/* This MenuSpec is not added to pSD->acceleratorMenuSpecs */
	pCD->systemMenuSpec = 
	    MAKE_MENU (PSD_FOR_CLIENT(pCD), pCD, pCD->systemMenu,
		      F_CONTEXT_WINDOW, F_CONTEXT_WINDOW|F_CONTEXT_ICON,
		      NULL, TRUE);
	if (pCD->systemMenuSpec != (MenuSpec *) NULL)
	{
	    pCD->systemMenuSpec = DuplicateMenuSpec(pCD->systemMenuSpec);
	    XtFree(pCD->systemMenuSpec->name);
	    pCD->systemMenuSpec->name = XtNewString("IconBoxMenu");
	    iconBoxMenuItems = GetIconBoxMenuItems (PSD_FOR_CLIENT(pCD));
	    
	    /* Find the last menu item in the menu spec's list. */
	    for (lastItem = pCD->systemMenuSpec->menuItems;
		 lastItem->nextMenuItem != (MenuItem *) NULL;
		 lastItem = lastItem->nextMenuItem)
	      /*EMPTY*/;
	    lastItem->nextMenuItem = iconBoxMenuItems;
	    
	    /* Now recreate the menu widgets since we've appended the
	       icon box menu items */
	    DestroyMenuSpecWidgets(pCD->systemMenuSpec);
	    pCD->systemMenuSpec->menuWidget =
	      CreateMenuWidget (PSD_FOR_CLIENT(pCD), pCD, "IconBoxMenu",
				PSD_FOR_CLIENT(pCD)->screenTopLevelW, TRUE,
				pCD->systemMenuSpec, NULL);
	}
	/** END FIX CR 6941 **/
#else
	pCD->systemMenuSpec = 
	    MAKE_MENU (PSD_FOR_CLIENT(pCD), pCD, pCD->systemMenu,
		       F_CONTEXT_WINDOW, F_CONTEXT_WINDOW|F_CONTEXT_ICON,
		       GetIconBoxMenuItems(PSD_FOR_CLIENT(pCD)),
		       TRUE);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    }



    /*
     * Setup the colormap data.
     */

    pCD->clientColormap = PSD_FOR_CLIENT(pCD)->workspaceColormap;


    /*
     * Return the pointer to the client data.
     */

    return (pCD);


} /* END OF FUNCTION GetWmClientInfo */



/*************************************<->*************************************
 *
 *  ProcessWmClass (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_CLASS property on the
 *  cient window.  The resource class and the resource name are saved in
 *  the ClientData structure (note that the space for the strings is
 *  allocated using Xmalloc).
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
ProcessWmClass (ClientData *pCD)
{
    XClassHint classHint;


#ifdef PORT_OLDXLIB
    classHint.res_class = "";
    classHint.res_name = "";
    XGetClassHint (DISPLAY, pCD->client, &classHint);
#else
#ifdef WSM
    if ((HasProperty (pCD, XA_WM_CLASS)) &&
	(XGetClassHint (DISPLAY, pCD->client, &classHint)))
#else /* WSM */
    if (XGetClassHint (DISPLAY, pCD->client, &classHint))
#endif /* WSM */
#endif
    {
	/* the WM_CLASS property exists for the client window */
	pCD->clientClass = classHint.res_class;
	pCD->clientName = classHint.res_name;
    }
    /* else no WM_CLASS property; assume clientClass, clientName are NULL */

} /* END OF FUNCTION ProcessWmClass */



/*************************************<->*************************************
 *
 *  ProcessSmClientID (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the SM_CLIENT_ID property on the
 *  cient window.  The value is saved in the ClientData structure
 *  (note that the space for the strings is allocated using Xmalloc).
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
ProcessSmClientID (ClientData *pCD)
{
    Atom actualType;
    int actualFormat;
    unsigned long nitems, leftover;
    char *clientID;

    if (pCD->smClientID != (String)NULL)
    {
	XFree(pCD->smClientID);
	pCD->smClientID = (String)NULL;
    }

    if ((XGetWindowProperty(DISPLAY, pCD->client, wmGD.xa_SM_CLIENT_ID,
			    0L, (long)1000000, False, AnyPropertyType,
			    &actualType, &actualFormat, &nitems,
			    &leftover, (unsigned char **)&clientID)
	 == Success) &&
	(actualType != None) && (actualFormat == 8))
    {
	/* the SM_CLIENT_ID property exists for the client window */
	pCD->smClientID = clientID;
    }

} /* END OF FUNCTION ProcessSmClientID */



/*************************************<->*************************************
 *
 *  ProcessWmSaveHint (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WMSAVE_HINT property on the
 *  cient window.  The value is saved in the ClientData structure.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
ProcessWmSaveHint (ClientData *pCD)
{
    Atom actualType;
    int actualFormat;
    unsigned long nitems, leftover;
    BITS32 *saveHintFlags = (BITS32 *)NULL;

    if ((XGetWindowProperty(DISPLAY, pCD->client, wmGD.xa_WMSAVE_HINT,
			    0L, (long)1000000, False, AnyPropertyType,
			    &actualType, &actualFormat, &nitems,
			    &leftover, (unsigned char **)&saveHintFlags)
	 == Success) &&
	(actualType != None) && (actualFormat == 32))
    {
	/* the WMSAVE_HINT property exists for the client window */
	pCD->wmSaveHintFlags = (int)*saveHintFlags;
    }
    else pCD->wmSaveHintFlags = 0;

    if (saveHintFlags)
	XFree(saveHintFlags);

} /* END OF FUNCTION ProcessWmSaveHint */


/*************************************<->*************************************
 *
 *  ProcessWmHints (pCD, firstTime)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_HINTS property on the
 *  cient window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for the window with the property
 *
 *  firstTime = if True this is the first time the property has been processed
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = initialize various WM_HINTS related fields
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
ProcessWmHints (ClientData *pCD, Boolean firstTime)
{
    register XWMHints *pXWMHints;
    register long flags;
    Pixmap iconPixmap;
    Pixmap iconMask;
#ifdef WSM
    WmWorkspaceData *pWsTmp;
    WsClientData *pWsc;
    int iws;
#endif /* WSM */
    int tmpIconX, tmpIconY;


    /*
     * If the WM_HINTS property does not exist the flags field will be
     * set to 0.  If flags is 0 don't reference the WMHints structure
     * since they may be none.
     */

#ifdef WSM
    if (firstTime && !HasProperty (pCD, XA_WM_HINTS))
	pXWMHints = NULL;
    else
#endif /* WSM */
    pXWMHints = XGetWMHints (DISPLAY, pCD->client);

    if (pXWMHints)
    {
	flags = pXWMHints->flags;
    }
    else
    {
	flags = 0;
    }


    /*
     * Parse the WM_HINTS information.  If this is the first time the hints
     * have been processed then parse all fields and set defaults where hint
     * fields are not set.  If this is not the first time do selective
     * parsing.
     */

    if (firstTime)
    {
#ifndef NO_OL_COMPAT
        ClientData *leader;
	Atom *pIDs;
	unsigned int numIDs = 0;

        /*
         * Save the window group.
         */

        if (flags & WindowGroupHint)
	{
	    pCD->windowGroup = pXWMHints->window_group;
	    /*
	     * Pretend this is a transient window 
	     */
	    if ((pCD->bPseudoTransient) && 
		(pCD->transientFor == (Window)0L))
	    {
		pCD->clientFlags |= CLIENT_TRANSIENT;

		/*
		 * Treat this like a transient window. This is transient
		 * for the window group window.
		 */

		if ((pCD->client != pCD->windowGroup) &&
		    !XFindContext (DISPLAY, pCD->windowGroup, 
			wmGD.windowContextType, (caddr_t *)&leader))
		{
		    pCD->transientFor = pCD->windowGroup;
		    pCD->transientLeader = leader;

		    /*
		     * Insure it is in the same set of workspaces
		     * as the leader.
		     */
		    if (pCD->transientLeader && 
			GetLeaderPresence(pCD, &pIDs, &numIDs))
		    {
			ProcessWorkspaceHintList (pCD, pIDs, numIDs);
		    }
		}
	    }
	}
	else
	{
	    pCD->windowGroup = 0L;
	}
#endif /* NO_OL_COMPAT */
        /*
         * The window manger does not do anything with the input hint.  Input
         * always goes to the selected window.
         */

        if (flags & InputHint)
	{
	    pCD->inputFocusModel = pXWMHints->input;
	}
	else
	{
	    pCD->inputFocusModel = True;
	}


        /*
         *  The default state is NORMAL_STATE.  States other than iconic
         *  (e.g., ZoomState from the R2 ICCC) indicate to the window manager
         *  that the NORMAL_STATE is to be used.
         */

	if (pCD->clientFlags & SM_CLIENT_STATE)
	{
	     if ((pCD->clientState == MINIMIZED_STATE) &&
	    	(!(pCD->clientFunctions & MWM_FUNC_MINIMIZE)))
	     {
	         pCD->clientState = NORMAL_STATE;
	     }
	}
	else
        if ((flags & StateHint) && (pXWMHints->initial_state == IconicState) &&
	    (pCD->clientFunctions & MWM_FUNC_MINIMIZE))
        {
	    pCD->clientState = MINIMIZED_STATE;
	}
        else
        {
	    /*
	     * States other than iconic are treated as normal.
	     */
	    pCD->clientState = NORMAL_STATE;
        }


#ifdef WSM
	if (!ClientInWorkspace (PSD_FOR_CLIENT(pCD)->pActiveWS, pCD))
	{
	    pCD->clientState |= UNSEEN_STATE;
	}
#endif /* WSM */


        /*
	 * If an icon is to be made for the client then ...
         * save the icon image if useClientIcon is True or there is no
	 * user specified icon image.  A client supplied image may be a
	 * pixmap or a window (a client icon window takes precedence over
	 * a pixmap).
         */

	if ((pCD->clientFunctions & MWM_FUNC_MINIMIZE) &&
	    (pCD->transientLeader == NULL))
	{
            if ((ICON_DECORATION(pCD) & ICON_IMAGE_PART) &&
	        (pCD->useClientIcon || !pCD->iconImage))
            {
	        if ((flags & IconWindowHint) &&
		    (pXWMHints->icon_window != pCD->client))
	        {
	            /* 
	             * An icon window has been supplied that is different from
		     * the client window.  Check out the window and get it 
		     * ready to be reparented to the window manager supplied
	             * icon frame.
	             */

	            if (!SetupClientIconWindow (pCD, pXWMHints->icon_window))
		    {
		        /*
		         * Cannot use the client supplied icon window.  Use
		         * an icon image if specified or a default image.
		         */
		    }
	        }
	        if (!pCD->iconWindow && (flags & IconPixmapHint))
	        {
		    iconMask = (flags & IconMaskHint) ?
				pXWMHints->icon_mask : (Pixmap) NULL;
	            /*
	             * A client supplied icon window is NOT 
		     * available so use the client supplied icon image.
	             */

	            if ((pCD->iconPixmap = 
			    MakeClientIconPixmap (pCD,
			      pXWMHints->icon_pixmap, iconMask)) != None)
		    {
		        /*
		         * Indicate that a client supplied icon image is being
		         * used.
		         */

		        pCD->iconFlags |= ICON_HINTS_PIXMAP;
		    }
		    else
		    {
		        /*
		         * Cannot make a client supplied image.  Use a user
		         * specified icon image if it is available or a default
		         * icon image.
		         */
		    }
	        }
	    }

            if ((ICON_DECORATION(pCD) & ICON_IMAGE_PART) && !pCD->iconPixmap)
	    {
	        /*
	         * Use a user supplied icon image if it is available or a
	         * default icon image.
	         */

	        if (pCD->iconImage)
	        {
		    /*
		     * Try to make a user specified icon image.
		     */

		    pCD->iconPixmap = 
			MakeNamedIconPixmap (pCD, pCD->iconImage);
	        }

	        if (!pCD->iconPixmap)
	        {
		    /*
		     * The icon image was not provided or not available.
		     * Use the default icon image.
		     */

		    pCD->iconPixmap = MakeNamedIconPixmap (pCD, NULL);
	        }
	    }
    

            /*
             * Save the client (user?) supplied icon position:
             */

            if ((flags & IconPositionHint) ||
		(pCD->clientFlags & (SM_ICON_X | SM_ICON_Y)))
            {
	        pCD->iconFlags |= ICON_HINTS_POSITION;
	        if (wmGD.iconAutoPlace)
#ifdef WSM
	        {
		    /* 
		     * Initialize icon placement data in all inhabited
		     * workspaces
		     */
		    for (iws = 0; iws< pCD->numInhabited; iws++)
		    {
			pWsc = &(pCD->pWsList[iws]);
			if (pWsTmp=GetWorkspaceData(pCD->pSD, pWsc->wsID))
			{
			    tmpIconX = (pCD->clientFlags & SM_ICON_X) ?
			      pWsc->iconX : pXWMHints->icon_x;
			    tmpIconY = (pCD->clientFlags & SM_ICON_Y) ?
			      pWsc->iconY : pXWMHints->icon_y;
			    pWsc->iconPlace =
				FindIconPlace (pCD, &(pWsTmp->IPData),
					       tmpIconX, tmpIconY);
			    if (pWsc->iconPlace != NO_ICON_PLACE)
			    {
				CvtIconPlaceToPosition ( &(pWsTmp->IPData),
				    pWsc->iconPlace,
				    &pWsc->iconX,
				    &pWsc->iconY);
			    }
			}
		    }
		}
	        else
	        {
		    for (iws = 0; iws< pCD->numInhabited; iws++)
		    {
			pWsc = &(pCD->pWsList[iws]);
			if (pWsTmp=GetWorkspaceData(pCD->pSD, pWsc->wsID))
			{
			    if (!(pCD->clientFlags & SM_ICON_X))
				pWsc->iconX = pXWMHints->icon_x;
			    if (!(pCD->clientFlags & SM_ICON_Y))
				pWsc->iconY = pXWMHints->icon_y;
			}
		    }
                }
#else /* WSM */
	        {
		    tmpIconX = (pCD->clientFlags & SM_ICON_X) ?
		      pCD->iconX : pXWMHints->icon_x;
		    tmpIconY = (pCD->clientFlags & SM_ICON_Y) ?
		      pCD->iconY : pXWMHints->icon_y;
		    pCD->iconPlace = 
			FindIconPlace (pCD, &(pCD->pSD->pActiveWS->IPData),
				       tmpIconX, tmpIconY);
		    if (pCD->iconPlace != NO_ICON_PLACE)
		    {
		        CvtIconPlaceToPosition (&(pCD->pSD->pActiveWS->IPData),
			    pCD->iconPlace, &pCD->iconX, &pCD->iconY);
		    }
	        }
	        else
	        {
		    if (!(pCD->clientFlags & SM_ICON_X))
			pCD->iconX = pXWMHints->icon_x;
		    if (!(pCD->clientFlags & SM_ICON_Y))
			pCD->iconY = pXWMHints->icon_y;
                }
#endif /* WSM */
	    }
	    else
	    {
	        if (wmGD.iconAutoPlace)
	        {
#ifdef WSM
		    /* 
		     * Initialize icon placement data in all inhabited
		     * workspaces
		     */
		    for (iws = 0; iws< pCD->numInhabited; iws++)
		    {
			pWsc = &(pCD->pWsList[iws]);
			pWsc->iconPlace = NO_ICON_PLACE;
			pWsc->iconX = 0;
			pWsc->iconY = 0;
		    }
#else /* WSM */
		    pCD->iconPlace = NO_ICON_PLACE;
		    pCD->iconX = 0;
		    pCD->iconY = 0;
#endif /* WSM */
	        }
	    }
	}

#ifdef NO_OL_COMPAT
        /*
         * Save the window group.
         */

        if (flags & WindowGroupHint)
	{
	    pCD->windowGroup = pXWMHints->window_group;
	}
	else
	{
	    pCD->windowGroup = 0L;
	}
#endif /* NO_OL_COMPAT */
    }
    else /* not the first time the hints are processed */
    {
	if (flags & IconPixmapHint)
	{
	    /*
             * Process an icon image change if the icon image was initially
             * set up with a client supplied icon image OR, if the client
             * now wants to supply an image.
             */
	    iconMask = (flags & IconMaskHint)?
                       pXWMHints->icon_mask : (Pixmap) NULL;

	    if ((iconPixmap = 
		 MakeClientIconPixmap (pCD, pXWMHints->icon_pixmap, 
					     iconMask)) != None)
	    {
		/*
		 * Made new icon image; free up the old image and display
		 * the new image.
		 */
		if (pCD->iconFlags & ICON_HINTS_PIXMAP)
                {
                    /*
                     * ICON_HINTS_PIXMAP was set either initally or
                     * below because a new pixmap was made for the client.
                     * It is now safe to free the previous pixmap since it
                     * is not the shared default iconPixmap
                     */
                    if (pCD->iconPixmap)
                    {
                        XFreePixmap (DISPLAY, pCD->iconPixmap);
                    }
                }
                else
                {
                    pCD->iconFlags |= ICON_HINTS_PIXMAP;
                }
		
                pCD->iconPixmap = iconPixmap;
		
                /*
                 * Display new icon image if the icon is showing:
                 */
		
                if (((pCD->clientState == MINIMIZED_STATE) ||
                     ((pCD->pSD->useIconBox) && (P_ICON_BOX(pCD)))) &&
                    ICON_FRAME_WIN(pCD))
                {
                    IconExposureProc (pCD, True);
                }
	    }
	}
    }

    if (pXWMHints)
    {
	XFree ((char*)pXWMHints);
    }


} /* END OF FUNCTION ProcessWmHints */



/*************************************<->*************************************
 *
 *  ProcessWmNormalHints (pCD, firstTime, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_NORMAL_HINTS property on
 *   the cient window.  There are several versions of the property that must be
 *  handled (currently R2 and CURRENT).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for the window with the property
 *
 *  firstTime = if True this is the first time the property has been processed
 *
 *  manageFlags = flags that indicate wm state information
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = client location and size fields set
 *
 *
 *  Comments:
 *  --------
 *  If the hints are being reprocessed (!firstTime) the configuration values
 *  will be ignored.  The size constraint values will be processed but the
 *  client configuration will not be changed even if it is not in line with
 *  the new values.  Reconfigurations subsequent to the hints changes will
 *  be done with the new constraints.
 *
 *************************************<->***********************************/

void 
ProcessWmNormalHints (ClientData *pCD, Boolean firstTime, long manageFlags)
{
    register SizeHints *pNormalHints;
    register long       flags;
    int                 diff;
    unsigned long       decoration;
    unsigned int        boxdim, tmpMin;
    unsigned int	oldWidthInc, oldHeightInc;
    unsigned int	oldBaseWidth, oldBaseHeight;
    unsigned int	incWidth, incHeight;

    /*
     * Use a custom verion of the Xlib routine to get WM_NORMAL_HINTS.
     * A custom version is necessary to handle the different versions
     * of WM_NORMAL_HINTS that may be encountered.  If the WM_NORMAL_HINTS
     * property does not exist the flags field will be set to 0.
     */

    pNormalHints = GetNormalHints (pCD);

    pCD->icccVersion = pNormalHints->icccVersion;


    /*
     * Parse the WM_NORMAL_HINTS information:
     */

    if (((flags = pNormalHints->flags) == 0) && !firstTime)
    {
	return;
    }


    /*
     * Process the size only if this is the first time the hints are
     * being processed for the window.
     */

    if (firstTime)
    {
        /*
         * Process client window size flags and information:
         */

        pCD->sizeFlags = flags & (US_POSITION | US_SIZE | P_POSITION | P_SIZE);

        /*
         * The R2 conventions and Xlib manual indicate that the window size
         * and position should be taken out of the WM_NORMAL_HINTS property
         * if they are specified there.  The current conventions indicate that
         * the size and position information should be gotten from the window
         * configuration. Mwm 1.1 always uses the current conventions.
         */

#ifdef R2_COMPAT
    /* 
     * Maintain R2 compatiblity code for CND product xnm
     */
        if ((pNormalHints->icccVersion == ICCC_R2) &&
            (flags & (US_POSITION | P_POSITION)) &&
	    !(manageFlags & MANAGEW_WM_RESTART))
        {
	    if (!(pCD->clientFlags & SM_X))
		pCD->clientX = pNormalHints->x;
	    if (!(pCD->clientFlags & SM_Y))
		pCD->clientY = pNormalHints->y;
        }
        else
        {
	    if (!(pCD->clientFlags & SM_X))
		pCD->clientX = wmGD.windowAttributes.x;
	    if (!(pCD->clientFlags & SM_Y))
		pCD->clientY = wmGD.windowAttributes.y;
        }
#else /* R2_COMPAT */
	if (!(pCD->clientFlags & SM_X))
	    pCD->clientX = wmGD.windowAttributes.x;
	if (!(pCD->clientFlags & SM_Y))
	    pCD->clientY = wmGD.windowAttributes.y;
#endif  /* R2_COMPAT */

	/*
	 * Use current conventions for initial window dimensions.
	 */

#ifdef R2_COMPAT
    /* 
     * Maintain R2 compatiblity code for CND product xnm
     */
	if ((pNormalHints->icccVersion == ICCC_R2) &&
	    (flags & (US_SIZE | P_SIZE)) &&
	    !(manageFlags & MANAGEW_WM_RESTART))
	{
	    if (!(pCD->clientFlags & SM_WIDTH))
		pCD->clientWidth = pNormalHints->width;
	    if (!(pCD->clientFlags & SM_HEIGHT))
		pCD->clientHeight = pNormalHints->height;
	}
	else
	{
	    if (!(pCD->clientFlags & SM_WIDTH))
		pCD->clientWidth = wmGD.windowAttributes.width;
	    if (!(pCD->clientFlags & SM_HEIGHT))
		pCD->clientHeight = wmGD.windowAttributes.height;
	}
#else /* R2_COMPAT */
	if (!(pCD->clientFlags & SM_WIDTH))
	    pCD->clientWidth = wmGD.windowAttributes.width;
	if (!(pCD->clientFlags & SM_HEIGHT))
	    pCD->clientHeight = wmGD.windowAttributes.height;
#endif /* R2_COMPAT */
    }

    /*
     * Process the minimum size:
     */

    if (flags & P_MIN_SIZE)
    {
	pCD->minWidth =
		 (pNormalHints->min_width < 0) ? 0 : pNormalHints->min_width;
	pCD->minHeight =
		 (pNormalHints->min_height < 0) ? 0 : pNormalHints->min_height;
	if (pCD->minWidth > MAX_MAX_SIZE(pCD).width)
	{
	    pCD->minWidth = MAX_MAX_SIZE(pCD).width;
	}
	if (pCD->minHeight > MAX_MAX_SIZE(pCD).height)
	{
	    pCD->minHeight = MAX_MAX_SIZE(pCD).height;
	}
    }
    else if (firstTime)
    {
	pCD->minWidth = 0;
	pCD->minHeight = 0;
    }


    /*
     * Process the resizing increments:
     */

    if (!firstTime)
    {
	oldWidthInc = (pCD->widthInc == 0) ? 1 : pCD->widthInc;
	oldHeightInc = (pCD->heightInc == 0) ? 1 : pCD->heightInc;
    }

    if (flags & P_RESIZE_INC)
    {
	pCD->widthInc =
		 (pNormalHints->width_inc < 1) ? 1 : pNormalHints->width_inc;
	pCD->heightInc =
		 (pNormalHints->height_inc < 1) ? 1 : pNormalHints->height_inc;
    }
    else if (firstTime)
    {
	pCD->widthInc = 1;
	pCD->heightInc = 1;
    }


    /*
     * Process the base size:
     */

    if (!firstTime)
    {
	oldBaseWidth = pCD->baseWidth;
	oldBaseHeight = pCD->baseHeight;
    }

    if (flags & P_BASE_SIZE)
    {
	pCD->baseWidth =
		(pNormalHints->base_width < 0) ? 0 : pNormalHints->base_width;
	pCD->baseHeight =
		(pNormalHints->base_height < 0) ? 0 : pNormalHints->base_height;
    }
    else if ((pNormalHints->icccVersion == ICCC_R2) &&
	     ((firstTime) ||
	      (!firstTime && (flags & P_MIN_SIZE))))
    {
	/*
	 * In this version of the hints the minimum size was effectively
	 * the base size.
	 */
	pCD->baseWidth = pCD->minWidth;
	pCD->baseHeight = pCD->minHeight;
    }
    else if (firstTime)
    {
        if (flags & P_MIN_SIZE)
        {
            pCD->baseWidth = pCD->minWidth;
            pCD->baseHeight = pCD->minHeight;
        }
        else
        {
            pCD->baseWidth = 0;
            pCD->baseHeight = 0;
        }
    }

    if (firstTime)
    {
	if (pCD->clientFlags & SM_WIDTH)
	{
	    pCD->clientWidth = ((pCD->clientWidth * pCD->widthInc) +
				pCD->baseWidth);
	}
	if (pCD->clientFlags & SM_HEIGHT)
	{
    	    pCD->clientHeight =((pCD->clientHeight * pCD->heightInc) +
				pCD->baseHeight);
	}
    }

    /*
     * Process the maximum width.  NOTE: maximumClientSize.width
     * and maximumClientSize.height will be set to BIGSIZE if
     * maximumClientSize is either set to 'horizontal' or 'vertical'.
     */

    pCD->oldMaxWidth = pCD->maxWidth;
    if (pCD->maximumClientSize.width)
    {
	/* If maximumClientSize is full 'horizontal' */
	if (IS_MAXIMIZE_HORIZONTAL(pCD))
	{
	    /* go to min (full screen width, max maximum width) */
	    pCD->maxWidth = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
				          (2 * pCD->clientOffset.x);

	    /*
	     * Hack to set max client to the current client height, maxHeight
	     * will be kept up to date whenever the window is reconfigured
	     */
	    pCD->maxHeight = pCD->clientHeight;

	}
	else
	{
	    pCD->maxWidth = (pCD->maximumClientSize.width * 
			     pCD->widthInc) + pCD->baseWidth;
	}
    }
    else
    {
	if (flags & P_MAX_SIZE)
	{
	    if (pNormalHints->max_width < 0)
	    {
	        /* go to min (full screen width, max maximum width) */
		pCD->maxWidth = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
				          (2 * pCD->clientOffset.x);
	    }
	    else
	    {
	        pCD->maxWidth = pNormalHints->max_width;
	    }
	}
	/* Don't reset maxWidth if it has been set earlier */
	else if (!IS_MAXIMIZE_VERTICAL(pCD))
	{
	    if (firstTime)
	    {
		/* go to min (full screen width, max maximum width) */
		pCD->maxWidth = DisplayWidth (DISPLAY,
					      SCREEN_FOR_CLIENT(pCD)) -
						(2 * pCD->clientOffset.x);
	    }
	    else
	    {
		/* reset the maxHeight before further processing */
		pCD->maxWidth = pCD->maxWidthLimit;
	    }
	}
	else
	{
	    /* 
	     * If the hints changed we need to adjust the maximum
	     * size (if not specified in the hints).
	     */
	    if (!firstTime &&
		((oldBaseWidth != pCD->baseWidth) ||
		 (oldWidthInc != pCD->widthInc)))
	    {
		incWidth = (pCD->maxWidth - oldBaseWidth) / oldWidthInc;
		pCD->maxWidth =  
		    (incWidth * pCD->widthInc) + pCD->baseWidth;
	    }
	    else
	    {
		/* reset the maxHeight before further processing */
		pCD->maxWidth = pCD->maxWidthLimit;
	    }
	}
	if (pCD->maxWidth > MAX_MAX_SIZE(pCD).width)
	{
	    pCD->maxWidth = MAX_MAX_SIZE(pCD).width;
	}
    }



    /*
     * Process the maximum height.
     */

    pCD->oldMaxHeight = pCD->maxHeight;
    if (pCD->maximumClientSize.height)
    {
	/* If maximumClientSize is full 'vertical' */
	if (IS_MAXIMIZE_VERTICAL(pCD))
	{
	    /* go to min (full screen height, max maximum height) */
	    pCD->maxHeight = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
			     (pCD->clientOffset.x +
			      pCD->clientOffset.y);
	    /*
	     * Hack to set max client to the current client width, maxWidth
	     * will be kept up to date whenever the window is reconfigured
	     */
	    pCD->maxWidth = pCD->clientWidth;

	}
	else
	{
	    pCD->maxHeight = (pCD->maximumClientSize.height *
			      pCD->heightInc) + pCD->baseHeight;
	}
    }
    else
    {
	if (flags & P_MAX_SIZE)
	{
	    if (pNormalHints->max_height < 0)
	    {
	        /* go to min (full screen height, max maximum height) */
	        pCD->maxHeight = DisplayHeight (
				 DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
				 (pCD->clientOffset.x +
				  pCD->clientOffset.y);
	    }
	    else
	    {
	        pCD->maxHeight = pNormalHints->max_height;
	    }
	}
	/* Don't reset maxHeight if it has been set above */
	else if (!IS_MAXIMIZE_HORIZONTAL(pCD))
	{
	    if (firstTime)
	    {
		/* go to min (full screen height, max maximum height) */
		pCD->maxHeight = DisplayHeight (DISPLAY,
						SCREEN_FOR_CLIENT(pCD)) -
						  (pCD->clientOffset.x +
						   pCD->clientOffset.y);
	    }
	    else
	    {
		/* reset the maxHeight before further processing */
		pCD->maxHeight = pCD->maxHeightLimit;
	    }
	}
	else
	{
	    /* 
	     * If the hints changed we need to adjust the maximum
	     * size (if not specified in the hints).
	     */
	    if (!firstTime &&
		((oldBaseHeight != pCD->baseHeight) ||
		 (oldHeightInc != pCD->heightInc)))
	    {
		incHeight = (pCD->maxHeight - oldBaseHeight) / oldHeightInc;
		pCD->maxHeight =  
		    (incHeight * pCD->heightInc) + pCD->baseHeight;
	    }
	    else
	    {
		/* reset the maxHeight before further processing */
		pCD->maxHeight = pCD->maxHeightLimit;
	    }
	}
	if (pCD->maxHeight > MAX_MAX_SIZE(pCD).height)
	{
	    pCD->maxHeight = MAX_MAX_SIZE(pCD).height;
	}
    }

    /*
     * Make sure not to exceed the maximumMaximumSize (width and height)
     */

    if (pCD->maxWidth > MAX_MAX_SIZE(pCD).width)
    {
	pCD->maxWidth = MAX_MAX_SIZE(pCD).width;
    }

    if (pCD->maxHeight > MAX_MAX_SIZE(pCD).height)
    {
	pCD->maxHeight = MAX_MAX_SIZE(pCD).height;
    }

    /*
     * Get the initial aspect ratios, if available.  Only use them if:
     *
     *   minAspect.y > 0
     *   maxAspect.y > 0
     *   0 <= minAspect.x / minAspect.y <= maxAspect.x / maxAspect.y 
     */

    if (flags & P_ASPECT)
    {
	pCD->minAspect.x = pNormalHints->min_aspect.x;
	pCD->minAspect.y = pNormalHints->min_aspect.y;
	pCD->maxAspect.x = pNormalHints->max_aspect.x;
	pCD->maxAspect.y = pNormalHints->max_aspect.y;

        if (pCD->minAspect.y > 0 &&
	    pCD->maxAspect.y > 0 &&
	    pCD->minAspect.x > 0 &&
	    pCD->maxAspect.x > 0 &&
	    (pCD->minAspect.x * pCD->maxAspect.y <=
	    pCD->maxAspect.x * pCD->minAspect.y))
        {
	    pCD->sizeFlags |= P_ASPECT;
	}
	else
	{
	    pCD->sizeFlags &= ~P_ASPECT;
	}
    }

    /* compute for minimum frame size */
    if ((decoration = pCD->decor) & MWM_DECOR_TITLE)
    {
	boxdim = TitleBarHeight(pCD);
	tmpMin = boxdim +
	         ((decoration & MWM_DECOR_MENU) ? boxdim : 0) +
	         ((decoration & MWM_DECOR_MINIMIZE) ? boxdim : 0) +
	         ((decoration & MWM_DECOR_MAXIMIZE) ? boxdim : 0) -
		 2*(pCD->matteWidth);
    }
    else {
	tmpMin = 0;
    }


    /*
     * Process the window gravity (for positioning):
     */

    if (flags & P_WIN_GRAVITY)
    {
	pCD->windowGravity = pNormalHints->win_gravity;
    }
    else
    {
        if (pNormalHints->icccVersion == ICCC_R2)
	{
	    pCD->windowGravity = wmGD.windowAttributes.win_gravity;
	}
	else
	{
	    pCD->windowGravity = NorthWestGravity;
	}
    }


    /*
     * Make sure that all the window sizing constraints are compatible:
     */

    /*
     * Make:
     *
     *   minWidth >= tmpMin
     *   minWidth >= max (baseWidth, widthInc) > 0
     *     & an integral number of widthInc from baseWidth.
     *   minHeight >= max (baseHeight, heightInc) > 0
     *     & an integral number of heightInc from baseHeight.
     */

    if (pCD->minWidth < tmpMin)
    {
        if ((diff = ((tmpMin - pCD->baseWidth)%pCD->widthInc)) != 0)
        {
	    pCD->minWidth = tmpMin + pCD->widthInc - diff;
        }
        else
        {
	    pCD->minWidth = tmpMin;
        }
    }

    if (pCD->minWidth < pCD->baseWidth)
    {
	pCD->minWidth = pCD->baseWidth;
    }

    if (pCD->minWidth == 0)
    {
	pCD->minWidth = pCD->widthInc;
    }
    else if ((diff = ((pCD->minWidth - pCD->baseWidth)%pCD->widthInc)) != 0)
    {
	pCD->minWidth += pCD->widthInc - diff;
    }

    if (pCD->minHeight < pCD->baseHeight)
    {
	pCD->minHeight = pCD->baseHeight;
    }

    if (pCD->minHeight == 0)
    {
	pCD->minHeight = pCD->heightInc;
    }
    else if ((diff = ((pCD->minHeight - pCD->baseHeight) % pCD->heightInc)) !=0)
    {
	pCD->minHeight += pCD->heightInc - diff;
    }

    /*
     * Make:
     *
     *   maxWidth >= minWidth 
     *     & an integral number of widthInc from baseWidth.
     *   maxHeight >= minHeight
     *     & an integral number of heightInc from baseHeight.
     */

    if (pCD->maxWidth < pCD->minWidth)
    {
	pCD->maxWidth = pCD->minWidth;
    }

    /*
     * Hack to use maxWidthLimit as the real maxWidth when maximumClientSize
     * set to 'vertical'.
     */
    if (IS_MAXIMIZE_VERTICAL(pCD))
    {
	/* go to min (full screen width, max maximum width) */
	pCD->maxWidthLimit = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
			    (2 * pCD->clientOffset.x);
    }
    else
    {
	pCD->maxWidthLimit = pCD->maxWidth;
    }

    pCD->maxWidth -= ((pCD->maxWidth - pCD->baseWidth) % pCD->widthInc);

    if (firstTime)
    {
	pCD->oldMaxWidth = pCD->maxWidth;
    }

    if (pCD->maxHeight < pCD->minHeight)
    {
	pCD->maxHeight = pCD->minHeight;
    }

    /*
     * Hack to use maxHeightLimit as the real maxHeight when maximumClientSize
     * set to 'horizontal'.
     */
    if (IS_MAXIMIZE_HORIZONTAL(pCD))
    {
	/* go to min (full screen height, max maximum height) */
	pCD->maxHeightLimit = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD)) -
			     (pCD->clientOffset.x +
			      pCD->clientOffset.y);
    }
    else
    {
	pCD->maxHeightLimit = pCD->maxHeight;
    }

    pCD->maxHeight -= ((pCD->maxHeight - pCD->baseHeight) % pCD->heightInc);

    if (firstTime)
    {
	pCD->oldMaxHeight = pCD->maxHeight;
    }

    if (!firstTime && pCD->maxConfig)
    {
	/* 
	 * If the hints changed while we were maximized then 
	 * we may need to adjust the normalized size of the window.
	 */
	if (!firstTime &&
	    ((oldBaseWidth != pCD->baseWidth) ||
	     (oldBaseHeight != pCD->baseHeight) ||
	     (oldWidthInc != pCD->widthInc) ||
	     (oldHeightInc != pCD->heightInc)))
	{
	    incWidth = (pCD->clientWidth - oldBaseWidth) / oldWidthInc;
	    incHeight = (pCD->clientHeight - oldBaseHeight) / oldHeightInc;
	    pCD->clientWidth =  
		(incWidth * pCD->widthInc) + pCD->baseWidth;
	    pCD->clientHeight =  
		(incHeight * pCD->heightInc) + pCD->baseHeight;
	}
    }

    /*
     * If using aspect ratios, make:
     *
     *  minWidth / maxHeight <= minAspect.x / minAspect.y 
     *                       <= maxAspect.x / maxAspect.y
     *                       <= maxWidth / minHeight
     */

    if (pCD->sizeFlags & P_ASPECT)
    {
        if (pCD->minWidth * pCD->minAspect.y >
	    pCD->minAspect.x * pCD->maxHeight)
        {
            pCD->minAspect.x = pCD->minWidth;
            pCD->minAspect.y = pCD->maxHeight;
	}

        if (pCD->maxAspect.x * pCD->minHeight >
	    pCD->maxWidth * pCD->maxAspect.y)
        {
            pCD->maxAspect.x = pCD->maxWidth;
            pCD->maxAspect.y = pCD->minHeight;
	}

        FixWindowSize (pCD, (unsigned int *) &(pCD->maxWidth),
	                    (unsigned int *) &(pCD->maxHeight),
			    (unsigned int) (pCD->widthInc), 
			    (unsigned int) (pCD->heightInc));
    }

    /*
     * If this is the first time, make sure the client dimensions are within 
     * range and that they satisfy any aspect ratio constraints:
     *
     *  0 < minWidth  <= clientWidth  <= maxWidth
     *  0 < minHeight <= clientHeight <= maxHeight
     *
     *  minAspect.x / minAspect.y <= clientWidth / clientHeight
     *                            <= maxAspect.x / maxAspect.y
     *
     * Initial max width/height are set to max of max size or normal
     * client size unless a maximumClientSize was specified.
     */

    if (firstTime)
    {
	if (!pCD->maximumClientSize.width)
	{
	    if (pCD->clientWidth > pCD->pSD->maximumMaximumSize.width)
	    {
		pCD->clientWidth = pCD->pSD->maximumMaximumSize.width;
	    }
	}

	if (!pCD->maximumClientSize.height)
	{
	    if (pCD->clientHeight > pCD->pSD->maximumMaximumSize.height)
	    {
		pCD->clientHeight = pCD->pSD->maximumMaximumSize.height;
	    }
	}

        FixWindowSize (pCD, (unsigned int *) &(pCD->clientWidth), 
	                    (unsigned int *) &(pCD->clientHeight),
			    (unsigned int) (pCD->widthInc), 
			    (unsigned int) (pCD->heightInc));
    }

} /* END OF FUNCTION ProcessWmNormalHints */


/*************************************<->*************************************
 *
 *  WmICCCMToXmString (wmNameProp)
 *
 *
 *  Description:
 *  -----------
 *  This function uses a property (WM_NAME or WM_ICON_NAME) that was
 *  retrieved from the window, and converts it to XmString.
 *
 *  Inputs:
 *  ------
 *  wmNameProp	- the text property
 * 
 *  Outputs:
 *  -------
 *  Return = new XmString, or NULL if the property didn't have a value.
 * 
 *************************************<->***********************************/

static XmString
WmICCCMToXmString (XTextProperty *wmNameProp)
{
  int status;
  XmString xms_return;
  XmStringTable xmsTable;
  int i, nStrings = -1;
  char msg[200];

  if (wmNameProp->value == 0 || strlen((char *)wmNameProp->value) == 0)
    {
      return (XmString)NULL;
    }

  if (((status = XmCvtTextPropertyToXmStringTable(DISPLAY, wmNameProp,
						  &xmsTable, &nStrings))
       != Success) || (nStrings <= 0))
  {
      switch (status)
      {
      case XConverterNotFound:
#ifndef MOTIF_ONE_DOT_ONE
	  sprintf(msg, GETMESSAGE (70,5,
		    "Window manager cannot convert property %.100s as clientTitle/iconTitle: XmbTextPropertyToTextList"), 
		  XGetAtomName (DISPLAY,wmNameProp->encoding));
	  Warning(msg);
#endif /* MOTIF_ONE_DOT_ONE */
	  break;

      case XNoMemory:
	  sprintf(msg, GETMESSAGE (70, 6, 
		    "insufficient memory to convert property %.100s as clientTitle/iconTitle: XmbTextPropertyToTextList"),
		  XGetAtomName(DISPLAY,wmNameProp->encoding));
	  Warning(msg);
	  break;

      case XLocaleNotSupported:
	  if ((wmNameProp->encoding == XA_STRING) ||
	      (wmNameProp->encoding == wmGD.xa_COMPOUND_TEXT))
	  {
	      sprintf(msg, LOCALE_MSG, setlocale(LC_ALL, NULL));
	  }
	  else
	  {
	      /* Atom was neither STRING nor COMPOUND_TEXT */
	      sprintf(msg, GETMESSAGE(70, 8, 
			"Window manager received unknown property as clientTitle/iconTitle: %.100s. Property ignored."),
		      XGetAtomName(DISPLAY, wmNameProp->encoding));
	  }
	  Warning(msg);
	  break;
      }

      /* Couldn't convert using Xm; apply a default */
      return XmCvtCTToXmString((char*)wmNameProp->value);
  }

  xms_return = xmsTable[0];
  for (i = 1; i < nStrings; i++)
  {
#ifdef CONCAT_TEXTLIST
      xms_return = XmStringConcatAndFree(xms_return, xmsTable[i]);
#else
      XmStringFree(xmsTable[i]);
#endif /* CONCAT_TEXTLIST */
  }
  XtFree((char *)xmsTable);

  return xms_return;
}


/*************************************<->*************************************
 *
 *  ProcessWmWindowTitle (pCD, firstTime)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_NAME property on the
 *  cient window.  A default name is set if the property does not exist.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data structure
 *  firstTime	- false if the window is already managed and the title
 *                is being changed.
 *
 * 
 *  Outputs:
 *  -------
 *  pCD		- clientTitle, iconTitle
 * 
 *************************************<->***********************************/

void 
ProcessWmWindowTitle (ClientData *pCD, Boolean firstTime)
{
    XTextProperty wmNameProp;
    XmString title_xms = NULL;

    if ((pCD->clientDecoration & MWM_DECOR_TITLE) &&
#ifdef WSM
	(!firstTime || HasProperty (pCD, XA_WM_NAME)) &&
#endif /* WSM */
	XGetWMName(DISPLAY, pCD->client, &wmNameProp))
    {
      title_xms = WmICCCMToXmString(&wmNameProp);
      if (wmNameProp.value)
	XFree ((char*)wmNameProp.value);
    }

    if (title_xms)
    {
      if (!firstTime && (pCD->iconTitle == pCD->clientTitle))
      {
	/*
	 * The client window title is being used for the icon title so
	 * change the icon title with the window title.
	 */
	pCD->iconTitle = title_xms;
	RedisplayIconTitle (pCD);
      }

      if ((pCD->clientFlags & CLIENT_HINTS_TITLE) &&
	  pCD->clientTitle != wmGD.clientDefaultTitle)
      {
	XmStringFree (pCD->clientTitle);
      }

      pCD->clientTitle = title_xms;
      pCD->clientFlags |= CLIENT_HINTS_TITLE;

      if (!firstTime)
      {
	DrawWindowTitle (pCD, True);
      }
    }
    /*
     * The client frame does not have a place to put the title or the WM_NAME
     * property does not exist or there was some error in getting
     * the property information, so use a default value.
     */
    else if (firstTime)
    {
	if (pCD->clientName)
        {
	    pCD->clientTitle = XmStringCreateLocalized(pCD->clientName);
        }
        else
        {
	    pCD->clientTitle = wmGD.clientDefaultTitle;
        }
    }

    /*
     * If this is a tear-off menu, then make sure title text is not clipped
     */

#ifdef PANELIST
    if ((pCD->window_status & MWM_TEAROFF_WINDOW) ||
        (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL))
#else /* PANELIST */
    if (pCD->window_status & MWM_TEAROFF_WINDOW)
#endif /* PANELIST */
    {
	unsigned int boxdim = TitleBarHeight (pCD);
	unsigned long decor = pCD->decor;
	XmFontList  fontList;
	int minWidth;

	if (DECOUPLE_TITLE_APPEARANCE(pCD))
	    fontList = CLIENT_TITLE_APPEARANCE(pCD).fontList;
	else
	    fontList = CLIENT_APPEARANCE(pCD).fontList;

	/*
	 * Calculations derived from GetTextBox() and GetFramePartInfo()
	 */
	minWidth = XmStringWidth(fontList, pCD->clientTitle) +
#ifdef PANELIST
	    ((pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL) ? 4 : 0) +
#endif /* PANELIST */
			    ((decor & MWM_DECOR_MENU) ? boxdim : 0) +
			    ((decor & MWM_DECOR_MINIMIZE) ? boxdim : 0) +
			    ((decor & MWM_DECOR_MAXIMIZE) ? boxdim : 0) +
			      WM_TOP_TITLE_SHADOW + WM_BOTTOM_TITLE_SHADOW +
				WM_TOP_TITLE_PADDING + WM_BOTTOM_TITLE_PADDING;

	if (minWidth > pCD->minWidth)
	{
	    pCD->minWidth = minWidth;
	}
#ifdef PANELIST
	if ((pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL) &&
            (pCD->clientWidth < pCD->minWidth))
	{
	    FixSubpanelEmbeddedClientGeometry (pCD);
	}
#endif /* PANELIST */
    }

} /* END OF FUNCTION ProcessWmWindowTitle */

#ifdef PANELIST

/*************************************<->*************************************
 *
 *  FixSubpanelEmbeddedClientGeometry ( pCD )
 *
 *
 *  Description:
 *  -----------
 *  This function adjusts the embedded clients in a subpanel if the 
 *  geometry of the subpanel is adjusted.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data structure
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comment:
 *  -------
 *  Only handles change in width right now.
 *
 *************************************<->***********************************/

static void 
FixSubpanelEmbeddedClientGeometry (ClientData *pCD)
{
    WmScreenData *pSD = PSD_FOR_CLIENT(pCD);
    Widget wSubpanel;
    Arg    al[5];
    int    ac;

    /*
     * Get the widget for the subpanel
     */
    wSubpanel = WmPanelistWindowToSubpanel (DISPLAY1, pCD->client);

    if (pSD->wPanelist && wSubpanel)
    {
	WmFpEmbeddedClientData  *pECD; 
	int i;

	/*
	 * set new shell width to minimum width
	 */
	if (pCD->clientWidth < pCD->minWidth)
	{
	    ac = 0;
	    XtSetArg (al[ac], XmNwidth, pCD->minWidth);	ac++;
	    XtSetValues (wSubpanel, al, ac);
	}

	/*
	 * Cause update of client geometries.
	 */
/*	WmPanelistSetClientGeometry (pSD->wPanelist);   */

	/*
	 * Update all affected reparented controls.
	 */

	for (i=0; i<pSD->numEmbeddedClients; i++)
	{
	    pECD =  &(((WmFpEmbeddedClientData *) pSD->pECD)[i]);

	    if (pECD->pCD)
	    {
		ClientData *pCD2 = pECD->pCD;

		if ((pCD2->clientWidth !=  pECD->width) ||
		    (pCD2->clientHeight != pECD->height) ||
		    (pCD2->clientX != pECD->x) ||
		    (pCD2->clientY != pECD->y))
		{
		    pCD2->clientX = pECD->x;
		    pCD2->clientY = pECD->y;
		    pCD2->clientWidth = pECD->width;
		    pCD2->clientHeight = pECD->height;

		    XMoveResizeWindow (DISPLAY1, pCD2->client, 
			pECD->x, pECD->y, pECD->width, pECD->height);
		}
	    }
	}
    }
} /* END OF FUNCTION FixEmbeddedClientGeometry */

#endif /* PANELIST */


/*************************************<->*************************************
 *
 *  ProcessWmIconTitle (pCD, firstTime)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_ICON_NAME property on the
 *  cient window.  The value of the property is a string that is used for the
 *  icon title.  A default title is set if the property does not exist.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data structure
 *
 *  firstTime	- false if the window is already managed and the title
 *                is being changed.
 *
 * 
 *  Outputs:
 *  -------
 *  pCD		- iconTitle
 * 
 *************************************<->***********************************/

void 
ProcessWmIconTitle (ClientData *pCD, Boolean firstTime)
{
  XTextProperty wmIconNameProp;
  XmString icon_xms = NULL;

  if ((pCD->clientFunctions & MWM_FUNC_MINIMIZE) &&
      (pCD->transientLeader == NULL) &&
#ifdef WSM
      (!firstTime || HasProperty(pCD, XA_WM_ICON_NAME)) &&
#endif /* WSM */
      XGetWMIconName (DISPLAY, pCD->client, &wmIconNameProp))
  {
    icon_xms = WmICCCMToXmString(&wmIconNameProp);
    if (wmIconNameProp.value)
      XFree ((char*)wmIconNameProp.value);
  }

  if (icon_xms)
  {
    if ((pCD->iconFlags & ICON_HINTS_TITLE) &&
	pCD->iconTitle != wmGD.iconDefaultTitle)
    {
      XmStringFree (pCD->iconTitle);
    }

    pCD->iconTitle = icon_xms;
    pCD->iconFlags |= ICON_HINTS_TITLE;

    if (!firstTime)
    {
      RedisplayIconTitle (pCD);
    }
  }
  /*
   * The WM_ICON_NAME property does not exist (or there was some error
   * in getting * the property information), so use a default value.
   */
  else if (firstTime)
  {
    if (pCD->clientTitle && (pCD->clientTitle != wmGD.clientDefaultTitle))
    {
      pCD->iconTitle = pCD->clientTitle;
    }
    else
    {
      pCD->iconTitle = wmGD.iconDefaultTitle;
    }
  }

} /* END OF FUNCTION ProcessWmIconTitle */



/*************************************<->*************************************
 *
 *  ProcessWmTransientFor (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function retrieves the contents of the WM_TRANSIENT_FOR property on
 *  the cient window.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data structure for the window with the property
 *
 * 
 *  Outputs:
 *  -------
 *  pCD.transientFor = if tranient then this is the associated main window
 *
 *  pCD.clientFlags = indicate that this is a transient window
 * 
 *************************************<->***********************************/

void 
ProcessWmTransientFor (ClientData *pCD)
{
    Window window;
    ClientData *leader;


#ifdef WSM
    if ((HasProperty (pCD, XA_WM_TRANSIENT_FOR)) &&
	(XGetTransientForHint (DISPLAY, pCD->client, &window)))
#else /* WSM */
    if (XGetTransientForHint (DISPLAY, pCD->client, &window))
#endif /* WSM */
    {
	pCD->clientFlags |= CLIENT_TRANSIENT;

	/*
	 * Only save the (leader) transientFor window if it is NOT the
	 * client window and it is already managed by the window manager.
	 */

	if ((pCD->client != window) &&
	    !XFindContext (DISPLAY, window, wmGD.windowContextType,
		(caddr_t *)&leader))
	{
	    pCD->transientFor = window;
	    pCD->transientLeader = leader;
	}
    }
    else {    /* else this is not a transient window */
	pCD->clientFlags &= ~CLIENT_TRANSIENT;
	pCD->transientFor = (Window)0L;
	pCD->transientLeader = NULL;
    }


} /* END OF FUNCTION ProcessWmTransientFor */



/*************************************<->*************************************
 *
 *  MakeSystemMenu (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function finds or makes a system menu for the client.  A check
 *  is made for the _MWM_MENU property and, if present, client-specific
 *  items are added to the custom system menu.  Any custom system menu
 *  must be destroyed when the client is unmanaged (or killed).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data structure for the managed window
 *
 * 
 *  Outputs:
 *  -------
 *  pCD.systemMenuSpec = system menu specification for the client, not added
 *                       to wmGD.acceleratorMenuSpecs 
 *
 *************************************<->***********************************/

void 
MakeSystemMenu (ClientData *pCD)
{
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    MenuItem *lastItem;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

    pCD->mwmMenuItems = GetMwmMenuItems(pCD);
    pCD->systemMenuSpec = 
       MAKE_MENU (PSD_FOR_CLIENT(pCD), pCD, pCD->systemMenu, F_CONTEXT_WINDOW,
	         F_CONTEXT_WINDOW|F_CONTEXT_ICON, pCD->mwmMenuItems, TRUE);

#ifdef NO_MESSAGE_CATALOG
    if (pCD->systemMenuSpec == NULL)
    {
        /*
	 * As the lookup has failed, let's try just one more time.
         */
	Warning("Retrying - using builtin window menu\n");

	pCD->systemMenuSpec =
	  MAKE_MENU(PSD_FOR_CLIENT(pCD), pCD, builtinSystemMenuName,
		    F_CONTEXT_WINDOW,
		    F_CONTEXT_WINDOW|F_CONTEXT_ICON, pCD->mwmMenuItems, TRUE);
    }
#endif

#if defined(MWM_QATS_PROTOCOL)
    /* Added to fix CDExc23338
     * Not sure what the MWM_QATS_PROTOCOL is trying to accomplish here,
     * but this code is causing the system menu to loose it's default
     * actions whenever client defined actions are added.  I thought
     * it prudent to minimize the changes.  It could be that the
     * #if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
     * should be
     * #if ((!defined(WSM)) && defined(MWM_QATS_PROTOCOL))
     * throughout the wm code, but I am loath to make such a change
     * without any documentation.
     */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    /** BEGIN FIX CR 6941 **/

    /* if we still don't have a menu spec, then just abort. */
    if (pCD->systemMenuSpec == NULL)
      return;

    pCD->systemMenuSpec = DuplicateMenuSpec(pCD->systemMenuSpec);
    XtFree(pCD->systemMenuSpec->name);
    pCD->systemMenuSpec->name = XtNewString("ProtocolsMenu");

    /* Find the last menu item in the menu spec's list. */
    for (lastItem = pCD->systemMenuSpec->menuItems;
	 lastItem->nextMenuItem != (MenuItem *) NULL;
	 lastItem = lastItem->nextMenuItem)
      /*EMPTY*/;
    lastItem->nextMenuItem = pCD->mwmMenuItems;

    /* Now recreate the menu widgets since we've appended the 
       protocol menu items */
    DestroyMenuSpecWidgets(pCD->systemMenuSpec);
    pCD->systemMenuSpec->menuWidget =
      CreateMenuWidget (PSD_FOR_CLIENT(pCD), pCD, "ProtocolsMenu",
			PSD_FOR_CLIENT(pCD)->screenTopLevelW, TRUE,
			pCD->systemMenuSpec, NULL);
    /** END FIX CR 6941 **/
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
#endif /* defined(MWM_QATS_PROTOCOL) */

} /* END OF FUNCTION MakeSystemMenu */



/*************************************<->*************************************
 *
 *  InitCColormapData (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function initializes colormap data for the client window that is
 *  by the window manager in maintaining the colormap focus.  This may
 *  involve retrieving and processing properties that deal with subwindow
 *  colormaps.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to the client data structure for the managed window
 *
 * 
 *  Outputs:
 *  -------
 *  pCD.clientColormap = client colormap to be installed when the client
 *                       window gets the colormap focus
 *
 *  pCD = (cmapWindows, clientCmapList, clientCmapCount, clientCmapIndex)
 *
 *************************************<->***********************************/

void 
InitCColormapData (ClientData *pCD)
{

    if (wmGD.windowAttributes.colormap == None)
    {
	pCD->clientColormap = WORKSPACE_COLORMAP(pCD);
    }
    else
    {
	pCD->clientColormap = wmGD.windowAttributes.colormap;
    }

    /*
     * Process subwindow colormap windows if they are specified.
     */

    ProcessWmColormapWindows (pCD);
    

} /* END OF FUNCTION InitCColormapData */



/*************************************<->*************************************
 *
 *  CalculateGravityOffset (pCD, xoff, yoff)
 *
 *
 *  Description:
 *  -----------
 *  This function calculates the window offsets based on the window gravity
 *  and the window frame client offset.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (client window configuration fields)
 *  xoff = pointer to xoffset
 *  yoff = pointer to yoffset
 *
 * 
 *  Outputs:
 *  -------
 *  xoff = pointer to xoffset set
 *  yoff = pointer to yoffset set
 * 
 *************************************<->***********************************/

void
CalculateGravityOffset (ClientData *pCD, int *xoff, int *yoff)
{
    int borderWidth = pCD->xBorderWidth;

    if (pCD->windowGravity < ForgetGravity ||
	pCD->windowGravity > StaticGravity)
    {
	*xoff = 0;
	*yoff = 0;
    }
    else
    {
	switch (pCD->windowGravity)
	{
	    case NorthWestGravity:
	    default:
	    {
		*xoff = pCD->clientOffset.x;
		*yoff = pCD->clientOffset.y;
		break;
	    }

	    case NorthGravity:
	    {
		*xoff = borderWidth;
		*yoff = pCD->clientOffset.y;
		break;
	    }
                
	    case NorthEastGravity:
	    {
		*xoff = -(pCD->clientOffset.x - (2 * borderWidth));
		*yoff = pCD->clientOffset.y;
		break;
	    }

	    case EastGravity:
	    {
		*xoff = -(pCD->clientOffset.x - (2 * borderWidth));
		*yoff = borderWidth +
				(pCD->clientOffset.y - pCD->clientOffset.x)/2;
		break;
	    }

	    case SouthEastGravity:
	    {
		*xoff = -(pCD->clientOffset.x - (2 * borderWidth));
		*yoff = -(pCD->clientOffset.x - (2 * borderWidth));
		break;
	    }

	    case SouthGravity:
	    {
		*xoff = borderWidth;
		*yoff = -(pCD->clientOffset.x - (2 * borderWidth));
		break;
	    }

	    case SouthWestGravity:
	    {
		*xoff = pCD->clientOffset.x;
		*yoff = -(pCD->clientOffset.x - (2 * borderWidth));
		break;
	    }

	    case WestGravity:
	    {
		*xoff = pCD->clientOffset.x;
		*yoff = borderWidth +
				(pCD->clientOffset.y - pCD->clientOffset.x)/2;
		break;
	    }

	    case CenterGravity:
	    {
		*xoff = 0;
		*yoff = 0;
		break;
	    }
	}
    }
} /* END OF FUNCTION CalculateGravityOffset */



/*************************************<->*************************************
 *
 *  InitClientPlacement (pCD, manageFlags)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the initial client window placement (for both
 *  the normal and maximized state).
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (client window configuration fields)
 *
 *  manageFlags = flags that indicate wm state information (e.g. whether
 *                the window manager is starting up or restarting)
 *
 * 
 *  Outputs:
 *  -------
 *  Return = True if position changed by this routine.
 *  pCD = changes to the client window configuration fields
 * 
 *************************************<->***********************************/

Boolean 
InitClientPlacement (ClientData *pCD, long manageFlags)
{
    Boolean interactivelyPlaced = False;
    Boolean autoPlaced = False;
    Boolean rval = False;
    int xoff, yoff;
    int origX, origY, origWidth, origHeight;
#ifdef WSM
    int iwsc;
#endif /* WSM */


    /*
     * Save initial client values
     */
    origX = pCD->clientX;
    origY = pCD->clientY;
    origWidth = pCD->clientWidth;
    origHeight = pCD->clientHeight;

    /*
     * Do interactive placement if...
     *     + the resource is turned on 
     *     + the window's coming up on the active screen
     *
     * Don't do it if...
     *     + position specified in DB or by Session Manager
     *     + the user has specified a position
     *     + the window is coming up iconic 
     *     + the window is transient
     *     + we're system modal
     */

    if (wmGD.interactivePlacement && 
	(!(pCD->clientFlags & (SM_X | SM_Y))) &&
	!(pCD->sizeFlags & US_POSITION) &&
	(pCD->clientState != MINIMIZED_STATE) &&
        (manageFlags == MANAGEW_NORMAL) &&
	!(pCD->clientFlags & CLIENT_TRANSIENT) &&
	(pCD->inputMode != MWM_INPUT_SYSTEM_MODAL) &&
#ifdef WSM
	(ClientInWorkspace(PSD_FOR_CLIENT(pCD)->pActiveWS, pCD)))
#else /* WSM */
	(PSD_FOR_CLIENT(pCD) == ACTIVE_PSD))
#endif /* WSM */
    {
	/*
	 * Interactively place the window on the screen.
	 */
	interactivelyPlaced = True;
	PlaceWindowInteractively (pCD);
    }


    /*
     * Check out the configuration values to insure that they are within
     * the constraints.
     */

    FixWindowConfiguration (pCD, (unsigned int *) &(pCD->clientWidth), 
	                         (unsigned int *) &(pCD->clientHeight),
				 (unsigned int) (pCD->widthInc), 
	                         (unsigned int) (pCD->heightInc));

    /*
     * Do autoplacement of the client window if appropriate.
     */

    if ((manageFlags == MANAGEW_NORMAL) && !interactivelyPlaced &&
	(!(pCD->clientFlags & (SM_X | SM_Y))) &&
	!(pCD->sizeFlags & US_POSITION) &&
	!(pCD->clientFlags & CLIENT_TRANSIENT) &&
	(pCD->inputMode != MWM_INPUT_SYSTEM_MODAL) && wmGD.clientAutoPlace)
    {
	/*
	 * if (PPosition is on or nonzero), then use current value for
	 * clientX and clientY which was set to windowAttributes.x,y
	 * by ProcessWmNormalHints(), else autoplace client.
	 */

	if ((pCD->sizeFlags & P_POSITION) &&
	    ((pCD->usePPosition == USE_PPOSITION_ON) ||
	     ((pCD->usePPosition == USE_PPOSITION_NONZERO) &&
	      ((pCD->clientX != 0) || (pCD->clientY != 0)))))
	{
	    /* do nothing */
	}
	else
	{
	    FindClientPlacement (pCD);
	    autoPlaced = True;
	}
    }

    /*
     * Do PositionIsFrame processing:
     * Use window gravity to allow the user to specify the window 
     * position on the screen  without having to know the dimensions 
     * of the decoration that mwm is adding.
     */

    if ((wmGD.positionIsFrame) &&
	!interactivelyPlaced && !autoPlaced)
    {
	CalculateGravityOffset (pCD, &xoff, &yoff);
	if (!(pCD->clientFlags & SM_X))
	    pCD->clientX += xoff;
	if (!(pCD->clientFlags & SM_Y))
	    pCD->clientY += yoff;
    }


    /*
     * Do PositionOnScreen processing:
     */


#ifdef WSM
#ifdef PANELIST
    if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)
    {
	if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUB_RESTORED)
	{
	    SetFrameInfo (pCD);   
	}
	else
	{
	    AdjustSlideOutGeometry (pCD);
	}
    }
    else
#endif /* PANELIST */
#endif /* WSM */
    if (((wmGD.positionOnScreen) && !interactivelyPlaced) &&
	(!(pCD->clientFlags & (SM_X | SM_Y))))
    {
	PlaceFrameOnScreen (pCD, &pCD->clientX, &pCD->clientY,
	    pCD->clientWidth, pCD->clientHeight);
    }


    /*
     * Position the maximized frame:
     */

    pCD->maxX = pCD->clientX;
    pCD->maxY = pCD->clientY;
    PlaceFrameOnScreen (pCD, &pCD->maxX, &pCD->maxY, pCD->maxWidth,
	pCD->maxHeight);


    if (!wmGD.iconAutoPlace)
    {
#ifdef WSM
	if (!(pCD->iconFlags & ICON_HINTS_POSITION))
	{
	    for (iwsc=0; iwsc<pCD->numInhabited; iwsc++)
	    {
		pCD->pWsList[iwsc].iconX = pCD->clientX;
		pCD->pWsList[iwsc].iconY = pCD->clientY;
		PlaceIconOnScreen (pCD, &pCD->pWsList[iwsc].iconX, 
					&pCD->pWsList[iwsc].iconY);
	    }
	}
#else /* WSM */
	if (!(pCD->iconFlags & ICON_HINTS_POSITION))
	{
	    pCD->iconX = pCD->clientX;
	    pCD->iconY = pCD->clientY;
	}
	PlaceIconOnScreen (pCD, &pCD->iconX, &pCD->iconY);
#endif /* WSM */
    }

    /* 
     * if client size or position has been changed by this routine,
     * then indicate in return value
     */
    if ((origX != pCD->clientX) || (origY != pCD->clientY) ||
	(origWidth != pCD->clientWidth) || (origHeight != pCD->clientHeight))
    {
	rval = True;
    }

    return (rval);

} /* END OF FUNCTION InitClientPlacement */

#ifdef PANELIST

/******************************<->*************************************
 *
 * void AdjustSlideOutGeometry (pCD)
 *
 *  Description:
 *  -----------
 *  Adjusts the geometry of the slide out panel
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to a client data of slide out
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Subpanel is to appear above or below the front panel, centered
 *  on the vertical axis of the spawning control.
 ******************************<->***********************************/
static void 
AdjustSlideOutGeometry ( ClientData *pCD)
{
    ClientData  *pCD_FP = NULL;
    WmPanelistObject  pPanelist;

    pCD->slideDirection = SLIDE_NORTH;	/* assume up for now */
    pPanelist = (WmPanelistObject) pCD->pSD->wPanelist;
    (void) XFindContext (DISPLAY, XtWindow(O_Shell(pPanelist)),
		  wmGD.windowContextType, (caddr_t *)&pCD_FP);

    if (pCD_FP)
    {
	/*
	* Adjust slide up position if coming from front
	* panel. 
	* (Assumes no nesting of panels !!!)
	* (Assumes horizontal oriented front panel!!!)
	*/
	if (pCD->transientLeader == pCD_FP)
	{
	    /* 
	     * Subpanel should be sort-of centered already,
	     * adjust by width of window manager frame.
	     */
	    pCD->clientX -= pCD->frameInfo.lowerBorderWidth;

	    /* 
	     * Adjust to slide up above front panel.
	     */
	    pCD->clientY = pCD_FP->frameInfo.y - 
			 pCD->frameInfo.lowerBorderWidth -
			 pCD->clientHeight + 3;

/*  RICK -- added the (+ 3)  */


	    if (pCD->clientY < 0)
	    {
		/*
		 * Adjust to slide down below front panel.
		 */
	        pCD->clientY = pCD_FP->frameInfo.y +
			     pCD_FP->frameInfo.height +
			     pCD->frameInfo.titleBarHeight +
			     pCD->frameInfo.upperBorderWidth - 3;
	        pCD->slideDirection = SLIDE_SOUTH;
/*  RICK -- added the (- 3)  */
	    }

	    if ((pCD->clientY + pCD->clientHeight +
		   pCD->frameInfo.lowerBorderWidth) > 
		XDisplayHeight (DISPLAY, pCD->pSD->screen))
	    {
	       /*
		* If the bottom of the slide-up is off the bottom
		* of the screen, then don't slide, just pop it up.
		*/
	       pCD->slideDirection = SLIDE_NOT;
	    }

	    PlaceFrameOnScreen (pCD, &pCD->clientX, &pCD->clientY,
		    pCD->clientWidth, pCD->clientHeight);
        }
	SetFrameInfo (pCD);
    }
}
#endif  /* PANELIST */


/*************************************<->*************************************
 *
 *  PlaceFrameOnScreen (pCD, pX, pY, w, h)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to nudge a client window so that it is totally
 *  onscreen if possible.  At least the top left corner will be onscreen.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  pX		- pointer to x-coord
 *  pY		- pointer to y-coord
 *  w		- width of window
 *  h		- height of window
 *
 * 
 *  Outputs:
 *  -------
 *  *pX		- new x-coord
 *  *pY		- new y-coord
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
PlaceFrameOnScreen (ClientData *pCD, int *pX, int *pY, int w, int h)
{
    int clientOffsetX;
    int clientOffsetY;
    int frameX;
    int frameY;
    int frameWidth;
    int frameHeight;
    int screenHeight;
    int screenWidth;


    clientOffsetX = pCD->clientOffset.x;
    clientOffsetY = pCD->clientOffset.y;
    frameX = *pX - clientOffsetX;
    frameY = *pY - clientOffsetY;
    frameWidth = w + (2 * clientOffsetX);
    frameHeight = h + clientOffsetX + clientOffsetY;
    screenWidth = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    screenHeight = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD));

    if ((frameX + frameWidth) > screenWidth)
    {
        frameX -= (frameX + frameWidth) - screenWidth;
    }
    if ((frameY + frameHeight) > screenHeight)
    {
        frameY -= (frameY + frameHeight) - screenHeight;
    }
    if (frameX < 0)
    {
        frameX = 0;
    }
    if (frameY < 0)
    {
        frameY = 0;
    }

    *pX = frameX + clientOffsetX;
    *pY = frameY + clientOffsetY;

} /* END OF FUNCTION PlaceFrameOnScreen */



/*************************************<->*************************************
 *
 *  PlaceIconOnScreen (pCD, pX, pY)
 *
 *
 *  Description:
 *  -----------
 *  This function positions an icon on-screen.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  pX		- pointer to x-coord
 *  pY		- pointer to y-coord
 * 
 *  Outputs:
 *  -------
 *  *pX		- new x-coord
 *  *pY		- new y-coord
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void 
PlaceIconOnScreen (ClientData *pCD, int *pX, int *pY)
{
    int screenWidth;
    int screenHeight;
    int iconX;
    int iconY;


    screenWidth = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    screenHeight = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    iconX = *pX;
    iconY = *pY;

    if ((iconX + ICON_WIDTH(pCD)) > screenWidth)
    {
        iconX = screenWidth - ICON_WIDTH(pCD);
    }
    else if (iconX < 0)
    {
        iconX = 0;
    }

    if ((iconY + ICON_HEIGHT(pCD)) > screenHeight)
    {
        iconY = screenHeight - ICON_HEIGHT(pCD);
    }
    else if (iconY < 0)
    {
        iconY = 0;
    }

    *pX = iconX;
    *pY = iconY;


} /* END OF FUNCTION PlaceIconOnScreen */



/*************************************<->*************************************
 *
 *  FixWindowConfiguration (pCD, pWidth, pHeight, widthInc, heightInc)
 *
 *
 *  Description:
 *  -----------
 *  This function adjusts the configuration for the client window so that
 *  it is in line with the client window's sizing constraints.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = a pointer to the client window data
 *  pWidth, pHeight = pointers to the window configuration values
 *  widthInc, heightInc = window size increment values
 *
 * 
 *  Outputs:
 *  -------
 *  pWidth, pHeight = adjusted configuration values are returned here
 *
 * 
 *************************************<->***********************************/

void 
FixWindowConfiguration (ClientData *pCD, unsigned int *pWidth, unsigned int *pHeight, unsigned int widthInc, unsigned int heightInc)
{
    register int  delta;

    /*
     * Make sure we're on width/height increment boundaries.
     */

    if ((int) *pWidth < pCD->minWidth)
    {
	*pWidth = pCD->minWidth;
    }
    else if ((delta = (*pWidth - pCD->baseWidth) % pCD->widthInc))
    {
	*pWidth -= delta;
    }

    if ((int) *pHeight < pCD->minHeight)
    {
	*pHeight = pCD->minHeight;
    }
    else if ((delta = (*pHeight - pCD->baseHeight) % pCD->heightInc))
    {
	*pHeight -= delta;
    }

    /*
     * Constrain size within bounds.
     */

    FixWindowSize (pCD, pWidth, pHeight, widthInc, heightInc);

} /* END OF FUNCTION FixWindowConfiguration */



/*************************************<->*************************************
 *
 *  FixWindowSize (pCD, pWidth, pHeight, widthInc, heightInc)
 *
 *
 *  Description:
 *  -----------
 *  This function adjusts the client window width and height so that
 *  it is in line with its sizing constraints.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = a pointer to the client window data
 *  pWidth, pHeight = pointers to the window size values
 *  widthInc, heightInc = window size increment values
 *  pWS->limitResize 
 *
 * 
 *  Outputs:
 *  -------
 *  pWidth, pHeight = adjusted size values.
 *
 * 
 *************************************<->***********************************/

void 
FixWindowSize (ClientData *pCD, unsigned int *pWidth, unsigned int *pHeight, unsigned int widthInc, unsigned int heightInc)
{
    register int  deltaW;
    register int  deltaH;
    WmScreenData *pSD = pCD->pSD;

    /*
     * All occurances of maxHeight and maxWidth in this routing has been
     * hacked to use maxHeightLimit and maxWidthLimit as the real max when
     * maximumClientSize is set to 'horizontal' or 'vertical', since
     * pCD->maxHeight and pCD->maxWidth is fiddle to on reconfiguration.
     */

    if ((int) *pWidth < pCD->minWidth)
    {
	*pWidth = pCD->minWidth;
    }
    else if (*pWidth > pCD->maxWidthLimit &&
             pSD->limitResize && 
	     !(pCD->clientFlags & CLIENT_WM_CLIENTS))
    {
	*pWidth = pCD->maxWidthLimit;
    }

    if ((int) *pHeight < pCD->minHeight)
    {
	*pHeight = pCD->minHeight;
    }
    else if (*pHeight > pCD->maxHeightLimit &&
             pSD->limitResize && 
	     !(pCD->clientFlags & CLIENT_WM_CLIENTS))
    {
	*pHeight = pCD->maxHeightLimit;
    }

    if ((pCD->sizeFlags & P_ASPECT) &&
        *pWidth * pCD->maxAspect.y > *pHeight * pCD->maxAspect.x)
    /* 
     * Client aspect is too big.
     * Candidate height >= client height:
     *   Try to increase the client's height without violating bounds.
     *   If this fails, use maximum height and try to decrease its width.
     * Candidate height < client height:
     *   Try to decrease the client's width without violating bounds.
     *   If this fails, use minimum width and try to increase its height.
     */
    {
        if ((*pHeight >= pCD->clientHeight) ||
            (*pWidth > pCD->clientWidth))
        /* 
         * Candidate height >= client height:
         *   Try to increase the client's height without violating bounds.
         *   If this fails, use maximum height and try to decrease its width.
	 */
        {
            deltaH = makemult (*pWidth * pCD->maxAspect.y / pCD->maxAspect.x -
		               *pHeight, heightInc);
	    if (*pHeight + deltaH <= pCD->maxHeightLimit ||
                !pSD->limitResize ||
		pCD->clientFlags & CLIENT_WM_CLIENTS)
	    {
	        *pHeight += deltaH;
	    }
	    else 
	    {
	        *pHeight = pCD->maxHeightLimit;
	        deltaW = makemult (*pWidth - *pHeight * pCD->maxAspect.x / 
			           pCD->maxAspect.y, widthInc);
	        if (*pWidth - deltaW >= pCD->minWidth)
	        {
	            *pWidth -= deltaW;
                }  
		else
		{
	            *pWidth = pCD->minWidth;
		}
	    }
	}
	else
        /*
         * Candidate height < client height and candidate width <= client width.
         *   Try to decrease the client's width without violating bounds.
         *   If this fails, use minimum width and try to increase its height.
	 */
	{
	    deltaW = makemult (*pWidth - *pHeight * pCD->maxAspect.x / 
			       pCD->maxAspect.y, widthInc);

	    if (*pWidth - deltaW >= pCD->minWidth)
	    {
	        *pWidth -= deltaW;
            }  
	    else
	    {
	        *pWidth = pCD->minWidth;
                deltaH = makemult (*pWidth * pCD->maxAspect.y / 
				   pCD->maxAspect.x - *pHeight, heightInc);
	        if (*pHeight + deltaH <= pCD->maxHeightLimit ||
                     !pSD->limitResize ||
	             pCD->clientFlags & CLIENT_WM_CLIENTS)
	        {
	            *pHeight += deltaH;
	        }
		else
		{
	            *pHeight = pCD->maxHeightLimit;
		}
	    }
	}
    }

    else if ((pCD->sizeFlags & P_ASPECT) &&
             *pHeight * pCD->minAspect.x > *pWidth * pCD->minAspect.y)
    /* 
     * Client aspect is too small.
     * Candidate width >= client width:
     *   Try to increase the client's width without violating bounds.
     *   If this fails, use maximum width and try to decrease its height.
     * Candidate width < client width:
     *   Try to decrease the client's height without violating bounds.
     *   If this fails, use minimum height and try to increase its width.
     */
    {
        if ((*pWidth >= pCD->clientWidth) ||
            (*pHeight > pCD->clientHeight))
        /*
         * Candidate width >= client width:
         *   Try to increase the client's width without violating bounds.
         *   If this fails, use maximum width and try to decrease its height.
	 */
	{
            deltaW = makemult (*pHeight * pCD->minAspect.x / pCD->minAspect.y -
			       *pWidth, widthInc);
	    if (*pWidth + deltaW <= pCD->maxWidthLimit ||
                !pSD->limitResize ||
	        pCD->clientFlags & CLIENT_WM_CLIENTS)
	    {
	        *pWidth += deltaW;
	    }
	    else
	    {
	        *pWidth = pCD->maxWidthLimit;
	        deltaH = makemult (*pHeight - *pWidth * pCD->minAspect.y / 
			           pCD->minAspect.x, heightInc); 
	        if (*pHeight - deltaH >= pCD->minHeight)
	        {
	            *pHeight -= deltaH;
                }
		else
		{
	            *pHeight = pCD->minHeight;
		}
	    }
	}
	else
        /*
         * Candidate width < client width and Candidate height <= client height:
         *   Try to decrease the client's height without violating bounds.
         *   If this fails, use minimum height and try to increase its width.
	 */
	{
	    deltaH = makemult (*pHeight - *pWidth * pCD->minAspect.y / 
			       pCD->minAspect.x, heightInc); 
	    if (*pHeight - deltaH >= pCD->minHeight)
	    {
	        *pHeight -= deltaH;
            }
	    else
	    {
	        *pHeight = pCD->minHeight;
                deltaW = makemult (*pHeight * pCD->minAspect.x / 
				   pCD->minAspect.y - *pWidth, widthInc);
	        if (*pWidth + deltaW <= pCD->maxWidthLimit ||
                     !pSD->limitResize ||
	             pCD->clientFlags & CLIENT_WM_CLIENTS)
	        {
	            *pWidth += deltaW;
	        }
		else
		{
	            *pWidth = pCD->maxWidthLimit;
		}
	    }
	}
    }
} /* END OF FUNCTION FixWindowSize */



/*************************************<->*************************************
 *
 *  FindClientPlacement (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function finds a position for the client window on the screen.
 *  Windows positions are stepped down the screen.  An attempt is made
 *  to keep windows from being clipped by the edge of the screen.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (client window configuration fields)
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = changes to the client window configuration fields
 * 
 *************************************<->***********************************/

void 
FindClientPlacement (ClientData *pCD)
{
    static Boolean clientPlacementInitialized = False;
    static int clientPlacementOffset;
    static int clientPlacementX;
    static int clientPlacementY;
    static int clientPlacementOrigin;
    static int clientPlacementXOrigin;

    Boolean placed = False;
    int frameWidth;
    int frameHeight;
    int screenWidth;
    int screenHeight;
    int borderWidth = 0;
    Boolean offScreenX;
    Boolean offScreenY;


    if (!clientPlacementInitialized)
    {
	if (pCD->clientDecoration & WM_DECOR_RESIZEH)
	{
	    borderWidth = ((RESIZE_BORDER_WIDTH(pCD) > FRAME_BORDER_WIDTH(pCD))
			  ? RESIZE_BORDER_WIDTH(pCD) : FRAME_BORDER_WIDTH(pCD));
	}
	else
	{
	    borderWidth = pCD->matteWidth;
	}
	clientPlacementOffset = TitleTextHeight(pCD) + borderWidth;
	clientPlacementOrigin = clientPlacementOffset;
	clientPlacementX = clientPlacementOrigin;
	clientPlacementY = clientPlacementOrigin;
	clientPlacementXOrigin = clientPlacementX;
	clientPlacementInitialized = True;
    }

    frameWidth = pCD->clientWidth + (2 * pCD->clientOffset.x);
    frameHeight = pCD->clientHeight + pCD->clientOffset.y + pCD->clientOffset.x;
    screenWidth = DisplayWidth (DISPLAY, SCREEN_FOR_CLIENT(pCD));
    screenHeight = DisplayHeight (DISPLAY, SCREEN_FOR_CLIENT(pCD));

    while (!placed)
    {
	if ((clientPlacementX - pCD->clientOffset.x + frameWidth)
	    > screenWidth)
	{
	    offScreenX = True;
	}
	else
	{
	    offScreenX = False;
	}
	if ((clientPlacementY - pCD->clientOffset.y + frameHeight)
	    > screenHeight)
	{
	    offScreenY = True;
	}
	else
	{
	    offScreenY = False;
	}

	if (offScreenX || offScreenY)
	{
	    if (clientPlacementX == clientPlacementOrigin)
	    {
		/*
		 * Placement location is already as far to the NW as it is
		 * going to go.
		 */

		placed = True;
	    }
	    else if (clientPlacementY == clientPlacementOrigin)
	    {
		/*
		 * Placement location is as far to the N as it is going to go.
		 * Use the current placement if the window is not off the
		 * screen in the x coordinate otherwise reset the placement
		 * back to the NW origin.
		 */

		if (offScreenX)
		{
		    clientPlacementXOrigin = clientPlacementOrigin;
		    clientPlacementX = clientPlacementXOrigin;
		}
		placed = True;
	    }
	    else
	    {
		/*
		 * If window is off the right edge of screen, just move
		 * window in the X direction onto screen.  Process similarly
		 * for windows that are off the bottom of the screen.
		 */

		if (offScreenX && !offScreenY)
		{
		    clientPlacementX = clientPlacementOrigin;
		}
		else if (offScreenY && !offScreenX)
		{
		    clientPlacementY = clientPlacementOrigin;
		}
		else
		{

		/*
		 * Reset the placement location back to the NW of the
		 * current location.  Go as far N as possible and step the
		 * x coordinate to the E.
		 */

		    clientPlacementXOrigin += clientPlacementOffset;
		    clientPlacementX = clientPlacementXOrigin;
		    clientPlacementY = clientPlacementOrigin;
		}
	    }
	}
	else
	{
	    placed = True;
	}
    }

    /*
     * The window has been placed, now update the placement information.
     */

    pCD->clientX = clientPlacementX;
    pCD->clientY = clientPlacementY;
    clientPlacementX += clientPlacementOffset;

    if (clientPlacementX >= screenWidth)
    {
	clientPlacementXOrigin = clientPlacementOrigin;
	clientPlacementX = clientPlacementXOrigin;
    }
    clientPlacementY += clientPlacementOffset;

    /*
     * Reset Y position to top of screen so that windows start new column of
     * placement that is offset from the previous column.  Previously, the new
     * column was place right over the old column, obscuring it.
     * NOTE: column == diagonal
     */

    if (clientPlacementY >= (screenHeight / 3))
    {
	clientPlacementY = clientPlacementOrigin;
    }


} /* END OF FUNCTION FindClientPlacement */



/*************************************<->*************************************
 *
 *  WmGetWindowAttributes (window)
 *
 *
 *  Description:
 *  -----------
 *  This function gets window attributes if necessary and saves them in the
 *  global window attribute cache.  If the window attributes are already
 *  there then no X call is made.
 *
 *
 *  Inputs:
 *  ------
 *  window = get attributes for window with this id
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.attributesWindow = set to window that matches windowAttributes
 *
 *  wmGD.windowAttributes = XWindowAttributes of window
 *
 *
 *  Comments:
 *  --------
 *  The attributes in the global cache are (known) current only for a
 *  single pass through the wm event processing loop.  They (should be)
 *  regularly cleared.
 * 
 *************************************<->***********************************/

Boolean 
WmGetWindowAttributes (Window window)
{
    if (wmGD.attributesWindow != window)
    {
	if (!XGetWindowAttributes (DISPLAY, window, &wmGD.windowAttributes))
	{
	    /*
	     * Cannot get window attributes.
	     */

	    wmGD.attributesWindow = (Window)0L;
	    return (False);
	}
	wmGD.attributesWindow = window;
    }

    return (True);

} /* END OF FUNCTION WmGetWindowAttributes */



/*************************************<->*************************************
 *
 *  SetupClientIconWindow (pCD, window)
 *
 *
 *  Description:
 *  -----------
 *  This function prepares a client supplied icon window for insertion into
 *  a window manager icon frame.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *
 *  window = client supplied icon window
 *
 * 
 *  Outputs:
 *  -------
 *  pCD = (iconWindow)
 *
 *  Return = True if the icon window can be used
 *
 *************************************<->***********************************/

Boolean 
SetupClientIconWindow (ClientData *pCD, Window window)
{
    ClientData *pcd;


    /*
     * Check to see if the icon window can be used (i.e there is no conflict
     * of interest.
     */

    if (!XFindContext (DISPLAY, window, wmGD.windowContextType,
	     (caddr_t *)&pcd))
    {
	if (window == pCD->client)
	{
	    /*
	     * The proposed icon window is the same as the client!
	     */

	    return (False);
	}

	/*
	 * The proposed icon window is already being managed. 
	 * Assume that we managed it by mistake. Unmanage the 
	 * window and use it as the icon window for this client.
	 */

	 UnManageWindow (pcd);
    }

    /* update client data */
    pCD->iconWindow = window;

    /* put in window manager's save set */
    XChangeSaveSet (DISPLAY, pCD->iconWindow, SetModeInsert);
    pCD->clientFlags  |=  ICON_IN_SAVE_SET;

    return (True);

} /* END OF FUNCTION SetupClientIconWindow */



/*************************************<->*************************************
 *
 *  ProcessMwmHints (pCD)
 *
 *
 *  Description:
 *  -----------
 *  Process the _MWM_HINTS property on the window (if any).  Setup the
 *  applicable function and decoration masks.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  pCD	= may be changed.
 *
 *************************************<->***********************************/

void 
ProcessMwmHints (ClientData *pCD)
{
    PropMwmHints *pHints;


    /*
     * Fix the client functions and decorations fields if they have
     * default resource values.
     */

    if (pCD->clientFunctions & WM_FUNC_DEFAULT)
    {
	if (pCD->clientFlags & CLIENT_TRANSIENT)
	{
	    pCD->clientFunctions = TRANSIENT_FUNCTIONS(pCD);
	}
	else
	{
	    pCD->clientFunctions = WM_FUNC_ALL;
	}
#ifdef PANELIST
        if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)
	{
	    pCD->clientFunctions &= WM_FUNC_SUBPANEL_DEFAULT;   
	    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
	}
        else if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_PANEL)
	{
	    pCD->clientFunctions &= WM_FUNC_PANEL_DEFAULT;   
	    pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
	}
#endif /* PANELIST */
    }

    if (pCD->clientDecoration & WM_DECOR_DEFAULT)
    {
	if (pCD->clientFlags & CLIENT_TRANSIENT)
	{
	    pCD->clientDecoration = TRANSIENT_DECORATION(pCD);
	}
	else
	{
	    pCD->clientDecoration = WM_DECOR_ALL;
	}
#ifdef PANELIST
        if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL)
	{
	    pCD->clientDecoration = pCD->pSD->subpanelDecoration;
	}
        else if (pCD->dtwmBehaviors & DtWM_BEHAVIOR_PANEL)
	{
	    pCD->clientDecoration &= WM_DECOR_PANEL_DEFAULT;   
	}
#endif /* PANELIST */
    }


    /*
     * Retrieve the _MWM_HINTS property if it exists.
     */

    pCD->inputMode = MWM_INPUT_MODELESS;

    if ((pHints = GetMwmHints (pCD)) != NULL)
    {
	if (pHints->flags & MWM_HINTS_FUNCTIONS)
	{
	    if (pHints->functions & MWM_FUNC_ALL)
	    {
		/* client indicating inapplicable functions */
		pCD->clientFunctions &= ~(pHints->functions);
	    }
	    else
	    {
		/* client indicating applicable functions */
		pCD->clientFunctions &= pHints->functions;
	    }
#if 0
	    if (!(pCD->clientFlags & GOT_DT_WM_HINTS) &&
		!pHints->functions)
	    {
		/*
		 * !!! Backward compatibility heurisitic !!!
		 * 
		 * If client doesn't want any functions and
		 * no DT_WM_HINTS specified, then remove 
		 * workspace functions.
		 */
		pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
	    }
#endif 
	    /* !!! check for some minimal level of functionality? !!! */
	}

	if (pHints->flags & MWM_HINTS_DECORATIONS)
	{
	    if (pHints->decorations & MWM_DECOR_ALL)
	    {
		/* client indicating decorations to be removed */
		pCD->clientDecoration &= ~(pHints->decorations);
	    }
	    else
	    {
		/* client indicating decorations to be added */
		pCD->clientDecoration &= pHints->decorations;
	    }

	    /*
	     * Fix up decoration configuration.
	     */

	    if (pCD->clientDecoration &
		  (MWM_DECOR_MENU | MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE))
	    {
		pCD->clientDecoration |= MWM_DECOR_TITLE;
	    }
	    if (pCD->clientDecoration & MWM_DECOR_RESIZEH)
	    {
		pCD->clientDecoration |= MWM_DECOR_BORDER;
	    }
	}

	if (pHints->flags & MWM_HINTS_INPUT_MODE)
	{
	    if ((pHints->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL) ||
		(pHints->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL) ||
		((pHints->inputMode == MWM_INPUT_SYSTEM_MODAL) &&
		 !wmGD.systemModalActive))
		
	    {
	        pCD->inputMode = pHints->inputMode;

	    }

	    /*
	     * Don't allow a system modal window to be a secondary window
	     * (except with respect to applicable functions and frame
	     * decorations). Also, don't allow system modal window to
	     * be minimized.
	     */

	    if (pCD->inputMode == MWM_INPUT_SYSTEM_MODAL)
	    {
		pCD->transientLeader = NULL;
		if (pCD->clientFunctions & MWM_FUNC_MINIMIZE)
		{
		    pCD->clientFunctions &= ~(MWM_FUNC_MINIMIZE);
		}
	    }
	}

	if (pHints->flags & MWM_HINTS_STATUS)
	{
	    pCD->window_status = pHints->status;
	}

	XFree ((char*)pHints);
    }
#ifndef NO_OL_COMPAT
    else
    {
	ProcessOLDecoration (pCD);
    }
#endif /* NO_OL_COMPAT */

#ifdef WSM
    /* 
     * If primary window can't move between workspaces, then
     * secondary window shouldn't either.
     */
    if (pCD->transientLeader &&
	!(pCD->transientLeader->dtwmFunctions & DtWM_FUNCTION_OCCUPY_WS))
    {
	pCD->dtwmFunctions &= ~DtWM_FUNCTION_OCCUPY_WS;
    }
#endif /* WSM */

    /*
     * Fix up functions based on system modal settings.  System modal
     * windows and their descendents cannot be minimized.
     */

    if (!((FindTransientTreeLeader (pCD))->clientFunctions&MWM_FUNC_MINIMIZE))
    {
	pCD->clientFunctions &= ~MWM_FUNC_MINIMIZE;
    }


    /*
     * Fix up decoration configuration based on applicable functions.
     */

    if (!(pCD->clientFunctions & MWM_FUNC_RESIZE))
    {
	pCD->clientDecoration &= ~MWM_DECOR_RESIZEH;
    }

    if (!(pCD->clientFunctions & MWM_FUNC_MINIMIZE))
    {
	pCD->clientDecoration &= ~MWM_DECOR_MINIMIZE;
    }

    if (!(pCD->clientFunctions & MWM_FUNC_MAXIMIZE))
    {
	pCD->clientDecoration &= ~MWM_DECOR_MAXIMIZE;
    }
    
    pCD->decor = pCD->clientDecoration;  /* !!! combine decor ... !!! */


} /* END OF ProcessMwmHints */
