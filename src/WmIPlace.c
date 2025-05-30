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

/*
 * include extern functions
 */

#include "WmError.h"
#include "WmIDecor.h"
#include "WmIconBox.h"
#include "WmWinConf.h"
#include "WmXinerama.h"
#include "WmWrkspace.h"


/*
 * Function Declarations:
 */
#include "WmIPlace.h"

static Boolean InitIconPlacementData(WmScreenData*, IconPlacementData*,
	int scr_width, int scr_height);
static void ResetClientIconPlacementData(WmWorkspaceData*);

/*
 * Global Variables:
 */
extern Dimension clipWidth;
extern Dimension clipHeight;
extern Position clipX;
extern Position clipY;

/*************************************<->*************************************
 *
 *  InitIconPlacement ()
 *
 *
 *  Description:
 *  -----------
 *  This function intializes icon placement information.
 *
 *  Inputs:
 *  ------
 *  pWS = pointer to workspace data
 *
 * 
 *  Outputs:
 *  -------
 *  IconPlacementData
 *
 *************************************<->***********************************/

void InitIconPlacement (WmWorkspaceData *pWS)
{
	int i, nxs = 0;
	XineramaScreenInfo xsi;
	Boolean xinerama = False;
	int scr_width = 0;
	int scr_height = 0;
	IconPlacementData *ipd;
	Boolean realloc = False;
	
	xinerama = GetXineramaScreenCount(&nxs);
	
	if(!xinerama) {
		nxs = 1;
		scr_width = DisplayWidth(DISPLAY, pWS->pSD->screen);
		scr_height = DisplayHeight(DISPLAY, pWS->pSD->screen);
	}
	
	ipd = (IconPlacementData*) XtCalloc(nxs, sizeof(IconPlacementData));

	if(!ipd) {
		Warning (((char *)GETMESSAGE(34, 2,
			"Insufficient memory for icon placement")));
		wmGD.iconAutoPlace = False;
		return;
	}
	
	if(pWS->IPData) {
		XtFree((char*)pWS->IPData);
		realloc = True;
	}
	pWS->IPData = ipd;

	for(i = 0; i < nxs; i++) {
		if(xinerama && GetXineramaScreenInfo(i, &xsi)) {
			scr_width = xsi.width;
			scr_height = xsi.height;
			pWS->IPData[i].xiOrgX = xsi.x_org;
			pWS->IPData[i].xiOrgY = xsi.y_org;
			pWS->IPData[i].xiScreen = i;
		}

		if(!InitIconPlacementData(pWS->pSD, pWS->IPData + i,
			scr_width, scr_height)) {
			Warning (((char *)GETMESSAGE(34, 2,
				"Insufficient memory for icon placement")));
			wmGD.iconAutoPlace = False;
			return;
		}
	}

	if(realloc) ResetClientIconPlacementData(pWS);
}

