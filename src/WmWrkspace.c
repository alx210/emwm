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

#include <stdlib.h>
#include <stdio.h>
#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmIPlace.h"
#include "WmInitWs.h"
#include <X11/Xutil.h>
#include "WmICCC.h"
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include "WmError.h"
#include "WmXinerama.h"
#include "WmPresence.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIconBox.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmResParse.h"
#include "WmWinInfo.h"
#include "WmWinList.h"
#include "WmWinState.h"
#include "WmXSMP.h"
#include "WmBackdrop.h"
#include "WmEwmh.h"

/* local macros */
#ifndef MIN
#define MIN(a,b) ((a)<=(b)?(a):(b))
#endif 

#ifndef MAX
#define MAX(a,b) ((a)>=(b)?(a):(b))
#endif 


/* internally defined functions */

#include "WmWrkspace.h"

/********    Static Function Declarations    ********/

static void InsureUniqueWorkspaceHints( 
                        ClientData *pCD) ;

/********    End Static Function Declarations    ********/

/*
 * Global Variables:
 */

/* a dynamically allocated list of workspaces used
 * by F_AddToAllWorkspaces
 */
static int numResIDs = 0;
static WorkspaceID *pResIDs = NULL;


/*************************************<->*************************************
 *
 *  ChangeToWorkspace (pNewWS)
 *
 *
 *  Description:
 *  -----------
 *  This function changes to a new workspace.
 *
 *  Inputs:
 *  ------
 *  pNewWS =  pointer to workspace data
 *
 * 
 *************************************<->***********************************/

void ChangeToWorkspace(WmWorkspaceData *pNewWS )

{
    ClientData *pCD;
    int i;
    WmScreenData *pSD = pNewWS->pSD;

    ClientData *pWsPCD;
    Context   wsContext = F_CONTEXT_NONE;

    if (pNewWS == pSD->pActiveWS)
	return;				/* already there */

    pSD->pLastWS = pSD->pActiveWS;

    /*
     * Go through client list of old workspace and hide windows
     * that shouldn't appear in new workspace.
     */

    if (pSD->presence.shellW && 
	pSD->presence.onScreen &&
	pSD->presence.contextForClient == F_CONTEXT_ICON)
    {
	pWsPCD = pSD->presence.pCDforClient;
	wsContext = pSD->presence.contextForClient;
	HidePresenceBox (pSD, False);
    }

    for (i = 0; i < pSD->pActiveWS->numClients; i++)
    {
	pCD = pSD->pActiveWS->ppClients[i];
	if (!ClientInWorkspace (pNewWS, pCD))
	{
	   SetClientWsIndex(pCD);
	   SetClientState (pCD, pCD->clientState | UNSEEN_STATE,
		 CurrentTime);
	}
    }

    /*
     * Hide active icon text label
     */
     if ((pSD->iconDecoration & ICON_ACTIVE_LABEL_PART) &&
	 wmGD.activeIconTextDisplayed)
     {
	 HideActiveIconText(pSD);
     }
    
    /*
     * Unmap old icon box
     */
    if (pSD->useIconBox)
    {
	UnmapIconBoxes (pSD->pLastWS);
    }
    
    /* 
     * Set new active workspace 
     */
    pSD->pActiveWS = pNewWS;
    ChangeBackdrop (pNewWS);

    /*
     * Go through client list of new workspace and show windows
     * that should appear.
     */
    for (i = 0; i < pNewWS->numClients; i++)
    {
	pCD = pNewWS->ppClients[i];
	SetClientWsIndex(pCD);
        if (pCD->clientState & UNSEEN_STATE)
	{
	    SetClientState (pCD, 
		(pCD->clientState & ~UNSEEN_STATE), CurrentTime);
	}
	if ((pCD->clientState == MINIMIZED_STATE) &&
			 ((!pCD->pSD->useIconBox) || 
			  (!P_ICON_BOX(pCD))))
	{
	    XMoveWindow (DISPLAY, ICON_FRAME_WIN(pCD), 
			ICON_X(pCD), ICON_Y(pCD));
	}

	if (pCD->iconWindow)
	{
	    unsigned int xOffset, yOffset;

	    /*
	     * Adjust for icons in the box
	     */

	    if (pNewWS->pIconBox)
	    {
		xOffset = IB_MARGIN_WIDTH;
		yOffset = IB_MARGIN_HEIGHT;
	    }
	    else
	    {
		xOffset = 0;
		yOffset = 0;
	    }

	    /*
	     * reparent icon window to frame in this workspace
	     */
	    if ((ICON_DECORATION(pCD) & ICON_IMAGE_PART) && 
		(pCD->iconWindow))
	    {
		ReparentIconWindow (pCD, xOffset, yOffset);
	    }
	}
    }

    if ( (wsContext == F_CONTEXT_ICON &&
	  ClientInWorkspace (ACTIVE_WS, pWsPCD)) ||
	
	 (pSD->presence.shellW && 
	  ! pSD->presence.userDismissed &&
	  ClientInWorkspace (ACTIVE_WS, pSD->presence.pCDforClient) &&
	  pSD->presence.contextForClient == F_CONTEXT_ICON))
    {
	ShowPresenceBox(pSD->presence.pCDforClient, F_CONTEXT_ICON);
    }

    SetCurrentWorkspaceProperty (pSD);

	UpdateEwmhActiveWorkspace(pSD, pNewWS->id);

} /* END OF FUNCTION ChangeToWorkspace */

/******************************<->*************************************
 *
 *  ChangeWorkspaceTitle (pWS, pchTitle)
 *
 *  Description:
 *  -----------
 *  Set the title for a workspace.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pchTitle = new title to assign to this workspace
 *
 *  Outputs:
 *  -------
 *  none
 * 
 *  Comments:
 *  --------
 *  
 ******************************<->***********************************/

void ChangeWorkspaceTitle(WmWorkspaceData *pWS, char * pchTitle)
{
    XmString xmstr;

    /*
     * Convert string to XmString
     */
    xmstr = XmStringCreateLocalized (pchTitle);

    /*
     * Validate title ?
     */

    /*
     * Replace title in workspace data
     */
    XmStringFree (pWS->title);
    pWS->title = xmstr;

    /*
     * Replace old workspace in info property
     */
    SetWorkspaceInfoProperty (pWS);
    XFlush (DISPLAY);

	UpdateEwmhWorkspaceProperties(pWS->pSD);

} /* END OF FUNCTION ChangeWorkspaceTitle */


/*************************************<->*************************************
 *
 *  UpdateWorkspacePresenceProperty (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function updates the _MWM_WORKSPACE_PRESENCE property for a
 *  client window
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data
 *
 * 
 *************************************<->***********************************/

void 
UpdateWorkspacePresenceProperty(ClientData *pCD)

{
    static Atom 	*pPresence = NULL;
    static unsigned long   cPresence = 0;
    unsigned long i;

    if (wmGD.useStandardBehavior)
    {
	/*
	 * Don't change any workspace properties in standard behavior
	 * mode.
	 */
	return;
    }

    if (!pPresence)
    {
	/* allocate initial list */
	if (!(pPresence = (Atom *) 
		    XtMalloc (pCD->pSD->numWorkspaces * sizeof(Atom))))
	{
	    Warning (((char *)GETMESSAGE(76, 1, "Insufficient memory for workspace presence property")));
	}
	else
	{
	    cPresence = pCD->pSD->numWorkspaces;
	}
    }

    if (cPresence < pCD->numInhabited)
    {
	/* allocate bigger list */
	if (!(pPresence = (Atom *) 
		XtRealloc ((char *)pPresence, pCD->numInhabited * sizeof(Atom))))
	{
	    Warning (((char *)GETMESSAGE(76, 2, "Insufficient memory for workspace presence property")));
	}
	else
	{
	    cPresence = pCD->numInhabited;
	}
    }

    for (i = 0; (i < pCD->numInhabited) && (i < cPresence) ; i++)
    {
	pPresence[i] = pCD->pWsList[i].wsID;
    }

    SetWorkspacePresence (pCD->client, pPresence,
				MIN(pCD->numInhabited, cPresence));

} /* END OF FUNCTION UpdateWorkspacePresenceProperty */


