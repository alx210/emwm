/* $XConsortium: debug.c /main/5 1995/07/15 20:38:27 drk $ */
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

static void PrintWindowInfo(
     String,
     WSMWinInfo *
);
static void PrintWindowData(
     String,
     String,
     WSMWinData *
);
static void PrintConfigFormat(
     String,
     WSMConfigFormatReply *
);

#define STRSIZE  	(BUFSIZ * 10)

/************************************************************
 *
 *  This file contains routines to ease in debugging.
 *
 ************************************************************/

/*	Function Name: PrintRequest
 *	Description: Prints out a request in human readable form.
 *	Arguments: request - The request to print.
 *	Returns: str - The string the request is printed to.
 */

String
PrintRequest(WSMRequest *request)
{
    static char str[STRSIZE]; /* The vile hacks we do in debugging code. */
    String ptr;
    register int i;

    sprintf(str, "Request: %s\n", _WSMReqTypeToName(request->any.type));
    ptr = str + strlen(str);

    switch (request->any.type) {
    case WSM_CONNECT:
	sprintf(ptr, "Number of Versions: %d\nVersions: ",
		request->connect.num_versions);
	for (i = 0; i < request->connect.num_versions; i++) {
	    ptr += strlen(ptr);
	    sprintf(ptr, "%d", (int) request->connect.known_versions[i]);
	    if ((i + 1) < request->connect.num_versions)
		strcat(ptr, ", ");
	}
	strcat(ptr, "\n");
	break;
    case WSM_EXTENSIONS:
	sprintf(ptr, "Number of Extension Suggestions: %d\n%s",
		request->extensions.num_extensions,
		"Extension Suggestions:\n        ");
	for (i = 0; i < request->extensions.num_extensions; i++) {
	    ptr += strlen(ptr);
	    strcat(ptr, request->extensions.extension_suggestions[i]);
	    if ((i + 1) < request->extensions.num_extensions)
		strcat(ptr, ", ");
	}
	strcat(ptr, "\n");
	break;	
    case WSM_CONFIG_FMT:
	sprintf(ptr, "No other data for this request.\n");
	break;
    case WSM_GET_STATE:
	sprintf(ptr, "Window: 0x%lX\nDiffs Allowed: %s\n", 
		request->get_state.window, 
		request->get_state.diffs_allowed ? "True" : "False");
	break;
    case WSM_SET_STATE:
	sprintf(ptr, "Number of Windows with Info: %d\n", 
		request->set_state.num_win_info_list);
	for (i = 0; i < request->set_state.num_win_info_list; i++) {
	    ptr += strlen(ptr);
	    PrintWindowInfo(ptr, request->set_state.win_info_list + i);
	}
	break;
    case WSM_REG_WINDOW:
	sprintf(ptr, "Window: 0x%lX\n", request->register_window.window);
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
    case WSM_WM_SET_BACKGROUND_WINDOW:
    case WSM_WM_WINDOWS:
    case WSM_WM_FOCUS:
    case WSM_WM_POINTER:
    case WSM_UNKNOWN:
	break;
    }

    return(str);
}

/*	Function Name: PrintReply
 *	Description: Prints out a reply in human readable form.
 *	Arguments: reply - The reply to print.
 *	Returns: str - The string the reply is printed to.
 */

String
PrintReply(WSMReply *reply)
{
    static char str[STRSIZE]; /* The vile hacks we do in debugging code. */
    register int i;
    String ptr;

    sprintf(str, "Reply: %s\n", _WSMReqTypeToName(reply->any.type));
    ptr = str + strlen(str);

    switch (reply->any.type) {
    case WSM_CONNECT:
	sprintf(ptr, "Version: %d\n", (int) reply->connect.version);
	break;
    case WSM_EXTENSIONS:
	sprintf(ptr, "Number of Extensions: %d\n%s",
		reply->extensions.num_extensions,
		"Extensions:\n        ");
	for (i = 0; i < reply->extensions.num_extensions; i++) {
	    ptr += strlen(ptr);
	    strcat(ptr, reply->extensions.extensions[i]);
	    if ((i + 1) < reply->extensions.num_extensions)
		strcat(ptr, ", ");
	}
	strcat(ptr, "\n");
	break;	
    case WSM_CONFIG_FMT:
	PrintConfigFormat(ptr, &(reply->config_format));
	break;
    case WSM_GET_STATE:
	sprintf(ptr, "Number of Windows with Info: %d\n", 
		reply->get_state.num_win_info_list);
	for (i = 0; i < reply->get_state.num_win_info_list; i++) {
	    ptr += strlen(ptr);
	    PrintWindowInfo(ptr, reply->get_state.win_info_list + i);
	}
	break;
    case WSM_SET_STATE:
	sprintf(ptr, "No other data for this reply.\n");
	break;
    case WSM_REG_WINDOW:
        {
	    int num = reply->register_window.num_window_data;
	    WSMWinData *win_data = reply->register_window.window_data;
    
	    sprintf(ptr, "Number of Data Attributes: %d\n", num);
	    for (i = 0; i < num; i++, win_data++) {
		ptr += strlen(ptr);
		PrintWindowData(ptr, "", win_data);
	    }
	}
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
    case WSM_WM_SET_BACKGROUND_WINDOW:
    case WSM_WM_WINDOWS:
    case WSM_WM_FOCUS:
    case WSM_WM_POINTER:
    case WSM_UNKNOWN:
	break;
    }

    return(str);
}

