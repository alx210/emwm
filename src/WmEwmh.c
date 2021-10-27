/*
 * Copyright (C) 2018-2021 alx@fastestcode.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "WmGlobal.h"
#include "WmError.h"
#include "WmImage.h"
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmXinerama.h"
#include "WmKeyFocus.h"
#include "WmIDecor.h"
#include "WmCEvent.h"
#include "WmWinConf.h"
#include "WmFunction.h"
#include "WmProtocol.h"
#include "WmEwmh.h"

static void* FetchWindowProperty(Window wnd, Atom prop,
	Atom req_type, unsigned long *size);
static Pixmap GetIconPixmap(const ClientData *pCD);
static void UpdateFrameExtents(ClientData *pCD);
static void ProcessMoveResize(ClientData *pCD,
	int x_root, int y_root, int dir, int button, int source);
static Boolean GetMultiscreenCoords(ClientData *pCD,
	int *xorg, int *yorg, int *width, int *height);

enum ewmh_atom {
	_NET_SUPPORTED, _NET_SUPPORTING_WM_CHECK, _NET_CLOSE_WINDOW,
	_NET_REQUEST_FRAME_EXTENTS, _NET_MOVERESIZE_WINDOW, 
	_NET_FRAME_EXTENTS, _NET_ACTIVE_WINDOW, _NET_CLIENT_LIST,
	_NET_WM_NAME, _NET_WM_ICON_NAME, _NET_WM_STATE,
	_NET_WM_STATE_FULLSCREEN, _NET_WM_STATE_MAXIMIZED_VERT,
	_NET_WM_STATE_MAXIMIZED_HORZ, _NET_WM_STATE_HIDDEN,
	_NET_WM_STATE_MODAL, _NET_WM_ICON, _NET_WM_MOVERESIZE,
	_NET_WM_ALLOWED_ACTIONS, _NET_WM_ACTION_MOVE, _NET_WM_ACTION_RESIZE,
	_NET_WM_ACTION_MINIMIZE, _NET_WM_ACTION_MAXIMIZE_HORZ,
	_NET_WM_ACTION_MAXIMIZE_VERT, _NET_WM_ACTION_FULLSCREEN,
	_NET_WM_ACTION_CLOSE, _NET_WM_FULLSCREEN_MONITORS,
	_NET_WM_WINDOW_TYPE, _NET_WM_WINDOW_TYPE_SPLASH,
	_NET_WM_WINDOW_TYPE_TOOLBAR, _NET_WM_WINDOW_TYPE_UTILITY,
	_NET_WM_WINDOW_TYPE_DIALOG,

	_NUM_EWMH_ATOMS
};

/* These must be in sync with enum ewmh_atom */
static char *ewmh_atom_names[_NUM_EWMH_ATOMS]={
	"_NET_SUPPORTED", "_NET_SUPPORTING_WM_CHECK", "_NET_CLOSE_WINDOW",
	"_NET_REQUEST_FRAME_EXTENTS", "_NET_MOVERESIZE_WINDOW", 
	"_NET_FRAME_EXTENTS", "_NET_ACTIVE_WINDOW", "_NET_CLIENT_LIST",
	"_NET_WM_NAME", "_NET_WM_ICON_NAME", "_NET_WM_STATE",
	"_NET_WM_STATE_FULLSCREEN", "_NET_WM_STATE_MAXIMIZED_VERT",
	"_NET_WM_STATE_MAXIMIZED_HORZ", "_NET_WM_STATE_HIDDEN",
	"_NET_WM_STATE_MODAL", "_NET_WM_ICON", "_NET_WM_MOVERESIZE",
	"_NET_WM_ALLOWED_ACTIONS", "_NET_WM_ACTION_MOVE", "_NET_WM_ACTION_RESIZE",
	"_NET_WM_ACTION_MINIMIZE", "_NET_WM_ACTION_MAXIMIZE_HORZ",
	"_NET_WM_ACTION_MAXIMIZE_VERT", "_NET_WM_ACTION_FULLSCREEN",
	"_NET_WM_ACTION_CLOSE", "_NET_WM_FULLSCREEN_MONITORS",
	"_NET_WM_WINDOW_TYPE", "_NET_WM_WINDOW_TYPE_SPLASH",
	"_NET_WM_WINDOW_TYPE_TOOLBAR", "_NET_WM_WINDOW_TYPE_UTILITY",
	"_NET_WM_WINDOW_TYPE_DIALOG"
};

/* Initialized in SetupEwhm() */
static Atom ewmh_atoms[_NUM_EWMH_ATOMS];
static Atom XA_UTF8_STRING;

/*
 * Initializes data and sets up root window properties for EWMH support.
 */
