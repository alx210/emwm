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
/* $TOG: WmCmd.c /main/11 1997/03/20 11:15:26 dbl $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <Xm/Xm.h>
#include <Xm/ScrollBarP.h>
#include "./WmWsmLib/utm_send.h"
#include "./WmWsmLib/wsm_proto.h"
#include "WmGlobal.h"
#include "WmFunction.h"
#include "WmCmd.h"
#include "WmMenu.h"
#include "WmResParse.h"
#include "WmWsm.h"
#include "WmDebug.h"
#include "WmWinConf.h"



#define MAX_CLIENT_CMDS 1000





static CARD32 cmdKillList[MAX_CLIENT_CMDS];
static unsigned long cmdKillListIndex;

#define ScrNum(w) (XScreenNumberOfScreen(XtScreen(w)))
#define CCI_TREE(w) (wmGD.Screens[ScrNum(w)].cciTree)
#define CCI_TREE_OF_SCR(scr) (wmGD.Screens[(scr)].cciTree)




/*----------------------------------------------------------------------*
 |                              NewCommand                              |
 *----------------------------------------------------------------------*/
CmdTree *
NewCommand (
     CARD32  commandID,
     CARD32  notifyWindow,
     char   *name,
     char   *defaultName)
	    
{
  CmdTree *ptr = (CmdTree *) XtMalloc(sizeof(CmdTree));

  ptr->commandID    = commandID;
  ptr->notifyWindow = notifyWindow;

  if (name != NULL) ptr->name = XtNewString(name);
  else              ptr->name = XtNewString("");
    
  if (defaultName != NULL) ptr->defaultName = XtNewString(defaultName);
  else                     ptr->defaultName = NULL;

  ptr->subTrees = NULL;

  ptr->next     = NULL;

  return(ptr);
}

/*---------------------------------------------------------------------------*
 |                                  FindCmd                                  |
 *---------------------------------------------------------------------------*/
CmdTree *
FindCmd(
     CARD32    commandID,
     CmdTree  *menuTree)
{
  CmdTree *pCmd;

  if (menuTree != NULL) PRINT("-> %s\n", menuTree->name);

  if (menuTree == NULL)
    {
      PRINT("<- NULL\n");
      return(NULL);
    }

  if (menuTree->commandID == commandID)
    {
      PRINT("<- %d, %s\n", commandID, menuTree->name);
      return(menuTree);
    }

  if ((pCmd = FindCmd(commandID, menuTree->next)) != NULL)
    return(pCmd);

  return(FindCmd(commandID, menuTree->subTrees));
}

/*---------------------------------------------------------------------------*
 |                             FindDuplicateName                             |
 | Scan through commandTree for another defined command with the same name.  |
 | Once found, return the ID of the duplicate.  Note that at most one match  |
 | can be found. If no match is found, 0 is returned.  This should NEVER be  |
 | the value of a command defined by a client.                               |
 *---------------------------------------------------------------------------*/
CARD32
FindDuplicateName(
     CmdTree  *menuTree,
     char     *name)
{
  CmdTree *tmp;
  CARD32  duplicateID;

  if (menuTree == NULL)
    return(0);

  else if ((menuTree->name != NULL) && (strcmp(menuTree->name, name) == 0))
    return(menuTree->commandID);

  else if ((duplicateID = FindDuplicateName(menuTree->subTrees, name)) != 0)
    return(duplicateID);

  else
    return(FindDuplicateName(menuTree->next, name));
}


/*----------------------------------------------------------------------*
 |                              AddCommand                              |
 | Returns true if the commandSet was found and a new command was added.|
 | Note that toplevel commands have 0 as their commandSet.  This will   |
 | always match the top entry in the command tree since it is 0.        |
 *----------------------------------------------------------------------*/
Boolean
AddCommand (
     int      scr,
     CARD32   commandSet,
     CARD32   commandID,
     CARD32   notifyWindow,
     char    *name,
     char    *defaultName,
     CmdTree *ptr)
{
  /*
   *  For each subtree of the cmdtree, look for the commandset.
   *  If found, add the element there else keep looking.
   */

  CmdTree *tmp;
  CARD32 duplicateID;

  if (ptr == NULL) return (False);

  else if (ptr->commandID == commandSet)
    /* FOUND IT */
    {
      if ((duplicateID = FindDuplicateName(CCI_TREE_OF_SCR(scr), name)) != 0)
	{
	  /* a duplicate menu item already exists here, remove it first.
	   */
	  if (ptr->subTrees != NULL)
	    DestroyMenuSpec (&(wmGD.Screens[scr]), commandID);
	  DeleteCommand(commandID, &CCI_TREE_OF_SCR(scr));
	}

      tmp = ptr->subTrees;
      ptr->subTrees = NewCommand(commandID, notifyWindow, name, defaultName);
      ptr->subTrees->next = tmp;

      return(True);
    }

  else
    {
      Boolean found;

      found = AddCommand(scr, commandSet, commandID, notifyWindow,
			 name, defaultName, ptr->next);
      if (!found)
	found = AddCommand(scr, commandSet, commandID, notifyWindow,
			   name, defaultName, ptr->subTrees);

      return (found);
    }
}


/*---------------------------------------------------------------------------*
 |                          RemoveCommandBranch                              |
 | This routine all commands at the specified node in the command tree.      |
 *---------------------------------------------------------------------------*/
void
RemoveCommandBranch (CmdTree *menuTree)
{
  CmdTree *tmp = menuTree;

  while (menuTree != NULL)
    {
      if (menuTree->subTrees != (CmdTree*)NULL)
	RemoveCommandBranch(menuTree->subTrees);

      menuTree = menuTree->next;
      if (tmp->name != NULL)
	XtFree((char*)tmp->name);
      XtFree((char*)tmp);
    }
}


/*---------------------------------------------------------------------------*
 |                              DeleteCommand                                |
 | This routine deletes any matching command entries in the Command Tree.    |
 *---------------------------------------------------------------------------*/
void
DeleteCommand (
     long      commandID,
     CmdTree **pmenuTree)
{
  if ((pmenuTree == NULL) || (*pmenuTree == NULL))
    return;

  else
    {
      CmdTree *tmp  = *pmenuTree;
      CmdTree *prev = NULL;

      while (tmp != (CmdTree *)NULL)

	if (tmp->commandID == commandID)
	  {
	    RemoveCommandBranch(tmp->subTrees); /* remove all sub-commands. */
	    if (prev == (CmdTree *)NULL)
	      *pmenuTree = tmp->next;
	    else
	      prev->next = tmp->next;
	    
	    if (tmp->name != NULL)
	      XtFree((char*)tmp->name);
	    XtFree((char *)tmp);
	    tmp = prev;
	  }

	else
	  {
	    if (tmp->subTrees != (CmdTree *)NULL)
	      DeleteCommand(commandID, &tmp->subTrees);
	    prev = tmp;
	    tmp = tmp->next;
	  }
    }
}


/*----------------------------------------------------------------------*
 |                            DefineCommand                             |
 | hack version - restriction: no spaces allowed in names.              |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
DefineCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32  commandID, commandSet, selection, duplicateID;
  String  name, defaultLabel;
  Boolean found = False;
  Window  owner;
  XWindowAttributes attr;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to DefineCommand.\n");
      return;
    }

  commandID    = UnpackCARD32(&data);
  selection    = UnpackCARD32(&data); /* selection to use for invoke cmd */
  commandSet   = UnpackCARD32(&data);
  name         = UnpackString(&data);
  defaultLabel = UnpackString(&data);

  PRINT("Define command: %d, %d, '%s', '%s'\n",
	commandID, commandSet, name, defaultLabel);
  
  /*
   *  Add command to menu structure.
   */

  if (CCI_TREE(w) == NULL)
    {
      CCI_TREE(w) = NewCommand(0, 0, NULL, NULL);
    }


  if (commandSet != 0)
    {
      found = AddCommand(ScrNum(w), commandSet, commandID, selection, name,
			 defaultLabel, CCI_TREE(w));
    }
  else
    /*
     * A leaf is being added to the command tree. There should not
     * be any other entries with the same name since matching with
     * the mwm resource file is done on names.  If a duplicate is
     * found, remove it first and any menu spec if it is a command
     * set. 
     */
    if ((duplicateID = FindDuplicateName(CCI_TREE(w), name)) != 0)
      {
	/* a duplicate menu item already exists here, remove it first.
	 */
	DestroyMenuSpec (&(wmGD.Screens[ScrNum(w)]), duplicateID);
	DeleteCommand(duplicateID, &CCI_TREE(w));
      }

  if (!found || commandSet == 0)
    {
      CmdTree *tmp;

      tmp = NewCommand(commandID, selection, name, defaultLabel);
      tmp->next = CCI_TREE(w)->next;
      CCI_TREE(w)->next = tmp;
    }
}



