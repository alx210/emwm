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

void AddIconBoxForWorkspace (WmWorkspaceData *pWS);

void AddNewBox (WmWorkspaceData *pWS, IconBoxData *pIBD);
void ChangeActiveIconboxIconText (Widget icon, caddr_t dummy, 
					 XFocusChangeEvent *event);
void CheckIconBoxResize (ClientData *pCD, unsigned int changedValues, 
				int newWidth, int newHeight);
Boolean CheckIconBoxSize (IconBoxData *pIBD);
void DeleteIconFromBox (IconBoxData *pIBD, ClientData *pCD);
void DeleteIconInfo (IconBoxData *pIBD, ClientData *pCD);
Boolean ExpandVirtualSpace (IconBoxData *pIBD, int newWidth, 
				   int newHeight);
Boolean ExtendIconList (IconBoxData *pIBD, int incr);
void FindNewPosition (Cardinal *newPosition, IconPlacementData *pIPD, 
			     int newPlace);
MenuItem *GetIconBoxMenuItems (WmScreenData *pSD);
void GetIconBoxIconRootXY (ClientData *pCD, int *pX, int *pY);
void HandleIconBoxButtonMotion (Widget icon, caddr_t client_data, 
				       XEvent *pev);
void HandleIconBoxIconKeyPress (Widget icon, caddr_t dummy, 
				       XKeyEvent *keyEvent);
#ifndef MOTIF_ONE_DOT_ONE
void IconScrollVisibleCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
#endif
void IconActivateCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
Boolean IconVisible (ClientData *pCD);
IconInfo *InsertIconInfo (IconBoxData *pIBD, ClientData *pCD, 
				 Widget theWidget);
Boolean InsertIconIntoBox (IconBoxData *pIBD, ClientData *pCD);
void InitIconBox (WmScreenData *pSD);
void InitializeClientData (ClientData *pCD, IconBoxData *pIBD);
void InitializeIconBoxData (WmWorkspaceData *pWS, IconBoxData *pIBD);
Cardinal InsertPosition (Widget w);
void MakeBulletinBoard (WmWorkspaceData *pWS, IconBoxData *pIBD);
void MakeFadeIconGC (WmScreenData *pSD);
Boolean MakeIconBox (WmWorkspaceData *pWS, ClientData *pCD);
void MakeScrolledWindow (WmWorkspaceData *pWS, IconBoxData *pIBD);
void MakeShell (WmWorkspaceData *pWS, IconBoxData *pIBD);
void MakeShrinkWrapIconsGC (WmScreenData *pSD, Pixmap bgPixmap);
void MapIconBoxes (WmWorkspaceData *pWS);
void PackIconBox (IconBoxData *pIBD, Boolean packVert, 
			 Boolean packHorz, int passedInWidth, 
			 int passedInHeight);
void RealignIconList (IconBoxData *pIBD, int newCols, int newRows);
void RealizeIconBox (WmWorkspaceData *pWS, IconBoxData *pIBD, 
			    ClientData *pCD);
void ReorderIconBoxIcons (ClientData *pCD, IconBoxData *pIBD, 
				 Widget theIcon, int newX, int newY);
void ResetArrowButtonIncrements (ClientData *pCD);
void ResetIconBoxMaxSize (ClientData *pCD, Widget bBoardWidget);
void SetGeometry (WmWorkspaceData *pWS, ClientData *pCD, 
			 IconBoxData *pIBD);
void SetGranularity (WmWorkspaceData *pWS, ClientData *pCD, 
			    IconBoxData *pIBD);
void SetIconBoxInfo (WmWorkspaceData *pWS, ClientData *pCD);
void SetNewBounds (IconBoxData *pIBD);
void ShowClientIconState (ClientData *pCD, int newState);

void UnmapIconBoxes (WmWorkspaceData *pWS);

void UpdateIncrements (Widget sWidget, IconBoxData *pIBD, 
			      XConfigureEvent *event);
String WmXmStringToString (XmString xmString);
void DestroyIconBox (WmWorkspaceData *pWS);