void SetupWmEwmh(void)
{
	int i;

	XInternAtoms(DISPLAY,ewmh_atom_names,_NUM_EWMH_ATOMS,False,ewmh_atoms);
	XA_UTF8_STRING = XInternAtom(DISPLAY,"UTF8_STRING",False);

	/* Add root properties indicating what EWMH protocols we support */
	for(i = 0; i < wmGD.numScreens; i++){
		Window check_wnd = XtWindow(wmGD.Screens[i].screenTopLevelW);
		
		XChangeProperty(DISPLAY,wmGD.Screens[i].rootWindow,
			ewmh_atoms[_NET_SUPPORTED],XA_ATOM,32,PropModeReplace,
			(unsigned char*)&ewmh_atoms[1],	_NUM_EWMH_ATOMS-1);
	
		/* Set up the device for _NET_SUPPORTING_WM_CHECK */
		XChangeProperty(DISPLAY,wmGD.Screens[i].rootWindow,
			ewmh_atoms[_NET_SUPPORTING_WM_CHECK],XA_WINDOW,32,PropModeReplace,
			(unsigned char*)&check_wnd,1);

		XChangeProperty(DISPLAY,check_wnd,
			ewmh_atoms[_NET_SUPPORTING_WM_CHECK],XA_WINDOW,32,PropModeReplace,
			(unsigned char*)&check_wnd,1);
			
		XChangeProperty(DISPLAY,check_wnd,ewmh_atoms[_NET_WM_NAME],
			XA_STRING,8,PropModeReplace,(unsigned char*)WM_RESOURCE_NAME,
			strlen(WM_RESOURCE_NAME));
	}
}

/*
 * Called by GetClientInfo to set up initial EWMH data for new clients
 */
void ProcessEwmh(ClientData *pCD)
{
	char *sz;
	Pixmap icon;
	unsigned long size = 0;
	
	sz = FetchWindowProperty(pCD->client,
		ewmh_atoms[_NET_WM_NAME],XA_UTF8_STRING,&size);
	if(size){
		if(pCD->ewmhClientTitle) XmStringFree(pCD->ewmhClientTitle);
		pCD->ewmhClientTitle = XmStringCreateLocalized(sz);
	}	
	if(sz) XFree(sz);

	sz = FetchWindowProperty(pCD->client,
		ewmh_atoms[_NET_WM_ICON_NAME],XA_UTF8_STRING,&size);
	if(size){
		if(pCD->ewmhIconTitle) XmStringFree(pCD->ewmhIconTitle);
		pCD->ewmhIconTitle = XmStringCreateLocalized(sz);
	}else if(pCD->ewmhClientTitle){
		pCD->ewmhIconTitle = XmStringCopy(pCD->ewmhClientTitle);
	}
	if(sz) XFree(sz);
	
	if(pCD->ewmhIconPixmap) XFreePixmap(DISPLAY,pCD->ewmhIconPixmap);
	pCD->ewmhIconPixmap = None;
	
	if(pCD->useClientIcon || !pCD->iconImage){
		/* client's icon preferred, or no user icon supplied */
		if((icon = GetIconPixmap(pCD))){
			pCD->ewmhIconPixmap = MakeClientIconPixmap(pCD,icon,None);
			XFreePixmap(DISPLAY,icon);
		}
	}

	UpdateFrameExtents(pCD);
}

/*
 * Maps EWMH window type to whatever it's closest to in MWM terms
 * and sets pCD's clientDecoration/Functions accordingly.
 */
void ProcessEwmhWindowType(ClientData *pCD)
{
	Atom *atoms;
	unsigned long count;

	atoms = FetchWindowProperty(
		pCD->client, ewmh_atoms[_NET_WM_WINDOW_TYPE], XA_ATOM, &count);
	if(atoms) {
		/* clients may list several types in order of preference,
		 * hence we can't go wrong by ignoring all but the first */
		if(atoms[0] == ewmh_atoms[_NET_WM_WINDOW_TYPE_TOOLBAR] ||
			atoms[0] == ewmh_atoms[_NET_WM_WINDOW_TYPE_UTILITY]) {
			pCD->clientDecoration = pCD->pSD->utilityDecoration;
			pCD->clientFunctions = pCD->pSD->utilityFunctions;
		} else if(atoms[0] == ewmh_atoms[_NET_WM_WINDOW_TYPE_DIALOG]) {
			pCD->clientDecoration = pCD->pSD->transientDecoration;
			pCD->clientFunctions = pCD->pSD->transientFunctions;
		} else if(atoms[0] == ewmh_atoms[_NET_WM_WINDOW_TYPE_SPLASH]) {
			pCD->clientDecoration = WM_DECOR_MINIMIZE | WM_DECOR_BORDER;
			pCD->clientFunctions = MWM_FUNC_MINIMIZE | MWM_FUNC_MOVE;
		}
		XtFree((char*)atoms);
	}
}

