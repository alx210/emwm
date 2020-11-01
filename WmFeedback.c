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
 * Motif Release 1.2.3
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmFeedback.c /main/6 1996/10/23 17:20:55 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */
#include "WmGlobal.h"
#include "WmResNames.h"

#define MWM_NEED_TIME16
#include "WmBitmap.h"

#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/Label.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>

#define MOVE_OUTLINE_WIDTH	2
#define FEEDBACK_BEVEL		2

#define DEFAULT_POSITION_STRING	"(0000x0000)"

#define  CB_HIGHLIGHT_THICKNESS  3

/*
 * include extern functions
 */
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmGraphics.h"
#ifdef PANELIST
#include "WmPanelP.h"  /* for typedef in WmManage.h */
#endif /* PANELIST */
#include "WmManage.h"
#include "WmColormap.h"
#include "stdio.h"


/*
 * Global Variables:
 */
static Cursor  waitCursor = (Cursor)0L;

/* see WmGlobal.h for index defines: */

#ifndef NO_MESSAGE_CATALOG
static char *confirm_mesg[4] = {"Switch to Default Behavior?",
				"Switch to Custom Behavior?",
                                "Restart Mwm?",
                                "QUIT Mwm?"};


void
initMesg()
{

    char * tmpString;

    /*
     * catgets returns a pointer to an area that is over written
     * on each call to catgets.  
     */

    tmpString = ((char *)GETMESSAGE(22, 12, "Switch to Default Behavior?"));
    if ((confirm_mesg[0] =
         (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(22, 2, "Insufficient memory for local message string")));
	confirm_mesg[0] = "Switch to Default Behavior?";
    }
    else
    {
	strcpy(confirm_mesg[0], tmpString);
    }

    tmpString = ((char *)GETMESSAGE(22, 13, "Switch to Custom Behavior?"));
    if ((confirm_mesg[1] =
         (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(22, 2, "Insufficient memory for local message string")));
	confirm_mesg[1] = "Switch to Custom Behavior?";
    }
    else
    {
	strcpy(confirm_mesg[1], tmpString);
    }

    if (MwmBehavior)
    {
	tmpString = ((char *)GETMESSAGE(22, 3, "Restart Mwm?"));
    }
    else
    {
	tmpString = ((char *)GETMESSAGE(22, 10, "Restart Workspace Manager?"));
    }
    if ((confirm_mesg[2] =
         (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(22, 5, "Insufficient memory for local message string")));
	if (MwmBehavior)
	{
	    confirm_mesg[2] = "Restart Mwm?";
	}
	else
	{
	    confirm_mesg[2] = "Restart Workspace Manager?";
	}
    }
    else
    {
	strcpy(confirm_mesg[2], tmpString);
    }



    if (MwmBehavior)
    {
	tmpString = ((char *)GETMESSAGE(22, 6, "QUIT Mwm?"));
    }
    else
    {
#ifdef MINIMAL_DT
	if (wmGD.dtLite)
	{
	    tmpString = ((char *)GETMESSAGE(22, 9, "Log out?"));
	}
	else
	{
	    tmpString = ((char *)GETMESSAGE(22, 11, "QUIT Workspace Manager?"));
	}
#else /* MINIMAL_DT */
	tmpString = ((char *)GETMESSAGE(22, 11, "QUIT Workspace Manager?"));
#endif /* MINIMAL_DT */
    }
    
    if ((confirm_mesg[3] =
         (char *)XtMalloc ((unsigned int) (strlen(tmpString) + 1))) == NULL)
    {
        Warning (((char *)GETMESSAGE(22, 8, "Insufficient memory for local message string")));
	if (MwmBehavior)
	{
	    confirm_mesg[3] = "QUIT Mwm?";
	}
	else
#ifdef MINIMAL_DT
	if (wmGD.dtLite)
	{
	    confirm_mesg[3] = "Log out?";
	}
	else
	{
	    confirm_mesg[3] = "QUIT Workspace Manager?";
	}
#else /* MINIMAL_DT */
	tmpString = ((char *)GETMESSAGE(22, 11, "QUIT Workspace Manager?"));
#endif /* MINIMAL_DT */
    }
    else
    {
	strcpy(confirm_mesg[3], tmpString);
    }


}
#else
static char *confirm_mesg[4] = {"Toggle to Default Behavior?",
				"Toggle to Custom Behavior?",
                                "Restart Mwm?",
                                "QUIT Mwm?"};

