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
 * Motif Release 1.2.4
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$TOG: WmIconBox.c /main/7 1999/05/20 16:35:12 mgreess $"
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
#ifdef WSM
#include "WmHelp.h"
#endif /* WSM */
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xutil.h>
#include <X11/Vendor.h>

#include <X11/keysymdef.h>
#include <X11/keysym.h>


#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/DrawnB.h>
#include <Xm/ScrolledW.h>
#include <Xm/BulletinB.h>
#include <Xm/ToggleB.h>

#define MWM_NEED_IIMAGE
#define MWM_NEED_GREYED75
#define MWM_NEED_SLANT2
#include "WmIBitmap.h"

#include "WmResNames.h"

#include <stdio.h>

/*
 * include extern functions
 */

#include "WmIconBox.h"
#include "WmCDInfo.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmImage.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmMenu.h"
#include "WmResParse.h"
#include "WmResource.h"
#include "WmWinInfo.h"
#ifdef WSM
#include "WmWrkspace.h"
#endif /* WSM */

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif



/*
 * Global Variables:
 */


Pixel select_color;
Pixmap greyedPixmap;

int frameShadowThickness;
int firstTime = 1;
Cardinal insertPosition = 0;
#define    DEFAULT_ICON_BOX_TITLE "Icons"
Const char *szhorizontal = "horizontal";
Const char *szvertical = "vertical";


/*************************************<->*************************************
 *
 *  InitIconBox (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function controls creation of icon boxes
 *
 *
 *************************************<->***********************************/
void InitIconBox (WmScreenData *pSD)

{
#ifdef WSM
    int iws;
#endif /* WSM */
    /*
     * Start the process of making the icon boxes
     */



#ifdef WSM
    /*
     * Manage a separate icon box in every workspace
     * on this screen.
     */
    for (iws = 0; iws < pSD->numWorkspaces; iws++)
    {
	AddIconBoxForWorkspace (&pSD->pWS[iws]);
    }

#else /* WSM */
    ManageWindow (pSD, None, MANAGEW_ICON_BOX);
#endif /* WSM */

    if (pSD->fadeNormalIcon)
    {
	MakeFadeIconGC (pSD);
    }


} /* END OF FUNCTION InitIconBox */

#ifdef WSM

/*************************************<->*************************************
 *
 *  AddIconBoxForWorkspace (pWS)
 *
 *
 *  Description:
 *  -----------
 *  This function adds an iconbox to a workspace
 *
 *
 *************************************<->***********************************/
void AddIconBoxForWorkspace (WmWorkspaceData *pWS)

{
    extern WmWorkspaceData *pIconBoxInitialWS;

    pIconBoxInitialWS = pWS;
    ManageWindow (pWS->pSD, NULL, MANAGEW_ICON_BOX);

} /* END OF FUNCTION AddIconBoxForWorkspace */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  MakeIconBox (pWS, pCD);
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS     = pointer to workspace data
 *  pCD     =  a pointer to ClientData
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  Return =  (Boolean) True iff successful.
 *
 *
 *  Comments:
 *  --------
 *  If fails, frees the ClientData structure pointed to by pCD.
 * 
 *************************************<->***********************************/

Boolean MakeIconBox (WmWorkspaceData *pWS, ClientData *pCD)
{
    IconBoxData *pIBD;


    /* 
     * Make an icon box and return the pCD
     */

    if (pCD)
    {
        if (!(pIBD = (IconBoxData *)XtMalloc (sizeof (IconBoxData))))
	{
	    /*
	     * We need a pointer to icon box data to add to the
	     * list of icon boxes linked to pWS->pIconBox. If
	     * we can't allocate space we need to free the space
	     * allocated for the ClientData structure 
	     */

	    Warning (((char *)GETMESSAGE(36, 1, "Insufficient memory to create icon box data")));
            XtFree ((char *)pCD);  
	    return (FALSE);  
	}

	InitializeIconBoxData (pWS, pIBD);
	InitializeClientData (pCD, pIBD);

        if (!(pIBD->IPD.placeList = 
	    (IconInfo *)XtMalloc (pIBD->IPD.totalPlaces * sizeof (IconInfo))))
	{
	    Warning (((char *)GETMESSAGE(36, 2, "Insufficient memory to create icon box data")));
	    XtFree ((char *)pIBD);
            XtFree ((char *)pCD);  
	    return (FALSE);  
	}
	memset (pIBD->IPD.placeList, 0, 
	    pIBD->IPD.totalPlaces * sizeof (IconInfo));

        /*
         * Make  the top level shell for this icon box
         */
        MakeShell (pWS, pIBD);

        /*
         * Make  the scrolled window for this icon box
         */

        MakeScrolledWindow (pWS, pIBD);

        /*
         * Make  the row column manager for this icon box
         */
        MakeBulletinBoard (pWS, pIBD);

        /*
         * Realize the widget tree and set client data fields
         */

        RealizeIconBox (pWS, pIBD, pCD);

	/*
	 * Link the new icon box to list of icon boxes
	 */
	AddNewBox (pWS, pIBD);
    }
    
    return (TRUE);  

} /* END OF FUNCTION MakeIconBox */

#ifdef WSM

/*************************************<->*************************************
 *
 *  DestroyIconBox (pWS)
 *
 *
 *  Description:
 *  -----------
 *  Destroys an icon box 
 *
 *
 *  Inputs:
 *  ------
 *  pWS     = pointer to workspace data
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  Return =  none
 *
 *
 *  Comments:
 *  --------
 *  Used when deleting a workspace
 *  Should be called AFTER all clients have been removed from the 
 *  workspace -- there should be no icons in the icon box.
 * 
 *************************************<->***********************************/

void DestroyIconBox (WmWorkspaceData *pWS)
{
    IconBoxData *pIBD;

    pIBD = pWS->pIconBox;

    XtDestroyWidget (pIBD->shellWidget);

    UnManageWindow (pIBD->pCD_iconBox);

    XtFree ((char *) pIBD);
   
} /* END OF FUNCTION DestroyIconBox */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  MakeShell (pWS, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 *  pIBD  = pointer to IconBoxData
 *
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  pIBD->shellWidget 
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void MakeShell (WmWorkspaceData *pWS, IconBoxData *pIBD)
{

    Arg setArgs[20];
    int i;
#ifdef WSM
    char *pchIBTitle = NULL;
#endif /* WSM */

    /*
     * Create top level application shell for icon box
     */

    i=0;

    XtSetArg (setArgs[i], XmNallowShellResize, (XtArgVal)True); i++; 
    
    XtSetArg (setArgs[i], XmNborderWidth, (XtArgVal)0); i++; 

    XtSetArg (setArgs[i], XmNkeyboardFocusPolicy, (XtArgVal)XmEXPLICIT); i++;

#ifndef WSM
    if (!(Monochrome (XtScreen (pWS->pSD->screenTopLevelW))))
    {
	XtSetArg (setArgs[i], XmNbackground,  
		  (XtArgVal) pWS->pSD->clientAppearance.background ); i++;
	XtSetArg (setArgs[i], XmNforeground,  
		  (XtArgVal) pWS->pSD->clientAppearance.foreground ); i++;
    }
#else  /* WSM  */
    if (pWS->pSD->iconBoxTitle)
    {
	pchIBTitle = WmXmStringToString (pWS->pSD->iconBoxTitle);

	XtSetArg (setArgs[i], XmNtitle, (XtArgVal)pchIBTitle); i++;
	XtSetArg (setArgs[i], XmNiconName, (XtArgVal)pchIBTitle); i++;
    }
#endif /* WSM */
    XtSetArg (setArgs[i], XmNmappedWhenManaged, (XtArgVal)False); i++;
    XtSetArg (setArgs[i], XmNdialogStyle, (XtArgVal)XmDIALOG_MODELESS); i++;
    XtSetArg (setArgs[i], XmNdepth, 
	(XtArgVal) DefaultDepth (DISPLAY, pWS->pSD->screen)); i++;
    XtSetArg (setArgs[i], XmNscreen, 
	(XtArgVal) ScreenOfDisplay (DISPLAY, pWS->pSD->screen)); i++;

#ifdef WSM
    pIBD->shellWidget = (Widget) XtCreatePopupShell (WmNclient, 
					topLevelShellWidgetClass,
                                        pWS->workspaceTopLevelW,
				        (ArgList)setArgs, i);

    if (pchIBTitle != NULL) XtFree (pchIBTitle);
#else /* WSM */
    pIBD->shellWidget = (Widget) XtCreatePopupShell (WmNiconBox, 
					topLevelShellWidgetClass,
                                        pWS->workspaceTopLevelW,
				        (ArgList)setArgs, i);
#endif /* WSM */

} /* END OF FUNCTION MakeShell */



/*************************************<->*************************************
 *
 *  MakeScrolledWindow (pWS, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS	= pointer to workspace data
 *  pIBD  = pointer to IconBoxData
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  Return =  pIBD with the  pIBD->scrolledWidget set
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void MakeScrolledWindow (WmWorkspaceData *pWS, IconBoxData *pIBD)
{

    Arg setArgs[20]; 
    int i;

    /*
     * Create frame widget to give the scrolled window 
     * an external bevel
     */

    i=0;
#ifndef WSM
/*
    if (!(Monochrome (XtScreen (pWS->pSD->screenTopLevelW))))
    {
	XtSetArg (setArgs[i], XmNbackground,  
		  (XtArgVal) pWS->pSD->clientAppearance.background ); i++;
	XtSetArg (setArgs[i], XmNforeground,  
		  (XtArgVal) pWS->pSD->clientAppearance.foreground ); i++;
    }
*/
#endif /* WSM */
    XtSetArg (setArgs[i], XmNborderWidth,  (XtArgVal) 0 ); i++;
    XtSetArg (setArgs[i], XmNmarginWidth,  (XtArgVal) 0 ); i++;
    XtSetArg (setArgs[i], XmNmarginHeight, (XtArgVal) 0 ); i++;
    XtSetArg (setArgs[i], XmNshadowType, (XtArgVal) XmSHADOW_OUT); i++;
    XtSetArg (setArgs[i], XmNshadowThickness,
			(XtArgVal) frameShadowThickness); i++;
    pIBD->frameWidget = XtCreateManagedWidget ("IBframe", 
					xmFrameWidgetClass, 
					pIBD->shellWidget,
					(ArgList)setArgs, i);

#ifdef WSM
    XtAddCallback (pIBD->frameWidget, XmNhelpCallback,
                   WmDtWmTopicHelpCB, WM_DT_ICONBOX_TOPIC);

#endif /* WSM */
    /*
     * Create scrolled window to hold row column manager 
     */

    i=0;

    XtSetArg (setArgs[i], XmNscrollingPolicy , (XtArgVal) XmAUTOMATIC ); i++;

    XtSetArg (setArgs[i], XmNborderWidth , (XtArgVal) 0 ); i++;
    XtSetArg (setArgs[i], XmNspacing , (XtArgVal) IB_MARGIN_WIDTH ); i++;
#ifndef WSM

    if (!(Monochrome (XtScreen (pWS->pSD->screenTopLevelW))))
    {
	XtSetArg (setArgs[i], XmNbackground,  
		  (XtArgVal) pWS->pSD->clientAppearance.background ); i++;
	XtSetArg (setArgs[i], XmNforeground,  
		  (XtArgVal) pWS->pSD->clientAppearance.foreground ); i++;
    }
#endif /* WSM */
    /*
     * do we want to get these from a resource or set it here
     * to control the appearance of the iconBox
     */

    XtSetArg (setArgs[i], XmNscrolledWindowMarginWidth, (XtArgVal) 3); i++;
    XtSetArg (setArgs[i], XmNscrolledWindowMarginHeight, (XtArgVal) 3); i++;
    XtSetArg (setArgs[i], XmNshadowThickness,
			(XtArgVal) FRAME_EXTERNAL_SHADOW_WIDTH); i++;

    XtSetArg (setArgs[i], XmNscrollBarDisplayPolicy,(XtArgVal) XmSTATIC ); i++;
    XtSetArg (setArgs[i], XmNvisualPolicy, (XtArgVal) XmVARIABLE ); i++;

    pIBD->scrolledWidget = XtCreateManagedWidget ("IBsWindow", 
					xmScrolledWindowWidgetClass, 
					pIBD->frameWidget,
					(ArgList)setArgs, i);

#ifndef MOTIF_ONE_DOT_ONE
    XtAddCallback(pIBD->scrolledWidget, XmNtraverseObscuredCallback,
		  (XtCallbackProc) IconScrollVisibleCallback, (caddr_t)NULL);
#endif

    XtAddEventHandler(pIBD->scrolledWidget, 
			StructureNotifyMask, 
			False, 
			(XtEventHandler)UpdateIncrements, 
			(XtPointer) pIBD);



} /* END OF FUNCTION MakeScrolledWindow */



