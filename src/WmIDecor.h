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

void CreateActiveIconTextWindow (WmScreenData *pSD);
void DrawIconTitle (ClientData *pcd);
void GetIconDimensions (WmScreenData *pSD, unsigned int *pWidth, 
			       unsigned int *pLabelHeight, 
			       unsigned int *pImageHeight);
void GetIconTitleBox (ClientData *pcd, XRectangle *pBox);
void HideActiveIconText (WmScreenData *pSD);
void IconExposureProc (ClientData *pcd, Boolean clearFirst);
void InitIconSize (WmScreenData *pSD);
Boolean MakeIcon (WmWorkspaceData *pWS, ClientData *pcd);
void MakeIconShadows (ClientData *pcd, int xOffset, int yOffset);
void MoveActiveIconText (ClientData *pcd);
void PaintActiveIconText (ClientData *pcd, Boolean erase);
void PutBoxInIconBox (ClientData *pCD, int *px, int *py, 
			     unsigned int *width, unsigned int *height);
void PutBoxOnScreen (int screen, int *px, int *py, unsigned int width, 
			    unsigned int height);
void RedisplayIconTitle (ClientData *pcd);
void ReparentIconWindow (ClientData *pcd, int xOffset, int yOffset);
void ShowActiveIcon (ClientData *pcd);
void ShowActiveIconText (ClientData *pcd);
void ShowInactiveIcon (ClientData *pcd, Boolean refresh);


