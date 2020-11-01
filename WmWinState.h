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
 * Motif Release 1.2.1
*/ 
/*   $XConsortium: WmWinState.h /main/4 1995/11/01 11:55:38 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void SetClientState (ClientData *pCD, int newState, Time setTime);
extern void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);
extern void ConfigureNewState (ClientData *pcd);
extern void SetClientWMState (ClientData *pCD, int wmState, int mwmState);
extern void MapClientWindows (ClientData *pCD);
extern void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD);
#ifdef WSM
extern void ShowAllIconsForMinimizedClient (ClientData *pCD);
#endif /* WSM */
#ifdef PANELIST
extern void SlideSubpanelBackIn (ClientData *pCD, Widget wSubpanel);
#endif /* PANELIST */
