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
static char rcsid[] = "$XConsortium: WmMenu.c /main/15 1996/11/20 15:20:17 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */
/*
 * (c) Copyright 1987, 1988 DIGITAL EQUIPMENT CORPORATION */
/*
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmCEvent.h"
#include "WmResource.h"
#include "WmResParse.h"
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# include "WmDebug.h"
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
#include <stdio.h>
#include <ctype.h>

#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MenuShell.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/RowColumnP.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>

#define SHELL_NAME "menu"
#define SEPARATOR_NAME "separator"
#define TITLE_NAME "title_name"
#define CASCADE_BTN_NAME "cascadebutton"
#define PUSH_BTN_NAME "pushbutton"

#define CHILDREN_CACHE  22
#define MENU_BUTTON_INC 5

/*
 * include extern functions
 */
#include "WmMenu.h"
#include "WmCDecor.h"
#include "WmColormap.h"
#include "WmEvent.h"
#include "WmFunction.h"
#include "WmIconBox.h"
#include "WmImage.h"
#include "WmError.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */


static void UnmapCallback (Widget w, XtPointer client_data,
			   XtPointer call_data);
static MenuItem *DuplicateMenuItems (MenuItem *menuItems);

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
static MenuExclusion *DuplicateMenuExclusions(MenuExclusion *exclusions);
static Boolean FindClientCommandMatch (MenuSpec *menuSpec,
				       String clientCommand,
				       MenuItem **menuItem);
static void InsertTreeOnClient (WmScreenData *pSD, ClientData *pCD,
				CmdTree *tree,
				MatchList **client_match_list,
				MatchList **global_match_list,
				MenuSpec *menuSpec, MenuItem *template,
				String command_so_far,
				Boolean duplicate_globals, Atom selection,
				Context greyed_context, Boolean inLine);
static MenuSpec *MakeMenuSpec (String menuName, CARD32 commandID);
static void UnmapPulldownCallback (Widget w, XtPointer client_data,
				   XtPointer call_data);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */



/*************************************<->*************************************
 *
 *  MakeMenu (menuName, initialContext, accelContext, moreMenuItems,
 *            fSystemMenu)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a menu widget.
 *
 *
 *  Inputs:
 *  ------
 *  menuName       = name of the top-level menu pane for the menu
 *  initialContext = initial context for menuitem sensitivity
 *  accelContext   = accelerator context
 *  moreMenuItems  = additional menuitems for custom menu.
 *  fSystemMenu    = TRUE iff the menu is a client system menu.
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a MenuSpec structure with updated currentContext,
 *           menuWidget, and menuButtons members.
 *
 *
 *  Comments:
 *  --------
 *  If moreMenuItems is nonNULL, a custom MenuSpec will be created, with
 *  menuItem member pointing to moreMenuItems.  The menuItems for the
 *  standard MenuSpec of the same name and the moreMenuItems list will be 
 *  used to create menubuttons, and the menu widget will be separate from 
 *  any existing standard menu widget.
 *
 *  When the client is destroyed, this custom MenuSpec, its menuItem and
 *  menuButton lists, and its menu widget should be freed.
 * 
 *************************************<->***********************************/
MenuSpec *MakeMenu (WmScreenData *pSD,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
		    ClientData *pCD,
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
		    String menuName, Context initialContext,
		    Context accelContext, MenuItem *moreMenuItems,
		    Boolean fSystemMenu)
{
    unsigned int n;
    MenuSpec     *menuSpec;
    MenuSpec     *newMenuSpec;
    MenuItem     *menuItem;
    KeySpec      *accelKeySpec;
    
    if ((menuName == NULL) || (pSD == NULL))
    {
	return (NULL);
    }

    /*
     * Look for the menu specification:
     */

    menuSpec = pSD->menuSpecs;
    while (menuSpec)
    {
	if ((menuSpec->name != NULL) && !strcmp (menuSpec->name, menuName))
	/* Found the menu pane. */
	{
	    break;
	}
	menuSpec = menuSpec->nextMenuSpec;
    }
    
    if (menuSpec == NULL)
    /* the menuSpecs list is exhausted */
    {
	MWarning(((char *)GETMESSAGE(48, 1, "Menu specification %s not found\n")), menuName);
	return (NULL);
    }

    /*
     * The top-level menu pane specification was found.
     * Adjust the menu accelerator context?
     */

    if (fSystemMenu)
    {
	accelContext = 0;
    }
    else if (accelContext & F_CONTEXT_ROOT)
    /* root context accelerators apply everywhere */
    {
	accelContext = F_CONTEXT_ALL;
    }

    /*
     * If making a custom menu, create a custom copy of the specification with 
     *   which to build the custom menu.
     * Otherwise, if the menu widget exists, possibly modify the accelerator
     *   contexts and return the specification.
     */

    if (moreMenuItems != NULL)
    {
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	if ((newMenuSpec = DuplicateMenuSpec(menuSpec)) == (MenuSpec *)NULL)
	    return NULL;
#else
        if ((newMenuSpec = (MenuSpec *) XtMalloc (sizeof (MenuSpec))) == NULL)
	/* Handle insufficent memory */
        {
            MWarning(((char *)GETMESSAGE(48, 2, "Insufficient memory for menu %s\n")), menuName);
	    return (NULL);
        }
	newMenuSpec->name = NULL;  /* distinguishes this as custom */
	newMenuSpec->whichButton = SELECT_BUTTON;
	newMenuSpec->height = 0;
	newMenuSpec->menuItems = menuSpec->menuItems; /* temporary */
	newMenuSpec->accelContext = menuSpec->accelContext;
	newMenuSpec->accelKeySpecs = NULL;
	newMenuSpec->nextMenuSpec = NULL;
#endif

	menuSpec = newMenuSpec;
    }
    else if (menuSpec->menuWidget)
    {
	/* 
	 * OR the accelContext into the accelerators, if necessary.
	 */
        if (accelContext != (menuSpec->accelContext & accelContext))
        {
            menuSpec->accelContext |= accelContext;
	    accelKeySpec = menuSpec->accelKeySpecs;
	    while (accelKeySpec)
	    {
		accelKeySpec->context |= accelContext;
	        accelKeySpec = accelKeySpec->nextKeySpec;
	    }
	}
        return (menuSpec);
    }

    /*
     * We have a menu specification with which to build the menu.
     * Set the initial and accelerator contexts -- they are needed within 
     *   CreateMenuWidget.
     */

    menuSpec->currentContext = initialContext;
    menuSpec->accelContext = accelContext;

    /*
     * Scan the toplevel MenuSpec and create its initial menuButtons array
     * if any of its items will need to be included.  This array will be
     * created or enlarged within CreateMenuWidget below if necessary.
     */

    n = 0;
    menuItem = menuSpec->menuItems;
    while (menuItem)
    {
	if ((menuItem->greyedContext) || (menuItem->mgtMask))
	{
	    n++;
	}
	menuItem = menuItem->nextMenuItem;
    }
    menuItem = moreMenuItems;
    while (menuItem)
    {
	if ((menuItem->greyedContext) || (menuItem->mgtMask))
	{
	    n++;
	}
	menuItem = menuItem->nextMenuItem;
    }
    if (n)
    {
        if ((menuSpec->menuButtons =
	       (MenuButton *) XtMalloc (n * sizeof(MenuButton))) == NULL)
        /* insufficent memory */
	{
            MWarning(((char *)GETMESSAGE(48, 3, "Insufficient memory for menu %s\n")), menuName);
	    return (NULL);
	}
        menuSpec->menuButtonSize = n;
    }
    else
    {
        menuSpec->menuButtons = NULL;
        menuSpec->menuButtonSize = 0;
    }
    menuSpec->menuButtonCount = 0;

    /*
     * Create a PopupShell widget as a child of the workspace manager widget
     *   and a PopupMenu as a child of the shell.
     * Fill the PopupMenu with the menu items.
     */

    menuSpec->menuWidget = CREATE_MENU_WIDGET (pSD, pCD, menuName, 
					     pSD->screenTopLevelW,
					     TRUE, menuSpec, moreMenuItems);
    if (menuSpec->menuWidget == NULL)
    {
        /*
	 *  Could not make the top-level menu pane.
	 */
	return (NULL);
    }
/*
    _XmSetPopupMenuClick(menuSpec->menuWidget, False); 
*/
    /* Return the top MenuSpec */

    return (menuSpec);

} /* END OF FUNCTION MakeMenu */



/*************************************<->***********************************/
void CheckTerminalSeparator(menuSpec, buttonWidget, manage)
     MenuSpec *menuSpec;
     Widget buttonWidget;
     Boolean manage;
{
    CompositeWidget cw;
    WidgetList      children;
    Cardinal        wPos;


    if ((menuSpec == NULL) || (menuSpec->menuWidget == NULL))
      {
	 return;
      }

    cw = (CompositeWidget)menuSpec->menuWidget;
    children = cw->composite.children;

    for (wPos = 0;  wPos < cw->composite.num_children; wPos++)
    {
	if((Widget)children[wPos] == buttonWidget)
	{
	    break;
	}
    }
    
    
    if(wPos > 0 &&
       XtClass((Widget) children[wPos -1]) == xmSeparatorGadgetClass)
    {
	if(manage)
	{
	    if (!(XtIsManaged((Widget)children[wPos -1])))
	    {
		XtManageChild((Widget)children[wPos -1]);
	    }
	}
	else
	{
	    if (XtIsManaged((Widget)children[wPos -1]))
	    {
		XtUnmanageChild((Widget)children[wPos -1]);
	    }
	}
    }

} /* END OF FUNCTION CheckTerminalSeparator */



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  MakeMenuSpec (menuName, commandID)
 *  
 *
 *
 *  Description:
 *  -----------
 *  This function creates and returns a MenuSpec structure.
 *
 *
 *  Inputs:
 *  ------
 *  menuName       = name of the menu specification
 *  commandID      = client command id of the menu item to build.
 *                   0 if not for a client command.
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a MenuSpec structure with zero'ed fields.
 *
 *
 *  Comments:
 *  --------
 *  A new MenuSpec structure is allocated. The name is set to the
 *  menuName argument. The menuItems list, menuButtons list and 
 *  accelerator related fields are zero'ed out to NULL values. 
 * 
 *************************************<->***********************************/
static MenuSpec *
MakeMenuSpec (String menuName, CARD32 commandID)
{
    MenuSpec *menuSpec;
    
    if ((menuSpec = (MenuSpec *) XtMalloc (sizeof (MenuSpec))) == NULL)
      /* Handle insufficent memory */
    {
	MWarning(((char *)GETMESSAGE(48, 2,
		 "Insufficient memory for menu %s\n")), menuName);
	return (NULL);
    }
    
    menuSpec->name = XtNewString(menuName);
    menuSpec->currentContext = F_CONTEXT_ALL;
    menuSpec->menuWidget = (Widget) NULL;
    menuSpec->whichButton = SELECT_BUTTON;
    menuSpec->height = 0;
    menuSpec->menuItems = (MenuItem *) NULL;
    menuSpec->menuButtons = (MenuButton *) NULL;
    menuSpec->menuButtonSize = 0;
    menuSpec->menuButtonCount = 0;
    menuSpec->accelContext = F_CONTEXT_ALL;
    menuSpec->accelKeySpecs = (KeySpec *) NULL;
    menuSpec->exclusions = (MenuExclusion *) NULL;
    menuSpec->clientLocal = FALSE;
    menuSpec->commandID = commandID;
    menuSpec->nextMenuSpec = (MenuSpec *) NULL;
    
    return(menuSpec);
}
#endif

/*************************************<->*************************************
 *
 *  DuplicateMenuItems (menuItems)
 *  
 *
 *
 *  Description:
 *  -----------
 *  This function creates an indentical duplicate of the given menuItems
 *  list.
 *
 *
 *  Inputs:
 *  ------
 *  menuItems = the linked list of menuItems to duplicate
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuItems list, identical to the original
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static MenuItem *
DuplicateMenuItems (MenuItem *menuItems)
{
    MenuItem *newMenuItem = (MenuItem *) NULL, *returnMenuItem, *curMenuItem;
    
    for (curMenuItem = menuItems;
	 curMenuItem != (MenuItem *) NULL;
	 curMenuItem = curMenuItem->nextMenuItem)
    {
	/* If its the first one ... */
	if (newMenuItem == (MenuItem *) NULL)
	{
	    newMenuItem = (MenuItem *)XtMalloc(sizeof(MenuItem));
	    returnMenuItem = newMenuItem;
	}
	else /* ... otherwise, get the next menuItem. */
	{
	    newMenuItem->nextMenuItem =
	      (MenuItem *)XtMalloc(sizeof(MenuItem));
	    newMenuItem = newMenuItem->nextMenuItem;
	}
	
	newMenuItem->labelType = curMenuItem->labelType;
	if (curMenuItem->label != (String) NULL)
	  newMenuItem->label = XtNewString(curMenuItem->label);
	else
	  newMenuItem->label = NULL;
	newMenuItem->labelBitmapIndex = curMenuItem->labelBitmapIndex;
	newMenuItem->mnemonic = curMenuItem->mnemonic;
	newMenuItem->accelState = curMenuItem->accelState;
	newMenuItem->accelKeyCode = curMenuItem->accelKeyCode;
	if (curMenuItem->accelText != (String) NULL)
	  newMenuItem->accelText = XtNewString(curMenuItem->accelText);
	else
	  newMenuItem->accelText = NULL;
	newMenuItem->wmFunction = curMenuItem->wmFunction;

	if ((curMenuItem->wmFunction == F_Send_Msg)
	    || (curMenuItem->wmFunction == F_Circle_Up)
	    || (curMenuItem->wmFunction == F_Circle_Down)
#ifdef WSM
	    || (curMenuItem->wmFunction == F_Set_Context)
# ifdef PANELIST
	    /*
	     * NOTE: For now, in dtwm this function is used only
	     * to copy the FrontPanel menu.  So, we know that
	     * curMenuItem->wmFuncArgs isn't going anywhere,
	     * so it's safe to simply point at it.  If at some
	     * point it becomes possible that curMenuItem->wmFuncArgs
	     * can go away, we'll need to make a (deep) copy of
	     * the WmActionArg.  11/20/96
	     */
	    || (curMenuItem->wmFunction == F_Action)
# endif /* PANELIST */
#endif /* WSM */
	    )
	  newMenuItem->wmFuncArgs = curMenuItem->wmFuncArgs;
	else if (curMenuItem->wmFuncArgs != (String) NULL)
	  newMenuItem->wmFuncArgs = XtNewString(curMenuItem->wmFuncArgs);
	else
	  newMenuItem->wmFuncArgs = NULL;

	newMenuItem->greyedContext = curMenuItem->greyedContext;
	newMenuItem->mgtMask = curMenuItem->mgtMask;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	newMenuItem->clientCommandName = 
	  XtNewString(curMenuItem->clientCommandName);
	newMenuItem->clientCommandID = curMenuItem->clientCommandID;
#endif
	newMenuItem->nextMenuItem = (MenuItem *) NULL;
    }
    
    return(returnMenuItem);
}


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  DuplicateMenuExclusions (exclusions)
 *  
 *
 *
 *  Description:
 *  -----------
 *  This function creates an indentical duplicate of the given menu exclusions
 *  list.
 *
 *
 *  Inputs:
 *  ------
 *  exclusions = the linked list of menu exclusions to duplicate
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuExclusion list, identical to the original
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static MenuExclusion *
DuplicateMenuExclusions (MenuExclusion *exclusions)
{
    MenuExclusion *newMenuExclusion = (MenuExclusion *) NULL;
    MenuExclusion *returnMenuExclusion = (MenuExclusion *) NULL;
    MenuExclusion *curMenuExclusion = (MenuExclusion *) NULL;
    
    for (curMenuExclusion = exclusions;
	 curMenuExclusion != (MenuExclusion *) NULL;
	 curMenuExclusion = curMenuExclusion->nextExclusion)
    {
	/* If its the first one ... */
	if (newMenuExclusion == (MenuExclusion *) NULL)
	{
	    newMenuExclusion =
	      (MenuExclusion *)XtMalloc(sizeof(MenuExclusion));
	    returnMenuExclusion = newMenuExclusion;
	}
	else /* ... otherwise, get the next menuExclusion. */
	{
	    newMenuExclusion->nextExclusion =
	      (MenuExclusion *)XtMalloc(sizeof(MenuExclusion));
	    newMenuExclusion = newMenuExclusion->nextExclusion;
	}

	newMenuExclusion->command_string =
	  XtNewString(curMenuExclusion->command_string);
    }

    /* Make sure we properly NULL terminate the list. */
    if (newMenuExclusion != (MenuExclusion *) NULL)
      newMenuExclusion->nextExclusion = (MenuExclusion *) NULL;
    
    return(returnMenuExclusion);
}
#endif

