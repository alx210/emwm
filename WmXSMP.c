/* $XConsortium: WmXSMP.c /main/12 1996/05/17 12:54:14 rswiston $ */
/*
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/param.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>
#include <X11/SM/SM.h>
#include <Xm/XmP.h>
#include "WmGlobal.h"
#include "WmXSMP.h"
#ifdef WSM
# include "WmWrkspace.h"
# include <Dt/Session.h>
#endif

#define FIX_1193

typedef struct _ProxyClientInfo
{
    int screen;
    char *wmCommand;
    char *wmClientMachine;
    char *clientID;
} ProxyClientInfo;

#define RESTORE_RESOURCE(pCD, resFlag) \
	((pCD)->ignoreWMSaveHints || !((pCD)->wmSaveHintFlags & (resFlag)))
#define SAVE_RESOURCE(pCD, resFlag) RESTORE_RESOURCE(pCD, resFlag)

#define MAX_RESOURCE_LEN 1024

#ifdef WSM
static char *dtwmFileName = "dtwm.db";
#else
static char *dtwmFileName = ".mwmclientdb";
# define EXTRA_FN_CHARS 20
#endif

/* Fully-qualified resource names/classes. */
static char *xPositionStr = "%s.position.x";
static char *yPositionStr = "%s.position.y";
static char *widthSizeStr = "%s.size.width";
static char *heightSizeStr = "%s.size.height";
static char *initialStateStr = "%s.initialState";
static char *wmCommandStr = "%s.wmCommand";
static char *wmClientMachineStr = "%s.wmClientMachine";
static char *screenStr = "%s.screen";
#ifdef WSM
static char *workspacesStr = "%s.workspaces";
static char *iconXPosStr = "%s.iconPos.x.%s";
static char *iconYPosStr = "%s.iconPos.y.%s";
#else
static char *iconXPosStr = "%s.iconPos.x";
static char *iconYPosStr = "%s.iconPos.y";
#endif

/* Header for private database. */
static char *dbHeader = "\
! %s\n\
!\n\
.version: %s\n\
.dtwmID: %s\n";

/* Format for client entries in database. */
static char *dbClientFormat = "\
!\n\
%s.%s: %s\n\
!\n";
static char *intArg = ": %d\n";
static char *strArg = ": %s\n";
static char *normalStateStr = "NormalState";
static char *iconicStateStr = "IconicState";

static char *XSMPClientStr = "Client";
static char *proxyClientStr = "ProxyClient";

#ifndef WSM
static char *dbFileArgStr = "-session";
#endif

/* Flag to tell us how to treat ProxyClient info. */
static Boolean smClientDBCheckpointed = False;

/*
 *  Prototypes
 */
/* Session mgmt callbacks. */
static void smSaveYourselfCallback(Widget, XtPointer, XtPointer);
static void smDieCallback(Widget, XtPointer, XtPointer);

/* Build client database file name. */
static void buildDBFileName(char [MAXPATHLEN], Boolean);
#ifndef WSM
/*
 *Get clientDB name according to argv; set according to dbFileName.
 */
static void getClientDBName(void);
static void setClientDBName(void);
static char **getNewRestartCmd(void);
static void freeNewRestartCmd(char **);
#endif /* ! WSM */

#ifdef WSM
/* Get string of client's workspaces. */
static char *getClientWorkspaces(ClientData *);
#endif

/* List-of-clients utilities. */
static Boolean addClientToList(ClientData ***, int *, ClientData *);
static int clientWorkspaceCompare(const void *, const void *);

/* XSMP/Proxy functions to save/restore resources. */
static char *getClientResource(char *, char *);
static char *getXSMPResource(ClientData *, int, char *);
static void getClientGeometry(ClientData *, int *, int *,
			      unsigned int *, unsigned int *);
static Boolean getProxyClientInfo(ClientData *, ProxyClientInfo *);
static Bool cmpProxyClientProc(XrmDatabase *, XrmBindingList,
			       XrmQuarkList, XrmRepresentation *,
			       XrmValue *, XPointer);
static char *findProxyClientID(ClientData *);
static Boolean findXSMPClientDBMatch(ClientData *, char **);
static Boolean findProxyClientDBMatch(ClientData *, char **);
static Boolean saveXSMPClient(FILE *, ClientData *);
static Boolean saveProxyClient(FILE *, ClientData *, int);
static void dbRemoveProxyClientEntry(char *);

static void
smSaveYourselfCallback(Widget w, XtPointer clientData, XtPointer callData)
{
    XtCheckpointToken cpToken = (XtCheckpointToken)callData;
    XrmDatabase newClientDB;
    int scr;
    static Boolean firstTime = True;

    /*
     *  This callback will be called on connection to the Session Manager.
     *  At that time, we don't want to save any state, and we don't
     *  want to request the second phase.
     */
    if (firstTime)
    {
	firstTime = False;
	return;
    }

    /* Only respond to Local and Both save requests. */
    if ((cpToken->save_type != SmSaveLocal) &&
	(cpToken->save_type != SmSaveBoth))
	return;

    if (cpToken->shutdown &&
	(cpToken->cancel_shutdown ||
	 cpToken->request_cancel ||
	 !cpToken->save_success))
	return;  /* Return, maintaining current state */

    /* If first phase, request notification when all other clients saved. */
    if (cpToken->phase == 1)
    {
	cpToken->request_next_phase = True;
	return;
    }

#ifdef WSM
    /* Second phase: all other clients saved; now I can save myself. */
    /* Copied from WmEvent.c. */
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	if (wmGD.Screens[scr].managed)
	{
	    /*
	     * Write out current workspace, frontpanel 
	     * position and iconbox position and size.
	     */
	    SaveResources(&wmGD.Screens[scr]);
	}
    }
