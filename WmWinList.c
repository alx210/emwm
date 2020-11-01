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
static char rcsid[] = "$TOG: WmWinList.c /main/8 1997/06/10 15:50:50 samborn $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990, 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#define MWM_NEED_NOENTER16
#include "WmBitmap.h"


/*
 * include extern functions
 */
#include "WmWinList.h"
#include "WmCEvent.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#include "WmMenu.h"
#include "WmResource.h"
#include "WmWinInfo.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */




/*
 * Global Variables:
 */


/*************************************<->*************************************
 *
 *  AddClientToList (pWS, pCD, onTop)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a client window to the client window list.  If it is
 *  a transient window then it is added to the transient window tree that
 *  contains its transient leader.  The window stacking order is also
 *  maintained for the cases where there is a system modal window active
 *  or the window is a transient window.  If a system modal window is being
 *  added then the system modal "input screen" window is setup.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to client data for the window to be added to the list
 *
 *  pWS	= pointer to workspace data
 *
 *  onTop = if True then the window is displayed on top of the window
 *      stack and is added to the beginning of the window list, otherwise
 *      it is added to the end of the window list.
 *
 * 
 *  Outputs:
 *  -------
 *  pWS = (clientList, lastClient)
 * 
 *************************************<->***********************************/

void AddClientToList (WmWorkspaceData *pWS, ClientData *pCD, Boolean onTop)
{
    Boolean belowSystemModal = False;
    XWindowChanges windowChanges;
    WmScreenData *pSD = pWS->pSD;
#ifdef WSM
    WsClientData *pWsc = GetWsClientData (pWS, pCD);
#endif /* WSM */


    if (pCD->inputMode == MWM_INPUT_SYSTEM_MODAL)
    {
	/*
	 * Set up the system modal input screen window just below the
	 * system modal window.
	 */

	SetupSystemModalState (pCD);

	if (!wmGD.systemModalActive || (wmGD.systemModalClient != pCD))
	{
	    /*
	     * If we failed to setup as system modal, then 
	     * back off to MWM_INPUT_FULL_APPLICATION_MODAL.
	     * This will do *something* if this is a transient 
	     * window.
	     */
	    pCD->inputMode = MWM_INPUT_FULL_APPLICATION_MODAL;
	}
    }
    else if (wmGD.systemModalActive &&
	     ((FindTransientTreeLeader (pCD))->inputMode !=
	      MWM_INPUT_SYSTEM_MODAL))
    {
	/*
	 * If a system modal window is active then place the window below
	 * the system modal input screen window if the window is not a
	 * descendant of the system modal window.
	 */

	windowChanges.sibling = pSD->inputScreenWindow;
	windowChanges.stack_mode = Below;
	XConfigureWindow (DISPLAY, pCD->clientFrameWin,
	    CWSibling | CWStackMode, &windowChanges);
	belowSystemModal = True;
    }

    if (pCD->transientLeader)
    {
	AddTransient (pWS, pCD);
    }
    else
    {
	pCD->clientEntry.type = NORMAL_STATE;
	pCD->clientEntry.pCD = pCD;

	if (belowSystemModal && wmGD.systemModalClient)
	{
	    AddEntryToList (pWS, &pCD->clientEntry, False /*below*/,
			    pSD->clientList);
	}
	else if (onTop)
	{
	    AddEntryToList (pWS, &pCD->clientEntry, True /*on top*/, NULL);
	}
	else
	{
	    AddEntryToList (pWS, &pCD->clientEntry, False /*on bottom*/, NULL);
	}


#ifdef WSM
	if (!pWsc->pIconBox && pWsc->iconFrameWin)
#else /* WSM */
	if (!pCD->pIconBox && pCD->iconFrameWin)
#endif /* WSM */
	{
	    /*
	     * Put the icon on the bottom of the stack.
	     */

	    if (pSD->lastClient->type == MINIMIZED_STATE)
	    {
#ifdef WSM
		WsClientData *pWsib;

		pWsib = &pSD->lastClient->pCD->pWsList[0];
		windowChanges.sibling = pWsib->iconFrameWin;
#else /* WSM */
		windowChanges.sibling = pSD->lastClient->pCD->iconFrameWin;
#endif /* WSM */
	    }
	    else
	    {
		windowChanges.sibling = pSD->lastClient->pCD->clientFrameWin;
	    }
	    windowChanges.stack_mode = Below;
#ifdef WSM
	    XConfigureWindow (DISPLAY, pWsc->iconFrameWin,
	        CWSibling | CWStackMode, &windowChanges);
#else /* WSM */
	    XConfigureWindow (DISPLAY, pCD->iconFrameWin,
	        CWSibling | CWStackMode, &windowChanges);
#endif /* WSM */

	    pCD->iconEntry.type = MINIMIZED_STATE;
	    pCD->iconEntry.pCD = pCD;
	    pCD->iconEntry.nextSibling = NULL;
	    pCD->iconEntry.prevSibling = pSD->lastClient;
	    pSD->lastClient->nextSibling = &pCD->iconEntry;
	    pSD->lastClient = &pCD->iconEntry;
	}
    }

} /* END OF FUNCTION AddClientToList */



/*************************************<->*************************************
 *
 *  AddEntryToList (pWS, pEntry, onTop, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This function adds a client list entry to the client window list.
 *  This is usually done as part of the process of changing the ordering
 *  of the window list.
 *
 *
 *  Inputs:
 *  ------
 *  pWS  = pointer to workspace data
 *  pEntry = pointer to a client list entry to be added to the client list
 *
 *  onTop = if True then the client list entry is added on top of the 
 *	specified client list stack entry (if the stack entry is not
 *	specified then the entry is added to the front of the list);
 *	otherwise the entry is added after the specified stacking entry
 *	(or to the end of the list if the stacking entry is not specified).
 *
 *  pStackEntry = pointer to a client list entry to be used as a reference
 *	in adding an entry to the client list.
 * 
 *  Outputs:
 *  -------
 *  pWS = (clientList, lastClient)
 * 
 *************************************<->***********************************/

void AddEntryToList (WmWorkspaceData *pWS, ClientListEntry *pEntry, Boolean onTop, ClientListEntry *pStackEntry)
{
    WmScreenData *pSD = pWS->pSD;

    if (onTop)
    {
	if (pStackEntry)
	{
	    if (pEntry != pStackEntry)
	    {
		pEntry->nextSibling = pStackEntry;
		pEntry->prevSibling = pStackEntry->prevSibling;
		pStackEntry->prevSibling = pEntry;
		if (pEntry->prevSibling)
		{
		    pEntry->prevSibling->nextSibling = pEntry;
		}
		else
		{
		    pSD->clientList = pEntry;
		}
	    }
	}
	else
	{
	    if (pSD->clientList != pEntry)
	    {
	        pEntry->nextSibling = pSD->clientList;
	        pEntry->prevSibling = NULL;
		if (pSD->clientList)
		{
		    pSD->clientList->prevSibling = pEntry;
		}
		else
		{
		    pSD->lastClient = pEntry;
		}
	        pSD->clientList = pEntry;
	    }
	}
    }
    else
    {
	if (pStackEntry)
	{
	    if (pEntry != pStackEntry)
	    {
		pEntry->nextSibling = pStackEntry->nextSibling;
		pEntry->prevSibling = pStackEntry;
		pStackEntry->nextSibling = pEntry;
		if (pEntry->nextSibling)
		{
		    pEntry->nextSibling->prevSibling = pEntry;
		}
		else
		{
		    pSD->lastClient = pEntry;
		}
	    }
	}
	else
	{
	    if (pSD->lastClient != pEntry)
	    {
	        pEntry->nextSibling = NULL;
	        pEntry->prevSibling = pSD->lastClient;
		if (pSD->clientList)
		{
		    pSD->lastClient->nextSibling = pEntry;
		}
		else
		{
		    pSD->clientList = pEntry;
		}
	        pSD->lastClient = pEntry;
	    }
	}
    }

} /* END OF FUNCTION AddEntryToList */



/*************************************<->*************************************
 *
 *  MoveEntryInList (pWS, pEntry, onTop, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This function moves a client list entry in the client window list.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 *  pEntry = pointer to a client list entry to be moved in the client list
 *
 *  onTop = if True then the client list entry is moved on top of the 
 *	specified client list stack entry (if the stack entry is not
 *	specified then the entry is moved to the front of the list);
 *	otherwise the entry is moved after the specified stacking entry
 *	(or to the end of the list if the stacking entry is not specified).
 *
 *  pStackEntry = pointer to a client list entry to be used as a reference
 *	in moving an entry in the client list.
 * 
 *  Outputs:
 *  -------
 *  pWS = (clientList, lastClient)
 * 
 *************************************<->***********************************/

