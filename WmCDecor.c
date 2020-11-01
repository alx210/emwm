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
static char rcsid[] = "$XConsortium: WmCDecor.c /main/7 1996/06/20 09:38:16 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987,1988,1989,1990,1991,1993 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmXSMP.h"


#include <X11/cursorfont.h>
#include <Xm/Xm.h>



/* 
 * Definitions
 */

/*
 * include extern functions
 */
#include "WmCDecor.h"
#include "WmCDInfo.h"
#include "WmError.h"
#include "WmGraphics.h"
#include "WmIconBox.h"
#include "WmMenu.h"
#include "WmWinInfo.h"


/*
 * Global Variables:
 */
typedef struct {
    int external;		/* bevel from frame to root */
    int join;			/* bevel between frame components */
    int internal;		/* bevel from frame to client */
} Single_Bevel_Count;

typedef struct {
    Single_Bevel_Count top;
    Single_Bevel_Count bottom;
} Bevel_Count;

/* 
 * "Worst case" bevel counts for frame pieces: this structure is 
 * indexed by definitions in WmGlobal.h. Edit if they change!
 *
 * These counts are multiplied by the internal, external,
 * and join bevel resources to determine the sizes of dynamic
 * data structures to allocate. 
 *
 */
static Bevel_Count Bevels[] =
{
    { {0, 0, 0}, {0, 0, 0} },		/* FRAME_NONE */
    { {0, 0, 0}, {0, 0, 0} },		/* FRAME_CLIENT */
    { {0, 4, 0}, {0, 3, 1} },		/* FRAME_SYSTEM */
    { {0, 2, 0}, {0, 1, 1} },		/* FRAME_TITLE */
    { {0, 4, 0}, {0, 3, 1} },		/* FRAME_MINIMIZE */
    { {0, 4, 0}, {0, 3, 1} },		/* FRAME_MAXIMIZE */
    { {2, 0, 0}, {0, 2, 2} },		/* FRAME_RESIZE_NW */
    { {1, 1, 0}, {0, 1, 1} },		/* FRAME_RESIZE_N */
    { {1, 1, 1}, {1, 1, 1} },		/* FRAME_RESIZE_NE */
    { {0, 1, 1}, {1, 1, 0} },		/* FRAME_RESIZE_E */
    { {0, 2, 2}, {2, 0, 0} },		/* FRAME_RESIZE_SE */
    { {0, 1, 1}, {1, 1, 0} },		/* FRAME_RESIZE_S */
    { {1, 1, 1}, {1, 1, 1} },		/* FRAME_RESIZE_SW */
    { {1, 1, 0}, {0, 1, 1} }		/* FRAME_RESIZE_W */
};






/*************************************<->*************************************
 *
 *  FrameWindow (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Build a decorated frame for a client window and reparent the client 
 *  window to the frame.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data structure for window
 *
 *		  << Need the following member data >>
 *
 *		  client
 *                fields from WM_HINTS property
 *		  fields from WM_CLASS property
 *		  fields from WM_NORMAL_HINTS property
 *		  clientX
 *		  clientY
 *		  clientWidth
 *		  clientHeight
 *		  fields from WM_NAME property
 * 
 * 
 *  Outputs:
 *  -------
 *
 * 
 *  Comments:
 *  --------
 *  This will create a top level shell (frame), fill in the appropriate 
 *  decoration, and reparent the window (in *pcd) to the frame.
 * 
 *************************************<->***********************************/

Boolean FrameWindow (ClientData *pcd)
{
    if (!ConstructFrame (pcd))		/* window hierarchy for frame */
    {
	return(FALSE);
    }

    GenerateFrameDisplayLists (pcd);	/* graphics for frame decoration */

    AdoptClient(pcd);			/* reparent the window */

#ifndef NO_SHAPE
    /* shape the frame */
    if (wmGD.hasShape && pcd->wShaped)
    {
        SetFrameShape (pcd);
    }
#endif /* NO_SHAPE */

    return(TRUE);

} /* END OF FUNCTION FrameWindow */



/*************************************<->*************************************
 *
 *  FrameExposureProc (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Repaint the frame graphics
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  Assumes that the display lists for the frame graphics are already
 *  set up.
 * 
 *************************************<->***********************************/

void FrameExposureProc (ClientData *pcd)
{
    GC topGC, botGC;
    Window win = pcd->clientFrameWin;

    /* use "active" GCs if we have keyboard focus */

    if (pcd == wmGD.keyboardFocus) {
	topGC = CLIENT_APPEARANCE(pcd).activeTopShadowGC;
	botGC = CLIENT_APPEARANCE(pcd).activeBottomShadowGC;
    }
    else {
	topGC = CLIENT_APPEARANCE(pcd).inactiveTopShadowGC;
	botGC = CLIENT_APPEARANCE(pcd).inactiveBottomShadowGC;
    }

    /* draw the frame decoration */

    if (pcd->pclientTopShadows)  {
	XFillRectangles (DISPLAY, 
			 win, 
			 topGC,
			 pcd->pclientTopShadows->prect,
			 pcd->pclientTopShadows->used);
    }

    if (pcd->pclientBottomShadows) { 
	XFillRectangles (DISPLAY,
			 win,
			 botGC,
			 pcd->pclientBottomShadows->prect,
			 pcd->pclientBottomShadows->used);
    }

    if (DECOUPLE_TITLE_APPEARANCE(pcd) && 
	(pcd->decor & MWM_DECOR_TITLE))
    {
	if (pcd == wmGD.keyboardFocus) {
	    topGC = CLIENT_TITLE_APPEARANCE(pcd).activeTopShadowGC;
	    botGC = CLIENT_TITLE_APPEARANCE(pcd).activeBottomShadowGC;
	}
	else {
	    topGC = CLIENT_TITLE_APPEARANCE(pcd).inactiveTopShadowGC;
	    botGC = CLIENT_TITLE_APPEARANCE(pcd).inactiveBottomShadowGC;
	}

	if (pcd->pclientTitleTopShadows)  {
		XFillRectangles (DISPLAY, 
			     pcd->clientTitleWin, 
			     topGC,
			     pcd->pclientTitleTopShadows->prect,
			     pcd->pclientTitleTopShadows->used);
	}

	if (pcd->pclientTitleBottomShadows) { 
	    XFillRectangles (DISPLAY,
			     pcd->clientTitleWin,
			     botGC,
			     pcd->pclientTitleBottomShadows->prect,
			     pcd->pclientTitleBottomShadows->used);
	}
    }

    /* draw the title bar text */
    DrawWindowTitle(pcd, False);
}



/*************************************<->*************************************
 *
 *  BaseWinExposureProc (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Repaint the beveled matte graphics if any.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  Assumes that the display lists for the matte graphics are already
 *  set up.
 * 
 *************************************<->***********************************/

void BaseWinExposureProc (ClientData *pcd)
{
    /* bevel the matte (if there is one) */

    if (pcd->matteWidth > 0) {

	if (pcd->pclientMatteTopShadows) {
	    XFillRectangles (DISPLAY,
			     pcd->clientBaseWin,
			     pcd->clientMatteTopShadowGC,
			     pcd->pclientMatteTopShadows->prect,
			     pcd->pclientMatteTopShadows->used);
	}

	if (pcd->pclientMatteBottomShadows) {
	    XFillRectangles (DISPLAY,
			     pcd->clientBaseWin,
			     pcd->clientMatteBottomShadowGC,
			     pcd->pclientMatteBottomShadows->prect,
			     pcd->pclientMatteBottomShadows->used);
	}
    }
}