/*************************************<->*************************************
 *
 *  MakeBulletinBoard (pWS, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pIBD  = pointer to IconBoxData
 *
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  Return =  pIBD with the  pIBD->bBoardWidget
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void MakeBulletinBoard (WmWorkspaceData *pWS, IconBoxData *pIBD)
{

    int i;
    Arg setArgs[20];

    /*
     * Create bulletin board to hold icons
     */

    i=0;
#ifdef DEBUG_ICON_BOX
    XtSetArg (setArgs[i], XmNborderWidth , 1); i++; 
#else
    XtSetArg (setArgs[i], XmNborderWidth , 0); i++; 
#endif /* DEBUG_ICON_BOX */
    
    XtSetArg (setArgs[i], XmNshadowThickness,(XtArgVal) 0); i++;
#ifndef WSM
    if (!(Monochrome (XtScreen (pWS->pSD->screenTopLevelW))))
    {
	XtSetArg (setArgs[i], XmNforeground,  
		  (XtArgVal) pWS->pSD->clientAppearance.background ); i++;
	XtSetArg (setArgs[i], XmNbottomShadowColor,  
		(XtArgVal) pWS->pSD->clientAppearance.bottomShadowColor ); i++;
	XtSetArg (setArgs[i], XmNtopShadowColor,  
		  (XtArgVal) pWS->pSD->clientAppearance.topShadowColor ); i++;
    }
#endif /* WSM */

    XtSetArg (setArgs[i], XmNspacing , 0); i++; 
    XtSetArg (setArgs[i], XmNmarginHeight , 0); i++;
    XtSetArg (setArgs[i], XmNmarginWidth ,  0); i++;

    XtSetArg (setArgs[i], XmNdialogStyle, (XtArgVal) XmDIALOG_WORK_AREA); i++;

    XtSetArg (setArgs[i], XmNresizePolicy, (XtArgVal) XmRESIZE_NONE); i++;
    XtSetArg (setArgs[i], XmNdefaultPosition , (XtArgVal) False); i++;

    XtSetArg (setArgs[i], XtNinsertPosition , InsertPosition); i++;

    pIBD->bBoardWidget = XtCreateManagedWidget ("IBbBoard", 
					xmBulletinBoardWidgetClass,
					pIBD->scrolledWidget,
					(ArgList)setArgs, i);

} /* END OF FUNCTION MakeBulletinBoard */



/*************************************<->*************************************
 *
 *  RealizeIconBox (pWS, pIBD, pCD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data 
 *
 *  pIBD  = pointer to IconBoxData
 *
 *  pCD   = pointer to ClientData
 *
 * 
 *  Outputs:
 *  -------
 *  

 *  Return =  pIBD with the  pIBD->shellWin set
 *  Return =  pIBD with the  pIBD->scrolledWin set
 *  Return =  pIBD with the  pIBD->bBoardWin set

 *
 *  Return =  pCD  with appropriate fields set
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void RealizeIconBox (WmWorkspaceData *pWS, IconBoxData *pIBD, ClientData *pCD)
{

    int i;
    Arg getArgs[10]; 
    Arg setArgs[2]; 
    Widget clipWidget;
    Pixmap  bgPixmap;
    Pixmap defaultImage;
    

    XtRealizeWidget (pIBD->shellWidget);

    pCD->client = XtWindow (pIBD->shellWidget);

    /*
     * This will set the scrolling granularity for the icon box
     */

    SetGeometry (pWS, pCD, pIBD);

    /*
     * Point to the iconBox 
     */

    pIBD->pCD_iconBox = pCD;
    pCD->thisIconBox = pIBD;    
    /*
     * get the background color of the bulletin board for
     * greyed icon work
     */

    i=0;
    XtSetArg (setArgs[i], XmNbackground, (XtArgVal) select_color ); i++;
    XtSetValues (pIBD->bBoardWidget, (ArgList) setArgs, i); 


    i=0;
    XtSetArg (getArgs[i], XmNbackgroundPixmap, (XtArgVal) &bgPixmap ); i++;
    XtGetValues (pIBD->bBoardWidget, getArgs, i);

    i=0;
    XtSetArg (getArgs[i], XmNclipWindow, (XtArgVal) &clipWidget ); i++;
    XtGetValues (pIBD->scrolledWidget, getArgs, i);

    /*
     * Set the background of the clip window for the scrolled 
     * window so the default widget background doesn't flash
     */

    i = 0;
    XtSetArg(setArgs[i], XmNbackground,	(XtArgVal) select_color); i++;
    XtSetValues (clipWidget, (ArgList) setArgs, i); 


    /*
     * Save the clipWidget id to use in constraining icon moves in box
     */

    pIBD->clipWidget = clipWidget; 

    MakeShrinkWrapIconsGC (pWS->pSD, bgPixmap);

    
    if (pWS->pSD->iconDecoration & ICON_IMAGE_PART)
    {
        /*
         * Make a pixmap to use when iconWindows are unmapped
         */
        defaultImage = XCreateBitmapFromData (DISPLAY, pWS->pSD->rootWindow,
					      (char*)iImage_bits, iImage_width, 
					      iImage_height);
	
        pWS->pSD->defaultPixmap = MakeIconPixmap (pCD,
                                                  defaultImage,
                                                  None, iImage_width,
                                                  iImage_height, 1);
    }
    
    
} /* END OF FUNCTION RealizeIconBox */



/*************************************<->*************************************
 *
 *  AddNewBox (pWS, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 *  pIBD  = pointer to IconBoxData
 *
 * 
 *  Outputs:
 *  -------
 *  
 *
 *  Comments:
 *  --------
 *  Finds the last iconbox on the list starting at pWS->pIconBox and
 *  adds the new icon box to the end of the list.
 * 
 *************************************<->***********************************/

void AddNewBox (WmWorkspaceData *pWS, IconBoxData *pIBD)
{
 
    IconBoxData *pibd;

    if (pWS->pIconBox)
    {
	pibd = pWS->pIconBox;

        while (pibd->pNextIconBox)
        {
	    pibd = pibd->pNextIconBox;
        }

        pibd->pNextIconBox = pIBD;
    }
    else
    {
	pWS->pIconBox = pIBD;
    }

  
} /* END OF FUNCTION AddNewBox */



/*************************************<->*************************************
 *
 *  InitializeIconBoxData (pWS, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to Workspace Data
 *
 *  pIBD  = pointer to IconBoxData
 *
 * 
 *  Outputs:
 *  -------
 *  
 *
 *  Comments:
 *  --------
 *  Initializes all pIBD fields to NULL
 * 
 *************************************<->***********************************/

void InitializeIconBoxData (WmWorkspaceData *pWS, IconBoxData *pIBD)
{
    int mask;
    int X;
    int Y;
    unsigned int width;
    unsigned int height;
    int sW, sH;

    frameShadowThickness = FRAME_INTERNAL_SHADOW_WIDTH;

    pIBD->numberOfIcons = 0;
    pIBD->currentRow = 0;
    pIBD->currentCol = 0;
    pIBD->lastRow = 0;
    pIBD->lastCol = 0;
    pIBD->IPD.placeList = NULL;

    pIBD->scrolledWidget = NULL;
    pIBD->bBoardWidget = NULL;
    pIBD->clipWidget = NULL; 
#ifdef WSM
    pIBD->wsID = pWS->id;
#endif /* WSM */

    ToLower ((unsigned char *) pWS->pSD->iconBoxSBDisplayPolicy);
    
    if (!((!strcmp(pWS->pSD->iconBoxSBDisplayPolicy , "all"))      ||
	  (!strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szvertical)) ||
	  (!strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szhorizontal))))
    {
	strcpy(pWS->pSD->iconBoxSBDisplayPolicy, "all");
    }

	

    /*
     * this will be set by the iconPlacement resource if
     * iconBoxGeometry width and height are not specified
     */

#ifdef WSM
    if (pWS->iconBoxGeometry == NULL) /* not set by user */
#else /* WSM */
    if (pWS->pSD->iconBoxGeometry == NULL) /* not set by user */
#endif /* WSM */
    {
	/*
	 * Use the iconPlacement resource 
	 */
	
	if (pWS->pSD->iconPlacement & 
	    (ICON_PLACE_TOP_PRIMARY | ICON_PLACE_BOTTOM_PRIMARY))
	{
	    pIBD->IPD.iconPlacement = ICON_PLACE_TOP_PRIMARY;
	    pIBD->IPD.placementCols = 1;  
	    pIBD->IPD.placementRows = 6;
	}
	else
	{
	    pIBD->IPD.iconPlacement = ICON_PLACE_LEFT_PRIMARY;
	    pIBD->IPD.placementCols = 6;  
	    pIBD->IPD.placementRows = 1;
	}

    }
    else
    {
#ifdef WSM
	mask = XParseGeometry(pWS->iconBoxGeometry, &X, &Y, 
			      &width, &height);
#else /* WSM */
	mask = XParseGeometry(pWS->pSD->iconBoxGeometry, &X, &Y, 
			      &width, &height);
#endif /* WSM */

	if ((mask & WidthValue) && (width > 0))
	{
	    pIBD->IPD.placementCols = (int)width; 
	}
	else
	{
	    pIBD->IPD.placementCols = 6;   
	}

	if ((mask & HeightValue) && (height > 0))
	{
	    pIBD->IPD.placementRows = (int)height; 
	}
	else
	{
	    pIBD->IPD.placementRows = 1; 
	}

	/*
	 * Set orientation 
	 */

	if (pIBD->IPD.placementRows <= pIBD->IPD.placementCols) 
	{
	    pIBD->IPD.iconPlacement = ICON_PLACE_LEFT_PRIMARY;
	}
	else
	{
	    pIBD->IPD.iconPlacement = ICON_PLACE_TOP_PRIMARY;
	}
    }


    /*
     * Override orientation if iconBoxSBDisplayPolicy is set to
     * horizontal or vertical
     */

    if (!(strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szvertical)))  
    {
	pIBD->IPD.iconPlacement = ICON_PLACE_LEFT_PRIMARY;	    
    }
    else if (!(strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szhorizontal)))
    {
	pIBD->IPD.iconPlacement = ICON_PLACE_TOP_PRIMARY;
    }
	

    

    /* 
     * set initial size of placement space to size of screen 
     */

    sW = DisplayWidth (DISPLAY, pWS->pSD->screen) / pWS->pSD->iconWidth;
    sH = DisplayHeight (DISPLAY, pWS->pSD->screen) / pWS->pSD->iconHeight;

    pIBD->IPD.totalPlaces = sW * sH;

    pIBD->IPD.onRootWindow = False;

    /*
     * The icon box does not live in an icon box in this version
     */

    pIBD->pNextIconBox =NULL;
    

} /* END OF FUNCTION InitializeIconBoxData */   



/*************************************<->*************************************
 *
 *  SetIconBoxInfo (pWS, pCD)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pCD
 *
 * 
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void SetIconBoxInfo (WmWorkspaceData *pWS, ClientData *pCD)
{
    pCD->clientClass = WmCIconBox;
    pCD->clientName = pWS->pSD->iconBoxName;
    ProcessClientResources (pCD); 

} /* END OF FUNCTION SetIconBoxInfo */



/*************************************<->*************************************
 *
 *  InitializeClientData (pCD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pCD
 *
 * 
 *  Outputs:
 *  -------
 *  
 *
 *  Comments:
 *  --------
 *  Initializes geometry, etc. fields 
 * 
 *************************************<->***********************************/

