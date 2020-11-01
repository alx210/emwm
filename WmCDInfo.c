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
static char rcsid[] = "$XConsortium: WmCDInfo.c /main/4 1995/11/01 11:32:15 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmCDInfo.h"



/*************************************<->*************************************
 *
 *  unsigned int TitleTextHeight (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the height of the title text plus padding
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  TitleTextHeight	-  Height of title text plus padding
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int TitleTextHeight (ClientData *pcd)
{
    unsigned int ttextheight;

#ifndef NO_MULTIBYTE
    if (DECOUPLE_TITLE_APPEARANCE(pcd))
    {
	ttextheight = CLIENT_TITLE_APPEARANCE(pcd).titleHeight;
    }
    else
    {
	ttextheight = CLIENT_APPEARANCE(pcd).titleHeight;
    }
#else

    XFontStruct	*font;

    if (DECOUPLE_TITLE_APPEARANCE(pcd))
    {
	font = CLIENT_TITLE_APPEARANCE(pcd).font;
    }
    else 
    {
	font = CLIENT_APPEARANCE(pcd).font;
    }

    if (font)
    {
	ttextheight = TEXT_HEIGHT(font) + WM_TITLE_BAR_PADDING;
    }
    else
    {
	/* 
	 * Failed to load any font! (probably due to error in 
	 * font path, toolkit, or resource converter. Set to
	 * arbitrary value for robustness.
	 *
	 */
	ttextheight = 16; 
    }
#endif
    return (ttextheight);
}


/*************************************<->*************************************
 *
 *  unsigned int InitTitleBarHeight (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Used to initialize pCD->frameInfo.titleBarHeight which is used by the new
 *  macro TitleBarHeight().  Returns the height of the title bar.
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  InitTitleBarHeight	-  height of title bar, 0 if no title bar.
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int InitTitleBarHeight (ClientData *pcd)
{
    unsigned int tbarheight;

    if (pcd->decor & MWM_DECOR_TITLE)
    {
	tbarheight = TitleTextHeight(pcd);
    }
    else
    {
	tbarheight = 0;
    }

    return (tbarheight);
}


/*************************************<->*************************************
 *
 *  unsigned int UpperBorderWidth (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the width of the upper frame border
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  UpperBorderWidth	- width of the upper frame border
 *
 *  Comments:
 *  --------
 *  The upper border width is thinner if there is a title bar. The
 *  title bar appears to sit across some of the inside beveling to
 *  make the frame look more integrated.
 * 
 *************************************<->***********************************/
unsigned int UpperBorderWidth (ClientData *pcd)
{
    unsigned int uswidth;
    unsigned int decoration = pcd->decor;

    if (decoration & MWM_DECOR_RESIZEH)
    {
	uswidth = RESIZE_BORDER_WIDTH(pcd);
    }
    else if (decoration & MWM_DECOR_BORDER)
    {
	uswidth = FRAME_BORDER_WIDTH(pcd);
#ifdef PANELIST
	if((pcd->clientFlags & FRONT_PANEL_BOX) &&
	   (uswidth > 0))
	{
	    uswidth -= 1;
	}
#endif /* PANELIST */
    }
    else if ((decoration & MWM_DECOR_TITLE) ||
	     (pcd->matteWidth > 0))
    {
	uswidth = 0;
    }
    else
    {
	uswidth = pcd->xBorderWidth;
    }

    return (uswidth);
}


/*************************************<->*************************************
 *
 *  unsigned int LowerBorderWidth (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the width of the lower frame border
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  LowerBorderWidth	- width of the lower frame border
 *
 *  Comments:
 *  --------
 *  The upper border width is thinner than the lower border if there 
 *  is a title bar. The title bar appears to sit across some of the 
 *  inside beveling to make the frame look more integrated.
 * 
 *************************************<->***********************************/
