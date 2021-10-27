/* $XConsortium: pack.c /main/5 1995/07/15 20:38:40 drk $ */
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

/* this will redifine bzero to memset if needed. */
#include <Xm/XmosP.h>

#define COUNT_SIZE (sizeof(CARD16))
#define BOOL_SIZE  1		/* at least 1 byte for a boolean. */

/*
 * More intuitive names for various functions.
 */

#define PackBoolean(d, num)	PackCARD8((d), (CARD8) (num))
#define PackListNum(d, num)	PackCARD16((d), (CARD16) (num))
#define PackWindow(d, num)	PackCARD32((d), (CARD32) (num))
#define PackProperty(d, num)	PackCARD32((d), (CARD32) (num))

#define UnpackBoolean(data)	((Boolean) UnpackCARD8(data))
#define UnpackListNum(data)	((int) UnpackCARD16(data))
#define UnpackWindow(data)	((Window) UnpackCARD32(data))
#define UnpackProperty(data)	((WindowProperty) UnpackCARD32(data))

#define START_CHECK_MASK 0x80

/************************************************************
 *
 *  Function definitions.
 *
 ************************************************************/

/*
 * Routines for getting the amount of space that a request will take in the
 * protocol data stream.
 */

static int WinInfoSizePacked(Display *, int, WSMWinInfo *);
static int WinEntrySizePacked(Display *, int, WSMWinEntry *);
static int WinDataSizePacked(Display *, int,
			     WSMWinData *, int, WSMConfigFormatType);

/*
 * Routines for packing protocol data stream.
 */

   static    MessageData PackConfigFormat(MessageData, WSMConfigFormatReply *);
   static    MessageData PackSingleAttribute(MessageData, WSMAttribute *, int);
   static    MessageData PackWinInfo(Display *, int, MessageData,
				     WSMWinInfo *);
   static    MessageData PackWinEntry(Display *, int, MessageData,
				      WSMWinEntry *);
   static    MessageData PackWinData(Display *, int, MessageData, 
			       WSMWinData *, int, WSMConfigFormatType);
/* public */ MessageData PackCARD16(MessageData, CARD16);
/* public */ MessageData PackCARD8(MessageData, CARD8);
/* public */ MessageData PackString(MessageData, String);
/* public */ MessageData PackCARD32(MessageData, CARD32);

/*
 * Routines for unpacking protocol data stream.
 */

static void UnpackWinData(MessageData *, Display *, 
			  int, WSMConfigFormatType, WSMWinData **, int *);
static void UnpackSingleWinDataRec(MessageData *, WSMAttribute *,WSMWinData *);
static void UnpackWinInfo(MessageData *, Display *, int, WSMWinInfo *);
static void UnpackWinEntry(MessageData *, Display *, int, WSMWinEntry *);

/* public */ String UnpackString(MessageData *);
/* public */ CARD32 UnpackCARD32(MessageData *);
/* public */ CARD16 UnpackCARD16(MessageData *);
/* public */ CARD8  UnpackCARD8(MessageData *);


/************************************************************
 *
 *  These two routines take data stored in a structure that
 *  humans can understand and pack it down into nice little
 *  protocol requests defined by the OSF WSM Protocol spec.
 *
 ************************************************************/

/*	Function Name: _WSMPackRequest
 *	Description: Packs up the request passed into an appropriate
 *                   message data structure.
 *	Arguments: dpy - the display we will be sending this message on.
 *                 screen_num - The screen we will be sending this message to.
 *                 request - request to send.
 * RETURNED        msg_data_len - The length of the message data.
 * RETURNED        error - An error code if something went wrong.
 *	Returns: A pointer to a message data struct.
 * 
 * NOTE: The message data struct must be freed by the calling routine.
 */