/*************************************<->*************************************
 *
 *  DuplicateMenuSpec (menuSpec)
 *  
 *
 *
 *  Description:
 *  -----------
 *  This function creates an indentical duplicate of the given menuSpec.
 *  The menuItems list in the menuSpec is also duplicated. 
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = the menuSpec to duplicate
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuSpec structure with the same field
 *	     values as the original
 *
 *
 *  Comments:
 *  --------
 *  A new MenuSpec structure is allocated. Most of he fields of the new
 *  structure are set to the same values as the passed in menuSpec.
 *  There are some differences between the two final structures.
 *  One difference: any fields related to push buttons and other
 *  widgets are left blank in the new MenuSpec to be filled in later.
 * 
 *************************************<->***********************************/
MenuSpec *
DuplicateMenuSpec (MenuSpec *menuSpec)
{
    MenuSpec *newMenuSpec;
    
    if ((newMenuSpec = (MenuSpec *) XtMalloc (sizeof (MenuSpec))) == NULL)
      /* Handle insufficent memory */
    {
	Warning((char *)GETMESSAGE(48, 9,
		 "Insufficient memory for menu specification\n"));
	return (NULL);
    }
    newMenuSpec->name = XtNewString(menuSpec->name);
    newMenuSpec->currentContext = menuSpec->currentContext;
    newMenuSpec->menuWidget = (Widget) NULL;
    newMenuSpec->whichButton = menuSpec->whichButton;
    newMenuSpec->height = menuSpec->height;
    newMenuSpec->menuItems = DuplicateMenuItems(menuSpec->menuItems);
    newMenuSpec->menuButtons = (MenuButton *) NULL;
    newMenuSpec->menuButtonSize = 0;
    newMenuSpec->menuButtonCount = 0;
    newMenuSpec->accelContext = menuSpec->accelContext;
    newMenuSpec->accelKeySpecs = (KeySpec *) NULL;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    newMenuSpec->exclusions = DuplicateMenuExclusions(menuSpec->exclusions);
    newMenuSpec->clientLocal = TRUE;
    newMenuSpec->commandID = menuSpec->commandID;
#endif
    newMenuSpec->nextMenuSpec = (MenuSpec *) NULL;
    
    return(newMenuSpec);
}


#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  MakeMenuItem (label, wmFunction, funcArgs, mnemonic, accelText)
 * 
 *
 *  Description:
 *  -----------
 *  This function creates and returns a MenuItem structure. 
 *
 *
 *  Inputs:
 *  ------
 *  label      = the display name of the menu item
 *  wmFunction = the wm function to invoke
 *  funcArgs   = the function arguments to pass to the wm function
 *  mnemonic   = the mnemonic keysym
 *  accelText  = the accelerator text
 *
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuItem structure with fields filled
 *           in as per passed arguments
 *
 *
 *  Comments:
 *  --------
 *  This function is actually used as the underlying mechanism for 
 *  MenuItem creation by MakeMenuItemFromTemplate and 
 *  MakeClientCommandMenuItem.
 *
 *  Assumptions:
 *  -----------
 *  This function assumes that ParseWmAccelerator simply takes a pointer
 *  to a string and parses the accelerator found in it. If ParseWmAccelerator
 *  is ever modified to call GetString to get more text from the parse
 *  stream (as other parse functions do) then this code will break.
 * 
 *************************************<->***********************************/
static MenuItem *
MakeMenuItem (String label, WmFunction wmFunction, String funcArgs,
	      KeySym mnemonic, unsigned int accelState,
	      KeyCode accelKeyCode, String accelText)
{
    MenuItem *menuItem;
/*
    unsigned char *copy_of_accelText;
*/
    
    if ((menuItem = (MenuItem *) XtMalloc (sizeof (MenuItem))) == NULL)
      /* Handle insufficent memory */
    {
	MWarning(((char *)GETMESSAGE(48, 10,
		  "Insufficient memory for menu item %s\n")), label);
	return (NULL);
    }
    
    menuItem->labelType = XmSTRING;
    menuItem->label = XtNewString(label);
    menuItem->labelBitmapIndex = -1;
    menuItem->mnemonic = mnemonic;
    menuItem->clientCommandName = NULL;
    menuItem->clientCommandID = 0;

/*     
    copy_of_accelText = (unsigned char *)XtNewString(accelText);
    ParseWmAccelerator(&copy_of_accelText, menuItem);
*/

    menuItem->accelState = accelState;
    menuItem->accelKeyCode = accelKeyCode;
    menuItem->accelText = XtNewString(accelText);

    menuItem->wmFunction = wmFunction;
    menuItem->wmFuncArgs = XtNewString(funcArgs);
    SetGreyedContextAndMgtMask(menuItem, wmFunction);
    menuItem->nextMenuItem = (MenuItem *) NULL;
    
    return(menuItem);
}


/*************************************<->*************************************
 *
 *  MakeMenuItemFromTemplate (template, name, funcArgs)
 * 
 *
 *  Description:
 *  -----------
 *  This function creates and returns a MenuItem structure. 
 *
 *
 *  Inputs:
 *  ------
 *  template   = a template menuItem used to fill in fields of the
 *		 new menu item
 *  name       = the display name this item should have
 *  funcArgs   = the function arguments to pass to the wm function
 *
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuItem structure with fields filled
 *           in as per template MenuItem and funcargs
 *
 *
 *  Comments:
 *  --------
 *  This function uses the values in the template MenuItem to create
 *  a new copy of the template with the given funcArgs.
 * 
 *************************************<->***********************************/

static MenuItem *MakeMenuItemFromTemplate (MenuItem *template, String name,
					   String funcArgs)
{
    if (template->clientCommandName == (String) NULL)
      return(MakeMenuItem(name, template->wmFunction, funcArgs,
			  template->mnemonic, template->accelState,
			  template->accelKeyCode, template->accelText));

    return(MakeMenuItem(template->clientCommandName, template->wmFunction,
			funcArgs, template->mnemonic, template->accelState,
			template->accelKeyCode, template->accelText));
}


/*************************************<->*************************************
 *
 *  MakeClientCommandMenuItem (label, funcArgs)
 * 
 *
 *  Description:
 *  -----------
 *  This function creates and returns a MenuItem structure filled as
 *  appropriate for client command menu items using the given label
 *  and funcArgs. 
 *
 *
 *  Inputs:
 *  ------
 *  label      = the display label for this menu item
 *  funcArgs   = the function arguments to pass to the wm function
 *
 *  Outputs:
 *  -------
 *  Return = pointer to a new MenuItem structure with fields filled
 *           in as per arguments and client command specs
 *
 *
 *  Comments:
 *  --------
 *  This function will fill in a new MenuItem structure as appropriate for
 *  client commands. This function is used when you want to insert a client
 *  command into a menu without using a MenuItem template constructed from
 *  mwmrc.
 * 
 *************************************<->***********************************/

static MenuItem *MakeClientCommandMenuItem (String label, String funcArgs)
{
    return(MakeMenuItem(label, F_InvokeCommand, funcArgs,
			(KeySym) NULL, (unsigned int)0,
			(KeyCode) NULL, (String)NULL));
}


/*************************************<->*************************************
 *
 *  PerformClientCommandMatch (clientCommand, menuItem, bestMatchSoFar)
 * 
 *
 *  Description:
 *  -----------
 *  This function determines whether the menuItemCommand specification 
 *  matches the clientCommand.
 *
 *  Inputs:
 *  ------
 *  clientCommand  = the clientCommand that we want to find a specification for
 *  menuItem       = the menu item we will look in for a specification
 *  bestMatchSoFar = the menu item we will return if the given menu item is
 *		     not a match or not a better match
 *
 *  Outputs:
 *  -------
 *  Return = pointer to the given menuItem if it contains a client command
 *	     specification and if that specification matches the given
 *	     clientCommand *and* if the match is a better match than
 *	     the bestMatchSoFar. Otherwise, the bestMatchSoFar is returned,
 *	     which could possibly be NULL.
 *
 *  Comments:
 *  --------
 *  If the menuItem does match, it also determines whether it is
 *  a better match than the bestMatchSoFar. If so, the menuItemCommand is
 *  returned. Otherwise, bestMatchSoFar is returned.
 *
 *  Best matching is defined as follows:
 *	1. A specification with fewer wildcards is considered a better
 *	   match than one with more wildcards.
 *	2. Given two specifications with the same number of wildcards,
 *	   the specification with its wildcards more towards the right
 *	   than the left is considered a better match.
 * 
 *************************************<->***********************************/

/* @RGC: This is kind of arbitrary, but I can't imagine there being more
   than a small number of segments in any given command specification. */
#define MAXSEGMENTS	100

static MenuItem *PerformClientCommandMatch (String clientCommand,
				     MenuItem *menuItem,
				     MenuItem *bestMatchSoFar)
{
    String menuItemCommand, bestMatchStr;
    int seglength, i;
    int segments = 0, wildcards = 0, wildcardPositions[MAXSEGMENTS];
    int bestSegments = 0, bestWildcards = 0;
    int bestWildcardPositions[MAXSEGMENTS];
    Boolean foundWildcard = FALSE;

    if (menuItem == (MenuItem *) NULL)
      return(bestMatchSoFar);
    menuItemCommand = menuItem->label;
    
    /* Skip any modifier characters at the beginning of the
       menu items client command. */
    /* @RGC: This is kind of kludgy. We shouldn't have to know
       the specifics of command parsing here. */
    if (menuItemCommand[0] == '~')
      ++menuItemCommand;
    else if (menuItemCommand[0] == '=' && menuItemCommand[1] == '>')
      menuItemCommand += 2;
    else if (menuItemCommand[0] == '=')
      ++menuItemCommand;
    else if (menuItemCommand[0] == '-' && menuItemCommand[1] == '>')
      menuItemCommand += 2;
    
    /* If the menu item doesn't even contain a client command spec,
       then just return the existing best match. */
    if (*menuItemCommand != '<') return(bestMatchSoFar);
    
    /* Run down the clientCommand and the menuItemCommand together,
       matching along the way. If matching fails at any point, then
       return the bestMatchSoFar.  */
    for (segments = 0;
	 *menuItemCommand != '\0' && *clientCommand != '\0';
	 ++segments)
    {
	/* Skip past the '<' at the beginning of the next segment and
	   any whitespace. */
	++menuItemCommand; ++clientCommand;
	while (isspace(*menuItemCommand)) ++menuItemCommand;
	while (isspace(*clientCommand)) ++clientCommand;
	
	/* First check whether the current menuItemCommand segment is
	   a wildcard. */
	if (*menuItemCommand == '*')
	{
	    /* Since the menuItemCommand segment is a wildcard, skip
	       it and the current segment of the client command since
	       the wildcard has to match at least one segment in
	       the client command. */
	    wildcardPositions[wildcards++] = segments;
	    ++menuItemCommand;
	    while (isspace(*menuItemCommand)) ++menuItemCommand;
	    while (*clientCommand != '>' && *clientCommand != '\0')
	      ++clientCommand;
	    foundWildcard = TRUE;
	}
	else
	{
	    /* Calculate how long the current segment of the
	       menuItemCommand is */
	    for (seglength = 0;
		 menuItemCommand[seglength] != '>' &&
		 menuItemCommand[seglength] != '\0';
		 ++seglength)
	      /*EMPTY*/;
	    
	    /* If we are pointing at '\0', then this isn't a match */
	    if (menuItemCommand[seglength] == '\0') return(bestMatchSoFar);
	    
	    /* Get rid of trailing white space on the segment. */
	    for (; seglength > 0; --seglength)
	    {
		if (!isspace(menuItemCommand[seglength - 1]))
		  break;
	    }
	    
	    /* Now string compare this segment with the clientCommand
	       segment, up to the number of characters in the menu
	       item segment. */
	    if (strncmp(menuItemCommand, clientCommand, seglength) == 0)
	    {
		/* So far so good. Just make sure clientCommand doesn't
		   have anything but whitespace after its seglength
		   character. */
		clientCommand += seglength;
		while (isspace(*clientCommand)) ++clientCommand;
		if (*clientCommand != '>') return(bestMatchSoFar);
		
		/* We have a match. Clear the foundWildcard since we
		   have sync'ed up and keep trying to match. */
		foundWildcard = FALSE;
		menuItemCommand += seglength;
		while (isspace(*menuItemCommand)) ++menuItemCommand;
	    }
	    else if (foundWildcard == FALSE)
	    {
		/* We didn't match and there wasn't wildcard to 
		   swallow the discrepancy. Therefore, this is not
		   a match. */
		return(bestMatchSoFar);
	    }
	}
	
	/* We finished the current segments, we should be looking at
	   a close bracket and a following period or a close bracket and
	   a following NULL. Skip past the close brackets and optional
	   period. If we don't see those, then this isn't a match. */
	if (menuItemCommand[0] == '>' && menuItemCommand[1] == '\0' &&
	    clientCommand[0] == '>'   && clientCommand[1] == '\0')
	{
	    ++menuItemCommand; ++clientCommand;
	}
	else if (menuItemCommand[0] == '>' && menuItemCommand[1] == '.' &&
		 clientCommand[0] == '>'   && clientCommand[1] == '.')
	{	
	    menuItemCommand += 2;
	    clientCommand += 2;
	}
	else return(bestMatchSoFar);
    }

    /* If we terminated the loop because only one of the two commands being
       compared was empty, then we don't have a complete match. Return the
       best match so far. */
    if (*menuItemCommand != '\0' || *clientCommand != '\0')
      return(bestMatchSoFar);
    
    /* So the menuItemCommand must have matched. If the current best
       match is NULL, then just return the menuItem. Otherwise calculate some
       matching quality metrics for the bestMatchSoFar and compare them
       to the menuItemCommand metrics to decide which of the two to
       return. */
    if (bestMatchSoFar == (MenuItem *) NULL)
      return(menuItem);
    
    bestMatchStr = bestMatchSoFar->label;
    
    /* Skip any modifier characters at the beginning of the
       best match client command. */
    /* @RGC: This is kind of kludgy. We shouldn't have to know
       the specifics of command parsing here. */
    if (bestMatchStr[0] == '~')
      ++bestMatchStr;
    else if (bestMatchStr[0] == '=' && bestMatchStr[1] == '>')
      bestMatchStr += 2;
    else if (bestMatchStr[0] == '=')
      ++bestMatchStr;
    else if (bestMatchStr[0] == '-' && bestMatchStr[1] == '>')
      bestMatchStr += 2;
    
    /* If the best match  doesn't even contain a client command spec,
       then just return the new match as the best match. */
    if (*bestMatchStr != '<') return(menuItem);
    
    for (bestSegments = 0;
	 *bestMatchStr != '\0';
	 ++bestSegments)
    {
	/* Skip past the '<' at the beginning of the next segment and
	   any whitespace. */
	++bestMatchStr;
	while (isspace(*bestMatchStr)) ++bestMatchStr;
	
	/* First check whether the current bestMatchStr segment is
	   a wildcard. @RGC: We are assuming that there is nothing
	   but possible whitespace after the *. */
	if (*bestMatchStr == '*')
	  bestWildcardPositions[bestWildcards++] = bestSegments;
	while (*bestMatchStr != '>' && *bestMatchStr != '\0')
	  ++bestMatchStr;
	
	/* Check for the closing > and .  or close > and NULL. If they
	   do not both appear then the bestMatch is bad and we should
	   return the menuItem. */
	if (bestMatchStr[0] == '>' && bestMatchStr[1] == '\0')
	  ++bestMatchStr;
	else if (bestMatchStr[0] == '>' && bestMatchStr[1] == '.')
	  bestMatchStr += 2;
	else return(menuItem);
    }
    
    /* Now compare the best match metrics with the menu item metrics to
       determine who should be returned. */
    if (bestWildcards != wildcards)
    {
	/* Return the menuItem with the fewest wildcards. */
	return(bestWildcards < wildcards ? bestMatchSoFar : menuItem);
    }
    else
    {
	/* Find which menu item has the earliest wild card and return
	   the other. */
	for (i = 0; i < wildcards; ++i)
	  if (wildcardPositions[i] != bestWildcardPositions[i])
	  {
	      return(bestWildcardPositions[i] < wildcardPositions[i] ?
		     bestMatchSoFar : menuItem);
	  }
	
	/* If we got this far, then the two specifications are too
	   close to call. Return bestMatchSoFar. */
	return(bestMatchSoFar);
    }
}


