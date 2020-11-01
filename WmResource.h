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
 * Motif Release 1.2.3
*/ 
/*   $TOG: WmResource.h /main/6 1997/04/14 12:52:19 dbl $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifndef NO_MESSAGE_CATALOG
extern void InitBuiltinSystemMenu(void);
#endif
extern void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
extern GC   GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
extern void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
#ifdef WSM
extern Boolean Monochrome (Screen *screen);
extern void ProcessWmColors (WmScreenData *pSD);
#endif /* WSM */
extern void ProcessWmResources (void);
extern void SetStdGlobalResourceValues (void);
extern void ProcessScreenListResource (void);
extern void ProcessAppearanceResources (WmScreenData *pSD);
extern void ProcessGlobalScreenResources (void);
extern void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
#ifdef WSM
extern void ProcessWorkspaceList (WmScreenData *pSD);
#endif /* WSM */
extern void ProcessWorkspaceResources (WmWorkspaceData *pWS);
extern void ProcessClientResources (ClientData *pCD);
extern void SetStdClientResourceValues (ClientData *pCD);
extern void SetStdScreenResourceValues (WmScreenData *pSD);
extern char *WmRealloc (char *ptr, unsigned size);
extern char *WmMalloc (char *ptr, unsigned size);
extern void SetupDefaultResources (WmScreenData *pSD);
extern Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);
#ifdef WSM
extern String ResCat (String s1, String s2, String s3, String s4);
void CheckForNoDither (AppearanceData *pAD);
#endif /* WSM */

#ifndef NO_MESSAGE_CATALOG
extern char *builtinSystemMenu;
#else
extern char builtinSystemMenu[];
#endif
extern char builtinKeyBindings[];
extern char builtinRootMenu[];
extern char builtinSystemMenuName[];

#ifndef WSM
#define Monochrome(screen) ( DefaultDepthOfScreen(screen) == 1 )
#endif /* not WSM */