/*************************************<->*************************************
 *
 *  ConstructFrame (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Construct the window hierarchy for the frame
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  pcd		- modified
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean ConstructFrame (ClientData *pcd)
{
    unsigned long 	 decoration = pcd->decor;
    unsigned int 	 wclass;		/* window class */
    unsigned long 	 attr_mask;
    XSetWindowAttributes window_attribs;
    int 		 frmX, frmY;

    /* set frame information */
    SetFrameInfo (pcd);

    /* allocate space */
    if (!AllocateFrameDisplayLists(pcd)) {
	return(FALSE);
    }

    /* create frame window  */

    attr_mask =  CWEventMask;
    window_attribs.event_mask = (ButtonPressMask | ButtonReleaseMask |
				 SELECT_BUTTON_MOTION_MASK | 
				 DMANIP_BUTTON_MOTION_MASK |
				 ExposureMask);

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
	(wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER))
    {
	window_attribs.event_mask |= EnterWindowMask | LeaveWindowMask;
    }

    /* 
     * Use background pixmap if one is specified, otherwise set the
     * appropriate background color. 
     */

    if (CLIENT_APPEARANCE(pcd).backgroundPixmap)
    {
	attr_mask |= CWBackPixmap;
	window_attribs.background_pixmap =
		CLIENT_APPEARANCE(pcd).backgroundPixmap;
    }
    else
    {
	attr_mask |= CWBackPixel;
	window_attribs.background_pixel = CLIENT_APPEARANCE(pcd).background;
    }

    attr_mask |= CWCursor;
    window_attribs.cursor = wmGD.workspaceCursor;

    frmY = pcd->frameInfo.y;
    frmX = pcd->frameInfo.x;

    if (CLIENT_APPEARANCE(pcd).saveUnder &&
	WmGetWindowAttributes (pcd->client) &&
	wmGD.windowAttributes.save_under)
    {
	attr_mask |= CWSaveUnder;
	window_attribs.save_under = True;
    }

	    pcd->clientFrameWin = XCreateWindow(DISPLAY, 
				RootWindow (DISPLAY,
				    SCREEN_FOR_CLIENT(pcd)),
				frmX, 
				frmY, 
				pcd->frameInfo.width, 
			pcd->frameInfo.height, 0, 
			CopyFromParent,InputOutput,CopyFromParent,
			attr_mask, &window_attribs);

    /* create resizing windows with cursors*/
    if (SHOW_RESIZE_CURSORS(pcd) && (decoration & MWM_DECOR_RESIZEH)) {
	CreateStretcherWindows (pcd);
    }

    /* 
     * Create title bar window. If the title bar has its own appearance,
     * or if there is no border around the client area,
     * then we need to create an input/output window to draw in. Otherwise
     * we can use an input-only window (to clip the corner resize windows).
     */
    if (decoration & MWM_DECOR_TITLE) {

	attr_mask = CWCursor;
	window_attribs.cursor = wmGD.workspaceCursor;

	if (DECOUPLE_TITLE_APPEARANCE(pcd)) 
	{
	    /* title bar has a different appearance than rest of frame */
	    wclass = InputOutput;

	    /* need to handle exposure events */
	    attr_mask |= CWEventMask;
	    window_attribs.event_mask = ExposureMask;

	    /* 
	     * Use background pixmap if one is specified, otherwise set the
	     * appropriate background color. 
	     */

	    if (CLIENT_TITLE_APPEARANCE(pcd).backgroundPixmap)
	    {
		attr_mask |= CWBackPixmap;
		window_attribs.background_pixmap =
			    CLIENT_TITLE_APPEARANCE(pcd).backgroundPixmap;
	    }
	    else
	    {
		attr_mask |= CWBackPixel;
		window_attribs.background_pixel = 
			    CLIENT_TITLE_APPEARANCE(pcd).background;
	    }
	}
	else 
	{
	    /* title bar has same appearance as rest of frame */
	    wclass = InputOnly;
	}

	pcd->clientTitleWin = XCreateWindow(DISPLAY, pcd->clientFrameWin,
				(int) pcd->frameInfo.upperBorderWidth, 
				(int) pcd->frameInfo.upperBorderWidth, 
				pcd->frameInfo.width - 
				    2*pcd->frameInfo.upperBorderWidth, 
				pcd->frameInfo.titleBarHeight, 
				0, 
				CopyFromParent,wclass,CopyFromParent,
				attr_mask, &window_attribs);
    }

    /* generate gadget position search structure */
    if (!AllocateGadgetRectangles (pcd))
	return(FALSE);
    ComputeGadgetRectangles (pcd);


    /*
     * Create base window for reparenting. Save rectangle data for use
     * in event dispatching.
     */

    window_attribs.event_mask = (SubstructureRedirectMask |
				 SubstructureNotifyMask |
				 FocusChangeMask);
    if (pcd->matteWidth > 0)
    {
	window_attribs.event_mask |= ExposureMask;
	window_attribs.background_pixel = pcd->matteBackground;
    }
    else
    {
	window_attribs.background_pixel = 
	    CLIENT_TITLE_APPEARANCE(pcd).background;
    }

    attr_mask = CWBackPixel | CWEventMask;

    	pcd->clientBaseWin = XCreateWindow(DISPLAY, pcd->clientFrameWin,
				BaseWindowX (pcd),
				BaseWindowY (pcd),
				BaseWindowWidth (pcd),
				BaseWindowHeight (pcd),
				0, 
				CopyFromParent,InputOutput,CopyFromParent,
				attr_mask, &window_attribs);

    /* map all subwindows of client frame */

    XMapSubwindows(DISPLAY, pcd->clientFrameWin);
    return(TRUE);
}




/*************************************<->*************************************
 *
 *  GenerateFrameDisplayLists (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Set up the graphic decorations for the frame
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data record
 *
 * 
 *  Outputs:
 *  -------
 *  pcd		- modified
 *
 *
 *  Comments:
 *  --------
 *  o This must be called after ConstructFrame to insure that the memory
 *    for the rectangles has been allocated.
 *  o If cnum values for StretcherCorner change, also change
 *    StretcherCorner() in WmGraphics.c
 *  o The variable internalBevel sets the depth of shadowing from the 
 *    frame to the client area.
 *  o The variable insideBevel is used to decide how deep the bevel is
 *    immediately inside the frame. This may not be internalBevel if
 *    there's a matte, for example.
 *  o The variable diffBevel stores the difference between insideBevel
 *    and what's needed so the bottom of the title bar is correctly 
 *    beveled down to the client.
 * 
 *************************************<->***********************************/

