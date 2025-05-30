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

void InitIconPlacement (WmWorkspaceData *pWS);
int GetNextIconPlace (IconPlacementData *pIPD);
void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY);
int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y);
int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y);
void PackRootIcons (void);
void MoveIconInfo (IconPlacementData *pIPD1, int p1,
	IconPlacementData *pIPD2, int p2);
IconPlacementData* PositionToPlacementData(WmWorkspaceData *pWS, int x, int y);
