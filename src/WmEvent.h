/* 
 * Motif
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 * Copyright (c) 2018-2026, alx@fastestcode.org
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

Boolean CheckForButtonAction (XButtonEvent *buttonEvent, 
				     Context context, Context subContext, 
				     ClientData *pCD);
Time GetTimestamp (void);
Boolean HandleKeyPress (XKeyEvent *keyEvent, KeySpec *keySpecs, 
			       Boolean checkContext, Context context, 
			       Boolean onlyFirst, ClientData *pCD);
void HandleWsButtonPress (XButtonEvent *buttonEvent);
void HandleWsButtonRelease (XButtonEvent *buttonEvent);
void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent);
void HandleWsEnterNotify (XEnterWindowEvent *enterEvent);
void HandleWsFocusIn (XFocusInEvent *focusEvent);
Boolean HandleWsKeyPress (XKeyEvent *keyEvent);
void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent);
void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, 
				  Context *pContext, int *pPartContext);
void InitEventHandling (void);
void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, 
				Context context, Context subContext);
void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, 
				  Context context, Context subContext);
void PullExposureEvents (void);
int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, 
			     int keyboardMode, long context);
Boolean WmDispatchMenuEvent (XButtonEvent *event);
Boolean WmDispatchWsEvent (XEvent *event);
void WmGrabButton (Display *display, unsigned int button, 
		unsigned int modifiers, Window grab_window, 
		unsigned int event_mask, Bool owner_events, int pointer_mode, 
		int keyboard_mode, Window confine_to, Cursor cursor);
void WmGrabKey (Display *display, int keycode, unsigned int modifiers, 
		Window grab_window, Bool owner_events, int pointer_mode, 
		int keyboard_mode);
void WmUngrabButton (Display *display, unsigned int button,
		unsigned int modifiers, Window grab_window);
Boolean ReplayedButtonEvent (XButtonEvent *pevB1, XButtonEvent *pevB2);
void HandleWmClientMessage (XClientMessageEvent *clientEvent);
void HandleMwmRequest (WmScreenData *pSD, XEvent *pev);