void GenerateFrameDisplayLists (ClientData *pcd)
{
    unsigned long decoration   = pcd->decor;
    int           matte_width  = pcd->matteWidth;

    int insideBevel, inset, diffBevel;
    unsigned int nTitleBevel, sTitleBevel, eTitleBevel, wTitleBevel;
    unsigned int meTitleBevel, inWidth;
    int x, y, xAdj, yAdj;
    unsigned int width, height;
    RList *prlTop, *prlBot;

    int jX, jY;
    unsigned int jW, jH;
 
    /* zero out part counts */

    if (pcd->pclientTopShadows)
	pcd->pclientTopShadows->used = 0;
    if (pcd->pclientBottomShadows)
	pcd->pclientBottomShadows->used = 0;

    if (pcd->pclientTitleTopShadows)
	pcd->pclientTitleTopShadows->used = 0;
    if (pcd->pclientTitleBottomShadows)
	pcd->pclientTitleBottomShadows->used = 0;

    if (pcd->pclientMatteTopShadows)
	pcd->pclientMatteTopShadows->used = 0;
    if (pcd->pclientMatteBottomShadows)
	pcd->pclientMatteBottomShadows->used = 0;

    /* adjust inside bevel of gadgetry if there's a matte */
    if ((wmGD.frameStyle == WmRECESSED) && (matte_width > 0))
	insideBevel = JOIN_BEVEL(pcd);
    else
	insideBevel = pcd->internalBevel;

    diffBevel = insideBevel - 1;

    if (decoration & MWM_DECOR_RESIZEH) 
    {
	/* adjust part width/heights if no title bar */
	if ((pcd->internalBevel > 1) && !(decoration & MWM_DECOR_TITLE))
	{
	    inset = 1;
	}
	else
	{
	    inset = 0;
	}

	/*
	 * Draw the stretchers. If the horizontal or vertical pieces
	 * get "too small", then don't draw them at all.
	 */
	GetFramePartInfo (pcd, FRAME_RESIZE_NW, &x, &y, &width, &height);
	StretcherCorner (pcd->pclientTopShadows,	/* NW */
		    pcd->pclientBottomShadows, 
		    x, y, 
		    STRETCH_NORTH_WEST, 
		    pcd->frameInfo.upperBorderWidth - inset, 
		    width, height);

	GetFramePartInfo (pcd, FRAME_RESIZE_N, &x, &y, &width, &height);
	if ((int)width > 0)
	    BevelRectangle (pcd->pclientTopShadows, 	/* N */
		    pcd->pclientBottomShadows, 
		    x, y, 
		    width, height - inset,
		    2, 1, ((wmGD.frameStyle == WmSLAB) ? 0 : 1), 1);

	GetFramePartInfo (pcd, FRAME_RESIZE_NE, &x, &y, &width, &height);
	StretcherCorner (pcd->pclientTopShadows, 
		    pcd->pclientBottomShadows, 
		    x, y,
		    STRETCH_NORTH_EAST, 
		    pcd->frameInfo.upperBorderWidth - inset, width, height);
  
	GetFramePartInfo (pcd, FRAME_RESIZE_E, &x, &y, &width, &height);
	if ((int)height > 0) 
	    BevelRectangle (pcd->pclientTopShadows, 	/* E */
		    pcd->pclientBottomShadows, 
		    x+diffBevel, y, 
		    width-diffBevel, height,
		    1, 2, 1, ((wmGD.frameStyle == WmSLAB) ? 0 : 1));

	GetFramePartInfo (pcd, FRAME_RESIZE_SE, &x, &y, &width, &height);
	StretcherCorner (pcd->pclientTopShadows, 	/* SE */
                    pcd->pclientBottomShadows, 
		    x, y, 
		    STRETCH_SOUTH_EAST, 
		    pcd->frameInfo.upperBorderWidth-inset, width, height);

	GetFramePartInfo (pcd, FRAME_RESIZE_S, &x, &y, &width, &height);
	if ((int) width > 0)
	    BevelRectangle (pcd->pclientTopShadows, 	/* S */
                    pcd->pclientBottomShadows, 
		    x, y+diffBevel, 
		    width, height-diffBevel,
		    ((wmGD.frameStyle == WmSLAB) ? 0 : 1), 1, 2, 1);

	GetFramePartInfo (pcd, FRAME_RESIZE_SW, &x, &y, &width, &height);
	StretcherCorner (pcd->pclientTopShadows, 	/* SW */
		    pcd->pclientBottomShadows, 
		    x, y, 
		    STRETCH_SOUTH_WEST, 
		    pcd->frameInfo.upperBorderWidth-inset, width, height);
  
	GetFramePartInfo (pcd, FRAME_RESIZE_W, &x, &y, &width, &height);
	if ((int) height > 0)
	    BevelRectangle (pcd->pclientTopShadows, 	/* W */
		    pcd->pclientBottomShadows, 
		    x, y, 
		    width-diffBevel, height,
		    1, ((wmGD.frameStyle == WmSLAB) ? 0 : 1), 1, 2);

	if (diffBevel)
	{
	    /*
	     * Draw second inside bevel level. This goes just around the 
	     * client area under the title bar.
	     */
	    BevelRectangle (pcd->pclientBottomShadows, 	/* inside */
			    pcd->pclientTopShadows, 
			    (int) (pcd->frameInfo.lowerBorderWidth-diffBevel), 
			    (int) (pcd->clientOffset.y - diffBevel),
			    pcd->frameInfo.width - 
				2*pcd->frameInfo.lowerBorderWidth + 
				2*diffBevel, 
			    pcd->frameInfo.height - pcd->clientOffset.y - 
			        pcd->frameInfo.lowerBorderWidth + 
			        2*diffBevel,
			    (unsigned int) diffBevel, (unsigned int) diffBevel,
			    (unsigned int) diffBevel, (unsigned int) diffBevel);
	}
    }
    else if (decoration & MWM_DECOR_BORDER)
    {

    /* produce default border with no resizing functions */

#ifdef WSM
	BevelRectangle (pcd->pclientTopShadows, 	/* outside */
		    pcd->pclientBottomShadows, 
		    0, 0, 
		    pcd->frameInfo.width, pcd->frameInfo.height,
		    FRAME_EXTERNAL_SHADOW_WIDTH,
		    FRAME_EXTERNAL_SHADOW_WIDTH,
		    FRAME_EXTERNAL_SHADOW_WIDTH,
		    FRAME_EXTERNAL_SHADOW_WIDTH);
#else /* WSM */
	BevelRectangle (pcd->pclientTopShadows, 	/* outside */
		    pcd->pclientBottomShadows, 
		    0, 0, 
		    pcd->frameInfo.width, pcd->frameInfo.height,
		    2, 2, 2, 2);
#endif /* WSM */

	if ((pcd->internalBevel > 1) &&
	    !matte_width && 
	    (decoration & MWM_DECOR_TITLE)) {
	    /*
	     * Need to do special beveling around the inside of the 
	     * client area separately from around title area.
	     */
	    GetFramePartInfo (pcd, FRAME_TITLE, &x, &y, &width, &height);
	    inset = 1 + (pcd->frameInfo.lowerBorderWidth - 
		         pcd->frameInfo.upperBorderWidth);
	    BevelRectangle (pcd->pclientBottomShadows,
			    pcd->pclientTopShadows, 
			    (int) (pcd->frameInfo.lowerBorderWidth-inset), 
			    (int) (pcd->frameInfo.lowerBorderWidth-inset), 
			    pcd->frameInfo.width - 
				2*pcd->frameInfo.lowerBorderWidth + 2*inset, 
			    pcd->frameInfo.height - 
				2*pcd->frameInfo.lowerBorderWidth + 2*inset,
			    1, 1, 1, 1);

	    BevelRectangle (pcd->pclientBottomShadows, 	/* inside */
			    pcd->pclientTopShadows, 
			    (int) (pcd->frameInfo.lowerBorderWidth-diffBevel), 
			    pcd->clientOffset.y - diffBevel,
			    pcd->frameInfo.width - 
				2*pcd->frameInfo.lowerBorderWidth + 
				2*diffBevel, 
			    pcd->frameInfo.height - pcd->clientOffset.y - 
				pcd->frameInfo.lowerBorderWidth + 2*diffBevel,
			    (unsigned int)diffBevel, (unsigned int)diffBevel,
			    (unsigned int)diffBevel, (unsigned int)diffBevel);
	}
	else 
	{
#ifdef PANELIST
            if((pcd->dtwmBehaviors & DtWM_BEHAVIOR_PANEL) &&
               (pcd->clientDecoration == WM_DECOR_BORDER))
            {
                insideBevel = 0;
            }
#endif /* PANELIST */
	    BevelRectangle (pcd->pclientBottomShadows, 	/* inside */
			    pcd->pclientTopShadows, 
			    (int)(pcd->frameInfo.lowerBorderWidth-insideBevel), 
			    (int)(pcd->frameInfo.lowerBorderWidth-insideBevel), 
			    pcd->frameInfo.width - 
				2*pcd->frameInfo.lowerBorderWidth + 
				2*insideBevel, 
			    pcd->frameInfo.height - 
				2*pcd->frameInfo.lowerBorderWidth + 
				2*insideBevel,
			    (unsigned int)insideBevel, 
			    (unsigned int)insideBevel,
			    (unsigned int)insideBevel, 
			    (unsigned int)insideBevel);
	}
    }


    /* draw title bar */

    /* 
     * set bevels for title bar and parts 
     */
    if (decoration & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {
	nTitleBevel = JOIN_BEVEL(pcd);	/* north side of title */
        if (wmGD.frameStyle == WmSLAB)
	{
	    sTitleBevel = JOIN_BEVEL(pcd);	/* south side of title */
	}
	else
	{
	    sTitleBevel = insideBevel;	/* south side of title */
        }
	eTitleBevel = JOIN_BEVEL(pcd);	/* east side of title */
	wTitleBevel = JOIN_BEVEL(pcd);	/* west side of title */
	meTitleBevel = JOIN_BEVEL(pcd);	/* btw Minimize, Maximize */
    }
    else 
    {
	/* borderless window */

	nTitleBevel = EXTERNAL_BEVEL(pcd);
        if (wmGD.frameStyle == WmSLAB)
	{
	    sTitleBevel = (matte_width > 0) ? JOIN_BEVEL(pcd) : 
						EXTERNAL_BEVEL(pcd);
	}
	else
	{
	    sTitleBevel = (matte_width > 0) ? insideBevel : EXTERNAL_BEVEL(pcd);
        }
	eTitleBevel = (decoration & (MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE))?
			  JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);
	wTitleBevel = (decoration & MWM_DECOR_MENU) ?
			  JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);

	/* beveling east of minimize */
	meTitleBevel = (decoration & (MWM_DECOR_MAXIMIZE)) ?
			  JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);
    }


    if (decoration & MWM_DECOR_TITLE) 
    {
	/*
	 * Use a different set of rectangles if title appearance
	 * is different from the rest of the frame.
	 */
	if (DECOUPLE_TITLE_APPEARANCE(pcd)) 
	{
	    prlTop = pcd->pclientTitleTopShadows;
	    prlBot = pcd->pclientTitleBottomShadows;
	    xAdj = yAdj = pcd->frameInfo.upperBorderWidth;
	}
	else
	{
	    prlTop = pcd->pclientTopShadows;
	    prlBot = pcd->pclientBottomShadows;
	    xAdj = yAdj = 0;
	}


	GetFramePartInfo (pcd, FRAME_TITLE, &x, &y, &width, &height);
	if (pcd->decorFlags & TITLE_DEPRESSED) {
	    /* show depressed title gadget */
	    GetDepressInfo (pcd, FRAME_TITLE, &jX, &jY, &jW, &jH, 
			    &inWidth);
	    BevelDepressedRectangle (prlTop, prlBot, 
			x-xAdj, y-yAdj, width, height,
			nTitleBevel, eTitleBevel, 
			sTitleBevel, wTitleBevel, inWidth);
	}
	else {
	    /* show normal title gadget */
	    BevelRectangle (prlTop, prlBot, 
			x-xAdj, y-yAdj, width, height,
			nTitleBevel, eTitleBevel, 
			sTitleBevel, wTitleBevel);
	}
    }

    if (decoration & MWM_DECOR_MENU) {

	GetFramePartInfo (pcd, FRAME_SYSTEM, &x, &y, &width, &height);
	if (pcd->decorFlags & SYSTEM_DEPRESSED) {
	    /* show depressed system gadget */
	    GetDepressInfo (pcd, FRAME_SYSTEM, &jX, &jY, &jW, &jH, 
			    &inWidth);
	    BevelDepressedRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, wTitleBevel, 
			    sTitleBevel, nTitleBevel, inWidth);
	}
	else
	{
	    /* show normal system gadget */
	    BevelRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, wTitleBevel, 
			    sTitleBevel, nTitleBevel);
	}


	/* system icon */
	BevelSystemButton (prlTop, prlBot, 
			   x-xAdj, y-yAdj, width, height);
    }
		    
    if (decoration & MWM_DECOR_MINIMIZE) { 
	GetFramePartInfo (pcd, FRAME_MINIMIZE, &x, &y, &width, &height);

	if (pcd->decorFlags & MINIMIZE_DEPRESSED) {
	    /* show depressed minimize gadget */
	    GetDepressInfo (pcd, FRAME_MINIMIZE, &jX, &jY, &jW, &jH, 
			    &inWidth);
	    BevelDepressedRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, meTitleBevel, 
			    sTitleBevel, eTitleBevel, inWidth);
	}
	else {
	    /* show normal minimize gadget */
	    BevelRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, meTitleBevel, 
			    sTitleBevel, eTitleBevel);
	}


	BevelMinimizeButton(prlTop, 	/* minimize icon */
			    prlBot, 
			    x-xAdj, y-yAdj, height);
    }

    if (decoration & MWM_DECOR_MAXIMIZE) {
	GetFramePartInfo (pcd, FRAME_MAXIMIZE, &x, &y, &width, &height);


	if (pcd->decorFlags & MAXIMIZE_DEPRESSED) {
	    /* show depressed maximize gadget */
	    GetDepressInfo (pcd, FRAME_MAXIMIZE, &jX, &jY, &jW, &jH, 
			    &inWidth);
	    BevelDepressedRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, nTitleBevel, 
			    sTitleBevel, eTitleBevel, inWidth);
	}
	else {
	    /* show normal maximize gadget */
	    BevelRectangle (prlTop, prlBot, 
			    x-xAdj, y-yAdj, width, height,
			    nTitleBevel, nTitleBevel, 
			    sTitleBevel, eTitleBevel);
	}

	/* maximize icon - in or out depending on client state */
	if (pcd->maxConfig) {
	    BevelMaximizeButton(prlBot, 
				prlTop,
				x-xAdj, y-yAdj, height);
	}
	else {
	    BevelMaximizeButton(prlTop,
				prlBot, 
				x-xAdj, y-yAdj, height);
	}
	
    }

    /* draw the client matte (this is in the base window!!!) */

    if (matte_width > 0) {
	unsigned int mWidth, mHeight, exMatteBevel, tMatteBevel;

	mWidth = BaseWindowWidth (pcd);
	mHeight = BaseWindowHeight (pcd);

	if (decoration & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
	{
	    exMatteBevel = JOIN_BEVEL(pcd);
	    tMatteBevel = JOIN_BEVEL(pcd);
	}
	else 
	{
	    exMatteBevel = EXTERNAL_BEVEL(pcd);
	    tMatteBevel = (decoration & MWM_DECOR_TITLE) ?
			   JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);
	}

	/* set up beveling around the edges */

	BevelRectangle (pcd->pclientMatteTopShadows,
		    pcd->pclientMatteBottomShadows,
		    0, 
		    0,
		    mWidth,
		    mHeight,
		    tMatteBevel, exMatteBevel, 
		    exMatteBevel, exMatteBevel);

	/* reversed beveling on inside rectange ! */

	BevelRectangle ( pcd->pclientMatteBottomShadows,
		    pcd->pclientMatteTopShadows,
		    matte_width - pcd->internalBevel, 
		    matte_width - pcd->internalBevel,
		    mWidth - 2*matte_width + 2*pcd->internalBevel, 
		    mHeight - 2*matte_width + 2*pcd->internalBevel,
		    (unsigned int) pcd->internalBevel, 
		    (unsigned int) pcd->internalBevel, 
		    (unsigned int) pcd->internalBevel, 
		    (unsigned int) pcd->internalBevel);
    }
}