void MoveEntryInList (WmWorkspaceData *pWS, ClientListEntry *pEntry, Boolean onTop, ClientListEntry *pStackEntry)
{
    DeleteEntryFromList (pWS, pEntry);
    AddEntryToList (pWS, pEntry, onTop, pStackEntry);

} /* END OF FUNCTION MoveEntryInList */



/*************************************<->*************************************
 *
 *  DeleteEntryFromList (pWS, pListEntry)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes a client list entry from the client window list.
 *  This is usually done as part of the process of changing the ordering
 *  of the window list.
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  listEntry = pointer to a client list entry
 * 
 *  Outputs:
 *  -------
 *  pWS = (clientList, lastClient)
 * 
 *************************************<->***********************************/

void DeleteEntryFromList (WmWorkspaceData *pWS, ClientListEntry *pListEntry)
{
    
    if (pListEntry->prevSibling)
    {
	pListEntry->prevSibling->nextSibling = pListEntry->nextSibling;
    }
    else
    {
	pWS->pSD->clientList = pListEntry->nextSibling;
    }

    if (pListEntry->nextSibling)
    {
	pListEntry->nextSibling->prevSibling = pListEntry->prevSibling;
    }
    else
    {
	pWS->pSD->lastClient = pListEntry->prevSibling;
    }

} /* END OF FUNCTION DeleteEntryFromList */



/*************************************<->*************************************
 *
 *  DeleteClientFromList (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes a client from the client window list.  If this is
 *  a transient window then it is deleted from its transient window tree.
 *  If this is a system modal window then clean up the system modal state.
 *
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to client data for the window to be added to the list
 * 
 *  Outputs:
 *  -------
 *  pWS = (clientList, lastClient)
 * 
 *************************************<->***********************************/

void DeleteClientFromList (WmWorkspaceData *pWS, ClientData *pCD)
{
#ifdef WSM
    WsClientData *pWsc = GetWsClientData (pWS, pCD);
#endif /* WSM */
    WmScreenData *pSD = pWS->pSD;

    if (pCD->transientLeader)
    {
	DeleteTransient (pCD);
    }
    else
    {
	/*
	 * If this is a system modal window then clean up the system modal
	 * state.
	 */

	if (pCD->inputMode == MWM_INPUT_SYSTEM_MODAL)
	{
	    UndoSystemModalState ();
	}

	/*
	 * Remove the client and icon entries from the window list.
	 */

#ifdef WSM
	if (!pWsc->pIconBox && pWsc->iconFrameWin)
#else /* WSM */
	if (!pCD->pIconBox && pCD->iconFrameWin)
#endif /* WSM */
	{
	    if (pCD->iconEntry.prevSibling)
	    {
		pCD->iconEntry.prevSibling->nextSibling =
						pCD->iconEntry.nextSibling;
	    }
	    else
	    {
		pSD->clientList = pCD->iconEntry.nextSibling;
	    }
	    if (pCD->iconEntry.nextSibling)
	    {
		pCD->iconEntry.nextSibling->prevSibling =
						pCD->iconEntry.prevSibling;
	    }
	    else
	    {
		pSD->lastClient = pCD->iconEntry.prevSibling;
	    }
	}

	if (pCD->clientEntry.prevSibling)
	{
	    pCD->clientEntry.prevSibling->nextSibling =
						pCD->clientEntry.nextSibling;
	}
	else
	{
	    pSD->clientList = pCD->clientEntry.nextSibling;
	}

	if (pCD->clientEntry.nextSibling)
	{
	    pCD->clientEntry.nextSibling->prevSibling =
						pCD->clientEntry.prevSibling;
	}
	else
	{
	    pSD->lastClient = pCD->clientEntry.prevSibling;
	}
    }

} /* END OF FUNCTION DeleteClientFromList */



/*************************************<->*************************************
 *
 *  AddTransient (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function adds the transient window to the lead window's list of
 *  transients.
 *
 *
 *  Inputs:
 *  ------
 *  pWS	= pointer to workspace data
 *  pCD	= pointer to client data of a transient window
 *
 * 
 *  Outputs:
 *  -------
 *  pCD->transientLeader = (transientChildren, modalCount)
 * 
 *************************************<->***********************************/

void AddTransient (WmWorkspaceData *pWS, ClientData *pCD)
{
    ClientData *pcdLeader = pCD->transientLeader;
    ClientData *pcdTop = FindTransientTreeLeader (pCD);
    Boolean restackTransients;
    WmScreenData *pSD = pWS->pSD;


    pCD->transientSiblings = pcdLeader->transientChildren;
    pcdLeader->transientChildren = pCD;


    /*
     * Insure that the new transient window is on top of its siblings
     * and that the transient window tree is on top of the window
     * stack (this is the standard behavior for newly mapped and
     * managed windows).  If there is a system modal window that the
     * transient window is not associated with then don't raise the
     * transient tree.
     */

    restackTransients = PutTransientOnTop (pCD);


    /*
     * Handle application modal transient windows
     */

    if (pCD->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL)
    {
	/*
	 * If this is a primary application modal window then increment 
	 * the modal count for transient leaders that are directly up 
	 * the transient tree.
	 *
	 * (This is the old MWM_INPUT_APPLICATION_MODAL behavior.)
	 */
        while (pcdLeader)
        {
	    MarkModalTransient (pcdLeader, pCD);
	    pcdLeader = pcdLeader->transientLeader;
        }
    }
    else if (pCD->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL)
    {
	/*
	 * If this is a full application modal window then increment 
	 * the modal count for the rest of the transient tree.
	 */

	MarkModalSubtree (pcdTop, pCD);
    }
    else if (pcdTop->fullModalCount)
    {
	/*
	 * There is already a full application modal window in the tree
	 */
	pcdLeader = pCD->transientLeader;
    	if ((pcdLeader->inputMode != MWM_INPUT_FULL_APPLICATION_MODAL) ||
	    (IS_APP_MODALIZED(pcdLeader)))
	{
	    /*
	     * The immediate parent of this transient is not the
	     * current full application modal window.  Set the full
	     * modal count to the parent's so that they both become
	     * unmodalized at the same time. This allows a full
	     * app modal window to have active, non-modal transients.
	     */
	    pCD->fullModalCount = pcdLeader->fullModalCount;
	}
    }


    /*
     * Do the actual restacking in the X window stack if necessary.
     */

    if ((pSD->clientList != &pcdTop->clientEntry) && !wmGD.systemModalActive)
    {
	F_Raise (NULL, pCD, NULL);
    }
    else if (restackTransients)
    {
	RestackTransientsAtWindow (pCD);
    }
    else if (pCD != FindTransientOnTop (pcdTop))
    {
	StackTransientWindow (pCD);
    }


} /* END OF FUNCTION AddTransient */



/*************************************<->*************************************
 *
 *  MarkModalSubtree (pcdTree, pcdAvoid)
 *
 *
 *  Description:
 *  -----------
 *  This function marks the transient tree with pcdTree as its leader.
 *  If pcdAvoid is in the tree, it is not marked.
 *
 *  Inputs:
 *  ------
 *  pcdTree	= pointer to client data of the tree to mark
 *  pcdAvoid	= pointer to client data to not mark if in tree
 *
 * 
 *************************************<->***********************************/

void MarkModalSubtree (ClientData *pcdTree, ClientData *pcdAvoid)
{
    /* Mark children, if any */

    if (pcdTree->transientChildren)
	MarkModalSubtree (pcdTree->transientChildren, pcdAvoid);

    /* Mark this node */

    if (pcdTree != pcdAvoid) 
    {
	MarkModalTransient (pcdTree, pcdAvoid);
    }

    /* Mark siblings */

    if (pcdTree->transientSiblings)
	MarkModalSubtree (pcdTree->transientSiblings, pcdAvoid);

}


/*************************************<->*************************************
 *
 *  MarkModalTransient (pcdLeader, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function marks a transient window for application modal processing.
 *  Grabs are done to eat up pointer button events.
 *
 *  Inputs:
 *  ------
 *  pcdLeader = pointer to client data to mark
 *  pCD	= pointer to client data of new transient
 *
 * 
 *************************************<->***********************************/

void MarkModalTransient (ClientData *pcdLeader, ClientData *pCD)
{
    if (!IS_APP_MODALIZED(pcdLeader))
    {
	/*
	 * Eat pointer button events while application modal.
	 */
	XGrabButton (DISPLAY, AnyButton, AnyModifier,
	    pcdLeader->clientBaseWin, True,
	    ButtonPressMask | ButtonMotionMask, GrabModeAsync,
	    GrabModeAsync, None, wmGD.workspaceCursor);
    }

    /* bump application modal count */
    if (pCD->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL)
	pcdLeader->fullModalCount++;
    else 
	pcdLeader->primaryModalCount++;
}


