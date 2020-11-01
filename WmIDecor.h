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
/*   $XConsortium: WmIDecor.h /main/4 1995/11/01 11:40:37 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


extern void CreateActiveIconTextWindow (WmScreenData *pSD);
extern void DrawIconTitle (ClientData *pcd);
extern void GetIconDimensions (WmScreenData *pSD, unsigned int *pWidth, 
			       unsigned int *pLabelHeight, 
			       unsigned int *pImageHeight);
extern void GetIconTitleBox (ClientData *pcd, XRectangle *pBox);
extern void HideActiveIconText (WmScreenData *pSD);
extern void IconExposureProc (ClientData *pcd, Boolean clearFirst);
extern void InitIconSize (WmScreenData *pSD);
extern Boolean MakeIcon (WmWorkspaceData *pWS, ClientData *pcd);
extern void MakeIconShadows (ClientData *pcd, int xOffset, int yOffset);
extern void MoveActiveIconText (ClientData *pcd);
extern void PaintActiveIconText (ClientData *pcd, Boolean erase);
extern void PutBoxInIconBox (ClientData *pCD, int *px, int *py, 
			     unsigned int *width, unsigned int *height);
extern void PutBoxOnScreen (int screen, int *px, int *py, unsigned int width, 
			    unsigned int height);
extern void RedisplayIconTitle (ClientData *pcd);
extern void ReparentIconWindow (ClientData *pcd, int xOffset, int yOffset);
extern void ShowActiveIcon (ClientData *pcd);
extern void ShowActiveIconText (ClientData *pcd);
extern void ShowInactiveIcon (ClientData *pcd, Boolean refresh);