/*************************************<->*************************************
 *
 *  AdoptClient (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Reparent the client window to the window frame
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data record
 *
 * 
 *  Outputs:
 *  -------
 *  None
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void AdoptClient (ClientData *pcd)
{
    XWindowChanges windowChanges;
    unsigned int mask;

    /* Put the window in the window manager's save set */

    if (!(pcd->clientFlags & CLIENT_WM_CLIENTS))
    {
	XChangeSaveSet (DISPLAY, pcd->client, SetModeInsert);
	pcd->clientFlags |= CLIENT_IN_SAVE_SET;
    }

    /*
     * set window geometry to be consistent with what we believe 
     */
    mask = CWWidth | CWHeight;
    windowChanges.width = pcd->clientWidth;
    windowChanges.height = pcd->clientHeight;

    /*
     * strip off previous window border if we're adding our own border
     * or matte
     */
    if ( (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)) ||
	 (pcd->matteWidth > 0) )
    {
	mask |= CWBorderWidth;
	windowChanges.border_width = 0;
    }

    XConfigureWindow (DISPLAY, pcd->client, mask, &windowChanges);

#ifndef NO_SHAPE
    /* shape our frame to match that of the client's window */
    if (wmGD.hasShape)
    {
	int xws, yws, xbs, ybs;
	unsigned wws, hws, wbs, hbs;
	int boundingShaped, clipShaped;
	
	XShapeSelectInput (DISPLAY, pcd->client, ShapeNotifyMask);
	XShapeQueryExtents (DISPLAY, pcd->client,
			    &boundingShaped, &xws, &yws, &wws, &hws,
			    &clipShaped, &xbs, &ybs, &wbs, &hbs);
	pcd->wShaped = boundingShaped;
    }
#endif /* NO_SHAPE  */
    /* reparent the window to the base window */

    XReparentWindow (DISPLAY, pcd->client, pcd->clientBaseWin, 
			pcd->matteWidth, 
			pcd->matteWidth);
    pcd->clientFlags |= CLIENT_REPARENTED;

} /* END OF FUNCTION AdoptClient */



/*************************************<->*************************************
 *
 *  GetTextBox (pcd, pBox)
 *
 *
 *  Description:
 *  -----------
 *  Gets the rectangle that the text should fit into in the title bar
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pBox	- pointer to an XRectangle structure that gets return data
 * 
 *  Outputs:
 *  -------
 *  pBox	- data is returned here
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void GetTextBox (ClientData *pcd, XRectangle *pBox)
{
    int x,y;
    unsigned int width,height;
#ifdef WSM
    Dimension textWidth;
    Dimension offset;
    XmFontList  fontList;
#endif /* WSM */

    /* get size of title area */

    if (!GetFramePartInfo (pcd, FRAME_TITLE, &x, &y, &width, &height))
    {
	/* no title area !!! */
	pBox->x = 0;
	pBox->y = 0;
	pBox->width = 0;
	pBox->height = 0;
        return;
    }

    /* adjust for shadowing and allow for some padding around the edges */
    x += WM_TOP_TITLE_SHADOW + WM_TOP_TITLE_PADDING;
    y += WM_TOP_TITLE_SHADOW + WM_TOP_TITLE_PADDING;

    width -= WM_TOP_TITLE_SHADOW + WM_BOTTOM_TITLE_SHADOW +
	     WM_TOP_TITLE_PADDING + WM_BOTTOM_TITLE_PADDING;
    height -= WM_TOP_TITLE_SHADOW + WM_BOTTOM_TITLE_SHADOW +
	      WM_TOP_TITLE_PADDING + WM_BOTTOM_TITLE_PADDING;

#ifdef DT_LEFT_JUSTIFIED_TITLE
    if (wmGD.frameStyle == WmSLAB)
    {
	/*
	 * We left justify the title in this style.
	 * To keep it a little neat, we offset the title from 
	 * the left edge just a little (half the title height).
	 * See if we have room to do this.
	 */
	if (DECOUPLE_TITLE_APPEARANCE(pcd))
	    fontList = CLIENT_TITLE_APPEARANCE(pcd).fontList;
	else
	    fontList = CLIENT_APPEARANCE(pcd).fontList;
	textWidth = XmStringWidth(fontList, pcd->clientTitle);

	offset = TitleBarHeight(pcd)/2;

	if ((textWidth + offset) <= width)
	{
	    /* We have plenty of room, do the offset */
	    x += offset;
	    width -= offset;
	}
	else if ((short) (width - textWidth) > 0)
	{
	    /* We don't have enough room to do our usual offset,
	     * but if we reduce the offset, the text won't get
	     * clipped.
	     */
	    offset = (width - textWidth) / 2;
	    x += offset;
	    width -= offset;
	}
    }

#endif /* DT_LEFT_JUSTIFIED_TITLE */
    /* return position and size */
    pBox->x = x;
    pBox->y = y;
    pBox->width = width;
    pBox->height = height;

}




/*************************************<->*************************************
 *
 *  DrawWindowTitle (pcd, eraseFirst)
 *
 *
 *  Description:
 *  -----------
 *  Overwrites or replaces the client's title text in the 
 *  title bar of the frame.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  eraseFirst	- if true, then the old title is erased first
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  o Assumes 8-bit text for now.
 *  
 * 
 *************************************<->***********************************/

void DrawWindowTitle (ClientData *pcd, Boolean eraseFirst)
{
    GC clientGC;
    unsigned long decoration = pcd->decor;
    XRectangle textBox;
    Window win;
    XmFontList  fontList;

    /* make sure there is a title bar first */
    if (!(decoration & MWM_DECOR_TITLE))
	return;

    if (DECOUPLE_TITLE_APPEARANCE(pcd))
    {
	/* use "active" GC if we have keyboard focus */
	if (pcd == wmGD.keyboardFocus) {
	    clientGC = CLIENT_TITLE_APPEARANCE(pcd).activeGC;
	}
	else {
	    clientGC = CLIENT_TITLE_APPEARANCE(pcd).inactiveGC;
	}

	/* get the area that the text must fit in */
	GetTextBox (pcd, &textBox);

	/* adjust position to be relative to titlebar window, not frame */
	textBox.x -= (short) pcd->frameInfo.upperBorderWidth;
	textBox.y -= (short) pcd->frameInfo.upperBorderWidth;

	win = pcd->clientTitleWin;
	fontList = CLIENT_TITLE_APPEARANCE(pcd).fontList;
    }
    else 
    {
	/* use "active" GC if we have keyboard focus */
	if (pcd == wmGD.keyboardFocus) {
	    clientGC = CLIENT_APPEARANCE(pcd).activeGC;
	}
	else {
	    clientGC = CLIENT_APPEARANCE(pcd).inactiveGC;
	}

	/* get the area that the text must fit in */
	GetTextBox (pcd, &textBox);
	win = pcd->clientFrameWin;
	fontList = CLIENT_APPEARANCE(pcd).fontList;
    }

    if (eraseFirst)
    {
	XClearArea (DISPLAY, win, textBox.x, textBox.y, 
		(unsigned int) textBox.width, (unsigned int) textBox.height, 
		FALSE);
    }

#ifdef  DT_LEFT_JUSTIFIED_TITLE
    WmDrawXmString(DISPLAY, win, fontList, pcd->clientTitle, clientGC,
		   textBox.x, textBox.y, textBox.width, &textBox,
		   ((wmGD.frameStyle == WmSLAB) ? False : True));
#else /* DT_LEFT_JUSTIFIED_TITLE */
#ifdef WSM
    WmDrawXmString(DISPLAY, win, fontList, pcd->clientTitle, clientGC,
		   textBox.x, textBox.y, textBox.width, &textBox,
		   True);
#else
    WmDrawXmString(DISPLAY, win, fontList, pcd->clientTitle, clientGC,
		   textBox.x, textBox.y, textBox.width, &textBox);
#endif
#endif /* DT_LEFT_JUSTIFIED_TITLE */
		     


} /* END OF FUNCTION DrawWindowTitle */



/*************************************<->*************************************
 *
 *  CreateStretcherWindows (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Create the input-only windows that overlay the resize gadgets.
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data.
 *
 * 
 *  Outputs:
 *  -------
 *  pcd		- modified
 *
 *  Return 	- none
 *
 *
 *  Comments:
 *  --------
 *  o The windows are sized based upon resizeBorderWidth
 *  o This should be called before creating the title bar, 
 *    and reparenting window. Later windows should obscure parts of the 
 *    stretchers.
 *  o The stretchers are given special cursors.
 * 
 *************************************<->***********************************/