unsigned int LowerBorderWidth (ClientData *pcd)
{
    unsigned int lswidth;
    unsigned int decoration = pcd->decor;

    if (decoration & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {
	if ((pcd->matteWidth == 0) && (decoration & MWM_DECOR_TITLE) &&
		(wmGD.frameStyle == WmRECESSED))
	    lswidth = UpperBorderWidth (pcd) + 
		      (pcd->internalBevel - JOIN_BEVEL(pcd));
	else
	    lswidth = UpperBorderWidth (pcd);
    }
    else if (pcd->matteWidth > 0)
    {
	lswidth = 0;
    }
    else
    {
	lswidth = pcd->xBorderWidth;
    }

    return (lswidth);
}


/*************************************<->*************************************
 *
 *  unsigned int FrameWidth (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the width of the frame
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  FrameWidth	- width of frame
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int FrameWidth (ClientData *pcd)
{
    unsigned int frameWidth;

    frameWidth = ((pcd->maxConfig) ? pcd->maxWidth : pcd->clientWidth) +
		  2*pcd->matteWidth + 2*LowerBorderWidth (pcd);

    return (frameWidth);
}


/*************************************<->*************************************
 *
 *  unsigned int CornerWidth (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the width of the corner frame resize handle
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  CornerWidth	- width of the corner resize handle
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int CornerWidth (ClientData *pcd)
{
    unsigned int cwidth;
    unsigned int frameWidth = FrameWidth (pcd);

    /* adjust for resize border (default border if no resize wanted) */
    if (pcd->decor & MWM_DECOR_RESIZEH)
    {
	/* corner size is driven by title bar height */
	cwidth = TitleTextHeight(pcd) + UpperBorderWidth (pcd);

	/* scale down corners to make resize pieces proportional */
	if (3*cwidth > frameWidth) cwidth = frameWidth / 3;
    }
    else {
	/* no resizing functions */
	cwidth = 0;
    }

    return (cwidth);
}


/*************************************<->*************************************
 *
 *  unsigned int FrameHeight (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the height of the frame
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  FrameHeight	-  position of frame
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int FrameHeight (ClientData *pcd)
{
    unsigned int frameHeight;

    if (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {
	frameHeight = ((pcd->maxConfig) ? 
			  pcd->maxHeight : pcd->clientHeight) +
		      2*pcd->matteWidth + LowerBorderWidth (pcd) + 
		      UpperBorderWidth (pcd) + TitleBarHeight(pcd);
    }
    else 
    {
	frameHeight = ((pcd->maxConfig) ? 
			  pcd->maxHeight : pcd->clientHeight) +
		      2*pcd->matteWidth + 2*LowerBorderWidth (pcd) + 
		      TitleBarHeight(pcd);
    }

    return (frameHeight);
}


/*************************************<->*************************************
 *
 *  unsigned int CornerHeight (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the height of the corner frame resize handle
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  CornerHeight	- Height of the corner resize handle
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
unsigned int CornerHeight (ClientData *pcd)
{
    unsigned int cheight;
    unsigned int frameHeight = FrameHeight (pcd);

    /* adjust for resize border (default border if no resize wanted) */
    if (pcd->decor & MWM_DECOR_RESIZEH)
    {
	/* corner size is driven by title bar height */
	cheight = TitleTextHeight(pcd) + UpperBorderWidth (pcd);

	/* scale down corners to make resize pieces proportional */
	if (3*cheight > frameHeight) cheight = frameHeight / 3;
    }
    else {
	/* produce default border with no resizing functions */
	cheight = 0;
    }

    return (cheight);
}


/*************************************<->*************************************
 *
 *  int BaseWindowX (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the X coord of the base window in the frame
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  BaseWindowX
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int BaseWindowX (ClientData *pcd)
{
    int rval;

    if (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
	rval = LowerBorderWidth(pcd);
    else
	rval = 0;
    
    return (rval);
}


/*************************************<->*************************************
 *
 *  int BaseWindowY (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the Y coord of the base window in the frame
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  BaseWindowY
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int BaseWindowY (ClientData *pcd)
{
    int rval;

    if (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
	rval = UpperBorderWidth (pcd) + TitleBarHeight(pcd);
    else
	rval = TitleBarHeight(pcd);
    
    return (rval);
}



/*************************************<->*************************************
 *
 *  int FrameX (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the X-coordinate of the frame for the window
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  FrameX		- X position of frame
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int FrameX (ClientData *pcd)
{
    int frameX;

    frameX = ((pcd->maxConfig) ? pcd->maxX : pcd->clientX)
	     - LowerBorderWidth (pcd)  - pcd->matteWidth;

    return (frameX);
}


/*************************************<->*************************************
 *
 *  int FrameY (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the Y-coordinate of the frame for the window
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  FrameY		- Y position of frame
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
int FrameY (ClientData *pcd)
{
    int frameY;

    frameY = ((pcd->maxConfig) ? pcd->maxY : pcd->clientY) 
	 - UpperBorderWidth (pcd) 
	 - TitleBarHeight (pcd) - pcd->matteWidth;

    if ((pcd->decor & MWM_DECOR_TITLE) && 
	!(pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)) &&
	!(pcd->matteWidth))
    {
	frameY -= pcd->xBorderWidth;
    }

    return (frameY);
}


/*************************************<->*************************************
 *
 *  unsigned int BaseWindowWidth (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the width of the base window 
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  BaseWindowWidth
 *
 *  Comments:
 *  --------
 *  o Based on code taken out of FrameWidth to make it a little 
 *    more efficient.
 * 
 *************************************<->***********************************/
