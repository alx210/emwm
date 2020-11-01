/* $XConsortium: wsm_proto.h /main/5 1995/07/15 20:39:12 drk $ */
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

#ifndef _WSM_PROTO_
#define _WSM_PROTO_

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xmd.h>
#include <Xm/Xm.h>

#define WSM_PROTO_FMT			8

/************************************************************
 *
 *  Defines
 *
 ************************************************************/

#define WSM_NAME_CONNECT 			"_MOTIF_WSM_CONNECT"
#define WSM_NAME_EXTENSIONS 			"_MOTIF_WM_EXTENSIONS"
#define WSM_NAME_CONFIG_FMT 			"_MOTIF_WM_CONFIG_FORMAT"
#define WSM_NAME_GET_STATE 			"_MOTIF_WSM_GET_STATE"
#define WSM_NAME_SET_STATE 			"_MOTIF_WSM_SET_STATE"
#define WSM_NAME_REG_WINDOW 			"_MOTIF_WSM_REGISTER_WINDOW"
#define WSM_NAME_WM_GET_BACKGROUND_WINDOW 	"_MOTIF_WSM_GET_BACKGROUND_WINDOW"
#define WSM_NAME_WM_SET_BACKGROUND_WINDOW 	"_MOTIF_WSM_SET_BACKGROUND_WINDOW"
#define WSM_NAME_WM_WINDOWS 			"_MOTIF_WSM_WM_WINDOWS"
#define WSM_NAME_WM_FOCUS 			"_MOTIF_WSM_WM_FOCUS"
#define WSM_NAME_WM_POINTER 			"_MOTIF_WSM_WM_POINTER"

#define NUM_EXTRA_TARGETS 		3
#define NUM_WSM_TARGETS 		11

#define WM_SELECTION_FORMAT		"WM_S%d"
#define WSM_SELECTION_FORMAT		"WSM_S%d"

#define WIN_MASK			0x1FFFFFFF

/************************************************************
 *
 *  New enumerated types.
 *
 ************************************************************/

typedef enum {WSM_CONNECT, WSM_EXTENSIONS, WSM_CONFIG_FMT, WSM_GET_STATE,
	      WSM_SET_STATE, WSM_REG_WINDOW,
	      WSM_WM_GET_BACKGROUND_WINDOW, WSM_WM_SET_BACKGROUND_WINDOW,
	      WSM_WM_WINDOWS, WSM_WM_FOCUS, WSM_WM_POINTER,
	      WSM_UNKNOWN} WSMRequestType;

typedef enum {WSM_WORKSPACE_MANAGER, WSM_WINDOW_MANAGER} WSMClientType;

typedef enum {WSM_GLOBAL_FMT = 0, 
	      WSM_WINDOW_FMT = 1, WSM_ICON_FMT = 2} WSMConfigFormatType;

typedef enum {WSM_VALUE_DATA, WSM_CHAR_LIST_DATA, 
	      WSM_SHORT_LIST_DATA, WSM_LONG_LIST_DATA, WSM_NONE} WSMDataType;

typedef enum {WSM_SUCCESS, WSM_ERROR_INTERNAL, WSM_ERROR_CONVERSION_FAILED,
	      WSM_ERROR_NO_SEL_OWNER, WSM_ERROR_TIMEOUT,
	      WSM_ERROR_NO_CONFIG_FORMAT,
	      WSM_ERROR_ALREADY_HAVE_CONFIG_FORMAT } WSMErrorCode;

typedef enum {WSM_CLIENT_WINDOW, WSM_CLIENT_FRAME, WSM_ICON_FRAME} WSMLocationFlag;

typedef XtPointer MessageData;

/************************************************************
 *
 * Global structures that are used by calling routines.
 *
 ************************************************************/

typedef union _WSMData {
    long value;
    char * char_ptr;
    short * short_ptr;
    long * long_ptr;
} WSMData;

typedef struct _WSMAttribute {
    XrmQuark nameq;		/* Name of this attribute, quarkified. */
    int size;			/* The size of this attribute. */
    Boolean is_list;		/* True if this attribute is a list. */
} WSMAttribute;

typedef struct _WSMConfigFormatData {
    WSMAttribute *attr_list;
    int num_attrs;
} WSMConfigFormatData;

typedef struct _WSMWinData {
    XrmQuark nameq;		/* Name of this data entry, quarkified. */
    WSMDataType type;		/* The type of data stored in data. */
    WSMData  data;		/* The data associated with this data entry. */
    int data_len;		/* if a list, then the size. */
} WSMWinData;