void CreateStretcherWindows (ClientData *pcd)
{
    int iWin;
    int x, y;
    unsigned int width, height;
    XSetWindowAttributes win_attribs;
    unsigned long attr_mask;

    for (iWin = 0; iWin < STRETCH_COUNT; iWin++) {
	switch (iWin) {
	    case STRETCH_NORTH_WEST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_NW, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_NORTH:
		    GetFramePartInfo (pcd, FRAME_RESIZE_N, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_NORTH_EAST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_NE, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_EAST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_E, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_SOUTH_EAST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_SE, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_SOUTH:
		    GetFramePartInfo (pcd, FRAME_RESIZE_S, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_SOUTH_WEST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_SW, 
				      &x, &y, &width, &height);
		    break;

	    case STRETCH_WEST:
		    GetFramePartInfo (pcd, FRAME_RESIZE_W, 
				      &x, &y, &width, &height);
		    break;
	}

	attr_mask = CWCursor;
	win_attribs.cursor = wmGD.stretchCursors[iWin];

	pcd->clientStretchWin[iWin] = 
		    XCreateWindow(DISPLAY, pcd->clientFrameWin,
		    x, y, width, height, 0, CopyFromParent,
		    InputOnly, CopyFromParent, attr_mask, &win_attribs);
    }
} /* END OF FUNCTION  CreateStretcherWindows  */



/*************************************<->*************************************
 *
 *  CountFrameRectangles (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Computes the number of top and bottom shadow rectangles to allocate
 *  per frame.
 *
 *  Inputs:
 *  ------
 *  pWS		- pointer to workspace data
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void CountFrameRectangles (WmScreenData *pSD)
{
    int i;

    pSD->Num_Title_Ts_Elements = pSD->Num_Title_Bs_Elements = 0;

    /* count up rectangles for title bar */
    for (i = FRAME_SYSTEM; i <= FRAME_MAXIMIZE; i++)
    {
	pSD->Num_Title_Ts_Elements += ((Bevels[i].top.external * 
				 pSD->externalBevel) +
			  (Bevels[i].top.internal * MAX_INTERNAL_BEVEL) +
			  (Bevels[i].top.join * pSD->joinBevel));

	pSD->Num_Title_Bs_Elements += ((Bevels[i].bottom.external*
				 pSD->externalBevel)+
			  (Bevels[i].bottom.internal * MAX_INTERNAL_BEVEL) +
			  (Bevels[i].bottom.join * pSD->joinBevel));
    }

    pSD->Num_Resize_Ts_Elements = pSD->Num_Resize_Bs_Elements = 0;

    /* count up rectangles for resize handles*/
    for (i = FRAME_RESIZE_NW; i <= FRAME_RESIZE_W; i++)
    {
	pSD->Num_Resize_Ts_Elements += ((Bevels[i].top.external * 
				   pSD->externalBevel) +
			  (Bevels[i].top.internal * MAX_INTERNAL_BEVEL) +
			  (Bevels[i].top.join * pSD->joinBevel));

	pSD->Num_Resize_Bs_Elements += ((Bevels[i].bottom.external*
				   pSD->externalBevel)+
			  (Bevels[i].bottom.internal * MAX_INTERNAL_BEVEL) +
			  (Bevels[i].bottom.join * pSD->joinBevel));
    }
} /* END OF FUNCTION  CountFrameRectangles  */



/*************************************<->*************************************
 *
 *  AllocateFrameDisplayLists (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Allocates memory for the graphic display lists for the frame.
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to the client data
 *
 * 
 *  Outputs:
 *  -------
 *  pcd		- fields modified
 *
 *  Return	- TRUE if successful, FALSE otherwise.
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

Boolean AllocateFrameDisplayLists (ClientData *pcd)
{
    int frame_top_count, frame_bottom_count;

    /*
     *	If the title bar has it's own appearance, then allocate
     *  separate display lists for it. 
     */
    if (DECOUPLE_TITLE_APPEARANCE(pcd) && 
	(pcd->decor & MWM_DECOR_TITLE))
    {
	if (((pcd->pclientTitleTopShadows = 
	      AllocateRList ((unsigned)NUM_TITLE_TS_ELEMENTS(pcd))) == NULL) ||
	    ((pcd->pclientTitleBottomShadows = 
	      AllocateRList ((unsigned)NUM_TITLE_BS_ELEMENTS(pcd))) == NULL))
	{
	    /* out of memory! */
	    Warning (((char *)GETMESSAGE(8, 1, "Insufficient memory for client window framing")));
	    return(FALSE);
	}

	frame_top_count = NUM_RESIZE_TS_ELEMENTS(pcd);
	frame_bottom_count = NUM_RESIZE_BS_ELEMENTS(pcd);
    }
    else
    {
	frame_top_count = NUM_RESIZE_TS_ELEMENTS(pcd) + 
	                  NUM_TITLE_TS_ELEMENTS(pcd);
	frame_bottom_count = NUM_RESIZE_BS_ELEMENTS(pcd) + 
			     NUM_RESIZE_BS_ELEMENTS(pcd);
    }

    /* 
     * Allocate the primary lists for the frame
     */
    if ( (pcd->pclientTopShadows == NULL) &&
	 ((pcd->pclientTopShadows = 
		 AllocateRList ((unsigned)frame_top_count)) == NULL) )
    {
	/* out of memory! */
	Warning (((char *)GETMESSAGE(8, 2, "Insufficient memory for client window framing")));
	return(FALSE);
    }

    if ( (pcd->pclientBottomShadows == NULL) &&
	 ((pcd->pclientBottomShadows = 
		 AllocateRList ((unsigned)frame_bottom_count)) == NULL) )
    {
	/* out of memory! */
	Warning (((char *)GETMESSAGE(8, 3, "Insufficient memory for client window framing")));
	return(FALSE);
    }

    /*
     * Only allocate matte lists if there is a matte.
     */
    if ( (pcd->matteWidth) &&
	 (pcd->pclientMatteTopShadows == NULL) &&
	 ((pcd->pclientMatteTopShadows = 
	     AllocateRList ((unsigned)NUM_MATTE_TS_RECTS)) == NULL))
    {
	/* out of memory! */
	Warning (((char *)GETMESSAGE(8, 4, "Insufficient memory for client window framing")));
	return(FALSE);
    }

    if ( (pcd->matteWidth) &&
	 (pcd->pclientMatteBottomShadows == NULL) &&
	 ((pcd->pclientMatteBottomShadows = 
		 AllocateRList ((unsigned)NUM_MATTE_BS_RECTS)) == NULL)) 
    {
	/* out of memory! */
	Warning (((char *)GETMESSAGE(8, 5, "Insufficient memory for client window framing")));
	return(FALSE);
    }

    return(TRUE);
} /* END OF FUNCTION  AllocateFrameDisplayLists  */


/*************************************<->*************************************
 *
 *  InitClientDecoration (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Initializes client decoration routines
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  This must be called once before decorating any client frames.
 *************************************<->***********************************/

void InitClientDecoration (WmScreenData *pSD)
{
    CountFrameRectangles(pSD);
} /* END OF FUNCTION   InitClientDecoration */



/*************************************<->*************************************
 *
 *  AllocateGadgetRectangles (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Allocate the memory for event rectangles structures.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data structure
 * 
 *  Outputs:
 *  -------
 *  pcd		- modified
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

Boolean AllocateGadgetRectangles (ClientData *pcd)
{
    int num_rects;
    unsigned long decor = pcd->decor;
    GadgetRectangle *pgr;
    
    if (decor & MWM_DECOR_TITLE) {

	/* count how many rectangles to allocate for titlebar */
	num_rects = 1;		
	if (decor & MWM_DECOR_MENU)	num_rects += 1;
	if (decor & MWM_DECOR_MINIMIZE)	num_rects += 1;
	if (decor & MWM_DECOR_MAXIMIZE)	num_rects += 1;
    
	/* allocate memory if no memory is allocated */
	if ( pcd->pTitleGadgets == NULL) {
	    /* allocate memory for these guys */
	    pgr = (GadgetRectangle *) 
		   XtMalloc (num_rects * sizeof(GadgetRectangle));
	    if (pgr == NULL)
	    {
		/* out of memory! */
	        Warning (((char *)GETMESSAGE(8, 6, "Insufficient memory for client window framing")));
		return (FALSE);
	    }

	    /* update client data */
	    pcd->pTitleGadgets = pgr;
	    pcd->cTitleGadgets = 0;
	}
    }

    if (decor & MWM_DECOR_RESIZEH) {

	/* allocate memory if no memory is allocated */
	if ( pcd->pResizeGadgets == NULL) {
	    /* allocate memory for these guys */
	    pgr = (GadgetRectangle *) 
		  XtMalloc (STRETCH_COUNT * sizeof(GadgetRectangle));
	    if (pgr == NULL) 
	    {
		/* out of memory! */
	        Warning (((char *)GETMESSAGE(8, 7, "Insufficient memory for client window framing")));
		return (FALSE);
	    }

	    /* update client data */
	    pcd->pResizeGadgets = pgr;
	}
    }
    return(TRUE);
} /* END OF FUNCTION  AllocateGadgetRectangles  */


/*************************************<->*************************************
 *
 *  ComputeGadgetRectangles (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Creates the event rectangles structures to aid in identifying
 *  frame parts when events come in
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data structure
 * 
 *  Outputs:
 *  -------
 *  pcd		- modified
 *
 *
 *  Comments:
 *  --------
 *  o assumes gadget rectangles are already allocated.
 * 
 *************************************<->***********************************/