/*************************************<->*************************************
 *
 *  ExcludeClientCommand (menuSpec, clientCommand)
 * 
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  menuSpec      = the menuSpec whose menuItems we want to search through
 *  clientCommand = the clientCommand that we want to find an exclusion for
 *
 *  Outputs:
 *  -------
 *  Return = TRUE if the command must be excluded from the menuSpec.
 *	     FALSE if there is no exclusion preventing the insertion.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static Boolean ExcludeClientCommand (MenuSpec *menuSpec, String clientCommand)
{
    MenuItem placeholder;
    MenuExclusion *curExclusion;

    /* Search for an exclusion that would cause this command to be
       excluded, if any such exclusion exists. */
    for (curExclusion = menuSpec->exclusions;
	 curExclusion != (MenuExclusion *) NULL;
	 curExclusion = curExclusion->nextExclusion)
    {
	/* We don't have menu items for exclusions so just use a bogus
	   placeholder menu item with the label field set to the string
	   found in the exclusion. */
	placeholder.label = curExclusion->command_string;
	
	/* If we don't get NULL back, then this exclusion matches. */
	if (PerformClientCommandMatch(clientCommand,
				      &placeholder, NULL) != (MenuItem *) NULL)
	{
	    return(TRUE);
	}
    }
    return(FALSE);
}


/*************************************<->*************************************
 *
 *  ForceInLineToCascade (menuSpec, clientCommand, bestMatch)
 * 
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  menuSpec      = the menuSpec whose menuItems we want to search through
 *  clientCommand = the clientCommand that we want to find an force for
 *  bestMatch     = the best matching menu item that was found
 *
 *  Outputs:
 *  -------
 *  Return = TRUE if the command set must be cascaded
 *	     FALSE if there is no forced cascade 
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static Boolean ForceInLineToCascade (MenuSpec *menuSpec,
				     String clientCommand,
				     MenuItem **bestMatch)
{
    /* First find the best match in the menu spec. */
    FindClientCommandMatch(menuSpec, clientCommand, bestMatch);

    /* If the best match is not NULL, then check whether it forces
       the client command to cascade. */
    if (*bestMatch != (MenuItem *) NULL)
    {
	/* If there is a force cascade modifier, then return TRUE. */
	if ((strncmp((*bestMatch)->label, "->", 2) == 0) ||
	    (strncmp((*bestMatch)->label, "=>", 2) == 0))
	  return(TRUE);
    }

    /* If the best match is NULL, then return FALSE. We have been
       given no indication that the inLine command should be forced
       to cascade. */
    return(FALSE);
}


/*************************************<->*************************************
 *
 *  FindClientCommandMatch (menuSpec, clientCommand, menuItem)
 * 
 *
 *  Description:
 *  -----------
 *  This function searches through the list of menuItems in the given
 *  menuSpec, searching for ones which have a client command specification
 *  and, for each one that does, whether that specification matches the
 *  given client. The best matching menuItem out of all that matched
 *  is returned.
 *
 *  Inputs:
 *  ------
 *  menuSpec      = the menuSpec whose menuItems we want to search through
 *  clientCommand = the clientCommand that we want to find a specification for
 *  menuItem      = the best matching menu item
 *
 *  Outputs:
 *  -------
 *  Return = TRUE if the command may be inserted into the menuSpec.
 *	     FALSE if there is an exclusion preventing the insertion.
 *	     Also return the best matching menu item in the menuItem
 *	     buffer argument. NULL is returned if no matching MenuItem
 *	     can be found.
 *
 *  Comments:
 *  --------
 *  Best matching is defined as follows:
 *	1. A specification with fewer wildcards is considered a better
 *	   match than one with more wildcards.
 *	2. Given two specifications with the same number of wildcards,
 *	   the specification with its wildcards more towards the right
 *	   than the left is considered a better match.
 * 
 *************************************<->***********************************/

static Boolean FindClientCommandMatch (MenuSpec *menuSpec,
				       String clientCommand,
				       MenuItem **menuItem)
{
    MenuItem *bestMatch = (MenuItem *) NULL, *curMenuItem, placeholder;
    MenuItem *bestExclusionItem = (MenuItem *) NULL;
    MenuExclusion *curExclusion;
    String    bestExclusionStr = (String) NULL;
    
    /* First search for a match in the menu items of the menu spec. */
    for (curMenuItem = menuSpec->menuItems;
	 curMenuItem != (MenuItem *) NULL;
	 curMenuItem = curMenuItem->nextMenuItem)
    {
	bestMatch =
	  PerformClientCommandMatch(clientCommand, curMenuItem, bestMatch);
    }

    /* Now search for the best exclusion that would cause this match to be
       excluded, if any such exclusion exists. */
    for (curExclusion = menuSpec->exclusions;
	 curExclusion != (MenuExclusion *) NULL;
	 curExclusion = curExclusion->nextExclusion)
    {
	/* We don't have menu items for exclusions so just use a bogus
	   placeholder menu item with the label field set to the string
	   found in the exclusion. */
	placeholder.label = curExclusion->command_string;

	/* Find the best exclusion string in the bunch. */
	bestExclusionItem =
	  PerformClientCommandMatch(clientCommand, &placeholder,
				    bestExclusionItem);

	/* Save the best exclusion string since we are going to reuse
	   the placeholder menu item. */
	if (bestExclusionItem != (MenuItem *) NULL)
	  bestExclusionStr = bestExclusionItem->label;
    }

    /* Okay, now if we found an exclusion, we need to determine if the
       exclusion was a better match than the best match that we found.
       If so, the item is *really* exclude. Otherwise, we return the
       best match and let the item be included. */
    placeholder.label = bestExclusionStr;
    if (bestExclusionStr == (String) NULL ||
	PerformClientCommandMatch(clientCommand, bestMatch, &placeholder) ==
	bestMatch)
    {
	*menuItem = bestMatch;
	return(TRUE);
    }
    else
    {
	*menuItem = NULL;
	return(FALSE);
    }
}



/*************************************<->*************************************
 *
 *  PerformInsertionsOnMatchList (matchlist)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void PerformInsertionsOnMatchList (MatchList **matchlist)
{
    MatchList *curmatch;
    MenuItem *newMenuItem, *curitem;
    
    if (*matchlist == (MatchList *) NULL)
      return;

    if ((*matchlist)->menuspec == (MenuSpec *) NULL)
      {
	/* should never get here, but if we do, then we can't
	   continue in this routine since mwm will dump.  This
	   may be caused by the cci code duplicating a global
	   menu for a client when it shouldn't be duplicated.
	   If we skip this routine, the cci command will not
	   be added which is far less disturbing than a dump. */
	return;
      }
       
    
    for (curmatch = *matchlist;
	 curmatch != (MatchList *) NULL;
	 curmatch = curmatch->next)
    {
      if (curmatch->menuitem != (MenuItem *) NULL)
	{
	  /* Find this menu item within the menuspec. */
	  for (curitem = curmatch->menuspec->menuItems;
	       curitem != curmatch->menuitem &&
	       curitem != (MenuItem *) NULL;
	       curitem = curitem->nextMenuItem)
	    /*EMPTY*/;
	  
	  /* If we didn't find the menuitem in the menuspec, then
	     don't do this match. */
	  if (curitem == (MenuItem *) NULL) continue;
	  
	  newMenuItem =
	    MakeMenuItemFromTemplate(curmatch->menuitem,
				     curmatch->treenode->defaultName,
				     curmatch->funcargs);
	  newMenuItem->wmFunction = curmatch->function;
	  newMenuItem->greyedContext = curmatch->greyed_context;
	  newMenuItem->nextMenuItem = curitem->nextMenuItem;
	  newMenuItem->clientCommandID = curmatch->treenode->commandID;
	  curitem->nextMenuItem = newMenuItem;
	}
      else
	{
	  MenuItem *last = (MenuItem *) NULL;
	  
	  if (curmatch->menuspec != NULL)
	    {
	      /* Find the last menu item in the menuspec */
	      for (last = curmatch->menuspec->menuItems;
		   last != (MenuItem *) NULL &&
		   last->nextMenuItem != (MenuItem *) NULL;
		   last = last->nextMenuItem)
		{
		  /* If the next item is f.quit and it is the last
		     item, then stop searching now. We don't want
		     to insert after a trailing f.kill (i.e. Close). */
		  if ((last->nextMenuItem->wmFunction == F_Kill) &&
		      (last->nextMenuItem->nextMenuItem == (MenuItem *) NULL))
		    break;
		}
	    }
	  
	  /* Create a new client command menu item */
	  newMenuItem =
	    MakeClientCommandMenuItem
	      (XtNewString(curmatch->treenode->defaultName),
	       XtNewString(curmatch->funcargs));
	  newMenuItem->wmFunction = curmatch->function;
	  newMenuItem->greyedContext = curmatch->greyed_context;
	  newMenuItem->clientCommandID = curmatch->treenode->commandID;
	  
	  /* Insert the new menu item at the end of the list */
	  if (last == (MenuItem *) NULL)
	    {
	      newMenuItem->nextMenuItem = (MenuItem *) NULL;
	      if (curmatch->menuspec != NULL)
		curmatch->menuspec->menuItems = newMenuItem;
	      else
		{
		  /* again, should never get here... */
		  return;
		}
	    }
	  else
	    {
	      newMenuItem->nextMenuItem = last->nextMenuItem;
	      last->nextMenuItem = newMenuItem;
	    }
	}
    }
}


/*************************************<->*************************************
 *
 *  void
 *  DestroyMenuSpecWidgets (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = pointer to MenuSpec structure
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Destroys all the menuspec widgets so that we can rebuild the menu from
 *  scratch.
 * 
 *************************************<->***********************************/

void DestroyMenuSpecWidgets (MenuSpec *menuSpec)
{
    /* check for bad input value - shouldn't happen. */
    if (menuSpec == (MenuSpec *) NULL) return;

    /* Destroy the menu widget */
    if (menuSpec->menuWidget != (Widget) NULL)
    {
      XtDestroyWidget(XtParent(menuSpec->menuWidget));
      menuSpec->menuWidget = (Widget) NULL;
    }

    /* Destroy the menu buttons array */
    if (menuSpec->menuButtonSize != 0)
    {  
      XtFree((char *)menuSpec->menuButtons);
      menuSpec->menuButtons = (MenuButton *) NULL;
    }

    /* Reset the counters */
    menuSpec->menuButtonSize = 0;
    menuSpec->menuButtonCount = 0;

    /* Clear the flag that says we have processed this menu spec for
       widget creation. (We probably don't need to do this after all
       since CreateMenuWidgets clears it when done.) */
    if (menuSpec->currentContext & CR_MENU_MARK)
      menuSpec->currentContext &= ~(CR_MENU_MARK);

    return;
}


/*************************************<->*************************************
 *
 *  void
 *  DestroyMenuSpec (pSD, commandID)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pSD        = screen data pointer of screen with command to remove
 *  commandID  = command id of the menuspec to be removed.
 *               if no match is found, then no removal is done.
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Destroy the specified menuSpec from the list of menuspecs on the
 *  specified screen. Note, there may be more than one copy of the
 *  spec floating around since duplications may have been done for
 *  some clients.
 * 
 *************************************<->***********************************/

void DestroyMenuSpec (WmScreenData *pSD, CARD32 commandID)
{
    MenuSpec *msToKill = NULL, *pMS;
    ClientListEntry *curClient;

    /* Scan through global menu specs. */
    if (pSD != NULL  &&  pSD->menuSpecs != NULL  &&  commandID != 0)
      {
	/* Scan through the list of menuSpecs and pull the mathing one
	 * out of the list.
	 */
	if (commandID == pSD->menuSpecs->commandID)
	  {
	    /* match at head of menuSpec list. */
	    msToKill = pSD->menuSpecs;
	    pSD->menuSpecs = pSD->menuSpecs->nextMenuSpec;
	    msToKill->nextMenuSpec = NULL;
	  }
	else
	  {
	    for (pMS = pSD->menuSpecs;
		 (pMS->nextMenuSpec != NULL &&
		  pMS->nextMenuSpec->commandID != commandID);
		 pMS = pMS->nextMenuSpec)
	      ;

	    if (pMS->nextMenuSpec != NULL)
	      {
		msToKill = pMS->nextMenuSpec;
		pMS->nextMenuSpec = msToKill->nextMenuSpec;
		msToKill->nextMenuSpec = NULL;
	      }
	  }

	/* found it - now remove the menuSpec. */
	if (msToKill != NULL)
	  FreeCustomMenuSpec(msToKill);
      }


    /* Check each client's menu spec list.  Stop searching if global. */
    for (curClient = pSD->clientList;
	 curClient != (ClientListEntry *)NULL;
	 curClient = curClient->nextSibling)
      {
	/*
	 * Check the first position.
	 * If matched, then we're done with this client.
	 */
	if (commandID == pSD->menuSpecs->commandID)
	  {
	    msToKill = curClient->pCD->systemMenuSpec;
	    curClient->pCD->systemMenuSpec = msToKill->nextMenuSpec;
	    msToKill->nextMenuSpec = NULL;
	  }

	/* Check the rest of the list. */
	else
	  {
	    for (pMS = curClient->pCD->systemMenuSpec;
		 (pMS->nextMenuSpec != (MenuSpec *)NULL)     &&
		 (pMS->nextMenuSpec->commandID != commandID) &&
		 pMS->clientLocal;
		 pMS = pMS->nextMenuSpec)
	      ;

	    if ((pMS->nextMenuSpec != (MenuSpec *)NULL)      &&
		(pMS->nextMenuSpec->commandID != commandID))
	      {
		msToKill = pMS->nextMenuSpec;
		pMS->nextMenuSpec = msToKill->nextMenuSpec;
		msToKill->nextMenuSpec = NULL;
	      }
	    else
	      msToKill = NULL;
	  }

	if (msToKill != NULL)
	  FreeCustomMenuSpec(msToKill);
      }

    return;
}


/*************************************<->*************************************
 *
 *  ReplaceMenuSpecForClient (menuspec, pCD)
 *
 *
 *  Description:
 *  -----------
 *  Duplicates the given menuspec and replaces the given menuspec if
 *  found in the clients menuspec list with the duplicate.
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = the duplicate menuspec
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static MenuSpec *ReplaceMenuSpecForClient (MenuSpec *menuSpec, ClientData *pCD)
{
    MenuSpec *newMenuSpec, *curMenuSpec;

    /* Duplicate the menu spec */
    newMenuSpec = DuplicateMenuSpec(menuSpec);
    
    /* Try to find this menuspec in the list of client
       menuspecs. If we find it then we want to replace it with
       the new one. */
    if (pCD->systemMenuSpec == menuSpec)
    {
	/* It was the head of the list. We need to handle that
	   a little special */
	newMenuSpec->nextMenuSpec = pCD->systemMenuSpec->nextMenuSpec;
	pCD->systemMenuSpec = newMenuSpec;
    }
    else
    {
	/* Search through the list until we find the menuspec or
	   the end of the list. */
	for (curMenuSpec = pCD->systemMenuSpec;
	     curMenuSpec->nextMenuSpec != (MenuSpec *) NULL;
	     curMenuSpec = curMenuSpec->nextMenuSpec)
	{
	    if (curMenuSpec->nextMenuSpec == menuSpec)
	    {
		newMenuSpec->nextMenuSpec =
		  curMenuSpec->nextMenuSpec->nextMenuSpec;
		curMenuSpec->nextMenuSpec = newMenuSpec;
		/* We found it and replaced it. Now get out of
		   the loop. */
		break;
	    }
	}
	if (curMenuSpec->nextMenuSpec == (MenuSpec *) NULL)
	{
	    /* We didn't find it. Just stick it at the end? We
	       should have found it. I'm not sure how to handle
	       this. */
	    curMenuSpec->nextMenuSpec = newMenuSpec;
	    newMenuSpec = (MenuSpec *) NULL;
	}
    }

    return(newMenuSpec);
}


