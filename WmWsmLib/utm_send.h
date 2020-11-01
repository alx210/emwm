/* $XConsortium: utm_send.h /main/5 1995/07/15 20:39:04 drk $ */
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

/*
 * This is used to pass the data from the request
 * setup routine to the callback routine where the
 * data is actually transferred.
 */
typedef struct _UTMPackageRec {
  int fmt;
  Atom target;
  unsigned long len;
  XtPointer param;
  XtCallbackProc doneProc;
  XtPointer closure;
} UTMPackageRec;


extern void UTMSendMessage(
Widget, Atom, Atom, XtPointer, unsigned long, int,
XtCallbackProc, XtPointer, Time
);

extern void UTMDestinationProc (
Widget, XtPointer, XtPointer
);