void ComputeGadgetRectangles (ClientData *pcd)
{
    unsigned long decor = pcd->decor;
    GadgetRectangle *pgr;
    int fpX, fpY;
    unsigned int fpWidth, fpHeight;
    int igr;
    int clientWidth = (pcd->maxConfig) ? pcd->maxWidth : pcd->clientWidth;
    

    /* title bar */

    if (decor & MWM_DECOR_TITLE) {

	if ( (pgr = pcd->pTitleGadgets) == NULL) {
	    return;		/* nothing there !!! */
	}

	/* do title rectangle */
	pcd->titleRectangle.x = pcd->frameInfo.upperBorderWidth;
	pcd->titleRectangle.y = pcd->frameInfo.upperBorderWidth;

	/*
	 * Fixed bug where last button in title bar did not activate when
	 * the client's X border was showing.
	 */
	pcd->titleRectangle.width = clientWidth +
	  (XBorderIsShowing(pcd) ? 2*pcd->xBorderWidth : 2*pcd->matteWidth);
	pcd->titleRectangle.height = pcd->frameInfo.titleBarHeight;

	/* fill in title bar rectangles */
	igr = 0;

	pgr[igr].id = FRAME_TITLE;
	GetFramePartInfo (pcd, FRAME_TITLE, &fpX, &fpY, &fpWidth, &fpHeight);

	/* copy in and convert to shorts */
        pgr[igr].rect.x = fpX;
	pgr[igr].rect.y = fpY;
	pgr[igr].rect.width = fpWidth;
	pgr[igr].rect.height = fpHeight;
	igr += 1;

	if (decor & MWM_DECOR_MENU) {
	    pgr[igr].id = FRAME_SYSTEM;
	    GetFramePartInfo (pcd, FRAME_SYSTEM, &fpX, &fpY, &fpWidth, 
			      &fpHeight);

	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;
	}

	if (decor & MWM_DECOR_MINIMIZE) {
	    pgr[igr].id = FRAME_MINIMIZE;
	    GetFramePartInfo (pcd, FRAME_MINIMIZE, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;
	}

	if (decor & MWM_DECOR_MAXIMIZE) {
	    pgr[igr].id = FRAME_MAXIMIZE;
	    GetFramePartInfo (pcd, FRAME_MAXIMIZE, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;
	}

	/* update client data */
	pcd->pTitleGadgets = pgr;
	pcd->cTitleGadgets = igr;
    }

    /* client matte area (actually base window area) */

    if (decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {
	pcd->matteRectangle.x = pcd->frameInfo.lowerBorderWidth;
	pcd->matteRectangle.y = pcd->frameInfo.upperBorderWidth + 
				    pcd->frameInfo.titleBarHeight;
	pcd->matteRectangle.width = pcd->frameInfo.width - 
				        (2 * pcd->frameInfo.lowerBorderWidth);
	pcd->matteRectangle.height = pcd->frameInfo.height - 
					 pcd->frameInfo.upperBorderWidth - 
					 pcd->frameInfo.lowerBorderWidth - 
					 pcd->frameInfo.titleBarHeight;
    }
    else 
    {
	pcd->matteRectangle.x = 0;
	pcd->matteRectangle.y = pcd->frameInfo.titleBarHeight;
	pcd->matteRectangle.width = pcd->frameInfo.width;
	pcd->matteRectangle.height = pcd->frameInfo.height - 
					 pcd->frameInfo.titleBarHeight;
    }

    if (decor & MWM_DECOR_RESIZEH) {

	if ( (pgr = pcd->pResizeGadgets) == NULL) {
	    return;		/* nothing there !!! */
	}

	/* fill in resize rectangles */
	igr = 0;
	if (decor & MWM_DECOR_RESIZEH) { 

	    pgr[igr].id = FRAME_RESIZE_NW;
	    GetFramePartInfo (pcd, FRAME_RESIZE_NW, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;

	    pgr[igr].id = FRAME_RESIZE_N;
	    GetFramePartInfo (pcd, FRAME_RESIZE_N, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    if ((int) fpWidth > 0)  {
		/* copy in and convert to shorts */
		pgr[igr].rect.x = fpX;
		pgr[igr].rect.y = fpY;
		pgr[igr].rect.width = fpWidth;
		pgr[igr].rect.height = fpHeight;
		igr += 1;
	    }

	    pgr[igr].id = FRAME_RESIZE_NE;
	    GetFramePartInfo (pcd, FRAME_RESIZE_NE, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;

	    pgr[igr].id = FRAME_RESIZE_W;
	    GetFramePartInfo (pcd, FRAME_RESIZE_W, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    if ((int)fpHeight > 0) {
		/* copy in and convert to shorts */
		pgr[igr].rect.x = fpX;
		pgr[igr].rect.y = fpY;
		pgr[igr].rect.width = fpWidth;
		pgr[igr].rect.height = fpHeight;
		igr += 1;
	    }

	    pgr[igr].id = FRAME_RESIZE_E;
	    GetFramePartInfo (pcd, FRAME_RESIZE_E, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    if ((int) fpHeight > 0) {
		/* copy in and convert to shorts */
		pgr[igr].rect.x = fpX;
		pgr[igr].rect.y = fpY;
		pgr[igr].rect.width = fpWidth;
		pgr[igr].rect.height = fpHeight;
		igr += 1;
	    }

	    pgr[igr].id = FRAME_RESIZE_SW;
	    GetFramePartInfo (pcd, FRAME_RESIZE_SW, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	    igr += 1;

	    pgr[igr].id = FRAME_RESIZE_S;
	    GetFramePartInfo (pcd, FRAME_RESIZE_S, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    if ((int) fpWidth > 0) {
		/* copy in and convert to shorts */
		pgr[igr].rect.x = fpX;
		pgr[igr].rect.y = fpY;
		pgr[igr].rect.width = fpWidth;
		pgr[igr].rect.height = fpHeight;
		igr += 1;
	    }

	    pgr[igr].id = FRAME_RESIZE_SE;
	    GetFramePartInfo (pcd, FRAME_RESIZE_SE, 
	                      &fpX, &fpY, &fpWidth, &fpHeight);
	    /* copy in and convert to shorts */
	    pgr[igr].rect.x = fpX;
	    pgr[igr].rect.y = fpY;
	    pgr[igr].rect.width = fpWidth;
	    pgr[igr].rect.height = fpHeight;
	}

	/* update client data */
	pcd->pResizeGadgets = pgr;
    }

} /* END OF FUNCTION  ComputeGadgetRectangles   */



/*************************************<->*************************************
 *
 *  GetSystemMenuPosition (pcd, px, py, height, context)
 *
 *
 *  Description:
 *  -----------
 *  Returns the position of where the system menu should be popped up.
 *  The hotspotRectangle in global is also set up to match the icon or
 *  system menu button area.
 *
 *
 *  Inputs:
 *  ------
 *  pcd	= pointer to client data
 *
 *  px = pointer to x location
 *
 *  py = pointer to y location
 *
 *  height = height of the system menu
 *
 *  context =  context that the menu is to be posted under.
 *
 *
 *  Outputs:
 *  -------
 *  *px	= x location
 *
 *  *py	= y location
 *
 *  wmGD.hotspotRectangle = system menu button or icon area (root relative)
 *
 *************************************<->***********************************/

void GetSystemMenuPosition (ClientData *pcd, int *px, int *py, 
			    unsigned int height, Context context)
{

    if ((pcd->clientState == MINIMIZED_STATE) ||
        ((pcd->clientState != MINIMIZED_STATE) &&
         (context == F_SUBCONTEXT_IB_WICON)))
    {
	/* 
	 * Try to put the menu directly above the icon.
	 * If it would hit the top of the screen then try to put it below
	 *   the icon and label.
	 * If it would then hit the bottom of the screen turn of the hotspot
	 *   processing.
	 */


	if (pcd->pSD->useIconBox && P_ICON_BOX(pcd))
        {
            GetIconBoxIconRootXY (pcd, px, py);

            wmGD.hotspotRectangle.x = *px;
            wmGD.hotspotRectangle.y = *py;

            *py -= height;

            if (*py < 0)
            {
                *py += height + ICON_HEIGHT(pcd);
                if (*py + height >= DisplayHeight (DISPLAY, 
						   SCREEN_FOR_CLIENT(pcd)))
                {
                    wmGD.checkHotspot = FALSE;
                }
            }
        }
	else
        {
	    *px = ICON_X(pcd);
	    *py = ICON_Y(pcd) - height;
	    
	    if (*py < 0)
	    {
		*py = ICON_Y(pcd) + ICON_HEIGHT(pcd);
		if (*py + height >= DisplayHeight (DISPLAY, 
						   SCREEN_FOR_CLIENT(pcd)))
		{
		    wmGD.checkHotspot = FALSE;
		}
	    }
	    
	    wmGD.hotspotRectangle.x = ICON_X(pcd);
	    wmGD.hotspotRectangle.y = ICON_Y(pcd);
	}

	/* setup the hotspot rectangle data */

	wmGD.hotspotRectangle.width = ICON_WIDTH(pcd);
	wmGD.hotspotRectangle.height = ICON_HEIGHT(pcd);
    }
    else
    {
	/* 
	 * Try to put the menu directly below the SW corner of the 
	 *   titlebar/border.
	 * If it would hit the bottom of the screen then try to put it directly
	 *   above the NW corner of the titlebar/border.
	 * If it would then hit the top of the screen turn of the hotspot
	 *   processing.
	 */

	if ((pcd->decor & MWM_DECOR_TITLE) &&
	    !(pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)))
	{
	    *px = pcd->frameInfo.x;
	    *py = pcd->frameInfo.y + pcd->frameInfo.titleBarHeight;
	}
	else 
	{
	    *px = pcd->frameInfo.x + pcd->frameInfo.lowerBorderWidth;
	    *py = pcd->frameInfo.y + pcd->frameInfo.upperBorderWidth + 
		  pcd->frameInfo.titleBarHeight;
	}
	if (*py + height >= DisplayHeight (DISPLAY, 
		  SCREEN_FOR_CLIENT(pcd)))
	{
	    if ((pcd->decor & MWM_DECOR_TITLE) &&
		!(pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)))
	    {
		*py = pcd->frameInfo.y - height;
	    }
	    else
	    {
		*py = pcd->frameInfo.y + pcd->frameInfo.upperBorderWidth - 
		    height;
	    }
	    if (*py < 0)
	    {
		wmGD.checkHotspot = FALSE;
	    }
	}

	/* setup the hotspot rectangle data */

	wmGD.hotspotRectangle.x = pcd->frameInfo.x + 
				  pcd->frameInfo.lowerBorderWidth;
	wmGD.hotspotRectangle.y = pcd->frameInfo.y + 
	                          pcd->frameInfo.upperBorderWidth;

	    /* assume square button */
	wmGD.hotspotRectangle.width = pcd->frameInfo.titleBarHeight;
	wmGD.hotspotRectangle.height = pcd->frameInfo.titleBarHeight;
    }

} /* END OF FUNCTION GetSystemMenuPosition */



/*************************************<->*************************************
 *
 *  ShowActiveClientFrame (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Paint the frame to indicate an "active" window
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This calls the frame exposure procedure, which gets some GCs based
 *    on the current keyboard focus. Thus, wmGD.keyboardFocus == pcd
 *    must be TRUE when this is called for the correct highlighting to 
 *    occur.
 * 
 *************************************<->***********************************/

void 
ShowActiveClientFrame (ClientData *pcd)
{
    unsigned long attr_mask = 0;
    XSetWindowAttributes window_attribs;

    if (DECOUPLE_TITLE_APPEARANCE(pcd) && 
	 (pcd->decor & MWM_DECOR_TITLE))
    {
	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (CLIENT_TITLE_APPEARANCE(pcd).activeBackgroundPixmap)
	    {
	    attr_mask |= CWBackPixmap;
	    window_attribs.background_pixmap =
		CLIENT_TITLE_APPEARANCE(pcd).activeBackgroundPixmap;
	}
	else
	{
	    attr_mask |= CWBackPixel;
	    window_attribs.background_pixel = 
		CLIENT_TITLE_APPEARANCE(pcd).activeBackground;
	}


	XChangeWindowAttributes (DISPLAY, pcd->clientTitleWin, attr_mask, 
				 &window_attribs);

	/* clear the frame to the right background */
	XClearWindow (DISPLAY, pcd->clientTitleWin);
    }

    /* 
     * Use background pixmap if one is specified, otherwise set the
     * appropriate background color. 
     */

    if (CLIENT_APPEARANCE(pcd).activeBackgroundPixmap)
    {
	attr_mask |= CWBackPixmap;
	window_attribs.background_pixmap =
		CLIENT_APPEARANCE(pcd).activeBackgroundPixmap;
    }
    else
    {
	attr_mask |= CWBackPixel;
	window_attribs.background_pixel = 
		CLIENT_APPEARANCE(pcd).activeBackground;
    }


    XChangeWindowAttributes (DISPLAY, pcd->clientFrameWin, attr_mask, 
			     &window_attribs);

    /* clear the frame to the right background */
    XClearWindow (DISPLAY, pcd->clientFrameWin);

    /* simulate exposure of window */
    FrameExposureProc (pcd);


} /* END OF FUNCTION ShowActiveClient */



/*************************************<->*************************************
 *
 *  ShowInactiveClientFrame (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Paint the frame to indicate an "inactive" window
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This calls the frame exposure procedure, which gets some GCs based
 *    on the current keyboard focus. Thus, wmGD.keyboardFocus == pcd
 *    must be FALSE when this is called for the correct highlighting to 
 *    occur.
 *  
 * 
 ******************************<->***********************************/

void 
ShowInactiveClientFrame (ClientData *pcd)
{
    unsigned long attr_mask = 0;
    XSetWindowAttributes window_attribs;

    if (DECOUPLE_TITLE_APPEARANCE(pcd) && 
	(pcd->decor & MWM_DECOR_TITLE))
    {
	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (CLIENT_TITLE_APPEARANCE(pcd).backgroundPixmap)
	{
	    attr_mask |= CWBackPixmap;
	    window_attribs.background_pixmap =
		CLIENT_TITLE_APPEARANCE(pcd).backgroundPixmap;
	}
	else
	{
	    attr_mask |= CWBackPixel;
	    window_attribs.background_pixel = 
		    CLIENT_TITLE_APPEARANCE(pcd).background;
	}


	XChangeWindowAttributes (DISPLAY, pcd->clientTitleWin, attr_mask, 
				 &window_attribs);

	/* clear the frame to the right background */
	XClearWindow (DISPLAY, pcd->clientTitleWin);
  
        /*
         * attr_mask must be cleared because it is set if
         * DECOUPLE_TITLE_APPEARANCE(pcd) is true.
         */
        attr_mask = 0;
  
    }
    /* 
     * Use background pixmap if one is specified, otherwise set the
     * appropriate background color. 
     */

    if (CLIENT_APPEARANCE(pcd).backgroundPixmap)
    {
	attr_mask |= CWBackPixmap;
	window_attribs.background_pixmap =
	    CLIENT_APPEARANCE(pcd).backgroundPixmap;
    }
    else
    {
	attr_mask |= CWBackPixel;
	window_attribs.background_pixel = 
		    CLIENT_APPEARANCE(pcd).background;
    }


    /* change window attribs so clear does the right thing */
    XChangeWindowAttributes (DISPLAY, pcd->clientFrameWin, attr_mask, 
			     &window_attribs);

    /* clear the frame to the right background */
    XClearWindow (DISPLAY, pcd->clientFrameWin);

    /* simulate exposure of window */
    FrameExposureProc (pcd);

} /* END OF FUNCTION ShowInactiveClientFrame */



/*************************************<->*************************************
 *
 *  RegenerateClientFrame (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Reconfigure the sizes of all the components of the client frame
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

void RegenerateClientFrame (ClientData *pcd)
{
    unsigned long decor = pcd->decor;
#ifdef PANELIST
    /* 
     * If an embedded client, there is no frame.
     */
    if (pcd->pECD)
    {
	if (!pcd->clientFrameWin)
	{
	    return;
	}
    }
#endif /* PANELIST */

    /* recompute frame information */
    SetFrameInfo (pcd);

    /* move & resize frame window */
    XMoveResizeWindow (DISPLAY, pcd->clientFrameWin, pcd->frameInfo.x, 
	   pcd->frameInfo.y, pcd->frameInfo.width, pcd->frameInfo.height);


    /* resize title bar window */
    if (decor & MWM_DECOR_TITLE)
    {
	XResizeWindow (DISPLAY, pcd->clientTitleWin, 
	   pcd->frameInfo.width - 2*pcd->frameInfo.upperBorderWidth, 
	   pcd->frameInfo.titleBarHeight);
    }

    /* resize base window */
    XResizeWindow (DISPLAY, pcd->clientBaseWin, BaseWindowWidth (pcd),
	   BaseWindowHeight (pcd));
    
    /* resize the stretcher windows */
    if (SHOW_RESIZE_CURSORS(pcd) && (decor & MWM_DECOR_RESIZEH)) {
	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_NORTH_WEST], 
	    0, 0, pcd->frameInfo.cornerWidth, 
	    pcd->frameInfo.cornerHeight);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_NORTH], 
	    (int) pcd->frameInfo.cornerWidth, 0, 
	    pcd->frameInfo.width - 2*pcd->frameInfo.cornerWidth, 
	    pcd->frameInfo.upperBorderWidth);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_NORTH_EAST], 
	    (int) (pcd->frameInfo.width - pcd->frameInfo.cornerWidth), 0, 
	    pcd->frameInfo.cornerWidth, pcd->frameInfo.cornerHeight);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_EAST], 
	    (int) (pcd->frameInfo.width - pcd->frameInfo.lowerBorderWidth),
	    (int) (pcd->frameInfo.cornerHeight), 
	    pcd->frameInfo.lowerBorderWidth, 
	    pcd->frameInfo.height - 2*pcd->frameInfo.cornerHeight);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_SOUTH_EAST], 
	    (int) (pcd->frameInfo.width - pcd->frameInfo.cornerWidth),
	    (int) (pcd->frameInfo.height - pcd->frameInfo.cornerHeight), 
	    pcd->frameInfo.cornerWidth, pcd->frameInfo.cornerHeight);
	
	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_SOUTH], 
	    (int) pcd->frameInfo.cornerWidth,
	    (int) (pcd->frameInfo.height - pcd->frameInfo.lowerBorderWidth), 
	    pcd->frameInfo.width - 2*pcd->frameInfo.cornerWidth,
	    pcd->frameInfo.lowerBorderWidth);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_SOUTH_WEST], 
	    0, (int) (pcd->frameInfo.height - pcd->frameInfo.cornerHeight), 
	    pcd->frameInfo.cornerWidth, pcd->frameInfo.cornerHeight);

	XMoveResizeWindow (DISPLAY, 
	    pcd->clientStretchWin[STRETCH_WEST], 
	    0, (int) pcd->frameInfo.cornerHeight, 
	    pcd->frameInfo.lowerBorderWidth,
	    pcd->frameInfo.height - 2*pcd->frameInfo.cornerHeight);
    }

    /* recreate gadget rectangles */
    ComputeGadgetRectangles (pcd);

    /* regenerate the graphics */
    GenerateFrameDisplayLists (pcd);