unsigned int BaseWindowWidth (ClientData *pcd)
{
    unsigned int rval;

    if (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {
	/*
	 *  rval = FrameWidth(pcd) - 2*LowerBorderWidth(pcd);
	 */

	rval = ((pcd->maxConfig) ? pcd->maxWidth : pcd->clientWidth) +
		  2*pcd->matteWidth;
    }
    else
    {
	/*
	 *  rval = FrameWidth(pcd);
	 */
	rval = ((pcd->maxConfig) ? pcd->maxWidth : pcd->clientWidth) +
		  2*pcd->matteWidth + 2*LowerBorderWidth (pcd);
    }
    
    return (rval);
}



/*************************************<->*************************************
 *
 *  unsigned int BaseWindowHeight (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns the height of the base window 
 *
 *  Inputs:
 *  ------
 *  pcd			- pointer to client data record
 * 
 *  Outputs:
 *  -------
 *  BaseWindowHeight
 *
 *  Comments:
 *  --------
 *  o Based on code taken out of FrameHeight to make it a little 
 *    more efficient.
 * 
 *************************************<->***********************************/
unsigned int BaseWindowHeight (ClientData *pcd)
{
    unsigned int rval;

    if (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
    {

	/* 
	 *  rval = FrameHeight(pcd) - LowerBorderWidth(pcd) - 
	 *             UpperBorderWidth(pcd) - TitleBarHeight(pcd);
	 */

	rval = ((pcd->maxConfig) ? pcd->maxHeight : pcd->clientHeight) +
		      2*pcd->matteWidth;
    }
    else
    {
	/* 
	 *  rval = FrameHeight(pcd) - TitleBarHeight(pcd); 
	 */

	rval = ((pcd->maxConfig) ? pcd->maxHeight : pcd->clientHeight) +
		      2*pcd->matteWidth + 2*LowerBorderWidth (pcd);
    }
    
    return (rval);
}



/*************************************<->*************************************
 *
 *  Boolean GetFramePartInfo (pcd, part, pX, pY, pWidth, pHeight)
 *
 *
 *  Description:
 *  -----------
 *  Gets frame relative position and size of the specified frame part
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  part	- part id (e.g. FRAME_TITLE, FRAME_SYSTEM, etc.)
 *  pX		- pointer to x-coord return value
 *  pY		- pointer to y-coord return value
 *  pWidth	- pointer to width return value
 *  pHeight	- pointer to width return value
 *
 * 
 *  Outputs:
 *  -------
 *  Return 	- True if values returned, false if no such part for this
 * 		  frame (values undefined).
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
Boolean GetFramePartInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight)
{
    unsigned int boxdim = TitleBarHeight (pcd);
    unsigned long decor = pcd->decor;
    int vert, horiz;
    Boolean rval = FALSE;
    
    switch (part) 
    {
	case FRAME_SYSTEM:
		if (decor & MWM_DECOR_MENU) {
		    *pX = pcd->frameInfo.upperBorderWidth;
		    *pY = pcd->frameInfo.upperBorderWidth;
		    *pWidth = boxdim;
		    *pHeight = boxdim;
		    rval = TRUE;
		}
		break;

	case FRAME_TITLE:
		if (decor & MWM_DECOR_TITLE) {
		    *pX = pcd->frameInfo.upperBorderWidth + 
			    ((decor & MWM_DECOR_MENU) ? boxdim : 0);
		    *pY = pcd->frameInfo.upperBorderWidth;
		    *pWidth = pcd->frameInfo.width - 
			    2*pcd->frameInfo.upperBorderWidth - 
			    ((decor & MWM_DECOR_MENU) ? boxdim : 0) -
			    ((decor & MWM_DECOR_MINIMIZE) ? boxdim : 0) -
			    ((decor & MWM_DECOR_MAXIMIZE) ? boxdim : 0);
		    *pHeight = boxdim;
		    rval = TRUE;
		}
		break;
#ifdef WSM

	case FRAME_TITLEBAR:
		if (decor & MWM_DECOR_TITLE) {
		    *pX = pcd->frameInfo.upperBorderWidth;
		    *pY = pcd->frameInfo.upperBorderWidth;
		    *pWidth = pcd->frameInfo.width - 
			    2*pcd->frameInfo.upperBorderWidth;
		    *pHeight = boxdim;
		    rval = TRUE;
		}
		break;
#endif /* WSM */

	case FRAME_MINIMIZE:
		if (decor & MWM_DECOR_MINIMIZE) {
		    *pX = pcd->frameInfo.width - 
			    pcd->frameInfo.upperBorderWidth - boxdim -
			    ((decor & MWM_DECOR_MAXIMIZE) ? boxdim : 0);
		    *pY = pcd->frameInfo.upperBorderWidth;
		    *pWidth= boxdim;
		    *pHeight = boxdim;
		    rval = TRUE;
		}
		break;

	case FRAME_MAXIMIZE:
		if (decor & MWM_DECOR_MAXIMIZE) {
		    *pX = pcd->frameInfo.width - 
			    pcd->frameInfo.upperBorderWidth - boxdim;
		    *pY = pcd->frameInfo.upperBorderWidth;
		    *pWidth = boxdim;
		    *pHeight = boxdim;
		    rval = TRUE;
		}
		break;

	/*
	 * For very small windows, the resize pieces will shrink
	 * equally until the corner pieces get within two pixels
	 * of the resize border width. At this point the "side" section
	 * is killed off (by giving it a zero length) and the corner
	 * pieces claim the additional space
	 */
	case FRAME_RESIZE_NW:
		if (decor & MWM_DECOR_RESIZEH) {
		    *pX = 0;
		    *pY = 0;

		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.cornerWidth;
		    }
		    else {
			horiz = pcd->frameInfo.width - 
				2*pcd->frameInfo.cornerWidth;
			*pWidth = pcd->frameInfo.cornerWidth + 
				horiz/2 + horiz%2;
		    }

		    if ((int)pcd->frameInfo.cornerHeight > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.cornerHeight;
		    }
		    else {
			vert = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
			*pHeight = pcd->frameInfo.cornerHeight + 
				vert/2 + vert%2;
		    }
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_N:
		if (decor & MWM_DECOR_RESIZEH) {
		    *pX = pcd->frameInfo.cornerWidth;
		    *pY = 0;
		    *pHeight = pcd->frameInfo.upperBorderWidth;
		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.width - 
				2*pcd->frameInfo.cornerWidth;
		    }
		    else {
			*pWidth = 0;
		    }
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_NE:
		if (decor & MWM_DECOR_RESIZEH) {
		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.cornerWidth;
		    }
		    else {
			horiz = pcd->frameInfo.width - 
			    2*pcd->frameInfo.cornerWidth;
			*pWidth = pcd->frameInfo.cornerWidth + horiz/2;
		    }
		    *pX = pcd->frameInfo.width - *pWidth;

		    if ((int)pcd->frameInfo.cornerHeight > 
			   (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.cornerHeight;
		    }
		    else {
			vert = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
			*pHeight = pcd->frameInfo.cornerHeight + 
				vert/2 + vert%2;
		    }
		    *pY = 0;
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_E:
		if (decor & MWM_DECOR_RESIZEH) {
		    *pX = pcd->frameInfo.width - 
		            pcd->frameInfo.lowerBorderWidth;
		    *pY = pcd->frameInfo.cornerHeight;
		    *pWidth = pcd->frameInfo.lowerBorderWidth;
		    if ((int)pcd->frameInfo.cornerHeight > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
		    }
		    else {
			*pHeight = 0;
		    }
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_SE:
		if (decor & MWM_DECOR_RESIZEH) {
		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.cornerWidth;
		    }
		    else {
			horiz = pcd->frameInfo.width - 
				2*pcd->frameInfo.cornerWidth;
			*pWidth = pcd->frameInfo.cornerWidth + horiz/2;
		    }
		    *pX = pcd->frameInfo.width - *pWidth;

		    if ((int)pcd->frameInfo.cornerHeight > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.cornerHeight;
		    }
		    else {
			vert = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
			*pHeight = pcd->frameInfo.cornerHeight + vert/2;
		    }
		    *pY = pcd->frameInfo.height - *pHeight;
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_S:
		if (decor & MWM_DECOR_RESIZEH) {
		    *pX = pcd->frameInfo.cornerWidth;
		    *pY = pcd->frameInfo.height - 
			    pcd->frameInfo.lowerBorderWidth;
		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.width - 
				2*pcd->frameInfo.cornerWidth;
		    }
		    else {
			*pWidth = 0;
		    }
		    *pHeight = pcd->frameInfo.lowerBorderWidth;
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_SW:
		if (decor & MWM_DECOR_RESIZEH) {
		    if ((int)pcd->frameInfo.cornerWidth > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pWidth = pcd->frameInfo.cornerWidth;
		    }
		    else {
			horiz = pcd->frameInfo.width - 
			        2*pcd->frameInfo.cornerWidth;
			*pWidth = pcd->frameInfo.cornerWidth + 
				horiz/2 + horiz%2;
		    }
		    *pX = 0;

		    if ((int)pcd->frameInfo.cornerHeight > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.cornerHeight;
		    }
		    else {
			vert = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
			*pHeight = pcd->frameInfo.cornerHeight + vert/2;
		    }
		    *pY = pcd->frameInfo.height - *pHeight;
		    rval = TRUE;
		}
		break;
	    
	case FRAME_RESIZE_W:
		if (decor & MWM_DECOR_RESIZEH) {
		    *pX = 0;
		    *pY = pcd->frameInfo.cornerHeight;
		    *pWidth = pcd->frameInfo.lowerBorderWidth;
		    if ((int)pcd->frameInfo.cornerHeight > 
			    (int)pcd->frameInfo.lowerBorderWidth+2) {
			*pHeight = pcd->frameInfo.height - 
				2*pcd->frameInfo.cornerHeight;
		    }
		    else {
			*pHeight = 0;
		    }
		    rval = TRUE;
		}
		break;
	    
	default:
		break;
    }
    return(rval);
}



/*************************************<->*************************************
 *
 *  IdentifyFramePart (pCD, x, y)
 *
 *
 *  Description:
 *  -----------
 *  Returns an ID representing which part of the frame received the event
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *
 *  x, y	- client coordinates of event
 *
 * 
 *  Outputs:
 *  -------
 *  Return 	- ID of frame part where the button event occurred.
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/

int IdentifyFramePart (ClientData *pCD, int x, int y)
{
    unsigned long decor = pCD->decor;
    int clientWidth = (pCD->maxConfig) ? pCD->maxWidth : pCD->clientWidth;
    int clientHeight= (pCD->maxConfig) ? pCD->maxHeight : pCD->clientHeight;
    int rval;


    /* check for client window */

    if ( (x >= pCD->clientOffset.x) &&
	 (x <  pCD->clientOffset.x + clientWidth) &&
	 (y >= pCD->clientOffset.y) &&
	 (y <  pCD->clientOffset.y + clientHeight) )
    {
	return (FRAME_CLIENT);
    }


    /* check for client matte */

    if (pCD->matteWidth)
    {
	if ( (x >= pCD->matteRectangle.x) &&
	     (x <  (int)pCD->matteRectangle.x + (int)pCD->matteRectangle.width) &&
	     (y >= pCD->matteRectangle.y) &&
	     (y <  (int)pCD->matteRectangle.y + (int)pCD->matteRectangle.height) )
	{
	    return (FRAME_MATTE);
	}
    }


    /* check title bar */

    if (decor & MWM_DECOR_TITLE)
    {
	if ( (x >= pCD->titleRectangle.x) &&
	     (x <  (int)pCD->titleRectangle.x + (int)pCD->titleRectangle.width) &&
	     (y >= pCD->titleRectangle.y) &&
	     (y <  (int)pCD->titleRectangle.y + (int)pCD->titleRectangle.height) )
	{
	    return(GadgetID(x, y, pCD->pTitleGadgets, 
			    (unsigned int)pCD->cTitleGadgets));
	}
    }


    /* try resize border */

    if (decor & MWM_DECOR_RESIZEH)
    {
	rval = GadgetID(x, y, pCD->pResizeGadgets, STRETCH_COUNT);
    }
    else
    {
	rval = FRAME_NONE;
    }


    /* if not a resize border, but still in bounds return FRAME_NBORDER */

    if (rval == FRAME_NONE)
    {
	if ((x >= 0) && (y >= 0) &&
	    (x < FrameWidth (pCD)) && (y < FrameHeight (pCD))) 
	{
	    rval = FRAME_NBORDER;
	}
    }

    return (rval);

} /* END OF FUNCTION IdentifyFramePart */




/*************************************<->*************************************
 *
 *  GadgetID (x, y, pgadget, count)
 *
 *
 *  Description:
 *  -----------
 *  returns the id of the gadget in the list that the event occured in
 *
 *
 *  Inputs:
 *  ------
 *  x		- x coordinate of event
 *  y		- y coordinate of event
 *  pgadget	- pointer to a list of GadgetRectangles
 *  count	- number of elements in the pgadget list
 * 
 *  Outputs:
 *  -------
 *  Return 	- ID of gadget if found, FRAME_NONE if not found
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/
int GadgetID (int x, int y, GadgetRectangle *pgadget, unsigned int count)
{
    int ix;

    for (ix = 0; ix < count; ix++, pgadget++) {
	if ( (x >= pgadget->rect.x) &&
	     (x <  (int)pgadget->rect.x + (int)pgadget->rect.width) &&
	     (y >= pgadget->rect.y) &&
	     (y <  (int)pgadget->rect.y + (int)pgadget->rect.height) ) {
	    return ((unsigned long) pgadget->id);
	}
    }
    return(FRAME_NONE);
}




/*************************************<->*************************************
 *
 *  FrameToClient (pcd, pX, pY, pWidth, pHeight)
 *
 *
 *  Description:
 *  -----------
 *  Converts frame position and size to client position and size
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pX		- pointer to frame x-coord
 *  pY		- pointer to frame y-coord
 *  pWidth	- pointer to frame width in pixels
 *  pHeight	- pointer to frame height in pixels
 * 
 *  Outputs:
 *  -------
 *  *pX		- client x-coord
 *  *pY		- client y-coord
 *  *pWidth	- client width in pixels
 *  *pHeight	- client height in pixels
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void FrameToClient (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight)
{
    /* compute client window coordinates from frame coordinates */

    *pWidth = *pWidth - 2*pcd->clientOffset.x; 
    *pHeight = *pHeight - pcd->clientOffset.x - pcd->clientOffset.y; 
    *pX =  *pX + pcd->clientOffset.x;
    *pY =  *pY + pcd->clientOffset.y;
}


