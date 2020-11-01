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
/*   $XConsortium: WmCDInfo.h /main/4 1995/11/01 11:32:34 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern int FrameX (ClientData *pcd);
extern int FrameY (ClientData *pcd);
extern unsigned int FrameWidth (ClientData *pcd);
extern unsigned int FrameHeight (ClientData *pcd);
extern unsigned int TitleTextHeight (ClientData *pcd);
extern unsigned int UpperBorderWidth (ClientData *pcd);
extern unsigned int LowerBorderWidth (ClientData *pcd);
extern unsigned int CornerWidth (ClientData *pcd);
extern unsigned int CornerHeight (ClientData *pcd);
extern int BaseWindowX (ClientData *pcd);
extern int BaseWindowY (ClientData *pcd);
extern unsigned int BaseWindowWidth (ClientData *pcd);
extern unsigned int BaseWindowHeight (ClientData *pcd);
extern Boolean GetFramePartInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern int IdentifyFramePart (ClientData *pCD, int x, int y);
extern int GadgetID (int x, int y, GadgetRectangle *pgadget, unsigned int count);
extern void FrameToClient (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern void ClientToFrame (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern Boolean GetDepressInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight, unsigned int *pInvertWidth);
extern void SetFrameInfo (ClientData *pcd);
extern void SetClientOffset (ClientData *pcd);
extern Boolean XBorderIsShowing (ClientData *pcd);
extern unsigned int InitTitleBarHeight (ClientData *pcd);

/*
 * TitleBarHeight() is now a simple macro instead of a procedure.
 */

#define TitleBarHeight(pcd) ((pcd)->frameInfo.titleBarHeight)