#endif
static char *confirm_widget[4] = {"confirmDefaultBehavior",
				  "confirmCustomBehavior",
				  "confirmRestart",
				  "confirmQuit"};


typedef void (*ConfirmFunc)(Boolean);
static ConfirmFunc confirm_func[4] = {Do_Set_Behavior,
				      Do_Set_Behavior,
				      Do_Restart,
				      Do_Quit_Mwm};


/*************************************<->*************************************
 *
 *  ShowFeedbackWindow(pSD, x, y, width, height, style)
 *
 *
 *  Description:
 *  -----------
 *  Pop up the window for moving and sizing feedback
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  x		- initial x-value
 *  y		- initial y-value
 *  width 	- initial width value
 *  height	- initial height value
 *  style	- show size, position, or both
 *  
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
void ShowFeedbackWindow (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height, unsigned long style)
{
    unsigned long        mask = 0;
    XSetWindowAttributes win_attribs;
    XWindowChanges       win_changes;
    int                  direction, ascent, descent;
    XCharStruct          xcsLocation;
    int                  winX, winY;
    int                  tmpX, tmpY;

    if ( (pSD->fbStyle = style) == FB_OFF)
	return;

    pSD->fbLastX = x;
    pSD->fbLastY = y;
    pSD->fbLastWidth = width;
    pSD->fbLastHeight = height;

    /*
     * Derive the size and position of the window from the text extents
     * Set starting position of each string 
     */
    XTextExtents(pSD->feedbackAppearance.font, DEFAULT_POSITION_STRING, 
		 strlen(DEFAULT_POSITION_STRING), &direction, &ascent, 
		 &descent, &xcsLocation);
    
    pSD->fbWinWidth = xcsLocation.width + 4*FEEDBACK_BEVEL;

    switch (pSD->fbStyle) 
    {
	case FB_SIZE:
	    pSD->fbSizeY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbWinHeight = (ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;

	case FB_POSITION:
	    pSD->fbLocY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbWinHeight = (ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;

	default:
	case (FB_SIZE | FB_POSITION):
	    pSD->fbLocY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbSizeY = pSD->fbLocY + ascent + descent;
	    pSD->fbWinHeight = 2*(ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;
    }

    if (pSD->feedbackGeometry) /* set by user */
    {
	unsigned int junkWidth, junkHeight;

	mask = XParseGeometry(pSD->feedbackGeometry, &tmpX, &tmpY,
			      &junkWidth, &junkHeight);
    }

    if (mask & (XValue|YValue))
    {
	winX = (mask & XNegative) ? 
	    DisplayWidth(DISPLAY, pSD->screen)  + tmpX - pSD->fbWinWidth : tmpX;
	winY = (mask & YNegative) ? 
	    DisplayHeight(DISPLAY, pSD->screen) + tmpY -pSD->fbWinHeight : tmpY;
    }
    else
    {
	winX = (DisplayWidth(DISPLAY, pSD->screen) - pSD->fbWinWidth)/2;
	winY = (DisplayHeight(DISPLAY, pSD->screen) -pSD->fbWinHeight)/2;
    }

    /* 
     * Put new text into the feedback strings
     */
    UpdateFeedbackText (pSD, x, y, width, height);

    /*
     * bevel the window border for a 3-D look
     */
    if ( (pSD->fbTop && pSD->fbBottom) ||
	 ((pSD->fbTop = AllocateRList((unsigned)2*FEEDBACK_BEVEL)) &&
	  (pSD->fbBottom = AllocateRList((unsigned)2*FEEDBACK_BEVEL))) )
    {
	pSD->fbTop->used = 0;
	pSD->fbBottom->used = 0;
	BevelRectangle (pSD->fbTop,
			pSD->fbBottom,
			0, 0, 
			pSD->fbWinWidth, pSD->fbWinHeight,
			FEEDBACK_BEVEL, FEEDBACK_BEVEL,
			FEEDBACK_BEVEL, FEEDBACK_BEVEL);
    }

    /*
     * Create window if not yet created, otherwise fix size and position
     */

    if (!pSD->feedbackWin)
    {

	/*
	 * Create the window
	 */

	mask = CWEventMask | CWOverrideRedirect | CWSaveUnder;
	win_attribs.event_mask = ExposureMask;
	win_attribs.override_redirect = TRUE;
	win_attribs.save_under = TRUE;

	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (pSD->feedbackAppearance.backgroundPixmap)
	{
	    mask |= CWBackPixmap;
	    win_attribs.background_pixmap =
				pSD->feedbackAppearance.backgroundPixmap;
	}
	else
	{
	    mask |= CWBackPixel;
	    win_attribs.background_pixel =
				pSD->feedbackAppearance.background;
	}

	pSD->feedbackWin = XCreateWindow (DISPLAY, pSD->rootWindow, 
					  winX, winY,
					  pSD->fbWinWidth, 
					  pSD->fbWinHeight,
					  0, CopyFromParent, 
					  InputOutput, CopyFromParent, 
					  mask, &win_attribs);
    }
    else
    {
	win_changes.x = winX;
	win_changes.y = winY;
	win_changes.width = pSD->fbWinWidth;
	win_changes.height = pSD->fbWinHeight;
	win_changes.stack_mode = Above;

	mask = CWX | CWY | CWWidth | CWHeight | CWStackMode;

	XConfigureWindow(DISPLAY, pSD->feedbackWin, (unsigned int) mask, 
	    &win_changes);
    }


    /*
     * Make the feedback window visible (map it)
     */

    if (pSD->feedbackWin)
    {
	/* Make sure the feedback window doesn't get buried */
	XRaiseWindow(DISPLAY, pSD->feedbackWin);
	XMapWindow (DISPLAY, pSD->feedbackWin);
	PaintFeedbackWindow(pSD);
    }

} /* END OF FUNCTION ShowFeedbackWindow */



/*************************************<->*************************************
 *
 *  PaintFeedbackWindow(pSD)
 *
 *
 *  Description:
 *  -----------
 *  Repaints the feedback window in response to exposure events
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
void PaintFeedbackWindow (WmScreenData *pSD)
{
    if (pSD->feedbackWin)
    {
	/* 
	 * draw beveling 
	 */
	if (pSD->fbTop->used > 0) 
	{
	    XFillRectangles (DISPLAY, pSD->feedbackWin, 
			     pSD->feedbackAppearance.inactiveTopShadowGC,
			     pSD->fbTop->prect, pSD->fbTop->used);
	}
	if (pSD->fbBottom->used > 0) 
	{
	    XFillRectangles (DISPLAY, pSD->feedbackWin, 
			     pSD->feedbackAppearance.inactiveBottomShadowGC,
			     pSD->fbBottom->prect, 
			     pSD->fbBottom->used);
	}

	/*
	 * clear old text 
	 */
	XClearArea (DISPLAY, pSD->feedbackWin, 
		    FEEDBACK_BEVEL, FEEDBACK_BEVEL,
		    pSD->fbWinWidth-2*FEEDBACK_BEVEL, 
		    pSD->fbWinHeight-2*FEEDBACK_BEVEL,
		    FALSE);

	/*
	 * put up new text
	 */
	if (pSD->fbStyle & FB_POSITION) 
	{
	    WmDrawString (DISPLAY, pSD->feedbackWin, 
			 pSD->feedbackAppearance.inactiveGC,
			 pSD->fbLocX, pSD->fbLocY, 
			 pSD->fbLocation, strlen(pSD->fbLocation));
	}
	if (pSD->fbStyle & FB_SIZE) 
	{
	    WmDrawString (DISPLAY, pSD->feedbackWin, 
			 pSD->feedbackAppearance.inactiveGC,
			 pSD->fbSizeX, pSD->fbSizeY, 
			 pSD->fbSize, strlen(pSD->fbSize));
	}
    }
}



