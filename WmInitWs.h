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
/*   $XConsortium: WmInitWs.h /main/5 1996/09/14 14:50:22 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void InitWmGlobal (int argc, char *argv [], char *environ []);
extern void InitWmScreen (WmScreenData *pSD, int sNum);
extern void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD);
extern void ProcessMotifWmInfo (Window rootWindowOfScreen);
extern void SetupWmWorkspaceWindows (void);
extern void MakeWorkspaceCursors (void);
extern void MakeWmFunctionResources (WmScreenData *pSD);
extern void MakeXorGC (WmScreenData *pSD);
extern void CopyArgv (int argc, char *argv []);
extern void InitScreenNames (void);
#ifndef NO_MESSAGE_CATALOG
extern void InitNlsStrings( void ) ;
#endif
#ifdef WSM
extern void InitWmDisplayEnv( void ) ;
#endif /* WSM */