/*
 * Called by HandleEventsOnClientWindow to update EWMH data after
 * ECCC properties have been processed.
 */
void HandleEwmhCPropertyNotify(ClientData *pCD, XPropertyEvent *evt)
{
	unsigned long size = 0;
	
	if(evt->atom == ewmh_atoms[_NET_WM_NAME]) {
		char *sz = FetchWindowProperty(pCD->client,
			ewmh_atoms[_NET_WM_NAME],XA_UTF8_STRING,&size);
		if(size){
			if(pCD->ewmhClientTitle) XmStringFree(pCD->ewmhClientTitle);
			pCD->ewmhClientTitle = XmStringCreateLocalized(sz);
			DrawWindowTitle(pCD,True);
		}
		if(sz) XFree(sz);
	}
	else if(evt->atom == ewmh_atoms[_NET_WM_ICON_NAME]) {
		char *sz = FetchWindowProperty(pCD->client,
			ewmh_atoms[_NET_WM_ICON_NAME],XA_UTF8_STRING,&size);
		if(size){
			if(pCD->ewmhIconTitle) XmStringFree(pCD->ewmhIconTitle);
			pCD->ewmhIconTitle = XmStringCreateLocalized(sz);
			RedisplayIconTitle(pCD);
		}else if(pCD->ewmhClientTitle){
			pCD->ewmhIconTitle = XmStringCopy(pCD->ewmhClientTitle);
			RedisplayIconTitle(pCD);
		}
		if(sz) XFree(sz);
	}
	else if(evt->atom == ewmh_atoms[_NET_WM_ICON]) {
		Pixmap icon;
		if(pCD->ewmhIconPixmap) XFreePixmap(DISPLAY,pCD->ewmhIconPixmap);
		pCD->ewmhIconPixmap = None;
		
		if(pCD->useClientIcon || !pCD->iconImage){
			if((icon = GetIconPixmap(pCD))){
				pCD->ewmhIconPixmap = MakeClientIconPixmap(pCD,icon,None);
				XFreePixmap(DISPLAY,icon);
				XClearArea(DISPLAY,ICON_FRAME_WIN(pCD),IB_MARGIN_WIDTH,
					IB_MARGIN_HEIGHT,ICON_WIDTH(pCD),ICON_HEIGHT(pCD),True);
			}
		}
	}
	else if(evt->atom == ewmh_atoms[_NET_REQUEST_FRAME_EXTENTS]) {
		UpdateFrameExtents(pCD);
	}
}

/*
 * Called by HandleEventsOnClientWindow.
 * Processes EWMH related ClientMessage events.
 */
