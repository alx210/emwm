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

#ifndef _WM_XINERAMA_H
#define _WM_XINERAMA_H

#include "WmGlobal.h"
#include <X11/extensions/Xinerama.h>

/*
 * Checks for xinerama availability and fetches screen info.
 */
void SetupXinerama(void);

/*
 * Called on xrandr screen change events
 */
void UpdateXineramaInfo(void);

/*
 * Retrieves xinerama screen info from given coordinates.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetXineramaScreenFromLocation(int x, int y, XineramaScreenInfo *xsi);

/*
 * Retrieves info for xinerama screen that contains the mouse pointer.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetXineramaScreenFromPointer(XineramaScreenInfo *xsi);

/*
 * Retrieves info for xinerama screen that contains a client window
 * with keyboard focus or the mouse pointer (in that order).
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetActiveXineramaScreen(XineramaScreenInfo *xsi);

/*
 * Retrieves user's preferred xinerama screen.
 * Returns True on success, False if xinerama is inactive or on error.
 */
Bool GetPreferredXineramaScreen(XineramaScreenInfo *xsi);

/* Convenience macro for ClientData */
#define GetXineramaScreenOfClient(cd,xsi)\
	GetXineramaScreenFromPoint(cd->clientX,cd->clientY,xsi)

/*
 * Retrieves xinerama screen info. Returns True on success.
 */
Bool GetXineramaScreenInfo(int index, XineramaScreenInfo *xsi);

#endif /* _WM_XINERAMA_H */