/*************************************<->*************************************
 *
 *  DeleteTransient (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function deletes the transient window from the lead window's list
 *  of transients
 *
 *  Much of the complication of this code arises from trying to handle
 *  mixtures of both full- and primary-application modal transients.
 *  It also tries to handle the case where a sequence of application
 *  modal transients appear in different places in the transient tree
 *  (i.e. not as descendents of a previously existing full app modal 
 *  transient).
 *
 *  Inputs:
 *  ------
 *  pCD	= pointer to client data of transient.
 *
 *************************************<->***********************************/

void DeleteTransient (ClientData *pCD)
{
    ClientData *pcdLeader;
    ClientData *pcdPrev; 
    int modalCount;


    /*
     * Handle primary application modality.
     * Reset the modal window counts for the leader windows up through the
     * transient window tree.
     */

    modalCount = pCD->primaryModalCount;
    if (pCD->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL)
    {
	modalCount += 1;
    }
    if (modalCount)
    {
	pcdLeader = pCD->transientLeader;
	while (pcdLeader)
	{
	    if (modalCount)
		UnMarkModalTransient (pcdLeader, modalCount, pCD);

	    pcdLeader = pcdLeader->transientLeader;
	}
    }

    /*
     * Handle full application modality.
     * Undo application modal windows in a depth first manner.
     */

    pcdLeader = FindTransientTreeLeader (pCD);

    if (pCD->transientChildren)
    {
	DeleteFullAppModalChildren (pcdLeader, pCD->transientChildren);
    }
    if (pCD->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL)

    {
	/*
	 * If this is a full application modal window then decrement 
	 * the modal count for the rest of the transient tree.
	 */

	FixupFullAppModalCounts (pcdLeader, pCD);
    }


    /*
     * Delete this transient from its parent's list of transient windows.
     */

    pcdPrev = pCD->transientLeader->transientChildren;
    if(pcdPrev)
    {
	if (pcdPrev == pCD)
	{
	    pCD->transientLeader->transientChildren = pCD->transientSiblings;
	}
	else
	{
	    while (pcdPrev && (pcdPrev->transientSiblings != pCD))
	    {
		pcdPrev = pcdPrev->transientSiblings;
	    }
	    if (pcdPrev)
	      pcdPrev->transientSiblings = pCD->transientSiblings;
	}
    }

} /* END OF FUNCTION DeleteTransient */


/*************************************<->*************************************
 *
 *  DeleteFullAppModalChildren (pcdLeader, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function handles the clean-up of nested full application modal
 *  windows. The deletion has to be handled carefully to keep a correct
 *  fullModalCount on the transients that remain in the tree.
 *
 *  The algorithm is to traverse the transient children depth first and
 *  fix up the tree's fullModalCount for each full application modal
 *  window that's found. 
 *
 *  Inputs:
 *  ------
 *  pcdLeader	= pointer to client data of transient tree root.
 *  pCD		= pointer to client data of transient subtree to delete.
 *
 *************************************<->***********************************/

void DeleteFullAppModalChildren (ClientData *pcdLeader, ClientData *pCD)
{

    /* recursively do children first */
    if (pCD->transientChildren)
	DeleteFullAppModalChildren (pcdLeader, pCD->transientChildren);

    /* do fullAppModal fixup for this guy */
    FixupFullAppModalCounts (pcdLeader, pCD);

    /* do siblings of passed transient */
    if (pCD->transientSiblings)
	DeleteFullAppModalChildren (pcdLeader, pCD->transientSiblings);

    
} /* END OF FUNCTION DeleteFullAppModalChildren */


/*************************************<->*************************************
 *
 *  FixupFullAppModalCounts (pcdLeader, pcdDelete)
 *
 *
 *  Description:
 *  -----------
 *  This function traverses the entire transient tree (pointed to by 
 *  pcdLeader) and fixes up the fullModalCounts to reflect the removal 
 *  of pcdDelete. 
 *
 *  The fix-up consists of decrementing the count
 *  of the remaining full app modal windows in the tree iff the remaining
 *  window's fullModalCount is greater than the fullModalCount of the 
 *  transient being deleted.
 *
 *  Inputs:
 *  ------
 *  pcdLeader	= pointer to client data for head of transient tree.
 *  pcdDelet	= pointer to client data of transient being deleted.
 *
 *************************************<->***********************************/

void
FixupFullAppModalCounts (ClientData *pcdLeader, ClientData *pcdDelete)

{

    /* fixup children */
    if (pcdLeader->transientChildren) 
    {
	FixupFullAppModalCounts (pcdLeader->transientChildren, pcdDelete);
    }

    /* 
     * fixup leader: decrement the count if it is greater than
     * the transient being deleted.
     *
     */
    if (pcdLeader->fullModalCount > pcdDelete->fullModalCount)
    {
	UnMarkModalTransient (pcdLeader, 1, pcdDelete);
    }

    /* fixup siblings */
    if (pcdLeader->transientSiblings) 
    {
	FixupFullAppModalCounts (pcdLeader->transientSiblings, pcdDelete);
    }
    
} /* END OF FUNCTION FixupFullAppModalCounts */



/*************************************<->*************************************
 *
 *  UnMarkModalTransient (pcdModee, modalCount, pcdModal)
 *
 *
 *  Description:
 *  -----------
 *  This function unmarks a transient window for application modal processing.
 *  Original grabs are restored.
 *
 *  Inputs:
 *  ------
 *  pcdModee   = pointer to client data for transient to unmark
 *  pcdModal   = pointer to client data for modal transient
 *  modalCount = amount to decrement the client's modal count by
 *
 * 
 *************************************<->***********************************/

void UnMarkModalTransient (ClientData *pcdModee, int modalCount, ClientData *pcdModal)
{
    /* decrement application modal count */
    if (pcdModal->inputMode == MWM_INPUT_FULL_APPLICATION_MODAL)
	pcdModee->fullModalCount -= modalCount;
    else if (pcdModal->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL)
	pcdModee->primaryModalCount -= modalCount;

    /*
     * Restore original button bindings/grabs if not modal anymore
     */
    if (!IS_APP_MODALIZED(pcdModee))
    {
	XUngrabButton (DISPLAY, AnyButton, AnyModifier, 
	    pcdModee->clientBaseWin);

	SetupCButtonBindings (pcdModee->clientBaseWin, BUTTON_SPECS(pcdModee));

	if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	    (wmGD.keyboardFocus != pcdModee))
	{
	    DoExplicitSelectGrab (pcdModee->clientBaseWin);
	}
    }
}


/*************************************<->*************************************
 *
 *  PutTransientOnTop (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function changes the transient window list to insure that the
 *  specified transient window is on top of its siblings and its parent
 *  is on top of its siblings, etc.  The sibling list is ordered such
 *  that the first window in the list is on top of second window in the
 *  list, etc.
 *
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a transient window
 *
 * 
 *  Outputs:
 *  -------
 *  pcdLeader = (transientSiblings)
 *
 *  RETURN = True if the transient tree needs to be restacked
 *
 *************************************<->***********************************/

Boolean PutTransientOnTop (ClientData *pcd)
{
    ClientData *pcdLeader;
    ClientData *pcdPrev;
    Boolean restack = False;


    pcdLeader = pcd->transientLeader;
    if (pcdLeader != NULL)
    {
	pcdPrev = pcdLeader->transientChildren;
	if (pcdPrev != pcd)
	{
	    while (pcdPrev->transientSiblings != pcd)
	    {
		pcdPrev = pcdPrev->transientSiblings;
	    }
	    pcdPrev->transientSiblings = pcd->transientSiblings;
	    pcd->transientSiblings = pcdLeader->transientChildren;
	    pcdLeader->transientChildren = pcd;
	    restack = True;
	}

	if (PutTransientOnTop (pcdLeader))
	{
	    restack = True;
	}
#ifdef WSM
	if (BumpPrimaryToBottom (pcdLeader))
	{
	    restack = True;
	}
#endif /* WSM */
    }

    return (restack);

} /* END OF FUNCTION PutTransientOnTop */



/*************************************<->*************************************
 *
 *  PutTransientBelowSiblings (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function changes the transient window list to insure that the
 *  specified transient window is below its sibling windows.
 *
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a transient window
 *
 * 
 *  Outputs:
 *  -------
 *  pcdLeader = (transientSiblings)
 *
 *  RETURN = True if restacking has been done in the transient window tree.
 *
 *************************************<->***********************************/