static Boolean InitIconPlacementData(WmScreenData *pSD,
	IconPlacementData *IPData, int sW, int sH)
{
	IconPlacementData tmp = *IPData;
    Boolean useMargin;
    int iSpaceX;
    int iSpaceY;
    int placementW;
    int placementH;
    int extraXSpace;
    int extraYSpace;
    int xMargin;
    int yMargin;
    int extraPX;
    int extraPY;
    int i;

    xMargin = yMargin = extraPX = extraPY = 0;

    useMargin = (pSD->iconPlacementMargin >= 0);
    tmp.iconPlacement = pSD->iconPlacement;

    if (useMargin)
    {
	tmp.placementCols =
	    (sW - (2 * pSD->iconPlacementMargin)) / pSD->iconWidth;
	tmp.placementRows =
	    (sH - (2 * pSD->iconPlacementMargin)) / pSD->iconHeight;
    }
    else
    {
	tmp.placementCols = sW / pSD->iconWidth;
	tmp.placementRows = sH / pSD->iconHeight;
    }

    if (tmp.iconPlacement & ICON_PLACE_TIGHT)
    {
	iSpaceX = 0;
	iSpaceY = 0;
	xMargin = 2;
	yMargin = 2;
    }
    else
    {
	do
	{
	    if (useMargin)
	    {
	        iSpaceX = 
		    (sW - (2 * pSD->iconPlacementMargin) -
			  (tmp.placementCols * pSD->iconWidth)) /
			      (tmp.placementCols - 1);
	    }
	    else
	    {
	        iSpaceX = 
		    (sW - (tmp.placementCols * pSD->iconWidth)) /
			       tmp.placementCols;
	    }
	    if (iSpaceX < MINIMUM_ICON_SPACING)
	    {
	        tmp.placementCols--;
	    }
	}
	while (iSpaceX < MINIMUM_ICON_SPACING);

	do
	{
	    if (useMargin)
	    {
	        iSpaceY = (sH - (2 * pSD->iconPlacementMargin) -
		       (tmp.placementRows * pSD->iconHeight)) /
				  (tmp.placementRows - 1);
	    }
	    else
	    {
	        iSpaceY = 
		    (sH - (tmp.placementRows * pSD->iconHeight)) /
					    tmp.placementRows;
	    }
	    if (iSpaceY < MINIMUM_ICON_SPACING)
	    {
	        tmp.placementRows--;
	    }
	}
	while (iSpaceY < MINIMUM_ICON_SPACING);
    }

    tmp.iPlaceW = pSD->iconWidth + iSpaceX;
    tmp.iPlaceH = pSD->iconHeight + iSpaceY;

    placementW = tmp.placementCols * tmp.iPlaceW;
    placementH = tmp.placementRows * tmp.iPlaceH;

    tmp.placeIconX = 
	((tmp.iPlaceW - pSD->iconWidth) + 1) / 2;
    tmp.placeIconY = 
        ((tmp.iPlaceH - pSD->iconHeight) + 1) / 2;

    /*
     * Special case margin handling for TIGHT icon placement
     */
    if (tmp.iconPlacement & ICON_PLACE_TIGHT)
    {
	if (useMargin)
	{
	    xMargin = pSD->iconPlacementMargin;
	    yMargin = pSD->iconPlacementMargin;
	}

	extraXSpace = 0;
	extraYSpace = 0;

	if ((tmp.iconPlacement & ICON_PLACE_RIGHT_PRIMARY) ||
	   (tmp.iconPlacement & ICON_PLACE_RIGHT_SECONDARY))
	    xMargin = sW - placementW - xMargin;

	if ((tmp.iconPlacement & ICON_PLACE_BOTTOM_PRIMARY) ||
	   (tmp.iconPlacement & ICON_PLACE_BOTTOM_SECONDARY))
	    yMargin = sH - placementH - yMargin;
    }
    else
    {
	if (useMargin)
	{
	    xMargin = pSD->iconPlacementMargin - tmp.placeIconX;
	    extraXSpace = sW - (2 * pSD->iconPlacementMargin) -
			  (placementW - iSpaceX);
	    extraPX = (tmp.iconPlacement & ICON_PLACE_RIGHT_PRIMARY) ?
				1 : (tmp.placementCols - extraXSpace);

	    yMargin = pSD->iconPlacementMargin - tmp.placeIconY;
	    extraYSpace = sH - (2 * pSD->iconPlacementMargin) -
			  (placementH - iSpaceY);
	    extraPY = (tmp.iconPlacement & ICON_PLACE_BOTTOM_PRIMARY) ?
				1 : (tmp.placementRows - extraYSpace);
	}
	else
	{
	    xMargin = (sW - placementW + 
			((tmp.iPlaceW - pSD->iconWidth) & 1)) / 2;
	    extraXSpace = 0;
	    yMargin = (sH - placementH + 
			((tmp.iPlaceH - pSD->iconHeight) & 1))/ 2;
	    extraYSpace = 0;

	    if (tmp.iconPlacement & ICON_PLACE_RIGHT_PRIMARY)
	    {
		xMargin = sW - placementW - xMargin;
		tmp.placeIconX = tmp.iPlaceW - pSD->iconWidth - tmp.placeIconX;
	    }
	    if (tmp.iconPlacement & ICON_PLACE_BOTTOM_PRIMARY)
	    {
		yMargin = sH - placementH - yMargin;
		tmp.placeIconY = tmp.iPlaceH - pSD->iconHeight - tmp.placeIconY;
	    }
	}
    }

    /*
     * Setup array of grid row positions and grid column positions:
     */
	if(IPData->placementRows < (tmp.placementRows + 2) ) {
		tmp.placementRowY = (int*)XtRealloc((char*)IPData->placementRowY,
			((tmp.placementRows + 2) * sizeof (int)) );
		if(!tmp.placementRowY) return False;
	} else {
		tmp.placementRowY = IPData->placementRowY;
	}
	
	if(IPData->placementCols < (tmp.placementCols + 2) ) {
		tmp.placementColX = (int*)XtRealloc((char*)IPData->placementColX,
			((tmp.placementCols + 2) * sizeof (int)) );
		if(!tmp.placementColX) return False;
	} else {
		tmp.placementColX = IPData->placementColX;
	}

    tmp.placementRowY[0] = yMargin;
    for (i = 1; i <= tmp.placementRows; i++)
    {
	tmp.placementRowY[i] = tmp.placementRowY[i - 1] + tmp.iPlaceH;
	if ((extraYSpace > 0) && (i >= extraPY))
	{
	    (tmp.placementRowY[i])++;
	    extraYSpace--;
	}
    }

    tmp.placementColX[0] = xMargin;
    for (i = 1; i <= tmp.placementCols; i++)
    {
	tmp.placementColX[i] = tmp.placementColX[i - 1] + tmp.iPlaceW;
	if ((extraXSpace > 0) && (i >= extraPX))
	{
	    (tmp.placementColX[i])++;
	    extraXSpace--;
	}
    }


    /*
     * Setup an array of icon places.
     */
    tmp.totalPlaces = tmp.placementRows * tmp.placementCols;

	if(IPData->totalPlaces < tmp.totalPlaces) {
		tmp.placeList = (IconInfo*)XtRealloc((char*)IPData->placeList,
			(tmp.totalPlaces * sizeof (IconInfo)) );
		if(!tmp.placeList) return False;
    } else {
		tmp.placeList = IPData->placeList;
	}

    memset(tmp.placeList, 0, tmp.totalPlaces * sizeof (IconInfo));

    tmp.onRootWindow = True;
	
	*IPData = tmp;
	
	return True;
} /* END OF FUNCTION InitIconPlacement */



