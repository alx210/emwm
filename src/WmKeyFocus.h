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

void AutoResetKeyFocus (ClientData *pcdNoFocus, Time focusTime);
void ClearFocusIndication (ClientData *pCD, Boolean refresh);
Boolean CheckForKeyFocus (ClientListEntry *pNextEntry, 
				 unsigned long type, Boolean focusNext, 
				 Time focusTime);
void DoExplicitSelectGrab (Window window);
ClientData *FindLastTransientTreeFocus (ClientData *pCD, 
					       ClientData *pcdNoFocus);
ClientData *FindNextTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
ClientData *FindPrevTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
Boolean FocusNextTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
Boolean FocusNextWindow (unsigned long type, Time focusTime);
Boolean FocusPrevTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
Boolean FocusPrevWindow (unsigned long type, Time focusTime);
ClientData *GetClientUnderPointer (Boolean *pSameScreen);
void InitKeyboardFocus (void);
void RepairFocus (void);
void ResetExplicitSelectHandling (ClientData *pCD);
void SetFocusIndication (ClientData *pCD);
void SetKeyboardFocus (ClientData *pCD, long focusFlags);