Boolean PutTransientBelowSiblings (ClientData *pcd)
{
    ClientData *pcdLeader;
    ClientData *pcdNext;
    Boolean restack = False;


    pcdLeader = pcd->transientLeader;
    if (pcdLeader)
    {
	if (pcd->transientSiblings || (pcdLeader->transientChildren != pcd))
	{
	    restack = True;
	    if (pcdLeader->transientChildren == pcd)
	    {
	        pcdLeader->transientChildren = pcd->transientSiblings;
	    }

	    pcdNext = pcdLeader->transientChildren;
	    while (pcdNext->transientSiblings)
	    {
		if (pcdNext->transientSiblings == pcd)
		{
		    pcdNext->transientSiblings = pcd->transientSiblings;
		}
		else
		{
		    pcdNext = pcdNext->transientSiblings;
		}
	    }
	    pcdNext->transientSiblings = pcd;
	}
	pcd->transientSiblings = NULL;
    }

    return (restack);

} /* END OF FUNCTION PutTransientBelowSiblings */



/*************************************<->*************************************
 *
 *  RestackTransients (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function restacks windows in a transient window tree.  Secondary
 *  (transient) windows are stacked on top of their associated primary
 *  windows and the first secondary window in the transientSiblings list
 *  is stacked on top of the second window in the list, etc.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree
 *
 *************************************<->***********************************/

void RestackTransients (ClientData *pcd)
{
    ClientData *pcdLeader;
    int count;
    static int size = 0;
    static Window *windows = NULL;
#ifndef WSM
    Window *nextWindow;
#endif /* WSM */
    XWindowChanges windowChanges;
    int i;
    int leaderIndex;

    /*
     * Build a restacking list and do the restacking.
     */

    pcdLeader = FindTransientTreeLeader (pcd);
    count = CountTransientChildren (pcdLeader);

    /* No work to do if no transient children; count includes leader. */
    if (count < 2)
      return;

    if (count > size)
    {
	/*
	 * Expand the (static) windows buffer that is used in restacking.
	 */

	if (!(windows =
		(Window *)WmMalloc ((char*)windows, (count + 5) * sizeof (Window))))
	{
	    /* cannot get memory space */
	    size = 0;
	    return;
	}
	size = count + 5;
    }

#ifdef WSM
    MakeTransientFamilyStackingList (windows, pcdLeader);
#else /* WSM */
    nextWindow = MakeTransientWindowList (windows, pcdLeader);
    *nextWindow = pcdLeader->clientFrameWin;
#endif /* WSM */

    /*
     *  Changes for CDExc19397.
     *  XRestackWindows may move pcdLeader; that messes up the
     *  global window stack.  Call XConfigureWindow() instead,
     *  and don't change location of pcdLeader.
     */
    for (leaderIndex = 0; leaderIndex < count; leaderIndex++)
    {
      if (windows[leaderIndex] == pcdLeader->clientFrameWin)
	break;
    }
    if (leaderIndex >= count) /* ? Couldn't find leader; should NOT happen. */
      leaderIndex = count - 1;

    windowChanges.stack_mode = Above;
    for (i = leaderIndex; i > 0; i--)
    {
      windowChanges.sibling = windows[i];
      XConfigureWindow (DISPLAY, windows[i - 1],
			CWSibling | CWStackMode, &windowChanges);
    }

    windowChanges.stack_mode = Below;
    for (i = leaderIndex; i < count - 1; i++)
    {
      windowChanges.sibling = windows[i];
      XConfigureWindow (DISPLAY, windows[i + 1],
			CWSibling | CWStackMode, &windowChanges);
    }

} /* END OF FUNCTION RestackTransients */



/*************************************<->*************************************
 *
 *  RestackTransientsAtWindow (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function restacks windows in a transient window tree.  The
 *  "anchor point" in the stack for the transient window tree is the
 *  specified window.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree
 *
 *************************************<->***********************************/

void RestackTransientsAtWindow (ClientData *pcd)
{
    ClientData *pcdLeader;
    XWindowChanges windowChanges;

    pcdLeader = FindTransientTreeLeader (pcd);
    if (pcdLeader && (pcdLeader != pcd))
    {
	windowChanges.sibling = pcd->clientFrameWin;
	windowChanges.stack_mode = Below;
	XConfigureWindow (DISPLAY, pcdLeader->clientFrameWin,
	    CWSibling | CWStackMode, &windowChanges);
    }

    RestackTransients (pcd);

} /* END OF FUNCTION RestackTransientsAtWindow */



/*************************************<->*************************************
 *
 *  FindTransientTreeLeader (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the leader of the transient tree that
 *  contains the specified client.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree.
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data for the transient tree leader.
 *
 *************************************<->***********************************/

ClientData * FindTransientTreeLeader (ClientData *pcd)

{

    /*
     * Find the head of the transient window tree.
     */

    while (pcd->transientLeader)
    {
	pcd = pcd->transientLeader;
    }

    return (pcd);

} /* END OF FUNCTION FindTransientTreeLeader */



/*************************************<->*************************************
 *
 *  CountTransientChildren (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function returns a count of the number of children in the 
 *  transient window tree headed by the specified client window.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree
 *
 *  Outputs:
 *  -------
 *  RETURN = count of transient windows in the transient window tree
 *
 *************************************<->***********************************/

int
CountTransientChildren (ClientData *pcd)

{
    ClientData *pcdNext;
    int count = 1;


    pcdNext = pcd->transientChildren;
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    count += CountTransientChildren (pcdNext);
	}
	else
	{
	    count++;
	}
	pcdNext = pcdNext->transientSiblings;
    }

    return (count);

} /* END OF FUNCTION CountTransientChildren */



/*************************************<->*************************************
 *
 *  MakeTransientWindowList (windows, pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a transient window list of windows in the 
 *  transient window tree headed by the specified client.  This list is
 *  to be passed to XRestackWindows.
 *  
 *
 *  Inputs:
 *  ------
 *  windows = pointer to the windows list to be filled out
 *
 *  pcd	= pointer to client data of a window in a transient tree
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the next entry in the windows list
 *
 *************************************<->***********************************/

Window * MakeTransientWindowList (Window *windows, ClientData *pcd)

{
    ClientData *pcdNext;


    pcdNext = pcd->transientChildren;
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    windows = MakeTransientWindowList (windows, pcdNext);
	}
	*windows = pcdNext->clientFrameWin;
	windows++;
	pcdNext = pcdNext->transientSiblings;
    }

    return (windows);


} /* END OF FUNCTION MakeTransientWindowList */



/*************************************<->*************************************
 *
 *  FindTransientFocus (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies a window in the transient tree that is headed
 *  by the specified client that can accept the keyboard input.  The
 *  effect of application modal windows is taken into account.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree.
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data for a window that can accept the
 *      keyboard input focus.
 *
 *************************************<->***********************************/

ClientData * FindTransientFocus (ClientData *pcd)

{

    ClientData *pcdFocus;

    /*
     * Find a window that does not have an application modal subordinate.
     * First, search descendents
     */

    pcdFocus = pcd;
    while (pcdFocus->transientChildren && IS_APP_MODALIZED(pcdFocus))
    {
	pcdFocus = pcdFocus->transientChildren;
    }

    /*
     * If (search of descendents FAILS) then search siblings.
     */
    
    if (IS_APP_MODALIZED(pcdFocus))
    {
	ClientData *pcdSibling;

	pcdFocus = pcd;
	while (pcdFocus && IS_APP_MODALIZED(pcdFocus))
	{
	    pcdSibling = pcdFocus;
	    while (pcdSibling->transientSiblings && IS_APP_MODALIZED(pcdFocus))
	    {
		pcdSibling = pcdSibling->transientSiblings;
	    }
	    if (IS_APP_MODALIZED(pcdSibling))
	    {
		pcdFocus = pcdFocus->transientChildren;
	    }
	    else
	    {
		pcdFocus = pcdSibling;
		break;
	    }
	}
    }

    return (pcdFocus ? pcdFocus : wmGD.keyboardFocus);

} /* END OF FUNCTION FindTransientFocus */



/*************************************<->*************************************
 *
 *  FindTransientOnTop (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the top-most transient window in the
 *  transient window tree that contains the specified client.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree.
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to the client data for the top-most transient window.
 *
 *************************************<->***********************************/

ClientData * FindTransientOnTop (ClientData *pcd)