MessageData 
_WSMPackRequest(Display *dpy, int screen_num, WSMRequest *request,
		unsigned long *msg_data_len, WSMErrorCode *error)
{
    MessageData data, save;
    int i;
    long size;
    WSMWinInfo *win_info;
    WSMConfigFormatData *global_attrs, *win_attrs, *icon_attrs;

    *msg_data_len = 0;		/* default value. */

    global_attrs = _WSMGetConfigFormat(dpy, screen_num, WSM_GLOBAL_FMT);
    win_attrs = _WSMGetConfigFormat(dpy, screen_num, WSM_WINDOW_FMT);
    icon_attrs = _WSMGetConfigFormat(dpy, screen_num, WSM_ICON_FMT);

    *error = WSM_SUCCESS;

    if (_WSMRequiresConfigFormat(request->any.type) && 
	(global_attrs->num_attrs == 0) && 
	(win_attrs->num_attrs == 0) && (icon_attrs->num_attrs == 0))
    {
	*error = WSM_ERROR_NO_CONFIG_FORMAT;
	return(NULL);
    }
    if (((request->any.type == WSM_CONNECT) ||
	 (request->any.type == WSM_CONFIG_FMT)) &&
	((global_attrs->num_attrs != 0) ||
	 (win_attrs->num_attrs != 0) || (icon_attrs->num_attrs != 0)))
    {
	*error = WSM_ERROR_ALREADY_HAVE_CONFIG_FORMAT;
	return(NULL);
    }

    /* before sending extensions make sure screen_info config format
       is cleared */
    /* may want to remove this and leave it up to application writer */
    if ((request->any.type == WSM_EXTENSIONS) && 
	((global_attrs->num_attrs != 0) ||
	 (win_attrs->num_attrs != 0) || (icon_attrs->num_attrs != 0)))
      {
	_WSMClearConfigScreenInfo(dpy,screen_num);
      }
    /*
     *  Calculate the size of the request.
     */
    switch (request->any.type) {
    case WSM_CONNECT:
	size = COUNT_SIZE;
	size += sizeof(CARD8) * request->connect.num_versions;
	break;
    case WSM_EXTENSIONS:
	size = COUNT_SIZE;
	for (i = 0; i < request->extensions.num_extensions; i++) {
	    size += (strlen(request->extensions.extension_suggestions[i]) +
		     COUNT_SIZE);
	}
	break;
    case WSM_CONFIG_FMT:
	size = 0;
	break;
    case WSM_GET_STATE:
	size = sizeof(CARD32) + BOOL_SIZE;
	break;
    case WSM_SET_STATE:
	size = COUNT_SIZE;	/* The count of window info. */
	win_info = request->set_state.win_info_list;
	for (i = 0; i < request->set_state.num_win_info_list; i++,win_info++)
	    size += WinInfoSizePacked(dpy, screen_num, win_info);
	break;
    case WSM_REG_WINDOW:
	size = sizeof(CARD32);
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	size = sizeof(CARD16);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	size = sizeof(CARD32);
	break;
    case WSM_WM_WINDOWS:
	size = sizeof(CARD32); /* flag */
	size += COUNT_SIZE;    /* prop count */
	size += request->wm_windows.num_window_properties * sizeof(Window);
	size += COUNT_SIZE;    /* num match attributes */
	for (i=0; i<request->wm_windows.num_match_attributes; i++)
	  {
	    size += COUNT_SIZE; /* num attributes */
	    size += 2 *
	            request->wm_windows.match_attributes[i]->num_attributes *
		    sizeof(WindowProperty);
	  }
	break;
    case WSM_WM_FOCUS:
	size = 0;
	break;
    case WSM_WM_POINTER:
	size = 0;
	break;
    default:
	return(NULL);
    }

    save = data = (MessageData) XtMalloc(size);

    /*
     * Fill in the message.
     */
    switch (request->any.type) {
    case WSM_CONNECT:
	data = PackListNum(data, request->connect.num_versions);
	for (i = 0; i < request->connect.num_versions; i++) 
	    data = PackCARD8(data, (CARD8) request->connect.known_versions[i]);
	break;
    case WSM_EXTENSIONS:
	data = PackListNum(data, request->extensions.num_extensions);
	for (i = 0; i < request->extensions.num_extensions; i++) 
	    data=PackString(data,request->extensions.extension_suggestions[i]);
	break;
    case WSM_CONFIG_FMT:
	break;
    case WSM_GET_STATE:
	data = PackWindow(data, request->get_state.window);
	(void) PackBoolean(data, request->get_state.diffs_allowed);
	break;
    case WSM_SET_STATE:
	win_info = request->set_state.win_info_list;
	data = PackListNum(data, request->set_state.num_win_info_list);
	for (i = 0; i < request->set_state.num_win_info_list; i++, win_info++)
	    data = PackWinInfo(dpy, screen_num, data, win_info);
	break;
    case WSM_REG_WINDOW:
	(void) PackWindow(data, request->register_window.window);
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	(void) PackCARD16(data, request->get_background.screen);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	(void) PackWindow(data, request->set_background.window);
	break;
    case WSM_WM_WINDOWS:
	{
	  int i, j;

	  data = PackCARD32(data, request->wm_windows.location_flag);
	  data = PackListNum(data, request->wm_windows.num_window_properties);
	  for (i=0; i<request->wm_windows.num_window_properties; i++)
	    data = PackProperty(data, request->wm_windows.window_properties[i]);

	  data = PackListNum(data, request->wm_windows.num_match_attributes);
	  for (i = 0;  i < request->wm_windows.num_match_attributes;  i++)
	    {
	      AttributePair *match_attr;

	      match_attr = request->wm_windows.match_attributes[i];
	      data = PackListNum(data, match_attr->num_attributes);

	      for (j = 0;  j < match_attr->num_attributes;  j++)
		data = PackProperty(data, match_attr->allowed_attributes[j]);

	      for (j = 0;  j < match_attr->num_attributes;  j++)
		data = PackProperty(data, match_attr->prohibited_attributes[j]);
	    }
	}
	break;
    case WSM_WM_FOCUS:
	break;
    case WSM_WM_POINTER:
	break;
    default:
	break;
    }
    
    *msg_data_len = size;
    return(save);
} /* _WSMPackRequest */

/*	Function Name: _WSMPackReply
 *	Description: Packs up the Reply passed into an appropriate
 *                   message data structure.
 *	Arguments: dpy - the display we will be sending this message on.
 *                 screen_num - The screen we will be sending this message to.
 *                 reply - reply to send.
 * RETURNED        msg_data_len - The length of the message data.
 *	Returns: A pointer to a message data struct.
 * 
 * NOTE: The message data struct must be freed by the calling routine.
 */

