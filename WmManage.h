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
/*   $XConsortium: WmManage.h /main/4 1995/11/01 11:44:22 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void AdoptInitialClients (WmScreenData *pSD);
extern void DeleteClientContext (ClientData *pCD);
extern void ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags);
extern void UnManageWindow (ClientData *pCD);
extern void WithdrawTransientChildren (ClientData *pCD);
extern void WithdrawWindow (ClientData *pCD);
extern void ResetWithdrawnFocii (ClientData *pCD);
extern void FreeClientFrame (ClientData *pCD);
extern void FreeIcon (ClientData *pCD);
extern void WithdrawDialog (Widget dialogboxW);
extern void ReManageDialog (WmScreenData *pSD, Widget dialogboxW);
#ifdef PANELIST
extern void RegisterEmbeddedClients (
	Widget wPanelist, 
	WmFpEmbeddedClientList pECD, 
	int count);
extern void RegisterPushRecallClients (
	Widget wPanelist, 
	WmFpPushRecallClientList pPRCD, 
	int count);
extern void UnParentControls(WmScreenData *pSD, Boolean unmap);
extern void RegisterIconBoxControl (Widget wPanelist);
extern Boolean ReparentEmbeddedClient (
	WmFpEmbeddedClientData *pECD,
	Widget newControl,
	Window newWin,
	int x, 
	int y,
	unsigned int width, 
	unsigned int height);
extern void ScanForEmbeddedClients (WmScreenData *pSD);
extern void ScanForPushRecallClients (WmScreenData *pSD);
#endif /* PANELIST */
