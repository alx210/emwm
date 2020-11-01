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
/*   $XConsortium: WmWinInfo.h /main/5 1995/12/27 17:24:50 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */


extern void FindClientPlacement (ClientData *pCD);
extern void FixWindowConfiguration (ClientData *pCD, unsigned int *pWidth, 
				    unsigned int *pHeight, 
				    unsigned int widthInc, 
				    unsigned int heightInc);
extern void FixWindowSize (ClientData *pCD, unsigned int *pWidth, 
			   unsigned int *pHeight, unsigned int widthInc, 
			   unsigned int heightInc);
extern ClientData *GetClientInfo (WmScreenData *pSD, Window clientWindow, 
				  long manageFlags);
extern ClientData *GetWmClientInfo (WmWorkspaceData *pWS, ClientData *pCD, 
				    long manageFlags);
extern void CalculateGravityOffset (ClientData *pCD, int *xoff, int *yoff);
extern Boolean InitClientPlacement (ClientData *pCD, long manageFlags);
extern void InitCColormapData (ClientData *pCD);
extern void MakeSystemMenu (ClientData *pCD);
extern void PlaceFrameOnScreen (ClientData *pCD, int *pX, int *pY, int w, 
				int h);
extern void PlaceIconOnScreen (ClientData *pCD, int *pX, int *pY);
extern void ProcessMwmHints (ClientData *pCD);
extern void ProcessWmClass (ClientData *pCD);
extern void ProcessWmHints (ClientData *pCD, Boolean firstTime);
extern void ProcessWmIconTitle (ClientData *pCD, Boolean firstTime);
extern void ProcessWmNormalHints (ClientData *pCD, Boolean firstTime, 
				  long manageFlags);
extern void ProcessWmTransientFor (ClientData *pCD);
extern void ProcessWmWindowTitle (ClientData *pCD, Boolean firstTime);
extern Boolean SetupClientIconWindow (ClientData *pCD, Window window);
extern Boolean WmGetWindowAttributes (Window window);
extern void ProcessSmClientID (ClientData *pCD);
extern void ProcessWmSaveHint (ClientData *pCD);