/*************************************<->*************************************
 *
 *  AddPersistentWindow (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function adds windows that want to be in all workspaces to
 *  the workspace passed in.
 *
 *  Inputs:
 *  ------
 *  pWS  =  pointer to workspace data
 *
 *  Outputs:
 *  --------
 * 
 *************************************<->***********************************/

void AddPersistentWindows(WmWorkspaceData *pWS)

{
    WmScreenData *pSD = pWS->pSD;
    ClientListEntry *pCLE;

    /*
     * For all the windows managed for this screen, see if they
     * want to be in all workspaces and add them to this workspace.
     */
    pCLE = pSD->clientList;

    while (1)
    {
	/*
	 * Process all the non-icon client list entries 
	 */
	if ((pCLE->type == NORMAL_STATE) &&
	    (pCLE->pCD->putInAll))
	{
	    AddClientToWorkspaces( pCLE->pCD, &(pWS->id), 1 );
	}
	
	/*
	 * Test for exit condition and advance client list pointer
	 */
	if (pCLE == pSD->lastClient) 
	    break;
	else
	    pCLE = pCLE->nextSibling;
    }

} /* END OF FUNCTION AddPersistentWindows */

/*************************************<->*************************************
 *
 *  CreateWorkspace (pSD, pchTitle)
 *
 *
 *  Description:
 *  -----------
 *  This function creates a new workspace.
 *
 *  Inputs:
 *  ------
 *  pSD  =  pointer to screen data
 *  pchTitle = user-visible title for the workspace (may be NULL)
 *
 *  Outputs:
 *  --------
 *  Returns pointer to workspace data if successful.
 * 
 *************************************<->***********************************/

WmWorkspaceData* CreateWorkspace(
        WmScreenData *pSD,
        unsigned char *pchTitle )

{
    WmWorkspaceData *pWS = NULL;
    String string;
    int iActiveWS;

    /*
     * Allocate more workspace datas if we have no spares
     */
    if (pSD->numWsDataAllocated <= pSD->numWorkspaces)
    {
	iActiveWS = (pSD->pActiveWS - pSD->pWS) / sizeof (WmWorkspaceData);
	pSD->numWsDataAllocated += WS_ALLOC_AMOUNT;
	pSD->pWS = (WmWorkspaceData *) XtRealloc ((char *)pSD->pWS,
		    pSD->numWsDataAllocated * sizeof(WmWorkspaceData));
	pSD->pActiveWS = &(pSD->pWS[iActiveWS]);
    }

    /*
     * Give this workspace a name
     */
    pWS = &pSD->pWS[pSD->numWorkspaces];
    string = (String) GenerateWorkspaceName (pSD, pSD->numWorkspaces);
    pWS->name = XtNewString (string);

    /*
     * Initialize the workspace data structure
     */
    InitWmWorkspace (pWS, pSD);
    if (pchTitle) 
    {
	if (pWS->title)
	    XmStringFree (pWS->title);
	pWS->title = XmStringCreateLocalized ((char *)pchTitle);
    }

    /*
     * bump workspace count
     */
    pSD->numWorkspaces++;

    /*
     * update the properties that announce workspace info
     */
    SetWorkspaceInfoProperty (pWS);
    SetWorkspaceListProperty (pSD);

    /*
     * Insure there's an iconbox for this workspace
     */
    if (pSD->useIconBox)
    {
	AddIconBoxForWorkspace (pWS);
    }

    /*
     * Add windows to this workspaces that want to be in "all"
     * workspaces.
     */
    AddPersistentWindows (pWS);

    /*
     * Update workspace presence dialog data
     */
    UpdatePresenceWorkspaces(pSD);

	UpdateEwmhWorkspaceProperties(pSD);
	
    return (pWS);
} /* END OF FUNCTION CreateWorkspace */

/*************************************<->*************************************
 *
 *  DeleteWorkspace (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes a workspace.
 *
 *  Inputs:
 *  ------
 *  pWS  =  pointer to screen data
 *
 *  Outputs:
 *  --------
 *  Returns pointer to workspace data if successful.
 * 
 *************************************<->***********************************/

void DeleteWorkspace(WmWorkspaceData *pWS)

{
    WmWorkspaceData *pWSdest;		/* destination WS */
    int iNextWs;
    ClientData *pCD;
    WmScreenData *pSD = pWS->pSD;

    if (pSD->numWorkspaces > 1)
    {
	/*
	 * Find index for "next" workspace
	 */
	for (iNextWs = 0; iNextWs < pSD->numWorkspaces; iNextWs++)
	{
	    if (pSD->pWS[iNextWs].id == pWS->id)
	    {
		iNextWs++;
		break;
	    }
	}

	/* check bounds and wrap */
	if (iNextWs >= pSD->numWorkspaces)
	    iNextWs = 0;

	/*
	 * Determine default destination for clients that exist
	 * only in the workspace being deleted.
	 */
	if (pWS == ACTIVE_WS)
	{
	    pWSdest = &(pSD->pWS[iNextWs]);
	}
	else
	{
	    /*
	     * Use the "current" workspace as the default destination
	     */
	    pWSdest = ACTIVE_WS;
	}

	/*
	 * Move all clients out of this workspace
	 */
	while (pWS->numClients > 0)
	{
	    /* repeatedly remove the first one until all are gone */
	    pCD = pWS->ppClients[0];


	    if (pCD->numInhabited == 1)
	    {
		if (!(pCD->clientFlags & (ICON_BOX)))
		{
		    AddClientToWorkspaces (pCD, &(pWSdest->id), 1);
		}
	    }

	    RemoveClientFromWorkspaces (pCD, &(pWS->id), 1);
	}

	/*
	 * If we're deleting the current workspace, 
	 * then change to another workspace.
	 */
	if (pWS == ACTIVE_WS)
	{
	    ChangeToWorkspace (pWSdest);
	}

	/*
	 * Destroy the icon box for the workspace if one was used
	 */
	if (pSD->useIconBox)
	{
	    DestroyIconBox (pWS);
	}

	/*
	 * Delete the property containing information on this workspace
	 */
	DeleteWorkspaceInfoProperty (pWS);

	/*
	 * Delete the workspace data structures
	 */
	if (pWS->backdrop.imagePixmap)
	{
	    if (!XmDestroyPixmap (XtScreen(pWS->workspaceTopLevelW),
			    pWS->backdrop.imagePixmap))
	    {
		/* not in Xm pixmap cache */
	    }
	}

	/* free pWS->backdrop.image */
	if ((pWS->backdrop.flags & BACKDROP_IMAGE_ALLOCED) &&
	    (pWS->backdrop.image))
	{
	    free (pWS->backdrop.image);
	}

    /* 
     * Free up icon placement data
     */
	if (wmGD.iconAutoPlace)
	{
    	int nxs, i;

    	if(!GetXineramaScreenCount(&nxs)) nxs = 1;

    	for(i = 0; i < nxs; i++) {
        	if (pWS->IPData[i].placeList != NULL)
        	   XtFree ((char *) pWS->IPData[i].placeList);
        	if (pWS->IPData[i].placementRowY != NULL)
        	   XtFree ((char *) pWS->IPData[i].placementRowY);
        	if (pWS->IPData[i].placementColX != NULL)
        	   XtFree ((char *) pWS->IPData[i].placementColX);
    	}
	}

	XtFree ((char *) pWS->name);
	XmStringFree (pWS->title);
	XtFree ((char *) pWS->ppClients);
	if (pWS->iconBoxGeometry) XtFree ((char *) pWS->iconBoxGeometry);
	XtDestroyWidget (pWS->workspaceTopLevelW);

	/*
	 * Compress the list of workspaces if we're not deleting
	 * the last one. (Do piece-wise to avoid overlapping copy
	 * problems).
	 */
	if (iNextWs > 0)
	{
	    WmWorkspaceData *pWSdest;
	    WmWorkspaceData *pWSsrc;
	    int j;

	    pWSdest = pWS;
	    pWSsrc = &(pSD->pWS[iNextWs]);

	    for (j=iNextWs; j < pSD->numWorkspaces; j++)
	    {
		memcpy (pWSdest, pWSsrc, sizeof(WmWorkspaceData));
		if (pSD->pActiveWS == pWSsrc)
		{
		    pSD->pActiveWS = pWSdest;
		}
		pWSdest++;
		pWSsrc++;
	    }
	}

	/*
	 * We now have one less workspace.
	 */
	pSD->numWorkspaces--;

	/*
	 * Update the properties that announce workspace info.
	 */
	SetWorkspaceListProperty (pSD);

	/*
	 * Update workspace presence dialog data
	 */
	UpdatePresenceWorkspaces(pSD);
	
	UpdateEwmhWorkspaceProperties(pSD);
    }
} /* END OF FUNCTION DeleteWorkspace */