#endif

    /*
     *  NEW FOR SESSION MANAGEMENT: Write private client resource database.
     *  Destroy old client database and save new one.
     */
    if ((newClientDB = SaveClientResourceDB())
	!= (XrmDatabase)NULL)
    {
	if (wmGD.clientResourceDB != (XrmDatabase)NULL)
	    XrmDestroyDatabase(wmGD.clientResourceDB);
	wmGD.clientResourceDB = newClientDB;
	smClientDBCheckpointed = True;

#ifndef WSM
	/* Set new session properties if wmGD.dbFileName is valid. */
	if (wmGD.dbFileName != (char *)NULL)
	{
	    char **newRestartCmd, **ptr;
	    char *newDiscardCmd[4];
	    Arg args[10];
	    int nargs;

	    newDiscardCmd[0] = "rm";
	    newDiscardCmd[1] = "-f";
	    newDiscardCmd[2] = wmGD.dbFileName;
	    newDiscardCmd[3] = (char *)NULL;

	    newRestartCmd = getNewRestartCmd();

	    nargs = 0;
	    XtSetArg(args[nargs], XtNrestartCommand, newRestartCmd); nargs++;
	    XtSetArg(args[nargs], XtNdiscardCommand, newDiscardCmd); nargs++;
	    XtSetValues(wmGD.topLevelW, args, nargs);

	    freeNewRestartCmd(newRestartCmd);
	}
#endif /* ! WSM */
    }
}

static void
smDieCallback(Widget w, XtPointer clientData, XtPointer callData)
{
    /* We assume we've saved our state by the time this is called. */
    ExitWM(0);
}

static void
buildDBFileName(char fileNameBuf[MAXPATHLEN], Boolean doingSave)
{
#ifdef WSM

    char *savePath = (char *)NULL;

    fileNameBuf[0] = '\0';
    if (doingSave)
    {
	char *saveFile = (char *)NULL;
	char *ptr;

	if (DtSessionSavePath(wmGD.topLevelW, &savePath, &saveFile))
	{
	    XtFree(saveFile);

	    if ((ptr = strrchr(savePath, '/')) != (char *)NULL)
		*ptr = '\0';

	    if (strlen(savePath) + strlen(dtwmFileName) + 2 < MAXPATHLEN)
		sprintf(fileNameBuf, "%s/%s", savePath, dtwmFileName);

	    XtFree(savePath);
	}
    }
    else
    {
	if (DtSessionRestorePath(wmGD.topLevelW, &savePath, dtwmFileName))
	{
	    if ((int)strlen(savePath) < MAXPATHLEN)
		strcpy(fileNameBuf, savePath);

	    XtFree(savePath);
	}
    }

    if (fileNameBuf[0] == '\0')
	strcpy(fileNameBuf, dtwmFileName);

#else

    strcpy(fileNameBuf, (wmGD.dbFileName == (char *)NULL) ?
	   dtwmFileName : wmGD.dbFileName);

#endif
}

#ifndef WSM

/*
 *  See if dbFileArgStr specified on command line.  Save subsequent arg;
 *  if not, see if resource set; if not, put files in user's home directory.
 *  NOTE: we allocate extra space for the filename so we can append numbers
 *  without reallocating in setClientDBName.
 */
static void
getClientDBName(void)
{
    char **argP;

    /* See if DB filename specified on command line. */
    wmGD.dbFileName = (char *)NULL;

    if (wmGD.argv != (char **)NULL)
    {
	for (argP = wmGD.argv; *argP != (char *)NULL; argP++)
	{
	    if (strcmp(*argP, dbFileArgStr) == 0)
	    {
		if (*(++argP) != (char *)NULL)
		{
		    if ((wmGD.dbFileName =
			 (char *)XtMalloc((strlen(*argP) + 1 +
					   EXTRA_FN_CHARS) *
					  sizeof(char)))
			!= (char *)NULL)
			strcpy(wmGD.dbFileName, *argP);
		}
		break;
	    }
	}
    }

    /* Check resource if necessary. */
    if (wmGD.dbFileName == (char *)NULL)
    {
	if (wmGD.sessionClientDB != (String)NULL)
	{
	    if ((wmGD.dbFileName =
		 (char *)XtMalloc((strlen(wmGD.sessionClientDB) + 1 +
				   EXTRA_FN_CHARS) *
				  sizeof(char)))
		!= (char *)NULL)
		strcpy(wmGD.dbFileName, wmGD.sessionClientDB);
	}
    }

    if (wmGD.dbFileName == (char *)NULL)
    {
	char *homeDir = XmeGetHomeDirName();

	if ((wmGD.dbFileName =
	     (char *)XtMalloc((strlen(homeDir) + strlen(dtwmFileName) + 2 +
			       EXTRA_FN_CHARS) * sizeof(char)))
	    != (char *)NULL)
	    sprintf(wmGD.dbFileName, "%s/%s", homeDir, dtwmFileName);
    }
}

