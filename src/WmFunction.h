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

Boolean F_Beep (String args, ClientData *pCD, XEvent *event);
Boolean F_Lower (String args, ClientData *pCD, XEvent *event);
void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event);
Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event);
Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event);
Boolean F_Exec (String args, ClientData *pCD, XEvent *event);
Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event);
void Do_Quit_Mwm (Boolean diedOnRestart);
Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event);
void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags);
Boolean F_Goto_Workspace (String args, ClientData *pCD, XEvent *event);
Boolean F_Help (String args, ClientData *pCD, XEvent *event);
Boolean F_Help_Mode (String args, ClientData *pCD, XEvent *event);
Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event);
Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event);
Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event);
Boolean F_Pass_Key (String args, ClientData *pCD, XEvent *event);
Boolean F_Marquee_Selection (String args, ClientData *pCD, XEvent *event);
Boolean F_Maximize (String args, ClientData *pCD, XEvent *event);
Boolean F_Menu (String args, ClientData *pCD, XEvent *event);
Boolean F_Minimize (String args, ClientData *pCD, XEvent *event);
Boolean F_Move (String args, ClientData *pCD, XEvent *event);
Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event);
Boolean F_Nop (String args, ClientData *pCD, XEvent *event);
Boolean F_Normalize (String args, ClientData *pCD, XEvent *event);
Boolean F_Normalize_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event);
Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event);
Boolean F_Kill (String args, ClientData *pCD, XEvent *event);
Boolean F_Refresh (String args, ClientData *pCD, XEvent *event);
Boolean F_Resize (String args, ClientData *pCD, XEvent *event);
Boolean F_Restart (String args, ClientData *pCD, XEvent *event);
Boolean F_Restore (String args, ClientData *pCD, XEvent *event);
Boolean F_Restore_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
void Do_Restart (Boolean dummy);
void RestartWm (long startupFlags);
void DeFrameClient (ClientData *pCD);
Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event);
Boolean F_Set_Context (String args, ClientData *pCD, XEvent *event);
Boolean F_Separator (String args, ClientData *pCD, XEvent *event);
Boolean F_Raise (String args, ClientData *pCD, XEvent *event);
void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event);
Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event);
Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event);
void Do_Set_Behavior (Boolean dummy);
Boolean F_Title (String args, ClientData *pCD, XEvent *event);
Boolean F_Screen (String args, ClientData *pCD, XEvent *event);
Boolean F_Next_Workspace (String args, ClientData *pCD, XEvent *event);
Boolean F_Prev_Workspace (String args, ClientData *pCD, XEvent *event);
Boolean F_Workspace_Presence (String args, ClientData *pCD, 
				     XEvent *event);
Time GetFunctionTimestamp (XButtonEvent *pEvent);
void ReBorderClient (ClientData *pCD, Boolean reMapClient);
void ClearDirtyStackEntry (ClientData *pCD);	/* Fix for 5325 */
#if defined(DEBUG)
Boolean F_ZZ_Debug (String, ClientData *, XEvent *);
#endif /* DEBUG */