/*************************************<->*************************************
 *
 *  GetClientWorkspaceInfo (pCD, manageFlags);
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the portion of client data that has to
 *  do with workspaces 
 *
 *  Inputs:
 *  ------
 *  pCD  =  pointer to client data (only partly initialized!!)
 *  manageFlags = tells us, in particular, if we're restarting.
 *
 *  Outputs:
 *  --------
 *  pCD  = updated client data
 *
 *************************************<->***********************************/

Boolean GetClientWorkspaceInfo(ClientData *pCD, long manageFlags )

{
    Atom *pIDs;
    int i;
    unsigned int numIDs = 0;
    Boolean bAll;

    /* 
     * Allocate initial workspace ID list 
     * fill with NULL IDs
     */
    if ((pCD->pWsList = (WsClientData *) 
	    XtMalloc(pCD->pSD->numWorkspaces * sizeof(WsClientData))) == NULL)
    {
	Warning (((char *)GETMESSAGE(76, 4, "Insufficient memory for client data")));
	return (False);
    }
    pCD->currentWsc = 0;
    pCD->pWorkspaceHints = NULL;
    pCD->sizeWsList = pCD->pSD->numWorkspaces;
    pCD->numInhabited = 0;		/* no valid ones yet */
    for (i = 0; i < pCD->pSD->numWorkspaces; i++)
    {
	pCD->pWsList[i].wsID = None;
	pCD->pWsList[i].iconPlace = NO_ICON_PLACE;
	pCD->pWsList[i].IPData = NULL;
	pCD->pWsList[i].iconX = 0;
	pCD->pWsList[i].iconY = 0;
	pCD->pWsList[i].iconFrameWin = None;
	pCD->pWsList[i].pIconBox = NULL;
    }
    pCD->putInAll = bAll = False;

    /* 
     * Determine initial workspace set.
     *
     * If this is a secondary window, use the hints from the
     * transient tree leader.
     *
     * Else if we're restarting, then use our own workspace presence.
     */
    if (pCD->client && ((pCD->transientLeader &&
		GetLeaderPresence(pCD, &pIDs, &numIDs)) ||
	 ((manageFlags & MANAGEW_WM_RESTART) && 
	   GetMyOwnPresence (pCD, &pIDs, &numIDs))) && numIDs)
    {
	/*
	 * Got some workspace hints! 
	 */
	pCD->putInAll = bAll;
	ProcessWorkspaceHintList (pCD, pIDs, numIDs);
    }

    if (pCD->numInhabited == 0)
    {
	/*
	 * If not in any workspaces, then put the client into
	 * the current one.
	 */
	PutClientIntoWorkspace (pCD->pSD->pActiveWS, pCD);
    }

    return (True);

} /* END OF FUNCTION GetClientWorkspaceInfo */


/*************************************<->*************************************
 *
 *  ConvertNamesToIDs (pSD, pch, ppAtoms, pNumAtoms)
 *
 *
 *  Description:
 *  -----------
 *  Takes a string containing a list of names separated by white space
 *  and converts it to a list of workspace IDs.
 * 
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  pchIn	- pointer to original string
 *  ppAtoms	- pointer to an atom pointer (for returning list pointer)
 *  pNumAtoms	- pointer to the number of atoms being returned.
 * 
 *  Outputs:
 *  -------
 *  *ppAtoms	- points to a list of atoms returned.
 *  *pNumAtoms	- the number of atoms being returned.
 *
 *  Return 	- True if some Atoms are being returned
 *
 *  Comments:
 *  --------
 *  Processes local copy of string so that pch is not modified.
 *
 *  The list of atoms returned has been dynamically allocated. 
 *  Please XtFree() it when you're done.
 *
 *************************************<->***********************************/

Boolean 
ConvertNamesToIDs(
        WmScreenData *pSD,
        unsigned char *pchIn,
        WorkspaceID **ppAtoms,
        unsigned int *pNumAtoms )