{

    /*
     * Find the head of the transient window tree.
     */

    pcd = FindTransientTreeLeader (pcd);
#ifdef WSM
    if (!(pcd->secondariesOnTop) &&
	(LeaderOnTop (pcd)))
    {
	ClientData *pcdSub;

	if (LeaderOnTop (pcd))
	{
	    /* The primary window is on top! */
	    return (pcd);
	}
	else
	{
	    pcdSub = FindSubLeaderToTop (pcd);
	    if (pcdSub)
		return (pcdSub);
	}
    }
#endif /* WSM */


    /*
     * Find the top-most transient window (the window in the transient tree
     * that is highest in the window stack).
     */

    while (pcd->transientChildren)
    {
	pcd = pcd->transientChildren;
    }

    return (pcd);

} /* END OF FUNCTION FindTransientOnTop */



/*************************************<->*************************************
 *
 *  StackWindow (pWS, pEntry, onTop, pStackEntry)
 *
 *
 *  Description:
 *  -----------
 *  This function stacks a window of a particular type (normal or icon)
 *  to the top or botton of the window stack on the screen.
 *  
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 *  pEntry = pointer to the client list entry for the window to be restacked.
 *
 *  onTop = if True then the window is to be restacked on top of the
 *	specified stack window (if the stack window is not specified then
 *	the entry is added to the top of the window stack)
 *	otherwise the window is stacked below the specified stack window
 *	(or at the bottom of the window stack if the stack window is not
 *	specified).
 *
 *  pStackEntry = pointer to a client list entry for a window in the window
 *	stack that is to be used as a reference in restacking.
 *
 *************************************<->***********************************/

void StackWindow (WmWorkspaceData *pWS, ClientListEntry *pEntry, Boolean onTop, ClientListEntry *pStackEntry)
{
    Window stackWindow;
    Boolean stackTransientTreeWindows = False;
    Window activeIconWindow;
    Window window;
    XWindowChanges changes;
    WmScreenData *pSD = pWS->pSD;


    if (pStackEntry)
    {
	if (pStackEntry->type == MINIMIZED_STATE)
	{
	    stackWindow = ICON_FRAME_WIN(pStackEntry->pCD);
	}
	else
	{
	    stackWindow = pStackEntry->pCD->clientFrameWin;
	}
    }
    else
    {
	stackWindow = (Window)0;
    }

    if (pEntry->type == MINIMIZED_STATE)
    {
	window = ICON_FRAME_WIN(pEntry->pCD);
    }
    else
    {
	/*
	 * Restack the transient tree if appropriate.
	 */

	if (pEntry->pCD->transientLeader || pEntry->pCD->transientChildren)
	{
	    stackTransientTreeWindows = True;

	    window = (FindTransientOnTop (pEntry->pCD))->clientFrameWin;
	}
	else
	{
	    window = pEntry->pCD->clientFrameWin;
	}
    }


    /*
     * The active icon text label must be restacked along with the associated
     * icon.
     */

    if ((pEntry->type == MINIMIZED_STATE) &&
	(pEntry->pCD == wmGD.keyboardFocus) &&
	(ICON_DECORATION(pEntry->pCD) & ICON_ACTIVE_LABEL_PART) &&
	(ACTIVE_ICON_TEXT_WIN))
    {
	activeIconWindow = ACTIVE_ICON_TEXT_WIN;
    }
    else
    {
	activeIconWindow = (Window)0;
    }

    if (onTop)
    {
	if ((stackWindow == 0) && (pSD->clientList))
	{
	    if (pSD->clientList->type == MINIMIZED_STATE)
	    {
		stackWindow = ICON_FRAME_WIN(pSD->clientList->pCD);
	    }
	    else
	    {
		if (pSD->clientList->pCD->transientChildren)
		{
		    stackWindow =
		     (FindTransientOnTop(pSD->clientList->pCD))->clientFrameWin;
		}
		else
		{
		    stackWindow = pSD->clientList->pCD->clientFrameWin;
		}
	    }
	}

	if (activeIconWindow)
	{
	    changes.sibling = stackWindow;
	    changes.stack_mode = Above;
	    XConfigureWindow (DISPLAY, activeIconWindow,
	        (CWSibling | CWStackMode), &changes);
	    changes.sibling = activeIconWindow;
	    changes.stack_mode = Below;
	    XConfigureWindow (DISPLAY, window, (CWSibling | CWStackMode),
	        &changes);
	}
	else
	{
	    changes.sibling = stackWindow;
	    changes.stack_mode = Above;
	    XConfigureWindow (DISPLAY, window, (CWSibling | CWStackMode),
	        &changes);
	    if (stackTransientTreeWindows)
	    {
		/* make sure that the leader is in the correct spot */
                changes.sibling = window;
		changes.stack_mode = Below;
		XConfigureWindow (DISPLAY, pEntry->pCD->clientFrameWin,
				  (CWSibling | CWStackMode), &changes);
	        RestackTransients (pEntry->pCD);
	    }
	}
    }
    else
    {
#ifdef WSM
	/*
	 * Adjust stack entry window if we're stacking below a
	 * transient tree.
	 */
	if (pStackEntry && pStackEntry->pCD->transientChildren)
	{
	    stackWindow = LowestWindowInTransientFamily (pStackEntry->pCD);
	}

#endif /* WSM */
	if (stackWindow == 0)
	{
	    if (pSD->lastClient->type == MINIMIZED_STATE)
	    {
		stackWindow = ICON_FRAME_WIN(pSD->lastClient->pCD);
	    }
	    else
	    {
#ifdef WSM
		if (pSD->lastClient->pCD->transientChildren)
		{
		    stackWindow = 
			LowestWindowInTransientFamily (pSD->lastClient->pCD);
		}
		else
#endif /* WSM */
		stackWindow = pSD->lastClient->pCD->clientFrameWin;
	    }
	}

	if (activeIconWindow)
	{
	    changes.sibling = stackWindow;
	    changes.stack_mode = Below;
	    XConfigureWindow (DISPLAY, activeIconWindow,
			      (CWSibling | CWStackMode), &changes);
	    changes.sibling = activeIconWindow;
	    changes.stack_mode = Below;
	    XConfigureWindow (DISPLAY, window, (CWSibling | CWStackMode),
			      &changes);
	}
	else
	{
	    changes.sibling = stackWindow;
	    changes.stack_mode = Below;
	    XConfigureWindow (DISPLAY, window, (CWSibling | CWStackMode),
			      &changes);
	    if (stackTransientTreeWindows)
	    {
		/* make sure that the leader is in the correct spot */
		changes.sibling = window;
		changes.stack_mode = Below;
		XConfigureWindow (DISPLAY, pEntry->pCD->clientFrameWin,
				  (CWSibling | CWStackMode), &changes);
	        RestackTransients (pEntry->pCD);
	    }
	}
    }

} /* END OF FUNCTION StackWindow */



/*************************************<->*************************************
 *
 *  StackTransientWindow (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function stacks a transient window within its transient window
 *  tree on the screen.  The transient window tree should indicate the
 *  intended stacking position.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data of a window in a transient tree
 *
 *************************************<->***********************************/

void StackTransientWindow (ClientData *pcd)
{
    XWindowChanges changes;
    ClientData *pcdPrev;


    if (pcd->transientLeader->transientChildren == pcd)
    {
	if (pcd->transientSiblings)
	{
	    changes.sibling = pcd->transientSiblings->clientFrameWin;
	}
	else
	{
	    changes.sibling = pcd->transientLeader->clientFrameWin;
	}
	changes.stack_mode = Above;
    }
    else
    {
	pcdPrev = pcd->transientLeader;
	while (pcdPrev->transientSiblings != pcd)
	{
	    pcdPrev = pcdPrev->transientSiblings;
	}
	changes.sibling = pcdPrev->clientFrameWin;
	changes.stack_mode = Below;
    }

    XConfigureWindow (DISPLAY, pcd->clientFrameWin, (CWSibling | CWStackMode),
	&changes);


} /* END OF FUNCTION StackTransientWindow */



/*************************************<->*************************************
 *
 *  CheckIfClientObscuring (pcdTop, pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a window or a transient window tree
 *  is obscuring (at least partially) a window or a transient window tree
 *  that is below it in the window stack.
 *  
 *
 *  Inputs:
 *  ------
 *  pcdTop = pointer to client data for a window (it may be the leader of
 *	a transient tree; this window is the higher in the window stack
 *	than the window it is be checked against.
 *
 *  pcd = pointer to client data for a window (it may be the leader of
 *	a transient tree.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the top window(s) overlap the lower window(s)
 *
 *************************************<->***********************************/