/*************************************<->*************************************
 *
 *  FindLastMenuSpecToModify (menuspec, command_id)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = the last menu spec that would be affected by modifications
 *	     to the given command id
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static MenuSpec * FindLastMenuSpecToModify(MenuSpec *menuSpec,
					   CARD32 command_id)
{
    MenuSpec *curMenuSpec, *lastToModify = (MenuSpec *) NULL;
    MenuItem *curItem;

    /* Search through all the menu specs in the list starting with
       the passed in menuSpec */
    for (curMenuSpec = menuSpec;
	 curMenuSpec != (MenuSpec *) NULL;
	 curMenuSpec = curMenuSpec->nextMenuSpec)
    {
	/* Try to find a menu item in this menu spec with the
	   command_id that will require modification */
	for (curItem = curMenuSpec->menuItems;
	     curItem != (MenuItem *) NULL;
	     curItem = curItem->nextMenuItem)
	{
	    if (curItem->clientCommandID == command_id)
	      break;
	}

	/* If we found a menu item that needs changing, then this
	   menu spec will need changing. Set the lastToModify to
	   point to this menu spec. If we find no other menu spec
	   that needs changing, then this will be the last one
	   in the list that needs changing. */
	if (curItem != (MenuItem *) NULL)
	  lastToModify = curMenuSpec;
    }

    /* We've looked through all the menu specs starting with menuSpec
       and we've looked at all the menu items in all of those menu
       specs. The lastToModify variable should be set to the last
       menu spec we saw that needed modification. Return it. */
    return(lastToModify);
}


/*************************************<->*************************************
 *
 *  RecreateMenuWidgets (matchlist)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void RecreateMenuWidgets (WmScreenData *pSD, ClientData *pCD,
				 MatchList **matchlist)
{
    MatchList *current;
    int count = 0, i;
    MenuSpec **to_change;

    /* First count how many menu specs we need to recreate widgets for */
    for (current = *matchlist;
	 current != (MatchList *) NULL;
	 current = current->next)
      ++count;

    /* If there are no affected menuspecs, then just return. */
    if (count == 0) return;

    /* Allocate an array of menuspec pointers that is the size of the
       number of menu specs we need to recreate widgets for */
    to_change = (MenuSpec **)XtMalloc(sizeof(MenuSpec *) * count);
    for (i = 0; i < count; ++i)
      to_change[i] = (MenuSpec *) NULL;

    /* Now run through all the matchlist items, storing menuspecs in
       that array. If the menuspec is already there, then don't store
       it again. */
    for (current = *matchlist;
	 current != (MatchList *) NULL;
	 current = current->next)
    {
      for (i = 0; i < count; ++i)
      {
	if (to_change[i] == current->menuspec) break;
	else if (to_change[i] == (MenuSpec *) NULL)
	{
	  to_change[i] = current->menuspec;
	  break;
	}
      }
    }

    /* Run through the array, destroy all existing widgets for each
       menuspec */
    for (i = 0; i < count && to_change[i] != (MenuSpec *) NULL ; ++i)
    {
	DestroyMenuSpecWidgets(to_change[i]);
    }

    /* Run through the array again creating widgets for all the menuspecs */
    for (i = 0; i < count && to_change[i] != (MenuSpec *) NULL; ++i)
    {
	to_change[i]->menuWidget =
	  CreateMenuWidget (pSD, pCD, to_change[i]->name, pSD->screenTopLevelW,
			    TRUE, to_change[i], NULL);
    }

    /* Free the array. We're done. */
    XtFree((char *) to_change);
}


/*************************************<->*************************************
 *
 *  FreeMatchList (matchlist)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void FreeMatchList (MatchList **matchlist)
{
    MatchList *current, *next;

    current = *matchlist;
	
    while (current != (MatchList *) NULL)
    {
	next = current->next;
	XtFree(current->command_string);
	XtFree(current->funcargs);
	XtFree((char *)current);
	current = next;
    }

    *matchlist = (MatchList *) NULL;
}


/*************************************<->*************************************
 *
 *  StoreMatchedCommand (matchlist, menuSpec, menuItem, command_string,
 *			 treenode, function, funcargs)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 *  If the existing match has NULL for the menuitem, then get rid of
 *  it and replace with proposed match.
 * 
 *************************************<->***********************************/

static void StoreMatchedCommand (MatchList **matchlist, MenuSpec *menuSpec,
				 MenuItem *menuItem, String command_string,
				 CmdTree *treenode, WmFunction function,
				 String funcargs, Context greyed_context)
{
    MatchList *current, *new;
    
    /* If this entry does not already exist in the match list, then insert
       it. This implies that we first have to perform a search of the list.
       The search is very easy. We can simply compare the tuple of
       <menuSpec,command_string> with each entry in the matchlist
       to see if we already have that tuple stored. We can do straight
       pointer value matching for the menuSpec and strcmp for the
       command_string */
    for (current = *matchlist;
	 current != (MatchList *) NULL;
	 current = current->next)
    {
	if (current->menuspec == menuSpec &&
	    strcmp(current->command_string, command_string) == 0)
	{
	    /* If the currently stored menu item is NULL,
	       then replace with the new menuitem and return. */
	    if (current->menuitem == (MenuItem *) NULL)
	    {
		current->menuitem = menuItem;
		return;
	    }
	    /* Otherwise, we have alreay inserted this
	       command into this menuspec so don't allow
	       another insertion. */
	    else return;
	}
    }
    
    /* Well, we didn't find a match, so store the entry */
    new = (MatchList *)XtMalloc(sizeof(MatchList));
    new->menuspec = menuSpec;
    new->menuitem = menuItem;
    new->command_string = XtNewString(command_string);
    new->treenode = treenode;
    new->function = function;
    new->funcargs = XtNewString(funcargs);
    new->greyed_context = greyed_context;
    new->next = (MatchList *) NULL;
    
    /* Stick it at the head of the list. It's easier. */
    new->next = *matchlist;
    *matchlist = new;
}


/*************************************<->*************************************
 *
 *  SearchForOtherMatches (pSD, pCD, treenode,
 *			   client_match_list, global_match_list,
 *			   menuSpec, command_string, 
 *			   function, funcargs, duplicate_globals, selection,
 *			   greyed_context)
 *
 *
 *  Description:
 *  -----------
 *  menuSpec = menu spec to exclude from search
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

static void SearchForOtherMatches (WmScreenData *pSD, ClientData *pCD,
				   CmdTree *treenode,
				   MatchList **client_match_list,
				   MatchList **global_match_list,
				   MenuSpec *menuSpec, String command_string,
				   WmFunction function, String funcargs,
				   Boolean duplicate_globals, Atom selection,
				   Context greyed_context, Boolean inLine)
{
    MenuSpec *current, *newMenuSpec;
    MenuItem *match;

    /* Search through all of the clients menuspecs first */
    for (current = (pCD == NULL ? NULL : pCD->systemMenuSpec);
	 current != (MenuSpec *) NULL;
	 current = current->nextMenuSpec)
    {
	/* If the current menu spec is a global, then just quit
	   this loop. Any menu specs from this point on will
	   have a next pointer that is still in the global list. */
	if (menuSpec->clientLocal != TRUE)  break;
        FindClientCommandMatch(current, command_string, &match);
	if (match != (MenuItem *) NULL)
	{
	    if (treenode->subTrees != (CmdTree *) NULL && inLine &&
		(strncmp(match->label, "->", 2) == 0 ||
		 strncmp(match->label, "=>", 2) == 0))
	    {
		CmdTree *tree;
		for (tree = treenode->subTrees;
		     tree != (CmdTree *) NULL;
		     tree = tree->next)
		{
		    char new_command_str[1024];
		    char new_funcargs[1024];
		    WmFunction inLine_function;

		    if (command_string == NULL)
		      sprintf(new_command_str, "<%s>", tree->name);
		    else
		      sprintf(new_command_str, "%s.<%s>", command_string,
			      tree->name);
		    if (tree->subTrees != (CmdTree *) NULL)
		    {
			/* menu to cascade to */
			sprintf(new_funcargs, "<%s>", tree->name);
			inLine_function = F_Menu;
		    }
		    else
		    {
			sprintf(new_funcargs, "%d %ld %ld", tree->commandID,
				pCD->client, selection);
			inLine_function = F_InvokeCommand;
		    }
		    StoreMatchedCommand(client_match_list, current, match,
					new_command_str, tree,
					inLine_function, new_funcargs,
					greyed_context);
		}
	    }
	    else
	    {
		StoreMatchedCommand(client_match_list, current, match,
				    command_string, treenode, function,
				    funcargs, greyed_context);
	    }
	}
    }

    /* Search through all of the global menuspecs also. */
    for (current = pSD->menuSpecs;
	 current != (MenuSpec *) NULL;
	 current = current->nextMenuSpec)
    {
        FindClientCommandMatch(current, command_string, &match);
	if (match != (MenuItem *) NULL)
	{
	    if (duplicate_globals == TRUE)
	    {
		/* Create a duplicate of the current menuspec and
		   store that away in the client instead of the current
		   menuspec. */
		newMenuSpec = ReplaceMenuSpecForClient(current, pCD);

		/* Now store enough information so that we can actually
		   create the insertion later. */
		StoreMatchedCommand(client_match_list, newMenuSpec, NULL,
				    command_string, treenode, function,
				    funcargs, greyed_context);
	    }
	    else /* Change global menu */
	    {
		StoreMatchedCommand(global_match_list, current, match,
				    command_string, treenode, function,
				    funcargs, greyed_context);
	    }
	}
    }
}


/*************************************<->*************************************
 *
 *  InsertTreeOnClient (pSD, pCD, tree, client_match_list, global_match_list,
 *			menuSpec, templateMenuItem, command_so_far,
 *			duplicate_globals, selection, greyed_context, inLine)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 *  If duplicate_globals is TRUE, then pCD cannot be NULL.
 * 
 *************************************<->***********************************/

static void InsertTreeOnClient (WmScreenData *pSD, ClientData *pCD,
				CmdTree *tree,
				MatchList **client_match_list,
				MatchList **global_match_list,
				MenuSpec *menuSpec, MenuItem *template,
				String command_so_far,
				Boolean duplicate_globals, Atom selection,
				Context greyed_context, Boolean inLine)
{
    String new_command_str;
    int length;
    char funcarg_buf[256];
    MenuSpec *newMenuSpec, *last, *dupMenuSpec;
    CmdTree *subtree;
    MenuItem *bestMatch = (MenuItem *) NULL;

    /* If the menuSpec we were given is NULL, then just return. We need to
       at least have a starting menuSpec. */
    if (menuSpec == (MenuSpec *) NULL)
      return;

    /* If we want global menus duplicated for a client, then the pCD
       had better not be NULL. */
    if (duplicate_globals && pCD == (ClientData *) NULL)
      return;

    while (tree != (CmdTree *) NULL)
    {
	/* The "4" below is to allow for brackets to surround the
	   tree->name, the period to separate it from the command
	   so far and a NULL. */
	length = (command_so_far != NULL ? strlen(command_so_far) : 0) + 
	         (tree->name != NULL ? strlen(tree->name) : 0) + 4;
	new_command_str = XtMalloc(sizeof(unsigned char) * length);
	if (command_so_far != (String) NULL)
	  sprintf(new_command_str, "%s.<%s>", command_so_far, tree->name);
	else
	  sprintf(new_command_str, "<%s>", tree->name);

	/* If there is an exclusion preventing this command from being 
	   inserted, then just continue the loop. @RGC: This is wrong.
	   We still want to search for other matches if there is an
	   exclusion. We just don't want to allow one of those other
	   matches to be this menuSpec. */
	if (ExcludeClientCommand(menuSpec, new_command_str))
	{
	    tree = tree->next;
	    XtFree(new_command_str);
	    continue;
	}

	/* If tree is a command set and the inLine flag is TRUE then
	 * we need to insert the command sets commands in the current
	 * menu spec instead of creating a cascade.
	 */
	if (tree->subTrees != (CmdTree *) NULL && inLine == TRUE &&
	    ForceInLineToCascade(menuSpec, new_command_str,
				 &bestMatch) == FALSE)
	{
	    /* Recursively process subtrees */
	    for (subtree = tree->subTrees;
		 subtree != (CmdTree *) NULL;
		 subtree = subtree->next)
	    {
		/* Pass along the bestMatch. If it is a valid menu item
		   then we want the insertion to occur at that menuitem
		   instead of at the end of the menu. */
		InsertTreeOnClient(pSD, pCD, subtree, client_match_list,
				   global_match_list, menuSpec, bestMatch,
				   new_command_str, duplicate_globals,
				   selection, greyed_context, inLine);
	    }
	    /* We don't want to search for other matches because we
	       want the items to be inserted inLine. Any other matches
	       will be found in the recursive calls. (??? or am I wrong?) */
	}
	/* If tree is a command set then we need to create a new
	   menuSpec. */
	else if (tree->subTrees != (CmdTree *) NULL)
	{
	    /* Create the name of the menu for the f.menu command. */
	    sprintf(funcarg_buf, "<%s>", tree->name);

	    /* Store the cascade button information so it can be
	       created later. */
	    StoreMatchedCommand(
	       (menuSpec->clientLocal ? client_match_list : global_match_list),
	       menuSpec, template, new_command_str, tree, F_Menu, funcarg_buf,
	       greyed_context);

	    /* We need to create a menu spec for the menu that this cascade
	       button will cascade to. Try to find one in the clients menu
	       spec list, stopping the first time we hit a global menu. If we
	       can't find one there and if we are *not* supposed to duplicate
	       globals, then try to find it in the global list. In all other
	       cases, create a new one using the funcarg_buf that we created
	       above as the name of the menuspec. */
	    for (newMenuSpec = (pCD == NULL ? NULL : pCD->systemMenuSpec);
		 newMenuSpec != (MenuSpec *) NULL;
		 newMenuSpec = newMenuSpec->nextMenuSpec)
	    {
		if (newMenuSpec->clientLocal == FALSE)
		{
		    newMenuSpec = (MenuSpec *) NULL;
		    break;
		}
		if (strcmp(newMenuSpec->name, funcarg_buf) == 0)
		  break;
	    }

	    /* If we didn't find it in the client list, maybe we should
	       look in the global list. */
	    if (newMenuSpec == (MenuSpec *) NULL && duplicate_globals == FALSE)
	    {
		for (newMenuSpec = pSD->menuSpecs;
		     newMenuSpec != (MenuSpec *) NULL;
		     newMenuSpec = newMenuSpec->nextMenuSpec)
		{
		    if (strcmp(newMenuSpec->name, funcarg_buf) == 0)
		      break;
		}
	    }

	    /* If we still don't have a menu spec, then create a new one. */
	    if (newMenuSpec == (MenuSpec *) NULL)
	    {
		newMenuSpec = MakeMenuSpec(funcarg_buf,
					   tree == NULL ? (CARD32)NULL
					                : tree->commandID);
		if (duplicate_globals) newMenuSpec->clientLocal = TRUE;
		else 		       newMenuSpec->clientLocal = FALSE;

		/* If we are duplicating globals, then add the new menu spec
		   to the client list. Otherwise add it to the global list. */
		if (duplicate_globals)
		  last = pCD->systemMenuSpec;
		else
		  last = pSD->menuSpecs;

		/* Find the last menu spec in the list. */
		while (last != (MenuSpec *) NULL &&
		       last->nextMenuSpec != (MenuSpec *) NULL)
		  last = last->nextMenuSpec;

		/* Put the new menu spec at the end of the list. */
		if (last == (MenuSpec *) NULL)
		{
		    if (duplicate_globals)
		      pCD->systemMenuSpec = newMenuSpec;
		    else
		      pSD->menuSpecs = newMenuSpec;
		}
		else last->nextMenuSpec = newMenuSpec;
	    }

	    /* Recursively process subtrees */
	    for (subtree = tree->subTrees;
		 subtree != (CmdTree *) NULL;
		 subtree = subtree->next)
	    {
		InsertTreeOnClient(pSD, pCD, subtree, client_match_list,
				   global_match_list, newMenuSpec, NULL,
				   new_command_str, duplicate_globals,
				   selection, greyed_context, inLine);
	    }

	    /* Search for any other matches in the existing menu specs
	       for this command, excluding newMenuSpec. */
	    SearchForOtherMatches(pSD, pCD, tree,
				  client_match_list, global_match_list,
				  newMenuSpec, new_command_str,
				  F_Menu, funcarg_buf,
				  duplicate_globals, selection,
				  greyed_context, inLine);
	    
	}
	else /* the tree is a simple command */
	{
	    /* Store away the push button information so it can be 
	       created later. */
	    sprintf(funcarg_buf, "%d %ld %ld", tree->commandID, 
		    (pCD == NULL ? None : pCD->client), selection);

	    /* If the menuSpec is global and we are supposed to be
	       duplicating global menu specs, then create a duplicate
	       and replace the menuspec with the duplicate for this
	       client. */
	    if (duplicate_globals)
	      dupMenuSpec = ReplaceMenuSpecForClient(menuSpec, pCD);
	    else
	      dupMenuSpec = menuSpec;

	    /* Now store the match away in the appropriate match list */
	    StoreMatchedCommand((dupMenuSpec->clientLocal ?
				 client_match_list : global_match_list),
				dupMenuSpec, template, new_command_str, tree,
				F_InvokeCommand, funcarg_buf, greyed_context);

	    /* Search for any other matches in the existing menu specs
	       for this command, excluding newMenuSpec. */
	    
	    SearchForOtherMatches(pSD, pCD, tree,
				  client_match_list, global_match_list,
				  dupMenuSpec, new_command_str,
				  F_InvokeCommand, funcarg_buf,
				  FALSE, /* Don't duplicate globals not associated
					    with this pCD. CR 9623 */
				  selection,
				  greyed_context, inLine);
	}

	/* Move on to the next tree item at this level */
	tree = tree->next;
	XtFree(new_command_str);
    }
}