{
    unsigned char *pchLocal, *pch, *pchName;
    int num = 0;
    int numLocalIDs;
    WorkspaceID *pLocalIDs;

    if ((pLocalIDs = (WorkspaceID *) XtMalloc (WS_ALLOC_AMOUNT *
	sizeof(WorkspaceID))) == NULL)
    {
	Warning (((char *)GETMESSAGE(76, 5, "Insufficient Memory (ConvertNamesToIDs)")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }
    numLocalIDs = WS_ALLOC_AMOUNT;

   if (pchIn && (pchLocal = (unsigned char *) XtMalloc(1+strlen((char *)pchIn))))
   {
        strcpy ((char *)pchLocal, (char *)pchIn);
	pch = pchLocal;

	while ((pchName = GetSmartString (&pch)))
	{
	    int iwsx;
	    XmString xms;

	    /*
	     * Check workspace for workspace titles; map to 
	     * workspace names.
	     */
            xms = XmStringCreateLocalized ((char *)pchName);
	    for (iwsx = 0; iwsx < pSD->numWorkspaces; iwsx++)
	    {
		if (XmStringCompare (xms, pSD->pWS[iwsx].title))
		{
		    break;
		}
	    }
	    XmStringFree (xms);

	    if (iwsx < pSD->numWorkspaces)
	    {
	       /*
		* Found a workspace title we've got,
		* use id for workspace name
		*/
		pLocalIDs[num] = pSD->pWS[iwsx].id;
		num++;
	    }
	    else 
	    {
		/*
		 * Try for match on workspace name
		 */
		pLocalIDs[num] = (WorkspaceID) 
			    XInternAtom (DISPLAY, (char *)pchName, False);
		num++;
	    }

	    if (num >= numLocalIDs)
	    {
		/* list too small */
		numLocalIDs += WS_ALLOC_AMOUNT;
		if ((pLocalIDs = (WorkspaceID *) XtRealloc ((char *)pLocalIDs,
			    numLocalIDs * sizeof(WorkspaceID))) == NULL)
		{
		    Warning (((char *)GETMESSAGE(76, 6, "Insufficient Memory (ConvertNamesToIDs)")));
		    ExitWM (WM_ERROR_EXIT_VALUE);
		}
	    }
	}

	XtFree ((char *)pchLocal);
    }

    *ppAtoms = pLocalIDs;
    *pNumAtoms = num;
    return (num != 0);
    
} /* END OF FUNCTION ConvertNamesToIDs */


/*************************************<->*************************************
 *
 *  CheckForPutInAllRequest (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  Tests for the presence of the "all" atom in the atom list
 *  and sets the "putInAll" flag on the client.
 * 
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  pIDs	- pointer to ID list
 *  numIDs	- number of IDs in list
 * 
 *  Outputs:
 *  -------
 *  pCD		- putInAll member may be set
 *
 *************************************<->***********************************/

void 
CheckForPutInAllRequest(
        ClientData *pCD,
        Atom *pIDs,
        unsigned int numIDs )

{
    unsigned int i;

    for (i = 0; (i < numIDs) && !(pCD->putInAll); i++)
    {
	if (pIDs[i] == wmGD.xa_ALL_WORKSPACES)
	{
	    pCD->putInAll = True;
	    break;
	}
    }
    
} /* END OF FUNCTION CheckForPutInAllRequest */


/*************************************<->*************************************
 *
 *  PutClientIntoWorkspace (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function updates the data for the client and workspace to
 *  reflect the presence of the client in the workspace.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD = pointer to client data
 *
 *  Outputs:
 *  --------
 *
 *************************************<->***********************************/

void 
PutClientIntoWorkspace(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    int i = pCD->numInhabited;
    int iAdded, j, k;

    /* insure the client's got enough workspace data */
    if (pCD->sizeWsList < pCD->pSD->numWorkspaces)
    {
	iAdded = pCD->pSD->numWorkspaces - pCD->sizeWsList;

	pCD->sizeWsList = pCD->pSD->numWorkspaces;
	pCD->pWsList = (WsClientData *) 
		XtRealloc((char *)pCD->pWsList, 
		    (pCD->pSD->numWorkspaces * sizeof(WsClientData)));

	/* intialized new data */
	j = pCD->sizeWsList - 1;
	for (j=1; j <= iAdded; j++)
	{
	    k = pCD->sizeWsList - j;
	    pCD->pWsList[k].iconPlace = NO_ICON_PLACE;
		pCD->pWsList[k].IPData = NULL;
	    pCD->pWsList[k].iconX = 0;
	    pCD->pWsList[k].iconY = 0;
	    pCD->pWsList[k].iconFrameWin = (Window) 0;
	    pCD->pWsList[k].pIconBox = NULL;
	}
    }


    /* update the client's list of workspace data */
    pCD->pWsList[i].wsID = pWS->id; 
    pCD->numInhabited++;

    if (!(pCD->clientFlags & WM_INITIALIZATION))
    {
	/* 
	 * Make sure there's an icon 
	 * (Don't do this during initialization, the pCD not
	 * ready for icon making yet).
	 */
	InsureIconForWorkspace (pWS, pCD);
    }

    /* update the workspace list of clients */
    AddClientToWsList (pWS, pCD);

} /* END OF FUNCTION PutClientIntoWorkspace */


/*************************************<->*************************************
 *
 *  TakeClientOutOfWorkspace (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function updates the data for the client and the workspace
 *  to reflect the removal of the client from the workspace.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD = pointer to client data
 *
 *  Outputs:
 *  --------
 *
 *************************************<->***********************************/

void 
TakeClientOutOfWorkspace(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    int ixA;
    Boolean Copying = False;
    WsClientData *pWsc;

    if (pWS && pCD && ClientInWorkspace(pWS, pCD))
    {
	/*
	 * Clean up icon
	 */
	if (!pCD->transientLeader)
	{
	    pWsc = GetWsClientData (pWS, pCD);

	    if ((pCD->pSD->useIconBox) && 
		(pWsc->pIconBox) &&
		(pCD->clientFunctions & MWM_FUNC_MINIMIZE))
	    {
		DeleteIconFromBox (pWS->pIconBox, pCD);
	    }
	    else if (wmGD.iconAutoPlace)
	    {
		/* 
		 * Free up root icon spot 
		 */

	        if ((pWsc->iconPlace != NO_ICON_PLACE) &&
		    (pWsc->IPData->placeList[pWsc->iconPlace].pCD == pCD))
		{
		    pWsc->IPData->placeList[pWsc->iconPlace].pCD = NULL;
		    pWsc->iconPlace = NO_ICON_PLACE;
			pWsc->IPData = NULL;
		}
	    }
	}

	/* 
	 *  Remove the selected workspace and copy the remaining ones
	 *  up. (Do piece-wise to avoid overlapping copy.)
	 */
	for (ixA = 0; ixA < pCD->numInhabited; ixA++)
	{
	    if (Copying)
	    {
		memcpy (&pCD->pWsList[ixA-1], &pCD->pWsList[ixA], 
			sizeof(WsClientData));
	    }
	    else if (pCD->pWsList[ixA].wsID == pWS->id)
	    {
		/* 
		 *  This is the one we're removing, start copying here.
		 */
		Copying = True;
	    }
	}

	/* 
	 * Decrement the number of workspaces inhabited.
	 */
	pCD->numInhabited--;

	/* update the workspaces list of clients */
	RemoveClientFromWsList (pWS, pCD);
    }
#ifdef DEBUG
    else
    {
	Warning("TakeClientOutOfWorkspace: null workspace passed in.");
    }
#endif /* DEBUG */


} /* END OF FUNCTION TakeClientOutOfWorkspace */


/*************************************<->*************************************
 *
 *  GetWorkspaceData (pSD, wsID)
 *
 *
 *  Description:
 *  -----------
 *  This function finds the data that is associated with a workspace ID.
 *
 *  Inputs:
 *  ------
 *  pSD   = pointer to screen data
 *  wsID  =  workspace ID
 *
 *  Outputs:
 *  --------
 *  Function returns a pointer to the workspace data if successful,
 *  or NULL if unsuccessful.
 *
 *************************************<->***********************************/

WmWorkspaceData * 
GetWorkspaceData(
        WmScreenData *pSD,
        WorkspaceID wsID )

{
    WmWorkspaceData *pWS = NULL;
    int i;

    for (i=0; i < pSD->numWorkspaces; i++)
    {
	if (pSD->pWS[i].id == wsID) 
	{
	    pWS = &pSD->pWS[i];
	    break;
	}
    }

#ifdef DEBUG
    if (!pWS)
    {
	/* failed to find one */
        Warning ("Failed to find workspace data");
    }
#endif

    return (pWS);

}  /* END OF FUNCTION GetWorkspaceData */


/*************************************<->*************************************
 *
 *  GenerateWorkspaceName (pSD, wsnum)
 *
 *
 *  Description:
 *  -----------
 *  This function generates and returns a workspace string name from
 *  a small number passed in.
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  wsNum  =  number for workspace
 *
 * 
 *  Outputs:
 *  -------
 *  returns pointer to statically allocated data. You must copy it
 *  to your local buffer.
 *
 *  Comments:
 *  ---------
 *  Name is of the form ws<n> where <n> is a number.
 * 
 *************************************<->***********************************/

unsigned char * 
GenerateWorkspaceName(
        WmScreenData *pSD,
        int wsnum )

{
    static unsigned char nameReturned[20];
    int i;

    /*
     * Nice n-squared algorithm...
     * (This should be OK for small number of workspaces)
     */
    for (i=0; i <= pSD->numWorkspaces; i++)
    {
	/* generate a name */
	snprintf ((char *)nameReturned, 20, "ws%d", i);
	if (!DuplicateWorkspaceName (pSD, nameReturned, wsnum))
	    break;
    }

    return (nameReturned);

}  /* END OF FUNCTION GenerateWorkspaceName */


/*************************************<->*************************************
 *
 *  InWindowList (w, wl, num)
 *
 *
 *  Description:
 *  -----------
 *  This function determines if a window is in a list of windows
 *
 *  Inputs:
 *  ------
 *  w = window of interest
 *  wl = list of windows
 *  num = number of windows in wl
 *
 * 
 *  Outputs:
 *  -------
 *  The function returns "True" if "w" appears in "wl"
 *
 *************************************<->***********************************/

Boolean 
InWindowList(
        Window w,
        Window wl[],
        int num )

{
    int i;
    Boolean rval = False;

    for (i = 0; (i < num) && !rval; i++)
    {
	if (w == wl[i])
	{
	    rval = True;
	}
    }

    return (rval);

}   /* END OF FUNCTION InWindowList */


/*************************************<->*************************************
 *
 *  ClientInWorkspace (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function determines if a client is in a workspace
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  The function returns "True" if client pCD is in workspace pWS
 *
 *************************************<->***********************************/

Boolean 
ClientInWorkspace(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    int i;
    Boolean rval = False;

    for (i = 0; (i < pCD->numInhabited) && !rval; i++)
    {
	if (pWS->id == pCD->pWsList[i].wsID)
	{
	    rval = True;
	}
    }

    return (rval);

}   /* END OF FUNCTION ClientInWorkspace */

/*************************************<->*************************************
 *
 *  GetWsClientData (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function returns a pointer to the client's specific data for
 *  this workspace
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  The function returns a pointer to the client's data for this
 *  workspace. If the client isn't in the workspace, an error is 
 *  printed and the first datum in the workspace list is returned.
 *
 *************************************<->***********************************/

WsClientData * 
GetWsClientData(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    int i;
    WsClientData *pWsc = NULL;

    for (i = 0; (i < pCD->numInhabited) && !pWsc; i++)
    {
	if (pWS->id == pCD->pWsList[i].wsID)
	{
	    pWsc = &pCD->pWsList[i];
	}
    }

    if (!pWsc)
    {
	pWsc = &pCD->pWsList[0];
    }

    return (pWsc);

}   /* END OF FUNCTION GetWsClientData */

/*************************************<->*************************************
 *
 *  SetClientWsIndex (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function sets the index into the client's array of workspace
 *  specific data. This index points to the data to be used for the
 *  currently active workspace.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  The function returns an index as described above. If the client is
 *  not in the currently active workspace, then the index returned is 0.
 *
 *************************************<->***********************************/

void 
SetClientWsIndex(
        ClientData *pCD )

{
    int i;
    WmWorkspaceData *pWS = pCD->pSD->pActiveWS;

    for (i = 0; (i < pCD->numInhabited); i++)
    {
	if (pWS->id == pCD->pWsList[i].wsID)
	{
	    break;
	}
    }

    if (i >= pCD->numInhabited)
    {
	i = 0;
    }

    pCD->currentWsc = i;

}   /* END OF FUNCTION SetClientWsIndex */


/*************************************<->*************************************
 *
 *  InsureUniqueWorkspaceHints (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function processes the workspace hints and removes duplicates.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  May modify *pWorkspaceHints and numWorkspaceHints
 *
 *************************************<->***********************************/

static void 
InsureUniqueWorkspaceHints(
        ClientData *pCD )

{
    int next, trail, i;
    WorkspaceID *pID;
    Boolean  duplicate;


    if (pCD->numWorkspaceHints < 2) return;

    pID = pCD->pWorkspaceHints;

    trail = 0;
    next = 1;

    while (next < pCD->numWorkspaceHints)
    {
	duplicate = False;
	for (i = 0; i < next; i++)
	{
	    if (pID [next] == pID [i])
	    {
		/* duplicate found! */
		duplicate = True;
		break;
	    }
	}

	if (duplicate)
	{
	    /* skip duplicates */
	    next++;
	}
	else
	{
	    /* not a duplicate */
	    trail++;
	    if (next > trail)
	    {
		/*
		 * We need to copy up over an old duplicate
		 */
		pID [trail] = pID [next];
	    }
	}
	next++;
    }

    pCD->numWorkspaceHints = trail+1;

}   /* END OF FUNCTION InsureUniqueWorkspaceHints */


/*************************************<->*************************************
 *
 *  ProcessWorkspaceHintList (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function processes a list of workspace hints for a client.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of IDs in the list
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
ProcessWorkspaceHintList(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    int i;
    WmWorkspaceData *pWS;


    if (numIDs > 0)
    {
	/*
	 * Keep these hints; make sure there are no duplicate
	 * workspace requests.
	 */
	pCD->pWorkspaceHints = pIDs;
	pCD->numWorkspaceHints = numIDs;
	InsureUniqueWorkspaceHints (pCD);
	numIDs = pCD->numWorkspaceHints;

	if (pCD->pWorkspaceHints)
	{
	    /*
	     *  Process request to put window in all workspaces
	     */
	    CheckForPutInAllRequest (pCD, pIDs, numIDs);

	    if (pCD->putInAll)
	    {
		for (i=0; i<pCD->pSD->numWorkspaces; i++)
		{
		    PutClientIntoWorkspace (&pCD->pSD->pWS[i], pCD);
		}
	    }
	    else
	    {
		for (i=0; i<numIDs; i++)
		{
		    /*
		     * Put the client into requested workspaces that
		     * exist.
		     */
		    if ((pWS = GetWorkspaceData (pCD->pSD, 
						pCD->pWorkspaceHints[i])))
		    {
			PutClientIntoWorkspace (pWS, pCD);
		    }
		}
	    }
	}
    }

}   /* END OF FUNCTION ProcessWorkspaceHintList */