/*----------------------------------------------------------------------*
 |                            IncludeCommand                            |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
IncludeCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32         inLine, commandID, selection, count;
  Window        *windowIDs;
  CmdTree       *tPtr, *pNext;
  int            i, win;
  unsigned long  activeContext = 0L;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to IncludeCommand.\n");
      return;
    }

  inLine    = UnpackCARD32(&data);
  commandID = UnpackCARD32(&data);
  selection = UnpackCARD32(&data);
  count     = UnpackCARD32(&data);

  if (count > 0) windowIDs = (Window *) XtMalloc(sizeof(Window)*count);
  for (win=0; win<count; win++)
  {
      windowIDs[win] = UnpackCARD32(&data);
      PRINT("Got window ID %d.\n", windowIDs[win]);
  }

  /*
   *  Insert on root menu
   */
  if ((windowIDs[0] & WINDOW_MASK) == 0L)
    {
      ShowWaitState (TRUE);
	tPtr = FindCmd (commandID, CCI_TREE(w));
	if (tPtr != NULL)
	{
	    activeContext = F_CONTEXT_ROOT;
	    pNext = tPtr->next;
	    tPtr->next = NULL;
	    InsertTreeOnRootMenu(ACTIVE_PSD, tPtr, selection, inLine);
	    tPtr->next = pNext;
	}
      ShowWaitState (FALSE);
    }

  /*
   * Insert on all clients
   */
  else if ((windowIDs[0] & WINDOW_MASK) == ALL_WINDOWS)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  ShowWaitState (TRUE);

	  activeContext = 0L;
	  if (windowIDs[0] &   ICON_ONLY) activeContext |= F_CONTEXT_ICON;
	  if (windowIDs[0] & WINDOW_ONLY) activeContext |= F_CONTEXT_WINDOW;
      
	  pNext = tPtr->next;
	  tPtr->next = NULL;
	  InsertTreeOnAllClients (ACTIVE_PSD, tPtr, selection,
				  activeContext, inLine);
	  tPtr->next = pNext;

	  PRINT("Inserted commandID %d on all windows.\n", commandID);
	  
	  ShowWaitState (FALSE);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Insert on specified clients
   */
  else
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  ShowWaitState (TRUE);

	  for (i=0; i<count; i++)
	    {
	      ClientData *pCD;
	      
	      activeContext = 0L;
	      if (windowIDs[i] &   ICON_ONLY)
		activeContext |= F_CONTEXT_ICON;
	      if (windowIDs[i] & WINDOW_ONLY)
		activeContext |= F_CONTEXT_WINDOW;
	      pCD = GetPCD(ScrNum(w), windowIDs[i] & WINDOW_MASK);
	      if (pCD != NULL)
	      {
		  pNext = tPtr->next;
		  tPtr->next = NULL;
		  InsertTreeOnSingleClient (pCD->pSD, pCD, tPtr, selection,
					    activeContext, inLine);
		  tPtr->next = pNext;
	      }
	    }

	  ShowWaitState (FALSE);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  if (count > 0)
    XtFree((char*)windowIDs);
}



/*----------------------------------------------------------------------*
 |                             EnableCommand                            |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
EnableCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32         commandID, count;
  Window        *windowIDs;
  CmdTree       *tPtr, *pNext;
  int            i, win;
  unsigned long  activeContext = 0L;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to EnableCommand.\n");
      return;
    }

  commandID = UnpackCARD32(&data);
  count     = UnpackCARD32(&data);

  if (count > 0) windowIDs = (Window *) XtMalloc(sizeof(Window)*count);
  for (win=0; win<count; win++)
    {
      windowIDs[win] = UnpackCARD32(&data);
      PRINT("Got window ID %d.\n", windowIDs[win]);
    }

  /*
   *  Enable on root menu
   */
  if ((windowIDs[0] & WINDOW_MASK) == 0L)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	  {
	      activeContext = F_CONTEXT_ROOT;
	      pNext = tPtr->next;
	      tPtr->next = NULL;
	      ModifyClientCommandTree (ACTIVE_PSD, NULL, ROOT, tPtr,
				       ENABLE, activeContext, NULL);
	      tPtr->next = pNext;
	  }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Enable on all clients
   */
  else if ((windowIDs[0] & WINDOW_MASK) == ALL_WINDOWS)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  activeContext = 0L;
	  if (windowIDs[0] &   ICON_ONLY) activeContext |= F_CONTEXT_ICON;
	  if (windowIDs[0] & WINDOW_ONLY) activeContext |= F_CONTEXT_WINDOW;
      
	  pNext = tPtr->next;
	  tPtr->next = NULL;
	  ModifyClientCommandTree (ACTIVE_PSD, NULL, ALL, tPtr, ENABLE,
				   activeContext, NULL);
	  tPtr->next = pNext;

	  PRINT("Enabled commandID %d on all windows.\n", commandID);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Enable on specified clients
   */
  else
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	    {
	      ClientData *pCD;

	      activeContext = 0L;
	      if (windowIDs[i] &   ICON_ONLY)
		activeContext |= F_CONTEXT_ICON;
	      if (windowIDs[i] & WINDOW_ONLY)
		activeContext |= F_CONTEXT_WINDOW;
	      
	      pCD = GetPCD(ScrNum(w), windowIDs[i] & WINDOW_MASK);
	      if (pCD != NULL)
	      {
		  pNext = tPtr->next;
		  tPtr->next = NULL;
		  ModifyClientCommandTree (pCD->pSD, pCD, SINGLE, tPtr,
					   ENABLE, activeContext, NULL);
		  tPtr->next = pNext;
	      }
	    }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  if (count > 0)
    XtFree((char*)windowIDs);
}



/*----------------------------------------------------------------------*
 |                             DisableCommand                            |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
DisableCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32         commandID, count;
  Window        *windowIDs;
  CmdTree       *tPtr, *pNext;
  int            i, win;
  unsigned long  activeContext = 0L;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to DisableCommand.\n");
      return;
    }

  commandID = UnpackCARD32(&data);
  count     = UnpackCARD32(&data);

  if (count > 0) windowIDs = (Window *) XtMalloc(sizeof(Window)*count);
  for (win=0; win<count; win++)
    {
      windowIDs[win] = UnpackCARD32(&data);
      PRINT("Got window ID %d.\n", windowIDs[win]);
    }

  /*
   *  Disable on root menu
   */
  if ((windowIDs[0] & WINDOW_MASK) == 0L)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	  {
	      activeContext = F_CONTEXT_ROOT;
	      pNext = tPtr->next;
	      tPtr->next = NULL;
	      ModifyClientCommandTree (ACTIVE_PSD, NULL, ROOT, tPtr,
				       DISABLE, activeContext, NULL);
	      tPtr->next = pNext;
	  }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Disable on all clients
   */
  else if ((windowIDs[0] & WINDOW_MASK) == ALL_WINDOWS)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  activeContext = 0L;
	  if (windowIDs[0] &   ICON_ONLY) activeContext |= F_CONTEXT_ICON;
	  if (windowIDs[0] & WINDOW_ONLY) activeContext |= F_CONTEXT_WINDOW;
      
	  pNext = tPtr->next;
	  tPtr->next = NULL;
	  ModifyClientCommandTree (ACTIVE_PSD, NULL, ALL, tPtr,
				   DISABLE, activeContext, NULL);
	  tPtr->next = pNext;

	  PRINT("Disabled commandID %d on all windows.\n", commandID);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Disable on specified clients
   */
  else
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	    {
	      ClientData *pCD;

	      activeContext = 0L;
	      if (windowIDs[i] &   ICON_ONLY)
		activeContext |= F_CONTEXT_ICON;
	      if (windowIDs[i] & WINDOW_ONLY)
		activeContext |= F_CONTEXT_WINDOW;
	      
	      pCD = GetPCD(ScrNum(w), windowIDs[i] & WINDOW_MASK);
	      if (pCD != NULL)
	      {
		  pNext = tPtr->next;
		  tPtr->next = NULL;
		  ModifyClientCommandTree (pCD->pSD, pCD, SINGLE, tPtr,
					   DISABLE, activeContext, NULL);
		  tPtr->next = pNext;
	      }
	    }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  if (count > 0)
    XtFree((char*)windowIDs);
}