/*************************************<->*************************************
 *
 *  InsertTreeOnAllClients (pSD, tree, selection, active_context, inLine)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD       = per screen data
 *  tree      = command tree
 *  selection = owned by inserting client
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void InsertTreeOnAllClients (WmScreenData *pSD, CmdTree *tree, Atom selection,
			     Context active_context, Boolean inLine)
{
    ClientListEntry *current;
    MatchList *global_matchlist = (MatchList *) NULL;
    MatchList *client_matchlist = (MatchList *) NULL;
    Context greyed_context = F_CONTEXT_ALL;

    /* If there aren't any clients, then there's nothing to do. */
    if (pSD->clientList == (ClientListEntry *) NULL)
      return;

    /* Setup the greyed context based on the active context */
    if (active_context & F_CONTEXT_WINDOW)
      greyed_context &= ~(F_CONTEXT_WINDOW);
    if (active_context & F_CONTEXT_ICON)
      greyed_context &= ~(F_CONTEXT_ICON);

    for (current = pSD->clientList;
	 current != (ClientListEntry *) NULL;
	 current = current->nextSibling)
    {
	/* Ignore client list entries for icons. */
	if (current->type == MINIMIZED_STATE)
	  continue;
	InsertTreeOnClient(pSD, current->pCD, tree, &client_matchlist,
			   &global_matchlist, current->pCD->systemMenuSpec,
			   NULL, NULL, FALSE, 
			   selection, greyed_context, inLine);
	PerformInsertionsOnMatchList(&client_matchlist);
	RecreateMenuWidgets(pSD, current->pCD, &client_matchlist);
        FreeMatchList(&client_matchlist);
    }
    PerformInsertionsOnMatchList(&global_matchlist);
    RecreateMenuWidgets(pSD, NULL /* no pcd */, &global_matchlist);
    FreeMatchList(&global_matchlist);
}


/*************************************<->*************************************
 *
 *  InsertTreeOnSingleClient (pSD, pCD, tree, selection, inLine)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD       = per screen data
 *  tree      = command tree
 *  selection = owned by inserting client
 *
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void InsertTreeOnSingleClient (WmScreenData *pSD, ClientData *pCD,
			       CmdTree *tree, Atom selection,
			       Context active_context, Boolean inLine)
{
    MatchList *global_matchlist = (MatchList *) NULL;
    MatchList *client_matchlist = (MatchList *) NULL;
    Context greyed_context = F_CONTEXT_ALL;

    /* A quick sanity check */
    if (pCD == (ClientData *) NULL)
      return;

    /* Setup the greyed context based on the active context */
    if (active_context & F_CONTEXT_WINDOW)
      greyed_context &= ~(F_CONTEXT_WINDOW);
    if (active_context & F_CONTEXT_ICON)
      greyed_context &= ~(F_CONTEXT_ICON);

    InsertTreeOnClient(pSD, pCD, tree, &client_matchlist,
		       &global_matchlist, pCD->systemMenuSpec,
		       NULL, NULL, TRUE, selection, greyed_context, inLine);
    PerformInsertionsOnMatchList(&client_matchlist);
    RecreateMenuWidgets(pSD, pCD, &client_matchlist);
    FreeMatchList(&client_matchlist);
}


/*************************************<->*************************************
 *
 *  InsertTreeOnRootMenu (pSD, tree, selection, active_context, inLine)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pSD       = per screen data
 *  tree      = command tree
 *  selection = owned by inserting client
 *
 * 
 *  Outputs:
 *  -------
 *  Return = 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void InsertTreeOnRootMenu (WmScreenData *pSD, CmdTree *tree, Atom selection,
			   Boolean inLine)
{
    MatchList *global_matchlist = (MatchList *) NULL;
    MatchList *client_matchlist = (MatchList *) NULL;
    Context greyed_context = F_CONTEXT_WINDOW | F_CONTEXT_ICON;
    MenuSpec *rootMenu;

    /* Find the root menu spec */
    for (rootMenu = pSD->menuSpecs;
	 rootMenu != (MenuSpec *) NULL;
	 rootMenu = rootMenu->nextMenuSpec)
    {
	if (strcmp(rootMenu->name, pSD->rootMenu) == 0)
	  break;
    }
    
    /* If we couldn't find the root menu, then do nothing. */
    if (rootMenu == (MenuSpec *) NULL) return;
    
    InsertTreeOnClient(pSD, NULL, tree, &client_matchlist,
		       &global_matchlist, rootMenu,
		       NULL, NULL, FALSE, selection, greyed_context, inLine);
    PerformInsertionsOnMatchList(&client_matchlist);
    RecreateMenuWidgets(pSD, NULL, &client_matchlist);
    FreeMatchList(&client_matchlist);
    PerformInsertionsOnMatchList(&global_matchlist);
    RecreateMenuWidgets(pSD, NULL, &global_matchlist);
    FreeMatchList(&global_matchlist);
}


/*************************************<->*************************************
 *
 *  RemoveClientCommandFromMenuSpec (menuSpec, id)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static Boolean RemoveClientCommandFromMenuSpec (MenuSpec *menuSpec,
						CARD32 id)
{
    MenuItem *curMenuItem, *prevMenuItem = (MenuItem *) NULL;
    MenuItem *tmpMenuItem;
    Boolean was_changed = FALSE;

    curMenuItem = menuSpec->menuItems;
    while (curMenuItem != (MenuItem *) NULL)
    {
	if (curMenuItem->clientCommandID == id)
	{
	    tmpMenuItem = curMenuItem;
	    curMenuItem = curMenuItem->nextMenuItem;
	    if (prevMenuItem == (MenuItem *) NULL)
		menuSpec->menuItems = tmpMenuItem->nextMenuItem;
	    else
		prevMenuItem->nextMenuItem = tmpMenuItem->nextMenuItem;
	    FreeMenuItem(tmpMenuItem);
	    was_changed = TRUE;
	}
	else
	{
	    prevMenuItem = curMenuItem;
	    curMenuItem = curMenuItem->nextMenuItem;
	}
    }
    return(was_changed);
}


/*************************************<->*************************************
 *
 *  ModifyClientCommandForMenuSpec (menuSpec, id, modifier, context, newname)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static Boolean ModifyClientCommandForMenuSpec (MenuSpec *menuSpec,
					       CARD32 id,
					       CmdModifier modifier,
					       Context context,
					       String newname)
{
    MenuItem *curMenuItem;
    Boolean was_changed = FALSE;
    int i, freebutton, buttoncount;

    /* If the menuspec doesn't have any buttons or a valid menu widget
       then we don't want to search it. */
    if (menuSpec->menuWidget == (Widget) NULL ||
	menuSpec->menuButtons == (MenuButton *) NULL ||
	menuSpec->menuButtonCount == 0)
      return(FALSE);

    /* Search through all the menu buttons of the menuspec for buttons
       which match the command ID to be removed. */
    for (i = 0; i < menuSpec->menuButtonCount; ++i)
    {
	curMenuItem = menuSpec->menuButtons[i].menuItem;

	if ((curMenuItem->clientCommandID == id) &&
	    (curMenuItem->wmFunction == F_InvokeCommand))
	{
	    switch(modifier)
	    {
	      case ENABLE:
		/* "context" is an active context */
		curMenuItem->greyedContext &= ~(context);
		/* Adjust the pushbutton to the state it would have had
		   given the last posting context. */
		if (menuSpec->currentContext & curMenuItem->greyedContext)
		  XtSetSensitive(menuSpec->menuButtons[i].buttonWidget,
				 FALSE);
		else
		  XtSetSensitive(menuSpec->menuButtons[i].buttonWidget,
				 TRUE);
		break;
	      case DISABLE:
		/* "context" is a greyed context */
		curMenuItem->greyedContext |= context;
		/* Adjust the pushbutton to the state it would have had
		   given the last posting context. */
		if (menuSpec->currentContext & curMenuItem->greyedContext)
		  XtSetSensitive(menuSpec->menuButtons[i].buttonWidget,
				 FALSE);
		else
		  XtSetSensitive(menuSpec->menuButtons[i].buttonWidget,
				 TRUE);
		break;
	      case RENAME:
		if (newname != NULL && *newname != '\0')
		{
		  /* When renaming a command, we shouldn't cause the
		   * entire menu to be recreated.  Recreating the menu
		   * will cause problems with tearoffs since the menu
		   * will disappear when it is destroyed. CR 9719
		   */
		  XmString labelString;

		  /* Change the label of the menu item */
		  XtFree(curMenuItem->label);
		  /* Give the menu item the new name */
		  curMenuItem->label = XtNewString(newname);
		  was_changed = False;  /*  all taken care of here. */
		  
		  /* This is needed when changing the label since
		   * mwm will wait for a geometry reply from itself which
		   * it can never service. CR 9719
		   */
		  XtVaSetValues(XtParent(XtParent(menuSpec->menuButtons[i].buttonWidget)),
				XmNuseAsyncGeometry, TRUE, NULL);

		  labelString = XmStringGenerate(curMenuItem->label,
						 XmFONTLIST_DEFAULT_TAG, 
						 XmCHARSET_TEXT, NULL);
		  XtVaSetValues(menuSpec->menuButtons[i].buttonWidget,
				XmNlabelString, labelString,
				NULL);
		  XmStringFree(labelString);
		}
		break;
	      case REMOVE:
		XtDestroyWidget(menuSpec->menuButtons[i].buttonWidget);
		menuSpec->menuButtons[i].managed = FALSE;
		menuSpec->menuButtons[i].menuItem = (MenuItem *) NULL;
		menuSpec->menuButtons[i].buttonWidget = (Widget) NULL;
		break;
	    }
	}
    }

    /* If we are being asked to remove a client command, then we need
     * to search through all the menu items as well as the buttons.
     * Do the menu items here.
     */
    if (modifier == REMOVE)
	was_changed = RemoveClientCommandFromMenuSpec(menuSpec, id);

    /* Compact the menu buttons array. */
    buttoncount = menuSpec->menuButtonCount;
    freebutton = 0;
    for (i = 0; i < buttoncount; ++i)
    {
	if (menuSpec->menuButtons[i].buttonWidget == (Widget) NULL)
	    --menuSpec->menuButtonCount;
	else
	{
	    menuSpec->menuButtons[freebutton].menuItem = 
	      menuSpec->menuButtons[i].menuItem;
	    menuSpec->menuButtons[freebutton].buttonWidget = 
	      menuSpec->menuButtons[i].buttonWidget;
	    menuSpec->menuButtons[freebutton].managed =
	      menuSpec->menuButtons[i].managed;
	    ++freebutton;
	}
    }
    return(was_changed);
}


/*************************************<->*************************************
 *
 *  ModifyClientCommandID (pSD, pCD, range, id, modifier, context, newname)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static void ModifyClientCommandID (WmScreenData *pSD,
				   ClientData *pCD,
				   OpRange range,
				   CARD32 id,
				   CmdModifier modifier,
				   Context context,
				   String newname)
{
    MenuSpec *curMenuSpec;
    ClientListEntry *curClient;

    switch(range)
    {
      case ALL:
	/* Search through all the menu specs of all the clients. */
	for (curClient = pSD->clientList;
	     curClient != (ClientListEntry *) NULL;
	     curClient = curClient->nextSibling)
	{
	    for (curMenuSpec = curClient->pCD->systemMenuSpec;
		 curMenuSpec != (MenuSpec *) NULL;
		 curMenuSpec = curMenuSpec->nextMenuSpec)
	    {
		/* If the menu spec is global then stop searching
		   for this client. */
		if (curMenuSpec->clientLocal == FALSE)
		  break;
		if (ModifyClientCommandForMenuSpec(curMenuSpec, id,
						   modifier, context,
						   newname) == TRUE)
		{
		    DestroyMenuSpecWidgets(curMenuSpec);
		    curMenuSpec->menuWidget =
		      CreateMenuWidget (pSD, curClient->pCD, curMenuSpec->name,
					pSD->screenTopLevelW, TRUE,
					curMenuSpec, NULL);
		}
	    }
	}
	/* Search through all the global menu specs. */
	for (curMenuSpec = pSD->menuSpecs;
	     curMenuSpec != (MenuSpec *) NULL;
	     curMenuSpec = curMenuSpec->nextMenuSpec)
	{
	    if (ModifyClientCommandForMenuSpec(curMenuSpec, id, modifier,
					       context, newname) == TRUE)
	    {
		DestroyMenuSpecWidgets(curMenuSpec);
		curMenuSpec->menuWidget =
		  CreateMenuWidget (pSD, NULL, curMenuSpec->name,
				    pSD->screenTopLevelW, TRUE,
				    curMenuSpec, NULL);
	    }
	}
	break;
      case ROOT:
        {
	    /*
	     * This section was changed to search the entire global menu list.
	     * This was done to allow access to menu entries included using the
	     * cci/.mwmrc interface.  Before, only the actual root menu could
	     * be modified; however, the user could still include commands in
	     * other menus specified in the .mwmrc file using the f.cci command.
	     */


	    MenuSpec *curMenuSpec;

	    /* Search through all the global menu specs. */
	    for (curMenuSpec = pSD->menuSpecs;
		 curMenuSpec != (MenuSpec *) NULL;
		 curMenuSpec = curMenuSpec->nextMenuSpec)
	    {
	      if (ModifyClientCommandForMenuSpec(curMenuSpec, id, modifier,
						 context, newname) == TRUE)
		{
		  DestroyMenuSpecWidgets(curMenuSpec);
		  curMenuSpec->menuWidget =
		    CreateMenuWidget (pSD, NULL, curMenuSpec->name,
				      pSD->screenTopLevelW, TRUE,
				      curMenuSpec, NULL);
		}
	    }
        }
	break;
      case SINGLE:
	/* If we weren't passed a valid pCD, then just return. */
	if (pCD == (ClientData *) NULL) return;

	/* Search through the clients menu specs. If we find one that 
	   is global then stop search if we are ENABLING or DISABLING.
	   If we are REMOVING and we find a global, we may need to 
	   perform some menu spec replacing to make the menu spec that
	   needs modification local to the client. */
	for (curMenuSpec = pCD->systemMenuSpec;
	     curMenuSpec != (MenuSpec *) NULL;
	     curMenuSpec = curMenuSpec->nextMenuSpec)
	{
	    if (curMenuSpec->clientLocal == FALSE)
	    {
		MenuSpec *last, *cur;

		/* Find the last global menuspec in the clients list
		   that needs to be changed and return it. Replace
		   all menuspecs between the current one and the
		   "last" one that needs changing. All the replaced
		   menuspecs will be marked as local, so that next
		   time clientLocal is FALSE in the enclosing for
		   loop above, there will be no global menu specs
		   needing changes. In other words, all the required
		   menu spec replacing will occur the first time we
		   find a global menu spec. */
		last = FindLastMenuSpecToModify(curMenuSpec, id);
		if (last != (MenuSpec *) NULL)
		{
		    MenuSpec *newMenuSpec = (MenuSpec *) NULL;
		    MenuSpec *firstMenuSpec = (MenuSpec *) NULL;
		    MenuSpec *lastMenuSpec = (MenuSpec *) NULL;

		    /* Replace all the global menu specs with local 
		       ones. */
		    for (cur = curMenuSpec;
			 cur != (MenuSpec *) NULL && cur != last->nextMenuSpec;
			 cur = cur->nextMenuSpec)
		    {
			newMenuSpec = ReplaceMenuSpecForClient(cur, pCD);
			if (cur == curMenuSpec)
			  curMenuSpec = firstMenuSpec = newMenuSpec;
			/* If there is only one menu spec to change,
			   the first will also be the last. */
			if (cur == last)
			  lastMenuSpec = newMenuSpec;
		    }

		    /* Now that we have replaced all the menu specs, 
		       recreate all the widgets for the new menu specs. */
		    for (cur = firstMenuSpec;
			 cur != (MenuSpec *) NULL &&
			 cur != lastMenuSpec->nextMenuSpec;
			 cur = cur->nextMenuSpec)
		    {
			DestroyMenuSpecWidgets(newMenuSpec);
			newMenuSpec->menuWidget = 
			  CreateMenuWidget(pSD, pCD, newMenuSpec->name,
					   pSD->screenTopLevelW,
					   TRUE, newMenuSpec, NULL);
		    }

		}
		/* None of the globals need changing. */
		else break;
	    }
	    if (ModifyClientCommandForMenuSpec(curMenuSpec, id, modifier,
					       context, newname) == TRUE)
	    {
		DestroyMenuSpecWidgets(curMenuSpec);
		curMenuSpec->menuWidget =
		  CreateMenuWidget (pSD, pCD, curMenuSpec->name,
				    pSD->screenTopLevelW, TRUE,
				    curMenuSpec, NULL);
	    }
	}
	break;
    }
}