typedef struct _WSMWinInfo {
    Window window;
    WSMWinData * data_list;
    int num_data_list;
} WSMWinInfo;

typedef Atom WindowProperty;

typedef struct _WSMWinEntry {
    Window         *windows;           /* one for each location flag set. */
    int             num_windows;
    WindowProperty *match_properties;  /* properties the window matched.  */
    int             num_match_properties;
} WSMWinEntry;

typedef struct _AttributePair {
    WindowProperty *allowed_attributes;
    WindowProperty *prohibited_attributes;
    int             num_attributes;
} AttributePair;

/************************************************************
 *
 * Structures for sending a message for each type of event
 *
 ************************************************************/

typedef struct _WSMAnyRequestOrReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean allocated;		/* Packlib allocated the data, do not move. */
} WSMAnyRequestOrReply;

typedef struct _WSMConnect {
    WSMRequestType type;	/* Type of event, do not move or change. */

    /* 
     * Packlib allocated the data, do not move. 
     * If True will free the known_versions ptr. 
     */

    Boolean allocated;
    short * known_versions;
    int num_versions;
} WSMConnect;

typedef struct _WSMExtensions {
    WSMRequestType type;	/* Type of event, do not move or change. */
    
    /*
     * Packlib allocated the data, do not move. 
     * If True will free each string on the extension_suggestions list
     * as well as the extension_suggestions ptr itself.
     */

    Boolean allocated;		
    String *extension_suggestions;
    int num_extensions;
} WSMExtensions;

typedef WSMAnyRequestOrReply WSMConfigFormat;

typedef struct _WSMGetState {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean allocated;		/* Packlib allocated the data, do not move. */
    Window window;
    Boolean diffs_allowed;
} WSMGetState;

typedef struct _WSMSetState {
    WSMRequestType type;	/* Type of event, do not move or change. */

    /*
     * Packlib allocated the data, do not move. 
     * If True will free the win_data list in each element on the 
     * win_info_list. as well as the win_info_list ptr itself.
     */

    Boolean allocated;		
    WSMWinInfo *win_info_list;
    int num_win_info_list;
} WSMSetState;

typedef struct _WSMRegisterWindow {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean allocated;		/* Packlib allocated the data, do not move. */
    Window window;
} WSMRegisterWindow;

typedef struct _WSMWmGetBackgroundWindow {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    int            screen;
} WSMWmGetBackgroundWindow;

typedef struct _WSMWmSetBackgroundWindow {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    Window         window;
} WSMWmSetBackgroundWindow;

typedef struct _WSMWmWindows {
    WSMRequestType   type;	/* Type of event, do not move or change. */
    Boolean          allocated;

    CARD32           location_flag;
    WindowProperty  *window_properties;
    int              num_window_properties;
    AttributePair  **match_attributes;
    int              num_match_attributes;
} WSMWmWindows;

typedef WSMAnyRequestOrReply WSMWmFocus;

typedef WSMAnyRequestOrReply WSMWmPointer;

typedef union _WSMRequest {
    WSMAnyRequestOrReply any;
    WSMConnect               connect;
    WSMExtensions            extensions;
    WSMConfigFormat          config_format;
    WSMGetState              get_state;
    WSMSetState              set_state;
    WSMRegisterWindow        register_window;
    WSMWmGetBackgroundWindow get_background;
    WSMWmSetBackgroundWindow set_background;
    WSMWmWindows             wm_windows;
    WSMWmFocus               wm_focus;
    WSMWmPointer             wm_pointer;
} WSMRequest;

/************************************************************
 *
 * Structures for recieving a reply message for each type of event.
 *
 ************************************************************/

typedef struct _WSMConnectReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean allocated;		/* Packlib allocated the data, do not move. */
    short version;
} WSMConnectReply;

typedef struct _WSMExtensionsReply {
    WSMRequestType type;	/* Type of event, do not move or change. */

    /*
     * Packlib allocated the data, do not move. 
     * If True will free each string on the extensions list
     * as well as the extensions ptr itself.
     */

    Boolean allocated;	
    String *extensions;
    int num_extensions;
} WSMExtensionsReply;

