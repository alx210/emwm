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
 * Motif Release 1.2
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmGraphics.c /main/4 1995/11/01 11:38:53 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/X.h>
#include <Xm/Xm.h>


#define RLIST_EXTENSION_SIZE	10

#ifndef MIN
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/*
 * include extern functions
 */
#include "WmGraphics.h"
#include "WmError.h"



/*
 * Global Variables:
 */


/*
 * Macros:
 */

/* test if > 0 and return 1 if true, 0 if false. */
#define GE1(x) ((x)>0?1:0)



/*************************************<->*************************************
 *
 *   Procedure:	BevelRectangle (prTop, prBot, x, y, 
 *                      width, height, top_wid, right_wid, bot_wid, left_wid)
 *
 *  Description:
 *  -----------
 *  Generate data for top- and bottom-shadow bevels on a box.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle to bevel
 *  width 	- (outside) width of rectangle
 *  height 	- (outside) height of rectangle
 *  top_wid	- width of beveling on top side of rectangle
 *  right_wid	- width of beveling on right side of rectangle
 *  bot_wid	- width of beveling on bottom side of rectangle
 *  left_wid	- width of beveling on left side of rectangle
 * 
 *  Outputs:
 *  -------
 *  prTop	- top shadows for this rectangle added to list
 *  prBot	- bottom shadows for this rectangle added to list
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

void BevelRectangle (RList *prTop, RList *prBot, int x, int y, unsigned int width, unsigned int height, unsigned int top_wid, unsigned int right_wid, unsigned int bot_wid, unsigned int left_wid)
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    register int count;		/* counter used for beveling operation */
    int join1, join2;		/* used to compute "good" bevel joints */
    int x1, y1, len;		/* used to compute bevel parameters */
    int *piTop, *piBot;


    /* build the rectangles to implement the beveling on each side */

    /* top side */

    if (((prTop->used + (top_wid + left_wid)) > prTop->allocated) &&
	(!ExtendRList (prTop, MAX (top_wid+left_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enough memory */
    }

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x;
    y1 = y;
    len = width;
    for (count=top_wid; count>0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* left side */

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x;
    y1 = y+GE1(join1);
    len = height-GE1(join1);
    for (count=left_wid; count >0; count--, prect++, (*piTop)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }


    /* bottom side */

    if (((prBot->used + (bot_wid + right_wid)) > prBot->allocated) &&
	(!ExtendRList(prBot, MAX (bot_wid+right_wid, RLIST_EXTENSION_SIZE))))
    {
	return;
    }

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x+GE1(join1);
    y1 = y+height-1;
    len = width-GE1(join1);
    /* fudge fat bottom shadow to overwrite corner of skinny left shadow */
    if (GE1(join1) && (bot_wid > left_wid)) {
	len++;
	x1--;
	join1++;
    }
    for (count=bot_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* right side */

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x+width-1;
    y1 = y+GE1(join1);
    len = height - GE1(join1) - GE1(join2);
    /* fudge fat right shadow to overwrite corner of skinny top shadow */
    if (GE1(join1) && (right_wid > top_wid)) {
	len++;
	y1--;
	join1++;
    }
    for (count=right_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

} /* END OF FUNCTION BevelRectangle */




/*************************************<->*************************************
 *
 *   Procedure:	BevelDepressedRectangle (prTop, prBot, x, y, 
 *                      width, height, top_wid, right_wid, bot_wid, left_wid
 *                      in_wid)
 *
 *  Description:
 *  -----------
 *  Generate data for top- and bottom-shadow bevels on a rectangle with
 *  the center part depressed.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle to bevel
 *  width 	- (outside) width of rectangle
 *  height 	- (outside) height of rectangle
 *  top_wid	- width of beveling on top side of rectangle
 *  right_wid	- width of beveling on right side of rectangle
 *  bot_wid	- width of beveling on bottom side of rectangle
 *  left_wid	- width of beveling on left side of rectangle
 *  in_wid	- width of depressed beveling inside of rectangle
 * 
 *  Outputs:
 *  -------
 *  prTop	- top shadows for this rectangle added to list
 *  prBot	- bottom shadows for this rectangle added to list
 *  
 *
 *  Comments:
 *  --------
 *
 *************************************<->***********************************/

void BevelDepressedRectangle (RList *prTop, RList *prBot, int x, int y, unsigned int width, unsigned int height, unsigned int top_wid, unsigned int right_wid, unsigned int bot_wid, unsigned int left_wid, unsigned int in_wid)
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    register int count;		/* counter used for beveling operation */
    int join1, join2;		/* used to compute "good" bevel joints */
    int x1, y1, len;		/* used to compute bevel parameters */
    int *piTop, *piBot;


    /* 
     *  Build the rectangles to implement the beveling on each side 
     *  First, guarantee that there is enough memory.
     */


    if (((prTop->used + (top_wid + left_wid)) > prTop->allocated) &&
	(!ExtendRList (prTop, MAX (top_wid+left_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enough memory */
    }

    if (((prBot->used + (bot_wid + right_wid)) > prBot->allocated) &&
	(!ExtendRList(prBot, MAX (bot_wid+right_wid, RLIST_EXTENSION_SIZE))))
    {
	return;		/* not enought memory */
    }



    /* top side (normal beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x;
    y1 = y;
    len = width;
    for (count=top_wid - in_wid; count>0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* top side (inverted beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    for (count=in_wid; count>0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 += 1;
	len -= GE1(join1) + GE1(--join2);
    }



    /* left side (normal beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x;
    y1 = y+GE1(join1);
    len = height-GE1(join1);
    for (count=left_wid-in_wid; count >0; count--, prect++, (*piTop)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

    /* left side (inverted beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    for (count=in_wid; count >0; count--, prect++, (*piBot)++) 
    {	
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 += 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }



    /* bottom side (normal beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = left_wid;
    join2 = right_wid;
    x1 = x+GE1(join1);
    y1 = y+height-1;
    len = width-GE1(join1);
    /* fudge fat bottom shadow to overwrite corner of skinny left shadow */
    if (GE1(join1) && (bot_wid > left_wid)) {
	len++;
	x1--;
	join1++;
    }
    for (count=bot_wid-in_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }

    /* bottom side (inverted beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    for (count=in_wid; count >0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = len;
	prect->height = 1;
	x1 += GE1(--join1);
	y1 -= 1;
	len -= GE1(join1) + GE1(--join2);
    }



    /* right side (normal beveling) */

    piBot = &(prBot->used);
    prect = &(prBot->prect[*piBot]);

    join1 = top_wid;
    join2 = bot_wid;
    x1 = x+width-1;
    y1 = y+GE1(join1);
    len = height - GE1(join1) - GE1(join2);
    /* fudge fat right shadow to overwrite corner of skinny top shadow */
    if (GE1(join1) && (right_wid > top_wid)) {
	len++;
	y1--;
	join1++;
    }
    for (count=right_wid-in_wid; count >0; count--, prect++, (*piBot)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

    /* right side (inverted beveling) */

    piTop = &(prTop->used);
    prect = &(prTop->prect[*piTop]);

    for (count=in_wid; count >0; count--, prect++, (*piTop)++) 
    {
	prect->x = x1;
	prect->y = y1;
	prect->width = 1;
	prect->height = len;
	x1 -= 1;
	y1 += GE1(--join1);
	len -= GE1(join1) + GE1(--join2);
    }

} /* END OF FUNCTION BevelDepressedRectangle */




/*************************************<->*************************************
 *
 *   Procedure:	StretcherCorner (prTop, prBot, x, y, cnum, 
 *                               swidth,  cwidth, cheight);
 *
 *  Description:
 *  -----------
 *  Generate data to draw a corner of the stretcher border.
 *
 *  Inputs:
 *  ------
 *  prTop	- ptr to top shadow RList
 *  prBot	- ptr to bottom shadow RList
 *  x,y		- position of rectangle enclosing the cornern
 *  cnum 	- corner number; which corner to draw
 *                ASSUMES only NW, NE, SE, SW for mwm ()
 *  swidth 	- width (thickness) of border (includes bevels)
 *  cwidth 	- corner width from corner to end of horizontal run
 *  cheight 	- corner height from corner to end of vertical run
 * 
 *  Outputs:
 *  -------
 *  prTop	- array filled in for top shadows 
 *  prBot	- array filledin for bottom shadows
 *
 *  Comments:
 *  --------
 *  o Uses only 1 pixel bevels. Beveling is hard coded.
 *  o XFillRectangles assumed as an optimization to take
 *    advantage of the block mover hardware.
 *
 *************************************<->***********************************/

void StretcherCorner (RList *prTop, RList *prBot, int x, int y, int cnum, unsigned int swidth, unsigned int cwidth, unsigned int cheight)
{
    XRectangle *prect;		/* pointer to "current" rectangle */
    int *piTop, *piBot;

    switch (cnum) {

	case STRETCH_NORTH_WEST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top (row 1) */
		prect->y = y;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* top (row 2) */
		prect->y = y+1;
		prect->width = cwidth-2;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left (col 1) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* left (col 2) */
		prect->y = y+2;
		prect->width = 1;
		prect->height = cheight-3;
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+1;			/* bottom end */
		prect->y = y+cheight-1;
		prect->width = swidth-1;
		prect->height = 1;
		prect++;
		(*piBot)++;

		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x+swidth-1;		/* right inside */
		    prect->y = y+swidth-1;
		    prect->width = 1;
		    prect->height = cheight-swidth;
		    prect++;
		    (*piBot)++;

		    prect->x = x+swidth;		/* bottom inside */
		    prect->y = y+swidth-1;
		    prect->width = cwidth-swidth;
		    prect->height = 1;
		    prect++;
		    (*piBot)++;
		}

		prect->x = x+cwidth-1;		/* right end */
		prect->y = y+1;
		prect->width = 1;
		prect->height = swidth-1-((wmGD.frameStyle == WmSLAB)? 0 : 1);
		(*piBot)++;

		break;

	case STRETCH_NORTH_EAST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top (row 1) */
		prect->y = y;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* top (row 2) */
		prect->y = y+1;
		prect->width = cwidth-2;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left end */
		prect->y = y+1;
		prect->width = 1;
		prect->height = swidth-1;
		prect++;
		(*piTop)++;

		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x+cwidth-swidth;	/* left inside (col 1) */
		    prect->y = y+swidth;
		    prect->width = 1;
		    prect->height = cheight-swidth;
		    (*piTop)++;
		}

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		/* bottom end */
		prect->x = x+cwidth-swidth+((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect->y = y+cheight-1;
		prect->width = swidth-((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right (col 2) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-2;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-2;		/* right (col 1) */
		prect->y = y+2;
		prect->width = 1;
		prect->height = cheight-3;
		prect++;
		(*piBot)++;

		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x+1;		/* bottom inside (row 2) */
		    prect->y = y+swidth-1;
		    prect->width = cwidth-swidth;
		    prect->height = 1;
		    (*piBot)++;
		}

		break;

	case STRETCH_SOUTH_EAST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x;			/* top inside */
		    prect->y = y+cheight-swidth;
		    prect->width = cwidth-swidth+1;
		    prect->height = 1;
		    prect++;
		    (*piTop)++;

		    prect->x = x+cwidth-swidth;	/* left inside */
		    prect->y = y;
		    prect->width = 1;
		    prect->height = cheight-swidth;
		    prect++;
		    (*piTop)++;
		}

		/* top end */
		prect->x = x+cwidth-swidth+
				((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect->y = y;
		prect->width = swidth-2+((wmGD.frameStyle == WmSLAB)? 1 : 0);;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left end */
		prect->y = y+cheight-swidth+
				((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect->width = 1;
		prect->height = swidth-2+((wmGD.frameStyle == WmSLAB)? 1 : 0);
		(*piTop)++;

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		prect->x = x+1;			/* bottom - row 1 */
		prect->y = y+cheight-2;
		prect->width = cwidth-1;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x;			/* bottom - row 2 */
		prect->y = y+cheight-1;
		prect->width = cwidth;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-2;		/* right  - column 1 */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-3;
		prect++;
		(*piBot)++;

		prect->x = x+cwidth-1;		/* right  - column 2 */
		prect->y = y;
		prect->width = 1;
		prect->height = cheight-2;
		(*piBot)++;

		break;

	case STRETCH_SOUTH_WEST:
		if (((prTop->used + 4) > prTop->allocated) &&
		    (!ExtendRList (prTop, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piTop = &(prTop->used);
		prect = &(prTop->prect[*piTop]);

		prect->x = x;			/* top end */
		prect->y = y;
		prect->width = swidth;
		prect->height = 1;
		prect++;
		(*piTop)++;

		prect->x = x;			/* left (col 1) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-1;
		prect++;
		(*piTop)++;

		prect->x = x+1;			/* left (col 2) */
		prect->y = y+1;
		prect->width = 1;
		prect->height = cheight-2;
		prect++;
		(*piTop)++;

		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x+swidth;		/* top inside (row 2) */
		    prect->y = y+cheight-swidth;
		    prect->width = cwidth-swidth;
		    prect->height = 1;
		    (*piTop)++;
		}

		if (((prBot->used + 4) > prBot->allocated) &&
		    (!ExtendRList (prBot, (unsigned int) RLIST_EXTENSION_SIZE)))
		{
		    return;
		}

		piBot = &(prBot->used);
		prect = &(prBot->prect[*piBot]); /* bottom shadow parts */


		if (wmGD.frameStyle == WmRECESSED)
		{
		    prect->x = x+swidth-1;	/* right inside (col 2) */
		    prect->y = y+1;
		    prect->width = 1;
		    prect->height = cheight-swidth;
		    prect++;
		    (*piBot)++;
		}

		prect->x = x+cwidth-1;		/* right end */
		prect->y = y+cheight-swidth+
				((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect->width = 1;
		prect->height = swidth-((wmGD.frameStyle == WmSLAB)? 0 : 1);
		prect++;
		(*piBot)++;

		prect->x = x+2;			/* bottom (row 1) */
		prect->y = y+cheight-2;
		prect->width = cwidth-3;
		prect->height = 1;
		prect++;
		(*piBot)++;

		prect->x = x+1;			/* bottom (row 2) */
		prect->y = y+cheight-1;
		prect->width = cwidth-2;
		prect->height = 1;
		(*piBot)++;

		break;
    }
} /* END OF FUNCTION StretcherCorner */



/*************************************<->*************************************
 *
 *  DrawStringInBox (dpy, win, gc, pbox, str)
 *
 *
 *  Description:
 *  -----------
 *  Draws a null-terminated string inside the specified box (rectangle)
 *
 *
 *  Inputs:
 *  ------
 *  dpy		- ptr to Display
 *  win		- an X Window
 *  gc 		- graphics context to use
 *  pfs		- pointer to XFontStruct for the font in "gc"
 *  pbox	- ptr to XRectangle that encloses text
 *  str		- String to write
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  o Assumes 8-bit text for now.
 *  o Algorithm:  
 *			get length of String
 *			if String is short than box width then
 *			    draw string centered in box
 *			otherwise
 *			    draw string left justified and clipped to box
 *  o The clip_x_origin, clip_y_origin, and clip_mask are reset to None
 *    upon exit.
 *  o Due to bugs and / or misunderstanding, I gave up on trying to 
 *    extract the XFontStruct from the GC. I just made it a separate 
 *    parameter.
 *			
 *************************************<->***********************************/
void DrawStringInBox (Display *dpy, Window win, GC gc, XFontStruct *pfs, XRectangle *pbox, String str)
{
    XGCValues gcv;
    int textWidth;
    int xCenter;
    XRectangle clipBox;

    /* compute text position */
    textWidth = XTextWidth(pfs, str, strlen(str));

    if (textWidth < (int) pbox->width) {	/* center text if there's room */
	xCenter = (int) pbox->x + ((int) pbox->width - textWidth) / 2 ;
	WmDrawString(dpy, win, gc, xCenter, (pbox->y + pfs->ascent), 
		    str, strlen(str));
    }
    else {				/* left justify & clip text */
	clipBox.x = 0;			/* set up clip rectangle */
	clipBox.y = 0;
	clipBox.width = pbox->width;
	clipBox.height = pbox->height;

	XSetClipRectangles (dpy, gc, pbox->x, pbox->y,	/* put into gc */
			    &clipBox, 1, Unsorted);

	WmDrawString(dpy, win, gc, pbox->x, (pbox->y + pfs->ascent), 
		    str, strlen(str));

	gcv.clip_x_origin = 0;		/* erase clip_mask from gc */
	gcv.clip_y_origin = 0;
	gcv.clip_mask = None;
	XChangeGC (dpy, gc, 
		   GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
    }
} /* END OF FUNCTION DrawStringInBox */




/*************************************<->*************************************
 *
 *  ExtendRList (prl, amt)
 *
 *
 *  Description:
 *  -----------
 *  Extends the size of the RList
 *
 *
 *  Inputs:
 *  ------
 *  prl		- ptr to Display
 *  amt		- how much to extend it by
 * 
 *  Outputs:
 *  -------
 *  Returns True if succeeded, false otherwise.
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
Boolean ExtendRList (RList *prl, unsigned int amt)
{
    unsigned int total, count;
    XRectangle *pNewRect;
    Boolean rval;


    total = prl->allocated + amt;
    if ( (pNewRect = (XRectangle *) XtMalloc (total * sizeof(XRectangle))) 
	 == NULL)
    {
	Warning (((char *)GETMESSAGE(28, 1, "Insufficient memory for graphics data")));
	rval = False;
    }
    else 
    {
	prl->allocated = total;
	rval = True;

	if (prl->used != 0) {		/* copy from old structure */
	    count = prl->used * sizeof(XRectangle);
	    (void) memcpy ((void *)pNewRect, (void *)prl->prect, count);
	    if (prl->prect != NULL)
		XtFree ((char *)prl->prect);
	    prl->prect = pNewRect;
	}
    }
    return (rval);
} /* END OF FUNCTION ExtendRList */



/*************************************<->*************************************
 *
 *  AllocateRList (amt)
 *
 *
 *  Description:
 *  -----------
 *  Allocates an RList of size "amt"
 *
 *
 *  Inputs:
 *  ------
 *  amt		- number of XRectangles to allocate in list
 * 
 *  Outputs:
 *  -------
 *  Returns ptr to new RList structure if success, returns NULL ptr otherwise
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
RList *AllocateRList (amt)

    unsigned int amt;
{
    RList *prl;


    if ((prl = (RList *) XtMalloc (sizeof (RList))) != NULL) 
    {
	if ( (prl->prect = (XRectangle *) XtMalloc (amt * sizeof(XRectangle))) 
	     == NULL)
	{
	    XtFree ((char *)prl);
	    prl = NULL;
	}
	else 
	{
	    prl->allocated = amt;
	    prl->used = 0;

	}
    }

    return (prl);
} /* END OF FUNCTION AllocateRList */



/*************************************<->*************************************
 *
 *  FreeRList (prl)
 *
 *
 *  Description:
 *  -----------
 *  Frees an RList
 *
 *
 *  Inputs:
 *  ------
 *  prl		- ptr to RList to free
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *			
 *************************************<->***********************************/
void FreeRList (RList *prl)
{
    if (prl) 
    {
	if (prl->prect) 
	    XtFree ((char *)prl->prect);

	XtFree ((char *)prl);
    }
}/* END OF FUNCTION FreeRList */


/*************************************<->*************************************
 *
 *  WmDrawString
 *
 *
 *  Description:
 *  -----------
 *  Draws a string
 *
 *
 *  Inputs:
 *  ------
 *  (same parameters used by XDrawString and XDrawImageString)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o If wmGD.cleanText is True, then the text is drawn using 
 *    XDrawImageString. This provides some clean space around the text
 *    if the background area is stippled -- especially useful on 
 *    B/W displays.
 *			
 *************************************<->***********************************/
void WmDrawString (Display *dpy, Drawable d, GC gc, int x, int y, char *string, unsigned int length)
{
    if (ACTIVE_PSD->cleanText)
    {
	XDrawImageString(dpy, d, gc, x, y, string, length);
    }
    else
    {
	XDrawString(dpy, d, gc, x, y, string, length);
    }

}/* END OF FUNCTION WmDrawString */



/*************************************<->*************************************
 *
 *  WmXmDrawString
 *
 *
 *  Description:
 *  -----------
 *  Draws a string
 *
 *
 *  Inputs:
 *  ------
 *  (subset of parameters used by XmStringDraw and XmStringDrawImage)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  o If wmGD.cleanText is True, then the text is drawn using 
 *    XmStringDrawImage. This provides some clean space around the text
 *    if the background area is stippled -- especially useful on 
 *    B/W displays.
 *			
 *************************************<->***********************************/
#ifdef WSM
void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
		     XmString xmstring, GC gc, Position x, Position y, 
		     Dimension width,  XRectangle *pbox, Boolean bCenter)
#else /* WSM */
void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
		     XmString xmstring, GC gc, Position x, Position y, 
		     Dimension width,  XRectangle *pbox)
#endif /* WSM */
{
    Dimension textWidth;
#ifdef WSM
    int alignment;
#else /* WSM */
    int alignment = XmALIGNMENT_BEGINNING;
#endif /* WSM */
    

    textWidth = XmStringWidth(xmfontlist, xmstring);

#ifdef WSM
    alignment = bCenter ? XmALIGNMENT_CENTER : XmALIGNMENT_BEGINNING;

    if (textWidth >= pbox->width)  /* can't center text if no room */
    {                              /* left justify & clip text */
	alignment = XmALIGNMENT_BEGINNING;
    }
#else /* WSM */
    if (textWidth < pbox->width) {      /* center text if there's room */
	alignment = XmALIGNMENT_CENTER;
    }
    else 
    {                              /* left justify & clip text */
	alignment = XmALIGNMENT_BEGINNING;
    }
#endif /* WSM */
    
    if (ACTIVE_PSD->cleanText)
    {
	XmStringDrawImage(dpy, w, xmfontlist, xmstring, gc, x, y, width, 
			  alignment, XmSTRING_DIRECTION_L_TO_R, 
			  pbox);
    }
    else
    {
	XmStringDraw (dpy, w, xmfontlist, xmstring, gc, x, y, width, 
		      alignment, XmSTRING_DIRECTION_L_TO_R, pbox);
    }
} /* END OF FUNCTION WmDrawXmString */

#ifdef WSM

/*************************************<->*************************************
 *
 *  WmInstallBitmapIntoXmCache (pchName, bitmap, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Installs all or part of a pixmap into the Xm cache. This pixmap
 *  may be retrieved later by a call to XmGetPixmap.
 *
 *  Inputs:
 *  ------
 *  pchName	= pointer to name of bitmap 
 *  bitmap	= depth-1 pixmap
 *  width 	= width of portion to install
 *  height	= height of portion to install
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  This always installs the Northwest corner of the passed in bitmap.
 *  If the width and height match the size of the bitmap, then the
 *  whole thing is installed in the cache.
 *			
 *************************************<->***********************************/
void WmInstallBitmapIntoXmCache (unsigned char *pchName,
    Pixmap bitmap, unsigned int width, unsigned int height)
{
    XImage *pImage;

    pImage = XGetImage (DISPLAY, bitmap, 0, 0, width, height, 1L, XYBitmap);

    XmInstallImage (pImage, (char *)pchName);
} /* END OF FUNCTION WmInstallBitmapIntoXmCache */



/*************************************<->*************************************
 *
 *  WmInstallBitmapDataIntoXmCache (pSD, pchName, pData)
 *
 *
 *  Description:
 *  -----------
 *  Installs built-in bitmap data into the Xm Pixmap cache. The image
 *  may be retrieved later by a call to XmGetPixmap.
 *
 *  Inputs:
 *  ------
 *  pSD		= pointer to screen data
 *  pchName	= pointer to name of bitmap 
 *  pData	= pointer to the bitmap data
 * 
 *  Outputs:
 *  -------
 *  none
 *
 *  Comments:
 *  --------
 *  This is principally for putting built-in pixmap data into the Xm
 *  cache to allow for uniform access to pixmap creation.
 *
 *  ***WARNING***
 *  Do NOT call XmDestroyPixmap on images cached via this routine unless
 *  pData passed in points to malloc'ed memory. XmDestroyPixmap could 
 *  try to free this data.
 *			
 *************************************<->***********************************/
void WmInstallBitmapDataIntoXmCache (WmScreenData *pSD, 
    unsigned char *pchName, char *pData, unsigned int width,
    unsigned int height)
{
    XImage *pImage;

    if (pImage = (XImage *) XtMalloc (sizeof (XImage)))
    {
	pImage->width = width; 
	pImage->height = height;
	pImage->xoffset = 0;
	pImage->data = pData;
	pImage->format = XYBitmap;
	pImage->byte_order = MSBFirst;
	pImage->bitmap_pad = 8;
	pImage->bitmap_bit_order = LSBFirst;
	pImage->bitmap_unit = 8;
	pImage->depth = 1;
	pImage->bytes_per_line = (width/8) + ((width%8) != 0 ? 1 : 0);
	pImage->obdata = NULL;

	XmInstallImage(pImage, (char *)pchName);
    }

} /* END OF FUNCTION WmInstallBitmapDataIntoXmCache */
#endif /* WSM */