void InitializeClientData (ClientData *pCD, IconBoxData *pIBD)
{
    pCD->internalBevel = (wmGD.frameStyle == WmSLAB) ? 0 : 
					    FRAME_INTERNAL_SHADOW_WIDTH;

    pCD->clientX = 0;
    pCD->clientY = 0;

    pCD->clientFlags |= ICON_BOX ;

    pCD->widthInc = pIBD->IPD.iPlaceW = ICON_WIDTH(pCD)   
	+ IB_SPACING 
	+ (2 * IB_MARGIN_WIDTH); 

    pCD->heightInc = pIBD->IPD.iPlaceH = ICON_HEIGHT(pCD) 
	+ IB_SPACING  
	+ (2 * IB_MARGIN_HEIGHT);

    pCD->clientWidth = pIBD->IPD.placementCols * pCD->widthInc;
    pCD->clientHeight = pIBD->IPD.placementRows * pCD->heightInc;
    
    if (!(pCD->pSD->iconBoxTitle))
    {
#ifndef NO_MESSAGE_CATALOG
	pCD->pSD->iconBoxTitle = 
	    XmStringCreateLocalized(wmNLS.default_icon_box_title);
#else
	pCD->pSD->iconBoxTitle = 
	    XmStringCreateLocalized(DEFAULT_ICON_BOX_TITLE);
#endif
    }

    pCD->clientTitle = pCD->pSD->iconBoxTitle;
    pCD->iconTitle   = pCD->pSD->iconBoxTitle;
    
} /* END OF FUNCTION InitializeClientData */   



/*************************************<->*************************************
 *
 *  MakeShrinkWrapIconsGC (pSD, bgPixmap)
 *
 *
 *  Description:
 *  -----------
 *  Make an  graphic context to shrink the icons in the icon box
 *      box that are not in the MINIMIZED_STATE.
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Modifies global data
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void MakeShrinkWrapIconsGC (WmScreenData *pSD, Pixmap bgPixmap)
{

    XtGCMask  copyMask;
   

    if (!pSD->shrinkWrapGC)
    {
	pSD->shrinkWrapGC = XCreateGC (DISPLAY, pSD->rootWindow, 0, 
		(XGCValues *) NULL);

	copyMask = ~0L;

	XCopyGC (DISPLAY, pSD->iconAppearance.inactiveGC,
		copyMask, pSD->shrinkWrapGC);

	if (bgPixmap != XmUNSPECIFIED_PIXMAP)
	{
	    XSetTile (DISPLAY, pSD->shrinkWrapGC,  bgPixmap); 
	    XSetFillStyle (DISPLAY, pSD->shrinkWrapGC, FillTiled);
	    XSetBackground (DISPLAY, pSD->shrinkWrapGC, select_color);
	}
	else
	{
	    XSetForeground (DISPLAY, pSD->shrinkWrapGC, select_color);
	}
    }

} /* END OF FUNCTION MakeShrinkWrapIconsGC */



/*************************************<->*************************************
 *
 *  MakeFadeIconGC (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Make an  graphic context for "greying" the icons in the icon
 *      box that are not in the MINIMIZED_STATE.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Modifies global data
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void MakeFadeIconGC (WmScreenData *pSD)
{

    XtGCMask  copyMask;
    static    Pixmap tmpFontClipMask;
   

    pSD->fadeIconGC = XCreateGC (DISPLAY, pSD->rootWindow, 0, 
				(XGCValues *) NULL);
    pSD->fadeIconTextGC = XCreateGC (DISPLAY, pSD->rootWindow, 0, 
				(XGCValues *) NULL);

    copyMask = ~0L;

    XCopyGC (DISPLAY, pSD->iconAppearance.inactiveGC,
		copyMask, pSD->fadeIconGC);

    XCopyGC (DISPLAY, pSD->iconAppearance.inactiveGC,
		copyMask, pSD->fadeIconTextGC);

    tmpFontClipMask = XCreateBitmapFromData (DISPLAY, pSD->rootWindow,
                        (char*)greyed75_bits, greyed75_width, greyed75_height);

    greyedPixmap = XCreateBitmapFromData (DISPLAY, pSD->rootWindow,
                        (char*)slant2_bits, slant2_width, slant2_height);

    XSetStipple (DISPLAY, pSD->fadeIconTextGC,  tmpFontClipMask); 
    XSetFillStyle (DISPLAY, pSD->fadeIconTextGC, FillStippled);

    XSetStipple (DISPLAY, pSD->fadeIconGC,  greyedPixmap); 
    XSetFillStyle (DISPLAY, pSD->fadeIconGC, FillStippled);
    XSetForeground (DISPLAY, pSD->fadeIconGC, select_color);

} /* END OF FUNCTION MakeFadeIconGC */



/*************************************<->*************************************
 *
 *  SetGeometry (pWS, pCD, pIBD)
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pIBD  = pointer to IconBoxData
 *  pCD   = pointer to ClientData
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void SetGeometry (WmWorkspaceData *pWS, ClientData *pCD, IconBoxData *pIBD)
{

    int i;
    Arg setArgs[10];

    int mask;
    int X;
    int Y;
    unsigned int width;
    unsigned int height;
    unsigned int boxdim, tmpMin;
    int diff;
    unsigned long       decoration;

    /*
     * Set horizontal and vertical scrolling granularity
     */

    SetGranularity (pWS, pCD, pIBD );

    /*
     * Set the initial width and height of the icon box bulletin board
     */

    i=0; 
    XtSetArg (setArgs[i], XmNwidth, (XtArgVal) pCD->clientWidth); i++;
    XtSetArg (setArgs[i], XmNheight, (XtArgVal) pCD->clientHeight); i++;
    XtSetValues (pIBD->bBoardWidget, (ArgList) setArgs, i); 

    /*
     * Adjust icon box window height for height of 
     * horizontal scroll bar etc.
     */

    pCD->clientHeight = pCD->clientHeight + pCD->baseHeight;
    pCD->oldMaxHeight = pCD->maxHeight = pCD->clientHeight;

    /*
     * Adjust iconbox window width for width of 
     * vertical scroll bar etc.
     */

    pCD->clientWidth = pCD->clientWidth + pCD->baseWidth;
    pCD->oldMaxWidth = pCD->maxWidth = pCD->clientWidth;


    /* 
     * Check that minWidth is large enough to disallow overlap
     * of title bar gadgets
     */

    /* compute for minimum frame size */
    if ((decoration = pCD->decor) & MWM_DECOR_TITLE)
    {
        boxdim = InitTitleBarHeight(pCD);   /* macro not valid yet */
        tmpMin = boxdim +
                 ((decoration & MWM_DECOR_MENU) ? boxdim : 0) +
                 ((decoration & MWM_DECOR_MINIMIZE) ? boxdim : 0) +
                 ((decoration & MWM_DECOR_MAXIMIZE) ? boxdim : 0) -
                 2*(pCD->matteWidth);
    }
    else {
        tmpMin = 0;
    }

    /* Make: 
     *   minWidth >= tmpMin
     *   minWidth >= max (baseWidth, widthInc) > 0
     *     & an integral number of widthInc from baseWidth.
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



    if (pCD->clientWidth < pCD->minWidth)
    {
        pCD->clientWidth = pCD->minWidth;
    }

    pIBD->IPD.placementCols = (int)((pCD->clientWidth - pCD->baseWidth)
				    / pCD->widthInc);


    /*
     * Make:
     *
     *   maxWidth >= minWidth
     *     & an integral number of widthInc from baseWidth.
     */

    if (pCD->maxWidth < pCD->minWidth)
    {
        pCD->maxWidth = pCD->minWidth;
    }

    pCD->maxWidthLimit = pCD->maxWidth;

    pCD->maxWidth -= ((pCD->maxWidth - pCD->baseWidth)% pCD->widthInc);
    pCD->oldMaxWidth = pCD->maxWidth;

    pCD->maxHeightLimit = pCD->maxHeight;


    /*
     * Set the initial width and height of the icon box bulletin board
     */

    i=0; 
    XtSetArg (setArgs[i], XmNwidth, (XtArgVal) pCD->clientWidth 
	      - pCD->baseWidth ); i++;
    XtSetArg (setArgs[i], XmNheight, (XtArgVal) pCD->clientHeight 
	      - pCD->baseHeight ); i++;
    XtSetValues (pIBD->bBoardWidget, (ArgList) setArgs, i); 


    /*
     * Set the initial width and height of the icon box scrolled Window
     */

    i=0; 
    XtSetArg (setArgs[i], XmNwidth, (XtArgVal) 
		(pCD->clientWidth - (2 * frameShadowThickness))); i++;
    XtSetArg (setArgs[i], XmNheight, (XtArgVal) 
		(pCD->clientHeight - (2 * frameShadowThickness))); i++;

    XtSetValues (pIBD->scrolledWidget, (ArgList) setArgs, i); 


	     
    /*
     * Call SetFrameInfo with fake X and Y so we can get clientOffset
     */

    pCD->xBorderWidth = 0;
    SetFrameInfo (pCD);


    /*
     * Set initial placement of icon box
     */

#ifdef WSM
    mask = XParseGeometry(pWS->iconBoxGeometry, &X, &Y, 
			      &width, &height);
#else /* WSM */
    mask = XParseGeometry(pCD->pSD->iconBoxGeometry, 
			  &X, &Y, &width, &height);
#endif /* WSM */    
    
    if (mask & XValue)
    {
	if (mask & XNegative)
	{
	    pCD->clientX = X 
	    		   + DisplayWidth(DISPLAY, SCREEN_FOR_CLIENT(pCD)) 
			   - pCD->clientWidth
			   - pCD->clientOffset.x;
	}
	else 
	{
	    pCD->clientX = X + pCD->clientOffset.x;
	}
    }
    else
    {
	pCD->clientX = pCD->clientOffset.x;
    }

    if (mask & YValue)
    {
	if (mask & YNegative)
	{
	    pCD->clientY = Y 
			   + DisplayHeight(DISPLAY, SCREEN_FOR_CLIENT(pCD)) 
			   - pCD->clientHeight
			   - pCD->clientOffset.x ;
	}
	else
	{
	    pCD->clientY = Y + pCD->clientOffset.y;
	}
    }
    else
    {
	pCD->clientY =  pCD->clientOffset.x
			+ DisplayHeight(DISPLAY, SCREEN_FOR_CLIENT(pCD)) 
			- pCD->clientHeight;
    }


    PlaceFrameOnScreen (pCD, &pCD->clientX, &pCD->clientY, pCD->clientWidth,
        pCD->clientHeight);
    pCD->clientX -= (wmGD.positionIsFrame
			? pCD->clientOffset.x
			: 0);
			
    pCD->clientY -=  (wmGD.positionIsFrame
			? pCD->clientOffset.y
			: 0);


    i=0; 

    XtSetArg (setArgs[i], XmNx, (XtArgVal) pCD->clientX); i++;
    XtSetArg (setArgs[i], XmNy, (XtArgVal) pCD->clientY); i++;

    XtSetValues (pIBD->shellWidget, (ArgList) setArgs, i); 

    pCD->maxX = pCD->clientX;
    pCD->maxY = pCD->clientY;


} /* END OF FUNCTION SetGeometry */