/*************************************<->*************************************
 *
 *  HideFeedbackWindow (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Hide the feedback window
 *
 *
 *  Inputs:
 *  ------
 *  pDS		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void HideFeedbackWindow (WmScreenData *pSD)
{
    if (pSD->feedbackWin)
    {
	XUnmapWindow (DISPLAY, pSD->feedbackWin);
#ifndef OLD_COLORMAP
	ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    }
    pSD->fbStyle = FB_OFF;
}




/*************************************<->*************************************
 *
 *  UpdateFeedbackInfo (pSD, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Update the information in the feedback window
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen info
 *  x		- x-value
 *  y		- y-value
 *  width 	- width value
 *  height	- height value
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void UpdateFeedbackInfo (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height)
{
    /*
     * Currently the feedback window must always be redrawn to (potentially)
     * repair damage done by moving the configuration outline.  The feedback
     * repainting generally only needs to be done when the information
     * changes or the feedback window is actually overwritten by the
     * configuration outline.
     */

#ifdef NOTDONE
    /* only update if something changed */
    if (((pSD->fbStyle & FB_POSITION) &&
	 ((pSD->fbLastX != x) || (pSD->fbLastY != y))) || 
	((pSD->fbStyle & FB_SIZE) &&
	 ((pSD->fbLastWidth != width) || (pSD->fbLastHeight != height))))