/*
 *  See comments above in getClientDBName.
 */
static void
setClientDBName(void)
{
    char *ptr;

    if (wmGD.dbFileName == (char *)NULL)
	return;

    /* Change trailing ".<number>" to ".<number+1>" */
    if ((ptr = strrchr(wmGD.dbFileName, '.')) != (char *)NULL)
    {
	char *p1;

	for (p1 = ++ptr; *p1 != '\0'; p1++)
	{
	    if (!isdigit(*p1))
		break;
	}

	if (*p1 == '\0')
	{
	    int numSuffix;

	    numSuffix = atoi(ptr) + 1;
	    sprintf(ptr, "%d", numSuffix);

	    /* Success!  We're all done here. */
	    return;
	}
    }

    /* Otherwise, append ".0" to filename. */
    strcat(wmGD.dbFileName, ".0");
}

static char **
getNewRestartCmd(void)
{
    char **argP;
    int argc, i;
    int fileArgIndex = -1;
    Arg args[10];
    int nargs;
    char **restartCmd;
    char **newRestartCmd;

    nargs = 0;
    XtSetArg(args[nargs], XtNrestartCommand, &restartCmd); nargs++;
    XtGetValues(wmGD.topLevelW, args, nargs);

    if (restartCmd == (char **)NULL)
	return (char **)NULL;

    for (argc = 0, argP = restartCmd; *argP != (char *)NULL; argP++, argc++)
    {
	if (strcmp(*argP, dbFileArgStr) == 0)
	{
	    if (*(++argP) == (char *)NULL)
		break;

	    fileArgIndex = argc++; /* Point at dbFileArgStr, not filename */
	}
    }

    if (fileArgIndex < 0)
    {
	fileArgIndex = argc;
	argc += 2;
    }

    if ((newRestartCmd = (char **)XtMalloc((argc + 1) * sizeof(char *)))
	== (char **)NULL)
	return (char **)NULL;

    for (i = 0; i < argc; i++)
    {
	if (i != fileArgIndex)
	{
	    newRestartCmd[i] = XtNewString(restartCmd[i]);
	}
	else
	{
	    newRestartCmd[i++] = XtNewString(dbFileArgStr);
	    newRestartCmd[i] = XtNewString(wmGD.dbFileName);
	}
    }
    newRestartCmd[i] = (char *)NULL;

    return newRestartCmd;
}

static void
freeNewRestartCmd(char **restartCmd)
{
#ifdef FIX_1193
    if(restartCmd)
    {
	char **tmp = restartCmd;
	while (*restartCmd != (char *)NULL)
		XtFree(*(restartCmd++));

	XtFree((char *)tmp);
    }
#else
    while (*restartCmd != (char *)NULL)
	XtFree(*(restartCmd++));

    XtFree((char *)restartCmd);
#endif
}

#endif /* ! WSM */

#ifdef WSM

static char *
getClientWorkspaces(ClientData *pCD)
{
    WmScreenData *pSD = pCD->pSD;
    WmWorkspaceData *pWS;

    /* Should we use _DtWmParseMakeQuotedString() when looking at */
    /* the name of the workspace, as is done in WmWrkspace.c? */

    /* Easy but slow way to do this would be to use XGetAtomName(). */
    /* To avoid XServer round trips (and to weed out invalid WS names) */
    /* we look through workspaces attached to this screen for ID matches. */
    char *cwsP, *tmpP, *wsNameP;
    int pLen = 0;
    int i;

    for (i = 0; i < pCD->numInhabited; i++)
    {
	if ((pWS = GetWorkspaceData(pSD, pCD->pWsList[i].wsID))
	    != (WmWorkspaceData *)NULL)
	{
	    wsNameP = pWS->name;
	    if (pLen == 0)
	    {
		pLen = strlen(wsNameP) + 1;  /* 1 for null termination */
		if ((cwsP = (char *)XtMalloc(pLen * sizeof(char)))
		    == (char *)NULL)
		    return (char *)NULL;

		strcpy(cwsP, wsNameP);
	    }
	    else
	    {
		pLen += strlen(wsNameP) + 1;  /* 1 for space */
		if ((tmpP = (char *)XtRealloc(cwsP, pLen * sizeof(char)))
		    == (char *)NULL)
		{
		    XtFree((char *)cwsP);
		    return (char *)NULL;
		}
		cwsP = tmpP;
		strcat(cwsP, " ");
		strcat(cwsP, wsNameP);
	    }
	}
    }

    return cwsP;
}

#endif /* WSM */

static Boolean
addClientToList(ClientData ***cdList, int *nClients, ClientData *pCD)
{
    ClientData **newPtr = (ClientData **)
	XtRealloc((char *)*cdList, (*nClients + 1) * sizeof(ClientData *));

    if (newPtr == (ClientData **)NULL)
    {
	if (*cdList != (ClientData **)NULL)
	    XtFree((char *)*cdList);
	return False;
    }

    *cdList = newPtr;
    newPtr[*nClients] = pCD;
    (*nClients)++;

    return True;
}