/*----------------------------------------------------------------------*
 |                             RenameCommand                            |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
RenameCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32         commandID, count;
  Window        *windowIDs;
  CmdTree       *tPtr, *pNext;
  int            i, win;
  unsigned long  activeContext = 0L;
  String	 newname;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to RenameCommand.\n");
      return;
    }

  commandID = UnpackCARD32(&data);
  newname   = UnpackString(&data);
  count     = UnpackCARD32(&data);

  if (count > 0) windowIDs = (Window *) XtMalloc(sizeof(Window)*count);
  for (win=0; win<count; win++)
    {
      windowIDs[win] = UnpackCARD32(&data);
      PRINT("Got window ID %d.\n", windowIDs[win]);
    }

  /*
   *  Rename on root menu
   */
  if ((windowIDs[0] & WINDOW_MASK) == 0L)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	  {
	      activeContext = F_CONTEXT_ROOT;
	      pNext = tPtr->next;
	      tPtr->next = NULL;
	      ModifyClientCommandTree (ACTIVE_PSD, NULL, ROOT, tPtr,
				       RENAME, activeContext, newname);
	      tPtr->next = pNext;
	  }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Rename on all clients
   */
  else if ((windowIDs[0] & WINDOW_MASK) == ALL_WINDOWS)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  activeContext = 0L;
	  if (windowIDs[0] &   ICON_ONLY) activeContext |= F_CONTEXT_ICON;
	  if (windowIDs[0] & WINDOW_ONLY) activeContext |= F_CONTEXT_WINDOW;
      
	  pNext = tPtr->next;
	  tPtr->next = NULL;
	  ModifyClientCommandTree (ACTIVE_PSD, NULL, ALL, tPtr,
				   RENAME, activeContext, newname);
	  tPtr->next = pNext;

	  PRINT("Renamed commandID %d on all windows.\n", commandID);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Rename on specified clients
   */
  else
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	    {
	      ClientData *pCD;

	      activeContext = 0L;
	      if (windowIDs[i] &   ICON_ONLY)
		activeContext |= F_CONTEXT_ICON;
	      if (windowIDs[i] & WINDOW_ONLY)
		activeContext |= F_CONTEXT_WINDOW;
	      
	      pCD = GetPCD(ScrNum(w), windowIDs[i] & WINDOW_MASK);
	      if (pCD != NULL)
	      {
		  pNext = tPtr->next;
		  tPtr->next = NULL;
		  ModifyClientCommandTree (pCD->pSD, pCD, SINGLE, tPtr,
					   RENAME, activeContext, newname);
		  tPtr->next = pNext;
	      }
	    }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  if (count > 0)
    XtFree((char*)windowIDs);
}



/*----------------------------------------------------------------------*
 |                             RemoveCommand                            |
 *----------------------------------------------------------------------*/
/*ARGSUSED*/
void
RemoveCommand (
     Widget         w,
     Atom           target,
     MessageData    data,
     unsigned long  len,
     int            fmt)
{
  CARD32         commandID, count;
  Window        *windowIDs;
  CmdTree       *tPtr, *pNext;
  int            i, win;
  unsigned long  activeContext = 0L;

  /*
   * check data to make sure somethings there.
   */
  if ((data == NULL) || (len == 0))
    {
      PRINT("Bad data passed to RemoveCommand.\n");
      return;
    }

  commandID = UnpackCARD32(&data);
  count     = UnpackCARD32(&data);

  if (count > 0) windowIDs = (Window *) XtMalloc(sizeof(Window)*count);
  for (win=0; win<count; win++)
    {
      windowIDs[win] = UnpackCARD32(&data);
      PRINT("Got window ID %d.\n", windowIDs[win]);
    }

  /*
   *  Remove on root menu
   */
  if ((windowIDs[0] & WINDOW_MASK) == 0L)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	  {
	      activeContext = F_CONTEXT_ROOT;
	      pNext = tPtr->next;
	      tPtr->next = NULL;
	      ModifyClientCommandTree (ACTIVE_PSD, NULL, ROOT, tPtr,
				       REMOVE, activeContext, NULL);
	      tPtr->next = pNext;
	  }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Remove on all clients
   */
  else if ((windowIDs[0] & WINDOW_MASK) == ALL_WINDOWS)
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  activeContext = 0L;
	  if (windowIDs[0] &   ICON_ONLY) activeContext |= F_CONTEXT_ICON;
	  if (windowIDs[0] & WINDOW_ONLY) activeContext |= F_CONTEXT_WINDOW;
      
	  pNext = tPtr->next;
	  tPtr->next = NULL;
	  ModifyClientCommandTree (ACTIVE_PSD, NULL, ALL, tPtr, REMOVE,
				   activeContext, NULL);
	  tPtr->next = pNext;

	  PRINT("Removed commandID %d on all windows.\n", commandID);
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  /*
   * Remove on specified clients
   */
  else
    {
      tPtr = FindCmd (commandID, CCI_TREE(w));
      if (tPtr != NULL)
	{
	  for (i=0; i<count; i++)
	  {
	      ClientData *pCD;
	      
	      activeContext = 0L;
	      if (windowIDs[i] &   ICON_ONLY)
		activeContext |= F_CONTEXT_ICON;
	      if (windowIDs[i] & WINDOW_ONLY)
		activeContext |= F_CONTEXT_WINDOW;

	      pCD = GetPCD(ScrNum(w), windowIDs[i] & WINDOW_MASK);
	      if (pCD != NULL)
	      {
		  pNext = tPtr->next;
		  tPtr->next = NULL;
		  ModifyClientCommandTree (pCD->pSD, pCD, SINGLE, tPtr,
					   REMOVE, activeContext, NULL);
		  tPtr->next = pNext;
	      }
	    }
	}
      else
	PRINT("ERROR - commandID %d not found in command tree.\n", commandID);
    }

  if (count > 0)
    XtFree((char*)windowIDs);
}



/*---------------------------------------------------------------------------*
 |                           RemoveMatchingCommands                          |
 | Recursively removes commands that match a given window in the specified   |
 | command tree.                                                             |
 *---------------------------------------------------------------------------*/
/*ARGSUSED*/
void
RemoveMatchingCommands (
     int    scr,
     Window clientWindow,
     CmdTree *tree)
{
  if (tree == NULL)
    return;

  if (tree->subTrees != NULL)
    RemoveMatchingCommands(scr, clientWindow, tree->subTrees);

  if (tree->notifyWindow == clientWindow)
    {
      cmdKillList[ cmdKillListIndex++ ] = tree->commandID;
    }

  if (tree->next != NULL)
    RemoveMatchingCommands(scr, clientWindow, tree->next);
}



/*---------------------------------------------------------------------------*
 |                           RemoveCommandsForClient                         |
 | This function will remove any command that was inserted by this client.   |
 *---------------------------------------------------------------------------*/
void
RemoveCommandsForClient (
     int    scr,
     Window clientWindow)
{
  int i;

  ShowWaitState (TRUE);

  cmdKillListIndex = 0;
  RemoveMatchingCommands(scr, clientWindow, CCI_TREE_OF_SCR(scr));

  /*
   * Remove any matching commands on the root menu.
   */
  for (i = 0;  i < cmdKillListIndex;  i++)
    {
      CmdTree *tPtr, *pNext;
      MenuSpec *pMS;
      WmScreenData *pSD = ACTIVE_PSD;

      while (tPtr = FindCmd (cmdKillList[ i ], CCI_TREE_OF_SCR(scr)))
	{
	  /* make sure ModifyClientCommandTree can't muck with cmdTree. */
	  pNext = tPtr->next;
	  tPtr->next = NULL;

	  ModifyClientCommandTree (pSD, NULL, ALL, tPtr,
				   REMOVE, F_CONTEXT_ALL, NULL);
	  /* restore tree. */
	  tPtr->next = pNext;

	  /*
	   * if this command caused a menu spec to be created, remove it.
	   */
	  DestroyMenuSpec(pSD, cmdKillList[i]);

	  /*
	   * Now delete the matching entry in the command tree.
	   */
	  DeleteCommand(cmdKillList[ i ], &CCI_TREE_OF_SCR(scr));
	}
    }

  ShowWaitState (FALSE);
}


/*---------------------------------------------------------------------------*
 |                            InvokeMessageReply                             |
 *---------------------------------------------------------------------------*/
/*ARGSUSED*/
void
InvokeMessageReply (Widget w, XtPointer clientData, XtPointer callData)
{
  PRINT("Invoke message reply received.\n");
}



/*---------------------------------------------------------------------------*
 |                            SendInvokeMessage                              |
 | Send a convert request to the client that owns the menu command.          |
 *---------------------------------------------------------------------------*/
