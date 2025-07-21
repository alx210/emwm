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
#include "WmCEvent.h"
#include "WmResource.h"
#include "WmResParse.h"
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
#define TITLE_NAME "menuTitle"
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
#include "WmXinerama.h"
#include "WmWrkspace.h"


static void UnmapCallback (Widget w, XtPointer client_data,
			   XtPointer call_data);
static MenuItem *DuplicateMenuItems (MenuItem *menuItems);
static void AdjustMenuPosition(int *x, int *y,
	unsigned int width, unsigned int height);
static void CheckTerminalSeparator(MenuSpec *menuSpec,
	Widget buttonWidget, Boolean manage);


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
		if ((newMenuSpec = (MenuSpec *) XtMalloc (sizeof (MenuSpec))) == NULL)
		/* Handle insufficent memory */
		{
   			MWarning(((char *)GETMESSAGE(48, 2, "Insufficient memory for menu %s\n")), menuName);
		return (NULL);
		}
		newMenuSpec->name = NULL;  /* distinguishes this as custom */
		newMenuSpec->whichButton = SELECT_BUTTON;
		newMenuSpec->height = 0;
		newMenuSpec->width = 0;
		newMenuSpec->menuItems = menuSpec->menuItems; /* temporary */
		newMenuSpec->accelContext = menuSpec->accelContext;
		newMenuSpec->accelKeySpecs = NULL;
		newMenuSpec->nextMenuSpec = NULL;

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

    menuSpec->menuWidget = CreateMenuWidget (pSD, menuName, 
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
static void CheckTerminalSeparator(MenuSpec *menuSpec,
	Widget buttonWidget, Boolean manage)
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
    MenuItem *newMenuItem = (MenuItem *) NULL, *returnMenuItem = NULL, *curMenuItem;
    
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
	    || (curMenuItem->wmFunction == F_Set_Context)
	    )
	  newMenuItem->wmFuncArgs = curMenuItem->wmFuncArgs;
	else if (curMenuItem->wmFuncArgs != (String) NULL)
	  newMenuItem->wmFuncArgs = XtNewString(curMenuItem->wmFuncArgs);
	else
	  newMenuItem->wmFuncArgs = NULL;

	newMenuItem->greyedContext = curMenuItem->greyedContext;
	newMenuItem->mgtMask = curMenuItem->mgtMask;
	newMenuItem->nextMenuItem = (MenuItem *) NULL;
    }
    
    return(returnMenuItem);
}

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
	newMenuSpec->width = menuSpec->width;
    newMenuSpec->height = menuSpec->height;
    newMenuSpec->menuItems = DuplicateMenuItems(menuSpec->menuItems);
    newMenuSpec->menuButtons = (MenuButton *) NULL;
    newMenuSpec->menuButtonSize = 0;
    newMenuSpec->menuButtonCount = 0;
    newMenuSpec->accelContext = menuSpec->accelContext;
    newMenuSpec->accelKeySpecs = (KeySpec *) NULL;
    newMenuSpec->nextMenuSpec = (MenuSpec *) NULL;
    
    return(newMenuSpec);
}

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

	    if (menuItem->wmFunction == F_Remove)
	    {
		/*
		 * Only allow remove from workspace if the client
		 * is in more than one workspace
		 */
		fSupported = (pCD && (pCD->numInhabited > 1));
		XtSetSensitive (menuButton->buttonWidget, fSupported);
	    }


	    if ((menuItem->mgtMask) && pCD)
	    /* PushButton might not apply */
	    {
	        if ((pCD->clientFunctions & menuItem->mgtMask & MWM_MGT_MASK) ||
		    (pCD->wsmFunctions & menuItem->mgtMask & WSM_MGT_MASK))
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
	Dimension   menuWidth;
    Boolean     fUseTitleSep = False;
    StrList    *stringsToFree = NULL, *sPtr;
    XmString    tmpstr;
    Screen     *scr;


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

	if (menuItem->wmFunction == F_Separator)
	/* 
	 * Add a Separator gadget for a menu separator.
	 * An immediately following title will not have a top separator.
	 */
	{
		children[n] =
		  XmCreateSeparatorGadget (menuW, SEPARATOR_NAME, 
					   (ArgList)NULL, 0);
		fUseTitleSep = FALSE;

	} /* F_Separator */

	else
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
		    subMenuW = CreateMenuWidget(pSD,
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
	XtSetArg(args[i], XtNwidth, &menuWidth); i++;
	XtSetArg(args[i], XtNheight, &menuHeight);  i++;
	XtGetValues (menuW, (ArgList)args, i);
	topMenuSpec->height = (unsigned int) menuHeight;
	topMenuSpec->width = (unsigned int) menuWidth;
    }

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
 *  flags =         POST_AT_XY bit set if x,y are valid, else compute from pCD
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
	Dimension menuWidth;
    Dimension menuHeight;
    XButtonPressedEvent event;
    Window           saveWindow;
    Display          *saveDisplay;
    
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
    
    if(pCD && pCD->clientFlags & ICON_BOX)
    {
        newContext |= F_CONTEXT_ICONBOX;
    }

    if (AdjustPBs (menuSpec, pCD, newContext))
    {
        i = 0;
		XtSetArg (args[i], XtNwidth, &menuWidth); i++;
        XtSetArg (args[i], XtNheight, &menuHeight);  i++;
        XtGetValues (menuSpec->menuWidget, (ArgList)args, i);
        menuSpec->height = (unsigned int) menuHeight;
		menuSpec->width = (unsigned int) menuWidth;
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
    if (flags & POST_AT_XY) {
		AdjustMenuPosition(&x, &y, menuSpec->width, menuSpec->height);
    } else {
		GetSystemMenuPosition (pCD, &x, &y, menuSpec->width,
			menuSpec->height, newContext);
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
        ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
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

    ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
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

    if ((menuSpec == NULL) || (menuSpec->name != NULL))
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

/*************************************<->*************************************
 *
 * Adjusts menu coordinates so that it doesn't appear off-screen, or
 * split across Xinerama screns.
 *
 *************************************<->***********************************/
static void AdjustMenuPosition(int *x, int *y,
	unsigned int width, unsigned int height)
{
	XineramaScreenInfo xsi;

	if(!GetXineramaScreenFromLocation(*x, *y, &xsi)){
		xsi.x_org = 0;
		xsi.y_org = 0;
		xsi.width = XDisplayWidth(DISPLAY, ACTIVE_SCREEN);
		xsi.height = XDisplayHeight(DISPLAY, ACTIVE_SCREEN);
	}
	
	if((*x + width) >= (xsi.x_org + xsi.width)) {
		*x -= ((*x + width) - (xsi.x_org + xsi.width));
	}
	if((*y + height) >= (xsi.y_org + xsi.height)) {
		*y -= ((*y + height) - (xsi.y_org + xsi.height));
	}
}

