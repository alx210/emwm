/* $XConsortium: recv.c /main/5 1995/07/15 20:38:46 drk $ */
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
#include "utm_send.h"
#include <X11/Xatom.h>
#include <Xm/TransferP.h>	/* for XmeNamedSource() */

#ifdef JUNK
static Boolean ConvertProc(
Widget, Atom *, Atom *, XtPointer, unsigned long, int, Atom *,
XtPointer *, unsigned long *, int *
);
#endif /* JUNK */


/*	Function Name: WSMDefaultOwnSelection
 *	Description: This is the default routine that owns the proper
 * 		     selection and the processes incomming messages
 *                   from the Wsm protocol.
 *	Arguments: w - A widget on the screen we want to own the
 *                     WSM or WM selection for.
 *                     NOTE: THIS MUST HAVE A XmNconvertCallback RESOURCE!
 *                 client_type - The type of client we are.
 *                 request_callback - The callback call when a message
 *                                    is received.
 *                 request_data - Data passed to the request callback.
 *	Returns: True if no one currently owns the selection and I have
 *               taken ownership.
 * 
 * NOTE: The reply that is filled in by the application programmer is
 *       passed to FreeReply, so that memory may be freed.  If you
 *       want to take advantage of this, check wsm_proto.h or free.c
 *       to see what is freed for each type of protocol request, and
 *       then set the reply->any.allocated field to True to activate 
 *       automatic memory freeing.
 */

Boolean
WSMDefaultOwnSelection(Widget w, WSMClientType client_type,
		       WSMRequestCallbackFunc request_callback,
		       XtPointer request_data)
{
    Time time;
    Display *dpy = XtDisplay(w);
    Atom own_selection =
	_WSMGetSelectionAtom(dpy, 
			     XScreenNumberOfScreen(XtScreen(w)), client_type);
    
    if (XGetSelectionOwner(dpy, own_selection) != None) {
	/*
	 * Someone out there already owns this selection, we should give
	 * up and return.
	 */

        fprintf(stderr,"Error - Someone out there already owns this selection.\n");

	return(False);
    }
    
    if (!XtIsRealized(w)) {
	fprintf(stderr, "%s must be realized, and is not.\n",
		"Programmer Error: Widget passed to WSMDefaultOwnSelection");
	return(False);
    }

    WSMRegisterRequestCallback(XtDisplay(w), 
 			       XScreenNumberOfScreen(XtScreen(w)),
 			       request_callback, request_data);
    
    /*
     * NOTE - w MUST have its XmNconvertCallback set properly,
     *        otherwise, no conversions will be handled!
     */
    time = GetTimestamp(XtDisplay(w)); /* CurrentTime or Zero is a no-no!*/
    XmeNamedSource(w, own_selection, time);

    return(True);
}

/*	Function Name: WSMRegisterRequestCallback
 *	Description: Registers the callback that will be called when 
 *                   a request comes in on the WSM protocol.
 *	Arguments: disp - The display.
 *                 scr_num - The Screen number.
 *                 callback, data - The proc to register and the data
 *                                  to send to it.
 *	Returns: none.
 * 
 * NOTE: The reply that is filled in by the application programmer is
 *       passed to FreeReply, so that memory may be freed.  If you
 *       want to take advantage of this, check wsm_proto.h or free.c
 *       to see what is freed for each type of protocol request, and
 *       then set the reply->any.allocated field to True to activate 
 *       automatic memory freeing.
 */

void
WSMRegisterRequestCallback(Display *disp, int scr_num,
			   WSMRequestCallbackFunc callback, XtPointer data)
{
    WSMScreenInfo *scr_info;

    scr_info = _WSMGetScreenInfo(disp, scr_num);

    /*
     * Since only one client can own a particular selection, we can hang
     * this data off of the screen data structure.  This is necessary because
     * No data is passed to the ConvertProc by Xm or Xt.
     *
     * NOTE: We do not need to have room for one callback for the WSM and 
     *       one for the WM, since any given client will be one or the other,
     *       not both.
     */
	
    scr_info->request_callback = callback;
    scr_info->request_data = data;
}


/*	Function Name: WSMIsKnownTarget
 *	Description: Returns True if this target is part of the WSM protocol.
 *	Arguments: w - Any widget on the screen of the client 
 *                     we are talking to.
 *                 target - the target to check if part of the protocol.
 *	Returns:  True if this target is part of the WSM protocol.
 *
 * NOTE: This could be extended to know about extensions.
 */