void HandleEwmhClientMessage(ClientData *pCD, XClientMessageEvent *evt)
{
	if(evt->message_type == ewmh_atoms[_NET_WM_STATE]){
		/* _NET_WM_STATE action constants. Keep in that order. */
		enum { REMOVE, ADD, TOGGLE } action = evt->data.l[0];
		
		if(evt->data.l[1] == ewmh_atoms[_NET_WM_STATE_FULLSCREEN]){
			Boolean set = False;
			
			switch(action) {
				case TOGGLE:
				set = pCD->fullScreen ? False : True;
				break;
				case ADD:
				set = True;
				break;
				case REMOVE:
				set = False;
				break;
			}
			ConfigureEwmhFullScreen(pCD,set);
		}
		else if((evt->data.l[1] == ewmh_atoms[_NET_WM_STATE_MAXIMIZED_VERT]) 
			&& (evt->data.l[2] == ewmh_atoms[_NET_WM_STATE_MAXIMIZED_HORZ])) {
			
			switch(action) {
				case ADD:
				if(pCD->clientState != MAXIMIZED_STATE)
					F_Maximize(NULL, pCD, (XEvent*)evt);
				break;
				case TOGGLE:
				if(pCD->clientState == MAXIMIZED_STATE)
					F_Restore(NULL, pCD, (XEvent*)evt);
				else
					F_Maximize(NULL, pCD, (XEvent*)evt);
				break;
				case REMOVE:
				if(pCD->clientState == MAXIMIZED_STATE)
					F_Restore(NULL, pCD, (XEvent*)evt);
				break;
			}
			UpdateEwmhClientState(pCD);	
		}
		else if(evt->data.l[1] == ewmh_atoms[_NET_WM_STATE_HIDDEN]) {
			switch(action) {
				case ADD:
				if(pCD->clientState != MINIMIZED_STATE)
					F_Minimize(NULL, pCD, (XEvent*)evt);
				break;
				case TOGGLE:
				if(pCD->clientState == MINIMIZED_STATE)
					F_Restore(NULL, pCD, (XEvent*)evt);
				else
					F_Minimize(NULL, pCD, (XEvent*)evt);
				break;
				case REMOVE:
				if(pCD->clientState == MINIMIZED_STATE)
					F_Restore(NULL, pCD, (XEvent*)evt);
				break;
			}
		UpdateEwmhClientState(pCD);
		}
	}
	else if(evt->message_type == ewmh_atoms[_NET_ACTIVE_WINDOW]) {
		F_Focus_Key(NULL,pCD,NULL);
	}
	else if(evt->message_type == ewmh_atoms[_NET_CLOSE_WINDOW]){
		F_Kill(NULL, pCD, NULL);
	}
	else if(evt->message_type == ewmh_atoms[_NET_MOVERESIZE_WINDOW]){
		int gravity = (evt->data.l[0] & 0x00FFl);
		int mask = (evt->data.l[0] & 0x0F00l) >> 8;
		int src = (evt->data.l[0] & 0xF000l) >> 12;

		if(!(mask & (CWX|CWY|CWWidth|CWHeight)) && src == 1){
			SendConfigureNotify(pCD);
			return;
		}

		int cx = (mask & CWX) ? evt->data.l[1] :
			(pCD->maxConfig ? pCD->maxX : pCD->clientX);

		int cy = (mask & CWY) ? evt->data.l[2] :
			(pCD->maxConfig ? pCD->maxY : pCD->clientY);

		unsigned int width = (mask & CWWidth) ? 
			evt->data.l[3] : pCD->clientWidth;
		unsigned int height = (mask & CWHeight) ?
			evt->data.l[4] : pCD->clientHeight;

		if(pCD->windowGravity != StaticGravity){
			if(!(mask & CWX)) cx -= pCD->clientOffset.x;
			if(!(mask & CWY)) cy -= pCD->clientOffset.y;
		}

		AdjustCoordinatesToGravity(pCD, gravity?gravity:pCD->windowGravity,
			&cx, &cy, &width, &height);
		ProcessNewConfiguration (pCD, cx, cy, width, height, True);
	}
	else if(evt->message_type == ewmh_atoms[_NET_WM_MOVERESIZE]) {
		ProcessMoveResize(pCD, evt->data.l[0], evt->data.l[1],
			evt->data.l[2], evt->data.l[3], evt->data.l[4]);
	}
	else if(evt->message_type == ewmh_atoms[_NET_WM_FULLSCREEN_MONITORS]) {
		pCD->fullScreenXineramaIndices[0] = evt->data.l[0];
		pCD->fullScreenXineramaIndices[1] = evt->data.l[1];
		pCD->fullScreenXineramaIndices[2] = evt->data.l[2];
		pCD->fullScreenXineramaIndices[3] = evt->data.l[3];
	}
}

/*
 * Initiates client requested window configuration
 */
static void ProcessMoveResize(ClientData *pCD,
	int x_root, int y_root, int dir, int button, int source)
{
	XEvent evt = {0};

	/* _NET_WM_MOVERESIZE direction constants. Keep in that order. */
	enum {
		SIZE_TOPLEFT, SIZE_TOP, SIZE_TOPRIGHT, SIZE_RIGHT,
		SIZE_BOTTOMRIGHT, SIZE_BOTTOM, SIZE_BOTTOMLEFT,
		SIZE_LEFT, MOVE, SIZE_KEYBOARD, MOVE_KEYBOARD, CANCEL
	};

	if(dir == CANCEL) {
		if(wmGD.configAction != NO_ACTION)
			CancelFrameConfig(pCD);
		return;
	}

	if(!((pCD->clientState == NORMAL_STATE) ||
		(pCD->clientState == MAXIMIZED_STATE))) return;
	
	if((dir == MOVE) ||	(dir == MOVE_KEYBOARD)) {
		if(!(pCD->clientFunctions & MWM_FUNC_MOVE)) return;
	} else {
		if(!(pCD->clientFunctions & MWM_FUNC_RESIZE)) return;
	}
	
	if(dir == SIZE_KEYBOARD) {
		evt.type = KeyPress;
		StartClientResize(pCD, &evt);
		HandleClientFrameResize(pCD, &evt);
	} else if(dir == MOVE_KEYBOARD) {
		evt.type = KeyPress;
		StartClientMove(pCD, &evt);
		HandleClientFrameMove(pCD, &evt);
	} else { 
		evt.type = ButtonPress;
		evt.xbutton.button = button;
		evt.xbutton.x_root = x_root;
		evt.xbutton.y_root = y_root;

		switch(dir) {
			case SIZE_TOPLEFT:
			wmGD.configPart = FRAME_RESIZE_NW;
			break;
			case SIZE_TOP:
			wmGD.configPart = FRAME_RESIZE_N;
			break;
			case SIZE_TOPRIGHT:
			wmGD.configPart = FRAME_RESIZE_NE;
			break;
			case SIZE_RIGHT:
			wmGD.configPart = FRAME_RESIZE_E;
			break;
			case SIZE_BOTTOMRIGHT:
			wmGD.configPart = FRAME_RESIZE_SE;
			break;
			case SIZE_BOTTOM:
			wmGD.configPart = FRAME_RESIZE_S;
			break;
			case SIZE_BOTTOMLEFT:
			wmGD.configPart = FRAME_RESIZE_SW;
			break;
			case SIZE_LEFT:
			wmGD.configPart = FRAME_RESIZE_W;
			break;
		};
		
		if(dir == MOVE) {
			StartClientMove(pCD, &evt);
			HandleClientFrameMove(pCD, &evt);
		} else {
			StartResizeConfig (pCD, &evt);
			HandleClientFrameResize(pCD, &evt);
		}
	}
}