/*************************************<->*************************************
 *
 *  ClientToFrame (pcd, pX, pY, pWidth, pHeight)
 *
 *
 *  Description:
 *  -----------
 *  Converts frame position and size to client position and size
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  pX		- client x-coord
 *  pY 		- client y-coord
 *  pWidth	- client width in pixels
 *  pHeight	- client height in pixels
 * 
 *  Outputs:
 *  -------
 *  *pX		- frame x-coord
 *  *pY		- frame y-coord
 *  *pWidth	- frame width in pixels
 *  *pHeight	- frame height in pixels
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void ClientToFrame (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight)
{
    /* compute client window coordinates from frame coordinates */
    *pWidth = *pWidth + 2*pcd->clientOffset.x;
    *pHeight = *pHeight + pcd->clientOffset.x + pcd->clientOffset.y;
    *pX = *pX - pcd->clientOffset.x;
    *pY = *pY - pcd->clientOffset.y;
}



/*************************************<->*************************************
 *
 *  Boolean GetDepressInfo (pcd, part, pX, pY, pWidth, pHeight)
 *
 *
 *  Description:
 *  -----------
 *  Gets frame relative position, size, and width of the beveling 
 *  to show the gadget in its depressed state.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  part	- part id (e.g. FRAME_TITLE, FRAME_SYSTEM, etc.)
 *  pX		- pointer to x-coord return value
 *  pY		- pointer to y-coord return value
 *  pWidth	- pointer to width return value
 *  pHeight	- pointer to width return value
 *  pInverWidth	- pointer to inversion width return value
 *
 * 
 *  Outputs:
 *  -------
 *  Return 	- True if values returned, false if no such part for this
 * 		  frame (values undefined).
 *
 *
 *  Comments:
 *  --------
 *  o The "part" must be a title bar gadget.
 * 
 *************************************<->***********************************/