MessageData 
_WSMPackReply(Display *dpy, int screen_num,
	      WSMReply *reply, unsigned long *msg_data_len)
{
    WSMAttribute *attr;
    WSMWinInfo *win_info;
    WSMWinEntry *win_entry;
    register int i;
    long size;
    MessageData data, save;

    *msg_data_len = 0;		/* default value. */

    switch (reply->any.type) {
    case WSM_CONNECT:
	size = sizeof(CARD8);
	break;
    case WSM_EXTENSIONS:
	size = COUNT_SIZE;
	for (i = 0; i < reply->extensions.num_extensions; i++) {
	    size += (strlen(reply->extensions.extensions[i]) + COUNT_SIZE);
	}
	break;
    case WSM_CONFIG_FMT:
	size = BOOL_SIZE;	/* accept_diffs. */
	size += COUNT_SIZE;	/* The count of global attributes. */
	attr = reply->config_format.global_formats;
	for (i = 0; i < reply->config_format.num_global_formats; i++)  {
	    /*
	     * Bytes in string
	     */
	    size += strlen(XrmQuarkToString(attr[i].nameq)) * sizeof(CARD8);
	    /*
	     * len of string + size + is_list.
	     */
	    size += COUNT_SIZE + sizeof(CARD8) + BOOL_SIZE; 
	}
	size += COUNT_SIZE;	/* The count of window attributes. */
	attr = reply->config_format.window_formats;
	for (i = 0; i < reply->config_format.num_window_formats; i++)  {
	    /*
	     * Bytes in string
	     */
	    size += strlen(XrmQuarkToString(attr[i].nameq)) * sizeof(CARD8);
	    /*
	     * len of string + size + is_list.
	     */
	    size += COUNT_SIZE + sizeof(CARD8) + BOOL_SIZE; 
	}
	size += COUNT_SIZE;	/* The count of icon attributes. */
	attr = reply->config_format.icon_formats;
	for (i = 0; i < reply->config_format.num_icon_formats; i++)  {
	    /*
	     * Bytes in string
	     */
	    size += strlen(XrmQuarkToString(attr[i].nameq)) * sizeof(CARD8);
	    /*
	     * len of string + size + is_list.
	     */
	    size += COUNT_SIZE + sizeof(CARD8) + BOOL_SIZE; 
	}
	break;
    case WSM_GET_STATE:
	size = COUNT_SIZE;	/* The count of window info. */
	win_info = reply->get_state.win_info_list;
	for (i = 0; i < reply->get_state.num_win_info_list; i++,win_info++)
	    size += WinInfoSizePacked(dpy, screen_num, win_info);
	break;
    case WSM_SET_STATE:
	size = 0;
	break;
    case WSM_REG_WINDOW:
	size = WinDataSizePacked(dpy, screen_num, 
				 reply->register_window.window_data,
				 reply->register_window.num_window_data,
				 WSM_WINDOW_FMT);
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	size = sizeof(CARD32);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	size = sizeof(CARD32);
	break;
    case WSM_WM_WINDOWS:
	size = COUNT_SIZE;	/* The count of window entries. */
	win_entry = reply->wm_windows.win_entry_list;
	for (i = 0; i < reply->wm_windows.num_win_entry_list; i++,win_entry++)
	    size += WinEntrySizePacked(dpy, screen_num, win_entry);
	break;
    case WSM_WM_FOCUS:
	size = sizeof(CARD32);
	break;
    case WSM_WM_POINTER:
	size  = sizeof(CARD32); /* window */
	size += sizeof(CARD32); /* flag */
	break;
    default:
	/*
	 * ||| Error Message.
	 */
	return(NULL);
    }

    save = data = (MessageData) XtMalloc(size);

    switch (reply->any.type) {
    case WSM_CONNECT:
	(void) PackCARD8(data, (CARD8) (reply->connect.version & 0xFF));
	break;
    case WSM_EXTENSIONS:
	data = PackListNum(data, reply->extensions.num_extensions);
	for (i = 0; i < reply->extensions.num_extensions; i++) 
	    data = PackString(data, reply->extensions.extensions[i]);
	break;
    case WSM_CONFIG_FMT:
	PackConfigFormat(data, &(reply->config_format));
	break;
    case WSM_GET_STATE:
	win_info = reply->get_state.win_info_list;
	data = PackListNum(data, reply->get_state.num_win_info_list);
	for (i = 0; i < reply->get_state.num_win_info_list; i++, win_info++)
	    data = PackWinInfo(dpy, screen_num, data, win_info);
	break;
    case WSM_SET_STATE:
	break;
    case WSM_REG_WINDOW:
	data = PackWinData(dpy, screen_num, data, 
			   reply->register_window.window_data,
			   reply->register_window.num_window_data,
			   WSM_WINDOW_FMT);
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	(void) PackWindow(data, reply->get_background.window);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	(void) PackWindow(data, reply->set_background.window);
	break;
    case WSM_WM_WINDOWS:
	win_entry = reply->wm_windows.win_entry_list;
	data = PackListNum(data, reply->wm_windows.num_win_entry_list);
	for (i = 0; i < reply->wm_windows.num_win_entry_list; i++, win_entry++)
	    data = PackWinEntry(dpy, screen_num, data, win_entry);
	break;
    case WSM_WM_FOCUS:
	(void) PackWindow(data, reply->wm_focus.window);
	break;
    case WSM_WM_POINTER:
	(void) PackWindow(data, reply->wm_pointer.window);
	(void) PackCARD32(data, reply->wm_pointer.location_flag);
	break;
    default:
	break;
    }
    
    *msg_data_len = size;
    return(save);
}/* _WSMPackReply */

/************************************************************
 *
 *  These routines take data on the wire and put it into a structure
 *  that can be manipulated by the app.
 *
 ************************************************************/

/*	Function Name: _WSMUnpackRequest
 *	Description: Unpacks the byte stream from the wire into a request
 *                   message structure.
 *	Arguments: dpy - The Display.
 *                 screen_num - The Screen number.
 *                 data - The bits from accross the wire.
 *                 len - The number of bits sent - Better error checking
 *                       makes this more necessary.
 *                 type - The type of message received.
 * RETURNED        request - This request structure is filled in with the data
 *                         unpacked from the protocol.
 *	Returns: none.
 */