void
SendInvokeMessage (
     CARD32 commandID,
     CARD32 clientWindow,
     Atom   selection,
     Time   time)
{
  MessageData  msg, fulldata;
  unsigned long size;

  size = (unsigned long)(sizeof(CARD32) + sizeof(CARD32));
  msg = fulldata = (MessageData)XtMalloc(sizeof(CARD8) * size);
  msg = PackCARD32(msg, commandID);
  msg = PackCARD32(msg, clientWindow);

  UTMSendMessage(ACTIVE_PSD->utmShell,
		 selection,
		 wmGD._MOTIF_WM_INVOKE_COMMAND,
		 (XtPointer)fulldata, size, WSM_PROTO_FMT,
		 InvokeMessageReply, NULL, /* no client data */
		 time);
}

/*----------------------------------------------------------------------*
 |                             CopyMwmGadget
 *----------------------------------------------------------------------*/

static void
CopyMwmGadget(GadgetRectangle *mwm_gadget, GadgetRectangle *auto_gadget)
{
	auto_gadget->id = mwm_gadget->id;
	auto_gadget->rect.x = mwm_gadget->rect.x;
	auto_gadget->rect.y = mwm_gadget->rect.y;
	auto_gadget->rect.width = mwm_gadget->rect.width;
	auto_gadget->rect.height = mwm_gadget->rect.height;
}


/*----------------------------------------------------------------------*
 |                             FillInvalidInfo
 *----------------------------------------------------------------------*/

static void
FillInvalidInfo(GadgetRectangle *auto_gadget)
{
	auto_gadget->id = INVALID;
	auto_gadget->rect.x = auto_gadget->rect.y = auto_gadget->rect.width =
	auto_gadget->rect.height = INVALID;
}


/*----------------------------------------------------------------------*
 |                             GetMinimizeInfo
 *----------------------------------------------------------------------*/

static void
GetMinimizeInfo(ClientData *pcd, XtPointer reply)
{
  /*
   * This function packs data about the minimize button in the following
   * order : 1. gadget count 2. id  3. x position 4. y position 5. width
   *         6. height
   */


  GadgetRectangle minimize_button;
  CARD32 filledCount;
  int i;
  Window frameWin;
  Boolean minFound = False;

      




   filledCount = pcd->cTitleGadgets;
   frameWin = pcd->clientFrameWin;
      
	
   for (i=0; i < filledCount; i++)
 	if (pcd->pTitleGadgets[i].id == FRAME_MINIMIZE)
	   {
	        CopyMwmGadget (&(pcd->pTitleGadgets[i]), &minimize_button); 
		minFound = True;
	   }

   if (minFound == False)
	FillInvalidInfo (&minimize_button);
	
		

      reply = PackCARD32 (reply, (CARD32)filledCount);
      reply = PackCARD32(reply, (CARD32)minimize_button.id);
      reply = PackCARD32 (reply, (CARD32)minimize_button.rect.x);
      reply = PackCARD32 (reply, (CARD32)minimize_button.rect.y);
      reply = PackCARD32 (reply, (CARD32)minimize_button.rect.width);
      reply = PackCARD32 (reply, (CARD32)minimize_button.rect.height); 
      reply = PackCARD32 (reply, (CARD32)frameWin);

}

/*----------------------------------------------------------------------*
 |                             GetMaximizeInfo
 *----------------------------------------------------------------------*/

static void
GetMaximizeInfo(ClientData *pcd, XtPointer reply)
{

  /*
   * This function packs data about the maximize button in the following
   * order : 1. gadget count 2. id  3. x position 4. y position 5. width
   *         6. height 7. event window
   */

      int i;
      GadgetRectangle maximize_button;
      CARD32 filledCount;
      Window frameWin;
      Boolean maxFound = False;


      filledCount = pcd->cTitleGadgets;
      frameWin = pcd->clientFrameWin;


      for (i=0; i < filledCount; i++)
	{
     
        if (pcd->pTitleGadgets[i].id == FRAME_MAXIMIZE)
	  {
	    CopyMwmGadget (&(pcd->pTitleGadgets[i]),&maximize_button);
	    maxFound = True;
          }

      	}
      
      if (maxFound == False)
	FillInvalidInfo (&maximize_button);



	reply = PackCARD32 (reply, (CARD32)filledCount);
	reply = PackCARD32 (reply, (CARD32)maximize_button.id);
	reply = PackCARD32 (reply, (CARD32)maximize_button.rect.x);
	reply = PackCARD32 (reply, (CARD32)maximize_button.rect.y);
	reply = PackCARD32 (reply, (CARD32)maximize_button.rect.width);
	reply = PackCARD32 (reply, (CARD32)maximize_button.rect.height);
	reply = PackCARD32 (reply, (CARD32)frameWin);
}


/*----------------------------------------------------------------------*
 |                             GetIconInfo
 *----------------------------------------------------------------------*/

static void
GetIconInfo(ClientData *pcd, XtPointer reply, Boolean use_icon_box)
{

   IconBoxData *icon_box;
   XmScrollBarWidget  hScrollBar, vScrollBar;
   Widget frameWidget, scrollWidget, shellWidget;

   

   CARD32 iconX, iconY, iconWidth, iconHeight ;
   CARD32 hMin, hMax, hSliderAreaWidth, hSliderX, hSliderAreaX,
          vMin, vMax, vSliderAreaHeight, vSliderY, vSliderAreaY;
   CARD32 rightArrowX, rightArrowY, leftArrowX, leftArrowY, 
          topArrowX, topArrowY, bottomArrowX, bottomArrowY;
   CARD32 iconBoxX, iconBoxY, iconBoxWidth, iconBoxHeight;
   Window frameWin, scrollWin, hScrollWin, vScrollWin, iconShellWin, iconFrameWin;
   CARD32 lastRow, lastCol;
   CARD32 iPlaceW, iPlaceH;
   CARD32 useIconBox;
   
   
   icon_box = pcd->pIconBox;
   useIconBox = pcd->pSD->useIconBox;
   



       /*
	* IconBox info
	*/
       if (use_icon_box == True)
	 {
	   hScrollBar =  (XmScrollBarWidget)icon_box->hScrollBar;

	   hMin = hScrollBar->scrollBar.minimum;
	   hMax = hScrollBar->scrollBar.maximum;
	   hSliderAreaWidth = hScrollBar->scrollBar.slider_area_width;
	   hSliderX = hScrollBar->scrollBar.slider_x;
	   hSliderAreaX = hScrollBar->scrollBar.slider_area_x;
	   leftArrowX = hScrollBar->core.x + 
	                  hScrollBar->scrollBar.arrow1_x +
	                  (hScrollBar->scrollBar.arrow_width/2);
	   leftArrowY = hScrollBar->core.y + 
	                  hScrollBar->scrollBar.arrow1_y +
	                  (hScrollBar->scrollBar.arrow_height/2);

	   rightArrowX = hScrollBar->core.x + hScrollBar->scrollBar.arrow2_x
	                  + (hScrollBar->scrollBar.arrow_width/2);
	   rightArrowY = hScrollBar->core.y + hScrollBar->scrollBar.arrow2_y
	                  + (hScrollBar->scrollBar.arrow_height/2);

	   vScrollBar =  (XmScrollBarWidget)icon_box->vScrollBar;

	   vMin = vScrollBar->scrollBar.minimum;
	   vMax = vScrollBar->scrollBar.maximum;
	   vSliderAreaHeight = vScrollBar->scrollBar.slider_area_height;
	   vSliderY = vScrollBar->scrollBar.slider_y;
	   vSliderAreaY = vScrollBar->scrollBar.slider_area_y;

	   topArrowX = vScrollBar->core.x + vScrollBar->scrollBar.arrow1_x
	                  + (vScrollBar->scrollBar.arrow_width/2);
	   topArrowY = vScrollBar->core.y + vScrollBar->scrollBar.arrow1_y
	                  + (vScrollBar->scrollBar.arrow_height/2);

	   bottomArrowX = vScrollBar->core.x + vScrollBar->scrollBar.arrow2_x
	                  + (vScrollBar->scrollBar.arrow_width/2);
	   bottomArrowY = hScrollBar->core.y + hScrollBar->scrollBar.arrow2_y
	                  + (hScrollBar->scrollBar.arrow_height/2);


	   

	   shellWidget = icon_box->shellWidget;


	   iconBoxX = shellWidget->core.x;
	   iconBoxY = shellWidget->core.y;
	   iconBoxWidth = shellWidget->core.width;
	   iconBoxHeight = shellWidget->core.height;
	   iconShellWin = XtWindow (shellWidget);


	   frameWidget = icon_box->frameWidget;
	   frameWin = XtWindow (frameWidget);

	   scrollWidget = icon_box->scrolledWidget;
	   scrollWin = XtWindow (scrollWidget);
	   
	   hScrollWin = XtWindow (hScrollBar);
	   vScrollWin = XtWindow (vScrollBar);
	   
	   lastCol = icon_box->lastCol;
	   lastRow = icon_box->lastRow;
	   iPlaceW = icon_box->IPD.iPlaceW;
	   iPlaceH = icon_box->IPD.iPlaceH;
	}


       /*
	* icon info
	*/

       iconX = pcd->iconX;
       iconY = pcd->iconY;
       iconWidth = pcd->pSD->iconWidth;


       iconHeight = pcd->pSD->iconHeight;
       iconFrameWin = pcd->iconFrameWin;



       reply = PackCARD32 (reply,(CARD32)useIconBox);
       reply = PackCARD32 (reply,(CARD32)iconX);
       reply = PackCARD32 (reply,(CARD32)iconY);
       reply = PackCARD32 (reply,(CARD32)iconWidth);
       reply = PackCARD32 (reply,(CARD32)iconHeight);
       reply = PackCARD32 (reply,(CARD32)iconFrameWin);

       if (use_icon_box == True)
	 {
	   reply = PackCARD32 (reply,(CARD32)hMin);
	   reply = PackCARD32 (reply,(CARD32)hMax);
	   reply = PackCARD32 (reply,(CARD32)hSliderAreaWidth);
	   reply = PackCARD32 (reply,(CARD32)hSliderX);
	   reply = PackCARD32 (reply,(CARD32)hSliderAreaX);
	   reply = PackCARD32 (reply,(CARD32)vMin);
	   reply = PackCARD32 (reply,(CARD32)vMax);
	   reply = PackCARD32 (reply,(CARD32)vSliderAreaHeight);
	   reply = PackCARD32 (reply,(CARD32)vSliderY);
	   reply = PackCARD32 (reply,(CARD32)vSliderAreaY);
	   reply = PackCARD32 (reply,(CARD32)rightArrowX);
	   reply = PackCARD32 (reply,(CARD32)rightArrowY);
	   reply = PackCARD32 (reply,(CARD32)leftArrowX);
	   reply = PackCARD32 (reply,(CARD32)leftArrowY);
	   reply = PackCARD32 (reply,(CARD32)topArrowX);
	   reply = PackCARD32 (reply,(CARD32)topArrowY);
	   reply = PackCARD32 (reply,(CARD32)bottomArrowX);
	   reply = PackCARD32 (reply,(CARD32)bottomArrowY);
	   reply = PackCARD32 (reply,(CARD32)iconBoxX);
	   reply = PackCARD32 (reply,(CARD32)iconBoxY);
	   reply = PackCARD32 (reply,(CARD32)iconBoxWidth);
	   reply = PackCARD32 (reply,(CARD32)iconBoxHeight);
	   reply = PackCARD32 (reply,(CARD32)iconShellWin);
	   reply = PackCARD32 (reply,(CARD32)frameWin);
	   reply = PackCARD32 (reply,(CARD32)scrollWin);
	   reply = PackCARD32 (reply,(CARD32)hScrollWin);
	   reply = PackCARD32 (reply,(CARD32)vScrollWin);
	   reply = PackCARD32 (reply,(CARD32)lastCol);
	   reply = PackCARD32 (reply,(CARD32)lastRow);
	   reply = PackCARD32 (reply,(CARD32)iPlaceH);
	   reply = PackCARD32 (reply,(CARD32)iPlaceW);
	   
	}

}

