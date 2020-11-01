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
/* 
 * Motif Release 1.2.2
*/ 
/*   $XConsortium: WmMenu.h /main/7 1996/11/20 15:20:45 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
# define MAKE_MENU(pSD, pCD, mName, iC, aC, moreItems, sysMenu) \
	MakeMenu(pSD, pCD, mName, iC, aC, moreItems, sysMenu)
# define CREATE_MENU_WIDGET(pSD, pCD, mName, parent, fTLP, tSpec, moreItems) \
	CreateMenuWidget(pSD, pCD, mName, parent, fTLP, tSpec, moreItems)
#else
# define MAKE_MENU(pSD, pCD, mName, iC, aC, moreItems, sysMenu) \
	MakeMenu(pSD, mName, iC, aC, moreItems, sysMenu)
# define CREATE_MENU_WIDGET(pSD, pCD, mName, parent, fTLP, tSpec, moreItems) \
	CreateMenuWidget(pSD, mName, parent, fTLP, tSpec, moreItems)
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

extern void ActivateCallback (Widget w, caddr_t client_data, 
			      caddr_t call_data);
extern Widget CreateMenuWidget (WmScreenData *pSD,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
				ClientData *pCD,
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
				String menuName, Widget parent,
				Boolean fTopLevelPane, MenuSpec *topMenuSpec,
				MenuItem *moreMenuItems);
extern void FreeCustomMenuSpec (MenuSpec *menuSpec);
extern void MWarning (char *format, char *message);
extern MenuSpec *MakeMenu (WmScreenData *pSD,
#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
			   ClientData *pCD,
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
			   String menuName, 
			   Context initialContext, Context accelContext, 
			   MenuItem *moreMenuItems, Boolean fSystemMenu);
extern void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, 
		      unsigned int button, Context newContext, long flags, 
		      XEvent *passedInEvent);
extern void TraversalOff (MenuSpec *menuSpec);
extern void TraversalOn (MenuSpec *menuSpec);
extern void UnpostMenu (MenuSpec *menuSpec);

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
extern void InsertTreeOnAllClients (WmScreenData *pSD, CmdTree *tree,
				    Atom selection, Context active_context,
				    Boolean in_line);
extern void InsertTreeOnSingleClient (WmScreenData *pSD, ClientData *pCD,
				      CmdTree *tree, Atom selection,
				      Context active_context, Boolean in_line);
extern void InsertTreeOnRootMenu (WmScreenData *pSD, CmdTree *tree,
				  Atom selection, Boolean in_line);
extern void ModifyClientCommandTree (WmScreenData *pSD, ClientData *pCD,
				     OpRange range, CmdTree *tree,
				     CmdModifier modifier, Context context,
				     String newname);
extern void DestroyMenuSpecWidgets (MenuSpec *menuspec);
extern void DestroyMenuSpec (WmScreenData *pSD, CARD32 commandID);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */

extern MenuSpec *DuplicateMenuSpec (MenuSpec *menuSpec);

#endif /* _WM_MENU_H */