static int
clientWorkspaceCompare(const void *ppCD1, const void *ppCD2)
{
    ClientData *pCD1 = *(ClientData **)ppCD1;
    ClientData *pCD2 = *(ClientData **)ppCD2;
    int screenDiff;

    /* Sort first by screen. */
    if ((screenDiff = pCD1->pSD->screen - pCD2->pSD->screen) != 0)
	return screenDiff;

#ifdef WSM

    /* If same screen, sort by workspace id. */
    /* How do we handle clients that live in more than one workspace? */
    /* For now, pick the "current" one - if not in active workspace, */
    /* this will simply be the first one in the client's list. */
    return (int)(pCD1->pWsList[pCD1->currentWsc].wsID -
		 pCD2->pWsList[pCD2->currentWsc].wsID);

#else

    /* If no WSM, must be in same workspace if screen is same! */
    return 0;

#endif
}

/*
 *  Assumes: wmGD.clientResourceDB is non-NULL
 */
static char *
getClientResource(char *clientID, char *fmtStr)
{
    char resourceBuf[MAX_RESOURCE_LEN];
    char *resourceType;
    XrmValue resourceValue;

    sprintf(resourceBuf, fmtStr, clientID);
    if (XrmGetResource(wmGD.clientResourceDB, resourceBuf, resourceBuf,
		       &resourceType, &resourceValue))
	return (char *)resourceValue.addr;

    return (char *)NULL;
}

/*
 *  Assumes: pCD has non-NULL smClientID;
 *           wmGD.clientResourceDB is non-NULL
 */
static char *
getXSMPResource(ClientData *pCD, int resourceFlag, char *fmtStr)
{
    if (RESTORE_RESOURCE(pCD, resourceFlag))
	return getClientResource(pCD->smClientID, fmtStr);

    return (char *)NULL;
}

/*
 *  Return True if client is XSMP, False otherwise.
 */
static Boolean
findXSMPClientDBMatch(ClientData *pCD, char **workSpaceNamesP)
{
    if (pCD->smClientID != (String)NULL)
    {
	if (wmGD.clientResourceDB != (XrmDatabase)NULL)
	{
	    char *resourcePtr;

	    if ((resourcePtr = getXSMPResource(pCD, WMSAVE_X, xPositionStr))
		!= (char *)NULL)
	    {
		pCD->clientX = atoi(resourcePtr);
		pCD->clientFlags |= SM_X;
	    }

	    if ((resourcePtr = getXSMPResource(pCD, WMSAVE_Y, yPositionStr))
		!= (char *)NULL)
	    {
		pCD->clientY = atoi(resourcePtr);
		pCD->clientFlags |= SM_Y;
	    }

#ifndef WSM
	    if ((resourcePtr =
		 getXSMPResource(pCD, WMSAVE_ICON_X, iconXPosStr))
		!= (char *)NULL)
	    {
		ICON_X(pCD) = atoi(resourcePtr);
		pCD->clientFlags |= SM_ICON_X;
	    }

	    if ((resourcePtr =
		 getXSMPResource(pCD, WMSAVE_ICON_Y, iconYPosStr))
		!= (char *)NULL)
	    {
		ICON_Y(pCD) = atoi(resourcePtr);
		pCD->clientFlags |= SM_ICON_Y;
	    }
#endif

	    if ((resourcePtr = getXSMPResource(pCD, WMSAVE_WIDTH,
					       widthSizeStr))
		!= (char *)NULL)
	    {
		pCD->clientWidth = atoi(resourcePtr);
		pCD->clientFlags |= SM_WIDTH;
	    }

	    if ((resourcePtr = getXSMPResource(pCD, WMSAVE_HEIGHT,
					       heightSizeStr))
		!= (char *)NULL)
	    {
		pCD->clientHeight = atoi(resourcePtr);
		pCD->clientFlags |= SM_HEIGHT;
	    }

	    if ((resourcePtr = getXSMPResource(pCD, WMSAVE_STATE,
					       initialStateStr))
		!= (char *)NULL)
	    {
		pCD->clientState =
		    (strcmp(resourcePtr, normalStateStr) == 0) ?
			NORMAL_STATE : MINIMIZED_STATE;
		pCD->clientFlags |= SM_CLIENT_STATE;
	    }

#ifdef WSM
	    if ((workSpaceNamesP != (char **)NULL) &&
		((resourcePtr = getXSMPResource(pCD, WMSAVE_WORKSPACES,
						workspacesStr))
		 != (char *)NULL))
	    {
		*workSpaceNamesP = XtNewString(resourcePtr);
	    }
#endif
	}

	/* Always return True for XSMP clients. */
	return True;
    }

    return False;
}

static Boolean
getProxyClientInfo(ClientData *pCD, ProxyClientInfo *proxyClientInfo)
{
    XTextProperty textProperty;
    unsigned long i;

    /* WM_COMMAND is required; WM_CLIENT_MACHINE is optional. */
    if (!XGetTextProperty(wmGD.display, pCD->client, &textProperty,
			  XA_WM_COMMAND))
	return False;

    if ((textProperty.encoding != XA_STRING) ||
	(textProperty.format != 8) ||
	(textProperty.value[0] == '\0'))
    {
	if (textProperty.value)
	    free((char *)textProperty.value);

	return False;
    }

    /* Convert embedded NULL characters to space characters. */
    /* (If last char is NULL, leave it alone) */
    for (i = 0; i < textProperty.nitems - 1; i++)
    {
	if (textProperty.value[i] == '\0')
	    textProperty.value[i] = ' ';
    }

    proxyClientInfo->screen = pCD->pSD->screen;
    proxyClientInfo->wmCommand = (char *)textProperty.value;

    /* Since WM_CLIENT_MACHINE is optional, don't fail if not found. */
    if (XGetWMClientMachine(wmGD.display, pCD->client, &textProperty))
	proxyClientInfo->wmClientMachine = (char *)textProperty.value;
    else proxyClientInfo->wmClientMachine = (char *)NULL;

    proxyClientInfo->clientID = (char *)NULL;

    return True;
}