/*----------------------------------------------------------------------*
 |                             GetMoveInfo
 *----------------------------------------------------------------------*/

static void
GetMoveInfo(ClientData *pcd, XtPointer reply)

{
      int i;
      GadgetRectangle title;
      GadgetRectangle menu;
      CARD32 filledCount;
      CARD32 upperBorderWidth, lowerBorderWidth;
      CARD32 windowX, windowY;
      Window frameWin;

      Boolean titleFound = False;
      Boolean system_found = False;


      filledCount = pcd->cTitleGadgets;
      upperBorderWidth = pcd->frameInfo.upperBorderWidth;
      lowerBorderWidth = pcd->frameInfo.lowerBorderWidth;
      windowX = pcd->clientX;
      windowY = pcd->clientY;
      frameWin = pcd->clientFrameWin;

      for (i=0; i < filledCount; i++)
	{
	  if (pcd->pTitleGadgets[i].id == FRAME_TITLE)
	    {
		CopyMwmGadget (&(pcd->pTitleGadgets[i]), &title);
		titleFound = True;
	    }

	  if (pcd->pTitleGadgets[i].id == FRAME_SYSTEM)
	    {
	      CopyMwmGadget (&(pcd->pTitleGadgets[i]), &menu);
	      system_found = True;
	    }

        }

        if (titleFound == False)
	 FillInvalidInfo (&title);

        if (system_found == False)
	 FillInvalidInfo (&menu);
			  

	 reply = PackCARD32 (reply, (CARD32)filledCount);
	 reply = PackCARD32 (reply, (CARD32)title.id);
	 reply = PackCARD32 (reply, (CARD32)title.rect.x);
	 reply = PackCARD32 (reply, (CARD32)title.rect.y);
	 reply = PackCARD32 (reply, (CARD32)title.rect.width);
	 reply = PackCARD32 (reply, (CARD32)title.rect.height);
       	 reply = PackCARD32 (reply, (CARD32)menu.id);
	 reply = PackCARD32 (reply, (CARD32)menu.rect.x);
	 reply = PackCARD32 (reply, (CARD32)menu.rect.y);
	 reply = PackCARD32 (reply, (CARD32)menu.rect.width);
	 reply = PackCARD32 (reply, (CARD32)menu.rect.height);
	 reply = PackCARD32 (reply, (CARD32)upperBorderWidth);
	 reply = PackCARD32 (reply, (CARD32)lowerBorderWidth);
	 reply = PackCARD32 (reply, (CARD32)windowX);
	 reply = PackCARD32 (reply, (CARD32)windowY);
	 reply = PackCARD32 (reply, (CARD32)frameWin);

    }




/*----------------------------------------------------------------------*
 |                             GetResizeInfo
 *----------------------------------------------------------------------*/

static void
GetResizeInfo(ClientData *pcd, XtPointer reply, int dir)
{
      int i;
      GadgetRectangle east_resize, west_resize, gravity_resize, title;
      CARD32  upperBorderWidth, lowerBorderWidth;
      Window frameWin;
      int filledCount;
      Boolean titleFound = False;


      filledCount = pcd->cTitleGadgets;

      if (!(pcd->decor & MWM_DECOR_RESIZEH))
      	FillInvalidInfo (&gravity_resize);
      else
	{
		CopyMwmGadget (&pcd->pResizeGadgets[dir], &gravity_resize);
		CopyMwmGadget (&pcd->pResizeGadgets[WM_WEST], &west_resize);
		CopyMwmGadget (&pcd->pResizeGadgets[WM_EAST], &east_resize);
	}


       for (i=0; i < filledCount; i++)
	if (pcd->pTitleGadgets[i].id == FRAME_TITLE)
	   {
	      CopyMwmGadget (&(pcd->pTitleGadgets[i]), &title);
              titleFound = True;
	   }

	if (titleFound == False)
	   FillInvalidInfo (&title);
		


	    

        upperBorderWidth = pcd->frameInfo.upperBorderWidth;
        lowerBorderWidth = pcd->frameInfo.lowerBorderWidth;
	frameWin = pcd->clientFrameWin;

		


	 reply = PackCARD32 (reply, (CARD32)east_resize.id);
	 reply = PackCARD32 (reply, (CARD32)east_resize.rect.width);
	 reply = PackCARD32 (reply, (CARD32)west_resize.id);
	 reply = PackCARD32 (reply, (CARD32)west_resize.rect.width);
	 reply = PackCARD32 (reply, (CARD32)gravity_resize.id);
	 reply = PackCARD32 (reply, (CARD32)gravity_resize.rect.x);
	 reply = PackCARD32 (reply, (CARD32)gravity_resize.rect.y);
	 reply = PackCARD32 (reply, (CARD32)gravity_resize.rect.width);
	 reply = PackCARD32 (reply, (CARD32)gravity_resize.rect.height);
	 reply = PackCARD32 (reply, (CARD32)title.id);
	 reply = PackCARD32 (reply, (CARD32)title.rect.x);
	 reply = PackCARD32 (reply, (CARD32)title.rect.y);
	 reply = PackCARD32 (reply, (CARD32)title.rect.width);
	 reply = PackCARD32 (reply, (CARD32)title.rect.height);
	 reply = PackCARD32 (reply, (CARD32)upperBorderWidth);
	 reply = PackCARD32 (reply, (CARD32)lowerBorderWidth);
	 reply = PackCARD32 (reply, (CARD32)frameWin);
}


/*----------------------------------------------------------------------*
 |                             GetWindowMenuUnpostInfo
 *----------------------------------------------------------------------*/

