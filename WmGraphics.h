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
/*   $XConsortium: WmGraphics.h /main/4 1995/11/01 11:39:06 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


extern RList *AllocateRList (unsigned int amt);
extern void BevelDepressedRectangle (RList *prTop, RList *prBot, int x, 
				     int y, unsigned int width, 
				     unsigned int height, unsigned int top_wid,
 				     unsigned int right_wid, 
				     unsigned int bot_wid, 
				     unsigned int left_wid, 
				     unsigned int in_wid);
extern void BevelRectangle (RList *prTop, RList *prBot, int x, int y, 
			    unsigned int width, unsigned int height, 
			    unsigned int top_wid, unsigned int right_wid, 
			    unsigned int bot_wid, unsigned int left_wid);
extern void DrawStringInBox (Display *dpy, Window win, GC gc, 
			     XFontStruct *pfs, XRectangle *pbox, String str);
extern Boolean ExtendRList (RList *prl, unsigned int amt);
extern void FreeRList (RList *prl);
extern void StretcherCorner (RList *prTop, RList *prBot, int x, int y, 
			     int cnum, unsigned int swidth, 
			     unsigned int cwidth, unsigned int cheight);
extern void WmDrawString (Display *dpy, Drawable d, GC gc, int x, int y, 
			  char *string, unsigned int length);
#ifdef WSM
extern void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
			    XmString xmstring, GC gc, Position x, Position y, 
			    Dimension width, XRectangle *pbox,
			    Boolean bCenter);
#else /* WSM */
extern void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
			    XmString xmstring, GC gc, Position x, Position y, 
			    Dimension width, XRectangle *pbox);
#endif /* WSM */

extern GC WmGetGC (WmScreenData *pSD, unsigned long gc_mask, XGCValues *pGcv);