#endif /* NOTDONE */
    {
	pSD->fbLastX = x;
	pSD->fbLastY = y;
	pSD->fbLastWidth = width;
	pSD->fbLastHeight = height;

	UpdateFeedbackText (pSD, x, y, width, height);

	PaintFeedbackWindow(pSD);
    }
}




/*************************************<->*************************************
 *
 *  UpdateFeedbackText (pSD, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Update the information in the feedback strings
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  x		- x-value
 *  y		- y-value
 *  width 	- width value
 *  height	- height value
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
void UpdateFeedbackText (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height)
{
    int         direction, ascent, descent;
    XCharStruct xcs;

    if (pSD->fbStyle & FB_POSITION) 
    {
	sprintf (pSD->fbLocation, "(%4d,%-4d)", x, y);
	XTextExtents(pSD->feedbackAppearance.font, pSD->fbLocation,
		 strlen(pSD->fbLocation), &direction, &ascent, 
		 &descent, &xcs);
	pSD->fbLocX = (pSD->fbWinWidth - xcs.width)/2;
    }

    if (pSD->fbStyle & FB_SIZE) 
    {
	sprintf (pSD->fbSize,     "%4dx%-4d", width, height);
	XTextExtents(pSD->feedbackAppearance.font, pSD->fbSize,
		 strlen(pSD->fbSize), &direction, &ascent, 
		 &descent, &xcs);
	pSD->fbSizeX = (pSD->fbWinWidth - xcs.width)/2;
    }
}



/*************************************<->*************************************
 *
 *  static void
 *  OkCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  QuestionBox Ok callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void OkCB (w, client_data, call_data)

   Widget w;
   caddr_t client_data;
   caddr_t call_data;
{
    WithdrawDialog (w);

    confirm_func[((WmScreenData *)client_data)->actionNbr] (False);

    wmGD.confirmDialogMapped = False;

} /* END OF FUNCTION OkCB */


/*************************************<->*************************************
 *
 *  static void
 *  CancelCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  QuestionBox Cancel callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void CancelCB (w, client_data, call_data)

   Widget w;
   caddr_t client_data;
   caddr_t call_data;
{
    WithdrawDialog (w);

    wmGD.confirmDialogMapped = False;

} /* END OF FUNCTION CancelCB */