/*************************************<->*************************************
 *
 *  SetGranularity (pWS, pCD, pIBD )
 *
 *
 *  Description:
 *  -----------
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pIBD  = pointer to IconBoxData
 *  pCD   = pointer to ClientData
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  
 *  
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void SetGranularity (WmWorkspaceData *pWS, ClientData *pCD, IconBoxData *pIBD)
{

    int i;
    Dimension hScrollBarHeight = 0;
    Dimension hBarHeight = 0;
    Dimension vScrollBarWidth = 0;
    Dimension vBarWidth = 0;

    Dimension spacing;
    short shadowThickness;
    short marginWidth;
    short marginHeight;

    short hShighlightThickness;
    short vShighlightThickness;

    Arg setArgs[10];
    Arg getArgs[10]; 

    i=0;



    XtSetArg(getArgs[i], XmNspacing, (XtArgVal) &spacing ); i++;
    XtSetArg(getArgs[i], XmNshadowThickness, (XtArgVal) &shadowThickness); i++;
    XtSetArg(getArgs[i], XmNscrolledWindowMarginWidth, 
					(XtArgVal) &marginWidth); i++;
    XtSetArg(getArgs[i], XmNscrolledWindowMarginHeight, 
					(XtArgVal) &marginHeight); i++;
    XtSetArg (getArgs[i], XmNverticalScrollBar, 
	                                (XtArgVal) &pIBD->vScrollBar); i++;
    XtSetArg(getArgs[i], XmNhorizontalScrollBar, 
	                                (XtArgVal) &pIBD->hScrollBar); i++;
    XtGetValues (pIBD->scrolledWidget, getArgs, i);

    
    if (strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szvertical))
    {
	
	/*
	 * Set horizontal scrolling granularity
	 */
	i=0;
        XtSetArg (getArgs[i], XmNheight, (XtArgVal) &hBarHeight ); i++;
        XtSetArg (getArgs[i], XmNhighlightThickness,
                 (XtArgVal) &hShighlightThickness); i++;
        XtGetValues (pIBD->hScrollBar, getArgs, i);


	i=0; 
	XtSetArg(setArgs[i], XmNincrement, (XtArgVal) pCD->widthInc); i++;
	XtSetArg (setArgs[i], XmNhighlightThickness ,
		  IB_HIGHLIGHT_BORDER); i++;
        XtSetArg(setArgs[i], XmNheight,
                 (XtArgVal) (hBarHeight - (2 * hShighlightThickness)) +
                 (2 * IB_HIGHLIGHT_BORDER)); i++;

	XtSetValues (pIBD->hScrollBar, (ArgList) setArgs, i); 
	
	/*
	 * Get hScrollBarHeight and troughColor
	 */
	
	i=0;
	XtSetArg (getArgs[i], XmNtroughColor, (XtArgVal) &select_color ); i++;
	XtSetArg (getArgs[i], XmNheight, (XtArgVal) &hScrollBarHeight ); i++;
	XtGetValues (pIBD->hScrollBar, getArgs, i);

    }
    
    
    if (strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szhorizontal))
    {
	
	/*
	 * Set vertical scrolling granularity
	 */
        i=0;
        XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &vBarWidth ); i++;
        XtSetArg (getArgs[i], XmNhighlightThickness,
                 (XtArgVal) &vShighlightThickness); i++;
        XtGetValues (pIBD->vScrollBar, getArgs, i);


	i=0; 
	XtSetArg (setArgs[i], XmNincrement, (XtArgVal) pCD->heightInc); i++;
	XtSetArg (setArgs[i], XmNhighlightThickness ,
                        IB_HIGHLIGHT_BORDER); i++;
        XtSetArg(setArgs[i], XmNwidth,
                 (XtArgVal) (vBarWidth - (2 * vShighlightThickness)) +
                 (2 * IB_HIGHLIGHT_BORDER)); i++;

	XtSetValues (pIBD->vScrollBar, (ArgList) setArgs, i); 
	
	/*
	 * Get vScrollBarWidth
	 */
	
	i=0;
	XtSetArg (getArgs[i], XmNtroughColor, (XtArgVal) &select_color ); i++;
	XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &vScrollBarWidth ); i++;
	XtGetValues (pIBD->vScrollBar, getArgs, i);
    }
    
    
    
    if (!strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szvertical))
    {
	XtUnmanageChild(pIBD->hScrollBar);
	hScrollBarHeight = 0;
	
	i=0;
	XtSetArg (setArgs[i], XmNscrollBarDisplayPolicy, 
		  (XtArgVal) XmAS_NEEDED ); i++;
	XtSetValues (pIBD->scrolledWidget, (ArgList) setArgs, i);
	
    }
    else if (!strcmp(pWS->pSD->iconBoxSBDisplayPolicy , szhorizontal))
    {
	XtUnmanageChild(pIBD->vScrollBar);
	vScrollBarWidth = 0;
	
	i=0;
	XtSetArg (setArgs[i], XmNscrollBarDisplayPolicy, 
		  (XtArgVal) XmAS_NEEDED ); i++;
	XtSetValues (pIBD->scrolledWidget, (ArgList) setArgs, i);
    }
    
    
    
    
    pCD->baseWidth =  IB_SPACING 
	               + 2 * IB_HIGHLIGHT_BORDER
#ifdef DEBUG_ICON_BOX
	               + 2
		       + spacing
#endif /* DEBUG_ICON_BOX */
		       + (int) vScrollBarWidth 
		       + 2 * frameShadowThickness
		       + (int) 2 * marginWidth
		       + (marginWidth > 0 
			       ? 2 * (int) shadowThickness 
			       : shadowThickness);

			

    pCD->baseHeight =  IB_SPACING
	                + 2 * IB_HIGHLIGHT_BORDER
#ifdef DEBUG_ICON_BOX
                        + 2
#endif /* DEBUG_ICON_BOX */
			+ spacing
			+ (int) hScrollBarHeight 
			+ 2 * frameShadowThickness
			+ (int) 2 * marginHeight
			+ (marginHeight > 0 
				? 2 * (int) shadowThickness 
				: shadowThickness);

    pCD->minWidth = pCD->baseWidth + pCD->widthInc;
    pCD->minHeight = pCD->baseHeight + pCD->heightInc;

    pCD->oldMaxWidth = pCD->maxWidth = pCD->minWidth;

    pCD->oldMaxHeight = pCD->maxHeight = pCD->minHeight;

} /* END OF FUNCTION SetGranularity */



/*************************************<->*************************************
 *
 * GetIconBoxMenuItems ()
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

MenuItem *GetIconBoxMenuItems (pSD)

    WmScreenData *pSD;

{

    return(ParseMwmMenuStr (pSD, 
	(unsigned char *)((char *)GETMESSAGE(36, 3, "\"Pack Icons\" _P  Alt Shift<Key>F7 f.pack_icons\n"))));

} /* END OF FUNCTION GetIconBoxMenuItems */



/*************************************<->*************************************
 *
 *  MapIconBoxes ()
 *
 *
 *  Description:
 *  -----------
 *  
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
 *  Maps all iconboxes on the list starting at pWS->pIconBox 
 * 
 *************************************<->***********************************/

void MapIconBoxes (WmWorkspaceData *pWS)
{
 
    IconBoxData *pibd;

    if (pWS->pIconBox)
    {
	pibd = pWS->pIconBox;

        while (pibd)
        {
	    XtPopup(pibd->shellWidget, XtGrabNone);
#ifndef WSM
	    F_Raise (NULL, pibd->pCD_iconBox, (XEvent *)NULL);
	    XMapWindow (DISPLAY, pibd->pCD_iconBox->clientFrameWin);
#endif /* WSM */
	    pibd = pibd->pNextIconBox;
        }
    }

  
} /* END OF FUNCTION MapIconBoxes */

#ifdef WSM

/*************************************<->*************************************
 *
 *  UnmapIconBoxes (pWS)
 *
 *
 *  Description:
 *  -----------
 *  Unmaps all the iconboxes in the  specified workspace
 *
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 * 
 *  Outputs:
 *  -------
 *  
 *
 *  Comments:
 *  --------
 *  Unmaps all iconboxes on the list starting at pWS->pIconBox 
 *  Does not do anything with icon windows.
 * 
 *************************************<->***********************************/

void UnmapIconBoxes (WmWorkspaceData *pWS)
{
 
    IconBoxData *pibd;

    if (pWS->pIconBox)
    {
	pibd = pWS->pIconBox;

        while (pibd)
        {
	    XUnmapWindow (DISPLAY, pibd->pCD_iconBox->clientFrameWin);
	    pibd = pibd->pNextIconBox;
        }
    }

} /* END OF FUNCTION UnmapIconBoxes */
#endif /* WSM */

#if defined(PANELIST)

/******************************<->*************************************
 *
 *  IconBoxShowing ()
 *
 *  Description:
 *  -----------
 *  Returns True if an icon box tied to a front panel button is
 *  showing.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCW = pointer to control window data (for front panel button )
 * 
 *  Outputs:
 *  -------
 *  Return = True if icon box is up, False if it's invisible
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/

    
#ifdef PANELIST
Boolean
IconBoxShowing (WmWorkspaceData *pWS)
#else /* PANELIST */
Boolean
IconBoxShowing (WmWorkspaceData *pWS, ControlWindowStruct *pCW)
#endif /* PANELIST */
{
    Boolean rval = False;
    int wsIndex =  GetCurrentWorkspaceIndex (pWS->pSD); 
    
#ifdef PANELIST
    if (pWS->pIconBox &&
	ClientInWorkspace (pWS, pWS->pIconBox->pCD_iconBox))
    {
	rval = True;
    }
#else /* PANELIST */
    if (pWS->pIconBox &&
	ClientInWorkspace (pWS, pWS->pIconBox->pCD_iconBox) &&
	(pCW->pWsStatus[wsIndex].wsClientStatus == CLIENT_WINDOW_OPEN))
    {
	rval = True;
    }
#endif /* PANELIST */

    return (rval);
    
} /* END OF FUNCTION IconBoxShowing */



/******************************<->*************************************
 *
 *  IconBoxPopUp (pWS, pCW, up)
 *
 *  Description:
 *  -----------
 *  Sets the state of the icon box attached to a front panel control.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *  pCW = pointer to control window data (for front panel button )
 *  up = flag, if True, pop the icon box up; if False, hide it.
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *  Comments:
 *  --------
 * 
 ******************************<->***********************************/
    
#ifdef PANELIST
void
IconBoxPopUp (WmWorkspaceData *pWS, Boolean up)
#else /* PANELIST */
void
IconBoxPopUp (WmWorkspaceData *pWS, 
    ControlWindowStruct *pCW, Boolean up)
#endif /* PANELIST */
{
    
    IconBoxData *pibd;
    int wsIndex =  GetCurrentWorkspaceIndex (pWS->pSD); 
    
    if (pWS->pIconBox)
    {
	pibd = pWS->pIconBox;
	
        while (pibd)
        {
	    if (up)
	    {
		if (ClientInWorkspace(pWS, pibd->pCD_iconBox))
		{
		    F_Raise (NULL, pibd->pCD_iconBox, (XEvent *)NULL);
		}
		else
		{ 
		    AddClientToWorkspaces (pibd->pCD_iconBox, 
					   &pWS->id, 1);
		    return;
		}
	    }
	    else if (!up && ClientInWorkspace (pWS, pibd->pCD_iconBox))
	    {
		RemoveClientFromWorkspaces (pibd->pCD_iconBox,
					    &pWS->id, 1);
		return;
	    }		    
	    pibd = pibd->pNextIconBox;
	}
	
    }
    
} /* END OF FUNCTION IconBoxPopUp */
#endif /* PANELIST */




