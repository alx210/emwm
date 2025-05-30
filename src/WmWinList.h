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

void AddClientToList (WmWorkspaceData *pWS, ClientData *pCD, 
			     Boolean onTop);
void AddEntryToList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			    Boolean onTop, ClientListEntry *pStackEntry);
void AddTransient (WmWorkspaceData *pWS, ClientData *pCD);
Boolean BumpPrimaryToBottom (ClientData *pcdLeader);
Boolean BumpPrimaryToTop (ClientData *pcdLeader);
Boolean CheckIfClientObscuredByAny (ClientData *pcd);
Boolean CheckIfClientObscuring (ClientData *pcdTop, ClientData *pcd);
Boolean CheckIfClientObscuringAny (ClientData *pcd);
Boolean CheckIfObscuring (ClientData *pcdA, ClientData *pcdB);
int CountTransientChildren (ClientData *pcd);
void DeleteClientFromList (WmWorkspaceData *pWS, ClientData *pCD);
void DeleteEntryFromList (WmWorkspaceData *pWS, 
				 ClientListEntry *pListEntry);
void DeleteFullAppModalChildren (ClientData *pcdLeader, 
					ClientData *pCD);
void DeleteTransient (ClientData *pCD);
ClientListEntry *FindClientNameMatch (ClientListEntry *pEntry, 
					     Boolean toNext, 
					     String clientName,
					     unsigned long types);
ClientData *FindSubLeaderToTop (ClientData *pcd);
ClientData *FindTransientFocus (ClientData *pcd);
ClientData *FindTransientOnTop (ClientData *pcd);
ClientData *FindTransientTreeLeader (ClientData *pcd);
void FixupFullAppModalCounts (ClientData *pcdLeader, 
				     ClientData *pcdDelete);
Boolean LeaderOnTop (ClientData *pcdLeader);
Window LowestWindowInTransientFamily (ClientData *pcdLeader);
void MakeTransientFamilyStackingList (Window *windows, 
					    ClientData *pcdLeader);
Window *MakeTransientWindowList (Window *windows, ClientData *pcd);
void MarkModalSubtree (ClientData *pcdTree, ClientData *pcdAvoid);
void MarkModalTransient (ClientData *pcdLeader, ClientData *pCD);
void MoveEntryInList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			     Boolean onTop, ClientListEntry *pStackEntry);
Boolean NormalizeTransientTreeStacking (ClientData *pcdLeader);
Boolean PutTransientBelowSiblings (ClientData *pcd);
Boolean PutTransientOnTop (ClientData *pcd);
void RestackTransients (ClientData *pcd);
void RestackTransientsAtWindow (ClientData *pcd);
void SetupSystemModalState (ClientData *pCD);
void StackTransientWindow (ClientData *pcd);
void StackWindow (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			 Boolean onTop, ClientListEntry *pStackEntry);
void UnMarkModalTransient (ClientData *pcdModee, int modalCount, 
				  ClientData *pcdModal);
void UndoSystemModalState (void);
