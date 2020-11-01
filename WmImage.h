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
 * Motif Release 1.2.2
*/ 
/*   $XConsortium: WmImage.h /main/4 1995/11/01 11:42:39 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern char  *BitmapPathName (char *string);
#ifdef WSM
extern int    GetBitmapIndex (WmScreenData *pSD, 
				char *name, 
				Boolean bReportError);
#else /* WSM */
extern int    GetBitmapIndex (WmScreenData *pSD, char *name);
#endif /* WSM */
extern Pixmap MakeCachedIconPixmap (ClientData *pCD, int bitmapIndex, Pixmap mask);
extern Pixmap MakeCachedLabelPixmap (WmScreenData *pSD, Widget menuW, int bitmapIndex);
extern Pixmap MakeClientIconPixmap (ClientData *pCD, Pixmap iconBitmap, Pixmap iconMask);
extern Pixmap MakeIconPixmap (ClientData *pCD, Pixmap bitmap, Pixmap mask, unsigned int width, unsigned int height, unsigned int depth);
extern Pixmap MakeNamedIconPixmap (ClientData *pCD, String iconName);
