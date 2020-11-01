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
/*   $XConsortium: WmProtocol.h /main/4 1995/11/01 11:49:37 rswiston $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

extern void SetupWmICCC (void);
extern void SendConfigureNotify (ClientData *pCD);
extern void SendClientOffsetMessage (ClientData *pCD);
extern void SendClientMsg (Window window, long type, long data0, Time time, long *pData, int dataLen);
extern Boolean AddWmTimer (unsigned int timerType, unsigned long timerInterval, ClientData *pCD);
extern void DeleteClientWmTimers (ClientData *pCD);
extern void TimeoutProc (caddr_t client_data, XtIntervalId *id);
