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
#ifndef _WmXSMP_h
#define _WmXSMP_h

#include <X11/Intrinsic.h>
#include "WmGlobal.h"

/* Atoms used for session management capabilities. */
#define _XA_MWM_SM_CLIENT_ID		"SM_CLIENT_ID"
#define _XA_MWM_WMSAVE_HINT		"_MWM_WMSAVE_HINT"

/* _DT_WMSAVE_HINT flag definitions */
#define WMSAVE_X		(1L << 0)
#define WMSAVE_Y		(1L << 1)
#define WMSAVE_WIDTH		(1L << 2)
#define WMSAVE_HEIGHT		(1L << 3)
#define WMSAVE_STATE		(1L << 4)
#define WMSAVE_WORKSPACES	(1L << 5)
#define WMSAVE_ICON_X		(1L << 6)
#define WMSAVE_ICON_Y		(1L << 7)

void AddSMCallbacks(void);
void ResignFromSM(void);
_X_NORETURN void ExitWM(int exitCode);
XrmDatabase LoadClientResourceDB(void);
XrmDatabase SaveClientResourceDB(void);
Boolean FindClientDBMatch(ClientData *, char **);
Boolean GetSmClientIdClientList(ClientData ***, int *);
void SortClientListByWorkspace(ClientData **, int);
void LoadClientIconPositions(ClientData *);


#endif /* _WmXSMP_h */
