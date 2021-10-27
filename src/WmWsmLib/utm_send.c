/* $XConsortium: utm_send.c /main/5 1995/07/15 20:38:59 drk $ */
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

#include <stdio.h>
#include <Xm/TransferT.h>
#include "utm_send.h"



/*
 * The following keeps track of multiple utm requests and
 * pulls the right data off
 */

typedef struct _DataQueueRec {
  XtPointer data;
  struct _DataQueueRec *next, *prev;
} DataQueueRec;

static DataQueueRec *dataQueue, *dataQueueTail  = NULL;


static void EnqueueUtmData(
XtPointer
);

static XtPointer DequeueUtmData(
void
);


/*================================================================*
 | The following functions are used to handle selection/UTM       |
 | requests made from the REQUESTORS side to the owner of the     |
 | selection.                                                     |
 | To send a request, the only function needed is UTMSendMessage. |
 *================================================================*/

/*----------------------------------------------------------------*
 |                         UTMSendMessage                         |
 | Send a message with parameters.                                |
 | Arguments:                                                     |
 |         widget - requestor. MUST have destination callback set |
 |                  to UTMDestinationProc below.
 |         selection - selection to convert against.              |
 |         target - target to make request against.               |
 |         param - data to send to selection owner.               |
 |         param_len - length of param in fmt units.              |
 |         param_fmt - param format (should be WSM_PROTO_FMT).    |
 |         callback - function to invoke when done.               |
 |         closure - data to pass to the callback.                |
 |         time - timestamp of the event; not CurrentTime.        | 
 *----------------------------------------------------------------*/
void
UTMSendMessage(
     Widget w,
     Atom selection,
     Atom target,
     XtPointer param,
     unsigned long paramLen,
     int paramFmt,
     XtCallbackProc doneProc,
     XtPointer closure,
     Time time)
{
  UTMPackageRec *pUtmData;


  /* make sure timestamp is valid
  if (time == 0) time = GetTimestamp(XtDisplay(w)); */

  /* allocates and builds utm package to send. */
  pUtmData = (UTMPackageRec *) XtMalloc(sizeof(UTMPackageRec));
  pUtmData->fmt = paramFmt;
  pUtmData->len = paramLen;
  pUtmData->target = target;
  pUtmData->param = param;
  pUtmData->doneProc = doneProc;
  pUtmData->closure = closure;

  /*
   * queue-up the data which is pulled off in the
   * UTMDestinationProc below. Note that the widget
   * passed in MUST have its destinationCallback set
   * to UTMDestionationProc!
   */

  EnqueueUtmData(pUtmData);

  /*
   * This causes the DestinationCB to be invoked.
   * That is where that param data is transfered.
   */
  if (!XmeNamedSink(w,
		    selection,
		    XmCOPY,
		    NULL, /* location_data. holds param data (client data),
			     free in TransferDone proc. */
		    time))
    fprintf(stderr, "UTM Error: UTMSendMessage failed.\n");

} /* UTMSendMessage */



/*----------------------------------------------------------------*
 |                      UTMDestinationProc                        |
 | Invoked by UTM when a sink has been set-up and a request       |
 | initiated against an owner of a selection.                     |
 | Arguments: w - the requestor widget                            |
 |            clientData - not used. data is in the queue.        |
 |            callData - pointer to the data passed in the call   |
 |                       XmeNamedSink().  This will assume that   |
 |                       the structure pointed to is a UTMDataRec.|
 | Returns: none                                                  |
 |                                                                |
 | Comments:                                                      |
 |    Xt's selection mechanism will not work if the same time-    |
 |    stamp is used. prevTimeStamp contains the time of the last  |
 |    conversion and if same (or greater) it is incremented.      |
 *----------------------------------------------------------------*/
void
UTMDestinationProc (
     Widget w,
     XtPointer clientData,
     XtPointer callData)
{
  static Time prevTimestamp = 0;
  static Time increment = 0;
  Time time;
  XmDestinationCallbackStruct *dcs = (XmDestinationCallbackStruct *)callData;
  UTMPackageRec *pUtmData;

  if (dcs == NULL)
    {
      /* bad data - just return. */
      return;
    }

  /* if duplicate timestamps are used, then make sure they're different. */
  if (prevTimestamp == dcs->time)
    {
      time = prevTimestamp + (++increment);
    }
  else
    {
      increment = 0;
      prevTimestamp = time = dcs->time;
    }

  /* pull off the real client data */
  pUtmData = (UTMPackageRec *)DequeueUtmData();
  
  /* if no UTMData, then transfer can't be done. Punt. */
  if (pUtmData == NULL)
    return;
  
  /* Setup the parameters to pass. */
  XmTransferSetParameters(dcs->transfer_id,
			  pUtmData->param, /* ptr to parameter data. */
			  pUtmData->fmt, /* param format (8,16,32) */
			  pUtmData->len, /* size of param in fmt units */
			  dcs->selection); /* not used */
  
  /* Make the transfer. This invokes the selection owner's ConvertCB.
   * When done, UTMReplyReceived callback proc is invoked.
   */
  XmTransferValue(dcs->transfer_id,
		  pUtmData->target, /* this is the target to convert against.*/
		  pUtmData->doneProc, /* an XtCallbackProc called when done. */
		  pUtmData->closure,  /* client data passed to done proc */
		  time);
      
  /* don't need the structure any more. */
  XtFree((char*)pUtmData);

} /* UTMDestinationProc */


/*----------------------------------------------------------------*
 |                          EnqueueUtmData                        |
 | Puts data onto the queue between sending messages and having   |
 | the destination callback invoked.                              |
 *----------------------------------------------------------------*/
static void
EnqueueUtmData(XtPointer data)
{
  DataQueueRec *dataRec = (DataQueueRec *)XtMalloc(sizeof(DataQueueRec));

  dataRec->data = data;
  dataRec->next = dataQueue;
  dataRec->prev = NULL;

  if (dataQueue == NULL)
    dataQueueTail = dataRec;
  else
    dataQueue->prev = dataRec;
  dataQueue = dataRec;

  return;
}



/*----------------------------------------------------------------*
 |                          DequeueUtmData                        |
 | Retreives data from the data queue after the destination proc  |
 | was called. Just grab the last entry in the queue.             |
 *----------------------------------------------------------------*/
static XtPointer
DequeueUtmData()
{
  DataQueueRec *ptr = dataQueueTail;
  XtPointer dataPtr = NULL;

  if (ptr)
    {
      dataQueueTail = ptr->prev;

      if (ptr->prev != NULL)
	ptr->prev->next = NULL;
      else
	dataQueue = NULL;
 
      dataPtr = ptr->data;
      XtFree((char*)ptr);
    }
  else
    fprintf(stderr, "UTM ERROR: Data not found in queue.\n");

  return (dataPtr);
}
