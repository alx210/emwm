/* $XConsortium: wsm_funcs.h /main/5 1995/07/15 20:39:08 drk $ */
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
 * 
 */
/*
 * HISTORY
 */

#ifndef _WSM_FUNCS_
#define _WSM_FUNCS_

/************************************************************
 *
 *  All function Prototypes for non static functions.
 *
 * Those that begin with _WSM are "private" for whatever that
 * is worth...
 *
 ************************************************************/

/*
 * disp.c
 */

WSMDispInfo * _WSMGetDispInfo(
Display *
);

WSMScreenInfo * _WSMGetScreenInfo(
Display *, int
);


void _WSMClearConfigScreenInfo(
Display *, int
);

WSMConfigFormatData * _WSMGetConfigFormat(
Display *, int, WSMConfigFormatType
);

Atom _WSMGetSelectionAtom(
Display *, int, WSMClientType
);

/*
 * free.c
 */

void FreeRequest(
WSMRequest *request
);

void FreeReply(
WSMReply *reply
);

/*
 * pack.c
 */

MessageData _WSMPackRequest(
Display *, int, WSMRequest *, unsigned long *, WSMErrorCode *
);

MessageData _WSMPackReply(
Display *, int, WSMReply *, unsigned long *
);

void _WSMUnpackRequest(
Display *, int, MessageData, unsigned long, WSMRequestType, WSMRequest *
);

void _WSMUnpackReply(
Display *, int, MessageData, unsigned long, WSMRequestType, WSMReply *
);

MessageData PackString(
MessageData, String
);

String UnpackString(
MessageData *
);

MessageData PackCARD8(
MessageData, CARD8
);

CARD8 UnpackCARD8(
MessageData *
);

MessageData PackCARD16(
MessageData, CARD16
);

CARD16 UnpackCARD16(
MessageData *
);

MessageData PackCARD32(
MessageData, CARD32
);

CARD32 UnpackCARD32(
MessageData *
);

/*
 * recv.c
 */

Boolean WSMDefaultOwnSelection(
Widget, WSMClientType, WSMRequestCallbackFunc, XtPointer
);

void WSMRegisterRequestCallback(
Display *, int, WSMRequestCallbackFunc, XtPointer
);

Boolean WSMIsKnownTarget(
Widget, Atom
);

Atom * GetTargetList(
Widget, Boolean, unsigned long *
);

Boolean WSMProcessProtoTarget(
Widget, Atom, XtPointer, unsigned long, 
int, Atom *, XtPointer *, unsigned long *, int *
);

/*
 * send.c
 */

Boolean WSMSendMessage(
Widget, WSMClientType, WSMRequest *, WSMReplyCallbackFunc, XtPointer
);

/*
 * utils.c
 */

String _WSMReqTypeToName(
WSMRequestType
);

WSMRequestType _WSMTargetToReqType(
Display *, Atom
);

Atom _WSMReqTypeToTarget(
Display *, WSMRequestType
);

Boolean _WSMRequiresConfigFormat(
WSMRequestType
);

WSMConfigFormatType _WSMGetConfigFormatType(
Window
);

WSMAttribute * _WSMGetMatchingAttr(
XrmQuark, WSMConfigFormatData *
);

WSMWinData * _WSMGetMatchingWinData(
WSMWinData *, int, XrmQuark
);

#endif /* DO NOT ADD ANYTHING AFTER THIS LINE */