#ifndef NO_SHAPE
    if (wmGD.hasShape && pcd->wShaped)
    {
        SetFrameShape (pcd);
    }
#endif /*  NO_SHAPE  */

} /* END OF FUNCTION  RegenerateClientFrame  */




/*************************************<->*************************************
 *
 *  BevelSystemButton (prTop, prBot, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Bevels a rectangle for the system button (drawer handle?)
 *
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow rectangles
 *  prBot	- ptr to bottom shadow rectangles
 *  x		- x coord of maximize gadget
 *  y 		- y coord of maximize gadget
 *  width	- width of maximize gadget
 *  height	- height of maximize gadget
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  o This draws a horizontal "drawer handle" for the system gadget.
 *    Assumptions: the enclosing box is square (width == height)
 *************************************<->***********************************/

void BevelSystemButton (RList *prTop, RList *prBot, int x, int y, 
			unsigned int width, unsigned int height)
{
    int offset1, offset2;
    unsigned int dim1, dim2;

    switch (height) {
	case 5:
	case 6:
	    offset1 = offset2 = 2;
	    dim1 = dim2 = height-4;
	    break;

	case 7:
	    offset1 = offset2 = 2;
	    dim1 = 3;
	    dim2 = 2;
	    break;
	
	case 8:
	case 9:
	    offset1 = 2;
	    offset2 = 3;
	    dim1 = width - 4;
	    dim2 = height - 6;
	    break;

	case 10:
	case 11:
	    offset1 = 3;
	    offset2 = 4;
	    dim1 = width - 6;
	    dim2 = height - 8;
	    break;

	case 12:
	case 13:
	    offset1 = 3;
	    offset2 = (height-3)/2;
	    dim1 = width - 6;
	    dim2 = 3;
	    break;

	default:
	    offset1 = 4;
	    offset2 = (height - 4)/2;
	    dim1 = width - 8;
	    dim2 = 4;
	    break;

    }

    if (height >= 5) {
	/* system icon */
	BevelRectangle (prTop, prBot, 		/* system icon */
		    (x+offset1), (y+offset2),
		    dim1, dim2,
		    1, 1, 1, 1);
	}
} /* END OF FUNCTION  BevelSystemButton   */



/*************************************<->*************************************
 *
 *  BevelMinimizeButton (prTop, prBot, x, y, height)
 *
 *
 *  Description:
 *  -----------
 *  Bevels a rectangle for the minimize button
 *
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow rectangles
 *  prBot	- ptr to bottom shadow rectangles
 *  x		- x coord of maximize gadget
 *  y 		- y coord of maximize gadget
 *  height	- height of maximize gadget
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

void BevelMinimizeButton (RList *prTop, RList *prBot, int x, int y, 
			  unsigned int height)
{
    int offset1, offset2;
    unsigned int dim1, dim2;

    switch (height) {
	case 7:
	case 8:
	case 9:
	    offset1 = offset2 = 3;
	    dim1 = dim2 = height-6;
	    break;

	case 10:
	case 11:
	case 12:
	    offset1 = offset2 = (height-3)/2;
	    dim1 = dim2 = 3;
	    break;

	default:
	    offset1 = offset2 = (height-4)/2;
	    dim1 = dim2 = 4;
	    break;
    }

    if (height >= 7) {
	/* minimize icon */
	BevelRectangle (prTop, prBot,
		    (x+offset1), (y+offset2),
		    dim1, dim2,
		    1, 1, 1, 1);
    }
} /* END OF FUNCTION  BevelMinimizeButton   */