/*************************************<->*************************************
 *
 *  GetNextIconPlace (pIPD)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies and returns the next free icon grid place.
 *
 * 
 *  Outputs:
 *  -------
 *  Return = next free place (index)
 *
 *************************************<->***********************************/

int GetNextIconPlace (IconPlacementData *pIPD)
{
    int i;


    for (i = 0; i < pIPD->totalPlaces; i++)
    {
	if (pIPD->placeList[i].pCD == (ClientData *)NULL)
	{
	    return (i);
	}
    }

    /*
     * All places are filled!  Find an alternative place.
     */

    return (NO_ICON_PLACE);

} /* END OF FUNCTION GetNextIconPlace */



/*************************************<->*************************************
 *
 *  CvtIconPlaceToPosition (pIPD, place, pX, pY)
 *
 *
 *  Description:
 *  -----------
 *  This function converts an icon place (index) into an icon position.
 *
 *
 *  Inputs:
 *  ------
 *  pIPD = ptr to icon placement data
 *
 *  place = place to be converted
 *
 *  wmGD = (iconPlacement ...)
 *
 * 
 *  Outputs:
 *  -------
 *  pX = pointer to icon place X location
 *
 *  pY = pointer to icon place Y location
 * 
 *************************************<->***********************************/

void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY)
{
    int row;
    int col;

    if (pIPD->iconPlacement &
	(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_RIGHT_PRIMARY))
    {
	col = place % pIPD->placementCols;
	row = place / pIPD->placementCols;
    }
    else
    {
	col = place / pIPD->placementRows;
	row = place % pIPD->placementRows;
    }

    if (pIPD->iconPlacement &
	(ICON_PLACE_RIGHT_PRIMARY | ICON_PLACE_RIGHT_SECONDARY))
    {
	col = pIPD->placementCols - col - 1;
    }
    if (pIPD->iconPlacement &
	     (ICON_PLACE_BOTTOM_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY))
    {
	row = pIPD->placementRows - row - 1;
    }

    if (pIPD->onRootWindow)
    {
	*pX = pIPD->placementColX[col] + pIPD->placeIconX + pIPD->xiOrgX;
	*pY = pIPD->placementRowY[row] + pIPD->placeIconY + pIPD->xiOrgY;
    }
    else 
    {
	*pX = col * pIPD->iPlaceW;
	*pY = row * pIPD->iPlaceH;
    }

} /* END OF FUNCTION CvtIconPlaceToPosition */