/*ARGSUSED*/
void
_WSMUnpackRequest(Display *dpy, int screen_num, MessageData data, 
		  unsigned long len, WSMRequestType type, WSMRequest *request)
{
    register int i;
    request->any.type = type;	/* Save the type. */
    request->any.allocated = False;

    switch (request->any.type) {
    case WSM_CONNECT:
	request->connect.num_versions = UnpackListNum(&data);
	request->connect.known_versions =
	    (short *) XtMalloc(sizeof(short) * request->connect.num_versions);
	request->connect.allocated = True;
	for (i = 0; i < request->connect.num_versions; i++) 
	    request->connect.known_versions[i] = (short) UnpackCARD8(&data);
	break;
    case WSM_EXTENSIONS:
        {
	    register int num;
	    register String *ptr;

	    num = request->extensions.num_extensions = UnpackListNum(&data);
	    ptr = (String *) XtMalloc(sizeof(String) * num);
	    request->extensions.extension_suggestions = ptr;
	    request->extensions.allocated = True;
	    for (i = 0; i < num; i++, ptr++) 
		*ptr = UnpackString(&data);
	}
	break;
    case WSM_CONFIG_FMT:
	break;
    case WSM_GET_STATE:
	request->get_state.window = UnpackWindow(&data);
	request->get_state.diffs_allowed = UnpackBoolean(&data);
	break;
    case WSM_SET_STATE:
        {
	    int num = UnpackListNum(&data);
	    
	    request->set_state.num_win_info_list = num;
	    request->set_state.win_info_list = 
		(WSMWinInfo *) XtMalloc(sizeof(WSMWinInfo) * num);
	    request->extensions.allocated = True;

	    for (i = 0; i < num; i++) {
		UnpackWinInfo(&data, dpy, screen_num, 
			      request->set_state.win_info_list + i);
	    }
	}
	break;
    case WSM_REG_WINDOW:
	request->register_window.window = UnpackWindow(&data);	
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	request->get_background.screen = (int)UnpackCARD16(&data);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	request->set_background.window = UnpackWindow(&data);
	break;
    case WSM_WM_WINDOWS:
	{
	  int num, i, j;

	  request->extensions.allocated = True;
	  request->wm_windows.location_flag = UnpackCARD32(&data);

	  num = request->wm_windows.num_window_properties = UnpackListNum(&data);
	  request->wm_windows.window_properties =
	    (WindowProperty *) XtMalloc(sizeof(WindowProperty) * num);
	  for (i=0; i<num; i++)
	    request->wm_windows.window_properties[i] = UnpackProperty(&data);


	  num = request->wm_windows.num_match_attributes = UnpackListNum(&data);
	  request->wm_windows.match_attributes =
	    (AttributePair **) XtMalloc(sizeof(AttributePair*) * num);

	  for (i=0; i<request->wm_windows.num_match_attributes; i++)
	    {
	      num = UnpackListNum(&data);

	      request->wm_windows.match_attributes[i] = (AttributePair *)
		XtMalloc(sizeof(AttributePair) * num * 2 + sizeof(int));

	      request->wm_windows.match_attributes[i]->num_attributes = num;
	      for (j=0; j<num; j++)
		{
		  request->wm_windows.match_attributes[i]->allowed_attributes[j] =
		    UnpackProperty(&data);
		}
	      for (j=0; j<num; j++)
		{
		  request->wm_windows.match_attributes[i]->prohibited_attributes[j] =
		    UnpackProperty(&data);
		}
	    }
	}
	break;
    case WSM_WM_FOCUS:
	break;
    case WSM_WM_POINTER:
	break;
    default:
	break;
    }
} /* _WSMUnpackRequest */

/*	Function Name: _WSMUnpackReply
 *	Description: Unpacks the byte stream from the wire into a reply
 *                   message structure.
 *	Arguments: dpy - The Display.
 *                 screen_num - The Screen number.
 *                 data - the bits from accross the wire.
 *                 len - The number of bits sent - Better error checking
 *                       makes this more necessary.
 *                 type - The type of message received.
 * RETURNED        reply - This reply structure is filled in with the data
 *                         unpacked from the protocol.
 *	Returns: none.
 */