Boolean
WSMIsKnownTarget(Widget w, Atom target)
{
    WSMDispInfo * disp_info = _WSMGetDispInfo(XtDisplay(w));

    /*
     * Can't switch on dynamic data, sigh...
     */
    
    if (disp_info->connect == target)
	return(True);
    if (disp_info->extensions == target)
	return(True);
    if (disp_info->config_fmt == target)
	return(True);
    if (disp_info->get_state == target)
	return(True);
    if (disp_info->set_state == target)
	return(True);
    if (disp_info->reg_window == target)
	return(True);
    if (disp_info->get_background == target)
	return(True);
    if (disp_info->set_background == target)
	return(True);
    if (disp_info->wm_windows == target)
	return(True);
    if (disp_info->wm_focus == target)
	return(True);
    if (disp_info->wm_pointer == target)
	return(True);

    return(False);
}

/*	Function Name: WSMGetTargetList
 *	Description: Returns the list of targets understood by the WSM
 *                   protocol.
 *	Arguments: w - Any widget on the same screen as the client we
 *                     are talking to.
 *                 include_defaults - Whether or not to include the
 *                                    targets TIMESTAMP, MULTIPLE, and TARGETS.
 *                                    in the returned list.
 * RETURNED        len_ret - The number of atoms returned.
 *	Returns: The targets atom list.  This list has been allocated with 
 *               XtMalloc, and must be free'd by the caller.
 */

Atom *
WSMGetTargetList(Widget w, Boolean include_defaults, unsigned long *len_ret)
{
    WSMDispInfo *disp_info = _WSMGetDispInfo(XtDisplay(w));
    register int i;
    Atom *list;

    *len_ret = NUM_WSM_TARGETS;
    if (include_defaults)
	*len_ret += NUM_EXTRA_TARGETS;

    list = (Atom *) XtMalloc(sizeof(Atom) * (*len_ret));

    i = 0;
    list[i++] = disp_info->connect;
    list[i++] = disp_info->extensions;
    list[i++] = disp_info->config_fmt;
    list[i++] = disp_info->get_state;
    list[i++] = disp_info->set_state;
    list[i++] = disp_info->reg_window;
    list[i++] = disp_info->get_background;
    list[i++] = disp_info->set_background;
    list[i++] = disp_info->wm_windows;
    list[i++] = disp_info->wm_focus;
    list[i++] = disp_info->wm_pointer;
    if (include_defaults) {
	list[i++] = disp_info->targets;
	list[i++] = disp_info->multiple;
	list[i++] = disp_info->timestamp;
    }

    return(list);
}

/*	Function Name: WSMProcessProtoTarget
 *	Description: Unpacks the data sent across to us that matches the
 *                   target, then calls the request_callback proceedure.
 *	Arguments: w - Any widget that is on the same screen as the
 *                     client that we are talking to.
 *                 target - The target that defines which request this is.
 *                 input, input_len, input_fmt - The input data.
 *                 return_type - This is always set to the target IFF we
 *                               know how to process this request.
 *                 output, output_len, output_fmt - The data to send back
 *                                                  to the requester.
 *	Returns: True if we know how to process this message and there
 *               are no errors
 */

Boolean
WSMProcessProtoTarget(Widget w, Atom target, XtPointer input, 
		      unsigned long input_len, int input_fmt,
		      Atom *return_type, XtPointer *output, 
		      unsigned long *output_len, int *output_fmt)
{
    Display *dpy;
    int scr_num;
    WSMScreenInfo *screen_info;
    WSMRequest request;
    WSMReply reply;    
    
    /*
     * Check the Target to make sure it is valid.
     * Check the format to make sure it is WSM_PROTO_FMT.
     */

    if (!WSMIsKnownTarget(w, target))
	return(False);

    if (input_fmt != WSM_PROTO_FMT) {
	fprintf(stderr, "Format of the request must be %d\n", WSM_PROTO_FMT);
	return(False);
    }

    dpy = XtDisplay(w);
    scr_num = XScreenNumberOfScreen(XtScreen(w));

    /*
     * Unpack up the request from the wire.
     */
    
    _WSMUnpackRequest(dpy, scr_num, (MessageData) input, input_len, 
		      _WSMTargetToReqType(dpy, target), &request);

    /*
     * Call the app's callback function to process the request.
     */

    screen_info = _WSMGetScreenInfo(dpy, scr_num);
    reply.any.type = request.any.type;
    reply.any.allocated = False;
    (*screen_info->request_callback)(w, screen_info->request_data,
				     &request, &reply);
    /*
     * Pack up the reply and send it back.
     */

    *output = (XtPointer) _WSMPackReply(dpy, scr_num, &reply, output_len);
    *output_fmt = WSM_PROTO_FMT;
    *return_type = target;	/* Is this the right return type? */

    FreeRequest(&request);
    FreeReply(&reply);

    return(TRUE);
}