/*************************************<->*************************************
 *
 *  void
 *  ConfirmAction (pSD,nbr)
 *
 *
 *  Description:
 *  -----------
 *  Post a QuestionBox and ask for confirmation.  If so, executes the
 *  appropriate action.
 *
 *
 *  Inputs:
 *  ------
 *  nbr = action number
 *  pSD->screen
 *  pSD->screenTopLevel
 *
 * 
 *  Outputs:
 *  -------
 *  actionNbr = current QuestionBox widget index.
 *  confirmW[actionNbr]  = QuestionBox widget.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

void ConfirmAction (WmScreenData *pSD, int nbr)
{
    Arg           args[8];
    register int  n;
    int           x, y;
    Dimension     width, height;
    Widget        dialogShellW = NULL;
    XmString	  messageString;
    static XmString	  defaultMessageString = NULL;


    /*
     * If there is a system modal window, don't post another
     * one.  We need to think about a way to let a new system
     * modal window be posted, and when unposted, restore the
     * modal state of the current system modal window.  
     */

    if(wmGD.systemModalActive)
    {
	return ;
    }

    if (pSD->confirmboxW[nbr] == NULL)
    /* First time for this one */
    {
#ifndef NO_MESSAGE_CATALOG
	/*
	 * Initialize messages
	 */
	initMesg();
#endif

        /* 
         * Create a dialog popup shell with explicit keyboard policy.
         */

        n = 0;
        XtSetArg(args[n], XmNx, (XtArgVal)
	         (DisplayWidth (DISPLAY, pSD->screen)/2)); n++;
        XtSetArg(args[n], XmNy, (XtArgVal)
	         (DisplayHeight (DISPLAY, pSD->screen)/2)); n++;
        XtSetArg(args[n], XtNallowShellResize, (XtArgVal) TRUE);  n++;
        XtSetArg(args[n], XtNkeyboardFocusPolicy, (XtArgVal) XmEXPLICIT);  n++;
        XtSetArg(args[n], XtNdepth, 
		(XtArgVal) DefaultDepth(DISPLAY, pSD->screen));  n++;
        XtSetArg(args[n], XtNscreen, 
		(XtArgVal) ScreenOfDisplay(DISPLAY, pSD->screen));  n++;

        dialogShellW =
    	        XtCreatePopupShell ((String) WmNfeedback, 
				    transientShellWidgetClass,
		                    pSD->screenTopLevelW, args, n);

        /* 
         * Create a QuestionBox as a child of the popup shell.
	 * Set traversalOn and add callbacks for the OK and CANCEL buttons.
	 * Unmanage the HELP button.
         */

        n = 0;
        XtSetArg(args[n], XmNdialogType, (XtArgVal) XmDIALOG_QUESTION); n++;
        XtSetArg(args[n], XmNmessageAlignment, (XtArgVal) XmALIGNMENT_CENTER);
	   n++;
        XtSetArg(args[n], XmNtraversalOn, (XtArgVal) TRUE); n++;

	/*
	 * In 1.2 confirmbox's widget name changed from the generic
	 * WmNconfirmbox (ie. 'confirmbox') to a more descriptive name
	 * so that each confirm dialog can be customized separately (e.g.
	 * "Mwm*confirmRestart*messageString: restart it?").
	 */

        pSD->confirmboxW[nbr] = 
	    XtCreateManagedWidget (confirm_widget[nbr], xmMessageBoxWidgetClass,
                                   dialogShellW, args, n);

        n = 0;
        XtSetArg(args[n], XmNmessageString, &messageString); n++;
        XtGetValues(pSD->confirmboxW[nbr], (ArgList) args, n);

	if (defaultMessageString == NULL)
	{
	    defaultMessageString = XmStringCreateLocalized ("");
	}

        n = 0;

	/*
	 * If the message string is the default, then put something
	 * 'reasonable' in instead.
	 */

	if (XmStringCompare( messageString, defaultMessageString ))
	{
            messageString = XmStringCreateLocalized(confirm_mesg[nbr]);
	    XtSetArg(args[n], XmNmessageString, (XtArgVal) messageString); n++;
	    XtSetValues(pSD->confirmboxW[nbr], (ArgList) args, n);
            XmStringFree(messageString);
	}

        n = 0;
        XtSetArg (args[n], XmNtraversalOn, (XtArgVal) TRUE); n++;
        XtSetArg (args[n], XmNhighlightThickness, 
		  (XtArgVal) CB_HIGHLIGHT_THICKNESS); n++;
#ifndef NO_MESSAGE_CATALOG
	XtSetArg(args[n], XmNlabelString, wmGD.okLabel); n++;
#endif
        XtSetValues ( XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
			    XmDIALOG_OK_BUTTON), args, n);
#ifndef NO_MESSAGE_CATALOG
	n--;
	XtSetArg(args[n], XmNlabelString, wmGD.cancelLabel); n++;
#endif
        XtSetValues ( XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
			    XmDIALOG_CANCEL_BUTTON), args, n);
        XtAddCallback (pSD->confirmboxW[nbr], XmNokCallback, 
	    (XtCallbackProc)OkCB, (XtPointer)pSD); 
        XtAddCallback (pSD->confirmboxW[nbr], XmNcancelCallback, 
	    (XtCallbackProc)CancelCB, (XtPointer)NULL); 

        XtUnmanageChild
	    (XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
		XmDIALOG_HELP_BUTTON));

        XtRealizeWidget (dialogShellW);

        /* 
         * Center the DialogShell in the display.
         */

        n = 0;
        XtSetArg(args[n], XmNheight, &height); n++;
        XtSetArg(args[n], XmNwidth, &width); n++;
        XtGetValues (dialogShellW, (ArgList) args, n);

        x = (DisplayWidth (DISPLAY, pSD->screen) - ((int) width))/2;
        y = (DisplayHeight (DISPLAY, pSD->screen) - ((int) height))/2;
        n = 0;
        XtSetArg(args[n], XmNx, (XtArgVal) x); n++;
        XtSetArg(args[n], XmNy, (XtArgVal) y); n++;
        XtSetValues (dialogShellW, (ArgList) args, n);

        ManageWindow (pSD, XtWindow(dialogShellW), MANAGEW_CONFIRM_BOX);
    }
    else
    {
        ReManageDialog (pSD, pSD->confirmboxW[nbr]);
    }

    pSD->actionNbr = nbr;

    XFlush(DISPLAY);

    wmGD.confirmDialogMapped = True;

} /* END OF FUNCTION ConfirmAction */