/*************************************<->*************************************
 *
 *  RemoveSingleClientFromWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a single client from a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
RemoveSingleClientFromWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    int i;
    WmWorkspaceData *pWS;

    for (i=0; i < numIDs; i++)
    {
	/*
	 *  Remove the client from the specified workspaces 
	 */
	if ((pWS = GetWorkspaceData (pCD->pSD, pIDs[i])) &&
	    (ClientInWorkspace (pWS, pCD)))
	{
	    /*
	     * If this workspace is active, then make the
	     * window unseen.  We only need to call
	     * SetClientState on the main window, the
	     * transients will get taken care of in there.
	     */
	    if ((pWS == pCD->pSD->pActiveWS) &&
		(pCD->transientLeader == NULL) &&
		!(pCD->clientState & UNSEEN_STATE))
	    {
		SetClientState (pCD, 
		    (pCD->clientState | UNSEEN_STATE), CurrentTime);
	    }
	    TakeClientOutOfWorkspace (pWS, pCD);

	    /* 
	     * Update the presence property
	     */
	    UpdateWorkspacePresenceProperty (pCD);
	}
    }

}   /* END OF FUNCTION RemoveSingleClientFromWorkspaces */

/*************************************<->*************************************
 *
 *  RemoveSubtreeFromWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a transient subtree from a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
RemoveSubtreeFromWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    ClientData *pNext;

    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* process all children first */
	if (pNext->transientChildren)
	{
	    RemoveSubtreeFromWorkspaces (pNext, pIDs, numIDs);
	}
	else
	{
	    RemoveSingleClientFromWorkspaces (pNext, pIDs, numIDs);
	}
	pNext = pNext->transientSiblings;
    }

    /* process the primary window */
    RemoveSingleClientFromWorkspaces (pCD, pIDs, numIDs);


}   /* END OF FUNCTION RemoveSubtreeFromWorkspaces */


/******************************<->*************************************
 *
 *  pIDs = GetListOfOccupiedWorkspaces (pCD, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function creates a list of occupied workspaces of a particular
 *  client, EXCLUDING the current workspace.
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 *
 *  Comment
 *  -------
 *  memory for pIDs is allocated with XtMalloc and should be 
 *  freed with XtFree.
 *
 *
 ******************************<->***********************************/