/*ARGSUSED*/
void
_WSMUnpackReply(Display *dpy, int screen_num, MessageData data, 
		unsigned long len, WSMRequestType type, WSMReply *reply)
{
    register int i;
    reply->any.type = type;		/* Save the type. */
    reply->any.allocated = False;

    switch (reply->any.type) {
    case WSM_CONNECT:
        if (data != NULL)
	  reply->connect.version = (short) UnpackCARD8(&data);
	else
	  fprintf(stderr, "Error - Connection request reply data is empty!\n");
	break;
    case WSM_EXTENSIONS:
        {
	    register int num;
	    register String *ptr;

	    num = reply->extensions.num_extensions = UnpackListNum(&data);
	    ptr = (String *) XtMalloc(sizeof(String) * num);
	    reply->extensions.allocated = True;
	    reply->extensions.extensions = ptr;
	    for (i = 0; i < num; i++, ptr++) 
		*ptr = UnpackString(&data);
	}
	break;
    case WSM_CONFIG_FMT:
        {
	    register int types;

	    WSMConfigFormatReply * config_format = &(reply->config_format);
	    WSMConfigFormatData *fmt;
	    WSMScreenInfo *scr_info = _WSMGetScreenInfo(dpy, screen_num);
	    
	    config_format->accepts_diffs = UnpackBoolean(&data);

	    for (types = 0; types < 3; types++) {
		switch(types) {
		case WSM_GLOBAL_FMT:
		    fmt = &(scr_info->global);
		    break;
		case WSM_WINDOW_FMT:
		    fmt = &(scr_info->window);
		    break;
		case WSM_ICON_FMT:
		    fmt = &(scr_info->icon);
		    break;
		}

		fmt->num_attrs = UnpackListNum(&data);
		fmt->attr_list = (WSMAttribute *) 
		    XtMalloc(sizeof(WSMAttribute) * fmt->num_attrs);

		for (i = 0; i < fmt->num_attrs; i++) {
		    String str = UnpackString(&data);
		    fmt->attr_list[i].nameq = XrmStringToQuark(str);
		    XtFree(str);
		    fmt->attr_list[i].size = UnpackCARD8(&data);
		    fmt->attr_list[i].is_list = UnpackCARD8(&data);
		}
	    }

	    /*
	     * No need to allocate this, since they are just pointers
	     * back to global data.
	     */

	    config_format->global_formats = scr_info->global.attr_list;
	    config_format->num_global_formats = scr_info->global.num_attrs;
	    config_format->window_formats = scr_info->window.attr_list;
	    config_format->num_window_formats = scr_info->window.num_attrs;
	    config_format->icon_formats = scr_info->icon.attr_list;
	    config_format->num_icon_formats = scr_info->icon.num_attrs;
	}
	break;
    case WSM_GET_STATE:
	{
	    int num =reply->get_state.num_win_info_list = UnpackListNum(&data);
	    reply->get_state.win_info_list = 
		(WSMWinInfo *) XtMalloc(sizeof(WSMWinInfo) * num);
	    reply->get_state.allocated = True;

	    for (i = 0; i < num; i++) 
		UnpackWinInfo(&data, dpy, screen_num, 
			      reply->get_state.win_info_list + i);
	}
	break;
    case WSM_SET_STATE:
	break;
    case WSM_REG_WINDOW:
	if (data != NULL)
	  {
	    UnpackWinData(&data, dpy, screen_num, WSM_WINDOW_FMT,
			  &(reply->register_window.window_data),
			  &(reply->register_window.num_window_data));
	    reply->register_window.allocated = True;
	  }
	else
	  fprintf(stderr, "Error - Register Window reply data is empty!\n");
	break;
    case WSM_WM_GET_BACKGROUND_WINDOW:
	reply->get_background.window = UnpackWindow(&data);
	break;
    case WSM_WM_SET_BACKGROUND_WINDOW:
	reply->set_background.window = UnpackWindow(&data);
	break;
    case WSM_WM_WINDOWS:
	{
	  int num;

	  num = reply->wm_windows.num_win_entry_list = UnpackListNum(&data);
	  reply->wm_windows.win_entry_list = 
	    (WSMWinEntry *) XtMalloc(sizeof(WSMWinEntry) * num);
	  reply->wm_windows.allocated = True;

	  for (i = 0; i < num; i++) 
	    UnpackWinEntry(&data, dpy, screen_num, 
			   reply->wm_windows.win_entry_list + i);
	}
	break;
    case WSM_WM_FOCUS:
	reply->wm_focus.window = UnpackWindow(&data);
	break;
    case WSM_WM_POINTER:
	reply->wm_pointer.location_flag = UnpackCARD32(&data);
	break;
    default:
	break;
    }
} /* _WSMUnpackReply */

/************************************************************
 *
 *  Internal Routines for getting size of packed requests.
 *
 ************************************************************/

/*	Function Name: WinInfoSizePacked
 *	Description: Returns the size this window structure will take
 *                   when packed into a protocol data stream.
 *	Arguments: dpy, screen_num - The display and screen of the client we 
 *                                   are talking to.
 *                 win_info - The window info whose size me are checking.
 *	Returns: the size (in bytes)
 */

static int 
WinInfoSizePacked(Display *dpy, int screen_num, WSMWinInfo *win_info)
{
    int size = sizeof(CARD32);
    size += WinDataSizePacked(dpy, screen_num, 
			      win_info->data_list, win_info->num_data_list,
			      _WSMGetConfigFormatType(win_info->window));
    return(size);
}

/*	Function Name: WinEntrySizePacked
 *	Description: Returns the size this window structure will take
 *                   when packed into a protocol data stream.
 *	Arguments: dpy, screen_num - The display and screen of the client we 
 *                                   are talking to.
 *                 win_entry - The window entry whose size we are checking.
 *	Returns: the size (in bytes)
 */

static int 
WinEntrySizePacked(Display *dpy, int screen_num, WSMWinEntry *win_entry)
{
    int size;

    size  = sizeof(CARD32);
    size += sizeof(CARD32) * win_entry->num_windows;
    size  = sizeof(CARD32);
    size += sizeof(WindowProperty) * win_entry->num_match_properties;

    return(size);
}

/*	Function Name: WinDataSizePacked
 *	Description: The size that will be taken up with this window data
 *                   once it is packed into the protocol stream.
 *	Arguments: dpy, screen_num - The screen nmuber and display that
 *                                   we are talking to.
 *                 win_data - The data stored in a window data structure list.
 *                 num - number of items in the list.
 *                 fmt - The type of format to use to pack this data.
 *	Returns: The size.
 */

