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
/*   $XConsortium: WmKeyFocus.h /main/4 1995/11/01 11:43:42 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


extern void AutoResetKeyFocus (ClientData *pcdNoFocus, Time focusTime);
extern void ClearFocusIndication (ClientData *pCD, Boolean refresh);
extern Boolean CheckForKeyFocus (ClientListEntry *pNextEntry, 
				 unsigned long type, Boolean focusNext, 
				 Time focusTime);
extern void DoExplicitSelectGrab (Window window);
extern ClientData *FindLastTransientTreeFocus (ClientData *pCD, 
					       ClientData *pcdNoFocus);
extern ClientData *FindNextTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
extern ClientData *FindPrevTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
extern Boolean FocusNextTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
extern Boolean FocusNextWindow (unsigned long type, Time focusTime);
extern Boolean FocusPrevTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
extern Boolean FocusPrevWindow (unsigned long type, Time focusTime);
extern ClientData *GetClientUnderPointer (Boolean *pSameScreen);
extern void InitKeyboardFocus (void);
extern void RepairFocus (void);
extern void ResetExplicitSelectHandling (ClientData *pCD);
extern void SetFocusIndication (ClientData *pCD);
extern void SetKeyboardFocus (ClientData *pCD, long focusFlags);



