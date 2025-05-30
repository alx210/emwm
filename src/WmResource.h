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

#ifndef NO_MESSAGE_CATALOG
void InitBuiltinSystemMenu(void);
#endif
void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
GC   GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
Boolean Monochrome (Screen *screen);
void ProcessWmColors (WmScreenData *pSD);
void ProcessWmResources (void);
void SetStdGlobalResourceValues (void);
void ProcessScreenListResource (void);
void ProcessAppearanceResources (WmScreenData *pSD);
void ProcessGlobalScreenResources (void);
void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
void ProcessWorkspaceList (WmScreenData *pSD);
void ProcessWorkspaceResources (WmWorkspaceData *pWS);
void ProcessClientResources (ClientData *pCD);
void SetStdClientResourceValues (ClientData *pCD);
void SetStdScreenResourceValues (WmScreenData *pSD);
char *WmRealloc (char *ptr, unsigned size);
char *WmMalloc (char *ptr, unsigned size);
void SetupDefaultResources (WmScreenData *pSD);
Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);
String ResCat (String s1, String s2, String s3, String s4);
void CheckForNoDither (AppearanceData *pAD);
void ProcessPresenceResources (WmScreenData *pSD);

#ifndef NO_MESSAGE_CATALOG
extern char *builtinSystemMenu;
#else
extern char builtinSystemMenu[];
#endif
extern char builtinKeyBindings[];
extern char builtinRootMenu[];
extern char builtinSystemMenuName[];