static void
GetWindowMenuUnpostInfo(ClientData *pcd, XtPointer reply)
{

    MenuSpec *menuSpec;

    CARD32 clientState;
    CARD32 menuWin;
    CARD32 frameWin;

    menuSpec = pcd->systemMenuSpec;
    

    clientState = pcd->clientState;
    menuWin = XtWindow (menuSpec->menuWidget);
    frameWin = pcd->clientFrameWin;

    reply = PackCARD32 (reply,clientState);
    reply = PackCARD32 (reply,menuWin);
    reply = PackCARD32 (reply,frameWin);
    
}

/*----------------------------------------------------------------------*
 |                             GetFocusInfo
 *----------------------------------------------------------------------*/

static void
GetFocusInfo(ClientData *pcd, XtPointer reply)
{


  GadgetRectangle title;
  CARD32 filledCount;
  int i;
  Window frameWin;
  Boolean titleFound = False;

      

      filledCount = pcd->cTitleGadgets;
      frameWin = pcd->clientFrameWin;

      for (i=0; i < filledCount; i++)
        {
          if (pcd->pTitleGadgets[i].id == FRAME_TITLE)
            {
	      CopyMwmGadget (&(pcd->pTitleGadgets[i]), &title);
              titleFound = True;

            }
	}


   if (titleFound == False)
	FillInvalidInfo (&title);
	
		

      reply = PackCARD32 (reply, (CARD32)filledCount);
      reply = PackCARD32(reply, (CARD32)title.id);
      reply = PackCARD32 (reply, (CARD32)title.rect.x);
      reply = PackCARD32 (reply, (CARD32)title.rect.y);
      reply = PackCARD32 (reply, (CARD32)title.rect.width);
      reply = PackCARD32 (reply, (CARD32)title.rect.height);
      reply = PackCARD32 (reply, (CARD32)frameWin);

}

/*----------------------------------------------------------------------*
 |                             GetWindowMenuPostInfo
 *----------------------------------------------------------------------*/

static void
GetWindowMenuPostInfo(ClientData *pcd, XtPointer reply)
{
      int i;
      GadgetRectangle title;
      GadgetRectangle menu;
      CARD32 filledCount;
      Window frameWin;


      Boolean titleFound = False;
      Boolean systemFound = False;


      filledCount = pcd->cTitleGadgets;
      frameWin = pcd->clientFrameWin;

      for (i=0; i < filledCount; i++)
        {
          if (pcd->pTitleGadgets[i].id == FRAME_TITLE)
            {
	      CopyMwmGadget (&(pcd->pTitleGadgets[i]), &title);
              titleFound = True;

            }

          if (pcd->pTitleGadgets[i].id == FRAME_SYSTEM)
            {
	      CopyMwmGadget (&(pcd->pTitleGadgets[i]), &menu);
              systemFound = True;
            }

        }


        if (titleFound == False)
		FillInvalidInfo (&title);

        if (systemFound == False)
          	FillInvalidInfo (&menu);


        reply = PackCARD32 (reply, (CARD32)filledCount);
	reply = PackCARD32 (reply, (CARD32)title.id);
        reply = PackCARD32 (reply, (CARD32)title.rect.x);
        reply = PackCARD32 (reply, (CARD32)title.rect.y);
        reply = PackCARD32 (reply, (CARD32)title.rect.width);
        reply = PackCARD32 (reply, (CARD32)title.rect.height);
	reply = PackCARD32 (reply, (CARD32)menu.id);
        reply = PackCARD32 (reply, (CARD32)menu.rect.x);
        reply = PackCARD32 (reply, (CARD32)menu.rect.y);
        reply = PackCARD32 (reply, (CARD32)menu.rect.width);
        reply = PackCARD32 (reply, (CARD32)menu.rect.height);
        reply = PackCARD32 (reply, (CARD32)frameWin);
}


/*----------------------------------------------------------------------*
 |                             GetIconMenuItemSelectInfo
 *----------------------------------------------------------------------*/