/*************************************<->*************************************
 *
 *  FindIconPlace (pCD, pIPD, x, y)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to find a free icon place in the proximity of the
 *  specified position.
 *
 *
 *  Inputs:
 *  ------
 *  pIPD = ptr to icon placement data
 *
 *  x = desired x location of icon place
 *
 *  y = desired y location of icon place
 *
 * 
 *  Outputs:
 *  -------
 *  Return = icon place (index)
 *
 *
 *  Comments:
 *  --------
 *  Look first for a free icon place at the position passed in.  If that place
 *  is taken then look at positions that are +- one half the icon width/height
 *  from the postion passed in.  If those positions are taken look at
 *  positions that are +- one half icon placement width/height from the
 *  position passed in.
 * 
 *************************************<->***********************************/

int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y)
{
    int place;
    int i;
    int j;
    int diffX;
    int diffY;
    int altX;
    int altY;
    int amt;

    place = CvtIconPositionToPlace (pIPD, x, y);

    if (place < pIPD->totalPlaces)
    {
	if (pIPD->placeList[place].pCD == (ClientData *)NULL)
	{
	return (place);
	}
    }
    else
    {
        if (pIPD->iconPlacement & ICON_PLACE_LEFT_PRIMARY)
        {
            amt = pIPD->placementCols;              /* add a new row */
        }
        else
        {
            amt = pIPD->placementRows;              /* add a new column */
        }

	if (!ExtendIconList (P_ICON_BOX(pCD), amt))
	{
	    Warning (((char *)GETMESSAGE(34, 4, "Insufficient memory to create icon box data")));
	    return (NO_ICON_PLACE);            
	}
    }
    /*
     * The place for the passed in position is in use, look at places for
     * alternative positions.
     */
	
	x -= pIPD->xiOrgX;
	y -= pIPD->xiOrgY;

	if(x < 0 || y < 0) {
		x = 0;
		y = 0;
	}
	
    for (i = 0; i < 2; i++)
    {
	switch (i)
	{
	    case 0:
	    {
		diffX = ICON_WIDTH(pCD) / 2;
		diffY = ICON_HEIGHT(pCD) / 2;
		break;
	    }

	    case 1:
	    {
		diffX = pIPD->iPlaceW / 2;
		diffY = pIPD->iPlaceH / 2;
		break;
	    }
	}


	for (j = 0; j < 4; j++)
	{
	    switch (j)
	    {
		case 0:
		{
		    if (pIPD->iconPlacement & ICON_PLACE_LEFT_PRIMARY)
		    {
			altX = x - diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_RIGHT_PRIMARY)
		    {
			altX = x + diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_TOP_PRIMARY)
		    {
			altX = x;
			altY = y - diffY;
		    }
		    else
		    {
			altX = x;
			altY = y + diffY;
		    }
		    break;
		}

		case 1:
		{
		    if (pIPD->iconPlacement & ICON_PLACE_LEFT_PRIMARY)
		    {
			altX = x + diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_RIGHT_PRIMARY)
		    {
			altX = x - diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_TOP_PRIMARY)
		    {
			altX = x;
			altY = y + diffY;
		    }
		    else
		    {
			altX = x;
			altY = y - diffY;
		    }
		    break;
		}

		case 2:
		{
		    if (pIPD->iconPlacement & ICON_PLACE_LEFT_SECONDARY)
		    {
			altX = x - diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_RIGHT_SECONDARY)
		    {
			altX = x + diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_TOP_SECONDARY)
		    {
			altX = x;
			altY = y + diffY;
		    }
		    else
		    {
			altX = x;
			altY = y - diffY;
		    }
		    break;
		}

		case 3:
		{
		    if (pIPD->iconPlacement & ICON_PLACE_LEFT_SECONDARY)
		    {
			altX = x + diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_RIGHT_SECONDARY)
		    {
			altX = x - diffX;
			altY = y;
		    }
		    else if (pIPD->iconPlacement & ICON_PLACE_TOP_SECONDARY)
		    {
			altX = x;
			altY = y - diffY;
		    }
		    else
		    {
			altX = x;
			altY = y + diffY;
		    }
		    break;
		}
	    }

	    if (P_ICON_BOX(pCD))
	    {
		GetClipDimensions(pCD, False);
		if (altX < clipX) 
		{
		    return (NO_ICON_PLACE);
		}
		if (altY < clipY) 
		{
		    return (NO_ICON_PLACE);
		}
		if (((int)altX) > ((int)clipX + 
			(int)clipWidth - ((int)ICON_WIDTH(pCD)))) 
		{
		    return (NO_ICON_PLACE);
		}
		if (((int)altY) > ((int)clipY + 
			(int)clipHeight - ((int)ICON_HEIGHT(pCD))))
		{
		    return (NO_ICON_PLACE);
		}
	    }

	    place = CvtIconPositionToPlace (pIPD, altX, altY);
	    if ((pIPD->placeList[place].pCD) == NULL)
	    {
		return (place);
	    }

	}
    }

    /*
     * Couldn't find an unoccupied place in the proximity of the passed-in
     * position.
     */

    return (NO_ICON_PLACE);


} /* END OF FUNCTION FindIconPlace */



