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

void ProcessWmFile (WmScreenData *pSD);
void ProcessCommandLine (int argc,  char *argv[]);
void ProcessMotifBindings (void);
Boolean FindSessionMatch(int commandArgc, char **commandArgv, ClientData *pCD,
	WmScreenData *pSD, char **pWorkSpaceList, char *clientMachine);
void GetActionIndex (int tableSize, int *actionIndex);
void GetFunctionTableValues (int *execIndex, int *nopIndex, int *actionIndex);
void GetNopIndex (int tableSize, int *nopIndex);
void GetExecIndex (int tableSize, int *execIndex);
Boolean GetSessionHintsInfo (WmScreenData *pSD, long numItems);
FILE          * FopenConfigFile (void);
void            FreeMenuItem (MenuItem *menuItem);

unsigned char * GetNextLine (void);
unsigned char * GetStringC (unsigned char **linePP, Boolean SmBehavior);
void SystemCmd (char *pchCmd);
unsigned char * GetString (unsigned char **linePP);
unsigned int PeekAhead(unsigned char *currentChar,
		       unsigned int currentLev);
Boolean ParseBtnEvent (unsigned char  **linePP,
                              unsigned int *eventType,
                              unsigned int *button,
                              unsigned int *state,
                              Boolean      *fClick);

void            ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
void            ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
MenuItem      * ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
void ParseSessionClientState (WmScreenData *pSD, int count,
			      unsigned char *string);
void ParseSessionCommand (WmScreenData *pSD,  int count,
			  unsigned char **commandString);
void ParseSessionGeometry (WmScreenData *pSD, int count,
			   unsigned char *string);
void ParseSessionItems (WmScreenData *pSD);
void ParseSessionWorkspaces (WmScreenData *pSD,  int count,
			     unsigned char *string);
void ParseSessionHost (WmScreenData *pSD,  int count,
			     unsigned char *string);
void ParseDtSessionHints (WmScreenData *pSD, unsigned char *property);
int             ParseWmFunction (unsigned char **linePP, unsigned int res_spec, WmFunction *pWmFunction);
void            PWarning (char *message);
void            SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
void      ScanAlphanumeric (unsigned char **linePP);
void            ScanWhitespace(unsigned char  **linePP);
void            ToLower (unsigned char  *string);
void		SyncModifierStrings(void);

#define GetSmartString(s)	GetString (s)