WorkspaceID * 
GetListOfOccupiedWorkspaces(
        ClientData *pCD,
        int *numIDs )
{
    int i;

    WorkspaceID *pLocalIDs = NULL;

    WorkspaceID activeWsID = pCD->pSD->pActiveWS->id;

    *numIDs = 0;

    if ((pLocalIDs = (WorkspaceID *) XtMalloc (pCD->numInhabited *
	sizeof(WorkspaceID))) == NULL)
    {
	Warning (((char *)GETMESSAGE(76, 7, "Insufficient memory")));
	return (NULL);
    }

    for (i = 0; i < pCD->numInhabited; i++)
    {
	if (activeWsID != pCD->pWsList[i].wsID)
	{
	      pLocalIDs[(*numIDs)++] = pCD->pWsList[i].wsID;
	}
    }

    return(pLocalIDs);

}   /* END OF FUNCTION GetListOfOccupiedWorkspaces */


/******************************<->*************************************
 *
 *   HonorAbsentMapBehavior(pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a client to the current workspace and
 *   if (pCD->absentMapBehavior == AMAP_BEHAVIOR_MOVE)
 *  removes the client from the other  workspaces 
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 ******************************<->***********************************/

void 
HonorAbsentMapBehavior(
        ClientData *pCD)
{
    int inWorkspace = 0;

    if (pCD->absentMapBehavior == AMAP_BEHAVIOR_MOVE)
    {
	int wsCnt;

	/* 
	 * Remove from other workspaces
	 */
	for (wsCnt = 0; wsCnt < pCD->numInhabited; wsCnt = inWorkspace)
	{
	    if (pCD->pWsList[wsCnt].wsID != pCD->pSD->pActiveWS->id)
	    {
		RemoveClientFromWorkspaces (pCD, 
					    &pCD->pWsList[wsCnt].wsID, 1);
	    }
	    else inWorkspace++;
	}
    }

    if (inWorkspace == 0)
	AddClientToWorkspaces (pCD, &ACTIVE_WS->id, 1);

}   /* END OF FUNCTION HonorAbsentMapBehavior */


/******************************<->*************************************
 *
 *  RemoveClientFromWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a client from a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 ******************************<->***********************************/

void 
RemoveClientFromWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    ClientData *pcdLeader;

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    RemoveSubtreeFromWorkspaces (pcdLeader, pIDs, numIDs);


}   /* END OF FUNCTION RemoveClientFromWorkspaces */


/*************************************<->*************************************
 *
 *  AddSingleClientToWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a single client to a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
AddSingleClientToWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    int i;
    WmWorkspaceData *pWS;

    for (i=0; i < numIDs; i++)
    {
	/*
	 *  Add the client to the specified workspaces if 
	 *  it is not already there.
	 */
	if ((pWS = GetWorkspaceData (pCD->pSD, pIDs[i])) &&
	    (!ClientInWorkspace (pWS, pCD)))
	{
	    PutClientIntoWorkspace (pWS, pCD);

	    if ((pWS == PSD_FOR_CLIENT(pCD)->pActiveWS) &&
		(pCD->transientLeader == NULL) &&
		(pCD->clientState & UNSEEN_STATE))
	    {
		SetClientState (pCD, 
		    (pCD->clientState & ~UNSEEN_STATE), CurrentTime);
	    }

	    /* 
	     * Update the presence property (only on transient leader)
	     */
	    UpdateWorkspacePresenceProperty (pCD);

	}
    }
} /* END OF FUNCTION AddSingleClientToWorkspace */


/*************************************<->*************************************
 *
 *  AddSubtreeToWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a client subtree to a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (head of subtree)
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
AddSubtreeToWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    ClientData *pNext;

    pNext = pCD->transientChildren;
    while (pNext)
    {
	/* process all children first */
	if (pNext->transientChildren)
	{
	    AddSubtreeToWorkspaces (pNext, pIDs, numIDs);
	}
	else
	{
	    AddSingleClientToWorkspaces (pNext, pIDs, numIDs);
	}
	pNext = pNext->transientSiblings;
    }

    /* process the primary window */
    AddSingleClientToWorkspaces (pCD, pIDs, numIDs);


}   /* END OF FUNCTION AddSubtreeToWorkspaces */


/*************************************<->*************************************
 *
 *  AddClientToWorkspaces (pCD, pIDs, numIDs)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a transient tree to a list of workspaces
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pIDs = pointer to a list of workspace IDs
 *  numIDs = number of workspace IDs
 * 
 *  Outputs:
 *  -------
 *
 *************************************<->***********************************/

void 
AddClientToWorkspaces(
        ClientData *pCD,
        WorkspaceID *pIDs,
        unsigned int numIDs )

{
    ClientData *pcdLeader;

    pcdLeader = (pCD->transientLeader) ? FindTransientTreeLeader (pCD) : pCD;

    AddSubtreeToWorkspaces (pcdLeader, pIDs, numIDs);

}   /* END OF FUNCTION AddClientToWorkspaces */



/*************************************<->*************************************
 *
 *  AddClientToWsList (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a client to a list of clients in a workspace
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pWS = pointer to workspace data
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *************************************<->***********************************/

void 
AddClientToWsList(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    if (pWS->numClients >= pWS->sizeClientList)
    {
	if (pWS->sizeClientList == 0)
	{
	    pWS->ppClients = (ClientData **) 
		XtMalloc (WINDOW_ALLOC_AMOUNT * sizeof(ClientData *));
	}
	else
	{
	    pWS->ppClients = (ClientData **) 
		XtRealloc ((char *)pWS->ppClients, 
			 (pWS->sizeClientList + WINDOW_ALLOC_AMOUNT) * 
			 sizeof(ClientData *));
	}

	if (!pWS->ppClients)
	{
	    Warning (((char *)GETMESSAGE(76, 9, "Insufficient memory to add window to workspace")));
	    ExitWM(WM_ERROR_EXIT_VALUE);
	}

	pWS->sizeClientList += WINDOW_ALLOC_AMOUNT;
    }

    if (pWS->numClients < pWS->sizeClientList)
    {
	pWS->ppClients[pWS->numClients] = pCD;
	pWS->numClients++;
    }
}   /* END OF FUNCTION AddClientToWsList */


/*************************************<->*************************************
 *
 *  RemoveClientFromWsList (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function removes a client from a list of clients in a workspace
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  pWS = pointer to workspace data
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *************************************<->***********************************/

void 
RemoveClientFromWsList(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    int src, dest;

    for (dest = 0; dest < pWS->numClients; dest++)
    {
	if (pWS->ppClients[dest] == pCD)
	{
	    break;
	}
    }

    for (src = dest+1; src < pWS->numClients; src++, dest++)
    {
	pWS->ppClients[dest] = pWS->ppClients[src];
    }

    pWS->numClients--;

}   /* END OF FUNCTION RemoveClientFromWsList */


/*************************************<->*************************************
 *
 *  Boolean
 *  F_CreateWorkspace (args, pCD, event)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  args = ...
 *  pCD = ...
 *  event = ...
 * 
 *  Outputs:
 *  -------
 *  Return = ...
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean 
F_CreateWorkspace(
        String args,
        ClientData *pCD,
        XEvent *event )

{
    WmScreenData *pSD = ACTIVE_PSD;

    if (pSD->numWorkspaces >= MAX_WORKSPACE_COUNT)
    {
	char buffer[MAXWMPATH];
	/*
	 * At the maximum number of allowed workspaces.
	 */
	sprintf (buffer, 
	((char *)GETMESSAGE(76, 14, "Maximum number of workspaces is %d. New workspace was not created.")), MAX_WORKSPACE_COUNT);
	Warning (buffer);
    }
    else
    {
	CreateWorkspace (ACTIVE_PSD, (unsigned char *)args);
    }

    return (TRUE);

} /* END OF FUNCTION F_CreateWorkspace */


