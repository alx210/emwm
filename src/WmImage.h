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

Pixmap GetNamedPixmap (Screen *scr, String iconName,
	Pixel fg, Pixel bg, int depth);
char* BitmapPathName (char *string);
int GetBitmapIndex (WmScreenData *pSD,
	char *name, Boolean bReportError);
Pixmap MakeCachedIconPixmap (ClientData *pCD,
	int bitmapIndex, Pixmap mask);
Pixmap MakeCachedLabelPixmap (WmScreenData *pSD,
	Widget menuW, int bitmapIndex);
Pixmap MakeClientIconPixmap (ClientData *pCD,
	Pixmap iconBitmap, Pixmap iconMask);
Pixmap MakeIconPixmap (ClientData *pCD, Pixmap bitmap, Pixmap mask,
	unsigned int width, unsigned int height, unsigned int depth);
Pixmap MakeNamedIconPixmap (ClientData *pCD, String iconName);
Boolean GetPixmapInfo(Pixmap pixmap,
	unsigned int *width, unsigned int *height, unsigned int *depth);