Boolean CheckIfClientObscuring (ClientData *pcdTop, ClientData *pcd)
{
    Boolean obscuring = False;
    ClientData *pcdNext;


    /*
     * Check only if the top window is visible onscreen.
     */

    if (pcdTop->transientChildren && (pcdTop->clientState != MINIMIZED_STATE))
    {
	pcdNext = pcdTop->transientChildren;
	while (pcdNext && !obscuring)
	{
	    obscuring = CheckIfClientObscuring (pcdNext, pcd);
	    pcdNext = pcdNext->transientSiblings;
	}
    }

    if (!obscuring && pcd->transientChildren &&
	(pcd->clientState != MINIMIZED_STATE))
    {
	pcdNext = pcd->transientChildren;
	while (pcdNext && !obscuring)
	{
	    obscuring = CheckIfClientObscuring (pcdTop, pcdNext);
	    pcdNext = pcdNext->transientSiblings;
	}
    }

    if (!obscuring)
    {
	obscuring = CheckIfObscuring (pcdTop, pcd);
    }

    return (obscuring);

} /* END OF FUNCTION CheckIfClientObscuring */



/*************************************<->*************************************
 *
 *  CheckIfObscuring (pcdA, pcdB)
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a window (not a transient tree)
 *  is obscuring (at least partially) a window (not a transient tree)
 *  that is below it in the window stack.
 *  
 *
 *  Inputs:
 *  ------
 *  pcdA = pointer to client data for a window; this window is higher in
 *	the window stack than the window it is be checked against.
 *
 *  pcdB = pointer to client data for a window.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the top window overlaps the lower window
 *
 *************************************<->***********************************/

Boolean CheckIfObscuring (ClientData *pcdA, ClientData *pcdB)
{
    Boolean obscuring = False;
    int aX1;
    int aX2;
    int aY1;
    int aY2;
    int bX1;
    int bX2;
    int bY1;
    int bY2;

#ifdef WSM
    /*
     * For workspace stuff: if either is unseen, then neither
     * is obscured.
     */
    if ((pcdA->clientState & UNSEEN_STATE) ||
	(pcdB->clientState & UNSEEN_STATE))
    {
	return (False);
    }
#endif /* WSM */

    if (pcdA->clientState == NORMAL_STATE)
    {
	aX1 = pcdA->clientX - pcdA->clientOffset.x;
	aY1 = pcdA->clientY - pcdA->clientOffset.y;
	aX2 = aX1 + pcdA->clientWidth + (2 * pcdA->clientOffset.x) - 1;
	aY2 = aY1 + pcdA->clientHeight + pcdA->clientOffset.y +
	      pcdA->clientOffset.x - 1;
    }
    else if (pcdA->clientState == MINIMIZED_STATE)
    {
	aX1 = ICON_X(pcdA);
	aY1 = ICON_Y(pcdA);
	aX2 = aX1 + ICON_WIDTH(pcdA) - 1; 
	aY2 = aY1 + ICON_HEIGHT(pcdA) - 1;
    }
    else /* (pcdA->clientState == MAXIMIZED_STATE) */
    {
	aX1 = pcdA->maxX - pcdA->clientOffset.x;
	aY1 = pcdA->maxY - pcdA->clientOffset.y;
	aX2 = aX1 + pcdA->maxWidth + (2 * pcdA->clientOffset.x) - 1;
	aY2 = aY1 + pcdA->maxHeight + pcdA->clientOffset.y +
	      pcdA->clientOffset.x - 1;
    }

    if (pcdB->clientState == NORMAL_STATE)
    {
	bX1 = pcdB->clientX - pcdB->clientOffset.x;
	bY1 = pcdB->clientY - pcdB->clientOffset.y;
	bX2 = bX1 + pcdB->clientWidth + (2 * pcdB->clientOffset.x) - 1;
	bY2 = bY1 + pcdB->clientHeight + pcdB->clientOffset.y +
	      pcdB->clientOffset.x - 1;
    }
    else if (pcdB->clientState == MINIMIZED_STATE)
    {
	bX1 = ICON_X(pcdB);
	bY1 = ICON_Y(pcdB);
	bX2 = bX1 + ICON_WIDTH(pcdB) - 1; 
	bY2 = bY1 + ICON_HEIGHT(pcdB) - 1;
    }
    else /* (pcdB->clientState == MAXIMIZED_STATE) */
    {
	bX1 = pcdB->maxX - pcdB->clientOffset.x;
	bY1 = pcdB->maxY - pcdB->clientOffset.y;
	bX2 = bX1 + pcdB->maxWidth + (2 * pcdB->clientOffset.x) - 1;
	bY2 = bY1 + pcdB->maxHeight + pcdB->clientOffset.y +
	      pcdB->clientOffset.x - 1;
    }

    /*
     * Check if there is overlap in both dimensions.
     */

    if (((aX1 >= bX1) && (aX1 <= bX2)) || ((aX2 >= bX1) && (aX2 <= bX2)) ||
	((bX1 >= aX1) && (bX1 <= aX2)) || ((bX2 >= aX1) && (bX2 <= aX2)))
    {
	if (((aY1 >= bY1) && (aY1 <= bY2)) || ((aY2 >= bY1) && (aY2 <= bY2)) ||
	    ((bY1 >= aY1) && (bY1 <= aY2)) || ((bY2 >= aY1) && (bY2 <= aY2)))
	{
	    obscuring = True;
	}
    }

    return (obscuring);


} /* END OF FUNCTION CheckIfObscuring */



/*************************************<->*************************************
 *
 *  CheckIfClientObscuredByAny (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a window or a transient window tree
 *  is obscured (at least partially) by any other window.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd = pointer to client data for a window (it may be the leader of
 *	a transient tree.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the window(s) are overlapped.
 *
 *************************************<->***********************************/

Boolean CheckIfClientObscuredByAny (ClientData *pcd)
{
    Boolean obscured = False;
    ClientListEntry *pListEntry;


    pListEntry = ACTIVE_PSD->clientList;
    while (pListEntry && !obscured)
    {
	if (pListEntry->pCD == pcd)
	{
	    if (((pListEntry->type == MINIMIZED_STATE) &&
		 (pListEntry->pCD->clientState == MINIMIZED_STATE)) ||
		((pListEntry->type != MINIMIZED_STATE) &&
		 (pListEntry->pCD->clientState != MINIMIZED_STATE)))
	    {
		pListEntry = NULL;
	    }
	}
	else if (((pListEntry->type == MINIMIZED_STATE) &&
		   (pListEntry->pCD->clientState == MINIMIZED_STATE)) ||
		 ((pListEntry->type != MINIMIZED_STATE) &&
		  (pListEntry->pCD->clientState != MINIMIZED_STATE)))
	{
	    /*
	     * The window for the entry is visible on screen.  See if it
	     * obscures the indicated window.
	     */

	    obscured = CheckIfClientObscuring (pListEntry->pCD, pcd);
	}

	if (pListEntry)
	{
	    pListEntry = pListEntry->nextSibling;
	}
    }

    return (obscured);

} /* END OF FUNCTION CheckIfClientObscuredByAny */



/*************************************<->*************************************
 *
 *  CheckIfClientObscuringAny (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a window or a transient window tree
 *  is obscuring another window.
 *  
 *
 *  Inputs:
 *  ------
 *  pcd = pointer to client data for a window (it may be the leader of
 *	a transient tree.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = True if the window(s) overlaps anther window.
 *
 *************************************<->***********************************/

Boolean CheckIfClientObscuringAny (ClientData *pcd)
{
    Boolean obscuring = False;
    ClientListEntry *pListEntry;


    pListEntry = (pcd->clientState == MINIMIZED_STATE) ?
					&pcd->iconEntry : &pcd->clientEntry;
    while (pListEntry && !obscuring)
    {
	if ((pListEntry->pCD != pcd) &&
	    (((pListEntry->type == MINIMIZED_STATE) &&
	      (pListEntry->pCD->clientState == MINIMIZED_STATE)) ||
	     ((pListEntry->type != MINIMIZED_STATE) &&
	      (pListEntry->pCD->clientState != MINIMIZED_STATE))))
	{
	    obscuring = CheckIfClientObscuring (pcd, pListEntry->pCD);
	}

	pListEntry = pListEntry->nextSibling;
    }

    return (obscuring);

} /* END OF FUNCTION CheckIfClientObscuringAny */



/*************************************<->*************************************
 *
 *  SetupSystemModalState (pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function prepares for mapping a system modal window.  An input
 *  screen window is mapped below the system modal window to prevent input
 *  to the windows not related to the system modal window.
 *  
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data for the system modal window; if NULL the
 *	system modal window is a special window manager dialog box
 *
 *
 *  Outputs:
 *  -------
 *  wmGD = changes to system modal state data
 *
 *************************************<->***********************************/