/******************************<->*************************************
 *
 *  InsertIconIntoBox
 *
 *  Inputs
 *  ------
 *  pCD		- pointer to data for client to insert
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

Boolean InsertIconIntoBox (IconBoxData *pIBD, ClientData *pCD)
{

    Boolean rval = False;
    Arg setArgs[20]; 
    int i;
    int iconWidth, iconHeight;
    IconBoxData  *tmpPointerToIconBox;
    Widget iconWidget;
    IconInfo *pIconInfo;
    static XmString dummyString = NULL;
#ifdef WSM
    WsClientData *pWsc;
    WmWorkspaceData *pWS = GetWorkspaceData (pCD->pSD, pIBD->wsID);

    pWsc = GetWsClientData (pWS, pCD);
#endif /* WSM */

    /*
     * If we go to multiple icon boxes, find the box this client
     * wants to live in.  For now, we only have one, so point to
     * the first one.
     */

    tmpPointerToIconBox = pIBD;
    
    if (pCD->client)
    {

#ifdef WSM
        pWsc->pIconBox = tmpPointerToIconBox;
#else /* WSM */
        P_ICON_BOX(pCD) = tmpPointerToIconBox;
#endif /* WSM */

        iconWidth = ICON_WIDTH(pCD)
		+ (2 * IB_MARGIN_WIDTH); 

        iconHeight = ICON_HEIGHT(pCD) 
		+ (2 * IB_MARGIN_HEIGHT);

#ifdef WSM
        pIconInfo = InsertIconInfo  (pWsc->pIconBox, pCD, (Widget) NULL);
#else /* WSM */
        pIconInfo = InsertIconInfo  (P_ICON_BOX(pCD), pCD, (Widget) NULL);
#endif /* WSM */

	if (pIconInfo)
	{
#ifdef WSM
	    pWsc->pIconBox->numberOfIcons++;
#else /* WSM */
	    P_ICON_BOX(pCD)->numberOfIcons++;
#endif /* WSM */

	    i = 0;
	    XtSetArg (setArgs[i], XmNbackground,  
			    (XtArgVal) ICON_APPEARANCE(pCD).background ); i++;
	    XtSetArg (setArgs[i], XmNforeground,  
			    (XtArgVal) ICON_APPEARANCE(pCD).foreground ); i++;

#ifdef WSM
	    XtSetArg (setArgs[i], XmNx ,  (XtArgVal) pWsc->iconX); i++;
	    XtSetArg (setArgs[i], XmNy ,  (XtArgVal) pWsc->iconY); i++;
#else /* WSM */
	    XtSetArg (setArgs[i], XmNx ,  (XtArgVal) ICON_X(pCD)); i++;
	    XtSetArg (setArgs[i], XmNy ,  (XtArgVal) ICON_Y(pCD)); i++;
#endif /* WSM */

	    XtSetArg (setArgs[i], XmNwidth ,  (XtArgVal) iconWidth); i++;
	    XtSetArg (setArgs[i], XmNheight ,  (XtArgVal) iconHeight); i++;

	    XtSetArg (setArgs[i], XmNborderWidth ,  (XtArgVal) 0); i++;

	    XtSetArg (setArgs[i], XmNhighlightThickness ,  
			    IB_HIGHLIGHT_BORDER); i++;

	    XtSetArg (setArgs[i], XmNmarginHeight , (XtArgVal) 0); i++;
	    XtSetArg (setArgs[i], XmNmarginWidth , 	(XtArgVal) 0); i++;
	    /* 
	     * Use type XmString so we don't get a message from XmLabel
	     */
	    XtSetArg (setArgs[i], XmNlabelType, (XtArgVal) XmSTRING); i++;

	    XtSetArg (setArgs[i], XmNrecomputeSize, (XtArgVal) False); i++;

	    XtSetArg (setArgs[i], XmNtraversalOn, (XtArgVal) True); i++;

	    XtSetArg (setArgs[i], XmNpushButtonEnabled, (XtArgVal) False); i++;

	    XtSetArg (setArgs[i], XmNshadowThickness, (XtArgVal) 0); i++;

	    iconWidget =  XtCreateManagedWidget("iconInIconBox",
					   xmDrawnButtonWidgetClass,
#ifdef WSM
					   pWsc->pIconBox->bBoardWidget,
#else /* WSM */
					   P_ICON_BOX(pCD)->bBoardWidget,
#endif /* WSM */
					   (ArgList)setArgs, i);

	    if (dummyString == NULL)
	    {
	        dummyString = 
		    XmStringCreateLocalized("");
	    }

	    i = 0;
	    XtSetArg (setArgs[i], XmNlabelString, 
					(XtArgVal) dummyString); i++;

	    XtSetValues (iconWidget, setArgs, i);

	    pIconInfo->theWidget = iconWidget;

#ifdef WSM
	    pWsc->iconFrameWin = XtWindow (iconWidget); 
#else /* WSM */
	    ICON_FRAME_WIN(pCD) = XtWindow (iconWidget); 
#endif /* WSM */

	    XtAddCallback (iconWidget, XmNactivateCallback, 
			   (XtCallbackProc)IconActivateCallback, 
			   (XtPointer)NULL);

	    XtAddEventHandler(iconWidget, 
			      SELECT_BUTTON_MOTION_MASK, 
			      False, 
			      (XtEventHandler)HandleIconBoxButtonMotion, 
			      (XtPointer)NULL);

	    XtAddEventHandler(iconWidget, 
			      DMANIP_BUTTON_MOTION_MASK, 
			      False, 
			      (XtEventHandler)HandleIconBoxButtonMotion, 
			      (XtPointer)NULL);

            XtAddEventHandler(iconWidget,
			      KeyPressMask,
			      False,
			      (XtEventHandler)HandleIconBoxIconKeyPress,
			      (XtPointer)NULL);

	    
	    
	    if (ICON_DECORATION(pCD) & ICON_ACTIVE_LABEL_PART)
	    {
		XtAddEventHandler(iconWidget, 
				  FocusChangeMask, 
				  False, 
				  (XtEventHandler)ChangeActiveIconboxIconText, 
				  (XtPointer)NULL);

		if (pCD->pSD->activeLabelParent != pCD->pSD->rootWindow)
		{
		    XRaiseWindow (DISPLAY, pCD->pSD->activeIconTextWin);
		}
	    }

#ifdef WSM
	    ResetIconBoxMaxSize(pWsc->pIconBox->pCD_iconBox, 
				pWsc->pIconBox->bBoardWidget);

	    ResetArrowButtonIncrements (pWsc->pIconBox->pCD_iconBox);

#else /* WSM */
	    ResetIconBoxMaxSize(P_ICON_BOX(pCD)->pCD_iconBox, 
				P_ICON_BOX(pCD)->bBoardWidget);

	    ResetArrowButtonIncrements (P_ICON_BOX(pCD)->pCD_iconBox);

#endif /* WSM */
	    rval = True;
	}
    } 
    return(rval);

} /* END FUNCTION InsertIconIntoBox() */   
   


/*************************************<->*************************************
 *
 *  InsertIconInfo  (pIBD, pCD, theWidget)
 *
 *
 *  Description:
 *  -----------
 *  Finds next available spot and inserts the icon 
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- pointer to icon box data
 *  pCD		- pointer to client data for this client
 *  theWidget	- widget containing the icon (may be null)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

IconInfo *InsertIconInfo (pIBD, pCD, theWidget)

    IconBoxData *pIBD;
    ClientData *pCD;
    Widget theWidget;

{
    IconInfo *pII;
    int place;
    int amt, i;
    Arg setArgs[3];
    Arg getArgs[4];
    Dimension clipWidth, clipHeight;
#ifdef WSM
    WsClientData *pWsc;
#endif /* WSM */

    place = GetNextIconPlace (&pIBD->IPD);
    if (place == NO_ICON_PLACE)
    {
	if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
	{
	    amt = pIBD->IPD.placementCols;		/* add a new row */
	}
	else
	{
	    amt = pIBD->IPD.placementRows;		/* add a new column */
	}

	if (!ExtendIconList (pIBD, amt))
	{
	    Warning (((char *)GETMESSAGE(36, 4, "Insufficient memory to create icon box data")));
	    return (NULL);
	}

	if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
	{
	    pIBD->IPD.placementRows++;
	}
	else
	{
	    pIBD->IPD.placementCols++;
	}
	place = GetNextIconPlace (&pIBD->IPD);
    }

    insertPosition = place;

    /*
     * Update icon info values
     */

    pII = &pIBD->IPD.placeList[place];
    pII->theWidget = theWidget;

    pII->pCD = pCD;

#ifdef WSM
    pWsc = GetWsClientData (GetWorkspaceData (pCD->pSD, pIBD->wsID), pCD);
    pWsc->iconPlace = place;

    CvtIconPlaceToPosition (&pIBD->IPD, pWsc->iconPlace,
	    &pWsc->iconX, &pWsc->iconY);


    /* update next free position */

    pIBD->currentCol = pWsc->iconX / pIBD->pCD_iconBox->widthInc;
    pIBD->currentRow = pWsc->iconY / pIBD->pCD_iconBox->heightInc;

#else /* WSM */
    ICON_PLACE(pCD) = place;

    CvtIconPlaceToPosition (&pIBD->IPD, ICON_PLACE(pCD),
	    &ICON_X(pCD), &ICON_Y(pCD));


    /* update next free position */

    pIBD->currentCol = ICON_X(pCD) / pIBD->pCD_iconBox->widthInc;
    pIBD->currentRow = ICON_Y(pCD) / pIBD->pCD_iconBox->heightInc;

#endif /* WSM */

    /* 
     * Increase bboard size if necessary
     */
    i = 0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &clipWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &clipHeight ); i++;
    XtGetValues (pIBD->clipWidget, getArgs, i);

    i = 0;
    if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
    {
	if (pIBD->currentCol > pIBD->lastCol)
	{
	    pIBD->lastCol = pIBD->currentCol;
	}

	if (pIBD->currentRow > pIBD->lastRow)
	{
	    pIBD->lastRow = pIBD->currentRow;
#ifdef WSM
	    if (clipHeight <= (Dimension) (pWsc->iconY + 
	                                   pIBD->pCD_iconBox->heightInc))
	    {
		/*
		 * Increase bulletin board height as needed.
		 */
		XtSetArg (setArgs[i], XmNheight, (XtArgVal) 
			  pWsc->iconY + pIBD->pCD_iconBox->heightInc); i++;
	    }
#else /* WSM */
	    if (clipHeight <= (pII->pCD->iconY + pIBD->pCD_iconBox->heightInc))
	    {
		/*
		 * Increase bulletin board height as needed.
		 */
		XtSetArg (setArgs[i], XmNheight, (XtArgVal) 
			  pII->pCD->iconY + pIBD->pCD_iconBox->heightInc); i++;
	    }
#endif /* WSM */
	}
    }
    else
    {
	if (pIBD->currentCol > pIBD->lastCol)
	{
	    pIBD->lastCol = pIBD->currentCol;
#ifdef WSM
	    if (clipWidth <= (Dimension) 
	                      (pWsc->iconX + pIBD->pCD_iconBox->widthInc))
	    {
		/*
		 * Increase bulletin board width as needed
		 */
		XtSetArg (setArgs[i], XmNwidth, 
		(XtArgVal) pWsc->iconX +
			   pIBD->pCD_iconBox->widthInc); i++;
	    }
#else /* WSM */
	    if (clipWidth <= (pII->pCD->iconX + pIBD->pCD_iconBox->widthInc))
	    {
		/*
		 * Increase bulletin board width as needed
		 */
		XtSetArg (setArgs[i], XmNwidth, 
		(XtArgVal) pII->pCD->iconX +
			   pIBD->pCD_iconBox->widthInc); i++;
	    }
#endif /* WSM */
	}

	if (pIBD->currentRow > pIBD->lastRow)
	{
	    pIBD->lastRow = pIBD->currentRow;
	}
    }

    if (i > 0)
    {
	XtSetValues (pIBD->bBoardWidget, setArgs, i);
    }

    return(pII);
    


} /* END OF FUNCTION InsertIconInfo */



/*************************************<->*************************************
 *
 *  DeleteIconFromBox
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

void DeleteIconFromBox (IconBoxData *pIBD, ClientData *pCD)
{
    Widget       theChild;
    ClientData  *pCD_tmp;
    Arg          args[4];
    Dimension    clipWidth, clipHeight;
    Dimension    oldWidth, oldHeight;
    int          newWidth, newHeight;
    int          i, newCols, newRows;
#ifdef WSM
    WmWorkspaceData *pWS = GetWorkspaceData (pCD->pSD, pIBD->wsID);
    WsClientData *pWsc;

    pWsc = GetWsClientData (pWS, pCD);
#endif /* WSM */

    i = 0;
    XtSetArg (args[i], XmNwidth, (XtArgVal) &oldWidth ); i++;
    XtSetArg (args[i], XmNheight, (XtArgVal) &oldHeight ); i++;
    XtGetValues (pIBD->bBoardWidget, args, i);

    i = 0;
    XtSetArg (args[i], XmNwidth, (XtArgVal) &clipWidth); i++;
    XtSetArg (args[i], XmNheight, (XtArgVal) &clipHeight ); i++;
    XtGetValues (pIBD->clipWidget, args, i);

    clipHeight /= (Dimension) pIBD->pCD_iconBox->heightInc;
    clipWidth  /= (Dimension) pIBD->pCD_iconBox->widthInc;

    /* 
     * find context of the activeIconTextWin to get pCD and then 
     * if it is the same as this client, hide it.
     */

    if (!(XFindContext (DISPLAY, pCD->pSD->activeIconTextWin,
			wmGD.windowContextType, (caddr_t *)&pCD_tmp)))
    {
	if (pCD == pCD_tmp)
	{
	    /* hide activeIconTextWin */
	    HideActiveIconText ((WmScreenData *)NULL);
	}
    }

#ifdef WSM
    DeleteIconInfo (pWsc->pIconBox, pCD);

    pWsc->pIconBox->numberOfIcons--;

    theChild = XtWindowToWidget (DISPLAY, pWsc->iconFrameWin);

    pWsc->pIconBox = NULL;
    pWsc->iconPlace = NO_ICON_PLACE;
#else /* WSM */
    DeleteIconInfo (P_ICON_BOX(pCD), pCD);

    pCD->pIconBox->numberOfIcons--;

    theChild = XtWindowToWidget (DISPLAY, ICON_FRAME_WIN(pCD));
