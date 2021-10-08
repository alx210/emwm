/*
 * Copyright (C) 2018 alx@fastestcode.org
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

#include "WmGlobal.h"
#include "WmError.h"
#include <X11/extensions/Xinerama.h>

static Boolean is_active=False;
static Boolean is_present=False;
static XineramaScreenInfo *g_xsi=NULL;
static int g_nxsi=0;

/*
 * Checks for xinerama availability and fetches screen info.
 */
void SetupXinerama(void)
{
	int major_opcode, first_event, first_error;
	if((is_present = XQueryExtension(DISPLAY,"XINERAMA",
		&major_opcode,&first_event,&first_error))) {

		if(!XineramaIsActive(DISPLAY) ||
			(g_xsi=XineramaQueryScreens(DISPLAY,&g_nxsi))==NULL){
			is_active=False;
			return; 
		}
		is_active=True;
	}
}

/*
 * Called on xrandr screen change events
 */
void UpdateXineramaInfo(void)
{
	if(!is_present) return;
	
	if(g_xsi) XFree(g_xsi);
	g_xsi = NULL;
	
	if(!XineramaIsActive(DISPLAY) ||
		(g_xsi=XineramaQueryScreens(DISPLAY,&g_nxsi))==NULL){
		is_active=False;
		return; 
	}
	is_active = True;
}

/*
 * Retrieves Xinerama screen info from given coordinates.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetXineramaScreenFromLocation(int x, int y, XineramaScreenInfo *xsi)
{
	int i;

	if(!is_active) return False;
	
	if(x < 0) x=0;
	if(y < 0) y=0;

	for(i=0; i < g_nxsi; i++){
		if((x >= g_xsi[i].x_org && x <= (g_xsi[i].x_org+g_xsi[i].width)) &&
			(y >= g_xsi[i].y_org &&	y <= (g_xsi[i].y_org+g_xsi[i].height)))
			break;
	}

	if(i < g_nxsi){
		memcpy(xsi,&g_xsi[i],sizeof(XineramaScreenInfo));
		return True;
	}
	return False;
}

/*
 * Retrieves info for xinerama screen that contains the mouse pointer.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetXineramaScreenFromPointer(XineramaScreenInfo *xsi)
{
	Window wroot, wchild;
	int root_x, root_y, child_x, child_y;
	unsigned int mask;
	
	if(!XQueryPointer(DISPLAY,ACTIVE_ROOT,&wroot,&wchild,
		&root_x,&root_y,&child_x,&child_y,&mask)) return False;

	return GetXineramaScreenFromLocation(root_x,root_y,xsi);
}

/*
 * Retrieves info for xinerama screen that contains a client window
 * with keyboard focus or the mouse pointer (in that order).
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetActiveXineramaScreen(XineramaScreenInfo *xsi)
{
	if(wmGD.keyboardFocus){
		return GetXineramaScreenFromLocation(
			wmGD.keyboardFocus->clientX,
			wmGD.keyboardFocus->clientY,xsi);
	}else{
		return GetXineramaScreenFromPointer(xsi);
	}
}

/*
 * Retrieves user's preferred xinerama screen.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetPreferredXineramaScreen(XineramaScreenInfo *xsi)
{
	if(!is_active) return False;

	if(wmGD.primaryXineramaScreen >= g_nxsi ||
		wmGD.primaryXineramaScreen < 0){
		Warning("PrimaryXineramaScreen out of range\n");
		memcpy(xsi, &g_xsi[0], sizeof(XineramaScreenInfo));
		return True;
	}
	memcpy(xsi,&g_xsi[wmGD.primaryXineramaScreen],
		sizeof(XineramaScreenInfo));
	return True;
}

/*
 * Retrieves xinerama screen info.
 */
Bool GetXineramaScreenInfo(int index, XineramaScreenInfo *xsi)
{
	if(!is_active || index >= g_nxsi) return False;
	memcpy(xsi, &g_xsi[index], sizeof(XineramaScreenInfo));
	return True;
}