void SetupSystemModalState (ClientData *pCD)
{
    XWindowChanges windowChanges;
    unsigned int   width, height;
    unsigned int   x_hot, y_hot;
    unsigned char *bits;
    unsigned char *mask_bits;
    WmScreenData *pSD;
    int scr;

    /*
     * If we've got a menu active, then unpost it first
     * so that grabs from the menu don't interfere with
     * the system modal dialog. We want to avoid lock-ups.
     */
    if (wmGD.menuActive != NULL)
    {
        UnpostMenu (wmGD.menuActive);
	XSync (DISPLAY, False);
    }

    /*
     * Try to grab the pointer and keyboard. If either
     * fails because event processing is frozen by another grab, then 
     * don't do system modal for fear of leaving the system unusable.
     */
    if (XGrabPointer(DISPLAY, 
		     ROOT_FOR_CLIENT(pCD),
		     FALSE,			/* owner_events */
		     (unsigned int) 0,		/* event mask */
		     GrabModeAsync,		/* pointer_mode */
		     GrabModeAsync,		/* keyboard_mode */
		     None,			/* confine_to window */
		     None,			/* cursor */
		     CurrentTime) == GrabFrozen)
    {	
	return;
    }
    else
    {
	XUngrabPointer (DISPLAY, CurrentTime);
    }

    if (XGrabKeyboard(DISPLAY, 
		     ROOT_FOR_CLIENT(pCD),
		     FALSE,			/* owner_events */
		     GrabModeAsync,		/* pointer_mode */
		     GrabModeAsync,		/* keyboard_mode */
		     CurrentTime) == GrabFrozen)
    {	
	return;
    }
    else
    {
	XUngrabKeyboard (DISPLAY, CurrentTime);
    }

#ifdef LARGECURSORS

    if (wmGD.useLargeCursors)
    {
	width = noenter32_width;
	height = noenter32_height;
	x_hot = noenter32_x_hot;
	y_hot = noenter32_y_hot;
	bits = noenter32_bits;
	mask_bits = noenter32m_bits;
    }
    else

#endif /* LARGECURSORS */

    {
	width = noenter16_width;
	height = noenter16_height;
	x_hot = noenter16_x_hot;
	y_hot = noenter16_y_hot;
	bits = noenter16_bits;
	mask_bits = noenter16m_bits;
    }

    for (scr=0; scr<wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);
	
	/*
	 * Make the system modal input screen window if necessary.
	 */
	
	if (pSD->managed && pSD->inputScreenWindow == 0)
	{
	    XSetWindowAttributes windowAttributes;
	    Pixmap               pixmap;
	    Pixmap               maskPixmap;
	    XColor               xcolors[2];
	    
	    windowAttributes.event_mask = ButtonPressMask;
	    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	    {
		windowAttributes.event_mask |= EnterWindowMask;
	    }
	    windowAttributes.override_redirect = True;
	    
	    pixmap = XCreateBitmapFromData (DISPLAY, pSD->rootWindow, 
					    (char *)bits, width, height);
	    
	    maskPixmap = XCreateBitmapFromData (DISPLAY, pSD->rootWindow, 
						(char *)mask_bits, width, height);
	    
	    xcolors[0].pixel = BlackPixel (DISPLAY, pSD->screen);
	    xcolors[1].pixel = WhitePixel (DISPLAY, pSD->screen);
	    XQueryColors (DISPLAY, DefaultColormap (DISPLAY, pSD->screen), 
			  xcolors, 2);
	    windowAttributes.cursor =
		XCreatePixmapCursor (DISPLAY, pixmap, maskPixmap,
				     &(xcolors[0]), &(xcolors[1]), 
				     x_hot, y_hot);
	    XFreePixmap (DISPLAY, pixmap);
	    XFreePixmap (DISPLAY, maskPixmap);
	    
	    pSD->inputScreenWindow =
		XCreateWindow (DISPLAY, pSD->rootWindow, 0, 0,
			       DisplayWidth (DISPLAY, pSD->screen),
			       DisplayHeight (DISPLAY, pSD->screen),
			       0,
			       0,
			       InputOnly,
			       CopyFromParent,
			       CWEventMask | CWOverrideRedirect | CWCursor,
			       &windowAttributes);
	}
	if (pSD->managed && pSD != ACTIVE_PSD)
	{
	    XMapRaised (DISPLAY, pSD->inputScreenWindow);
	}
    }
    
    if (pCD)
    {
	wmGD.systemModalWindow = pCD->clientFrameWin;
    }
    else
    {
        /*
         * ELSE: the system modal window is a special window manager dialog
         * box and wmGD.systemModalWindow is set prior to the call to 
         * SetupSystemModalState.  Set the focus to the special window manager
	 * dialog box.
         */
	
	SetKeyboardFocus (NULL, REFRESH_LAST_FOCUS);
	XSetInputFocus (DISPLAY, wmGD.systemModalWindow, RevertToPointerRoot,
			CurrentTime);
    }
    
    
    /*
     * Map the system modal input screen window below the system modal
     * window.
     */
    
    windowChanges.sibling = wmGD.systemModalWindow;
    windowChanges.stack_mode = Below;
    XConfigureWindow (DISPLAY, ACTIVE_PSD->inputScreenWindow, 
		      CWSibling | CWStackMode, &windowChanges);
    
    XMapWindow (DISPLAY, ACTIVE_PSD->inputScreenWindow);
    
    
    /*
     * Setup the system modal global data.
     */
    
    wmGD.systemModalActive = True;
    wmGD.systemModalClient = pCD;
    
    
} /* END OF FUNCTION SetupSystemModalState */



/*************************************<->*************************************
 *
 *  UndoSystemModalState ()
 *
 *
 *  Description:
 *  -----------
 *  This function cleans up after a system modal window goes away.
 *  
 *
 *  Inputs:
 *  ------
 *  wmGD = (system modal state data)
 *
 *
 *  Outputs:
 *  -------
 *  wmGD = changes to system modal state data
 *
 *************************************<->***********************************/

void UndoSystemModalState (void)
{
    int scr;
    
    /*
     * Unmap the system modal input screen window.
     */

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if(wmGD.Screens[scr].managed)
	{
	    XUnmapWindow (DISPLAY, wmGD.Screens[scr].inputScreenWindow);
	}
    }

    /*
     * Reset the focus if a window manager system modal dialog box was
     * being displayed.
     */

    if (!wmGD.systemModalClient)
    {
	AutoResetKeyFocus (NULL, GetTimestamp());
    }


    /*
     * Reset the system modal global data.
     */

    wmGD.systemModalActive = False;
    wmGD.systemModalClient = NULL;
    wmGD.systemModalWindow = 0;

} /* END OF FUNCTION UndoSystemModalState */



/*************************************<->*************************************
 *
 *  FindClientNameMatch (pStartingEntry, toNext, clientName, types)
 *
 *
 *  Description:
 *  -----------
 *  This function searches for a client that has a particular name or class.
 *  A match will be indicated if the client with the name or class also
 *  is in a particular state.
 *  
 *
 *  Inputs:
 *  ------
 *  pEntry = pointer to the client list entry where the search is
 *	to begin.
 *
 *  toNext = if True then search client list from first to last; otherwise
 *	search the client list last to first.
 *
 *  clientName = string that indicates a client name or class.
 *
 *  type = types of objects (icon, window, ...) that are to be matched.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = pointer to client list entry for matched client.
 *
 *************************************<->***********************************/
ClientListEntry * FindClientNameMatch (ClientListEntry *pEntry,
				       Boolean toNext,
				       String clientName,
				       unsigned long types)

{
    Boolean foundMatch = False;
    Boolean checkEntry;
    ClientData *pCD;


    while (!foundMatch && pEntry)
    {
	checkEntry = False;
	pCD = pEntry->pCD;
	if (pEntry->type == MINIMIZED_STATE)
	{
	    if ((pCD->clientState == MINIMIZED_STATE) &&
		(types & F_GROUP_ICON))
	    {
		checkEntry = True;
	    }
	}
	else
	{
	    if ((pCD->clientState != MINIMIZED_STATE) &&
		(types & F_GROUP_WINDOW))
	    {
		checkEntry = True;
	    }
	}

	if (checkEntry &&
	    ((pCD->clientName && (strcmp (clientName,pCD->clientName) == 0)) ||
	     (pCD->clientClass && (strcmp (clientName,pCD->clientClass) == 0))))
	{
	    foundMatch = True;
	}
	else
	{
	    pEntry = (toNext) ? pEntry->nextSibling : pEntry->prevSibling;
	}
    }

    return (pEntry);

} /* END OF FUNCTION FindClientNameMatch */
#ifdef WSM