/*************************************<->*************************************
 *
 *  Boolean
 *  F_DeleteWorkspace (args, pCD, event)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  args = ...
 *  pCD = ...
 *  event = ...
 * 
 *  Outputs:
 *  -------
 *  Return = ...
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean 
F_DeleteWorkspace(
        String args,
        ClientData *pCD,
        XEvent *event )

{
    WmScreenData *pSD = ACTIVE_PSD;
    WmWorkspaceData *pWS = NULL;
    int i;

    if (args == NULL)
    {
	pWS= ACTIVE_WS;
    } 
    else
    {
	for (i=0; i<pSD->numWorkspaces; i++)
	{
	    if (!strcmp(pSD->pWS[i].name, args))
	    {
		pWS = &(pSD->pWS[i]);
		break;
	    }
	}
    }

    if (pWS)
	DeleteWorkspace (pWS);

    return (TRUE);

} /* END OF FUNCTION F_DeleteWorkspace */


/*************************************<->*************************************
 *
 *  Boolean
 *  F_GotoWorkspace (args, pCD, event)
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  args = ...
 *  pCD = ...
 *  event = ...
 * 
 *  Outputs:
 *  -------
 *  Return = ...
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean 
F_GotoWorkspace(
        String args,
        ClientData *pCD,
        XEvent *event )

{
    WorkspaceID wsID;
    WmWorkspaceData *pWS;

    wsID = XInternAtom (DISPLAY, args, False);
    pWS = GetWorkspaceData (ACTIVE_PSD, wsID);

    if (pWS)
    {
	ChangeToWorkspace (pWS);
    }
    return (TRUE);

} /* END OF FUNCTION F_GotoWorkspace */



/*************************************<->*************************************
 *
 *  Boolean
 *  F_AddToAllWorkspaces (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  Puts a client into all workspaces
 *
 *
 *  Inputs:
 *  ------
 *  args = ...
 *  pCD = pointer to client data
 *  event = ...
 *
 * 
 *  Outputs:
 *  -------
 *  Return = True
 *
 *
 *  Comments:
 *  --------
 *  The list of Ids returned has been privately allocated. Copy
 *  if you want to save or do anything with it.
 * 
 *************************************<->***********************************/

Boolean 
F_AddToAllWorkspaces(
        String args,
        ClientData *pCD,
        XEvent *event )

{
    WmScreenData *pSD;
    int i;

    if (pCD && (pCD->wsmFunctions & WSM_FUNCTION_OCCUPY_WS))
    {
	pSD = pCD->pSD;

	ReserveIdListSpace (pSD->numWorkspaces);

	for (i = 0; i < pSD->numWorkspaces; i++)
	{
	    pResIDs[i] = pSD->pWS[i].id;
	}

	AddClientToWorkspaces (pCD, pResIDs, pSD->numWorkspaces);

	pCD->putInAll = True;
    }

    return (True);

} /* END OF FUNCTION F_AddToAllWorkspaces */


/*************************************<->*************************************
 *
 *  Boolean
 *  F_Remove (args, pCD, event)
 *
 *
 *  Description:
 *  -----------
 *  Removes a client from the current workspace
 *
 *
 *  Inputs:
 *  ------
 *  args = ...
 *  pCD = pointer to client data
 *  event = ...
 *
 * 
 *  Outputs:
 *  -------
 *  Return = True
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean 
F_Remove(
        String args,
        ClientData *pCD,
        XEvent *event )

{
    Boolean rval = False;

    /*
     * Only remove if in more than one workspace.
     */
    if ((pCD && (pCD->wsmFunctions & WSM_FUNCTION_OCCUPY_WS)) &&
	(pCD->numInhabited > 1))
    {
	if (ClientInWorkspace (ACTIVE_WS, pCD))
	{
	    RemoveClientFromWorkspaces (pCD, &ACTIVE_WS->id, 1);
            pCD->putInAll = False;
	}
    }

    return (rval);

} /* END OF FUNCTION F_Remove */



/*************************************<->*************************************
 *
 *  GetCurrentWorkspaceIndex (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Returns an index into the screens array of workspace structures
 *  for the current workspace.
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
int 
GetCurrentWorkspaceIndex(
        WmScreenData *pSD )
{

    int i;

    for (i = 0 ; i < pSD->numWorkspaces; i++)
    {
	if (pSD->pWS[i].id == pSD->pActiveWS->id)
	break;
    }

    if (i >= pSD->numWorkspaces)
    {
	/* failed to find workspace!!! How did that happen??? */
	i = 0;
#ifdef DEBUG
	Warning ("Failed to find workspace index");
#endif /* DEBUG */
    }

    return(i);
} /* END OF FUNCTION GetCurrentWorkspaceIndex */


/*************************************<->*************************************
 *
 *  void
 *  InsureIconForWorkspace (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  Makes sure an icon exists for the workspace
 *
 *
 *  Inputs:
 *  ------
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
InsureIconForWorkspace(
        WmWorkspaceData *pWS,
        ClientData *pCD )

{
    WsClientData *pWsc;

    if (pCD->clientFunctions & MWM_FUNC_MINIMIZE)
    {
	pWsc = GetWsClientData (pWS, pCD);
	if ((pCD->pSD->useIconBox) && !(pCD->clientFlags & CLIENT_WM_CLIENTS))
	{
	    /*
	     * Create a new widget for the icon box
	     */
	    if (MakeIcon (pWS, pCD)) 
	    {
		XSaveContext (DISPLAY, pWsc->iconFrameWin, 
			wmGD.windowContextType, (caddr_t)pCD);

		if (pCD->iconWindow && pWsc->iconFrameWin)
		{
		    XGrabButton (DISPLAY, AnyButton, AnyModifier, 
			pWsc->iconFrameWin, True,
			ButtonPressMask|ButtonReleaseMask|
			    ButtonMotionMask,
			GrabModeAsync, GrabModeAsync, None, 
			wmGD.workspaceCursor);
		}

		ShowClientIconState (pCD, (pCD->clientState & ~UNSEEN_STATE));
	    }
	}
	else
	{
	    /* 
	     * Reuse existing icon in new workspaces. Suggest
	     * icon position in current WS as position of icon
	     * in new WS.
	     */
	    pWsc->iconFrameWin = pCD->pWsList[0].iconFrameWin;
	    pWsc->iconX = ICON_X(pCD); 
	    pWsc->iconY = ICON_Y(pCD);

		pWsc->IPData = PositionToPlacementData(pWS, pWsc->iconX, pWsc->iconY);

	    if ((pCD->clientState & ~UNSEEN_STATE) != MINIMIZED_STATE)
	    {
		pWsc->iconPlace = NO_ICON_PLACE;
	    }
	    else if (!wmGD.iconAutoPlace)
	    {
               PlaceIconOnScreen (pCD, &pWsc->iconX, &pWsc->iconY);
	    }
	    else	/* icon auto placement */
	    {
		pWsc->iconPlace = 
		CvtIconPositionToPlace (pWsc->IPData,
					pWsc->iconX, pWsc->iconY);

		if (pWsc->IPData->placeList[pWsc->iconPlace].pCD)
		{
		    /* The spot is already occupied!  Find a 
		       spot nearby. */
		    pWsc->iconPlace = 
		    FindIconPlace (pCD, pWsc->IPData, pWsc->iconX,
				pWsc->iconY);

		    if (pWsc->iconPlace == NO_ICON_PLACE)
		    {
			/* Can't find a spot close by. Use the
			   next available slot */
			pWsc->iconPlace = GetNextIconPlace (pWsc->IPData);
			if (pWsc->iconPlace == NO_ICON_PLACE)
			{
			    pWsc->iconPlace =
				CvtIconPositionToPlace (pWsc->IPData,
                                                pCD->clientX,
                                                pCD->clientY);
			}
		    }
		}
		CvtIconPlaceToPosition (pWsc->IPData, pWsc->iconPlace, 
					&pWsc->iconX, &pWsc->iconY);

		
		if (!(pWsc->IPData->placeList[pWsc->iconPlace].pCD))
		{
		    pWsc->IPData->placeList[pWsc->iconPlace].pCD = pCD;
		}
	    }
	}
    }
} /* END OF FUNCTION InsureIconForWorkspace */