/*
 *  IMPORTANT: This function is called by XrmEnumerateDatabase().
 *  It calls other Xrm*() functions - if dtwm is threaded, THIS
 *  WILL HANG.  For now, dtwm is NOT threaded, so no problem.
 */
static Bool
cmpProxyClientProc(XrmDatabase *clientDB, XrmBindingList bindingList,
		   XrmQuarkList quarkList, XrmRepresentation *reps,
		   XrmValue *value, XPointer uData)
{
    char *clientScreen;
    char *wmCommand;
    char *wmClientMachine;
    char *clientID = (char *)value->addr;
    ProxyClientInfo *proxyClientInfo = (ProxyClientInfo *)uData;

    if (((wmCommand =
	  getClientResource(clientID, wmCommandStr)) == (char *)NULL) ||
	(strcmp(wmCommand, proxyClientInfo->wmCommand) != 0) ||
	((clientScreen =
	  getClientResource(clientID, screenStr)) == (char *)NULL) ||
	(atoi(clientScreen) != proxyClientInfo->screen))
	return FALSE;

    /* So far so good.  If WM_CLIENT_MACHINE missing from either, */
    /* or if it is set in both and it's the same, we've got a match! */
    if (!proxyClientInfo->wmClientMachine ||
	((wmClientMachine =
	  getClientResource(clientID, wmClientMachineStr)) == (char *)NULL) ||
	(strcmp(proxyClientInfo->wmClientMachine, wmClientMachine) == 0))
    {
	proxyClientInfo->clientID = clientID;
	return TRUE;
    }

    return FALSE;
}

static char *
findProxyClientID(ClientData *pCD)
{
    ProxyClientInfo proxyClientInfo;
    char *clientID = (char *)NULL;
    static XrmName proxyName[2] = {NULLQUARK, NULLQUARK};
    static XrmClass proxyClass[2] = {NULLQUARK, NULLQUARK};

    if (proxyName[0] == NULLQUARK)
    {
	proxyName[0] = XrmStringToName(proxyClientStr);
	proxyClass[0] = XrmStringToClass(proxyClientStr);
    }

    /*
     *  We need to match the screen and
     *  the WM_COMMAND and WM_CLIENT_MACHINE properties.
     */
    if (!getProxyClientInfo(pCD, &proxyClientInfo))
	return clientID;

    if (XrmEnumerateDatabase(wmGD.clientResourceDB, proxyName, proxyClass,
			     XrmEnumOneLevel, cmpProxyClientProc,
			     (XPointer)&proxyClientInfo))
	clientID = proxyClientInfo.clientID;

    if (proxyClientInfo.wmCommand)
	free(proxyClientInfo.wmCommand);
    if (proxyClientInfo.wmClientMachine)
	free(proxyClientInfo.wmClientMachine);

    return clientID;
}

/*
 *  Return True if client is *not* XSMP and is listed in the resource DB
 *  and no checkpoint done yet.  Also remove entry from DB if found.
 */
static Boolean
findProxyClientDBMatch(ClientData *pCD, char **workSpaceNamesP)
{
    if ((pCD->smClientID == (String)NULL) &&
	(wmGD.clientResourceDB != (XrmDatabase)NULL) &&
	(!smClientDBCheckpointed))
    {
	char *proxyClientID;

	if ((proxyClientID = findProxyClientID(pCD)) != (char *)NULL)
	{
	    char *resourcePtr;

	    if ((resourcePtr =
		 getClientResource(proxyClientID, xPositionStr))
		!= (char *)NULL)
	    {
		pCD->clientX = atoi(resourcePtr);
		pCD->clientFlags |= SM_X;
	    }

	    if ((resourcePtr =
		 getClientResource(proxyClientID, yPositionStr))
		!= (char *)NULL)
	    {
		pCD->clientY = atoi(resourcePtr);
		pCD->clientFlags |= SM_Y;
	    }

#ifndef WSM
	    if ((resourcePtr =
		 getClientResource(proxyClientID, iconXPosStr))
		!= (char *)NULL)
	    {
		ICON_X(pCD) = atoi(resourcePtr);
		pCD->clientFlags |= SM_ICON_X;
	    }

	    if ((resourcePtr =
		 getClientResource(proxyClientID, iconYPosStr))
		!= (char *)NULL)
	    {
		ICON_Y(pCD) = atoi(resourcePtr);
		pCD->clientFlags |= SM_ICON_Y;
	    }
#endif

	    if ((resourcePtr =
		 getClientResource(proxyClientID, widthSizeStr))
		!= (char *)NULL)
	    {
		pCD->clientWidth = atoi(resourcePtr);
		pCD->clientFlags |= SM_WIDTH;
	    }

	    if ((resourcePtr =
		 getClientResource(proxyClientID, heightSizeStr))
		!= (char *)NULL)
	    {
		pCD->clientHeight = atoi(resourcePtr);
		pCD->clientFlags |= SM_HEIGHT;
	    }

	    if ((resourcePtr =
		 getClientResource(proxyClientID, initialStateStr))
		!= (char *)NULL)
	    {
		pCD->clientState =
		    (strcmp(resourcePtr, normalStateStr) == 0) ?
			NORMAL_STATE : MINIMIZED_STATE;
		pCD->clientFlags |= SM_CLIENT_STATE;
	    }

#ifdef WSM
	    if ((workSpaceNamesP != (char **)NULL) &&
		((resourcePtr =
		  getClientResource(proxyClientID, workspacesStr))
		 != (char *)NULL))
	    {
		*workSpaceNamesP = XtNewString(resourcePtr);
	    }
#endif

#ifndef WSM
	    /* This is done in LoadClientIconPositions() if WSM defined. */
	    dbRemoveProxyClientEntry(proxyClientID);
#endif

	    return True;
	}
    }

    return False;
}

