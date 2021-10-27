/* $XConsortium: free.c /main/5 1995/07/15 20:38:35 drk $ */
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

#include "wsm_proto.h"
#include <X11/Xatom.h>

static void FreeWinData(
WSMWinData *data, int num
);

/*	Function Name: FreeRequest
 *	Description: Frees data in the request that has been allocated.
 *	Arguments: request - the request.
 *	Returns: none.
 */

void
FreeRequest(WSMRequest *request)
{
    register int i;

    if (!request->any.allocated)
	return;			/* Nothing to free. */

    switch(request->any.type) {
    case WSM_CONNECT:
	XtFree((XtPointer) request->connect.known_versions);
	break;
    case WSM_EXTENSIONS:
        {
	    register int num = request->extensions.num_extensions;
	    register String *ptr;

	    num = request->extensions.num_extensions;
	    ptr = request->extensions.extension_suggestions;
	    for (i = 0; i < num; i++, ptr++) 
		XtFree((XtPointer) *ptr);

	    XtFree((XtPointer) request->extensions.extension_suggestions);
	}
	break;
    case WSM_SET_STATE:
        {
	    register int num = request->set_state.num_win_info_list;
	    WSMWinInfo *win_info = request->set_state.win_info_list;

	    for (i = 0; i < num; i++, win_info++) 
		FreeWinData(win_info->data_list, win_info->num_data_list);

	    XtFree((XtPointer) request->set_state.win_info_list);
	}
	break;
    default:
	break;
    }
}

/*	Function Name: FreeRequest
 *	Description: Frees data in the request that has been allocated.
 *	Arguments: reply - the reply.
 *	Returns: none.
 */

void
FreeReply(WSMReply *reply)
{
    register int i;

    if (!reply->any.allocated)
	return;			/* Nothing to free. */

    switch(reply->any.type) {
    case WSM_EXTENSIONS:
        {
	    register int num = reply->extensions.num_extensions;
	    register String *ptr;

	    num = reply->extensions.num_extensions;
	    ptr = reply->extensions.extensions;
	    for (i = 0; i < num; i++, ptr++) 
		XtFree((XtPointer) *ptr);

	    XtFree((XtPointer) reply->extensions.extensions);
	}
	break;
    case WSM_GET_STATE:
        {
	    WSMWinInfo *win_info = reply->get_state.win_info_list;
	    register int num = reply->get_state.num_win_info_list;

	    for (i = 0; i < num; i++, win_info++) 
		FreeWinData(win_info->data_list, win_info->num_data_list);

	    XtFree((XtPointer) reply->get_state.win_info_list);
	}
	break;
    case WSM_REG_WINDOW:
	FreeWinData(reply->register_window.window_data,
		    reply->register_window.num_window_data);
	break;
    default:
	break;
    }
}

/************************************************************
 *
 *  Internal routines
 *
 ************************************************************/

/*	Function Name: FreeWinData
 *	Description: Frees the data associated with this window.
 *	Arguments: data - The data to free.
 *                 num - number of elements in data.
 *	Returns: none.
 */

static void
FreeWinData(WSMWinData *data, int num)
{
    register int i;
    WSMWinData *top = data;

    for (i = 0; i < num; i++, data++) {
	switch (data->type) {
	case WSM_CHAR_LIST_DATA:
	    XtFree((XtPointer) data->data.char_ptr);
	    break;
	case WSM_SHORT_LIST_DATA: 
	    XtFree((XtPointer) data->data.short_ptr);
	    break;
	case WSM_LONG_LIST_DATA:
	    XtFree((XtPointer) data->data.long_ptr);
	    break;
	default:
	    break;
	}
    }
    
    XtFree((XtPointer) top);
}	