/*************************************<->*************************************
 *
 *  Boolean
 *  GetLeaderPresence (pCD, pIDs, pnumIDs)
 *
 *
 *  Description:
 *  -----------
 *  Gets the workspace presence of the transient tree leader for a
 *  client.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  ppIDs = pointer to pointer to list of workspace ids
 *  pnumIDs = pointer to number of workspace ids
 *
 * 
 *  Outputs:
 *  -------
 *  *ppIDS = list of workspace IDs
 *  *pnumIDs = number of workspace IDs in list
 *
 *  Return = true on success
 *
 *
 *  Comments:
 *  --------
 *  ID list is dynamically allocated, please XtFree() it when you're
 *  done.
 * 
 *************************************<->***********************************/

Boolean 
GetLeaderPresence(
        ClientData *pCD,
        WorkspaceID **ppIDs,
        unsigned int *pnumIDs )

{
    ClientData *pcdLeader;
    int i;
    Boolean rval = False;
    WorkspaceID *pLocalIDs;

    if ((pLocalIDs = (WorkspaceID *) XtMalloc (pCD->pSD->numWorkspaces *
	sizeof(WorkspaceID))) == NULL)
    {
	Warning (((char *)GETMESSAGE(76, 10, "Insufficient Memory (GetLeaderPresence)")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

    /*
     * Make up list of workspaces for primary window
     */
    if (pCD->transientLeader)
    {
	pcdLeader = FindTransientTreeLeader (pCD);

	for (i = 0; i < pcdLeader->numInhabited; i++)
	{
	    pLocalIDs[i] = pcdLeader->pWsList[i].wsID;
	}

	*ppIDs = pLocalIDs;
	*pnumIDs = pcdLeader->numInhabited;
	rval = True;
    }

    return (rval);

} /* END OF FUNCTION GetLeaderPresence */


/*************************************<->*************************************
 *
 *  Boolean
 *  GetMyOwnPresence (pCD, pIDs, pnumIDs)
 *
 *
 *  Description:
 *  -----------
 *  Returns the current workspace presence for the client
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data
 *  ppIDs = pointer to pointer to list of workspace ids
 *  pnumIDs = pointer to number of workspace ids
 *
 * 
 *  Outputs:
 *  -------
 *  *ppIDS = list of workspace IDs
 *  *pnumIDs = number of workspace IDs in list
 *
 *  Return = true on success
 *
 *
 *  Comments:
 *  --------
 *  ID list is dynamically allocated; XtFree() it when you're done.
 * 
 *************************************<->***********************************/

Boolean GetMyOwnPresence(ClientData *pCD,
	WorkspaceID **ppIDs, unsigned int *pnumIDs )

{
    /*
     * Get the workspace presence property 
     */
    if (HasProperty (pCD, wmGD.xa_MWM_WORKSPACE_PRESENCE))
    {
		WorkspaceID *IDs;
		unsigned long nIDs = 0;
		Atom actualType;
		int actualFormat;
		unsigned long leftover = 0;
		Status status;
		
		nIDs = pCD->pSD->numWorkspaces;
		
		status = XGetWindowProperty(DISPLAY, pCD->client,
			wmGD.xa_MWM_WORKSPACE_PRESENCE, 0L,
			nIDs, False, wmGD.xa_MWM_WORKSPACE_PRESENCE,
			&actualType, &actualFormat, &nIDs, &leftover,
			(unsigned char **)&IDs);

		if(status != Success) return False;	
		
		if( (actualType != wmGD.xa_MWM_WORKSPACE_PRESENCE)
			|| (actualFormat != 32) || leftover) {

			XFree(IDs);
			return False;
		}
		*ppIDs = IDs;
	    *pnumIDs = nIDs;
    }

    return True;

} /* END OF FUNCTION GetMyOwnPresence */


/*************************************<->*************************************
 *
 *  void
 *  ReserveIdListSpace (numIDs)
 *
 *
 *  Description:
 *  -----------
 *  Insures that there is enough room in our privately allocated
 *  list of workspace IDs
 *
 *
 *  Inputs:
 *  ------
 *  numIDs = number of workspace ids
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/

void 
ReserveIdListSpace(
        int numIDs )

{
    if (numResIDs == 0)
    {
	pResIDs = (WorkspaceID *) 
		    XtMalloc (numIDs * sizeof (WorkspaceID));
	if (pResIDs)
	{
	    numResIDs = numIDs;
	}
    }
    else if (numResIDs < numIDs)
    {
	pResIDs = (WorkspaceID *) XtRealloc ((char *)pResIDs, 
					numIDs * sizeof (WorkspaceID));

	numResIDs = (pResIDs)? numIDs : 0;
    }

    if (pResIDs == NULL)
    {
	Warning (((char *)GETMESSAGE(76, 11, "Insufficient memory")));
	ExitWM (WM_ERROR_EXIT_VALUE);
    }

} /* END OF FUNCTION ReserveIdListSpace */


/*************************************<->*************************************
 *
 *  DuplicateWorkspaceName (pSD, name, num)
 *
 *
 *  Description:
 *  -----------
 *  This function searches the first "num" workspace names to see if the
 *  passed "name" duplicates any workspace name defined so far.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  name = potential string name for workspace
 *  num = number of workspaces to check against
 * 
 *  Outputs:
 *  -------
 *  Return = True if a dupicate was found
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Boolean
DuplicateWorkspaceName (WmScreenData *pSD, unsigned char *name, int num)
{
    int i;
    Boolean duplicate = False;

    if (pSD && pSD->pWS)
    {
	for (i = 0; (i < num) && !duplicate; i++)
	{
	    if (!strcmp (pSD->pWS[i].name, (char *)name))
	    {
		duplicate = True;
	    }
	}
    }

    return (duplicate);
}

#ifdef DEBUG
void PrintWorkspaceList (WmScreenData *pSD)
{
    int i, j, k;
    WmWorkspaceData *pWS;
    ClientData *pCD;
    ClientData *pClients[500];
    int numSaved = 0;
    Boolean Saved;

    fprintf (stderr, "Screen: %d\n", pSD->screen);

    for (i =0; i < pSD->numWorkspaces; i++)
    {
	pWS = &pSD->pWS[i];

	fprintf (stderr, "\nWorkspace %s contains: \n", pWS->name);

	for (j = 0; j < pWS->numClients; j++)
	{
	    pCD = pWS->ppClients[j];
	    fprintf (stderr, "\t%s\n", pCD->clientName);

	    Saved = False;
	    for (k = 0; k < numSaved; k++)
	    {
		if (pCD == pClients[k]) 
		{
		    Saved = True;
		    break;
		}
	    }

	    if (!Saved)
	    {
		pClients[numSaved++] = pCD;
	    }
	}
    }

    for (i = 0; i < numSaved; i++)
    {
	pCD = pClients[i];
	fprintf (stderr, "\nClient %s is in: \n", pCD->clientName);
	for (j = 0; j < pCD->numInhabited; j++)
	{
	    pWS = GetWorkspaceData (pCD->pSD, pCD->pWsList[j].wsID);
	    fprintf (stderr, "\t%s\n", pWS->name);
	}

    }
} /* END OF FUNCTION PrintWorkspaceList */
#endif /* DEBUG */

