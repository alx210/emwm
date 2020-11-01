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
/*   $XConsortium: WmFunction.h /main/5 1996/06/11 15:59:42 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


#ifdef WSM
extern Boolean F_Action (String actionName, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Beep (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Lower (String args, ClientData *pCD, XEvent *event);
extern void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Exec (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event);
extern void Do_Quit_Mwm (Boolean diedOnRestart);
extern Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event);
extern void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags);
#ifdef WSM
extern Boolean F_Goto_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Help (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Help_Mode (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Pass_Key (String args, ClientData *pCD, XEvent *event);
#ifdef WSM
extern Boolean F_Marquee_Selection (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Maximize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Menu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Minimize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Move (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Nop (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event);
#ifdef PANELIST
extern Boolean F_Post_FpMenu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Push_Recall (String args, ClientData *pCD, XEvent *event);
#endif /* PANELIST */
extern Boolean F_Kill (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Resize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restart (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern void Do_Restart (Boolean dummy);
extern void RestartWm (long startupFlags);
extern void DeFrameClient (ClientData *pCD);
#ifdef WSM
extern Boolean F_Send (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Separator (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Raise (String args, ClientData *pCD, XEvent *event);
extern void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event);
#ifdef WSM
extern Boolean F_Set_Context (String args, ClientData *pCD, XEvent *event);
#endif
extern void Do_Set_Behavior (Boolean dummy);
extern Boolean F_Title (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Screen (String args, ClientData *pCD, XEvent *event);
#if defined(PANELIST)
extern Boolean F_Toggle_Front_Panel (String args, ClientData *pCD, 
				     XEvent *event);
extern Boolean F_Version (String args, ClientData *pCD, XEvent *event);
#endif /* PANELIST */
#ifdef WSM
extern Boolean F_Next_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Workspace_Presence (String args, ClientData *pCD, 
				     XEvent *event);
#if defined(DEBUG)
extern Boolean F_ZZ_Debug (String, ClientData *, XEvent *);
#endif /* DEBUG */
#endif /* WSM */
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
extern Boolean F_InvokeCommand (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Post_RMenu (String args, ClientData *pCD, XEvent *event);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
extern Time GetFunctionTimestamp (XButtonEvent *pEvent);
extern void ReBorderClient (ClientData *pCD, Boolean reMapClient);
extern void ClearDirtyStackEntry (ClientData *pCD);	/* Fix for 5325 */

#ifdef PANELIST

typedef struct _WmPushRecallArg {
    int ixReg;
    WmFunction wmFunc;
    void *pArgs;
} WmPushRecallArg;


#endif /* PANELIST */