/*
 * Returns rectangular coordinates from Xinerama screens obtained earlier
 * with the _NET_WM_FULLSCREEN_MONITORS client message event.
 * Returns True on success, False if no data available.
 */
static Boolean GetMultiscreenCoords(ClientData *pCD,
	int *xorg, int *yorg, int *width, int *height)
{
	XineramaScreenInfo xsi[4];
	int i;
	
	if(pCD->fullScreenXineramaIndices[0] == (-1)) return False;
	
	for(i = 0; i < 4; i++) {
		if(!GetXineramaScreenInfo(pCD->fullScreenXineramaIndices[i], &xsi[i]))
			return False;
	}

	*yorg = xsi[0].y_org;
	*height = xsi[1].y_org + xsi[1].height;
	*xorg = xsi[2].x_org;
	*width = xsi[3].x_org + xsi[3].width;

	return True;
}

/*
 * Removes decorations and resizes frame and client windows to the size of
 * display or xinerama screen they're on if 'set' is true, restores normal
 * configuration otherwise.
 */
void ConfigureEwmhFullScreen(ClientData *pCD, Boolean set)
{
	int xorg = 0;
	int yorg = 0;
	int swidth;
	int sheight;
	XineramaScreenInfo xsi;
	int i;

	if(set == pCD->fullScreen) return;
	
	if(!GetMultiscreenCoords(pCD, &xorg, &yorg, &swidth, &sheight)) {
		if(GetXineramaScreenFromLocation(pCD->clientX,pCD->clientY,&xsi)){
			xorg = xsi.x_org;
			yorg = xsi.y_org;
			swidth = xsi.width;
			sheight = xsi.height;
		}else{
			swidth = XDisplayWidth(DISPLAY,pCD->pSD->screen);
			sheight = XDisplayHeight(DISPLAY,pCD->pSD->screen);
		}
	}
	
	/*
	 * XXX
	 * This is sketchy. We just shift frame parts around, without maintaining
	 * a proper configuration state for full screen clients. It works because
	 * we don't need to process any size/position config requests on these.
	 * Ultimately MWMs decoration code needs to be tweaked to handle things
	 * dynamically.
	 * XXX
	 */
	if(set){
		pCD->normalClientFunctions = pCD->clientFunctions;
		pCD->fullScreenWidth = swidth;
		pCD->fullScreenHeight = sheight;
		pCD->fullScreenX = xorg;
		pCD->fullScreenY = yorg;

		pCD->clientFunctions = pCD->clientFunctions & 
			(~(MWM_FUNC_RESIZE|MWM_FUNC_MOVE|MWM_FUNC_MAXIMIZE));

		XUnmapWindow(DISPLAY,pCD->clientTitleWin);
		for(i = 0; i < STRETCH_COUNT; i++){
			XUnmapWindow(DISPLAY,pCD->clientStretchWin[i]);
		}
		XMoveResizeWindow(DISPLAY,pCD->clientFrameWin,xorg,yorg,swidth,sheight);
		XMoveResizeWindow(DISPLAY,pCD->clientBaseWin,0,0,swidth,sheight);
		XMoveResizeWindow(DISPLAY, pCD->client, 0, 0, swidth, sheight);

		pCD->fullScreen = True;
		UpdateEwmhClientState(pCD);
	}else{
		pCD->clientFunctions = pCD->normalClientFunctions;

		XMoveResizeWindow(DISPLAY, pCD->client, pCD->matteWidth,
			pCD->matteWidth, pCD->clientWidth, pCD->clientHeight);
		XMoveResizeWindow(DISPLAY,pCD->clientBaseWin,
			BaseWindowX(pCD),BaseWindowY(pCD),
			BaseWindowWidth(pCD),BaseWindowHeight(pCD));
		if (pCD->maxConfig){
			XResizeWindow (DISPLAY, pCD->client,
				pCD->maxWidth, pCD->maxHeight);
		}else{
			XResizeWindow (DISPLAY, pCD->client,
				pCD->clientWidth, pCD->clientHeight);
		}
		RegenerateClientFrame(pCD);
		XMapWindow(DISPLAY,pCD->clientTitleWin);
		for(i = 0; i < STRETCH_COUNT; i++){
			XMapWindow(DISPLAY,pCD->clientStretchWin[i]);
		}

		pCD->fullScreen = False;
		UpdateEwmhClientState(pCD);
	}
}