/*
 *  Translate the client geometry into what's needed on restore.
 */
static void
getClientGeometry(ClientData *pCD, int *clientX, int *clientY,
		  unsigned int *clientWd, unsigned int *clientHt)
{
    *clientX = pCD->clientX;
    *clientY = pCD->clientY;
    *clientWd = (pCD->widthInc != 0) ?
	(pCD->clientWidth - pCD->baseWidth) / pCD->widthInc :
	    pCD->clientWidth;
    *clientHt = (pCD->heightInc != 0) ?
	(pCD->clientHeight - pCD->baseHeight) / pCD->heightInc :
	    pCD->clientHeight;
}

/*
 *  Assumes: pCD->smClientID is not NULL
 */
static Boolean
saveXSMPClient(FILE *fp, ClientData *pCD)
{
    int clientX, clientY;
    unsigned int clientWd, clientHt;
    char *clientID = pCD->smClientID;

    fprintf(fp, dbClientFormat, XSMPClientStr, clientID, clientID);

    getClientGeometry(pCD, &clientX, &clientY, &clientWd, &clientHt);

    if (SAVE_RESOURCE(pCD, WMSAVE_X))
    {
	fprintf(fp, xPositionStr, clientID);
	fprintf(fp, intArg, clientX);
    }

    if (SAVE_RESOURCE(pCD, WMSAVE_Y))
    {
	fprintf(fp, yPositionStr, clientID);
	fprintf(fp, intArg, clientY);
    }

    if (!pCD->pSD->useIconBox)
    {
#ifdef WSM
	WmScreenData *pSD = pCD->pSD;
	WmWorkspaceData *pWS;
	int i;

	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if ((pWS = GetWorkspaceData(pSD, pCD->pWsList[i].wsID))
		!= (WmWorkspaceData *)NULL)
	    {
		if (SAVE_RESOURCE(pCD, WMSAVE_ICON_X))
		{
		    fprintf(fp, iconXPosStr, clientID, pWS->name);
		    fprintf(fp, intArg, pCD->pWsList[i].iconX);
		}

		if (SAVE_RESOURCE(pCD, WMSAVE_ICON_Y))
		{
		    fprintf(fp, iconYPosStr, clientID, pWS->name);
		    fprintf(fp, intArg, pCD->pWsList[i].iconY);
		}
	    }
	}
#else
	if (SAVE_RESOURCE(pCD, WMSAVE_ICON_X))
	{
	    fprintf(fp, iconXPosStr, clientID);
	    fprintf(fp, intArg, ICON_X(pCD));
	}

	if (SAVE_RESOURCE(pCD, WMSAVE_ICON_Y))
	{
	    fprintf(fp, iconYPosStr, clientID);
	    fprintf(fp, intArg, ICON_Y(pCD));
	}
#endif
    }

    if (SAVE_RESOURCE(pCD, WMSAVE_WIDTH))
    {
	fprintf(fp, widthSizeStr, clientID);
	fprintf(fp, intArg, clientWd);
    }

    if (SAVE_RESOURCE(pCD, WMSAVE_HEIGHT))
    {
	fprintf(fp, heightSizeStr, clientID);
	fprintf(fp, intArg, clientHt);
    }

    if (SAVE_RESOURCE(pCD, WMSAVE_STATE))
    {
	int clientState;

#ifdef WSM
	clientState = pCD->clientState & ~UNSEEN_STATE;
#else
	clientState = pCD->clientState;
#endif

	fprintf(fp, initialStateStr, clientID);
	fprintf(fp, strArg, (clientState == NORMAL_STATE) ?
		normalStateStr : iconicStateStr);
    }

#ifdef WSM
    if (SAVE_RESOURCE(pCD, WMSAVE_WORKSPACES))
    {
	char *clientWorkspaces = getClientWorkspaces(pCD);

	if (clientWorkspaces != (char *)NULL)
	{
	    fprintf(fp, workspacesStr, clientID);
	    fprintf(fp, strArg, clientWorkspaces);
	    XtFree(clientWorkspaces);
	}
    }
#endif

    return True;
}

/*
 *  Assumes: pCD->smClientID is NULL
 */