#endif /* WSM */
    XtUnmanageChild (theChild);

    XtDestroyWidget (theChild);

    /* update last row and col */

    SetNewBounds (pIBD);

    /* resize Bulletin board  (so scroll bars show correctly */
    i = 0;

    if (clipWidth <= (Dimension) (pIBD->lastCol + 1))
    {
        newWidth = (pIBD->lastCol + 1) * pIBD->pCD_iconBox->widthInc;    
        XtSetArg (args[i], XmNwidth, (XtArgVal) newWidth); i++;
	newCols = newWidth / pIBD->pCD_iconBox->widthInc;
    }
    else
    {
	newWidth = clipWidth * pIBD->pCD_iconBox->widthInc;    
	XtSetArg (args[i], XmNwidth, (XtArgVal) newWidth); i++;
	newCols = newWidth / pIBD->pCD_iconBox->widthInc;
    }

    if (clipHeight <= (Dimension) (pIBD->lastRow + 1))
    {
        /* set height of bboard */
        newHeight = (pIBD->lastRow + 1) * pIBD->pCD_iconBox->heightInc;
        XtSetArg (args[i], XmNheight, (XtArgVal) newHeight ); i++;
	newRows = newHeight / pIBD->pCD_iconBox->heightInc;
    }
    else
    {
	newHeight = clipHeight * pIBD->pCD_iconBox->heightInc;
	XtSetArg (args[i], XmNheight, (XtArgVal) newHeight ); i++;
	newRows = newHeight / pIBD->pCD_iconBox->heightInc;
    }

    if (i > 0  &&  ExpandVirtualSpace(pIBD, newWidth, newHeight))
    {
	XtSetValues (pIBD->bBoardWidget, args, i);
	RealignIconList (pIBD, newCols, newRows);
	pIBD->IPD.placementCols = newCols;
	pIBD->IPD.placementRows = newRows;
    }


    /* reset max size for icon box */
    ResetIconBoxMaxSize(pIBD->pCD_iconBox, pIBD->bBoardWidget);
    
    ResetArrowButtonIncrements (pIBD->pCD_iconBox);    

} /* END FUNCTION DeleteIconFromBox */



/*************************************<->*************************************
 *
 *  DeleteIconInfo (pIBD, pCD)
 *
 *
 *  Description:
 *  -----------
 *  Deletes an icon info record from the icon box list based on the 
 *  client data pointer.
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- pointer to icon box data
 *  pCD		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o The deleted item is freed
 *  o Is pCD the correct key???? !!!
 * 
 *************************************<->***********************************/
void DeleteIconInfo (IconBoxData *pIBD, ClientData *pCD)
{
    int ix, count;
    IconInfo *pII;

    /* find first matching entry in list */

    pII = &pIBD->IPD.placeList[0]; 
    count = pIBD->IPD.totalPlaces;

    for (ix = 0; ix < count && pII->pCD != pCD; ix++, pII++)
    {
    }

    if (ix < count)
    {
	/* found it, zero the entry out */
	pII->theWidget = NULL;
	pII->pCD = NULL;
    }


} /* END FUNCTION DeleteIconInfo */



/*************************************<->*************************************
 *
 *  ResetIconBoxMaxSize(pCD, bBoardWidget)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

void ResetIconBoxMaxSize (ClientData *pCD, Widget bBoardWidget)
{
    int i;
    Arg getArgs[3]; 
    Dimension newWidth;
    Dimension newHeight;

    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &newWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &newHeight ); i++;
    XtGetValues (bBoardWidget, getArgs, i);

    pCD->oldMaxWidth = pCD->maxWidth = newWidth + pCD->baseWidth;

    pCD->oldMaxHeight = pCD->maxHeight = newHeight + pCD->baseHeight;

    pCD->maxX = pCD->clientX;
    pCD->maxY = pCD->clientY;
    PlaceFrameOnScreen (pCD, &pCD->maxX, &pCD->maxY, 
			pCD->maxWidth, pCD->maxHeight);

} /* END OF FUNCTION 	ResetIconBoxMaxSize */



/*************************************<->*************************************
 *
 * CheckIconBoxSize(pIBD)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

Boolean CheckIconBoxSize (IconBoxData *pIBD)
{
    int i;
    Arg getArgs[3]; 
    Arg setArgs[3]; 
    Dimension oldWidth;
    Dimension oldHeight;
    Dimension newWidth;
    Dimension newHeight;
    int oldCol, oldRow;
    Boolean rval = True;

    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &oldWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &oldHeight ); i++;
    XtGetValues (pIBD->bBoardWidget, getArgs, i);

    newWidth = oldWidth;
    newHeight = oldHeight;
    oldCol = oldWidth / (Dimension) pIBD->pCD_iconBox->widthInc;
    oldRow = oldHeight / (Dimension) pIBD->pCD_iconBox->heightInc;

    /* 
     * Increase bboard size if necessary
     */

    i = 0;
    if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
    {
	if (oldRow < pIBD->lastRow + 1)
	{
	    /*
	     * increase bulletin board height as needed
	     */
	    newHeight = (pIBD->lastRow * pIBD->pCD_iconBox->heightInc)
			 + pIBD->pCD_iconBox->heightInc;

	    XtSetArg (setArgs[i], XmNheight, (XtArgVal) newHeight); i++;
	}
    }
    else
    {
	if (oldCol  < pIBD->lastCol + 1)
	{
	    /*
	     * increase bulletin board width as needed
	     */
	    newWidth = (pIBD->lastCol * pIBD->pCD_iconBox->widthInc)
			    + pIBD->pCD_iconBox->widthInc;

	    XtSetArg (setArgs[i], XmNwidth, newWidth); i++;
	}
    }

    if (i > 0)
    {
	if (! ExpandVirtualSpace(pIBD, newWidth, newHeight))
	{
	    /*
	     * The user has resized the iconbox larger than
	     * memory will allow.  Don't honor the resize request
	     */
	    rval = False;
	    return(rval);
	}
	XtSetValues (pIBD->bBoardWidget, setArgs, i);
    }

    ResetIconBoxMaxSize(pIBD->pCD_iconBox, pIBD->bBoardWidget);
    

    return(rval);

} /* END OF FUNCTION CheckIconBoxSize */



/*************************************<->*************************************
 *
 * CheckIconBoxResize(pCD, changedValues)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

void CheckIconBoxResize (ClientData *pCD, unsigned int changedValues, int newWidth, int newHeight)
{

    Boolean  packVert = False;
    Boolean  packHorz = False;
    WmScreenData *pSD;

    IconBoxData *pIBD;
    IconPlacementData *pIPD;
    int i, newCols, newRows;
    Arg getArgs[3]; 
    Arg setArgs[3]; 
    Dimension oldWidth;
    Dimension oldHeight;

    pIPD = &pCD->thisIconBox->IPD;
    pIBD = pCD->thisIconBox;

    pSD = &(wmGD.Screens[SCREEN_FOR_CLIENT(pCD)]);

    
    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &oldWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &oldHeight ); i++;
    XtGetValues (pIBD->bBoardWidget, getArgs, i);
    
    newCols = pIPD->placementCols;
    newRows = pIPD->placementRows;
    newWidth = newWidth - pCD->baseWidth;	    	
    newHeight = newHeight - pCD->baseHeight;
    
    i = 0;
    
    if (changedValues & CWWidth) 
    {
	/*
	 * There was a change in Width, see if we need to change the
	 * bulletin board
	 */
	if (newWidth > (int) oldWidth)
	{
	    newCols = newWidth / pCD->widthInc;
	    XtSetArg (setArgs[i], XmNwidth, (XtArgVal) newWidth ); i++;
	}
	
	if (newWidth < (int) oldWidth)
	{
	    if ((!strcmp(pSD->iconBoxSBDisplayPolicy, szvertical)) &&
		(newWidth / pCD->widthInc < pIBD->lastCol + 1))
	    {
		XtSetArg (setArgs[i], XmNwidth, (XtArgVal) newWidth ); i++;
		newCols = newWidth / pCD->widthInc;
		packVert = True;
	    }
	    else if (newWidth / pCD->widthInc < pIBD->lastCol + 1)
	    {
		newWidth = (pIBD->lastCol +1) * pCD->widthInc;
		XtSetArg (setArgs[i], XmNwidth, (XtArgVal) newWidth ); i++;
	    }
	    else
	    {
		newCols = newWidth / pCD->widthInc;
		XtSetArg (setArgs[i], XmNwidth, (XtArgVal) newWidth ); i++;
	    }
	}
    }
    else
    {
	newWidth = oldWidth;
    }
	
    if (changedValues & CWHeight) 
    {
	/*
	 * There was a change in Height, see if we need to change the
	 * bulletin board
	 */
	if (newHeight > (int) oldHeight)
	{
	    newRows = newHeight / pCD->heightInc;
	    XtSetArg (setArgs[i], XmNheight, (XtArgVal) newHeight ); i++;
	}

	if (newHeight < (int) oldHeight)
	{
	    if ((!strcmp(pSD->iconBoxSBDisplayPolicy, szhorizontal)) &&
                (newHeight / pCD->heightInc < pIBD->lastRow + 1))
	    {
		XtSetArg (setArgs[i], XmNheight, (XtArgVal) newHeight ); i++;
		newRows = newHeight / pCD->heightInc;
		packHorz = True;		
	    }
	    else if (newHeight / pCD->heightInc < pIBD->lastRow + 1)
	    {
		newHeight = (pIBD->lastRow + 1) * pCD->heightInc;
		XtSetArg (setArgs[i], XmNheight, (XtArgVal) newHeight ); i++;
	    }
	    else
	    {
		newRows = newHeight / pCD->heightInc;
		XtSetArg (setArgs[i], XmNheight, (XtArgVal) newHeight ); i++;
	    }
	}
    }
    else
    {
	newHeight = oldHeight;
    }
    
    if ( i >0   &&   ExpandVirtualSpace(pIBD, newWidth, newHeight))
    {
	XtSetValues (pIBD->bBoardWidget, setArgs, i);
    }

    RealignIconList (pIBD, newCols, newRows);

    pIPD->placementCols = newCols;
    pIPD->placementRows = newRows;
    
    ResetIconBoxMaxSize(pCD, pIBD->bBoardWidget);
    
    /*
     * Pack the icon box if there are icons that can no longer
     * be scrolled to due to iconBoxSBDisplayPolicy.
     */
    if (packVert)
    {
	PackIconBox (pIBD, packVert, False , newWidth, 0);
    }
    else if (packHorz)
    {
	PackIconBox (pIBD, False, packHorz , 0, newHeight);
    }


} /* END OF FUNCTION CheckIconBoxResize */



/*************************************<->*************************************
 *
 *  ExpandVirtualSpace (pIBD, newWidth, newHeight)
 *
 *
 *  Description:
 *  -----------
 *  Add virtural space (really the icon list )
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- ptr to icon box data
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- True if successful, False otherwise
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean ExpandVirtualSpace (IconBoxData *pIBD, int newWidth, int newHeight)
{
    Boolean rval = True;
    int newSize;
    int increment;

    newSize = (newWidth / pIBD->pCD_iconBox->widthInc) *
		(newHeight / pIBD->pCD_iconBox->heightInc);

    if (newSize > pIBD->IPD.totalPlaces )
    {
	increment = newSize - pIBD->IPD.totalPlaces;
	rval = ExtendIconList (pIBD, increment);
    }

    return (rval);

} /* END OF FUNCTION ExpandVirtualSpace */



/*************************************<->*************************************
 *
 *  ExtendIconList (pIBD, incr);
 *
 *
 *  Description:
 *  -----------
 *  Add space to the icon list
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- ptr to icon box data
 *  incr	- number of cells to add
 *
 * 
 *  Outputs:
 *  -------
 *  Return	- True if successful, False otherwise
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean ExtendIconList (IconBoxData *pIBD, int incr)
{
    Boolean rval;
    int newSize;
    IconInfo *pTmp;

    newSize = pIBD->IPD.totalPlaces + incr;

    if ((pTmp = (IconInfo *) XtMalloc (newSize*sizeof(IconInfo))) != NULL)
    {
	/* copy data */
	memcpy (pTmp, pIBD->IPD.placeList, 
	    pIBD->IPD.totalPlaces*sizeof(IconInfo));
	memset (&pTmp[pIBD->IPD.totalPlaces], 0, incr*sizeof(IconInfo));

	/* out with the old, in with the new */
	XtFree ((char *)pIBD->IPD.placeList);
	pIBD->IPD.placeList = pTmp;
	pIBD->IPD.totalPlaces = newSize;
	rval = True;
    }
    else
    {
	rval = False;
    }

    return (rval);
} /* END OF FUNCTION ExtendIconList */