/*
 * Sets the _NET_WM_ACTIVE_WINDOW property on client's root
 */
void SetEwmhActiveWindow(ClientData *pCD)
{
	XChangeProperty(DISPLAY,ROOT_FOR_CLIENT(pCD),
		ewmh_atoms[_NET_ACTIVE_WINDOW],XA_WINDOW,32,PropModeReplace,
		(unsigned char*)&pCD->client,1);
}

/*
 * Sets the _NET_FRAME_EXTENTS property on client
 */
static void UpdateFrameExtents(ClientData *pCD)
{
	unsigned long data[4]; /* left, right, top, bottom */
	
	data[0] = data[1] = data[3] = (pCD->decor & MWM_DECOR_BORDER)?
			LowerBorderWidth(pCD):0;
	data[2] = ((pCD->decor & MWM_DECOR_TITLE)?TitleBarHeight(pCD):0) +
			((pCD->decor & MWM_DECOR_BORDER)?UpperBorderWidth(pCD):0);

	XChangeProperty(DISPLAY,pCD->client,ewmh_atoms[_NET_FRAME_EXTENTS],
		XA_CARDINAL,32,PropModeReplace,(unsigned char*)data,4);
}

/*
 * Sets the _NET_CLIENT_LIST property on pSD's root window.
 */
void UpdateEwmhClientList(WmScreenData *pSD)
{
	ClientListEntry *e = pSD->clientList;
	Window *wlist;
	unsigned int n = 0;
	
	if(!e){
		XChangeProperty(DISPLAY,pSD->rootWindow,
			ewmh_atoms[_NET_CLIENT_LIST],
			XA_WINDOW,32,PropModeReplace,NULL,0);
		return;
	}
	
	while(e) {
		if(e->type != MINIMIZED_STATE) n++;
		e = e->nextSibling;
	}
	
	wlist = malloc(sizeof(Window) * n);
	if(!wlist){
		Warning("Failed to allocate memory for _NET_CLIENT_LIST\n");
		return;
	}

	e = pSD->clientList;
	n = 0;
	while(e) {
		if(e->type != MINIMIZED_STATE){
			wlist[n] = e->pCD->client;
			n++;
		}
		e = e->nextSibling;
	}
	XChangeProperty(DISPLAY,pSD->rootWindow, ewmh_atoms[_NET_CLIENT_LIST],
		XA_WINDOW,32,PropModeReplace,(unsigned char*)wlist,n);

	free(wlist);
}

/*
 * Sets _NET_WM_STATE and _NET_WM_ALLOWED_ACTIONS according
 * to client's configuration.
 */
void UpdateEwmhClientState(ClientData *pCD)
{
	Atom state[4];
	Atom actions[10];
	unsigned int i = 0;

	if(pCD->fullScreen && (pCD->clientState != MINIMIZED_STATE)){
		state[i++] = ewmh_atoms[_NET_WM_STATE_FULLSCREEN];
	}
	else if(pCD->clientState == MAXIMIZED_STATE){
		state[i++] = ewmh_atoms[_NET_WM_STATE_MAXIMIZED_HORZ];
		state[i++] = ewmh_atoms[_NET_WM_STATE_MAXIMIZED_VERT];
	}
	else if(pCD->clientState == MINIMIZED_STATE){
		state[i++] = ewmh_atoms[_NET_WM_STATE_HIDDEN];
	}

	if(pCD->inputMode == MWM_INPUT_PRIMARY_APPLICATION_MODAL)
		state[i++] = ewmh_atoms[_NET_WM_STATE_MODAL];

	XChangeProperty(DISPLAY,pCD->client,ewmh_atoms[_NET_WM_STATE],
		XA_ATOM,32,PropModeReplace,(unsigned char*)state,i);

	i = 0;
	
	if(pCD->clientFunctions & MWM_FUNC_MOVE) {
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_MOVE];
	}
	
	if(pCD->clientFunctions & MWM_FUNC_RESIZE) {
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_RESIZE];
	}

	if(pCD->clientFunctions & MWM_FUNC_MAXIMIZE) {
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_MAXIMIZE_HORZ];
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_MAXIMIZE_VERT];
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_FULLSCREEN];
	}

	if(pCD->clientFunctions & MWM_FUNC_MINIMIZE) {
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_MINIMIZE];
	}

	if(pCD->clientFunctions & MWM_FUNC_CLOSE) {
		actions[i++] = ewmh_atoms[_NET_WM_ACTION_CLOSE];
	}

	XChangeProperty(DISPLAY,pCD->client,ewmh_atoms[_NET_WM_ALLOWED_ACTIONS],
		XA_ATOM,32,PropModeReplace,(unsigned char*)actions, i);
}