static int 
WinDataSizePacked(Display *dpy, int screen_num, 
		  WSMWinData *win_data, int num, WSMConfigFormatType fmt)
{
    register int i, size;
    WSMConfigFormatData *fmt_data = _WSMGetConfigFormat(dpy, screen_num, fmt);

    size = fmt_data->num_attrs / 8 + 1;	/* Size of header bits. */
    for ( i = 0; i < num; i++, win_data++) {
	WSMAttribute *attr = _WSMGetMatchingAttr(win_data->nameq, fmt_data);
	if (attr == NULL) {
	    /*
	     * ||| ERROR.
	     */
	    continue;
	}

	switch(win_data->type) {
	case WSM_VALUE_DATA:
	    size += attr->size/8;
	    break;
	default:		/* All others are lists. */
	    size += COUNT_SIZE;
	    size += ((attr->size/8) * win_data->data_len);
	    break;
	}
    }

    return(size);
}

/************************************************************
 *
 *  Internal Routines for packing data into the protocol stream.
 *
 ************************************************************/

/*	Function Name: PackString
 *	Description: Packs a string into the data stream.
 *	Arguments: data - The data stream to pack into.
 *                 str - The string to pack.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

MessageData
PackString(MessageData data, String str)
{
    register int i, len = strlen(str);

    data = PackListNum(data, len);
    for (i = 0; i < len; i++, str++) 
	data = PackCARD8(data, *str);

    return(data);
}

/*	Function Name: PackConfigFormat
 *	Description: Packs the configuration format into the data stream.
 *	Arguments: data - the datastream.
 *                 config - the reply config format message.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

static MessageData 
PackConfigFormat(MessageData data, WSMConfigFormatReply *config)
{
    data = PackBoolean(data, config->accepts_diffs);
    data = PackSingleAttribute(data, config->global_formats,
			       config->num_global_formats);
    data = PackSingleAttribute(data, config->window_formats,
			       config->num_window_formats);
    data = PackSingleAttribute(data, config->icon_formats,
			       config->num_icon_formats);
    return(data);
}

/*	Function Name: PackSingleAttribute
 *	Description: Packs a single attribute into a the data stream.
 *	Arguments: data - the datastream.
 *                 attr_list, num - The attribute list to pack into the stream.
 *                                  The number of items in the list.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

static MessageData 
PackSingleAttribute(MessageData data, WSMAttribute *attr_list, int num)
{
    register int i;

    data = PackListNum(data, num);
    for (i = 0; i < num; i++, attr_list++) {
	data = PackString(data, XrmQuarkToString(attr_list->nameq));
	data = PackCARD8(data, (CARD8) attr_list->size);
	data = PackBoolean(data, attr_list->is_list);
    }
    return(data);
}

/*	Function Name: PackWinInfo
 *	Description: Packs the window information into the data stream.
 *	Arguments: dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 data - The message stream.
 *                 win_info - The window info list to pack into the data stream.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

static MessageData
PackWinInfo(Display *dpy, 
	    int screen_num, MessageData data, WSMWinInfo *win_info)
{
    data = PackWindow(data, win_info->window);
    return(PackWinData(dpy, screen_num, data,
		       win_info->data_list, win_info->num_data_list, 
		       _WSMGetConfigFormatType(win_info->window)));
}

/*	Function Name: PackWinEntry
 *	Description: Packs the window information into the data stream.
 *	Arguments: dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 data - The message stream.
 *                 win_info - The window info list to pack into the data stream.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

static MessageData
PackWinEntry(Display *dpy, 
	    int screen_num, MessageData data, WSMWinEntry *win_entry)
{
    int i;

    data = PackListNum(data, win_entry->num_windows);
    for (i = 0;  i < win_entry->num_windows;  i++)
      data = PackWindow(data, win_entry->windows[i]);

    data = PackListNum(data, win_entry->num_match_properties);
    for (i = 0;  i <  win_entry->num_match_properties;  i++)
      data = PackProperty(data, win_entry->match_properties[i]);

    return(data);
}

/*	Function Name: PackWinData
 *	Description: Packs the window data into the data stream.
 *	Arguments: dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 data - The message stream.
 *                 win_data, num - The window data to pack
 *                                  into the data stream, and its size.
 *                 fmt - The type of config format to use.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */

static MessageData
PackWinData(Display *dpy, int screen_num, MessageData data, 
	    WSMWinData *win_data, int num, WSMConfigFormatType fmt)
{
    WSMConfigFormatData *conf_fmt = _WSMGetConfigFormat(dpy, screen_num, fmt);
    MessageData mask_pos = data;
    WSMAttribute *attr;
    register int i, size = (conf_fmt->num_attrs / 8) + 1;

    data = (MessageData) ((char *)data + size);
    bzero((char *) mask_pos, size);	/* Set all bits to zero initially*/

    attr = conf_fmt->attr_list;
    for (i = 0; i < conf_fmt->num_attrs ; i++, attr++) {
	WSMWinData *this_data;

	this_data = _WSMGetMatchingWinData(win_data, num, attr->nameq);
	if (this_data != NULL) {
	    if (this_data->type != WSM_VALUE_DATA) 
		data = PackListNum(data, this_data->data_len); 

	    switch(this_data->type) {
	    case WSM_CHAR_LIST_DATA:
	        {
		    register int j;
		    char * ptr = this_data->data.char_ptr;
		    for (j = 0 ; j < this_data->data_len; j++, ptr++)
			data = PackCARD8(data, (CARD8) *ptr);
		    break;
		}
	    case WSM_SHORT_LIST_DATA:
	        {
		    register int j;
		    short * ptr = this_data->data.short_ptr;
		    for (j = 0 ; j < this_data->data_len; j++, ptr++)
			data = PackCARD16(data, (CARD16) *ptr);
		    break;
		}
	    case WSM_LONG_LIST_DATA:
	        {
		    register int j;
		    long * ptr = this_data->data.long_ptr;
		    for (j = 0 ; j < this_data->data_len; j++, ptr++) 
			data = PackCARD32(data, (CARD32) *ptr);
		    break;
		}
	    case WSM_VALUE_DATA:
		switch(attr->size) {
		case 8:
		    data = PackCARD8(data, (CARD8) this_data->data.value);
		    break;
		case 16:
		    data = PackCARD16(data, (CARD16) this_data->data.value);
		    break;
		case 32:
		    data = PackCARD32(data, (CARD32) this_data->data.value);
		    break;			
		}
	    case WSM_NONE:
		break;
	    }

	    /*
	     * The first item sets the MSBit, the 8th Item sets the LSBit.
	     * The Nineth sets the MSBit in the second byte.  The unused
	     * bits are the LSBits of the last byte.
	     */

	    (*(unsigned char *)mask_pos) |= (char)(1 << (7 - (i % 8)));
	}

	/*
	 * When we get to the end of a char, move to the next one.
	 */

	if ((i % 8) == 7)	
	    mask_pos = ((char *)mask_pos + 1);
    }

    return(data);
}

