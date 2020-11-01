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
/*   $XConsortium: WmEvent.h /main/5 1996/06/11 15:59:25 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef WSM
/* Shared Variables */
extern int smAckState;

#endif /* WSM */

extern Boolean CheckForButtonAction (XButtonEvent *buttonEvent, 
				     Context context, Context subContext, 
				     ClientData *pCD);
extern Time GetTimestamp (void);
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
extern Time LastTime (void);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
extern Boolean HandleKeyPress (XKeyEvent *keyEvent, KeySpec *keySpecs, 
			       Boolean checkContext, Context context, 
			       Boolean onlyFirst, ClientData *pCD);
extern void HandleWsButtonPress (XButtonEvent *buttonEvent);
extern void HandleWsButtonRelease (XButtonEvent *buttonEvent);
extern void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent);
extern void HandleWsEnterNotify (XEnterWindowEvent *enterEvent);
extern void HandleWsFocusIn (XFocusInEvent *focusEvent);
extern Boolean HandleWsKeyPress (XKeyEvent *keyEvent);
extern void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent);
extern void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, 
				  Context *pContext, int *pPartContext);
extern void InitEventHandling (void);
extern void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, 
				Context context, Context subContext);
extern void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, 
				  Context context, Context subContext);
extern void PullExposureEvents (void);
extern int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, 
			     int keyboardMode, long context);
extern Boolean WmDispatchMenuEvent (XButtonEvent *event);
extern Boolean WmDispatchWsEvent (XEvent *event);
extern void WmGrabButton (Display *display, unsigned int button, 
		unsigned int modifiers, Window grab_window, 
		unsigned int event_mask, Bool owner_events, int pointer_mode, 
		int keyboard_mode, Window confine_to, Cursor cursor);
extern void WmGrabKey (Display *display, int keycode, unsigned int modifiers, 
		Window grab_window, Bool owner_events, int pointer_mode, 
		int keyboard_mode);
extern void WmUngrabButton (Display *display, unsigned int button,
		unsigned int modifiers, Window grab_window);
#ifdef WSM
extern void HandleDtWmClientMessage (XClientMessageEvent *clientEvent);
extern void HandleDtWmRequest (WmScreenData *pSD, XEvent *pev);
extern Boolean ReplayedButtonEvent (XButtonEvent *pevB1, XButtonEvent *pevB2);
#endif /* WSM */
