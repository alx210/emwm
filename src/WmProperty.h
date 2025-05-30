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

SizeHints * GetNormalHints (ClientData *pCD);
void ProcessWmProtocols (ClientData *pCD);
void ProcessMwmMessages (ClientData *pCD);
void SetMwmInfo (Window propWindow, long flags, Window wmWindow);
void SetMwmSaveSessionInfo (Window wmWindow);
PropWMState * GetWMState (Window window);
void SetWMState (Window window, int state, Window icon);
PropMwmHints * GetMwmHints (ClientData *pCD);
PropMwmInfo * GetMwmInfo (Window rootWindowOfScreen);
void ProcessWmColormapWindows (ClientData *pCD);
Colormap FindColormap (ClientData *pCD, Window window);
MenuItem * GetMwmMenuItems (ClientData *pCD);
void GetInitialPropertyList (ClientData *pCD);
Status GetWorkspaceHints (Display *display,
	Window window, Atom **ppWsAtoms, unsigned int *pCount, Boolean *pbAll);
#ifdef HP_VUE
void SetWorkspaceInfo (Window propWindow,
	WorkspaceInfo *pWsInfo, unsigned long cInfo);
#endif /* HP_VUE */
void SetWorkspacePresence (Window propWindow,
	Atom *pWsPresence, unsigned long cPresence);
Boolean HasProperty(ClientData *pCD, Atom property);
void DiscardInitialPropertyList (ClientData *pCD);
void GetInitialPropertyList (ClientData *pCD);
void SetWorkspaceListProperty (WmScreenData *pSD);
void SetCurrentWorkspaceProperty (WmScreenData *pSD);
void SetWorkspaceInfoProperty (WmWorkspaceData *pWS);
void DeleteWorkspaceInfoProperty (WmWorkspaceData *pWS);
char *WorkspacePropertyName (WmWorkspaceData *pWS);
void SetWorkspaceInfoProperty(WmWorkspaceData *pWS);
void DeleteWorkspaceInfoProperty(WmWorkspaceData *pWS);