/*************************************<->*************************************
 *
 *  CvtIconPostionToPlace (pIPD, x, y)
 *
 *
 *  Description:
 *  -----------
 *  This function converts an icon position to an icon place.
 *
 *
 *  Inputs:
 *  ------
 *  pIPD = ptr to icon placement data
 *
 *  x,y = location to be converted into an icon place
 *
 * 
 *  Outputs:
 *  -------
 *  Return = icon place (index)
 *
 *************************************<->***********************************/

int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y)
{
    int row;
    int col;

    if (pIPD->onRootWindow)
    {
	/*
	 * Scan through the root window row/column arrays and find the 
	 * placement position.
	 */
	x -= pIPD->xiOrgX;
	y -= pIPD->xiOrgY;
	
	if(x < 0 || y < 0) {
		x = 0;
		y = 0;
	}
	
	for (row = 1; row < pIPD->placementRows; row++)
	{
	    if (y < pIPD->placementRowY[row])
	    {
		break;
	    }
	}
	row--;

	for (col = 1; col < pIPD->placementCols; col++)
	{
	    if (x < pIPD->placementColX[col])
	    {
		break;
	    }
	}
	col--;


	if (pIPD->iconPlacement &
	    (ICON_PLACE_RIGHT_PRIMARY | ICON_PLACE_RIGHT_SECONDARY))
	{
	    col = pIPD->placementCols - col - 1;
	}
	if (pIPD->iconPlacement &
	    (ICON_PLACE_BOTTOM_PRIMARY | ICON_PLACE_BOTTOM_SECONDARY))
	{
	    row = pIPD->placementRows - row - 1;
	}
    }
    else
    {
	/* 
	 * convert icon box coords
	 */
	col = x / pIPD->iPlaceW;
	row = y / pIPD->iPlaceH;
    }


    if (pIPD->iconPlacement &
	(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_RIGHT_PRIMARY))
    {
	return ((row * pIPD->placementCols) + col);
    }
    else
    {
	return ((col * pIPD->placementRows) + row);
    }
    

} /* END OF FUNCTION CvtIconPositionToPlace */