/*
 * Retrieves RGBA image data from the _NET_WM_ICON property. 
 * For True/DirectColor visuals, if an icon of appropriate size (larger or
 * equal to MWM icon size) is available it will be converted to a pixmap.
 * Returns a valid pixmap on success, or None.
 */
static Pixmap GetIconPixmap(const ClientData *pCD)
{
	unsigned long *prop_data;
	unsigned long *rgb_data;
	unsigned long prop_data_size;
	unsigned int rgb_width = 0;
	unsigned int rgb_height = 0;
	XImage *dest_img;
	void *dest_img_data;
	Visual *visual;
	int depth;
	XColor bg = {0};
	Pixmap pixmap = None;
	unsigned short red_shift = 0;
	unsigned short green_shift = 0;
	unsigned short blue_shift = 0;
	union {	unsigned long *i; unsigned char *b; } fptr;
	unsigned int x, y;
	float dx, dy;
	/* icon size we actually want, sans space for decorations
	 * MakeClientIconPixmap adds later on */
	unsigned int icon_width =
		PSD_FOR_CLIENT(pCD)->iconImageMaximum.width - 
		 (2 * ICON_INTERNAL_SHADOW_WIDTH +
		 ((wmGD.frameStyle == WmSLAB) ? 2 : 0));
    unsigned int icon_height =
		PSD_FOR_CLIENT(pCD)->iconImageMaximum.height -
		 ( 2 * ICON_INTERNAL_SHADOW_WIDTH +
		  ((wmGD.frameStyle == WmSLAB) ? 2 : 0));
	
	
	visual = XDefaultVisual(DISPLAY,SCREEN_FOR_CLIENT(pCD));
	depth = XDefaultDepth(DISPLAY,SCREEN_FOR_CLIENT(pCD));
	if(visual->class != TrueColor && visual->class != DirectColor)
		return None;
	
	prop_data = FetchWindowProperty(pCD->client,
		ewmh_atoms[_NET_WM_ICON],XA_CARDINAL,&prop_data_size);
	if(!prop_data) return None;
	if(!prop_data_size){
		XFree(prop_data);
		return None;
	}
	
	/* loop trough available images looking for usable size */
	fptr.i = prop_data;
	rgb_width = fptr.i[0];
	rgb_height = fptr.i[1];

	while(rgb_width < icon_width || rgb_height < icon_height) {
		if(!rgb_width || !rgb_height){
			XFree(prop_data);
			return None;
		}
		
		if((fptr.i + 2+rgb_width*rgb_height) >= 
			(prop_data + prop_data_size)) {
				XFree(prop_data);
				return None;
		}
		
		fptr.i += 2+rgb_width*rgb_height;
		rgb_width = fptr.i[0];
		rgb_height = fptr.i[1];
	};
	rgb_data = fptr.i + 2; /* fptr is at width/height */
	
	dest_img_data = calloc(icon_width * icon_height, XBitmapPad(DISPLAY) / 8);
	if(dest_img_data){
		dest_img = XCreateImage(DISPLAY,visual,depth,ZPixmap,0,dest_img_data,
			icon_width,icon_height, XBitmapPad(DISPLAY),0);
		if(!dest_img){
			free(dest_img_data);
			XFree(prop_data);
			return None;
		}
	} else {
		XFree(prop_data);
		return None;
	}
	
	/* color we want to aplha-blend the image with */
	bg.pixel = ICON_APPEARANCE(pCD).background;
	XQueryColor(DISPLAY,pCD->clientColormap,&bg);
	bg.red >>= 8; bg.green >>= 8; bg.blue >>= 8;
	
	/* figure out what server pixels are like */
	red_shift = visual->red_mask?ffs(visual->red_mask)-1:0;
	green_shift = visual->green_mask?ffs(visual->green_mask)-1:0;
	blue_shift = visual->blue_mask?ffs(visual->blue_mask)-1:0;
	
	/* scale and alpha-blend RGBA to XImage, then  make a pixmap out of it */
	dx = (float)rgb_width / dest_img->width;
	dy = (float)rgb_height / dest_img->height;

	for(y = 0; y < dest_img->height; y++){
		for(x = 0; x < dest_img->width; x++){
			unsigned int r[4],g[4],b[4],a[4];
			float na;
			unsigned long pixel;
			
			/* typically the image will be about 2x the size we want, hence
			 * doing some basic bi-linear interpolation should suffice */
			fptr.i = rgb_data + (unsigned int)
				(floorf(y * dy) * rgb_width + floorf(x * dx));
			a[0] = fptr.b[3];
			r[0] = fptr.b[2];
			g[0] = fptr.b[1];
			b[0] = fptr.b[0];
			fptr.i = rgb_data + (unsigned int)
				(floorf(y * dy) * rgb_width + ceilf(x * dx));
			a[1] = fptr.b[3];
			r[1] = fptr.b[2];
			g[1] = fptr.b[1];
			b[1] = fptr.b[0];
			fptr.i = rgb_data + (unsigned int)
				(ceilf(y * dy) * rgb_width + floorf(x * dx));
			a[2] = fptr.b[3];
			r[2] = fptr.b[2];
			g[2] = fptr.b[1];
			b[2] = fptr.b[0];
			fptr.i = rgb_data + (unsigned int)
				(ceilf(y * dy) * rgb_width + ceilf(x * dx));
			a[3] = fptr.b[3];
			r[3] = fptr.b[2];
			g[3] = fptr.b[1];
			b[3] = fptr.b[0];

			na = (float)((a[0] + a[1] + a[2] + a[3]) / 4) / 256;
			r[0] = (r[0] + r[1] + r[2] + r[3]) / 4;
			g[0] = (g[0] + g[1] + g[2] + g[3]) / 4;
			b[0] = (b[0] + b[1] + b[2] + b[3]) / 4;

			r[0] = ((float)r[0] * na + (float)bg.red * (1.0 - na));
			g[0] = ((float)g[0] * na + (float)bg.green * (1.0 - na));
			b[0] = ((float)b[0] * na + (float)bg.blue * (1.0 - na));

			pixel = (r[0] << red_shift) |
				(g[0] << green_shift) |(b[0] << blue_shift);
			
			XPutPixel(dest_img,x,y,pixel);
		}
	}

	pixmap = XCreatePixmap(DISPLAY,ROOT_FOR_CLIENT(pCD),
		icon_width,icon_height,depth);	
	if(pixmap){
		GC gc;
		XGCValues gcv;

		gcv.graphics_exposures = False;
		gc = XCreateGC(DISPLAY,ROOT_FOR_CLIENT(pCD),
			GCGraphicsExposures,&gcv);
		if(gc){
			XPutImage(DISPLAY,pixmap,gc,dest_img,
				0,0,0,0,icon_width,icon_height);
			XFreeGC(DISPLAY,gc);
		} else {
			XFreePixmap(DISPLAY,pixmap);
			pixmap = None;
		}
	}

	XDestroyImage(dest_img);
	XFree(prop_data);
	
	return pixmap;
}

/*
 * Retrieves property data of specified type from the given window.
 * Returns property data and sets size to the number of items of
 * requested type on success. Returns NULL if no data available or
 * is not of type specified.
 * The caller is responsible for freeing the memory.
 */
static void* FetchWindowProperty(Window wnd, Atom prop,
	Atom req_type, unsigned long *size)
{
	unsigned long ret_items, ret_bytes_left;
	char *result = NULL;
	Atom ret_type;
	int ret_fmt;
	
	if(XGetWindowProperty(DISPLAY,wnd,prop,0,BUFSIZ,
		False,req_type,&ret_type,&ret_fmt,&ret_items,
		&ret_bytes_left,(unsigned char**)&result) != Success ) return NULL;
	
	if(ret_type!=req_type){
		if(result) XFree(result);
		return NULL;
	}
	
	if(ret_bytes_left){
		XFree(result);
		if(XGetWindowProperty(DISPLAY,wnd,prop,0,BUFSIZ+ret_bytes_left+1,
			False,req_type,&ret_type,&ret_fmt,&ret_items,
			&ret_bytes_left,(unsigned char**)&result) != Success) return NULL;

	}

	if(size) *size = ret_items;
	return result;
}