Boolean GetDepressInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight, unsigned int *pInvertWidth)
{
    Boolean rval;
    unsigned int exBevel, eBevel;
    unsigned int wBevel, sBevel;
    unsigned int insideBevel, meBevel;
    unsigned int decoration = pcd->decor;
    unsigned int tmp;

    switch (part)
    {
	case FRAME_SYSTEM:
	case FRAME_TITLE:
	case FRAME_MINIMIZE:
	case FRAME_MAXIMIZE:
	    if ((rval = GetFramePartInfo (pcd, part, pX, pY, pWidth, pHeight)))
	    {
		/*
		 * set bevel width based on join bevel
		 */
		*pInvertWidth = 1 + (JOIN_BEVEL(pcd) / 2);
		
		/* bevel between title and client (or matte) */
		insideBevel = (pcd->matteWidth > 0) ?  JOIN_BEVEL(pcd) : 
						   pcd->internalBevel;


		/*
		 * Compute beveling around the title area of the
		 * title bar.
		 */
		if (decoration & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER))
		{
		    exBevel = JOIN_BEVEL(pcd);	/* north side */
		    sBevel = insideBevel;		/* south side */
		    eBevel = JOIN_BEVEL(pcd);	/* east side */
		    wBevel = JOIN_BEVEL(pcd);	/* west side */
		    meBevel = JOIN_BEVEL(pcd);	/* east of Min */
		}
		else 
		{
		    /* borderless window */

		    exBevel = EXTERNAL_BEVEL(pcd);

		    sBevel = (pcd->matteWidth > 0) ? insideBevel : 
						      EXTERNAL_BEVEL(pcd);

		    eBevel = (decoration & (MWM_DECOR_MINIMIZE | 
						 MWM_DECOR_MAXIMIZE))?
				      JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);

		    wBevel = (decoration & MWM_DECOR_MENU) ?
				      JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);

		    meBevel = (decoration & (MWM_DECOR_MAXIMIZE)) ?
				      JOIN_BEVEL(pcd) : EXTERNAL_BEVEL(pcd);
		}

		/*
		 * Adjust height of all title bar gadgets if necessary.
		 * (The bottom bevel is the same for all the pieces.)
		 */

		if (sBevel > *pInvertWidth)
		{
		    *pHeight -= (sBevel - *pInvertWidth);
		}

		/*
		 * Adjust the beveling on the other sides of the 
		 * gadgets.
		 */
		switch (part)
		{
		    case FRAME_SYSTEM:
			/* west and north sides */
			if (exBevel > *pInvertWidth) 
			{
			    tmp = exBevel - *pInvertWidth;
			    *pX += tmp;
			    *pWidth -= tmp;
			    *pY += tmp;
			    *pHeight -= tmp;
			}

			/* east side */
			if (wBevel > *pInvertWidth) 
			{
			    *pWidth -= (wBevel - *pInvertWidth);
			}
			break;

		    case FRAME_TITLE:
			/* west side */
			if (wBevel > *pInvertWidth) 
			{
			    tmp = wBevel - *pInvertWidth;
			    *pX += tmp;
			    *pWidth -= tmp;
			}

			/* north side */
			if (exBevel > *pInvertWidth) 
			{
			    tmp = exBevel - *pInvertWidth;
			    *pY += tmp;
			    *pHeight -= tmp;
			}

			/* east side */
			if (eBevel > *pInvertWidth) 
			{
			    *pWidth -= eBevel - *pInvertWidth;
			}
			break;

		    case FRAME_MINIMIZE:
			/* north side */
			if (exBevel > *pInvertWidth) 
			{
			    tmp = exBevel - *pInvertWidth;
			    *pY += tmp;
			    *pHeight -= tmp;
			}

			/* west side */
			if (eBevel > *pInvertWidth) 
			{
			    tmp = eBevel - *pInvertWidth;
			    *pX += tmp;
			    *pWidth -= tmp;
			}

			/* east side */
			if (meBevel > *pInvertWidth) 
			{
			    *pWidth -= meBevel - *pInvertWidth;
			}
			break;

		    case FRAME_MAXIMIZE:
			/* north and east sides */
			if (exBevel > *pInvertWidth) 
			{
			    tmp = exBevel - *pInvertWidth;
			    *pY += tmp;
			    *pHeight -= tmp;
			    *pWidth -= tmp;
			}

			/* west side */
			if (eBevel > *pInvertWidth) 
			{
			    tmp = eBevel - *pInvertWidth;
			    *pX += tmp;
			    *pWidth -= tmp;
			}
			break;
		} /* end switch */
	    } /* end case "title bar part" */	
	    break;

	default:
	    rval = False;
	    break;

    }	/* end switch */

    return (rval);
}