/************************************************************
 *
 *  Internal Routines.
 *
 ************************************************************/

/*	Function Name:  PrintWindowInfo
 *	Description: Prints the info about a single window.
 *	Arguments: str - String to stuff it into.
 *                 win_info - The window info to print.
 *	Returns: none.
 */

static void
PrintWindowInfo(
     String      str,
     WSMWinInfo *win_info)
{
    register int i;
    int num = win_info->num_data_list;
    WSMWinData *win_data = win_info->data_list;
    
    sprintf(str, "Window: 0x%lX\nNumber of Data Attributes: %d\n", 
	    win_info->window, num);

    for (i = 0; i < num; i++, win_data++) {
	str += strlen(str);
	PrintWindowData(str, "       ", win_data);
    }
}

/*	Function Name:  PrintWindowData
 *	Description: Prints the data about a single window.
 *	Arguments: str - String to stuff it into.
 *                 win_data - The window data to print.
 *	Returns: none.
 */

static void
PrintWindowData(
     String      str,
     String      tab_str,
     WSMWinData *win_data)
{
    register int i;
    char *type_str;

    if (win_data->type == WSM_VALUE_DATA) {
	sprintf(str, "%sName: %s, Type: Value, Value: %ld\n", tab_str,
		XrmQuarkToString(win_data->nameq), win_data->data.value);
	return;
    }

	
    switch(win_data->type) {
    case WSM_CHAR_LIST_DATA:
	type_str = "Char List";
	break;
    case WSM_SHORT_LIST_DATA:
	type_str = "Short List";
	break;
    case WSM_LONG_LIST_DATA:
	type_str = "Long List";
	break;
    case WSM_VALUE_DATA:
    case WSM_NONE:
	type_str = "<NONE>";
	break;
    }
    
    sprintf(str, "%sName: %s, Type: %s, Len: %d\n%s%s", tab_str,
	    XrmQuarkToString(win_data->nameq), type_str,
	    win_data->data_len, tab_str, "        ");
    
    for (i = 0; i < win_data->data_len; i++) {
	str += strlen(str);
	
	switch(win_data->type) {
	case WSM_CHAR_LIST_DATA:
	    sprintf(str, "%c(%d)", win_data->data.char_ptr[i],
		    (int) win_data->data.char_ptr[i]);
	    break;
	case WSM_SHORT_LIST_DATA:
	    sprintf(str, "%d", (int) win_data->data.short_ptr[i]);
	    break;
	case WSM_LONG_LIST_DATA:
	    sprintf(str, "%ld", win_data->data.long_ptr[i]);
	    break;
	case WSM_VALUE_DATA:
	case WSM_NONE:
	    break;
	}
	if (i < (win_data->data_len - 1))
	    strcat(str, ", ");
    }
    
    strcat(str, "\n");
}

/*	Function Name: PrintConfigFormat
 *	Description: Prints the configuration format.
 *	Arguments: str - The string to print the conf info into.
 *                 config_fmt  - The configuration format reply.
 *	Returns: none
 */

static void
PrintConfigFormat(
     String                str,
     WSMConfigFormatReply *config_fmt)
{
    register int i;

    sprintf("Accepts Diffs: %s\n",
	    (config_fmt->accepts_diffs ? "True" : "False"));

    for (i = 0; i < 3; i++) {
	WSMAttribute *fmt;
	register int j, num;

	str += strlen(str);

	switch (i) {
	case WSM_GLOBAL_FMT:
	    fmt = config_fmt->global_formats;
	    num = config_fmt->num_global_formats;
	    sprintf(str, "Global Formats - Number: %d\n", num);
	    break;
	case WSM_WINDOW_FMT:
	    fmt = config_fmt->window_formats;
	    num = config_fmt->num_window_formats;
	    sprintf(str, "Window Formats - Number: %d\n", num);
	    break;
	case WSM_ICON_FMT:
	    fmt = config_fmt->icon_formats;
	    num = config_fmt->num_icon_formats;
	    sprintf(str, "Icon Formats - Number: %d\n", num);
	    break;
	}

	for (j = 0; j < num; j++) {
	    str += strlen(str);
	    sprintf(str, "        Name: %s, Size: %d, Is_List: %s\n",
		    XrmQuarkToString(fmt[j].nameq), (int) fmt[j].size, 
		    fmt[j].is_list ? "True" : "False");
	}
    }
}
