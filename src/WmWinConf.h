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

void AdjustPos (int *pX, int *pY, unsigned int oWidth, 
		       unsigned int oHeight, unsigned int nWidth, 
		       unsigned int nHeight);
unsigned int ButtonStateBit (unsigned int button);
void CancelFrameConfig (ClientData *pcd);
void	CheckEatButtonRelease (ClientData *pcd, XEvent *pev);
Boolean CheckVisualPlace (ClientData *pCD, int tmpX, int tmpY);
void CompleteFrameConfig (ClientData *pcd, XEvent *pev);
Cursor ConfigCursor (int frame_part);
void DoFeedback (ClientData *pcd, int x, int y, unsigned int width, 
			unsigned int height, unsigned long newStyle, 
			Boolean resizing);
Boolean DoGrabs (Window grab_win, Cursor cursor, unsigned int pmask, 
			Time grabTime, ClientData *pCD, Boolean alwaysGrab);
void DrawOutline (int x, int y, unsigned int width, 
			 unsigned int height);
void	EatButtonRelease (unsigned int releaseButtons);
void FixFrameValues (ClientData *pcd, int *pfX, int *pfY, 
			    unsigned int *pfWidth, unsigned int *pfHeight, 
			    Boolean resizing);
void FlashOutline (int x, int y, unsigned int width, 
			  unsigned int height);
void ForceOnScreen (int screen, int *pX, int *pY);
void GetClipDimensions (ClientData *pCD, Boolean fromRoot);
void GetConfigEvent (Display *display, Window window, 
			    unsigned long mask, int curX, int curY, 
			    int oX, int oY, unsigned oWidth, 
			    unsigned oHeight, XEvent *pev);
Window GrabWin (ClientData *pcd, XEvent *pev);
void HandleClientFrameMove (ClientData *pcd, XEvent *pev);
void HandleClientFrameResize (ClientData *pcd, XEvent *pev);
Boolean HandleResizeKeyPress (ClientData *pcd, XEvent *pev);
void    MoveOpaque (ClientData *pcd, int x, int y,
		 unsigned int width, unsigned int height);
void MoveOutline (int x, int y, unsigned int width, 
			 unsigned int height);
void ProcessNewConfiguration (ClientData *pCD, int x, int y, 
				     unsigned int width, unsigned int height, 
				     Boolean clientRequest);
void ReGrabPointer (Window grab_win, Time grabTime);
void SetOutline (XSegment *pOutline, int x, int y, unsigned int width, 
			unsigned int height, int fatness);
void SetPointerPosition (int newX, int newY, int *actualX, 
				int *actualY);
Boolean SetPointerResizePart (ClientData *pcd, XEvent *pev);
Boolean StartClientMove (ClientData *pcd, XEvent *pev);
void StartClientResize (ClientData *pcd, XEvent *pev);
Boolean StartResizeConfig (ClientData *pcd, XEvent *pev);
int ResizeType (ClientData *pcd, XEvent *pev);
void UndoGrabs (void);
void HandleMarqueeSelect (WmScreenData *pSD, XEvent *pev);
void StartMarqueeSelect(WmScreenData *pSD, XEvent *pev);
void UpdateMarqueeSelectData (WmScreenData *pSD);
Boolean HandleMarqueeKeyPress (WmScreenData *pSD, XEvent *pev);
void WindowOutline (int x, int y, unsigned int width, 
			 unsigned int height);
void RecomputeMaxConfig(ClientData *pCD);
Boolean WindowIsOnScreen (ClientData *pCD, int *dx, int *dy);