/*	Function Name: PackCARD32
 *	Description: Packs an 32 bit value into the data stream.
 *	Arguments: data - The data stream to pack into.
 *                 val - The 32 bit value to pack.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */
    
MessageData
PackCARD32(MessageData data, CARD32 val)
{
    CARD16 bottom = val & (0xFFFF);
    CARD16 top = val >> 16;

    data = PackCARD16(data, top);
    data = PackCARD16(data, bottom);
    return(data);
}

/*	Function Name: PackCARD16
 *	Description: Packs an 16 bit value into the data stream.
 *	Arguments: data - The data stream to pack into.
 *                 val - The 16 bit value to pack.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */
    
MessageData
PackCARD16(MessageData data, CARD16 val)
{
    CARD8 bottom = val & (0xFF);
    CARD8 top = val >> 8;

    data = PackCARD8(data, top);
    data = PackCARD8(data, bottom);
    return(data);
}

/*	Function Name: PackCARD8
 *	Description: Packs an 8 bit value into the data stream.
 *	Arguments: data - The data stream to pack into.
 *                 val - The 8 bit value to pack.
 *	Returns: data - A pointer into the next empty location in the
 *                      data stream.
 */
    
MessageData
PackCARD8(MessageData data, CARD8 val)
{
    CARD8 *ptr = (CARD8 *) data;

    *ptr = (CARD8) val;
    data = ((char*)data) + 1;

    return(data);
}

/************************************************************
 *
 *  Internal routines for unpacking the data from the 
 *  protocol stream.
 *
 ************************************************************/

/*	Function Name: UnpackWinData
 *	Description: Unpacks the window data from the protocol stream.
 *	Arguments: data_ptr - A pointer to the message data stream.
 *                 dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 fmt - The window format to use.
 * RETURNED        win_data - The window data information.
 * RETURNED        num - The number of window data structs in "win_data".
 *	Returns: none
 */

static void
UnpackWinData(MessageData *data_ptr, Display *dpy, int screen_num,
	      WSMConfigFormatType fmt,
	      WSMWinData **win_data, int *num)
{
    register int i, size;
    WSMConfigFormatData *conf_fmt = _WSMGetConfigFormat(dpy, screen_num, fmt);
    unsigned char *bit_mask, *current_mask, check_mask;
    WSMAttribute *attr;
    WSMWinData *ptr;
    
    /*
     * Figure out how many values have been specified so we know how much
     * memory to allocate.
     */

    size = conf_fmt->num_attrs / 8 + 1;
    current_mask = bit_mask = (unsigned char *) XtMalloc(size);

    /*
     * Unpack the bits from the wire.
     */
    for (i = 0; i < size; i++, current_mask++) 
	*current_mask = UnpackCARD8(data_ptr);

    current_mask = bit_mask;
    check_mask = START_CHECK_MASK;
    for (*num = 0, i = 0; i < conf_fmt->num_attrs; i++) {
	if (*current_mask & check_mask) 
	    (*num)++;

	check_mask >>= 1;
	if ((i % 8) == 7) {
	    current_mask++;
	    check_mask = START_CHECK_MASK;
	}
    }

    ptr = *win_data = (WSMWinData *) XtMalloc(*num * sizeof(WSMWinData));

    /*
     * For each value that was specified pack the data into the window
     * data record.
     */

    current_mask = bit_mask;
    check_mask = START_CHECK_MASK;
    attr = conf_fmt->attr_list;
    for (i = 0; i < conf_fmt->num_attrs; i++, attr++) {
	if (*current_mask & check_mask) 
	    UnpackSingleWinDataRec(data_ptr, attr, ptr++);

	check_mask >>= 1;
	if ((i % 8) == 7) {
	    current_mask++;
	    check_mask = START_CHECK_MASK;
	}
    }

    XtFree((XtPointer) bit_mask);
}

/*	Function Name: UnpackSingleWinDataRec
 *	Description: Packs a single window data record.
 *	Arguments: data_ptr - The data pointer.
 *                 attr - The attribute record that defines general information
 *                        on how to unpack this data.
 *                 win_data - the window data struct.
 *	Returns: none
 */