typedef struct _WSMConfigFormatReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean allocated;		/* Packlib allocated the data, do not move. */
    Boolean accepts_diffs;
    WSMAttribute * global_formats;
    int num_global_formats;
    WSMAttribute * window_formats;
    int num_window_formats;
    WSMAttribute * icon_formats;
    int num_icon_formats;
} WSMConfigFormatReply;

typedef struct _WSMGetStateReply {
    WSMRequestType type;	/* Type of event, do not move or change. */

    /*
     * Packlib allocated the data, do not move. 
     * If True will free the win_data list in each element on the 
     * win_info_list. as well as the win_info_list ptr itself.
     */

    Boolean allocated;		
    WSMWinInfo *win_info_list;
    int num_win_info_list;
} WSMGetStateReply;

typedef WSMAnyRequestOrReply WSMSetStateReply;

typedef struct _WSMRegisterWindowReply {
    WSMRequestType type;	/* Type of event, do not move or change. */

    /*
     * Packlib allocated the data, do not move. 
     * If True will free the window_data pointer.
     */

    Boolean allocated;		
    WSMWinData * window_data;
    int num_window_data;
} WSMRegisterWindowReply;

typedef struct _WSMWmGetBackgroundWindowReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    Window         window;
} WSMWmGetBackgroundWindowReply;

typedef struct _WSMWmSetBackgroundWindowReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    Window         window;
} WSMWmSetBackgroundWindowReply;

typedef struct _WSMWmWindowsReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    WSMWinEntry   *win_entry_list;
    int            num_win_entry_list;
} WSMWmWindowsReply;

typedef struct _WSMWmFocusReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    Window         window;
} WSMWmFocusReply;

typedef struct _WSMWmPointerReply {
    WSMRequestType type;	/* Type of event, do not move or change. */
    Boolean        allocated;

    Window         window;
    CARD32         location_flag;  /* client, frame, icon. */
} WSMWmPointerReply;

typedef union _WSMReply {
    WSMAnyRequestOrReply          any;
    WSMConnectReply               connect;
    WSMExtensionsReply            extensions;
    WSMConfigFormatReply          config_format;
    WSMGetStateReply              get_state;
    WSMSetStateReply              set_state;
    WSMRegisterWindowReply        register_window;
    WSMWmGetBackgroundWindowReply get_background;
    WSMWmSetBackgroundWindowReply set_background;
    WSMWmWindowsReply             wm_windows;
    WSMWmFocusReply               wm_focus;
    WSMWmPointerReply             wm_pointer;
} WSMReply;

/************************************************************
 *
 *  New function types.
 *
 ************************************************************/
typedef void (*WSMReplyCallbackFunc)(
    Widget,			/* A Widget on the same screen as the */
                                /* client that we are talking to. */
    XtPointer,			/* The user data. */
    WSMReply *,			/* The reply structure. */
    WSMErrorCode		/* The error code, or WSM_SUCCESS. */
);

typedef void (*WSMRequestCallbackFunc) (
    Widget,			/* A Widget on the same screen as the */
                                /* client that we are talking to. */
    XtPointer,			/* The user data. */
    WSMRequest *,		/* The request made of us. */
    WSMReply *			/* A reply structure that the user should */
				/* fill in with the reply to send back. */
);

/************************************************************
 *
 *  These depend on lots of stuff above, and must go last.
 *
 ************************************************************/

/*
 * There is an assumption that there is only one WM and WSM per screen
 * and therefore the config_data can be stored in the screen info.
 * This seems pretty safe.
 */

typedef struct _WSMScreenInfo {
    int screen_num;
    Atom wsm_selection;
    Atom wm_selection;
    WSMConfigFormatData global;
    WSMConfigFormatData window;
    WSMConfigFormatData icon;
    WSMRequestCallbackFunc request_callback;
    XtPointer request_data;
    struct _WSMScreenInfo *next;
} WSMScreenInfo;

typedef struct _WSMDispInfo {
    Display *disp;
    Atom connect;
    Atom extensions;
    Atom config_fmt;
    Atom get_state;
    Atom set_state;
    Atom reg_window;
    Atom get_background;
    Atom set_background;
    Atom wm_windows;
    Atom wm_focus;
    Atom wm_pointer;
    Atom targets, multiple, timestamp; /* Required default targets. */
    WSMScreenInfo *screen_info;
    struct _WSMDispInfo *next;
} WSMDispInfo;

#include "wsm_funcs.h"

#endif /* _WSM_PROTO_ --- Do not add anything after this line. */