/*************************************<->*************************************
 *
 *  ModifyClientCommandTree (pSD, pCD, range, tree, modifier, context, newname)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void ModifyClientCommandTree (WmScreenData *pSD,
			      ClientData *pCD,
			      OpRange range,
			      CmdTree *tree,
			      CmdModifier modifier,
			      Context context,
			      String newname)
{
    CmdTree *curTree;
    CARD32 cmdID;

    /* Run through the top level of the tree. */
    for (curTree = tree; curTree != (CmdTree *) NULL; curTree = curTree->next)
    {
	cmdID = curTree->commandID;
	ModifyClientCommandID(pSD, pCD, range, cmdID, modifier,
			      context, newname);
	if (curTree->subTrees != (CmdTree *) NULL)
	  ModifyClientCommandTree(pSD, pCD, range, curTree->subTrees,
				  modifier, context, newname);
    }
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

/*************************************<->*************************************
 *
 *  static Boolean
 *  AdjustPBs (menuSpec, pCD, newContext)
 *
 *
 *  Description:
 *  -----------
 *  This procedure adjusts menu PushButton sensitivities and manage/unmanaged
 *  status for a toplevel menu.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =    nonNULL toplevel menu specification with gadget
 *  pCD =         client data
 *  newContext =  context that the menu is to be posted under.
 *
 * 
 *  Outputs:
 *  -------
 *  menuSpec =      menu specification with modifications
 *  Return =        TRUE iff at least one menu item changed manage status.
 *
 *
 *  Comments:
 *  --------
 *  Adjusts PushButton sensitivity according to context and function type.
 *  Manages/Unmanages PushButtons according to clientFunction resource.
 * 
 *************************************<->***********************************/
static Boolean AdjustPBs (MenuSpec *menuSpec, ClientData  *pCD,
			  Context newContext)
{
    MenuButton    *menuButton;
    MenuItem      *menuItem;
    int            msgc;
    unsigned int   n;
    long          *pMsg;
    Boolean        fSupported;
    Boolean        fChangeManaged = FALSE;

    /*
     *  Set PushButton sensitivity.
     *  Set f.send_msg button sensitivity according to context and client 
     *  message list.  Adjust other button sensitivities only for context.
     */

    /* check for bad input value - shouldn't happen. */
    if (menuSpec == NULL) return (FALSE);

    for (n = 0, menuButton = menuSpec->menuButtons;
         n < menuSpec->menuButtonCount;
	 n++, menuButton++)
    {
        menuItem = menuButton->menuItem;
        if (menuItem->wmFunction == F_Send_Msg)
	/* f.send_msg button:  set according to context and message. */
        {
            if ((newContext & menuItem->greyedContext) ||
		!(pCD && pCD->mwmMessagesCount && pCD->mwmMessages))
	    /* insensitive context or empty client message list */
	    {
	        XtSetSensitive (menuButton->buttonWidget, FALSE);
	    }
	    else
            /* 
             * Have a context sensitive f.send_msg item and a client with a 
	     * nonempty message list.  Set sensitive only if the message is 
	     * supported by this client.  Otherwise set insensitive.
             */
            {
                msgc = pCD->mwmMessagesCount;
		pMsg = pCD->mwmMessages;
	        fSupported = FALSE;
	        while (msgc--)
		/* scan nonempty message list */
	        {
	            if (*pMsg == (long) menuItem->wmFuncArgs)
	            /* found match */
	            {
		        fSupported = TRUE;
		        break;
	            }
	            pMsg++;  /* next message in list */
	        }
	        XtSetSensitive (menuButton->buttonWidget, fSupported);
            }
	}
	else
	/*
	 * Non f.send_msg button:
	 *  Adjust sensitivity according to context.
	 *  Manage/Unmanage according to clientFunction.
	 */
	{
	    if (menuSpec->currentContext & menuItem->greyedContext)
	    /* button is currently insensitive */
	    {
	        if (!(newContext & menuItem->greyedContext))
	        /* insensitive -> sensitive */
	        {
	            XtSetSensitive (menuButton->buttonWidget, TRUE);
	        }
	    }
	    else
	    /* button is currently sensitive */
	    {
	        if (newContext & menuItem->greyedContext)
	        /* sensitive -> insensitive */
	        {
	            XtSetSensitive (menuButton->buttonWidget, FALSE);
	        }
	    }
#ifdef WSM
	    if (menuItem->wmFunction == F_Remove)
	    {
		/*
		 * Only allow remove from workspace if the client
		 * is in more than one workspace
		 */
		fSupported = (pCD && (pCD->numInhabited > 1));
		XtSetSensitive (menuButton->buttonWidget, fSupported);
	    }
#endif /* WSM */

	    if ((menuItem->mgtMask) && pCD)
	    /* PushButton might not apply */
	    {
#ifdef WSM
	        if ((pCD->clientFunctions & menuItem->mgtMask & MWM_MGT_MASK) ||
		    (pCD->dtwmFunctions & menuItem->mgtMask & DTWM_MGT_MASK))
#else /* WSM */
	        if (pCD->clientFunctions & menuItem->mgtMask)
#endif /* WSM */
	        /* function applies -- manage it */
	        {
	            if (!menuButton->managed)
	            /* unmanaged -> managed */
	            {
	                XtManageChild (menuButton->buttonWidget);
	                menuButton->managed = TRUE;
                        fChangeManaged = TRUE;
			if (n == menuSpec->menuButtonCount - 1)
			{
			    /* 
			     * last item, if it has a separator before
			     * it, manage the separator
			     */
			    
			    CheckTerminalSeparator(menuSpec, 
						   menuButton->buttonWidget, 
						   True);
			}
	            }
	        }
	        else
	        /* function does not apply -- unmanage it */
	        {
	            if (menuButton->managed)
	            /* managed -> unmanaged */
	            {
	                XtUnmanageChild (menuButton->buttonWidget);
	                menuButton->managed = FALSE;
                        fChangeManaged = TRUE;

			if (n == menuSpec->menuButtonCount - 1)
			{
			    /* 
			     * last item, if it has a separator before
			     * it, unmanage the separator
			     */
			    CheckTerminalSeparator(menuSpec, 
						   menuButton->buttonWidget, 
						   False);
			}

	            }
	        }
	    }
	    else if (!menuButton->managed)
	    /* unmanaged PushButton applies */
	    {
	        XtManageChild (menuButton->buttonWidget);
	        menuButton->managed = TRUE;
                fChangeManaged = TRUE;
	    }
	}
    }

    return (fChangeManaged);

} /* END OF FUNCTION AdjustPBs */



/*************************************<->*************************************
 *
 *  static Boolean
 *  SavePBInfo (topMenuSpec, menuItem, itemW)
 *
 *
 *  Description:
 *  -----------
 *  Fills a MenuButton structure for a PushButton.
 *  If necessary, mallocs or reallocs the menuButtons array in the toplevel
 *  MenuSpec.
 *
 *
 *  Inputs:
 *  ------
 *  topMenuSpec = pointer to toplevel MenuSpec structure
 *  menuItem    = pointer to PushButton MenuItem structure
 *  itemW       = PushButton gadget
 *  topMenuSpec->menuButtons[]
 *  topMenuSpec->menuButtonSize
 *  topMenuSpec->menuButtonCount
 *
 * 
 *  Outputs:
 *  -------
 *  Return = FALSE iff insufficient memory for malloc or realloc
 *           or bad input value forces exit.
 *  topMenuSpec->menuButtons[]
 *  topMenuSpec->menuButtonSize
 *  topMenuSpec->menuButtonCount
 *
 *
 *  Comments:
 *  --------
 *  The initial managed status of PushButtons is TRUE.
 * 
 *************************************<->***********************************/
static Boolean SavePBInfo (MenuSpec *topMenuSpec, MenuItem *menuItem,
			     Widget itemW)
{
    MenuButton *menuButton;


    /* check for bad input value - shouldn't happen. */
    if (topMenuSpec == NULL) return (FALSE);

    if (topMenuSpec->menuButtonSize == 0)
    /* need to create array */
    {
        topMenuSpec->menuButtonSize = MENU_BUTTON_INC;
        topMenuSpec->menuButtons =
	    (MenuButton *) XtMalloc (MENU_BUTTON_INC * sizeof(MenuButton));
    }
    else if (topMenuSpec->menuButtonCount == topMenuSpec->menuButtonSize)
    /* need larger array */
    {
        topMenuSpec->menuButtonSize += MENU_BUTTON_INC;
        topMenuSpec->menuButtons = (MenuButton *) 
	    XtRealloc ((char*)topMenuSpec->menuButtons,
		     topMenuSpec->menuButtonSize * sizeof(MenuButton));
    }

    if (topMenuSpec->menuButtons == NULL)
    /* insufficent memory */
    {
	topMenuSpec->menuButtonSize = 0;
	topMenuSpec->menuButtonCount = 0;
	return (FALSE);
    }

    menuButton = &(topMenuSpec->menuButtons[topMenuSpec->menuButtonCount]);
    topMenuSpec->menuButtonCount++;

    menuButton->menuItem = menuItem;
    menuButton->buttonWidget = itemW;
    menuButton->managed = TRUE;
    return (TRUE);

}



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 * AdjustTearOffControl (cascade, closure, cbackdata) 
 *  
 * 
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *     returns true iff the tearoff control was enabled or diabled
 *     resulting in a change in height.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
static Boolean
AdjustTearOffControl (Widget cascade,
		      XtPointer closure,
		      XtPointer cbackdata)
{
    Widget submenu = (Widget) closure;
    int argn;
    Arg args[10];
    unsigned char tearoff_model;
    Boolean isMwmMenu;

    argn = 0;
    XtSetArg(args[argn], XmNtearOffModel, &tearoff_model); ++argn;
    XtGetValues(submenu, args, argn);

    /* Is this a root menu or a cascade of a root menu?         */
    /* If cbackdata is not null, then we got here by cascading. */
    /* Cascade menus from a tearoff menu-pane are not allowed.  */
    /* there is no way to know if the cascade is from a tearoff */
    /* or from a cascade on a system menu.                      */
    if ((wmGD.menuClient == NULL) && (cbackdata == NULL))
      isMwmMenu = True;
    else
      isMwmMenu = False;

    if ((tearoff_model == XmTEAR_OFF_ENABLED) && !isMwmMenu)
    {
	PRINT("Disabling the tear off\n");
	argn = 0;
	XtSetArg(args[argn], XmNtearOffModel, XmTEAR_OFF_DISABLED); ++argn;
	XtSetValues(submenu, args, argn);

	return (True);
    }

    /* If this was invoked as a cascadingCallback and not by hand and if
       the menuActive field of the global data has not yet been set, then
       we can safely assume that we have just cascaded off of a torn off
       menu. In that case, set the menuActive field to be the menu spec of
       the torn off menu and register an unmap callback on the cascaded
       menu that will clear the menuActive field. */
    if (cbackdata != (XtPointer) NULL && wmGD.menuActive == (MenuSpec *) NULL)
    {
	MenuSpec *menuspec;
	Widget tearoff_widget = XtParent(cascade);

	for (menuspec = wmGD.Screens[0].menuSpecs;
	     menuspec != (MenuSpec *) NULL;
	     menuspec = menuspec->nextMenuSpec)
	{
	    if (tearoff_widget == menuspec->menuWidget)
	    {
		wmGD.menuActive = menuspec;
		break;
	    }
	}

	/* If we can't find a menuspec for the torn off menu, then just
	   take the first menu spec in the list of menuSpecs for the
	   active pSD. NOTE: THIS SHOULD NEVER HAPPEN. In fact if it
	   does, I'm not sure how mwm will behave having been given
	   the wrong menu spec as the active menu. */
	if (wmGD.menuActive == (MenuSpec *) NULL)
	{
	    wmGD.menuActive = ACTIVE_PSD->menuSpecs;
	    PRINT("Couldn't find menu spec for tear off\n");
	}

	/* Add a callback that will clear menuActive when this cascade
	   is unmapped. */
#if 0
	XtAddCallback (submenu, XmNunmapCallback,
#else
	XtAddCallback (XtParent(submenu), XmNpopdownCallback,
#endif
		       UnmapPulldownCallback,
		       (XtPointer) NULL);
    }

  return (False);
}


/*************************************<->*************************************
 *
 *  static Boolean
 *  CreateClientCommandSeparator (menuItem, child_position, last_child,
 *				  newMenuItem)
 *
 *
 *  Description:
 *  -----------
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
static Boolean CreateClientCommandSeparator (MenuItem *menuItem,
					     int       child_position,
					     Widget    last_child,
					     MenuItem **newMenuItem)
{
    MenuItem *curMenuItem;

    /* If it is a client command, then we only want to create the
     * separator under particular circumstances. Specifically, we
     * want to make sure that:
     *   1. a separator doesn't directly precede this one
     *   2. a separator doesn't directly follow this one
     *   3. this separator won't be the first or last item in the menu
     *   4. the client command that this separator surrounds actually
     *      matched something and is not an unmatched template
     */

    /* Check if a separator directly precedes this one. */
    if (child_position > 0 && last_child != (Widget) NULL &&
	XmIsSeparator(last_child))
      return(FALSE);

    /* Check if a separator directly follows this one. */
    if (menuItem->nextMenuItem != (MenuItem *) NULL &&
	menuItem->nextMenuItem->wmFunction == F_Separator &&
	IsClientCommand(menuItem->nextMenuItem->label) == FALSE)
      return(FALSE);

    /* Make sure this separator won't be the first item in the menu. */
    if (child_position == 0) return(FALSE);

    /* Make sure this separator won't be the last item in the menu. */
    if (menuItem->nextMenuItem == (MenuItem *) NULL)
      return(FALSE);

    /* Make sure that the client command this separator surrounds actually
       matches something. We only do this check if the separator is the 
       TOP separator in the separator pair. If we are looking at a bottom
       separator then we can safely assume something matched, otherwise
       we would have passed over it when we look at the corresponding top
       separator. */
    if (menuItem->labelType == TOP_SEPARATOR)
    {
	/* If we find a real menu item (not just a template) before we find
	   a bottom separator, then create the separator. */
	for (curMenuItem = menuItem;
	     curMenuItem != (MenuItem *) NULL;
	     curMenuItem = curMenuItem->nextMenuItem)
	{
	    /* If we found the closing separator, then return FALSE and
	       our new menu item position. */
	    if (curMenuItem->wmFunction == F_Separator &&
		IsClientCommand(curMenuItem->label) &&
		curMenuItem->labelType == BOTTOM_SEPARATOR)
	    {
		*newMenuItem = curMenuItem;
		return(FALSE);
	    }
	    /* If we found a real menu item, then return TRUE. */
	    if (curMenuItem->wmFunction != F_Separator &&
		!IsClientCommand(curMenuItem->label))
	    {
		return(TRUE);
	    }
	}
	/* If by some bizarre chance we get to the end of the list
	   without finding either, then return FALSE. Something is wrong. */
	if (curMenuItem == (MenuItem *) NULL) return(FALSE);
    }

    /* Well, nothing failed so let's create it. */
    return(TRUE);
}
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

