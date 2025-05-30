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

void AdoptInitialClients (WmScreenData *pSD);
void DeleteClientContext (ClientData *pCD);
void ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags);
void UnManageWindow (ClientData *pCD);
void WithdrawTransientChildren (ClientData *pCD);
void WithdrawWindow (ClientData *pCD);
void ResetWithdrawnFocii (ClientData *pCD);
void FreeClientFrame (ClientData *pCD);
void FreeIcon (ClientData *pCD);
void WithdrawDialog (Widget dialogboxW);
void ReManageDialog (WmScreenData *pSD, Widget dialogboxW);