static Boolean
saveProxyClient(FILE *fp, ClientData *pCD, int clientIDNum)
{
    char clientID[50];
    int clientState;
    ProxyClientInfo proxyClientInfo;
    int clientX, clientY;
    unsigned int clientWd, clientHt;
#ifdef WSM
    char *clientWorkspaces;
#endif

    if (!getProxyClientInfo(pCD, &proxyClientInfo))
	return False;

    sprintf(clientID, "%d", clientIDNum);
    fprintf(fp, dbClientFormat, proxyClientStr, clientID, clientID);

    fprintf(fp, screenStr, clientID);
    fprintf(fp, intArg, proxyClientInfo.screen);

    fprintf(fp, wmCommandStr, clientID);
    fprintf(fp, strArg, proxyClientInfo.wmCommand);
    free(proxyClientInfo.wmCommand);

    if (proxyClientInfo.wmClientMachine != (char *)NULL)
    {
	fprintf(fp, wmClientMachineStr, clientID);
	fprintf(fp, strArg, proxyClientInfo.wmClientMachine);
	free(proxyClientInfo.wmClientMachine);
    }

    getClientGeometry(pCD, &clientX, &clientY, &clientWd, &clientHt);

    fprintf(fp, xPositionStr, clientID);
    fprintf(fp, intArg, clientX);

    fprintf(fp, yPositionStr, clientID);
    fprintf(fp, intArg, clientY);

    if (!pCD->pSD->useIconBox)
    {
#ifdef WSM
	WmScreenData *pSD = pCD->pSD;
	WmWorkspaceData *pWS;
	int i;

	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if ((pWS = GetWorkspaceData(pSD, pCD->pWsList[i].wsID))
		!= (WmWorkspaceData *)NULL)
	    {
		fprintf(fp, iconXPosStr, clientID, pWS->name);
		fprintf(fp, intArg, pCD->pWsList[i].iconX);

		fprintf(fp, iconYPosStr, clientID, pWS->name);
		fprintf(fp, intArg, pCD->pWsList[i].iconY);
	    }
	}
#else
	fprintf(fp, iconXPosStr, clientID);
	fprintf(fp, intArg, ICON_X(pCD));

	fprintf(fp, iconYPosStr, clientID);
	fprintf(fp, intArg, ICON_Y(pCD));
#endif
    }

    fprintf(fp, widthSizeStr, clientID);
    fprintf(fp, intArg, clientWd);

    fprintf(fp, heightSizeStr, clientID);
    fprintf(fp, intArg, clientHt);

#ifdef WSM
    clientState = pCD->clientState & ~UNSEEN_STATE;
#else
    clientState = pCD->clientState;
#endif

    fprintf(fp, initialStateStr, clientID);
    fprintf(fp, strArg, (clientState == NORMAL_STATE) ?
	    normalStateStr : iconicStateStr);

#ifdef WSM
    clientWorkspaces = getClientWorkspaces(pCD);
    if (clientWorkspaces != (char *)NULL)
    {
	fprintf(fp, workspacesStr, clientID);
	fprintf(fp, strArg, clientWorkspaces);
	XtFree(clientWorkspaces);
    }
#endif

    return True;
}

static void
dbRemoveProxyClientEntry(char *proxyClientID)
{
    char resourceBuf[MAX_RESOURCE_LEN];

    /* Remove entry from DB.  Since Xrm does not provide a means */
    /* of removing something from the DB, we blank out key info. */
    sprintf(resourceBuf, wmCommandStr, proxyClientID);
    strcat(resourceBuf, ":");
    XrmPutLineResource(&wmGD.clientResourceDB, resourceBuf);
}

/*
 *  Add callbacks used in session management.
 */
void
AddSMCallbacks(void)
{
    XtAddCallback(wmGD.topLevelW, XtNsaveCallback,
		  smSaveYourselfCallback, (XtPointer)NULL);
    XtAddCallback(wmGD.topLevelW, XtNdieCallback,
		  smDieCallback, (XtPointer)NULL);
}

/*
 *  Resign from session management, closing any connections made.
 */
void
ResignFromSM(void)
{
    if (wmGD.topLevelW)
    {
	XtVaSetValues(wmGD.topLevelW,
		      XtNjoinSession, False,
		      NULL);
    }
}

/*
 *  Exit the WM, being polite by first resigning from session mgmt.
 */
void
ExitWM(int exitCode)
{
    ResignFromSM();
    exit(exitCode);
}

/*
 *  Read our private database of client resources.
 */
XrmDatabase
LoadClientResourceDB(void)
{
    char dbFileName[MAXPATHLEN];

#ifndef WSM
    getClientDBName();
#endif
    buildDBFileName(dbFileName, False);

    return XrmGetFileDatabase(dbFileName);
}

/*
 *  Write our private database of client resources.
 */
