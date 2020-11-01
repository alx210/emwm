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
/*   $XConsortium: WmResParse.h /main/5 1996/06/11 16:00:58 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#include <stdio.h>


#ifdef PANELIST
extern void            ProcessWmFile (WmScreenData *pSD, Boolean bNested);
#else /* PANELIST */
extern void ProcessWmFile (WmScreenData *pSD);
#endif /* PANELIST */
extern void ProcessCommandLine (int argc,  char *argv[]);
extern void ProcessMotifBindings (void);
#ifdef WSM
extern Boolean         FindDtSessionMatch(int commandArgc, 
					   char **commandArgv, 
					   ClientData *pCD, 
					   WmScreenData *pSD, 
					   char **pWorkSpaceList,
					   char *clientMachine);
extern void            WmDtGetHelprgs(char *args, 
				       unsigned char** volume, 
				       unsigned char** topic, 
				       int *argsCount);
extern void GetActionIndex (int tableSize, int *actionIndex);
extern void            GetFunctionTableValues (int *execIndex, int *nopIndex,
		    int *actionIndex);
extern void GetNopIndex (int tableSize, int *nopIndex);
extern void GetExecIndex (int tableSize, int *execIndex);
extern Boolean GetSessionHintsInfo (WmScreenData *pSD, long numItems);
#endif /* WSM */
extern FILE          * FopenConfigFile (void);
extern void            FreeMenuItem (MenuItem *menuItem);
#ifndef WSM
extern unsigned char * GetNextLine (void);
#endif /* not WSM */
#ifdef WSM
extern unsigned char * GetStringC (unsigned char **linePP, Boolean SmBehavior);
extern void SystemCmd (char *pchCmd);
#else /* WSM */
extern unsigned char * GetString (unsigned char **linePP);
#endif /* WSM */
extern Boolean ParseBtnEvent (unsigned char  **linePP,
                              unsigned int *eventType,
                              unsigned int *button,
                              unsigned int *state,
                              Boolean      *fClick);

extern void            ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
extern void            ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
extern Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
extern MenuItem      * ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
#ifdef WSM
extern void ParseSessionClientState (WmScreenData *pSD, int count,
			      unsigned char *string);
extern void ParseSessionCommand (WmScreenData *pSD,  int count,
			  unsigned char **commandString);
extern void ParseSessionGeometry (WmScreenData *pSD, int count,
			   unsigned char *string);
extern void ParseSessionItems (WmScreenData *pSD);
extern void ParseSessionWorkspaces (WmScreenData *pSD,  int count,
			     unsigned char *string);
extern void ParseSessionHost (WmScreenData *pSD,  int count,
			     unsigned char *string);
extern void ParseDtSessionHints (WmScreenData *pSD, unsigned char *property);
#endif /* WSM */
extern int             ParseWmFunction (unsigned char **linePP, unsigned int res_spec, WmFunction *pWmFunction);
extern void            PWarning (char *message);
extern void            SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
extern void      ScanAlphanumeric (unsigned char **linePP);
#ifndef WSM
extern void            ScanWhitespace(unsigned char  **linePP);
#endif /* not WSM */
extern void            ToLower (unsigned char  *string);
extern void		SyncModifierStrings(void);
#ifdef PANELIST
extern void DeleteTempConfigFileIfAny (void);
extern Boolean ParseWmFunctionArg (
		unsigned char **linePP,
		int ix, 
		WmFunction wmFunc, 
		void **ppArg,
		String sClientName,
		String sTitle);
extern Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
extern Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
extern Boolean ParseWmFuncActionArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
#endif /* PANELIST */
#ifdef WSM
#define ToLower(s)		(_DtWmParseToLower (s))
#define GetNextLine()		(_DtWmParseNextLine (wmGD.pWmPB))
#define GetSmartSMString(s)	(_DtWmParseNextTokenC (s, True))
#define GetSmartString(s)	(_DtWmParseNextTokenC (s, False))
#define GetString(s)		(_DtWmParseNextTokenC (s, False))
#define ScanWhitespace(s)	(_DtWmParseSkipWhitespaceC (s))
#endif /* WSM */

#if ((!defined(WSM)) || defined(MWM_QATS_PROTOCOL))
extern Boolean IsClientCommand (String);
extern Boolean SetGreyedContextAndMgtMask (MenuItem *menuItem,
					   WmFunction wmFunction);
#endif /* !defined(WSM) || defined(MWM_QATS_PROTOCOL) */