/*************************************<->*************************************
 *
 *  SetFrameInfo (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Sets frame information into client data structure for easy access.
 *
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
 *  o Sets values into the FrameInfo component of the client data structure
 * 
 *************************************<->***********************************/
void SetFrameInfo (ClientData *pcd)
{

    /*
     * The title bar height value stored in pcd->frameInfo is used by the
     * macro TitleBarHeight(pcd).
     */

    pcd->frameInfo.titleBarHeight = InitTitleBarHeight (pcd);

    pcd->frameInfo.x = FrameX (pcd);
    pcd->frameInfo.y = FrameY (pcd);
    pcd->frameInfo.width = FrameWidth (pcd);
    pcd->frameInfo.height = FrameHeight (pcd);
    pcd->frameInfo.upperBorderWidth = UpperBorderWidth (pcd);
    pcd->frameInfo.lowerBorderWidth = LowerBorderWidth (pcd);
    pcd->frameInfo.cornerWidth = CornerWidth (pcd);
    pcd->frameInfo.cornerHeight = CornerHeight (pcd);

    /* set client offset */
     
    if ( (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)) ||
         (pcd->matteWidth > 0) )
    {
	/*
	 *  The window has a window manager border.
	 */
	pcd->clientOffset.x = pcd->frameInfo.lowerBorderWidth + 
			          pcd->matteWidth;
	pcd->clientOffset.y = pcd->frameInfo.upperBorderWidth + 
				  pcd->frameInfo.titleBarHeight + 
			          pcd->matteWidth;
    }
    else 
    {
	/*
	 * No window manager border, the original X border is showing
	 * through.
	 */
	pcd->clientOffset.x =  pcd->xBorderWidth + pcd->matteWidth;
	pcd->clientOffset.y =  pcd->xBorderWidth + 
				   pcd->frameInfo.titleBarHeight + 
				   pcd->matteWidth;
    }

}