/*************************************<->*************************************
 *
 *  CreateMenuWidget (pSD, menuName, parent, fTopLevelPane, topMenuSpec, 
 *                    moreMenuItems)
 *
 *
 *  Description:
 *  -----------
 *  Creates a MenuShell as a child of the specified parent widget, and a 
 *  PopupMenu or PulldownMenu as a child of the shell.  Fill the menu with
 *  the named menupane items.
 *
 *
 *  Inputs:
 *  ------
 *  pSD ---------- pointer to screen data
 *  menuName ----- the name of the menu specification to be used to create
 *                 the menu widget.
 *  parent -------- parent of popup shell
 *  fTopLevelPane - TRUE iff the menupane is a top level one
 *  topMenuSpec --- pointer to the top menu specification.
 *  moreMenuItems - pointer to additional menu items for custom menu.
 * 
 * 
 *  Outputs:
 *  -------
 *  Return = created PopupMenu or PulldownMenu widget, or NULL.
 *
 *
 *  Comments:
 *  --------
 *  We attach a popdowncallback to the menu to set wmGD.menuActive to NULL,
 *  allowing us to not dispatch key events separately from the toolkit 
 *  dispatcher.
 * 
 *************************************<->***********************************/

typedef struct _StrList
{
   XmString         string;
   struct _StrList *next;
} StrList;

Widget CreateMenuWidget (WmScreenData *pSD,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
			 ClientData *pCD,
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
			 String menuName, Widget parent,
			 Boolean fTopLevelPane, MenuSpec *topMenuSpec,
			 MenuItem *moreMenuItems)
{
    int         i, n;
    Arg         sepArgs[1];
    Arg         args[10];
    MenuSpec   *menuSpec = (MenuSpec *)NULL;
    MenuItem   *menuItem;
    Widget      menuShellW;
    Widget      menuW;
    Widget      subMenuW;
    Widget      children[CHILDREN_CACHE];
    Pixmap      labelPixmap;
    KeySpec    *accelKeySpec;
    Dimension   menuHeight;
    Boolean     fUseTitleSep = False;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    Boolean     labelIsClientCommand = False;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    StrList    *stringsToFree = NULL, *sPtr;
    XmString    tmpstr;
#ifndef	IBM_151913
    Screen     *scr;
#endif


    /* check for bad input values. */
    if ((menuName == NULL) || (pSD == NULL))
    {
	return (NULL);
    }

    /*
     *  Find the menu pane specifications for menuName.
     *  The top-level menu specification is passed as an argument (it may
     *  be custom).  A submenu specification must be found and might not exist.
     *  Return NULL if a submenu specification is not found.
     */
    if (fTopLevelPane)
    {
        menuSpec = topMenuSpec;
    }
    else
    {
        menuSpec = pSD->menuSpecs;
        while (menuSpec)
        {
	    if ((menuSpec->name != NULL) && !strcmp (menuSpec->name, menuName))
	    {
	        break;  /* found menuName's specification */
	    }
	    menuSpec = menuSpec->nextMenuSpec;  /* keep looking */
        }
    }

    if (menuSpec == NULL)
    /* (submenu) specification not found */
    {
	MWarning(((char *)GETMESSAGE(48, 4, "Menu specification %s not found\n")), menuName);
	return (NULL);
    }

    /*
     * If menuSpec is marked, we have menu recursion => fail.
     *  Otherwise, mark it.
     */

    if (menuSpec->currentContext & CR_MENU_MARK)   /* marked? */
    /* menu recursion */
    {
	MWarning(((char *)GETMESSAGE(48, 5, "Menu recursion detected for %s\n")), menuName);
	return (NULL);
    }
    menuSpec->currentContext |= CR_MENU_MARK;   /* no, mark it */

    /*
     * Create a PopupShell widget.
     * If the parent of the specified parent ("grandparent") is a MenuShell 
     * widget, then use the grandparent as the parent of the PopupShell.
     * Otherwise, use the specified parent.
     */
    i = 0;
    XtSetArg (args[i], XmNwidth, (XtArgVal) 5); i++;
    XtSetArg (args[i], XmNheight, (XtArgVal) 5); i++;
    XtSetArg (args[i], XmNallowShellResize, (XtArgVal) TRUE); i++;
    XtSetArg (args[i], XtNoverrideRedirect, (XtArgVal) TRUE); i++;
    XtSetArg (args[i], XtNdepth, 
	      (XtArgVal) DefaultDepth(XtDisplay(parent), pSD->screen)); i++;
    XtSetArg (args[i], XtNscreen, 
	      (XtArgVal) ScreenOfDisplay(XtDisplay(parent), pSD->screen)); i++;

    if ((XtParent (parent) != NULL) && XmIsMenuShell (XtParent (parent)))
    {
	parent = XtParent (parent);
    }

    menuShellW = XtCreatePopupShell (SHELL_NAME, xmMenuShellWidgetClass,
                                     parent, (ArgList) args, i);

    /*
     * Create a RowColumn widget as a child of the shell for the menu pane.
     * If the menu pane is top-level, create a popup menu for it and attach 
     *   the unmap callback to it.
     * Otherwise, create a pulldown menu for it.
     */

    i = 0;
    XtSetArg (args[i], XmNborderWidth, (XtArgVal) 0); i++;
    XtSetArg (args[i], XmNwhichButton, (XtArgVal) SELECT_BUTTON); i++;
    XtSetArg (args[i], XmNadjustMargin, (XtArgVal) TRUE); i++;

    if (fTopLevelPane)
    {
        XtSetArg (args[i], XmNrowColumnType, (XtArgVal) XmMENU_POPUP); i++;
        XtSetArg (args[i], XmNpopupEnabled, (XtArgVal) TRUE); i++;
        menuW = XtCreateWidget (menuName, xmRowColumnWidgetClass, menuShellW,
				(ArgList) args, i);
	XtAddCallback (menuW, XmNunmapCallback, UnmapCallback, 
				(XtPointer) menuSpec);
    }
    else
    {
        XtSetArg (args[i], XmNrowColumnType, (XtArgVal) XmMENU_PULLDOWN); i++;
        menuW = XtCreateWidget (menuName, xmRowColumnWidgetClass, menuShellW,
				(ArgList) args, i);
    }

    /*
     * Create the specified menu entries as children of the menupane.
     * Menus may contain the following widgets:
     *
     *   Label
     *   Separator 
     *   CascadeButton
     *   PushButton
     *
     * Add separator gadgets around menu titles.
     */

    XtSetArg (sepArgs[0], XmNseparatorType, (XtArgVal) XmDOUBLE_LINE);

    n = 0;
    menuItem = menuSpec->menuItems;
    if ((menuItem == NULL) && (moreMenuItems != NULL))
    /* handle custom menu with empty standard specification */
    {
        menuSpec->menuItems = menuItem = moreMenuItems;
	moreMenuItems = NULL;
    }
    while (menuItem)
    {
        i = 0;

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	labelIsClientCommand = IsClientCommand(menuItem->label);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

	if (menuItem->wmFunction == F_Separator)
	/* 
	 * Add a Separator gadget for a menu separator.
	 * An immediately following title will not have a top separator.
	 */
	{
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	    /* If it is a client command, then we only want to create the
	     * separator under particular circumstances. Specifically, we
	     * want to make sure that:
	     *   1. a separator doesn't directly precede this one
	     *   2. a separator doesn't directly follow this one
	     *   3. this separator won't be the first or last item in the menu
	     */
	    if (labelIsClientCommand)
	    {
		if (CreateClientCommandSeparator(menuItem, n,
						 (n > 0 ? children[n - 1] :
						  (Widget) NULL),
						 &menuItem))
		{    
		    /* Increment the counter here because we only increment
		       at the end of the loop if the item is not a client
		       command item (i.e. labelIsClientCommand == FALSE) */
		    children[n++] =
		      XmCreateSeparatorGadget (menuW, SEPARATOR_NAME, 
					       (ArgList)NULL, 0);
		    fUseTitleSep = FALSE;
		}
	    }
	    else
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	    {
		children[n] =
		  XmCreateSeparatorGadget (menuW, SEPARATOR_NAME, 
					   (ArgList)NULL, 0);
		fUseTitleSep = FALSE;
	    }
	} /* F_Separator */

	else
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	    if (!labelIsClientCommand)
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	/*
         * We will use one of:
	 *
         *   Label
         *   CascadeButton
         *   PushButton
	 */
        {
	    /*
	     * Construct the label
	     */
            if ((menuItem->labelType == XmPIXMAP) &&
	         (labelPixmap =
		      MakeCachedLabelPixmap (pSD, menuW,
                                             menuItem->labelBitmapIndex)))
            {
                XtSetArg (args[i], XmNlabelType, (XtArgVal) XmPIXMAP); i++;
                XtSetArg (args[i], XmNlabelPixmap, (XtArgVal) labelPixmap); i++;
                XtSetArg (args[i], XmNlabelInsensitivePixmap,
			  (XtArgVal) labelPixmap); i++;
            }
            else
            {
                XtSetArg (args[i], XmNlabelType, (XtArgVal) XmSTRING); i++;
                XtSetArg (args[i], XmNlabelString, (XtArgVal)
			  (tmpstr = XmStringCreateLocalized(menuItem->label))); i++;
		sPtr = (StrList *) XtMalloc(sizeof(StrList));
		if (sPtr == NULL)
		  {
		     MWarning(((char *)GETMESSAGE(48, 2, "Insufficient memory for menu %s\n")), menuName);
		     return (NULL);
		  }
		else
		  {
		     sPtr->string  = tmpstr;
		     sPtr->next    = stringsToFree;
		     stringsToFree = sPtr;
		  }
            }

	    if (menuItem->wmFunction == F_Title)
	    /* 
	     * Add a centered Label gadget for a menu title.
	     * Include separators above and below the title.
	     * Don't include the top one if the title is the first pane item
	     *   or immediately follows a user-supplied separator.
	     */
	    {
                if (fUseTitleSep)
		{
                    children[n] =
			XmCreateSeparatorGadget (menuW, SEPARATOR_NAME,
					         sepArgs, 1); n++;
		}

                XtSetArg (args[i], XmNalignment, XmALIGNMENT_CENTER); i++; 
	        children[n] = XmCreateLabelGadget (menuW, TITLE_NAME,
					           (ArgList) args, i); n++;
                children[n] = XmCreateSeparatorGadget (menuW, SEPARATOR_NAME,
					  sepArgs, 1); 

                /*
		 * A following title will have both separators.
		 */

                fUseTitleSep = TRUE;
	    }

	    else
	    /*
             * We will use one of:
	     *
             *   CascadeButton
             *   PushButton
             *
	     * Both support mnemonics; only PushButtons support accelerators.
	     */
	    {
	        /*
		 * Align text on the left.
	         * Set any mnemonic text.
	         */
                XtSetArg (args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;

                if (menuItem->mnemonic)
                {
                    XtSetArg (args[i], XmNmnemonic, 
			       (XtArgVal) menuItem->mnemonic); i++;
                }

	        if (menuItem->wmFunction == F_Menu)
	        /* 
	         * Create a PopupShell and PulldownMenu for a submenu (the 
		 *   menushells are linked together).
	         * Create a CascadeButton Widget 
	         * The submenu widget is attached to the CascadeButton gadget
		 *   using the subMenuId resource.
	         * Make the CascadeButton insensitive if the submenu cannot be 
	         *   created.
	         */
	        {
		    subMenuW = CREATE_MENU_WIDGET (pSD, pCD,
						 menuItem->wmFuncArgs, menuW,
						 FALSE, topMenuSpec, 
						 (MenuItem *)NULL);
                    if (subMenuW)
		    /*
		     * Attach submenu to cascade button. 
		     */
		    {
                        XtSetArg (args[i], XmNsubMenuId, (XtArgVal) subMenuW);
			    i++;
	                children[n] = XmCreateCascadeButtonGadget (menuW,
					  CASCADE_BTN_NAME, (ArgList) args, i);
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
			XtAddCallback(children[n], XmNcascadingCallback,
				      (XtCallbackProc)AdjustTearOffControl,
				      (XtPointer)subMenuW);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
		    }
		    else
		    /*
		     * Unable to create submenupane: make the entry insensitive.
		     */
	            {
	                children[n] = XmCreateCascadeButtonGadget (menuW,
					  CASCADE_BTN_NAME, (ArgList) args, i);
	                XtSetSensitive (children[n], FALSE);
	            }

                    /*
		     * A following title will have both separators.
		     */

                    fUseTitleSep = TRUE;
	        }

	        else 
	        /* 
	         * Create a PushButton gadget.
	         */
	        {
	            /*
	             * If an accelerator is specified, set acceleratorText,
		     * then create an accelerator KeySpec and insert it at the
		     * head of the toplevel MenuSpec's list.
	             */
                    if (menuItem->accelText)
                    {
		        XtSetArg (args[i], XmNacceleratorText, (XtArgVal)
				  (tmpstr = XmStringCreateLocalized(menuItem->accelText))); i++;
		        sPtr = (StrList *) XtMalloc(sizeof(StrList));
			if (sPtr == NULL)
			  {
			     MWarning(((char *)GETMESSAGE(48, 2, "Insufficient memory for menu %s\n")), menuName);
			     return (NULL);
			  }
			else
			  {
			     sPtr->string = tmpstr;
			     sPtr->next   = stringsToFree;
			     stringsToFree = sPtr;
			  }

                        if ((accelKeySpec = (KeySpec *)
                                 XtMalloc (sizeof (KeySpec ))) == NULL)
	                /* Handle insufficent memory */
                        {
                            MWarning (((char *)GETMESSAGE(48, 6, "Insufficient memory for menu %s\n")),
				      menuName);
                            menuSpec->currentContext &= ~CR_MENU_MARK;
	                    return (NULL);
                        }

		        accelKeySpec->state = menuItem->accelState;
		        accelKeySpec->keycode = menuItem->accelKeyCode;
		        accelKeySpec->context = topMenuSpec->accelContext;
		        accelKeySpec->subContext = 0;
		        accelKeySpec->wmFunction = menuItem->wmFunction;
		        accelKeySpec->wmFuncArgs = menuItem->wmFuncArgs;
		        accelKeySpec->nextKeySpec = topMenuSpec->accelKeySpecs;
                        topMenuSpec->accelKeySpecs = accelKeySpec;
                    }

	            children[n] = XmCreatePushButtonGadget (menuW, 
				      PUSH_BTN_NAME, (ArgList) args, i);

	            /* 
		     * Set sensitivity.  Initially we only consider the context
		     * of the top level menupane.
		     */

	            if (menuItem->greyedContext & topMenuSpec->currentContext)
	            /* insensitive button in this context*/
	            {
	                XtSetSensitive (children[n], FALSE);
	            }
	            else
	            /* sensitive button in this context*/
	            {
	                XtSetSensitive (children[n], TRUE);
	            }

		    /*
		     * If necessary, fill a menuButtons element for this 
		     * PushButton.  Malloc or Realloc the array if necessary.
		     */
                    if ((menuItem->greyedContext) || (menuItem->mgtMask))
                    {
			if (!SavePBInfo (topMenuSpec, menuItem, children[n]))
			{
                            MWarning(((char *)GETMESSAGE(48, 7, "Insufficient memory for menu %s\n")),
				       menuName);
                            menuSpec->currentContext &= ~CR_MENU_MARK;
	                    return (NULL);
                        }
		    }

	            /*
	             * Set up the function callback.
		     * A following title will have both separators.
	             */

	            XtAddCallback (children[n], XmNactivateCallback,
			    (XtCallbackProc)ActivateCallback, 
			    (XtPointer) menuItem);

                    fUseTitleSep = TRUE;
	        }
	    }
	}

	/*
	 * Increment the children array count if we actually
	 * created a new child.
	 */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	if (!labelIsClientCommand)
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	    n++;

	/*
	 * Next menu item:  handle custom items and full children[]. 
	 */
	menuItem = menuItem->nextMenuItem;
        if ((menuItem == NULL) && (moreMenuItems != NULL))
        {
            menuSpec->menuItems = menuItem = moreMenuItems;
	    moreMenuItems = NULL;
        }
	if (n >= CHILDREN_CACHE - 2)  /* leave room for title separators */
	{
            XtManageChildren (children, n);
	    n = 0;
        }
    }

    if (n > 0)
    {
        XtManageChildren (children, n);
    }

    /*
     * Get the initial height of the top level menu pane shell.
     * The actual height will change according to clientFunctions.
     */
    if (fTopLevelPane)
    {
	i = 0;
	XtSetArg (args[i], XtNheight, &menuHeight);  i++;
	XtGetValues (menuW, (ArgList)args, i);
	topMenuSpec->height = (unsigned int) menuHeight;
    }

#ifndef	IBM_151913
    /*
     * Check if the menu that's been created is higher than the screen.
     * If it is, force it to wrap.  Taken straight from the 1.1 fix.
    */

    i = 0;
    XtSetArg (args[i], XtNheight, &menuHeight);  i++;
    XtGetValues (menuW, (ArgList)args, i);
    scr = XtScreen (menuW);
    if (menuHeight > (Dimension)scr->height) {
        i = 0;
        XtSetArg (args[i], XmNresizeHeight, (XtArgVal) FALSE); i++;
        XtSetArg (args[i], XmNpacking, (XtArgVal) XmPACK_TIGHT); i++;
        XtSetArg (args[i], XmNorientation, (XtArgVal) XmVERTICAL); i++;
        XtSetArg (args[i], XmNheight, scr->height); i++;
        XtSetValues (menuW, (ArgList)args, i);
    }
#endif	/* IBM_151913 */

    /* free the string that may have been created earlier. */
    for (sPtr = stringsToFree; sPtr != NULL; )
      {
	 stringsToFree = stringsToFree->next;
	 XmStringFree(sPtr->string);
         XtFree((char *)sPtr);
	 sPtr = stringsToFree;
      }


    /* Unmark the menu specification and return. */
    menuSpec->currentContext &= ~CR_MENU_MARK;
    return (menuW);

} /* END OF FUNCTION CreateMenuWidget */



/*************************************<->*************************************
 *
 *  PostMenu (menuSpec, pCD, x, y, button, newContext, flags, passedInEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to post a menu at a particular location.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =      menu specification
 *  pCD =           client data
 *  x,y =           position to post the menu if (flags & POST_AT_XY) set
 *  button =        button number posting the menu or NoButton (WmGlobal.h) if
 *                  posted by a key
 *  newContext =    context that the menu is to be posted under.
 *  flags =         POST_AT_XY bit set iff x,y are valid, else compute from pCD
 *                  POST_TRAVERSAL_ON bit set if set traversal on
 * 
 *  Outputs:
 *  -------
 *  menuSpec =        menu specification with modifications
 *  wmGD.menuClient = pCD
 *  wmGD.menuActive = menuSpec
 *
 *
 *  Comments:
 *  --------
 *  Accepts x,y only if POST_AT_XY flag bit set.  Otherwise, computes from pCD.
 *  Adjusts PushButton sensitivity according to context and function type.
 *  Manages/Unmanages PushButtons according to clientFunction resource.
 *  Sets traversal on if button==NoButton or POST_TRAVERSAL_ON flag bit set.
 * 
 *************************************<->***********************************/

void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, unsigned int button, Context newContext, long flags, XEvent *passedInEvent)
{
    int              i;
    Arg              args[3];
    unsigned int     whichButton;
    Dimension        menuHeight;
    XButtonPressedEvent event;
    Window           saveWindow;
    Display          *saveDisplay;
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    Boolean          menuAdjusted;
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    
    if ((menuSpec == NULL) || (menuSpec->menuWidget == NULL))
    {
	return;
    }


    /* 
     * Don't post a menu from an icon in the iconbox if the
     * icon is not visible
     */
    if((newContext == F_SUBCONTEXT_IB_WICON ||
       newContext == F_SUBCONTEXT_IB_IICON) &&
       (!(IconVisible(pCD))))
    {
	return;
    }

    /*
     * Set grabContext to be used in GrabWin when no event is passed
     * to GrabWin. 
     */

    wmGD.grabContext = newContext;

    /*
     *  Adjust PushButton sensitivity and manage/unmanage status.
     *  If the manage status of the system menu has changed, 
     *  then get the height of the top level menu pane shell and
     *  cache it in its MenuSpec.
     * 
     *  Also...
     *  Adjust the tear off control. If we are posting this menu from
     *  a client then force the tear off to be disabled. NOTE: This must
     *  be done after wmGD.menuClient has been set.
     *  Since turning off the tear-off control could result in a height
     *  change, we may need to remeasure things. (CR 9316)
     */
    
#ifdef WSM
    if(pCD && pCD->clientFlags & ICON_BOX)
    {
        newContext |= F_CONTEXT_ICONBOX;
    }

#endif /* WSM */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
    menuAdjusted =
      AdjustTearOffControl(NULL, (XtPointer) (menuSpec->menuWidget), NULL);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
    if (AdjustPBs (menuSpec, pCD, newContext)
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
	|| menuAdjusted
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
	)
    {
        i = 0;
        XtSetArg (args[i], XtNheight, &menuHeight);  i++;
        XtGetValues (menuSpec->menuWidget, (ArgList)args, i);
        menuSpec->height = (unsigned int) menuHeight;
    }
    menuSpec->currentContext = newContext;

    /*
     *  Adjust the whichButton resource if necessary.
     *  Use SELECT_BUTTON for NoButton.
     */

    whichButton = (button == NoButton) ? SELECT_BUTTON : button;
    if (whichButton != menuSpec->whichButton)
    {
        i = 0;
        XtSetArg (args[i], XmNwhichButton, (XtArgVal) whichButton); i++;
        XtSetValues (menuSpec->menuWidget, args, i);
        menuSpec->whichButton = whichButton;
    }

    /*
     *  Determine the position of the popup menu.
     *  Compute position if necessary (system menu).
     */

    if (!(flags & POST_AT_XY))
    /* compute the position */
    {
	GetSystemMenuPosition (pCD, &x, &y, menuSpec->height, newContext);
    }

    event.x_root = x;
    event.y_root = y;
    XmMenuPosition (menuSpec->menuWidget, &event);

    wmGD.menuClient = pCD;
    wmGD.menuActive = menuSpec;   /* set to NULL within UnmapCallback() */

    /* 
     * Post the menu by managing its top-level RowColumn.
     *
     * First dispatch the event to set the time stamp in the toolkit
     */

    if(passedInEvent)
    {
	saveWindow = passedInEvent->xany.window;
	saveDisplay = passedInEvent->xany.display;
	passedInEvent->xany.window = 0;
	passedInEvent->xany.display = XtDisplay(menuSpec->menuWidget);

	XtDispatchEvent(passedInEvent);
	passedInEvent->xany.window = saveWindow;
	passedInEvent->xany.display = saveDisplay;

	/* If menu posted by ButtonPress/ButtonRelease, release grabs. */
	if ((passedInEvent->type == ButtonPress) ||
	    (passedInEvent->type == ButtonRelease))
	    XUngrabPointer(passedInEvent->xany.display,
			   passedInEvent->xbutton.time);
    }
    
#ifndef ALTERNATE_POSTMENU

    XtManageChild (menuSpec->menuWidget);

#else
    if (flags & POST_STICKY)
    {
	_XmSetPopupMenuClick(menuSpec->menuWidget, True);
    }
    else
    {
	_XmSetPopupMenuClick(menuSpec->menuWidget, False);
    }

    /* 
     * Post the menu by calling the convenience routine that verifies
     * the button event, updates the Xt timestamp, and finally manages
     * the pane.
     */

    _XmPostPopupMenu( menuSpec->menuWidget, passedInEvent);
#endif


    /*
     *  set the traversal state.
     */

    if ((button == NoButton) || (flags & POST_TRAVERSAL_ON))
    /* turn traversal on */
    {
	TraversalOn (menuSpec);
    }
    else
    /* turn traversal off */
    {
	TraversalOff (menuSpec);
    }

} /* END OF FUNCTION PostMenu */