/*
 * Finds icon placement data according to x, y position
 */
IconPlacementData* PositionToPlacementData(WmWorkspaceData *pWS, int x, int y)
{
	XineramaScreenInfo xsi;

	if(wmGD.xineramaIconifyToPrimary && GetPrimaryXineramaScreen(&xsi)) {
		return pWS->IPData + xsi.screen_number;
	} else if(GetXineramaScreenFromLocation(x, y, &xsi)) {
		return pWS->IPData + xsi.screen_number;
	}

	return pWS->IPData;
}


/*************************************<->*************************************
 *
 *  PackRootIcons ()
 *
 *
 *  Description:
 *  -----------
 *  This function packs the icons on the root window
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  ---------
 *
 *************************************<->***********************************/

void PackRootIcons (void)
{
    int iOld, iNew;
    ClientData *pCD;
    ClientData *pCD_active;
    int hasActiveText = 1;
	int nxsi, xsi;
    WsClientData *pWsc;

    /* 
     * find context of the activeIconTextWin to get pCD and then 
     * if it is the same as this client, hide it.
     */

    if (XFindContext (DISPLAY, ACTIVE_PSD->activeIconTextWin,
			wmGD.windowContextType, (caddr_t *)&pCD_active))
    {
	hasActiveText = 0;
    }

    /* 
     *  Traverse the list and pack them together
     */

    if (wmGD.iconAutoPlace)
    {
	
	if(!GetXineramaScreenCount(&nxsi)) nxsi = 1;
	
	for(xsi = 0; xsi < nxsi; xsi++) 
	{
		IconPlacementData *IPData = ACTIVE_WS->IPData + xsi;
		
		for (iOld = iNew = 0; iOld < IPData->totalPlaces; iOld++, iNew++)
		{
	    	if (IPData->placeList[iOld].pCD == NULL)
	    	{
				/* advance to next non-null entry */
				while (++iOld < IPData->totalPlaces &&
					!IPData->placeList[iOld].pCD);
	    	}

	    	if (iOld < IPData->totalPlaces && iOld != iNew)
	    	{
			/* move the icon from its old place to the new place */

			MoveIconInfo (IPData, iOld, IPData, iNew);

			pCD = IPData->placeList[iNew].pCD;
			pWsc = GetWsClientData (ACTIVE_WS, pCD);
			pWsc->iconPlace = iNew;
			CvtIconPlaceToPosition(IPData, 
		    	pWsc->iconPlace, &pWsc->iconX, &pWsc->iconY);

			if (hasActiveText && (pCD == pCD_active))
			{
		    	/* hide activeIconTextWin first */
		    	HideActiveIconText ((WmScreenData *)NULL);
		    	XMoveWindow (DISPLAY, pWsc->iconFrameWin, pWsc->iconX, 
			    	 pWsc->iconY);
		    	ShowActiveIconText (pCD);
			}
			else
			{
		    	XMoveWindow (DISPLAY, pWsc->iconFrameWin, pWsc->iconX, 
			    	 pWsc->iconY);
			}
	    	}
		}
	}
    }
} /* END OF FUNCTION PackRootIcons */