/*************************************<->*************************************
 *
 *  SetClientOffset (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Sets the client offset into client data structure
 *
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
 *  This can be called before SetFrameInfo to set up the client offset
 *  without the danger of using uninitialized variables during 
 *  extraneous computation.
 *
 *************************************<->***********************************/
void SetClientOffset (ClientData *pcd)
{
    /*
     * The title bar height value stored in pcd->frameInfo is used by the
     * macro TitleBarHeight(pcd).
     */

    pcd->frameInfo.titleBarHeight = InitTitleBarHeight (pcd);

    pcd->frameInfo.upperBorderWidth = UpperBorderWidth (pcd);
    pcd->frameInfo.lowerBorderWidth = LowerBorderWidth (pcd);

    /* set client offset */
     
    if ( (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)) ||
         (pcd->matteWidth > 0) )
    {
	/*
	 *  The window has a window manager border.
	 */
	pcd->clientOffset.x = pcd->frameInfo.lowerBorderWidth + 
			          pcd->matteWidth;
	pcd->clientOffset.y = pcd->frameInfo.upperBorderWidth + 
				  pcd->frameInfo.titleBarHeight + 
			          pcd->matteWidth;
    }
    else 
    {
	/*
	 * No window manager border, the original X border is showing
	 * through.
	 */
	pcd->clientOffset.x =  pcd->xBorderWidth + pcd->matteWidth;
	pcd->clientOffset.y =  pcd->xBorderWidth + 
				   pcd->frameInfo.titleBarHeight + 
				   pcd->matteWidth;
    }
}



/*************************************<->*************************************
 *
 *  XBorderIsShowing (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Returns true if the X border is showing.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 * 
 *  Outputs:
 *  -------
 *  XBorderIsShowing	- True if no window manager border & x border is 
 *			  visible, False otherwise
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/
Boolean XBorderIsShowing (ClientData *pcd)
{
    Boolean rval;

    if ( (pcd->decor & (MWM_DECOR_RESIZEH | MWM_DECOR_BORDER)) ||
         (pcd->matteWidth > 0) )
    {
	rval = False; 	/* hidden by window manager */
    }
    else 
    {
        rval = True;	/* it's showing */
    }
    return(rval);
}
