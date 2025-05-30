#ifndef _WM_MENU_H
#define _WM_MENU_H

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

void ActivateCallback (Widget w, caddr_t client_data, 
			      caddr_t call_data);
Widget CreateMenuWidget (WmScreenData *pSD,
				String menuName, Widget parent,
				Boolean fTopLevelPane, MenuSpec *topMenuSpec,
				MenuItem *moreMenuItems);
void FreeCustomMenuSpec (MenuSpec *menuSpec);
void MWarning (char *format, char *message);
MenuSpec *MakeMenu (WmScreenData *pSD,
			   String menuName, 
			   Context initialContext, Context accelContext, 
			   MenuItem *moreMenuItems, Boolean fSystemMenu);
void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, 
		      unsigned int button, Context newContext, long flags, 
		      XEvent *passedInEvent);
void TraversalOff (MenuSpec *menuSpec);
void TraversalOn (MenuSpec *menuSpec);
void UnpostMenu (MenuSpec *menuSpec);
MenuSpec *DuplicateMenuSpec (MenuSpec *menuSpec);

#endif /* _WM_MENU_H */