static void
GetIconMenuItemSelectInfo(ClientData *pcd, XtPointer reply, Boolean use_icon_box)
{
  MenuSpec *menuSpec;
  int menuItemCount,sensitiveCount;
  Window menuWin, frameWin;
  int sensitive[MAX_MENU_ITEMS];
  int itemY[MAX_MENU_ITEMS];
  char itemName[MAX_MENU_ITEMS][25];


   IconBoxData *icon_box;
   XmScrollBarWidget  hScrollBar, vScrollBar;
   Widget frameWidget, scrollWidget, shellWidget;
  
  MenuButton              *NewMenuButton;
  MenuItem                *NewMenuItem;
  int n;
  CARD32 iconX, iconY, iconWidth, iconHeight;
  CARD32 hMin, hMax, hSliderAreaWidth, hSliderX, hSliderAreaX,
          vMin, vMax, vSliderAreaHeight, vSliderY, vSliderAreaY;
  CARD32 rightArrowX, rightArrowY, leftArrowX, leftArrowY, 
          topArrowX, topArrowY, bottomArrowX, bottomArrowY;
  CARD32 iconBoxX, iconBoxY, iconBoxWidth, iconBoxHeight;
  Window scrollWin, hScrollWin, vScrollWin, iconShellWin, iconFrameWin;
  CARD32 lastRow, lastCol;
  CARD32 iPlaceW, iPlaceH;
  CARD32 clientState;
  CARD32 useIconBox;



  useIconBox = pcd->pSD->useIconBox;

  icon_box = pcd->pIconBox;

  menuSpec = pcd->systemMenuSpec;

  if (menuSpec == NULL)
    return;

  menuItemCount = menuSpec->menuButtonCount;

  for (n = 0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
    {
      itemName[n][0] = '\0';
      sensitive[n] = FALSE;
    }

  menuWin = XtWindow (menuSpec->menuWidget);
  frameWin = pcd->clientFrameWin;
  clientState = pcd->clientState;

      sensitiveCount = 0;

      for (n = 0, NewMenuButton = menuSpec->menuButtons;
	   n < menuSpec->menuButtonCount;
	   n++, NewMenuButton++)
	{
	  if (NewMenuButton->managed == FALSE)
            continue;

	  NewMenuItem = NewMenuButton->menuItem;
	  sensitive[n] = NewMenuButton->buttonWidget->core.sensitive;
	  itemY[n] = NewMenuButton->buttonWidget->core.y +
                            (NewMenuButton->buttonWidget->core.height / 2);
	  strcpy(itemName[n], NewMenuItem->label);

	  if (sensitive[n] == TRUE)
            sensitiveCount++;
	}


  


       if (use_icon_box == True)
	 {
	   hScrollBar =  (XmScrollBarWidget)icon_box->hScrollBar;
	   hMin = hScrollBar->scrollBar.minimum;
	   hMax = hScrollBar->scrollBar.maximum;
	   hSliderAreaWidth = hScrollBar->scrollBar.slider_area_width;
	   hSliderX = hScrollBar->scrollBar.slider_x;
	   hSliderAreaX = hScrollBar->scrollBar.slider_area_x;
	   leftArrowX = hScrollBar->core.x + 
	                  hScrollBar->scrollBar.arrow1_x +
	                  (hScrollBar->scrollBar.arrow_width/2);
	   leftArrowY = hScrollBar->core.y + 
	                  hScrollBar->scrollBar.arrow1_y +
	                  (hScrollBar->scrollBar.arrow_height/2);

	   rightArrowX = hScrollBar->core.x + hScrollBar->scrollBar.arrow2_x
	                  + (hScrollBar->scrollBar.arrow_width/2);
	   rightArrowY = hScrollBar->core.y + hScrollBar->scrollBar.arrow2_y
	                  + (hScrollBar->scrollBar.arrow_height/2);

	   vScrollBar =  (XmScrollBarWidget)icon_box->vScrollBar;

	   vMin = vScrollBar->scrollBar.minimum;
	   vMax = vScrollBar->scrollBar.maximum;
	   vSliderAreaHeight = vScrollBar->scrollBar.slider_area_height;
	   vSliderY = vScrollBar->scrollBar.slider_y;
	   vSliderAreaY = vScrollBar->scrollBar.slider_area_y;

	   topArrowX = vScrollBar->core.x + vScrollBar->scrollBar.arrow1_x
	                  + (vScrollBar->scrollBar.arrow_width/2);
	   topArrowY = vScrollBar->core.y + vScrollBar->scrollBar.arrow1_y
	                  + (vScrollBar->scrollBar.arrow_height/2);

	   bottomArrowX = vScrollBar->core.x + vScrollBar->scrollBar.arrow2_x
	                  + (vScrollBar->scrollBar.arrow_width/2);
	   bottomArrowY = hScrollBar->core.y + hScrollBar->scrollBar.arrow2_y
	                  + (hScrollBar->scrollBar.arrow_height/2);


	   

	   shellWidget = icon_box->shellWidget;


	   iconBoxX = shellWidget->core.x;
	   iconBoxY = shellWidget->core.y;
	   iconBoxWidth = shellWidget->core.width;
	   iconBoxHeight = shellWidget->core.height;
	   iconShellWin = XtWindow (shellWidget);


	   frameWidget = icon_box->frameWidget;
	   frameWin = XtWindow (frameWidget);

	   scrollWidget = icon_box->scrolledWidget;
	   scrollWin = XtWindow (scrollWidget);
	   
	   hScrollWin = XtWindow (hScrollBar);
	   vScrollWin = XtWindow (vScrollBar);
	   
	   lastCol = icon_box->lastCol;
	   lastRow = icon_box->lastRow;
	   iPlaceW = icon_box->IPD.iPlaceW;
	   iPlaceH = icon_box->IPD.iPlaceH;
	}


       /*
	* icon info
	*/

       iconX = pcd->iconX;
       iconY = pcd->iconY;
       iconWidth = pcd->pSD->iconWidth;
       iconHeight = pcd->pSD->iconHeight;
       iconFrameWin = pcd->iconFrameWin;



      reply = PackCARD32 (reply, (CARD32)clientState);
      reply = PackCARD32 (reply, (CARD32)menuItemCount);
      reply = PackCARD32 (reply, (CARD32)sensitiveCount);
      reply = PackCARD32 (reply, (CARD32)menuWin);
      reply = PackCARD32 (reply, (CARD32)frameWin);
      for (n=0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
	{
	  reply = PackCARD32 (reply, (CARD32)sensitive[n]);
	  reply = PackCARD32 (reply, (CARD32)itemY[n]);	
	  reply = PackString (reply, (String)itemName[n]);		
	}

       reply = PackCARD32 (reply,(CARD32)use_icon_box);
       reply = PackCARD32 (reply,(CARD32)iconX);
       reply = PackCARD32 (reply,(CARD32)iconY);
       reply = PackCARD32 (reply,(CARD32)iconWidth);
       reply = PackCARD32 (reply,(CARD32)iconHeight);
       reply = PackCARD32 (reply,(CARD32)iconFrameWin);

       if (use_icon_box == True)
	 {
	   reply = PackCARD32 (reply,(CARD32)hMin);
	   reply = PackCARD32 (reply,(CARD32)hMax);
	   reply = PackCARD32 (reply,(CARD32)hSliderAreaWidth);
	   reply = PackCARD32 (reply,(CARD32)hSliderX);
	   reply = PackCARD32 (reply,(CARD32)hSliderAreaX);
	   reply = PackCARD32 (reply,(CARD32)vMin);
	   reply = PackCARD32 (reply,(CARD32)vMax);
	   reply = PackCARD32 (reply,(CARD32)vSliderAreaHeight);
	   reply = PackCARD32 (reply,(CARD32)vSliderY);
	   reply = PackCARD32 (reply,(CARD32)vSliderAreaY);
	   reply = PackCARD32 (reply,(CARD32)rightArrowX);
	   reply = PackCARD32 (reply,(CARD32)rightArrowY);
	   reply = PackCARD32 (reply,(CARD32)leftArrowX);
	   reply = PackCARD32 (reply,(CARD32)leftArrowY);
	   reply = PackCARD32 (reply,(CARD32)topArrowX);
	   reply = PackCARD32 (reply,(CARD32)topArrowY);
	   reply = PackCARD32 (reply,(CARD32)bottomArrowX);
	   reply = PackCARD32 (reply,(CARD32)bottomArrowY);
	   reply = PackCARD32 (reply,(CARD32)iconBoxX);
	   reply = PackCARD32 (reply,(CARD32)iconBoxY);
	   reply = PackCARD32 (reply,(CARD32)iconBoxWidth);
	   reply = PackCARD32 (reply,(CARD32)iconBoxHeight);
	   reply = PackCARD32 (reply,(CARD32)iconShellWin);
	   reply = PackCARD32 (reply,(CARD32)frameWin);
	   reply = PackCARD32 (reply,(CARD32)scrollWin);
	   reply = PackCARD32 (reply,(CARD32)hScrollWin);
	   reply = PackCARD32 (reply,(CARD32)vScrollWin);
	   reply = PackCARD32 (reply,(CARD32)lastCol);
	   reply = PackCARD32 (reply,(CARD32)lastRow);
	   reply = PackCARD32 (reply,(CARD32)iPlaceH);
	   reply = PackCARD32 (reply,(CARD32)iPlaceW);
	   
	}

}




/*----------------------------------------------------------------------*
 |                             GetItemSelectInfo
 *----------------------------------------------------------------------*/

static void
GetWindowItemSelectInfo(ClientData *pcd, XtPointer reply)
{
  MenuSpec *menuSpec;
  CARD32 menuItemCount,sensitiveCount;
  Window menuWin, frameWin;
  CARD32 sensitive[MAX_MENU_ITEMS];
  CARD32 itemY[MAX_MENU_ITEMS];
  char itemName[MAX_MENU_ITEMS][MAX_NAME_LEN + 1];
  
  MenuButton              *NewMenuButton;
  MenuItem                *NewMenuItem;
  int n;


  menuSpec = pcd->systemMenuSpec;

  if (menuSpec == NULL)
    return;

  menuItemCount = menuSpec->menuButtonCount;

  for (n = 0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
    {
      itemName[n][0] = '\0';
      sensitive[n] = FALSE;
    }

  menuWin = XtWindow (menuSpec->menuWidget);
  frameWin = pcd->clientFrameWin;

   sensitiveCount = 0;

      for (n = 0, NewMenuButton = menuSpec->menuButtons;
	   n < menuSpec->menuButtonCount;
	   n++, NewMenuButton++)
	{
	  if (NewMenuButton->managed == FALSE)
            continue;

	  NewMenuItem = NewMenuButton->menuItem;
	  sensitive[n] = NewMenuButton->buttonWidget->core.sensitive;
	  itemY[n] = NewMenuButton->buttonWidget->core.y +
                            (NewMenuButton->buttonWidget->core.height / 2);
	  strcpy(itemName[n], NewMenuItem->label);

	  if (sensitive[n] == TRUE)
            sensitiveCount++;
	}

      reply = PackCARD32 (reply, (CARD32)menuItemCount);
      reply = PackCARD32 (reply, (CARD32)sensitiveCount);
      reply = PackCARD32 (reply, (CARD32)menuWin);
      reply = PackCARD32 (reply, (CARD32)frameWin);
      for (n=0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
	{
	  reply = PackCARD32 (reply, (CARD32)sensitive[n]);
	  reply = PackCARD32 (reply, (CARD32)itemY[n]);	
	  reply = PackString (reply, (String)itemName[n]);		
	}

}


/*----------------------------------------------------------------------*
 |                             GetItemCheckInfo
 *----------------------------------------------------------------------*/

static void
GetItemCheckInfo(ClientData *pcd, XtPointer reply)
{
  MenuSpec *menuSpec;
  int menuItemCount;
  int clientState;
  int titleGadgetCount;
  int titleId, systemId, minimizeId, maximizeId, northwestId;
  int upperBorderWidth, lowerBorderWidth;
  Window menuWin;
  char itemName[MAX_MENU_ITEMS][MAX_NAME_LEN + 1];
  
  MenuButton              *NewMenuButton;
  MenuItem                *NewMenuItem;
  int i,filledCount;
  Boolean titleFound, systemFound, minimizeFound, maximizeFound = False;

  menuSpec = pcd->systemMenuSpec;

  if (menuSpec == NULL)
    return;

  menuItemCount = menuSpec->menuButtonCount;

  for (i = 0; i < menuItemCount && i < MAX_MENU_ITEMS; i++)
      itemName[i][0] = '\0'; 

  menuWin = XtWindow (menuSpec->menuWidget);

      for (i = 0, NewMenuButton = menuSpec->menuButtons;
	   i < menuSpec->menuButtonCount;
	   i++, NewMenuButton++)
	{
	  if (NewMenuButton->managed == FALSE)
            continue;

	  NewMenuItem = NewMenuButton->menuItem;
	  strcpy (itemName[i],NewMenuItem->label);

	}

      filledCount = pcd->cTitleGadgets;

      for (i=0; i < filledCount; i++)
        {
          if (pcd->pTitleGadgets[i].id == FRAME_TITLE)
	      { 
		  titleId = FRAME_TITLE;
		  titleFound = True;
	      }
	  else
          if (pcd->pTitleGadgets[i].id == FRAME_SYSTEM)
	      {
		  systemId = FRAME_SYSTEM;
		  systemFound = True;
	      }

	  else
          if (pcd->pTitleGadgets[i].id == FRAME_MINIMIZE)
	      {
		  minimizeId = FRAME_MINIMIZE;
		  minimizeFound = True;
	      }
	  else
          if (pcd->pTitleGadgets[i].id == FRAME_MAXIMIZE)
	      {
		  maximizeId = FRAME_MAXIMIZE;
		  maximizeFound = True;
	      }


        }

   if (titleFound == False)
       titleId = INVALID;
   if (systemFound == False)
       systemId = INVALID;
   if (minimizeFound == False)
       minimizeId = INVALID;
   if (maximizeFound == False)
       maximizeId = INVALID;

   if (!(pcd->decor & MWM_DECOR_RESIZEH))
       northwestId = INVALID;
   else
       northwestId = pcd->pResizeGadgets[WM_NORTHWEST].id;

  upperBorderWidth = pcd->frameInfo.upperBorderWidth;
  lowerBorderWidth = pcd->frameInfo.lowerBorderWidth;
      	



  clientState = pcd->clientState;

  reply = PackCARD32 (reply, (CARD32)clientState);
  reply = PackCARD32 (reply, (CARD32)menuWin);
  reply = PackCARD32 (reply, (CARD32)menuItemCount);
  for (i=0; i < menuItemCount && i < MAX_MENU_ITEMS; i++)
      reply = PackString (reply, (String)itemName[i]);		
  reply = PackCARD32 (reply, (CARD32)filledCount);
  reply = PackCARD32 (reply, (CARD32)titleId);
  reply = PackCARD32 (reply, (CARD32)systemId);	
  reply = PackCARD32 (reply, (CARD32)minimizeId);	  
  reply = PackCARD32 (reply, (CARD32)maximizeId);	
  reply = PackCARD32 (reply, (CARD32)northwestId);	
  reply = PackCARD32 (reply, (CARD32)upperBorderWidth);	
  reply = PackCARD32 (reply, (CARD32)lowerBorderWidth);	
}


/*----------------------------------------------------------------------*
 |                             GetRaiseInfo
 *----------------------------------------------------------------------*/

static void
GetRaiseInfo(ClientData *pcd, XtPointer reply)
{

  CARD32 state = pcd->clientState;
  reply = PackCARD32 (reply, (CARD32)state);
}


/*----------------------------------------------------------------------*
 |                             GetAutomationData
 *----------------------------------------------------------------------*/
void
GetAutomationData (XtPointer input, Atom *outputType, XtPointer *output, unsigned long *outputLen, int *outputFmt)
{
  ClientData *pcd;
  CARD32 infoWanted;
  
  Window winId;  
  XtPointer reply;
  int size;

  int i, n, menuItemCount;
  MenuItem *menu_item;





  winId        = UnpackCARD32(&input);
  infoWanted    = UnpackCARD32(&input);


  
  /*
   * Get client data associated with window if the widget making the request.
   */

  if ((pcd = GetPCD (0,winId)) == NULL)
    {

      PRINT ("Can't get client data\n");
      return;
    }


  size = 0;

  switch (infoWanted)
      {
	case WINDOW_MINIMIZE_INFO:
	  size += sizeof(CARD32) * 7;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetMinimizeInfo (pcd,*output);
          break;

	case WINDOW_MAXIMIZE_INFO:
	  size += sizeof (CARD32) * 7;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetMaximizeInfo(pcd,*output);
          break;

        case WINDOW_RAISE_INFO:
	  size += sizeof (CARD32);
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetRaiseInfo(pcd, *output);
	  break;

        case WINDOW_MOVE_INFO:
	   SetGrabServer();    /* tell mwm not to grab server */
	   size += sizeof (CARD32) * 16;
	    *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	   GetMoveInfo(pcd,*output);
	   break;

	case WINDOW_RESIZE_NORTH_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_NORTH);
	  break;

	case WINDOW_RESIZE_SOUTH_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_SOUTH);
	  break;

	case WINDOW_RESIZE_EAST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_EAST);
	  break;

	case WINDOW_RESIZE_WEST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_WEST);
	  break;

	case WINDOW_RESIZE_NORTHEAST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_NORTHEAST);
	  break;


	case WINDOW_RESIZE_NORTHWEST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_NORTHWEST);
	  break;


	case WINDOW_RESIZE_SOUTHEAST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_SOUTHEAST);
	  break;


	case WINDOW_RESIZE_SOUTHWEST_INFO:
	  SetGrabServer();    /* tell mwm not to grab server */
	  size += sizeof (CARD32) * 17;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetResizeInfo(pcd,*output,WM_SOUTHWEST);
	  break;


	case WINDOW_MENU_ITEM_SELECT_INFO:
	  if (pcd->systemMenuSpec)
	      menuItemCount = pcd->systemMenuSpec->menuButtonCount;
	  size += sizeof(CARD32) * 4;
	  for (n=0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
	      {
		  menu_item = pcd->systemMenuSpec->menuButtons->menuItem;
		  size += sizeof (CARD32);    /* sensitive */
		  size += sizeof (CARD32);    /* itemY */
		  size += sizeof (CARD16) + ((strlen(menu_item->label) + 1) * sizeof(CARD8)); /*itemName */
	      }
	 *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetWindowItemSelectInfo(pcd,*output);
	  break;
      
	case WINDOW_DEICONIFY_INFO:
	  if (pcd->pSD->useIconBox == True)
	      {
		  size += sizeof (CARD32) * 37;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,True);
	      }
	  else
	      {
		  size += sizeof (CARD32) * 6;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,False);
	      }
	  break;


	case WINDOW_MENU_POST_INFO:
	   size += sizeof (CARD32) * 12;
	   *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	   GetWindowMenuPostInfo(pcd,*output);
	   break;

	case WINDOW_FOCUS_INFO:
	   size += sizeof (CARD32) * 7;
	   *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	   GetFocusInfo(pcd,*output);
	   break;


	case WINDOW_MENU_UNPOST_INFO:
	   size += sizeof (CARD32) * 3;
	   *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	   GetWindowMenuUnpostInfo(pcd,*output);
	   break;

	case WINDOW_MENU_ITEM_CHECK_INFO:
	  if (pcd->systemMenuSpec)
	      menuItemCount = pcd->systemMenuSpec->menuButtonCount;
	  size += sizeof(CARD32) * 11;
	  for (n=0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
	      {
		  menu_item = pcd->systemMenuSpec->menuButtons->menuItem;
		  size += sizeof (CARD16) + (strlen(menu_item->label + 1) * sizeof(CARD8)); /*itemName */
	      }
	 *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetItemCheckInfo(pcd, *output);
	  break;

	case ICON_MOVE_INFO:
	  {
	      SetGrabServer();    /* tell mwm not to grab server */
	      size += sizeof (CARD32) * 6;
	      *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	      GetIconInfo(pcd,*output,False);
	      break;
	  }

	case ICON_MENU_POST_INFO:
	  if (pcd->pSD->useIconBox == True)
	      {
		  size += sizeof (CARD32) * 37;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,True);
	      }
	  else
	      {
		  size += sizeof (CARD32) * 6;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,False);
	      }
	  break;

	case ICON_MENU_UNPOST_INFO:
	  if (pcd->pSD->useIconBox == True)
	      {
		  size += sizeof (CARD32) * 37;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,True);

	      }
	  else
	      {
		  size += sizeof (CARD32) * 6;
		  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
		  GetIconInfo(pcd,*output,False);
	      }
	  break;

	case ICON_MENU_ITEM_SELECT_INFO:
	  if (pcd->systemMenuSpec)
	      menuItemCount = pcd->systemMenuSpec->menuButtonCount;
	  size += sizeof(CARD32) * 11;
	  for (n=0; n < menuItemCount && n < MAX_MENU_ITEMS; n++)
	      {
		  size += sizeof (CARD32);    /* sensitive */
		  size += sizeof (CARD32);    /* itemY */
		  menu_item = pcd->systemMenuSpec->menuButtons->menuItem;
		  size += sizeof (CARD16) + ((strlen(menu_item->label) + 1) * sizeof(CARD8)); /*itemName */
	      }
	  if (pcd->pSD->useIconBox == True)
		  size += sizeof (CARD32) * 37;
	  else
	  	  size += sizeof (CARD32) * 6;
	  *output = (XtPointer)XtMalloc(sizeof(CARD8) * size);
	  GetIconMenuItemSelectInfo (pcd,*output,pcd->pSD->useIconBox);
	  break;


	default:
	  PRINT ("Illegal operation from Automation");
	  break;
      }

	*outputLen = (unsigned long)size;
	*outputFmt = WSM_PROTO_FMT;
	
}

