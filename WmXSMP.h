#ifndef _WmXSMP_h
#define _WmXSMP_h

/* $XConsortium: WmXSMP.h /main/4 1996/04/19 11:22:40 rswiston $ */
/*
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#include <X11/Intrinsic.h>
#include "WmGlobal.h"

/* Atoms used for session management capabilities. */
#define _XA_DT_SM_CLIENT_ID		"SM_CLIENT_ID"
#define _XA_DT_WMSAVE_HINT		"_DT_WMSAVE_HINT"

/* _DT_WMSAVE_HINT flag definitions */
#define WMSAVE_X		(1L << 0)
#define WMSAVE_Y		(1L << 1)
#define WMSAVE_WIDTH		(1L << 2)
#define WMSAVE_HEIGHT		(1L << 3)
#define WMSAVE_STATE		(1L << 4)
#ifdef WSM
# define WMSAVE_WORKSPACES	(1L << 5)
#endif
#define WMSAVE_ICON_X		(1L << 6)
#define WMSAVE_ICON_Y		(1L << 7)

extern void AddSMCallbacks(void);
extern void ResignFromSM(void);
extern void ExitWM(int exitCode);
extern XrmDatabase LoadClientResourceDB(void);
extern XrmDatabase SaveClientResourceDB(void);
extern Boolean FindClientDBMatch(ClientData *, char **);
extern Boolean GetSmClientIdClientList(ClientData ***, int *);
extern void SortClientListByWorkspace(ClientData **, int);
#ifdef WSM
extern void LoadClientIconPositions(ClientData *);
#endif

#endif /* _WmXSMP_h */