XrmDatabase
SaveClientResourceDB(void)
{
    String mySessionID;
    char dbFileName[MAXPATHLEN];
    FILE *fp;
    int scr;
    WmScreenData *pSD;
    ClientData *pCD;
    int clientIDNum = 0;
    ClientListEntry *pCL;

    /* Iterate through client list, saving */
    /* appropriate resources for each. */
#ifndef WSM
    setClientDBName();
#endif
    buildDBFileName(dbFileName, True);
    if ((fp = fopen(dbFileName, "w")) == (FILE *)NULL)
	return (XrmDatabase)NULL;

    XtVaGetValues(wmGD.topLevelW,
		  XtNsessionID, &mySessionID,
		  NULL);
    fprintf(fp, dbHeader, dtwmFileName, "dtwm Version XSMP1.0",
	    (mySessionID != (String)NULL) ? mySessionID : "");

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

	for (pCL = pSD->clientList;
	     pCL != (ClientListEntry *)NULL;
	     pCL = pCL->nextSibling)
	{
	    /* Each client may be in list twice: normal & icon */
	    if (pCL->type != NORMAL_STATE)
		continue;

	    pCD = pCL->pCD;

	    if (pCD->smClientID != (String)NULL)
	    {
		saveXSMPClient(fp, pCD);
	    }
	    else
	    {
		if (saveProxyClient(fp, pCD, clientIDNum))
		    clientIDNum++;
	    }
	}
    }

    fclose(fp);

    /* Retrieve database from file. */
    return XrmGetFileDatabase(dbFileName);
}

/*
 *  As with FindDtSessionMatch(), sets properties and then returns
 *  an allocated string of workspace names.  This string must be
 *  freed by the caller using XtFree().
 */
Boolean
FindClientDBMatch(ClientData *pCD, char **workSpaceNamesP)
{
    return (findXSMPClientDBMatch(pCD, workSpaceNamesP) ||
	    findProxyClientDBMatch(pCD, workSpaceNamesP));
}

Boolean
GetSmClientIdClientList(ClientData ***clients, int *nClients)
{
    int scr;
    WmScreenData *pSD;
    ClientData *pCD;
    ClientListEntry *pCL;

    *nClients = 0;
    *clients = (ClientData **)NULL;
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

	for (pCL = pSD->clientList;
	     pCL != (ClientListEntry *)NULL;
	     pCL = pCL->nextSibling)
	{
	    /* Each client may be in list twice: normal & icon */
	    if (pCL->type != NORMAL_STATE)
		continue;

	    pCD = pCL->pCD;

	    if (pCD->smClientID != (String)NULL)
	    {
		/* addClientToList() reclaims memory on failure. */
		if (!addClientToList(clients, nClients, pCD))
		    return False;
	    }
	}
    }

    return True;
}

void
SortClientListByWorkspace(ClientData **clients, int nClients)
{
    if (nClients > 0)
    {
	qsort((void *)clients, nClients,
	      sizeof(ClientData *), clientWorkspaceCompare);
    }
}

#ifdef WSM
/* This needs to be called if WSM defined; if WSM not defined, icon */
/* positions are read at the same time as other resources. */
void
LoadClientIconPositions(ClientData *pCD)
{
    char resourceBuf[MAX_RESOURCE_LEN];
    WmScreenData *pSD = pCD->pSD;
    WmWorkspaceData *pWS;
    int i;
    char *resourcePtr;

    if (wmGD.clientResourceDB == (XrmDatabase)NULL)
	return;

    if (pCD->smClientID != (String)NULL)
    {
	for (i = 0; i < pCD->numInhabited; i++)
	{
	    if ((pWS = GetWorkspaceData(pSD, pCD->pWsList[i].wsID))
		!= (WmWorkspaceData *)NULL)
	    {
		sprintf(resourceBuf, iconXPosStr, "%s", pWS->name);
		if ((resourcePtr =
		     getXSMPResource(pCD, WMSAVE_ICON_X, resourceBuf))
		    != (char *)NULL)
		{
		    pCD->pWsList[i].iconX = atoi(resourcePtr);
		    pCD->clientFlags |= SM_ICON_X;
		}

		sprintf(resourceBuf, iconYPosStr, "%s", pWS->name);
		if ((resourcePtr =
		     getXSMPResource(pCD, WMSAVE_ICON_Y, resourceBuf))
		    != (char *)NULL)
		{
		    pCD->pWsList[i].iconY = atoi(resourcePtr);
		    pCD->clientFlags |= SM_ICON_Y;
		}
	    }
	}
	return;
    }

    /* Proxy client */
    if (!smClientDBCheckpointed)
    {
	char *proxyClientID;

	if ((proxyClientID = findProxyClientID(pCD)) != (char *)NULL)
	{
	    for (i = 0; i < pCD->numInhabited; i++)
	    {
		if ((pWS = GetWorkspaceData(pSD, pCD->pWsList[i].wsID))
		    != (WmWorkspaceData *)NULL)
		{
		    sprintf(resourceBuf, iconXPosStr, "%s", pWS->name);
		    if ((resourcePtr =
			 getClientResource(proxyClientID, resourceBuf))
			!= (char *)NULL)
		    {
			pCD->pWsList[i].iconX = atoi(resourcePtr);
			pCD->clientFlags |= SM_ICON_X;
		    }

		    sprintf(resourceBuf, iconYPosStr, "%s", pWS->name);
		    if ((resourcePtr =
			 getClientResource(proxyClientID, resourceBuf))
			!= (char *)NULL)
		    {
			pCD->pWsList[i].iconY = atoi(resourcePtr);
			pCD->clientFlags |= SM_ICON_Y;
		    }
		}
	    }
	    dbRemoveProxyClientEntry(proxyClientID);
	}
    }
}
#endif /* WSM */
