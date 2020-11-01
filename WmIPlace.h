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
/*   $XConsortium: WmIPlace.h /main/4 1995/11/01 11:41:34 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void InitIconPlacement (WmWorkspaceData *pWS);
extern int GetNextIconPlace (IconPlacementData *pIPD);
extern void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY);
extern int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y);
extern int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y);
extern void PackRootIcons (void);
extern void MoveIconInfo (IconPlacementData *pIPD, int p1, int p2);