/*************************************<->*************************************
 *
 *  BevelMaximizeButton (prTop, prBot, x, y, height)
 *
 *
 *  Description:
 *  -----------
 *  Bevels a rectangle for the maximize button
 *
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow rectangles
 *  prBot	- ptr to bottom shadow rectangles
 *  x		- x coord of maximize gadget
 *  y 		- y coord of maximize gadget
 *  height	- height of maximize gadget
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

void BevelMaximizeButton (RList *prTop, RList *prBot, int x, int y, 
			  unsigned int height)
{
    int offset1, offset2;
    unsigned int dim1, dim2;

    switch (height) {
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	    offset1 = offset2 = 2;
	    dim1 = dim2 = height-4;
	    break;

	case 12:
	case 13:
	case 14:
	case 15:
	    offset1 = offset2 = 3;
	    dim1 = dim2 = height-6;
	    break;

	default:
	    offset1 = offset2 = 4;
	    dim1 = dim2 = height-8;
	    break;
    }

    /* maximize icon */
    BevelRectangle (prTop, prBot,
		    (x+offset1), (y+offset2),
		    dim1, dim2,
		    1, 1, 1, 1);
} /* END OF FUNCTION  BevelMaximizeButton   */


/*************************************<->*************************************
 *
 *  DepressGadget (pcd, gadget, depressed)
 *
 *
 *  Description:
 *  -----------
 *  Show the gadget in a "depressed" state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  gadget	- gadget id
 *  depressed	- if True, then gadget is shown depressed, if False it is
 *		  shown not depressed
 *
 * 
 *  Outputs:
 *  -------
 *  return	- true if sucessful
 *
 *
 *  Comments:
 *  --------
 *  o This assumes there is a one-pixel bevel around the gadget.
 *  o This only works on title bar gadgets.
 * 
 *************************************<->***********************************/

Boolean DepressGadget (ClientData *pcd, int gadget, Boolean depressed)
{
    int x, y; 
    unsigned int width, height, invertWidth;
    static RList *pTopRect = NULL; 
    static RList *pBotRect = NULL; 
    GC topGC, botGC;
    Window win;

    /* get outside dimensions of box we want */

    switch (gadget) {
	case FRAME_TITLE:
	case FRAME_SYSTEM:
	case FRAME_MINIMIZE:
	case FRAME_MAXIMIZE:
	    if (!GetDepressInfo (pcd, gadget, &x, &y, &width, 
				 &height, &invertWidth))
		return(FALSE);
	    
	    break;

	default:
	    return(FALSE);	/* do nothing on non-title bar gagdets */
    }

    if (DECOUPLE_TITLE_APPEARANCE(pcd) && 
	 (pcd->decor & MWM_DECOR_TITLE))
    {
	/* adjust position to be relative to titlebar window, not frame */
	x -= (short) pcd->frameInfo.upperBorderWidth;
	y -= (short) pcd->frameInfo.upperBorderWidth;

	/* use "active" GCs if we have keyboard focus */
	if (pcd == wmGD.keyboardFocus) {
	    topGC = CLIENT_TITLE_APPEARANCE(pcd).activeTopShadowGC;
	    botGC = CLIENT_TITLE_APPEARANCE(pcd).activeBottomShadowGC;
	}
	else {
	    topGC = CLIENT_TITLE_APPEARANCE(pcd).inactiveTopShadowGC;
	    botGC = 
		CLIENT_TITLE_APPEARANCE(pcd).inactiveBottomShadowGC;
	}

	/* draw into title bar window */
	win = pcd->clientTitleWin;
    }
    else 
    {
	/* use "active" GCs if we have keyboard focus */
	if (pcd == wmGD.keyboardFocus) {
	    topGC = CLIENT_APPEARANCE(pcd).activeTopShadowGC;
	    botGC = CLIENT_APPEARANCE(pcd).activeBottomShadowGC;
	}
	else {
	    topGC = CLIENT_APPEARANCE(pcd).inactiveTopShadowGC;
	    botGC = CLIENT_APPEARANCE(pcd).inactiveBottomShadowGC;
	}

	/* draw into client frame window */
	win = pcd->clientFrameWin;
    }

    /* 
     * Bevel a rectangle for the desired button effect 
     * Allocate the rectangles if necessary.
     */
    if ( (pTopRect && pBotRect) ||
	 ((pTopRect = AllocateRList(2)) &&
	  (pBotRect = AllocateRList(2))))
    {
	pTopRect->used = 0;
	pBotRect->used = 0;
	BevelRectangle (pTopRect, pBotRect, 
		    x, y, width, height, 
		    invertWidth, invertWidth, 
		    invertWidth, invertWidth);
    }

    /* draw the gadget border to make it look depressed or normal */

    if (depressed) {
	XFillRectangles (DISPLAY, win, botGC, pTopRect->prect, pTopRect->used);
	XFillRectangles (DISPLAY, win, topGC, pBotRect->prect, pBotRect->used);
    }
    else {
	XFillRectangles (DISPLAY, win, topGC, pTopRect->prect, pTopRect->used);
	XFillRectangles (DISPLAY, win, botGC, pBotRect->prect, pBotRect->used);
    }
    return(TRUE);
} /* END OF FUNCTION  DepressGadget   */


/*************************************<->*************************************
 *
 *  PushGadgetIn (pcd, gadget)
 *
 *
 *  Description:
 *  -----------
 *  Shows a title bar gadget in a depressed state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  gadget	- gadget id
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void PushGadgetIn (ClientData *pcd, int gadget)
{
    switch (gadget) {
	case FRAME_SYSTEM:
	    pcd->decorFlags |= SYSTEM_DEPRESSED;
	    break;

	case FRAME_TITLE:
	    pcd->decorFlags |= TITLE_DEPRESSED;
	    break;

	case FRAME_MINIMIZE:
	    pcd->decorFlags |= MINIMIZE_DEPRESSED;
	    break;

	case FRAME_MAXIMIZE:
	    pcd->decorFlags |= MAXIMIZE_DEPRESSED;
	    break;

	default:
	    return;
    }
    GenerateFrameDisplayLists(pcd);
    (void) DepressGadget (pcd, gadget, TRUE);
    wmGD.gadgetClient = pcd;
    wmGD.gadgetDepressed =  gadget;
} /* END OF FUNCTION  PushGadgetIn   */


/*************************************<->*************************************
 *
 *  PopGadgetOut (pcd, gadget)
 *
 *
 *  Description:
 *  -----------
 *  Shows a title bar gadget in a depressed state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  gadget	- gadget id
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void PopGadgetOut (ClientData *pcd, int gadget)
{
    switch (gadget) {
	case FRAME_SYSTEM:
	    pcd->decorFlags &= ~SYSTEM_DEPRESSED;
	    break;

	case FRAME_TITLE:
	    pcd->decorFlags &= ~TITLE_DEPRESSED;
	    break;

	case FRAME_MINIMIZE:
	    pcd->decorFlags &= ~MINIMIZE_DEPRESSED;
	    break;

	case FRAME_MAXIMIZE:
	    pcd->decorFlags &= ~MAXIMIZE_DEPRESSED;
	    break;

	default:
	    return;
    }
    GenerateFrameDisplayLists(pcd);
    (void) DepressGadget (pcd, gadget, FALSE);
    wmGD.gadgetClient    = NULL;
    wmGD.gadgetDepressed = 0;
} /* END OF FUNCTION  PopGadgetOut   */

#ifndef NO_SHAPE

/*************************************<->*************************************
 *
 *  SetFrameShape (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Shapes the frame and base window to the shape of the client
 *  window. Also ors the title window into the shaped frame
 *  window if present.
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o currently punt on resize handle around the frame.
 *  
 *************************************<->***********************************/
void SetFrameShape (ClientData *pcd)
{
    /*
     * The frame consists of the shape of the contents window offset by
     * title_height or'ed with the shape of title window (which is always
     * rectangular).
     */
    int xOffset = 0;
    int yOffset = 0;

    if (XBorderIsShowing(pcd))
    {
	xOffset = pcd->xBorderWidth;
	yOffset = pcd->xBorderWidth;
    }
    else if(pcd->matteWidth > 0)
    {
	xOffset = pcd->matteWidth;
	yOffset = pcd->matteWidth;
    }

    if (pcd->wShaped)
    {
        /*
	 * need to do general case
	 */
	XShapeCombineShape (DISPLAY, pcd->clientBaseWin, ShapeBounding,
			    xOffset,
			    yOffset,
                            pcd->client, ShapeBounding,
                            ShapeSet);

	XShapeCombineShape (DISPLAY, pcd->clientFrameWin, ShapeBounding,
			    BaseWindowX (pcd),
                            BaseWindowY (pcd),
                            pcd->clientBaseWin, ShapeBounding,
                            ShapeSet);

	if (pcd->decor & MWM_DECOR_TITLE)
        {
	    XShapeCombineShape (DISPLAY, pcd->clientFrameWin, ShapeBounding,
			        pcd->frameInfo.upperBorderWidth,
			        pcd->frameInfo.upperBorderWidth,
				pcd->clientTitleWin, ShapeBounding,
				ShapeUnion);
	}
    }
    else
    {
	 (void) XShapeCombineMask (DISPLAY, pcd->clientFrameWin, 
				   ShapeBounding, 0, 0,
				   None, ShapeSet);
	 (void) XShapeCombineMask (DISPLAY, pcd->clientFrameWin, 
				   ShapeClip, 0, 0,
				   None, ShapeSet);
	 (void) XShapeCombineMask (DISPLAY, pcd->clientBaseWin, 
				   ShapeBounding, 0, 0,
				   None, ShapeSet);
	 (void) XShapeCombineMask (DISPLAY, pcd->clientBaseWin, 
				   ShapeClip, 0, 0,
				   None, ShapeSet);
    }
} /* END OF FUNCTION  SetFrameShape  */
#endif /* NO_SHAPE */