/*************************************<->*************************************
 *
 *  ShowWaitState (flag)
 *
 *
 *  Description:
 *  -----------
 *  Enter/Leave the wait state.
 *
 *
 *  Inputs:
 *  ------
 *  flag = TRUE for Enter, FALSE for Leave.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

void ShowWaitState (Boolean flag)
{
    char        *bits;
    char        *maskBits;
    unsigned int width;
    unsigned int height;
    unsigned int xHotspot;
    unsigned int yHotspot;
    Pixmap       pixmap;
    Pixmap       maskPixmap;
    XColor       xcolors[2];

    if (!waitCursor)
    {
#ifdef LARGECURSORS
	if (wmGD.useLargeCursors)
	{
	    width = time32_width;
	    height = time32_height;
	    bits = (char *)time32_bits;
	    maskBits = (char *)time32m_bits;
	    xHotspot = time32_x_hot;
	    yHotspot = time32_y_hot;
	}
	else
#endif /* LARGECURSORS */

	{
	    width = time16_width;
	    height = time16_height;
	    bits = (char *)time16_bits;
	    maskBits = (char *)time16m_bits;
	    xHotspot = time16_x_hot;
	    yHotspot = time16_y_hot;
	}

        pixmap = XCreateBitmapFromData (DISPLAY, 
		         DefaultRootWindow(DISPLAY), bits, 
			 width, height);

        maskPixmap = XCreateBitmapFromData (DISPLAY, 
		         DefaultRootWindow(DISPLAY), maskBits, 
			 width, height);
#ifdef INTEGRATION_TESTING_
        xcolors[1].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(DISPLAY));
        xcolors[0].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(DISPLAY));
#else /* INTEGRATION_TESTING */

        xcolors[0].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(DISPLAY));
        xcolors[1].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(DISPLAY));

#endif /* INTEGRATION_TESTING */
        XQueryColors (DISPLAY, 
		      DefaultColormapOfScreen(DefaultScreenOfDisplay
					      (DISPLAY)), 
		      xcolors, 2);
	waitCursor = XCreatePixmapCursor (DISPLAY, pixmap, maskPixmap,
	                                  &(xcolors[0]), &(xcolors[1]),
                                          xHotspot, yHotspot);
        XFreePixmap (DISPLAY, pixmap);
        XFreePixmap (DISPLAY, maskPixmap);
    }

    if (flag)
    {
	XGrabPointer (DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			0, GrabModeAsync, GrabModeAsync, None, 
			waitCursor, CurrentTime);
	XGrabKeyboard (DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			GrabModeAsync, GrabModeAsync, CurrentTime);
    }
    else
    {
	XUngrabPointer (DISPLAY, CurrentTime);
	XUngrabKeyboard (DISPLAY, CurrentTime);
    }

} /* END OF FUNCTION ShowWaitState */



/*************************************<->*************************************
 *
 *  InitCursorInfo ()
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a server supports large cursors.  It it
 *  does large feedback cursors are used in some cases (wait state and
 *  system modal state); otherwise smaller (16x16) standard cursors are used.
 *
 *  Outputs:
 *  -------
 *  wmGD.useLargeCusors = set to True if larger cursors are supported.
 * 
 *************************************<->***********************************/

void InitCursorInfo (void)
{
    unsigned int cWidth;
    unsigned int cHeight;

    wmGD.useLargeCursors = False;

    if (XQueryBestCursor (DISPLAY, DefaultRootWindow(DISPLAY), 
	32, 32, &cWidth, &cHeight))
    {
	if ((cWidth >= 32) && (cHeight >= 32))
	{
	    wmGD.useLargeCursors = True;
	}
    }

} /* END OF FUNCTION InitCursorInfo */