/************************************************************
 *
 *  Internal routines.
 *
 ************************************************************/


#ifdef JUNK
/*================================================================*
 | The following functions are used to handle selection/UTM       |
 | conversion requests that are received by the selection OWNER.  |
 *================================================================*/


/*----------------------------------------------------------------*
 |                          UTMConvertProc                        |
 | This is the UTM conversion proc called when a request comes in |
 | for the selection owner.                                       |
 |                                                                |
 | NOTE - This function MUST have been set up as the callback for |
 |        the utm widget's XmNconvertCallback procedure!          |
 |                                                                |
 | Arguments: w - The widget making the request.                  |
 |            clientData - not used.                              |
 |            callData - the UTM convert callback structure.      |
 *----------------------------------------------------------------*/
static void
UTMConvertProc(Widget w, XtPointer clientData, XtPointer callData)
{
  int scr = XScreenNumberOfScreen(XtScreen(w));
  XmConvertCallbackStruct *ccs = (XmConvertCallbackStruct *)callData;
  Atom lose_sel = XInternAtom(XtDisplay(w), "_MOTIF_LOSE_SELECTION", False);
  WSMScreenInfo *scrInfo = _WSMGetScreenInfo(XtDisplay(w), scr);

  /*
   * Check if the callback was invoked for the right reason.
   * The reason field is not set to anything interresting by the transfer
   * code, so check the selection.
   */
  if ((!ccs) || ((ccs->selection != scrInfo->wm_selection) &&
		 (ccs->selection != scrInfo->wsm_selection)))
    return;

  if (ccs->target == lose_sel)
    {
      /* Done with the conversion - free any data used. */
    }

  /*
   * Handle a conversion request with parameter data.
   */
  else
    {
      ConvertProc (w, &(ccs->selection), &(ccs->target),
		   ccs->parm,
		   ccs->parm_length,
		   ccs->parm_format,
		   &(ccs->type),
		   &(ccs->value),
		   &(ccs->length),
		   &(ccs->format));
    }
}



/*	Function Name: ConvertProc
 *	Description: This is the conversion proc. called when a request
 *                   comes in that we need to process.
 *	Arguments: w - The widget that owns the selection?
 *                 selection - The selection to be converted.
 *                 target - The target to convert.
 *                 return_type - The type of the return stream.
 *                 output - The output data stream.
 *                 output_len - The length of the output stream.
 *                 output_fmt - The format of the output stream.
 *	Returns: True if we can convert the selection, False otherwise.
 */

/*ARGSUSED*/
static Boolean
ConvertProc(Widget w, Atom *selection, Atom *target,
	    XtPointer input, unsigned long input_len, int input_fmt,
	    Atom *return_type, XtPointer *output, unsigned long *output_len,
	    int *output_fmt)
{
    WSMDispInfo *disp_info;

    /* set up some defaults. selection code doesn't like garbage! */
    *output = NULL;
    *output_len = 0;
    *output_fmt = 8;

    disp_info = _WSMGetDispInfo(XtDisplay(w));

    /*
     * Since this function is only registered for one selection, I am going
     * to assume that the intrinsics didn't give me the wrong thing.
     */

    if (*target == disp_info->targets) {
	*return_type = XA_STRING;
	*output = (XtPointer) WSMGetTargetList(w, TRUE, output_len);
	*output_fmt = 32;
	return(True);
    }
    /*
     * Intrinsics will handle MULTIPLE and TIMESTAMP for me.
     */

    if (WSMProcessProtoTarget(w, *target, input, input_len, input_fmt,
			      return_type, output, output_len, output_fmt))
    {
	return(TRUE);
    }

    return(False);		/* unknown type, returning... */
}
#endif /*JUNK*/
