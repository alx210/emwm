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
/*   $XConsortium: WmColormap.h /main/4 1995/11/01 11:35:11 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void InitWorkspaceColormap (WmScreenData *pSD);
extern void InitColormapFocus (WmScreenData *pSD);
#ifndef OLD_COLORMAP
extern void ForceColormapFocus (WmScreenData *pSD, ClientData *pCD);
#endif
extern void SetColormapFocus (WmScreenData *pSD, ClientData *pCD);
extern void WmInstallColormap (WmScreenData *pSD, Colormap colormap);
extern void ResetColormapData (ClientData *pCD, Window *pWindows, int count);
#ifndef IBM_169380
extern void AddColormapWindowReference (ClientData *pCD, Window window);
extern void RemoveColormapWindowReference (ClientData *pCD, Window window);
#endif
extern void ProcessColormapList (WmScreenData *pSD, ClientData *pCD);