/*************************************<->*************************************
 *
 *  MoveIconInfo (pIPD, p1, p2)
 *
 *
 *  Description:
 *  -----------
 *  Move icon info from place 1 to place 2 in the placement list.
 *
 *
 *  Inputs:
 *  ------
 *  pIPD	- ptr to icon placement data
 *  p1		- placement index 1 (source)
 *  p2		- placement index 2 (destination)
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void MoveIconInfo (IconPlacementData *pIPD1,
	 int p1, IconPlacementData *pIPD2, int p2)
{
    WsClientData *pWsc;

    /* only move if destination is empty */
    if (pIPD2->placeList[p2].pCD == NULL)
    {
	pIPD2->placeList[p2].pCD = pIPD1->placeList[p1].pCD;
	pIPD2->placeList[p2].theWidget = pIPD1->placeList[p1].theWidget;

	pWsc = GetWsClientData (pIPD2->placeList[p2].pCD->pSD->pActiveWS,
				pIPD2->placeList[p2].pCD);
	pWsc->iconPlace = p2;

	pIPD1->placeList[p1].pCD =  NULL;
	pIPD1->placeList[p1].theWidget = NULL;
    }
}

/*
 * Resets icon placement data for all clients. This is done after
 * reallocating placement data e.g. on screen changes.
 */
static void ResetClientIconPlacementData(WmWorkspaceData *pWS)
{
	ClientListEntry *e = pWS->pSD->clientList;
	
	while(e) {
		XineramaScreenInfo xsi;
		int s_width, s_height;
		int s_xorg = 0, s_yorg = 0;
		ClientData *cd = e->pCD;
		WsClientData *wscd;
		int iws;
		

		/* Skip icon entries */
		if(e->type == MINIMIZED_STATE) {
			e = e->nextSibling;
			continue;
		}
		
		for(iws = 0; iws < cd->sizeWsList; iws++) {
			wscd = &cd->pWsList[iws];
			
			/* If the icon was placed before, try to find an appropriate
			 * location according to its coordinates, falling back to next free */
			if(wscd->iconPlace != NO_ICON_PLACE && !P_ICON_BOX(cd)) {
				if(GetXineramaScreenFromLocation(
						wscd->iconX, wscd->iconY, &xsi) ||
					GetXineramaScreenFromLocation(
						cd->clientX, cd->clientY, &xsi) ||
					GetPrimaryXineramaScreen(&xsi)) {
					wscd->IPData = pWS->IPData + xsi.screen_number;
					s_width = xsi.width;
					s_height = xsi.height;
					s_xorg = xsi.x_org;
					s_yorg = xsi.y_org;
				} else {
					s_width = XDisplayWidth(DISPLAY, cd->pSD->screen);
					s_height = XDisplayHeight(DISPLAY, cd->pSD->screen);
					wscd->IPData = pWS->IPData;
				}

				if((wscd->iconX - s_xorg) >= s_width ||
					(wscd->iconY - s_yorg) >= s_height) {
					wscd->iconX = 0;
					wscd->iconY = 0;
					wscd->iconPlace = NO_ICON_PLACE;
				} else {
					wscd->iconPlace = FindIconPlace(cd,
						wscd->IPData, wscd->iconX, wscd->iconY);
				}

				if( (wscd->iconPlace != NO_ICON_PLACE) || (wscd->iconPlace =
					GetNextIconPlace(wscd->IPData)) != NO_ICON_PLACE) {

					CvtIconPlaceToPosition(wscd->IPData,
						wscd->iconPlace, &wscd->iconX, &wscd->iconY);
					wscd->IPData->placeList[wscd->iconPlace].pCD = cd;

					XMoveWindow(DISPLAY, ICON_FRAME_WIN(cd),
							ICON_X(cd), ICON_Y(cd));
				} else {
					wscd->IPData = NULL;
				}
			} else {
				wscd->IPData = NULL;
			}
		}

		/* Next client */
		e = e->nextSibling;
	}
}

/****************************   eof    ***************************/
