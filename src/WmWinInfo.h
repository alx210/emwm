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

void FindClientPlacement (ClientData *pCD);
void FixWindowConfiguration (ClientData *pCD, unsigned int *pWidth, 
				    unsigned int *pHeight, 
				    unsigned int widthInc, 
				    unsigned int heightInc);
void FixWindowSize (ClientData *pCD, unsigned int *pWidth, 
			   unsigned int *pHeight, unsigned int widthInc, 
			   unsigned int heightInc);
ClientData *GetClientInfo (WmScreenData *pSD, Window clientWindow, 
				  long manageFlags);
ClientData *GetWmClientInfo (WmWorkspaceData *pWS, ClientData *pCD, 
				    long manageFlags);
void CalculateGravityOffset (ClientData *pCD, int *xoff, int *yoff);
Boolean InitClientPlacement (ClientData *pCD, long manageFlags);
void InitCColormapData (ClientData *pCD);
void MakeSystemMenu (ClientData *pCD);
void PlaceFrameOnScreen (ClientData *pCD, int *pX, int *pY, int w, 
				int h);
void PlaceIconOnScreen (ClientData *pCD, int *pX, int *pY);
void ProcessMwmHints (ClientData *pCD, Boolean firstTime);
void ProcessWmClass (ClientData *pCD);
void ProcessWmHints (ClientData *pCD, Boolean firstTime);
void ProcessWmIconTitle (ClientData *pCD, Boolean firstTime);
void ProcessWmNormalHints (ClientData *pCD, Boolean firstTime, 
				  long manageFlags);
void ProcessWmTransientFor (ClientData *pCD);
void ProcessWmWindowTitle (ClientData *pCD, Boolean firstTime);
Boolean SetupClientIconWindow (ClientData *pCD, Window window);
Boolean WmGetWindowAttributes (Window window);
void ProcessSmClientID (ClientData *pCD);
void ProcessWmSaveHint (ClientData *pCD);