static void
UnpackSingleWinDataRec(MessageData *data_ptr,
		     WSMAttribute *attr, WSMWinData *win_data)
{
    register int i;

    /*
     * Set these no matter what the size.
     */

    win_data->nameq = attr->nameq; /* Save the name. */

    if (attr->is_list) {
	win_data->data_len = UnpackListNum(data_ptr);
    }
    else {
	win_data->type = WSM_VALUE_DATA;
	win_data->data_len = 0;	/* unused... */
    }

    /*
     * Based on the size, and whether or not this is a list, unpack
     * the data from the wire.
     */

    switch(attr->size) {
    case 8:
	if (attr->is_list) {
	    char *local;
	    
	    win_data->type = WSM_CHAR_LIST_DATA;
	    local = win_data->data.char_ptr = 
		(char *) XtMalloc(sizeof(char) * win_data->data_len);
	    for (i = 0; i < win_data->data_len; i++, local++) 
		*local = UnpackCARD8(data_ptr);
	}
	else
	    win_data->data.value = UnpackCARD8(data_ptr);

	break;
    case 16:
	if (attr->is_list) {
	    short *local;
	    
	    win_data->type = WSM_SHORT_LIST_DATA;
	    local = win_data->data.short_ptr = 
		(short *) XtMalloc(sizeof(short) * win_data->data_len);
	    for (i = 0; i < win_data->data_len; i++, local++) 
		*local = UnpackCARD16(data_ptr);
	}
	else
	    win_data->data.value = UnpackCARD16(data_ptr);

	break;
    case 32:
	if (attr->is_list) {
	    long *local;
	    
	    win_data->type = WSM_LONG_LIST_DATA;
	    local = win_data->data.long_ptr = 
		(long *) XtMalloc(sizeof(long) * win_data->data_len);
	    for (i = 0; i < win_data->data_len; i++, local++) 
		*local = UnpackCARD32(data_ptr);
	}
	else
	    win_data->data.value = UnpackCARD32(data_ptr);

	break;
    }
}

/*	Function Name: UnpackWinInfo
 *	Description: Unpacks the window information from the protocol stream.
 *	Arguments: data_ptr - A pointer to the message data stream.
 *                 dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 win_info - The window information.
 *	Returns: none
 */

static void
UnpackWinInfo(MessageData *data, Display *dpy,
	      int screen_num, WSMWinInfo *win_info)
{
    win_info->window = UnpackWindow(data);
    UnpackWinData(data, dpy, screen_num,
		  _WSMGetConfigFormatType(win_info->window),
		  &(win_info->data_list), &(win_info->num_data_list));
}

/*	Function Name: UnpackWinEntry
 *	Description: Unpacks the window entry information from the protocol stream.
 *	Arguments: data_ptr - A pointer to the message data stream.
 *                 dpy, screen_num - The display and screen of the client
 *                                   that we are talking to.
 *                 win_entry - The window entry information.
 *	Returns: none
 */

static void
UnpackWinEntry(MessageData *data, Display *dpy,
	       int screen_num, WSMWinEntry *win_entry)
{
    int i;

    win_entry->num_windows = UnpackListNum(data);
    win_entry->windows = (Window *) XtMalloc(sizeof(Window) * win_entry->num_windows);

    for (i=0; i<win_entry->num_windows; i++)
      win_entry->windows[i] = UnpackWindow(data);

    win_entry->num_match_properties = UnpackListNum(data);
    win_entry->match_properties =
      (WindowProperty *) XtMalloc(sizeof(WindowProperty) *
				  win_entry->num_match_properties);

    for (i=0; i<win_entry->num_match_properties; i++)
      win_entry->match_properties[i] = UnpackProperty(data);
}

/*	Function Name: UnpackString
 *	Description: Unpacks a string from the protocol data stream.
 *	Arguments: data - Pointer to the data stream.
 *	Returns: the string from the data stream.
 *
 * NOTE: Data is modified to point to the next empty location in the stream.
 */

String
UnpackString(MessageData *data_ptr)
{
    register int i;
    int len = UnpackListNum(data_ptr);
    char *str, *top = XtMalloc((len + 1) * sizeof(char));
    
    for (str = top, i = 0; i < len; i++, str++) {
	*str = (char) UnpackCARD8(data_ptr);
    }
    *str = '\0';

    return((String) top);
}

/*	Function Name: UnpackCARD32
 *	Description: Unpacks an 32 bit value from the protocol data stream.
 *	Arguments: data - Pointer to the data stream.
 *	Returns: the CARD32 from the data stream.
 *
 * NOTE: data is modified to point to the next empty location in the stream.
 */

CARD32
UnpackCARD32(MessageData *data_ptr)
{
    CARD32 val = UnpackCARD16(data_ptr) << 16;
    val |= UnpackCARD16(data_ptr);
    return(val);
}

/*	Function Name: UnpackCARD16
 *	Description: Unpacks an 16 bit value from the protocol data stream.
 *	Arguments: data - Pointer to the data stream.
 *	Returns: the CARD16 from the data stream.
 *
 * NOTE: data is modified to point to the next empty location in the stream.
 */

CARD16
UnpackCARD16(MessageData *data_ptr)
{
    CARD16 val = UnpackCARD8(data_ptr) << 8;
    val |= UnpackCARD8(data_ptr);
    return(val);
}

/*	Function Name: UnpackCARD8
 *	Description: Unpacks an 8 bit value from the protocol data stream.
 *	Arguments: data - Pointer to the data stream.
 *	Returns: the CARD8 from the data stream.
 *
 * NOTE: data is modified to point to the next empty location in the stream.
 */

CARD8
UnpackCARD8(MessageData *data_ptr)
{
    CARD8 ret_val = *((char *) *data_ptr);

    *data_ptr = ((char *) *data_ptr) + 1;

    return(ret_val);
}
