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
/*   $XConsortium: WmWinList.h /main/5 1996/05/17 12:54:05 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


extern void AddClientToList (WmWorkspaceData *pWS, ClientData *pCD, 
			     Boolean onTop);
extern void AddEntryToList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			    Boolean onTop, ClientListEntry *pStackEntry);
extern void AddTransient (WmWorkspaceData *pWS, ClientData *pCD);
#ifdef WSM
extern Boolean BumpPrimaryToBottom (ClientData *pcdLeader);
extern Boolean BumpPrimaryToTop (ClientData *pcdLeader);
#endif /* WSM */
extern Boolean CheckIfClientObscuredByAny (ClientData *pcd);
extern Boolean CheckIfClientObscuring (ClientData *pcdTop, ClientData *pcd);
extern Boolean CheckIfClientObscuringAny (ClientData *pcd);
extern Boolean CheckIfObscuring (ClientData *pcdA, ClientData *pcdB);
extern int CountTransientChildren (ClientData *pcd);
extern void DeleteClientFromList (WmWorkspaceData *pWS, ClientData *pCD);
extern void DeleteEntryFromList (WmWorkspaceData *pWS, 
				 ClientListEntry *pListEntry);
extern void DeleteFullAppModalChildren (ClientData *pcdLeader, 
					ClientData *pCD);
extern void DeleteTransient (ClientData *pCD);
extern ClientListEntry *FindClientNameMatch (ClientListEntry *pEntry, 
					     Boolean toNext, 
					     String clientName,
					     unsigned long types);
#ifdef WSM
extern ClientData *FindSubLeaderToTop (ClientData *pcd);
#endif /* WSM */
extern ClientData *FindTransientFocus (ClientData *pcd);
extern ClientData *FindTransientOnTop (ClientData *pcd);
extern ClientData *FindTransientTreeLeader (ClientData *pcd);
extern void FixupFullAppModalCounts (ClientData *pcdLeader, 
				     ClientData *pcdDelete);
#ifdef WSM
extern Boolean LeaderOnTop (ClientData *pcdLeader);
extern Window LowestWindowInTransientFamily (ClientData *pcdLeader);
extern void MakeTransientFamilyStackingList (Window *windows, 
					    ClientData *pcdLeader);
#endif /* WSM */
extern Window *MakeTransientWindowList (Window *windows, ClientData *pcd);
extern void MarkModalSubtree (ClientData *pcdTree, ClientData *pcdAvoid);
extern void MarkModalTransient (ClientData *pcdLeader, ClientData *pCD);
extern void MoveEntryInList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			     Boolean onTop, ClientListEntry *pStackEntry);
#ifdef WSM
extern Boolean NormalizeTransientTreeStacking (ClientData *pcdLeader);
#endif /* WSM */
extern Boolean PutTransientBelowSiblings (ClientData *pcd);
extern Boolean PutTransientOnTop (ClientData *pcd);
extern void RestackTransients (ClientData *pcd);
extern void RestackTransientsAtWindow (ClientData *pcd);
extern void SetupSystemModalState (ClientData *pCD);
extern void StackTransientWindow (ClientData *pcd);
extern void StackWindow (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			 Boolean onTop, ClientListEntry *pStackEntry);
extern void UnMarkModalTransient (ClientData *pcdModee, int modalCount, 
				  ClientData *pcdModal);
extern void UndoSystemModalState (void);