/*************************************<->*************************************
 *
 *  BumpPrimaryToTop (pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function moves the primary window to the "top" of the transient
 *  tree. 
 *
 *  Inputs:
 *  ------
 *  pcdLeader	= pointer to client data of transient tree root.
 *
 *  Returns: True if stacking order of leader window changed.
 *           False if not stacking change.
 *
 *  Comments:
 *  ---------
 *  This affects only the clientData structures. There is no immediate
 *  effect on the actual stacking order on the display. That is done
 *  by StackWindow and/or RestackTransients.
 *
 *************************************<->***********************************/

Boolean BumpPrimaryToTop (ClientData *pcdLeader)
{
    int count;
    Boolean rval;

    count = CountTransientChildren (pcdLeader);

    if (pcdLeader->primaryStackPosition != (count-1))
    {
	pcdLeader->primaryStackPosition = count - 1;
	rval = True;
    }
    else
    {
	rval = False;
    }
    return (rval);
}


/*************************************<->*************************************
 *
 *  BumpPrimaryToBottom (pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function moves the primary window to the "bottom" of the transient
 *  tree. 
 *
 *  Inputs:
 *  ------
 *  pcdLeader	= pointer to client data of transient tree root.
 *
 *  Returns: True if stacking order of leader window changed.
 *           False if not stacking change.
 *
 *  Comments:
 *  ---------
 *  This affects only the clientData structures. There is no immediate
 *  effect on the actual stacking order on the display. That is done
 *  by StackWindow and/or RestackTransients.
 *
 *************************************<->***********************************/

Boolean BumpPrimaryToBottom (ClientData *pcdLeader)
{
    Boolean rval;

    if (pcdLeader->primaryStackPosition != 0)
    {
	pcdLeader->primaryStackPosition = 0;
	rval = True;
    }
    else
    {
	rval = False;
    }

    return (rval);
}


/*************************************<->*************************************
 *
 *  LowestWindowInTransientFamily (pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function returns the lowest stacked window in a transient
 *  tree family.
 *
 *  Inputs:
 *  ------
 *  pcdLeader = pointer to client data of leader of a transient tree
 *
 *  Returns:  id of lowest window in the transient tree for this family
 *
 *************************************<->***********************************/

Window
LowestWindowInTransientFamily (ClientData *pcdLeader)
{
    int count;
    static int size = 0;
    static Window *windows = NULL;
    Window wReturn = None;

    /*
     * Build a window list 
     */
    count = CountTransientChildren (pcdLeader);

    if (count > size)
    {
	/*
	 * Expand the (static) windows buffer
	 */

	if (!(windows =
		(Window *)WmMalloc ((char*)windows, (count + 5) * sizeof (Window))))
	{
	    /* cannot get memory space */
	    size = 0;
	    return;
	}
	size = count + 5;
    }

    MakeTransientFamilyStackingList (windows, pcdLeader);

    if (count > 0)
    {
	wReturn = windows[count-1];
    }
    else
    {
	wReturn = None;
    }

    return (wReturn);

} /* END OF FUNCTION LowestWindowInTransientFamily */


/*************************************<->*************************************
 *
 *  FindSubLeaderToTop (pcd)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies a candidate window to top within the 
 *  transient tree that is a local transient leader (the window has 
 *  transients hanging off of it, too). 
 *  
 *
 *  Inputs:
 *  ------
 *  pcd = pointer to client data of a transient leader window
 *
 *  Return:  ptr to client data for client that should be topped, or NULL
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

ClientData *
FindSubLeaderToTop (
	ClientData *pcd)

{
    ClientData *pcdRet = NULL;
    ClientData *pcdNext;

    pcdNext = pcd->transientChildren;
    while (pcdNext && (!pcdRet))
    {
	if (pcdNext->transientChildren)
	{
	    if (LeaderOnTop (pcdNext))
	    {
		pcdRet = pcdNext;
	    }
	    else
	    {
		pcdRet = FindSubLeaderToTop (pcdNext);
	    }
	}
	pcdNext = pcdNext->transientSiblings;
    }

    return (pcdRet);
}


/*************************************<->*************************************
 *
 *  MakeTransientFamilyStackingList (windows, pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a transient window list of windows in the 
 *  transient window tree headed by the specified client.  
 *  
 *
 *  Inputs:
 *  ------
 *  windows = pointer to the windows list to be filled out
 *
 *  pcdLeader = pointer to client data of a window in a transient tree
 *
 *  Outputs:
 *  -------
 *  The windows array is modified.
 *
 *  Comments:
 *  --------
 *  This function puts the transient leader window in the list in the 
 *  right place.
 *
 *************************************<->***********************************/

void
MakeTransientFamilyStackingList (
	Window *windows, 
	ClientData *pcdLeader)

{
    ClientData *pcdNext, *pcdSub;
    Window *nextWindow, wSave, wTemp, wTop;
    int count = CountTransientChildren (pcdLeader);
    register int i, j;

    /*
     * Construct the transient stacking list according to
     * normal Motif rules.
     */
    nextWindow = MakeTransientWindowList (windows, pcdLeader);

    if (!(pcdLeader->secondariesOnTop))
    {
	/*
	 * If the leader window shouldn't be on the bottom , then 
	 * adjust the stacking of the list.
	 */
	if ((pcdLeader->primaryStackPosition > 0) &&
	    (pcdLeader->primaryStackPosition < count))
	{
	    for (i=0; i<pcdLeader->primaryStackPosition; i++)
	    {
		j = count - i - 1;
		windows[j] = windows[j-1];
	    }
	    j = count - pcdLeader->primaryStackPosition - 1;
	    windows[j] = pcdLeader->clientFrameWin;
	}
	else
	{
	    /*
	     * Put the leader at the bottom.
	     */
	    *nextWindow = pcdLeader->clientFrameWin;

	    /*
	     * If one of the transients is also a local leader
	     * and wants to be on top, then adjust the list.
	     */
	    pcdSub = FindSubLeaderToTop (pcdLeader);
	    if (pcdSub && (pcdSub->clientFrameWin != None))
	    {
		/* insert this window at top */
		wTop = wSave = pcdSub->clientFrameWin;

		/* shuffle the rest down */
		for (i=0; i<count; i++)
		{
		    wTemp = windows[i];
		    windows[i] = wSave;
		    wSave = wTemp;

		    if (wTop == wSave)
			break;
		}
	    }
	}
    }
    else
    {
	/*
	 * Put the leader at the bottom.
	 */
	*nextWindow = pcdLeader->clientFrameWin;
    }

} /* END OF FUNCTION MakeTransientFamilyStackingList */


/*************************************<->*************************************
 *
 *  NormalizeTransientTreeStacking (pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function traverses the transient tree and cleans up any 
 *  local primary windows that are above their respective secondary
 *  windows.
 *  
 *
 *  Inputs:
 *  ------
 *  pcdLeader = pointer to client data of a transient tree leader
 *
 *  Return:  True if any changes in stacking order were made
 *  
 *
 *  Comments:
 *  --------
 *  This only touches the data structures. 
 *
 *************************************<->***********************************/

Boolean
NormalizeTransientTreeStacking (
	ClientData *pcdLeader)

{
    ClientData *pcdNext;
    Boolean bChanged = False;

    pcdNext = pcdLeader->transientChildren;
    bChanged = BumpPrimaryToBottom (pcdLeader);
    while (pcdNext)
    {
	if (pcdNext->transientChildren)
	{
	    bChanged |= BumpPrimaryToBottom (pcdNext);

	    bChanged |= NormalizeTransientTreeStacking (pcdNext);
	}
	pcdNext = pcdNext->transientSiblings;
    }
    return (bChanged);
}


/*************************************<->*************************************
 *
 *  LeaderOnTop (pcdLeader)
 *
 *
 *  Description:
 *  -----------
 *  This function tests a leader of a transient (sub) tree to see if
 *  it should be on top of its transient windows. 
 *  
 *
 *  Inputs:
 *  ------
 *  pcdLeader = pointer to client data of a transient tree leader
 *
 *  Return:  True if this leader is on top of its transients
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

Boolean
LeaderOnTop (
	ClientData *pcdLeader)

{
    Boolean bOnTop = False;
    int count = CountTransientChildren (pcdLeader);

    if ((pcdLeader->primaryStackPosition > 0) &&
	(pcdLeader->primaryStackPosition < count))
    {
	bOnTop = True;
    }

    return (bOnTop);
}

#endif /* WSM */
