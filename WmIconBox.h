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
/*   $XConsortium: WmIconBox.h /main/4 1995/11/01 11:42:06 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


#ifdef WSM
extern void AddIconBoxForWorkspace (WmWorkspaceData *pWS);
#endif /* WSM */
extern void AddNewBox (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void ChangeActiveIconboxIconText (Widget icon, caddr_t dummy, 
					 XFocusChangeEvent *event);
extern void CheckIconBoxResize (ClientData *pCD, unsigned int changedValues, 
				int newWidth, int newHeight);
extern Boolean CheckIconBoxSize (IconBoxData *pIBD);
extern void DeleteIconFromBox (IconBoxData *pIBD, ClientData *pCD);
extern void DeleteIconInfo (IconBoxData *pIBD, ClientData *pCD);
extern Boolean ExpandVirtualSpace (IconBoxData *pIBD, int newWidth, 
				   int newHeight);
extern Boolean ExtendIconList (IconBoxData *pIBD, int incr);
extern void FindNewPosition (Cardinal *newPosition, IconPlacementData *pIPD, 
			     int newPlace);
extern MenuItem *GetIconBoxMenuItems (WmScreenData *pSD);
extern void GetIconBoxIconRootXY (ClientData *pCD, int *pX, int *pY);
extern void HandleIconBoxButtonMotion (Widget icon, caddr_t client_data, 
				       XEvent *pev);
extern void HandleIconBoxIconKeyPress (Widget icon, caddr_t dummy, 
				       XKeyEvent *keyEvent);
#ifndef MOTIF_ONE_DOT_ONE
extern void IconScrollVisibleCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
#endif
extern void IconActivateCallback (Widget w, caddr_t client_data, 
				  XmAnyCallbackStruct *call_data);
extern Boolean IconVisible (ClientData *pCD);
extern IconInfo *InsertIconInfo (IconBoxData *pIBD, ClientData *pCD, 
				 Widget theWidget);
extern Boolean InsertIconIntoBox (IconBoxData *pIBD, ClientData *pCD);
extern void InitIconBox (WmScreenData *pSD);
extern void InitializeClientData (ClientData *pCD, IconBoxData *pIBD);
extern void InitializeIconBoxData (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern Cardinal InsertPosition (Widget w);
extern void MakeBulletinBoard (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeFadeIconGC (WmScreenData *pSD);
extern Boolean MakeIconBox (WmWorkspaceData *pWS, ClientData *pCD);
extern void MakeScrolledWindow (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeShell (WmWorkspaceData *pWS, IconBoxData *pIBD);
extern void MakeShrinkWrapIconsGC (WmScreenData *pSD, Pixmap bgPixmap);
extern void MapIconBoxes (WmWorkspaceData *pWS);
extern void PackIconBox (IconBoxData *pIBD, Boolean packVert, 
			 Boolean packHorz, int passedInWidth, 
			 int passedInHeight);
extern void RealignIconList (IconBoxData *pIBD, int newCols, int newRows);
extern void RealizeIconBox (WmWorkspaceData *pWS, IconBoxData *pIBD, 
			    ClientData *pCD);
extern void ReorderIconBoxIcons (ClientData *pCD, IconBoxData *pIBD, 
				 Widget theIcon, int newX, int newY);
extern void ResetArrowButtonIncrements (ClientData *pCD);
extern void ResetIconBoxMaxSize (ClientData *pCD, Widget bBoardWidget);
extern void SetGeometry (WmWorkspaceData *pWS, ClientData *pCD, 
			 IconBoxData *pIBD);
extern void SetGranularity (WmWorkspaceData *pWS, ClientData *pCD, 
			    IconBoxData *pIBD);
extern void SetIconBoxInfo (WmWorkspaceData *pWS, ClientData *pCD);
extern void SetNewBounds (IconBoxData *pIBD);
extern void ShowClientIconState (ClientData *pCD, int newState);
#ifdef WSM
extern void UnmapIconBoxes (WmWorkspaceData *pWS);
#endif /* WSM */
extern void UpdateIncrements (Widget sWidget, IconBoxData *pIBD, 
			      XConfigureEvent *event);
extern String WmXmStringToString (XmString xmString);