/*************************************<->*************************************
 *
 *  UnpostMenu (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to unpost a menu.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =      menu specification
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  wmGD.menuActive and wmGD.menuUnpostKey are set to NULL within 
 *  UnmapCallback.
 * 
 *************************************<->***********************************/

void UnpostMenu (MenuSpec *menuSpec)
{
    if (menuSpec && (menuSpec->menuWidget))
    /* 
     * Unpost the menu by unmanaging its top-level RowColumn.
     */
    {
        XtUnmanageChild (menuSpec->menuWidget);
#ifndef OLD_COLORMAP
        ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    }

} /* END OF FUNCTION UnpostMenu */



/*************************************<->*************************************
 *
 *  ActivateCallback (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  This function is called whenever a menu item is selected.
 *
 *
 *  Inputs:
 *  ------
 *  w =               menubuttonWidget
 *  client_data =     pointer to menu button's MenuItem structure
 *  call_data =       not used
 *  wmGD.menuClient = pointer to client's ClientData structure
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void ActivateCallback (Widget w, caddr_t client_data, caddr_t call_data)
{
    WmScreenData *pSD;

    /* set active screen */
    pSD = GetScreenForWindow (XtWindow(w));
    if (pSD) SetActiveScreen (pSD);

    ((MenuItem *)client_data)->wmFunction (
		((MenuItem *)client_data)->wmFuncArgs, wmGD.menuClient, NULL);

} /* END OF FUNCTION ActivateCallback */



/*************************************<->*************************************
 *
 *  UnmapCallback (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  This function is called whenever a toplevel RowColumn is unmapped.
 *
 *
 *  Inputs:
 *  ------
 *  w =
 *  client_data =       not used
 *  call_data =         not used
 *  wmGD.gadgetClient = last client with depressed client
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.menuActive = NULL
 *  wmGD.menuUnpostKeySpec = NULL
 *  wmGD.checkHotspot = FALSE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void UnmapCallback (Widget w, XtPointer client_data,
			   XtPointer call_data)
{
    wmGD.menuActive = NULL;
    wmGD.menuUnpostKeySpec = NULL;
    wmGD.checkHotspot = FALSE;

    if (wmGD.gadgetClient) 
    {
	PopGadgetOut(wmGD.gadgetClient, FRAME_SYSTEM);
    }

#ifndef OLD_COLORMAP
    ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    PullExposureEvents();

} /* END OF FUNCTION UnmapCallback */


/*************************************<->*************************************
 *
 *  MWarning (message)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a message to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  format  = pointer to a format string
 *  message = pointer to a message string
 * 
 *************************************<->***********************************/

void MWarning (char *format, char *message)
{

    if (strlen(format) + strlen(message)  <  (size_t) MAXWMPATH)
      {
	 char pch[MAXWMPATH+1];

	 sprintf (pch, format, message);
	 Warning (pch);
      }

} /* END OF FUNCTION MWarning */



#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
/*************************************<->*************************************
 *
 *  UnmapPulldownCallback (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
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
 * 
 *************************************<->***********************************/

static void UnmapPulldownCallback (Widget w, XtPointer client_data,
				   XtPointer call_data)
{
    wmGD.menuActive = (MenuSpec *) NULL;
} /* END OF FUNCTION UnmapPulldownCallback */
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */


/*************************************<->*************************************
 *
 *  TraversalOff (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function turns menu traversal off.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = menu specification
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void TraversalOff (MenuSpec *menuSpec)
{
    if (menuSpec && (menuSpec->menuWidget))
    {
	/* function pointer */
	(*((XmRowColumnWidgetClass) XtClass (menuSpec->menuWidget))
				       ->row_column_class.menuProcedures) 
		/* argument list */
	       (XmMENU_TRAVERSAL, menuSpec->menuWidget, False, NULL, NULL);
    }

} /* END OF FUNCTION TraversalOff */



/*************************************<->*************************************
 *
 *  TraversalOn (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function turns menu traversal on.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = menu specification
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void TraversalOn (MenuSpec *menuSpec)
{

    if (menuSpec && (menuSpec->menuWidget))
    {
	/* function pointer */
	(*((XmRowColumnWidgetClass) XtClass (menuSpec->menuWidget))
				   ->row_column_class.menuProcedures) 
		/*argument list */
	       (XmMENU_TRAVERSAL, menuSpec->menuWidget, True, NULL, NULL);
    }

} /* END OF FUNCTION TraversalOn */



/*************************************<->*************************************
 *
 *  FreeCustomMenuSpec (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This procedure destroys a custom MenuSpec structure and its associated 
 *  menu widget, menuItems list, menuButtons array, and menu accelerator list.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = MenuSpec structure
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  Assumes that a MenuSpec is custom iff its name is NULL.
 *
 *  Assumes that ParseWmFuncStr() has parsed a menu item's function
 *  argument only for F_Exec and F_Menu.  If it is used for other functions,
 *  be sure to include them here!
 * 
 *************************************<->***********************************/

void FreeCustomMenuSpec (MenuSpec *menuSpec)
{
    MenuItem    *menuItem;
    MenuItem    *nextMenuItem;
    KeySpec     *accelKeySpec;
    KeySpec     *nextAccelKeySpec;

    /* Fix memory leak: use menuSpec->clientLocal to identify
       custom menus instead of menuSpec->name */

    if ((menuSpec == NULL) || (menuSpec->clientLocal == FALSE))
    /* we only destroy custom menus! */
    {
	return;
    }
  
    /*
     * Fix for CR 5450 - If the custom menu is the same as wmGD.menuActive, call
     *                   the UnmapCallback directly to clean things up.  Since
     *                   the menu is going to be destroyed, this callback will
     *                   not get called, leaving MWM in a failure state.
     */
     if (wmGD.menuActive == menuSpec)
       UnmapCallback((Widget)NULL, (caddr_t)NULL, (caddr_t)NULL);
    /*
     * End fix for CR 5450
     */
 
    if (menuSpec->name != NULL)
        XtFree(menuSpec->name);

    menuItem = menuSpec->menuItems;
    while (menuItem)
    {
	nextMenuItem = menuItem->nextMenuItem;
        FreeMenuItem (menuItem);
	menuItem = nextMenuItem;
    }

    if (menuSpec->menuButtons)
    {
        XtFree ((char *)menuSpec->menuButtons);
    }

    accelKeySpec = menuSpec->accelKeySpecs;
    while (accelKeySpec)
    {
	nextAccelKeySpec = accelKeySpec->nextKeySpec;
	XtFree ((char *)accelKeySpec);
	accelKeySpec = nextAccelKeySpec;
    }

    if (menuSpec->menuWidget)
    /* destroy all children of the menu's MenuShell parent */
    {
        XtDestroyWidget (XtParent(menuSpec->menuWidget));
    }

    XtFree ((char *)menuSpec);

} /* END OF FUNCTION FreeCustomMenuSpec */


