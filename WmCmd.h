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
/* $TOG: WmCmd.h /main/8 1997/03/20 11:23:06 dbl $
 *
 * (c) Copyright 1996 Digital Equipment Corporation.
 * (c) Copyright 1996 Hewlett-Packard Company.
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc. 
 * (c) Copyright 1996 FUJITSU LIMITED.
 * (c) Copyright 1996 Hitachi.
 */

#ifndef _WM_CMD_
#define _WM_CMD_

#include <Xm/Xm.h>
#include "WmWsmLib/wsm_proto.h"

#define WINDOW_MASK 0x3fffffff
#define ALL_WINDOWS 0x3fffffff
#define ICON_ONLY   ((unsigned int)1)<<31
#define WINDOW_ONLY 1<<30


extern void    DefineCommand       (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    IncludeCommand      (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    EnableCommand       (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    DisableCommand      (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    RenameCommand       (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    RemoveCommand       (Widget, Atom, MessageData,
				    unsigned long, int);
extern void    AddWindowMenuEntry  (ClientData *);
extern void    DeleteCommand       (long, CmdTree **);
extern void    SendInvokeMessage   (CARD32, CARD32, Atom, Time);


#endif /* _WM_CMD_ */