/*************************************<->*************************************
 *
 *  PackIconBox(pIBD, packVert, packHorz, passedInWidth, passedInHeight)
 *
 *
 *  Description:
 *  -----------
 *  Packs the icons in the icon box
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- pointer to icon box data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void PackIconBox (IconBoxData *pIBD, Boolean packVert, Boolean packHorz, int passedInWidth, int passedInHeight)
{
    IconInfo *pII_2, *pII_1;
    int ix1, ix2;
    int count;
    int newX, newY;
    ClientData *pCD_tmp, *pMyCD;
    int hasActiveText = 1;
    Arg args[4];
    Dimension majorDimension, minorDimension;
    Dimension oldWidth, oldHeight;
    int newWidth, newHeight;
    int i;
    Boolean rippling = False;
#ifdef WSM
    WsClientData *pWsc;
    WmWorkspaceData *pWS;
#endif /* WSM */

    i = 0;
    XtSetArg (args[i], XmNwidth, (XtArgVal) &oldWidth ); i++;
    XtSetArg (args[i], XmNheight, (XtArgVal) &oldHeight ); i++;
    XtGetValues (pIBD->bBoardWidget, args, i);

    /*
     * packing to visual space, first update IconBoxData
     */

    i = 0;

    if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
    {
	XtSetArg (args[i], XmNwidth, (XtArgVal) &majorDimension ); i++;
	XtSetArg (args[i], XmNheight, (XtArgVal) &minorDimension ); i++;
	XtGetValues (pIBD->clipWidget, args, i);
	if (packVert)
	{
	    majorDimension = passedInWidth;
	}
	
	minorDimension /= (Dimension) pIBD->pCD_iconBox->heightInc;
	majorDimension /= (Dimension) pIBD->pCD_iconBox->widthInc;
	if (majorDimension != pIBD->IPD.placementCols)
	{
	    pIBD->IPD.placementCols = majorDimension;
            if (pIBD->IPD.placementCols == 0) pIBD->IPD.placementCols++;
	}
    }
    else
    {
	XtSetArg (args[i], XmNheight, (XtArgVal) &majorDimension ); i++;
	XtSetArg (args[i], XmNwidth, (XtArgVal) &minorDimension ); i++;
	XtGetValues (pIBD->clipWidget, args, i);
	if (packHorz)
	{
	    majorDimension = passedInHeight;
	}

	minorDimension /= (Dimension) pIBD->pCD_iconBox->widthInc;
	majorDimension /= (Dimension) pIBD->pCD_iconBox->heightInc;
	if (majorDimension != pIBD->IPD.placementRows)
	{
	    pIBD->IPD.placementRows = majorDimension;
            if (pIBD->IPD.placementRows == 0) pIBD->IPD.placementRows++;
	}
    }

    /* 
     * find context of the activeIconTextWin to get pCD and then 
     * if it is the same as this client, hide it.
     */

    pMyCD = pIBD->pCD_iconBox;
    if (ICON_DECORATION(pMyCD) & ICON_ACTIVE_LABEL_PART)
    {
	if (XFindContext (DISPLAY, pMyCD->pSD->activeIconTextWin,
			wmGD.windowContextType, (caddr_t *)&pCD_tmp))
	{
	    hasActiveText = 0;
	}
    }

    pII_2 = pII_1 = pIBD->IPD.placeList;
    ix1 = ix2 = 0;
    count = pIBD->IPD.totalPlaces;

    while (ix1 < count)
    {
	if (!rippling && (pII_2->pCD != NULL))
	{
	    /* 
	     * We need to start rippling the icons into new positions if
	     * their (x,y) position changed 
	     */
#ifdef WSM
	    pWS = GetWorkspaceData (pII_2->pCD->pSD, pIBD->wsID);
	    pWsc = GetWsClientData (pWS, pII_2->pCD);
	    CvtIconPlaceToPosition (&pIBD->IPD, pWsc->iconPlace,
		&newX, &newY);

	    rippling = ((newX != pWsc->iconX) ||
		        (newY != pWsc->iconY));
#else /* WSM */
	    CvtIconPlaceToPosition (&pIBD->IPD, pII_2->pCD->iconPlace,
		&newX, &newY);

	    rippling = ((newX != pII_2->pCD->iconX) ||
		        (newY != pII_2->pCD->iconY));
#endif /* WSM */
	}

	if ((pII_2->pCD == NULL) || rippling)
	{
	    /* find next one to move */
	    while ((ix1 < count) && (pII_1->pCD == NULL))
	    {
		ix1++;
		pII_1++;
	    }

	    if ((ix1 < count) && (pII_1->pCD != NULL))
	    {
		if (ix1 != ix2)
		{
		    MoveIconInfo (&pIBD->IPD, ix1, ix2);
		}

		CvtIconPlaceToPosition (&pIBD->IPD, ix2, &newX, &newY);

#ifdef WSM
		pWS = GetWorkspaceData (pII_2->pCD->pSD, pIBD->wsID);
		pWsc = GetWsClientData (pWS, pII_2->pCD);
		pWsc->iconX = newX;
	 	pWsc->iconY = newY;
#else /* WSM */
		pII_2->pCD->iconX = newX;
	 	pII_2->pCD->iconY = newY;
#endif /* WSM */

		if (hasActiveText && (pII_2->pCD == pCD_tmp))
		{
		    /* hide activeIconTextWin first */
		    HideActiveIconText ((WmScreenData *)NULL);
		    XtMoveWidget (pII_2->theWidget, newX, newY);
		    ShowActiveIconText (pII_2->pCD);
		}
		else
		{
		    XtMoveWidget (pII_2->theWidget, newX, newY);
		}
	    }
	}

	if (ix1 < count)
	{
	    ix2++;
	    pII_2++;
	}

	ix1++;
	pII_1++;
    }

    /* update last row and col */

    SetNewBounds (pIBD);

    /* resize Bulletin board  (so scroll bars show correctly */
    i = 0;
    if (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)
    {
	if (majorDimension <= (Dimension) (pIBD->lastCol + 1))
	{
	    newWidth = (pIBD->lastCol + 1) * pIBD->pCD_iconBox->widthInc;    
	    XtSetArg (args[i], XmNwidth, (XtArgVal) newWidth); i++;
	}
	else
	{
	    newWidth = oldWidth;
	}

	if (minorDimension <= (Dimension) (pIBD->lastRow + 1))
	{
	    /* set height of bboard */
	    newHeight = (pIBD->lastRow + 1) * pIBD->pCD_iconBox->heightInc;
	    XtSetArg (args[i], XmNheight, (XtArgVal) newHeight ); i++;
	}
	else
	{
	    newHeight = minorDimension * pIBD->pCD_iconBox->heightInc;
	    XtSetArg (args[i], XmNheight, (XtArgVal) newHeight ); i++;
	}
    }
    else
    {
	if (majorDimension <= (Dimension) (pIBD->lastRow + 1))
	{
	    newHeight = (pIBD->lastRow + 1) * pIBD->pCD_iconBox->heightInc;
	    XtSetArg (args[i], XmNheight, (XtArgVal) newHeight ); i++;
	}
	else
	{
	    newHeight = oldHeight;
	}

	if (minorDimension <= (Dimension) (pIBD->lastCol + 1))
	{
	    /* set width of bboard */
	    newWidth = (pIBD->lastCol + 1) * pIBD->pCD_iconBox->widthInc;    
	    XtSetArg (args[i], XmNwidth, (XtArgVal) newWidth); i++;
	}
	else
	{
	    newWidth = minorDimension * pIBD->pCD_iconBox->widthInc;    
	    XtSetArg (args[i], XmNwidth, (XtArgVal) newWidth); i++;
	}
    }

    if (i > 0  &&  ExpandVirtualSpace(pIBD, newWidth, newHeight))
    {
	XtSetValues (pIBD->bBoardWidget, args, i);
    }


    /* reset max size for icon box */

    ResetIconBoxMaxSize (pIBD->pCD_iconBox, pIBD->bBoardWidget);
    
    ResetArrowButtonIncrements (pIBD->pCD_iconBox);    

} /* END FUNCTION PackIconBox */


/*************************************<->*************************************
 *
 *  RealignIconList (pIBD, newRows, newCols)
 *
 *
 *  Description:
 *  -----------
 *  Realigns the icon list according to the new virtual space dimensions 
 *
 *
 *  Inputs:
 *  ------
 *  pIBD	- ptr to icon box data
 *  newRows	- new number of rows
 *  newCols	- new number of columns
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o The placement data structure contains the old values.
 * 
 *************************************<->***********************************/

void RealignIconList (IconBoxData *pIBD, int newCols, int newRows)
{
    int c1, c2, ix1, ix2;
    int oldRows, oldCols;
    IconPlacementData  ipdNew;
    IconInfo *pII;

    /* 
     * create new icon placement data for ease of calling conversion 
     * routines.
     */
    ipdNew.onRootWindow = pIBD->IPD.onRootWindow;
    ipdNew.iconPlacement = pIBD->IPD.iconPlacement;
    ipdNew.placementRows = newRows;
    ipdNew.placementCols = newCols;
    ipdNew.iPlaceW = pIBD->IPD.iPlaceW;
    ipdNew.iPlaceH = pIBD->IPD.iPlaceH;
    ipdNew.placeList = pIBD->IPD.placeList;
    ipdNew.totalPlaces = pIBD->IPD.totalPlaces;

    oldRows = pIBD->IPD.placementRows;
    oldCols = pIBD->IPD.placementCols;

    /*
     * Use the new organization and placement discipline to
     * determine how to move the icon info data around.
     */
    if (((oldRows < newRows) && 
	 (pIBD->IPD.iconPlacement & ICON_PLACE_TOP_PRIMARY)) ||
	((oldCols < newCols) && 
	 (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)))
    { 
    /* 
     * work backwards 
     */
	for (ix1 = pIBD->IPD.totalPlaces - 1, 
		 pII = &pIBD->IPD.placeList[ix1]; ix1 >= 0; ix1--, pII--)
	{
	    if (pII->pCD != NULL)
	    {
		CvtIconPlaceToPosition (&pIBD->IPD, ix1, &c1, &c2);
		ix2 = CvtIconPositionToPlace (&ipdNew, c1, c2);
		if (ix1 != ix2)
		{ 
		    MoveIconInfo (&pIBD->IPD, ix1, ix2);
		} 
	    }
	}
    }
    else 
    if (((oldRows > newRows) && 
	 (pIBD->IPD.iconPlacement & ICON_PLACE_TOP_PRIMARY)) ||
	((oldCols > newCols) && 
	 (pIBD->IPD.iconPlacement & ICON_PLACE_LEFT_PRIMARY)))
    {
	/* 
	 * work forwards 
	 */
	for (ix1 = 0, pII = &pIBD->IPD.placeList[ix1]; 
		ix1 < pIBD->IPD.totalPlaces; ix1++, pII++)
	{
	    if (pII->pCD != NULL)
	    {
		CvtIconPlaceToPosition (&pIBD->IPD, ix1, &c1, &c2);
		ix2 = CvtIconPositionToPlace (&ipdNew, c1, c2);
		if (ix1 != ix2)
		{
		    MoveIconInfo (&pIBD->IPD, ix1, ix2);
		}
	    }
	}
    }

    /* 
     * update info in placement structure to reflect new reality
     */
    pIBD->IPD.placementRows = newRows;
    pIBD->IPD.placementCols = newCols;

} /* END OF FUNCTION RealignIconList */




/*************************************<->*************************************
 *
 *  SetNewBounds (pIBD)
 *
 *
 *  Description:
 *  -----------
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

void SetNewBounds (IconBoxData *pIBD)
{

    int i;
    int X = 0;
    int Y = 0; 
    CompositeWidget cw;
    WidgetList      children;

    cw = (CompositeWidget) pIBD->bBoardWidget;
    children = cw->composite.children;

    for (i = 0; i < cw->composite.num_children; i++)
    {
        if (children[i]->core.x > X)
        {
	    X = children[i]->core.x;
        }
        if (children[i]->core.y > Y)
        {
	    Y = children[i]->core.y;
        }
    }

    pIBD->lastCol = X / pIBD->pCD_iconBox->widthInc;
    pIBD->lastRow = Y / pIBD->pCD_iconBox->heightInc;

} /* END OF FUNCTION SetNewBounds */



