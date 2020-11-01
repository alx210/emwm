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
/*   $XConsortium: WmProperty.h /main/4 1995/11/01 11:48:54 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern SizeHints * GetNormalHints (ClientData *pCD);
extern void ProcessWmProtocols (ClientData *pCD);
extern void ProcessMwmMessages (ClientData *pCD);
extern void SetMwmInfo (Window propWindow, long flags, Window wmWindow);
#ifdef WSM
void SetMwmSaveSessionInfo (Window wmWindow);
#endif /* WSM */
extern PropWMState * GetWMState (Window window);
extern void SetWMState (Window window, int state, Window icon);
extern PropMwmHints * GetMwmHints (ClientData *pCD);
extern PropMwmInfo * GetMwmInfo (Window rootWindowOfScreen);
extern void ProcessWmColormapWindows (ClientData *pCD);
extern Colormap FindColormap (ClientData *pCD, Window window);
extern MenuItem * GetMwmMenuItems (ClientData *pCD);
#ifdef WSM
extern void GetInitialPropertyList (ClientData *pCD);
extern Status GetWorkspaceHints (Display *display, Window window, Atom **ppWsAtoms, unsigned int *pCount, Boolean *pbAll);
#ifdef HP_VUE
extern void SetWorkspaceInfo (Window propWindow, WorkspaceInfo *pWsInfo, unsigned long cInfo);
#endif /* HP_VUE */
extern void SetWorkspacePresence (Window propWindow, Atom *pWsPresence, unsigned long cPresence);
extern Boolean HasProperty(ClientData *pCD, Atom property);
extern void DiscardInitialPropertyList (ClientData *pCD);
extern void GetInitialPropertyList (ClientData *pCD);
extern void SetWorkspaceListProperty (WmScreenData *pSD);
extern void SetCurrentWorkspaceProperty (WmScreenData *pSD);
extern void SetWorkspaceInfoProperty (WmWorkspaceData *pWS);
extern void DeleteWorkspaceInfoProperty (WmWorkspaceData *pWS);
extern char *WorkspacePropertyName (WmWorkspaceData *pWS);
#endif /* WSM */