/*************************************<->*************************************
 *
 *  InsertPosition (w)
 *
 *
 *  Description:
 *  -----------
 *  This procedure is passed to the bulletin board at create time
 *  to be used when a child is inserted into the bulletin board
 *  
 *
 *************************************<->***********************************/
Cardinal InsertPosition (Widget w)
{
    return (insertPosition);

} /* END OF FUNCTION InsertPosition */



/*************************************<->*************************************
 *
 *  ShowClientIconState ();
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *************************************<->***********************************/

void ShowClientIconState (ClientData *pCD, int newState)
{

    /*
     * Changing the appearance of an icon window in the box to 
     * reflect the client's state
     */

    if ((newState == MINIMIZED_STATE) && (pCD->iconWindow))
	XMapRaised (DISPLAY, pCD->iconWindow);

    if (((newState == NORMAL_STATE) || (newState == MAXIMIZED_STATE )) 
						&& (pCD->iconWindow))
    {
	XUnmapWindow (DISPLAY, pCD->iconWindow);
    }

} /* END FUNCTION ShowClientIconState */



#ifndef MOTIF_ONE_DOT_ONE
/*************************************<->*************************************
 *
 *  IconScrollVisibleCallback
 *
 *
 *  Description:
 *  -----------
 *  for each icon in the icon box
 * 
 *************************************<->***********************************/

void IconScrollVisibleCallback (Widget w, caddr_t client_data, XmAnyCallbackStruct *call_data)
{
    XmTraverseObscuredCallbackStruct *vis_data;

    vis_data = (XmTraverseObscuredCallbackStruct *) call_data;

    XmScrollVisible(ACTIVE_WS->pIconBox->scrolledWidget,
		    vis_data->traversal_destination,
		    0,0);
/*
		    IB_MARGIN_WIDTH, IB_MARGIN_HEIGHT);
*/
} /* END OF FUNCTION IconScrollVisibleCallback */

#endif


/*************************************<->*************************************
 *
 *  IconActivateCallback
 *
 *
 *  Description:
 *  -----------
 *  for each icon in the icon box
 * 
 *************************************<->***********************************/

void IconActivateCallback (Widget w, caddr_t client_data, XmAnyCallbackStruct *call_data)
{
    ClientData    	*pCD;
    Window		theIcon;

    theIcon = XtWindow(w);

    /* 
     * find context to get pCD and then carry out
     * default action.
     */

    if (!(XFindContext (DISPLAY, theIcon,
			wmGD.windowContextType, (caddr_t *)&pCD)))
    {
	F_Restore_And_Raise ((String)NULL, pCD, (XEvent *)NULL );
/*	F_Normalize_And_Raise ((String)NULL, pCD, (XEvent *)NULL );
*/    }

} /* END OF FUNCTION IconActivateCallback */



/*************************************<->*************************************
 *
 *  UpdateIncrements
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

void UpdateIncrements (Widget sWidget, IconBoxData *pIBD, XConfigureEvent *event)
{
    ResetArrowButtonIncrements (pIBD->pCD_iconBox);    
  
} /* END OF FUNCTION UpdateIncrements */


/*************************************<->*************************************
 *
 *  ResetArrowButtonIncrements(pCD)
 *
 *************************************<->***********************************/

void ResetArrowButtonIncrements (ClientData *pCD)
{
    int i;
    Arg setArgs[2]; 
        
    i=0;
    XtSetArg (setArgs[i], XmNincrement, (XtArgVal) pCD->heightInc); i++;    
    XtSetValues (pCD->thisIconBox->vScrollBar, (ArgList) setArgs, i); 

    i=0;
    XtSetArg (setArgs[i], XmNincrement, (XtArgVal) pCD->widthInc); i++;    
    XtSetValues (pCD->thisIconBox->hScrollBar, (ArgList) setArgs, i);     
    
} /* END OF FUNCTION ResetArrowButtonIncrements */    



/*************************************<->*************************************
 *
 *  ChangeActiveIconboxIconText
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 * 
 *************************************<->***********************************/

void ChangeActiveIconboxIconText (Widget icon, caddr_t dummy, XFocusChangeEvent *event)
{

    ClientData    	*pCD;
    Window		theIcon;

    /* 
     * find context to get pCD and then hide or show active icon text. 
     * Show/hide the active icon text only if the icon box is not 
     * iconified.
     */

    theIcon = XtWindow(icon);

    if (!(XFindContext (DISPLAY, theIcon,
			wmGD.windowContextType, (caddr_t *)&pCD)) &&
	P_ICON_BOX(pCD) &&
	P_ICON_BOX(pCD)->pCD_iconBox &&
	P_ICON_BOX(pCD)->pCD_iconBox->clientState !=  MINIMIZED_STATE)
    {
	if (event->type == FocusIn) 
	{
	    if (event->send_event)
	    {
	       ShowActiveIconText (pCD);
	    }
	}
	else
	{
	    if (event->send_event)
	    {
	        HideActiveIconText (pCD->pSD);
	    }
	}
    }

} /* END OF FUNCTION ChangeActiveIconboxIconText */



/*************************************<->*************************************
 *
 *  HandleIconBoxIconKeyPress
 *
 *
 *  Description:
 *  -----------
 *  This event handler catches keyevents for icons in the icon box and
 *  passes them on to the standard key handling routine for mwm.
 *
 *************************************<->***********************************/

void HandleIconBoxIconKeyPress (Widget icon, caddr_t dummy, XKeyEvent *keyEvent)
{
    
    Context context; 
    ClientData          *pCD; 
    Window              theIcon;

    /*
     * find context to get pCD and then post menu show active icon text.
     */
    
    theIcon = XtWindow(icon);
    if (!(XFindContext (DISPLAY, theIcon,
			wmGD.windowContextType, (caddr_t *)&pCD)))
    {
#ifdef WSM
	SetClientWsIndex (pCD);
#endif /* WSM */
	keyEvent->window = ICON_FRAME_WIN(pCD);

	if (pCD->clientState == MINIMIZED_STATE)
	{
	    context = F_SUBCONTEXT_IB_IICON;
	    pCD->grabContext = F_SUBCONTEXT_IB_IICON;
	}
	else
	{
	    context = F_SUBCONTEXT_IB_WICON;
	    pCD->grabContext = F_SUBCONTEXT_IB_WICON;
	}
	
	if(!(HandleKeyPress (keyEvent, ACTIVE_PSD->keySpecs, 
			True, context, False, pCD)))
	{
	    keyEvent->window = 0;
	    keyEvent->type = 0;
	}

    }
	
} /* END OF FUNCTION HandleIconBoxIconKeyPress */


/*************************************<->*************************************
 *
 *  HandleIconBoxButtonMotion
 *
 *
 *  Description:
 *  -----------
 *  Event handler for button motion events on icon frame window in 
 *  in icon box.
 *
 *
 *  Inputs:
 *  ------
 *  icon		- widget for icon frame 
 *  client_data		- extra client data
 *  pev			- ptr to event
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This is added to make sure that ButtonXMotion gets added to the 
 *    event mask for icons in the icon box.
 * 
 *************************************<->***********************************/

void HandleIconBoxButtonMotion (Widget icon, caddr_t client_data, XEvent *pev)
{

} /* END OF FUNCTION HandleIconBoxButtonMotion */



/*************************************<->*************************************
 *
 *  GetIconBoxIconRootXY (pCD, pX, pY)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pCD         - pointer to client data
 *  pX          - pointer to X return value
 *  pY          - pointer to Y return value
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o returns root-window coords
 *
 *************************************<->***********************************/
void GetIconBoxIconRootXY (ClientData *pCD, int *pX, int *pY)
{

    Window child;
#ifdef WSM
    WsClientData *pWsc = GetWsClientData (pCD->pSD->pActiveWS, pCD);
#endif /* WSM */

#ifdef WSM
    if (pCD->pSD->useIconBox && pWsc->pIconBox)
#else /* WSM */
    if (pCD->pSD->useIconBox && P_ICON_BOX(pCD))
#endif /* WSM */
    {
#ifdef WSM
        XTranslateCoordinates(DISPLAY,
                              XtWindow(pWsc->pIconBox->bBoardWidget),
                              ROOT_FOR_CLIENT(pCD),
                              pWsc->iconX + IB_MARGIN_WIDTH,
                              pWsc->iconY + IB_MARGIN_HEIGHT,
                              pX, pY, &child);
#else /* WSM */
        XTranslateCoordinates(DISPLAY,
                              XtWindow(P_ICON_BOX(pCD)->bBoardWidget),
                              ROOT_FOR_CLIENT(pCD),
                              ICON_X(pCD) + IB_MARGIN_WIDTH,
                              ICON_Y(pCD) + IB_MARGIN_HEIGHT,
                              pX, pY, &child);
#endif /* WSM */

    }
    else
    {
        *pX = *pY = 0;
    }
} /* END FUNCTION GetIconBoxIconRootXY */


/*************************************<->*************************************
 *
 *  IconVisible (pCD)
 *
 *
 *  Description:
 *  -----------
 *
 *  Inputs:
 *  ------
 *  pCD         - pointer to client data
 *
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
Boolean IconVisible (ClientData *pCD)
{

    /*
     * May use icon->core.visible field if that gets fixed and
     * we want to accept the Intrinsics idea of what is visible.
     */

    Boolean rval = True;
#ifdef WSM
    WsClientData *pWsc = GetWsClientData (pCD->pSD->pActiveWS, pCD);
#endif /* WSM */


    
    int i;
    Arg getArgs[5];

    Dimension tmpWidth = 0;
    Dimension tmpHeight = 0;
    Position clipX = 0;
    Position clipY = 0;
    Position tmpX = 0;
    Position tmpY = 0;
    int iconX, iconY;

    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &tmpWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &tmpHeight ); i++;
    XtSetArg (getArgs[i], XmNx, (XtArgVal) &tmpX ); i++;
    XtSetArg (getArgs[i], XmNy, (XtArgVal) &tmpY ); i++;
#ifdef WSM
    XtGetValues (pWsc->pIconBox->clipWidget, getArgs, i);
    XtTranslateCoords(pWsc->pIconBox->scrolledWidget,
                        tmpX, tmpY,
                        &clipX, &clipY);
#else /* WSM */
    XtGetValues (P_ICON_BOX(pCD)->clipWidget, getArgs, i);
    XtTranslateCoords(P_ICON_BOX(pCD)->scrolledWidget,
                        tmpX, tmpY,
                        &clipX, &clipY);
#endif /* WSM */

    GetIconBoxIconRootXY(pCD, &iconX, &iconY);


    /* 
     * demand at least 2 pixels of the 
     * real icon (not drawnButton) be showing 
     */
       
    if (iconX + 2 > ((int)clipX + (int)tmpWidth))
    {
	return(False);
    }
    if (iconY + 2 > ((int)clipY + (int)tmpHeight))
    {
	return(False);
    }

    if ((iconX + (ICON_WIDTH(pCD) -2)) < (int)clipX)
    {
	return(False);
    }
    if ((iconY + (ICON_HEIGHT(pCD) -2)) < (int)clipY)
    {
	return(False);
    }

    return(rval);

} /* END OF FUNCTION IconVisible */

/*************************************<->*************************************
 *
 *  WmXmStringToString (xmString
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
 *
 *  Comments:
 *  --------
 *  Return the ascii part of the first segment of an XmString 
 *  If xmString is NULL, then do nothing 
 *
 *************************************<->***********************************/

String WmXmStringToString (XmString xmString)
{ 
    XmStringContext       xmStrContext;
    char                 *asciiString = NULL; 
    XmStringCharSet      ibTitleCharset; 
    XmStringDirection    ibTitleDirection; 
    Boolean              separator; 
    
    if (xmString)
    {
	XmStringInitContext (&xmStrContext, xmString);
    
	XmStringGetNextSegment (xmStrContext, &asciiString, 
				&ibTitleCharset, &ibTitleDirection, 
				&separator);

	if (ibTitleCharset != NULL) 
	{
	    XtFree ((char *)ibTitleCharset);
	}

	XmStringFreeContext (xmStrContext);
    }
    
    return(asciiString);
    
} /* END OF FUNCTION WmXmStringToString */
