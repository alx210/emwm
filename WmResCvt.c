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
 * Motif Release 1.2.4
*/ 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmResCvt.c /main/4 1995/11/01 11:49:54 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResNames.h"
#include <ctype.h>
#include <stdio.h>

#ifndef NO_MULTIBYTE
#include <stdlib.h>
#endif

#ifndef MOTIF_ONE_DOT_ONE
#include <Xm/XmosP.h>
#endif

/*
 * include extern functions
 */

#include "WmResParse.h"

/*
 * Function Declarations:
 */

unsigned char *NextToken ();
long           DecStrToL ();

#include "WmResCvt.h"




/*************************************<->*************************************
 *
 *  AddWmResourceConverters (args)
 *
 *
 *  Description:
 *  -----------
 *  This function adds resource type converters for mwm specific resource
 *  types to the X Toolkit collection.
 *
 *
 *  Inputs:
 *  ------
 *  XXinput = ...
 *
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  XXOutput = ...
 *
 *
 *  Comments:
 *  --------
 *  XXComments ...
 * 
 *************************************<->***********************************/

void AddWmResourceConverters (void)
{
#ifdef WSM
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRAbsentMapBehavior, 
	(XtConverter)WmCvtStringToAMBehavior, NULL, 0);
#endif /* WSM */
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRCFocusPolicy, 
	(XtConverter)WmCvtStringToCFocus, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRClientDecor, 
	(XtConverter)WmCvtStringToCDecor, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRClientFunction, 
	(XtConverter)WmCvtStringToCFunc, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRFrameStyle, 
	(XtConverter)WmCvtStringToFrameStyle, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRIconDecor, 
	(XtConverter)WmCvtStringToIDecor, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRIconPlacement, 
	(XtConverter)WmCvtStringToIPlace, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRKFocusPolicy, 
	(XtConverter)WmCvtStringToKFocus, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRSize, 
	(XtConverter)WmCvtStringToSize, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRShowFeedback, 
	(XtConverter)WmCvtStringToShowFeedback, NULL, 0);
    XtAppAddConverter (wmGD.mwmAppContext, XtRString, WmRUsePPosition, 
	(XtConverter)WmCvtStringToUsePPosition, NULL, 0);

} /* END OF FUNCTION AddWmResourceConverters */

#ifdef WSM

/*************************************<->*************************************
 *
 *  WmCvtStringToAMBehavior (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to an absent map behavior description.
 *
 *
 *  Inputs:
 *  ------
 *  args = additional XrmValue arguments to the converter - NULL here
 *
 *  numArgs = number of XrmValue arguments - 0 here
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToAMBehavior (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;

/*
 * Absent map behavior policies:
 */
#define AMAP_BEHAVIOR_ADD_STR           (unsigned char *)"add"
#define AMAP_BEHAVIOR_MOVE_STR          (unsigned char *)"move"
#define AMAP_BEHAVIOR_IGNORE_STR        (unsigned char *)"ignore"

    /*
     * Convert the absent map behavior policy resource value:
     */

    if (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'A') || (*pch == 'a'))
	{
	    if (StringsAreEqual (pch, AMAP_BEHAVIOR_ADD_STR, len))
	    {
	        cval = AMAP_BEHAVIOR_ADD;
	        fHit = True;
	    }
	}

	else if ((*pch == 'M') || (*pch == 'm'))
	{
	    if (StringsAreEqual (pch, AMAP_BEHAVIOR_MOVE_STR, len))
	    {
	        cval = AMAP_BEHAVIOR_MOVE;
	        fHit = True;
	    }
	}

	else if ((*pch == 'I') || (*pch == 'i'))
	{
	    if (StringsAreEqual (pch, AMAP_BEHAVIOR_IGNORE_STR, len))
	    {
	        cval = AMAP_BEHAVIOR_IGNORE;
	        fHit = True;
	    }
        }
    }

    if (!fHit)
    {
	cval =  AMAP_BEHAVIOR_ADD;
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t)&cval;


} /* END OF FUNCTION WmCvtStringToAMBehavior */
#endif /* WSM */


/*************************************<->*************************************
 *
 *  WmCvtStringToCFocus (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a colormap focus policy description.
 *
 *
 *  Inputs:
 *  ------
 *  args = additional XrmValue arguments to the converter - NULL here
 *
 *  numArgs = number of XrmValue arguments - 0 here
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToCFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;

/*
 * Colormap focus policies:
 */

#define CMAP_FOCUS_EXPLICIT_STR		(unsigned char *)"explicit"
#define CMAP_FOCUS_KEYBOARD_STR		(unsigned char *)"keyboard"
#define CMAP_FOCUS_POINTER_STR		(unsigned char *)"pointer"


    /*
     * Convert the colormap focus policy resource value:
     */

    if (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'E') || (*pch == 'e'))
	{
	    if (StringsAreEqual (pch, CMAP_FOCUS_EXPLICIT_STR, len))
	    {
	        cval = CMAP_FOCUS_EXPLICIT;
	        fHit = True;
	    }
	}

	else if ((*pch == 'K') || (*pch == 'k'))
	{
	    if (StringsAreEqual (pch, CMAP_FOCUS_KEYBOARD_STR, len))
	    {
	        cval = CMAP_FOCUS_KEYBOARD;
	        fHit = True;
	    }
	}

	else if ((*pch == 'P') || (*pch == 'p'))
	{
	    if (StringsAreEqual (pch, CMAP_FOCUS_POINTER_STR, len))
	    {
	        cval = CMAP_FOCUS_POINTER;
	        fHit = True;
	    }
        }
    }

    if (!fHit)
    {
	cval =  CMAP_FOCUS_KEYBOARD;
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t)&cval;


} /* END OF FUNCTION WmCvtStringToCFocus */



/*************************************<->*************************************
 *
 *  WmCvtStringToCDecor (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a mwm client window frame decoration
 *  description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *
 *  Comments:
 *  --------
 *  o Accepts the following syntax:
 *
 *    CDecor ::= [sign] decor_spec [decor_spec ...]
 *
 *    sign ::= ['+' | '-']
 *
 *    decor_spec ::= [sign] decor_name
 *
 *    decor_name ::=  "all" | "none" | "title" | "titlebar" |
 *                     "menu" | "minimize" | "maximize" | "resize" 
 * 
 *************************************<->***********************************/

void WmCvtStringToCDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;
    Boolean    fAddNext = True;
/*
 * Client decoration parts:
 */

#define WM_DECOR_ALL_STR		(unsigned char *)"all"
#define WM_DECOR_NONE_STR		(unsigned char *)"none"
#define WM_DECOR_BORDER_STR		(unsigned char *)"border"
#define WM_DECOR_RESIZEH_STR		(unsigned char *)"resizeh"
#define WM_DECOR_TITLE_STR		(unsigned char *)"title"
#define WM_DECOR_TITLEBAR_STR		(unsigned char *)"titlebar"
#define WM_DECOR_MINIMIZE_STR		(unsigned char *)"minimize"
#define WM_DECOR_MAXIMIZE_STR		(unsigned char *)"maximize"
#define WM_DECOR_MENU_STR		(unsigned char *)"menu"
#define WM_DECOR_RESIZE_STR		(unsigned char *)"resize"

    /*
     * Check first token. If '-' we subtract from all decoration.
     * Otherwise, we start with no decoration and add things in.
     */
    if (*pch && 
	(NextToken (pch, &len, &pchNext)) && 
	(*pch == '-'))
    {
	cval = WM_DECOR_ALL;   
	fHit = True;
    }
    else
    {
	cval = WM_DECOR_NONE;
    }


    while (*pch && NextToken(pch, &len, &pchNext)) 
    {
	   /*
	    * Strip off "sign" if prepended to another token, and process
	    * that token the next time through.
	    */

	if (*pch == '+')
	{
	    if (len != 1)
	    {
	        pchNext = pch + 1;
	    }
	    fAddNext = TRUE;
	}

	else if (*pch == '-')
	{
	    if (len != 1) 
	    {
	        pchNext = pch + 1;
	    }
	    fAddNext = FALSE;
	}

	else if ((*pch == 'A') || (*pch == 'a'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_ALL_STR,len))  
	    {
	        cval = fAddNext ? (cval | WM_DECOR_ALL) : 
				 (cval & ~WM_DECOR_ALL);
	        fHit = True;
	    }
	}
	       
	else if ((*pch == 'N') || (*pch == 'n'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_NONE_STR,len))
	    {
	        /* don't bother adding or subtracting nothing */
	        fHit = True;
	    }
	}

	else if ((*pch == 'T') || (*pch == 't'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_TITLE_STR,len))
	    {
	        cval = fAddNext ? (cval | WM_DECOR_TITLE) : 
	    	                  (cval & ~WM_DECOR_TITLEBAR);
		fHit = True;
    	    }
	    else if (StringsAreEqual(pch, WM_DECOR_TITLEBAR_STR,len))  
	    {
		cval = fAddNext ? (cval | WM_DECOR_TITLEBAR) : 
		  	          (cval & ~WM_DECOR_TITLEBAR);
		fHit = True;
	    }
	}
	       
	else if ((*pch == 'M') || (*pch == 'm'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_MINIMIZE_STR,len)) 
	    {
		cval = fAddNext ? (cval | WM_DECOR_MINIMIZE) : 
				  (cval & ~MWM_DECOR_MINIMIZE);
		fHit = True;
	    }
	    else if (StringsAreEqual(pch, WM_DECOR_MAXIMIZE_STR,len))  
	    {
		cval = fAddNext ? (cval | WM_DECOR_MAXIMIZE) : 
				  (cval & ~MWM_DECOR_MAXIMIZE);
		fHit = True;
	    }
	    else if (StringsAreEqual(pch, WM_DECOR_MENU_STR,len))
	    {
		cval = fAddNext ? (cval | WM_DECOR_SYSTEM) : 
				  (cval & ~MWM_DECOR_MENU);
		fHit = True;
	    }
	}

	else if ((*pch == 'R') || (*pch == 'r'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_RESIZE_STR,len) ||
	        StringsAreEqual(pch, WM_DECOR_RESIZEH_STR,len)) 
	    {
		cval = fAddNext ? (cval | WM_DECOR_RESIZEH) : 
				  (cval & ~MWM_DECOR_RESIZEH);
		fHit = True;
	    }
	}
	       
	else if ((*pch == 'B') || (*pch == 'b'))
	{
	    if (StringsAreEqual(pch, WM_DECOR_BORDER_STR,len))
	    {
		cval = fAddNext ? (cval | WM_DECOR_BORDER) : 
				  (cval & ~WM_DECOR_BORDER);
		fHit = True;
	    }
	}

	pch = pchNext;
    }

    if (!fHit) cval =  WM_DECOR_ALL;

    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t) &cval;

} /* END OF FUNCTION WmCvtStringToCDecor */



/*************************************<->*************************************
 *
 *  WmCvtStringToCFunc (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a mwm client-applicable function
 *  description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *
 *  Comments:
 *  --------
 *  o Accepts the following syntax:
 *
 *    CFunc ::= [sign] func_spec [func_spec ...]
 *
 *    sign ::= ['+' | '-']
 *
 *    func_spec ::= [sign] func_name
 *
 *    func_name ::=  "all" | "none" | "resize" | "move" | "minimize" |
 *                   "maximize" | "close" 
 * 
 *************************************<->***********************************/

void WmCvtStringToCFunc (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;
    Boolean    fAddNext = True;

/*
 * Client-applicable functions:
 */

#define WM_FUNC_ALL_STR			(unsigned char *)"all"
#define WM_FUNC_NONE_STR		(unsigned char *)"none"
#define WM_FUNC_RESIZE_STR		(unsigned char *)"resize"
#define WM_FUNC_MOVE_STR		(unsigned char *)"move"
#define WM_FUNC_MINIMIZE_STR		(unsigned char *)"minimize"
#define WM_FUNC_MAXIMIZE_STR		(unsigned char *)"maximize"
#define WM_FUNC_CLOSE_STR		(unsigned char *)"close"

    /*
     * Check first token. If '-' we subtract from all functions.
     * Otherwise, we start with no functions and add things in.
     */

    if (*pch && 
	(NextToken (pch, &len, &pchNext)) && 
	(*pch == '-'))
    {
	cval = WM_FUNC_ALL;   
	fHit = True;
    }
    else
    {
	cval = WM_FUNC_NONE;
    }


    while (*pch && NextToken(pch, &len, &pchNext)) 
    {
	   /*
	    * Strip off "sign" if prepended to another token, and process
	    * that token the next time through.
	    */

	if (*pch == '+')
	{
	    if (len != 1)
	    {
	        pchNext = pch + 1;
	    }
	    fAddNext = TRUE;
	}

	else if (*pch == '-')
	{
	    if (len != 1) 
	    {
	        pchNext = pch + 1;
	    }
	    fAddNext = FALSE;
	}

	else if ((*pch == 'A') || (*pch == 'a'))
	{
	    if (StringsAreEqual(pch, WM_FUNC_ALL_STR,len))  
	    {
	        cval = fAddNext ? (cval | WM_FUNC_ALL) : 
				  (cval & ~WM_FUNC_ALL);
	        fHit = True;
	    }
	}
	       
	else if ((*pch == 'N') || (*pch == 'n'))
	{
	    if (StringsAreEqual(pch, WM_FUNC_NONE_STR,len))
	    {
	        /* don't bother adding or subtracting nothing */
	        fHit = True;
	    }
	}

	else if ((*pch == 'R') || (*pch == 'r'))
	{
	    if (StringsAreEqual(pch, WM_FUNC_RESIZE_STR,len))
	    {
	        cval = fAddNext ? (cval | MWM_FUNC_RESIZE) : 
	    	                  (cval & ~MWM_FUNC_RESIZE);
		fHit = True;
    	    }
	}
	       
	else if ((*pch == 'M') || (*pch == 'm'))
	{
	    if (StringsAreEqual(pch, WM_FUNC_MINIMIZE_STR,len)) 
	    {
		cval = fAddNext ? (cval | MWM_FUNC_MINIMIZE) : 
				  (cval & ~MWM_FUNC_MINIMIZE);
		fHit = True;
	    }
	    else if (StringsAreEqual(pch, WM_FUNC_MAXIMIZE_STR,len))  
	    {
		cval = fAddNext ? (cval | MWM_FUNC_MAXIMIZE) : 
				  (cval & ~MWM_FUNC_MAXIMIZE);
		fHit = True;
	    }
	    else if (StringsAreEqual(pch, WM_FUNC_MOVE_STR,len))  
	    {
		cval = fAddNext ? (cval | MWM_FUNC_MOVE) : 
				  (cval & ~MWM_FUNC_MOVE);
		fHit = True;
	    }
	}
	       
	else if ((*pch == 'C') || (*pch == 'c'))
	{
	    if (StringsAreEqual(pch, WM_FUNC_CLOSE_STR,len))
	    {
		cval = fAddNext ? (cval | MWM_FUNC_CLOSE) : 
				  (cval & ~MWM_FUNC_CLOSE);
		fHit = True;
	    }
	}

	pch = pchNext;
    }

    if (!fHit) cval =  WM_FUNC_ALL;

    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t) &cval;

} /* END OF FUNCTION WmCvtStringToCFunc */


/*************************************<->*************************************
 *
 *  WmCvtStringToFrameStyle (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a frame style description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToFrameStyle (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static FrameStyle frameStyle;
    Boolean    fHit = False;


#define FRAME_STYLE_RECESSED_STR	(unsigned char *)"recessed"
#define FRAME_STYLE_SLAB_STR		(unsigned char *)"slab"

    /*
     * Convert the resource value:
     */

    if (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'R') || (*pch == 'r'))
	{
	    if (StringsAreEqual (pch, FRAME_STYLE_RECESSED_STR, len))
	    {
		frameStyle = WmRECESSED;
		fHit = True;
	    }
	}
	else if (StringsAreEqual (pch, FRAME_STYLE_SLAB_STR, len))
	{
	    frameStyle = WmSLAB;
	    fHit = True;
	}
    }

    if (!fHit)
    {
	frameStyle = WmRECESSED;
    }

    (*toVal).size = sizeof (FrameStyle);
    (*toVal).addr = (caddr_t)&frameStyle;


} /* END OF FUNCTION WmCvtStringToFrameStyle */



/*************************************<->*************************************
 *
 *  WmCvtStringToIDecor (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to an icon decoration description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToIDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char       *pch = (unsigned char *) (fromVal->addr);
    unsigned char       *pchNext;
    int         len;
    static int cval;
    Boolean     fHit = False;

/*
 * Icon decoration parts:
 */

#define ICON_DECOR_IMAGE_STR		(unsigned char *)"image"
#define ICON_DECOR_LABEL_STR		(unsigned char *)"label"
#define ICON_DECOR_ACTIVE_LABEL_STR	(unsigned char *)"activelabel"


    /*
     * Convert the icon decoration resource value:
     */

    cval = 0;

    while (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'A') || (*pch == 'a'))
	{
	    if (StringsAreEqual (pch, ICON_DECOR_ACTIVE_LABEL_STR, len))
	    {
		cval |= ICON_ACTIVE_LABEL_PART;
		fHit = True;
	    }
	}

	else if ((*pch == 'I') || (*pch == 'i'))
	{
	    if (StringsAreEqual (pch, ICON_DECOR_IMAGE_STR, len))
	    {
		cval |= ICON_IMAGE_PART;
		fHit = True;
	    }
	}

	else if ((*pch == 'L') || (*pch == 'l'))
	{
	    if (StringsAreEqual (pch, ICON_DECOR_LABEL_STR, len))
	    {
		cval |= ICON_LABEL_PART;
		fHit = True;
	    }
	}

	pch = pchNext;
    }

    /*
     * If we didn't match anything or only have the active label
     * (which is just a modifier) then give 'em the whole ball of wax.
     */
    if (!fHit || cval == ICON_ACTIVE_LABEL_PART)
    {
	cval =  ICON_LABEL_PART | ICON_IMAGE_PART | ICON_ACTIVE_LABEL_PART;
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t) &cval;

} /* END OF FUNCTION WmCvtStringToIDecor */



/*************************************<->*************************************
 *
 *  WmCvtStringToIPlace (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to an icon placement scheme description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToIPlace (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char       *pch = (unsigned char *) (fromVal->addr);
    unsigned char       *pchNext;
    int         len;
    static int cval;
    Boolean     fPrimarySet = False;
    Boolean     fSecondarySet = False;

/*
 * Icon placement layout values:
 */

#define ICON_PLACE_BOTTOM_STR		(unsigned char *)"bottom"
#define ICON_PLACE_LEFT_STR		(unsigned char *)"left"
#define ICON_PLACE_RIGHT_STR		(unsigned char *)"right"
#define ICON_PLACE_TOP_STR		(unsigned char *)"top"
#define ICON_PLACE_TIGHT_STR		(unsigned char *)"tight"


    /*
     * Convert the icon placement resource value:
     */

    cval = 0;

    while (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'B') || (*pch == 'b'))
	{
	    if (StringsAreEqual (pch, ICON_PLACE_BOTTOM_STR, len))
	    {
		if (!fPrimarySet)
		{
		    cval |= ICON_PLACE_BOTTOM_PRIMARY;
		    fPrimarySet = True;
		}
		else if (!fSecondarySet)
		{
		    if (!(cval &
		      (ICON_PLACE_BOTTOM_PRIMARY | ICON_PLACE_TOP_PRIMARY)))
		    {
			cval |= ICON_PLACE_BOTTOM_SECONDARY;
			fSecondarySet = True;
		    }
		}
	    }
	}
	else if ((*pch == 'L') || (*pch == 'l'))
	{
	    if (StringsAreEqual (pch, ICON_PLACE_LEFT_STR, len))
	    {
		if (!fPrimarySet)
		{
		    cval |= ICON_PLACE_LEFT_PRIMARY;
		    fPrimarySet = True;
		}
		else if (!fSecondarySet)
		{
		    if (!(cval &
			(ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_RIGHT_PRIMARY)))
		    {
			cval |= ICON_PLACE_LEFT_SECONDARY;
			fSecondarySet = True;
		    }
		}
 	    }
	}

	else if ((*pch == 'R') || (*pch == 'r'))
	{
	    if (StringsAreEqual (pch, ICON_PLACE_RIGHT_STR, len))
	    {
		if (!fPrimarySet)
		{
		    cval |= ICON_PLACE_RIGHT_PRIMARY;
		    fPrimarySet = True;
		}
		else if (!fSecondarySet)
		{
		    if (!(cval &
			(ICON_PLACE_RIGHT_PRIMARY | ICON_PLACE_LEFT_PRIMARY)))
		    {
			cval |= ICON_PLACE_RIGHT_SECONDARY;
			fSecondarySet = True;
		    }
		}
	    }
	}

	else if ((*pch == 'T') || (*pch == 't'))
	{
	    if (StringsAreEqual (pch, ICON_PLACE_TOP_STR, len))
	    {
		if (!fPrimarySet)
		{
		    cval |= ICON_PLACE_TOP_PRIMARY;
		    fPrimarySet = True;
		}
		else if (!fSecondarySet)
		{
		    if (!(cval &
			(ICON_PLACE_TOP_PRIMARY | ICON_PLACE_BOTTOM_PRIMARY)))
		    {
		        cval |= ICON_PLACE_TOP_SECONDARY;
		        fSecondarySet = True;
		    }
		}
	    }

	    else if (StringsAreEqual (pch, ICON_PLACE_TIGHT_STR, len))
	    {
		cval |= ICON_PLACE_TIGHT;
	    }
	}

	pch = pchNext;
    }

    if (!fPrimarySet)
    {
	cval =  ICON_PLACE_LEFT_PRIMARY;
    }
    if (!fSecondarySet)
    {
	if (cval & (ICON_PLACE_LEFT_PRIMARY | ICON_PLACE_RIGHT_PRIMARY))
	{
	    cval |= ICON_PLACE_BOTTOM_SECONDARY;
	}
	else
	{
	    cval |= ICON_PLACE_LEFT_SECONDARY;
	}
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t) &cval;

} /* END OF FUNCTION WmCvtStringToIPlace */



/*************************************<->*************************************
 *
 *  WmCvtStringToKFocus (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a keyboard focus policy description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToKFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;

/*
 * Keyboard focus policies:
 */

#define KEYBOARD_FOCUS_EXPLICIT_STR		(unsigned char *)"explicit"
#define KEYBOARD_FOCUS_POINTER_STR		(unsigned char *)"pointer"


    /*
     * Convert the keyboard focus policy resource value:
     */

    if (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'E') || (*pch == 'e'))
	{
	    if (StringsAreEqual (pch, KEYBOARD_FOCUS_EXPLICIT_STR, len))
	    {
		cval = KEYBOARD_FOCUS_EXPLICIT;
		fHit = True;
	    }
        }

	else if ((*pch == 'P') || (*pch == 'p'))
	{
	    if (StringsAreEqual (pch, KEYBOARD_FOCUS_POINTER_STR, len))
	    {
		cval = KEYBOARD_FOCUS_POINTER;
		fHit = True;
	    }
	}
    }

    if (!fHit)
    {
	cval =  KEYBOARD_FOCUS_EXPLICIT;
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t)&cval;


} /* END OF FUNCTION WmCvtStringToKFocus */



/*************************************<->*************************************
 *
 *  WmCvtStringToSize (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a size description (<width>x<height>).
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToSize (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char  *pch = (unsigned char *) (fromVal->addr);
    unsigned char  *pchNext;
    static WHSize cval;
    int        len;

/*
 * Convenience values for WmSize:
 */

#define VERTICAL_STR	(unsigned char *)"vertical"
#define HORIZONTAL_STR	(unsigned char *)"horizontal"


    /*
     * Convert the size resource value.  The syntax is "<width>[xX]<height>"
     * OR it is the string 'vertical' or 'horizontal'.  It's kinda neat that
     * BIGSIZE is a legal Dimension so that we get vertical and horizontal
     * for free.
     */

    cval.width = 0;
    cval.height = 0;

    if (*pch)
    {
	cval.width = (int) DecStrToL (pch, &pchNext);
	if (!((cval.width == 0) && (pchNext == pch)))
	{
	    /*
	     * Width was converted.
	     * Check for a delimiter (must be 'x' or 'X'):
	     */

	    pch = pchNext;
	    if (*pch && ((*pch == 'x') || (*pch == 'X')))
	    {
		/*
		 * Delimiter found now get the height:
	         */

		pch++;
		cval.height = (int) DecStrToL (pch, &pchNext);
	    }
	}
	else
	{
	    if (*pch && NextToken (pch, &len, &pchNext))
	    {
		if ((*pch == 'V') || (*pch == 'v'))
		{
		    if (StringsAreEqual (pch, VERTICAL_STR, len))
		    {
			cval.height = BIGSIZE;
		    }
		}
		else if ((*pch == 'H') || (*pch == 'h'))
		{
		    if (StringsAreEqual (pch, HORIZONTAL_STR, len))
		    {
			cval.width = BIGSIZE;
		    }
		}
	    }
	}
    }

    /* !!! check for the maximum maximum sizes !!! */

    (*toVal).size = sizeof (WHSize);
    (*toVal).addr = (caddr_t)&cval;


} /* END OF FUNCTION WmCvtStringToSize */


/*************************************<->*************************************
 *
 *  WmCvtStringToShowFeedback (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a value for the showFeedback flag set.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToShowFeedback (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char       *pch = (unsigned char *) (fromVal->addr);
    unsigned char       *pchNext;
    int         len;
    static int cval;
    Boolean     fHit = False;
    Boolean    fAddNext = True;

/*
 * Names of feedback options
 */

#define SHOW_FB_ALL_STR		(unsigned char *)"all"
#define SHOW_FB_BEHAVIOR_STR	(unsigned char *)"behavior"
#define SHOW_FB_KILL_STR	(unsigned char *)"kill"
#define SHOW_FB_MOVE_STR	(unsigned char *)"move"
#define SHOW_FB_NONE_STR	(unsigned char *)"none"
#define SHOW_FB_PLACEMENT_STR	(unsigned char *)"placement"
#define SHOW_FB_QUIT_STR	(unsigned char *)"quit"
#define SHOW_FB_RESIZE_STR	(unsigned char *)"resize"
#define SHOW_FB_RESTART_STR	(unsigned char *)"restart"

    /*
     * Check first token. If '-' we subtract from all functions.
     * Otherwise, we start with no functions and add things in.
     */

    if (*pch &&
        (NextToken (pch, &len, &pchNext)) &&
        (*pch == '-'))
    {
	cval = WM_SHOW_FB_DEFAULT;
        fHit = True;
    }
    else
    {
        cval = WM_SHOW_FB_NONE;
    }


    /*
     * Convert the feedback option resource value:
     */


    while (*pch && NextToken (pch, &len, &pchNext))
    {
           /*
            * Strip off "sign" if prepended to another token, and process
            * that token the next time through.
            */

        if (*pch == '+')
        {
            if (len != 1)
            {
                pchNext = pch + 1;
            }
            fAddNext = TRUE;
        }

        else if (*pch == '-')
        {
            if (len != 1)
            {
                pchNext = pch + 1;
            }
            fAddNext = FALSE;
        }

	if ((*pch == 'A') || (*pch == 'a'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_ALL_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_ALL) :
		                  (cval & ~WM_SHOW_FB_ALL);
		fHit = True;
	    }
	}

	else if ((*pch == 'B') || (*pch == 'b'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_BEHAVIOR_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_BEHAVIOR) :
		                  (cval & ~WM_SHOW_FB_BEHAVIOR);
		fHit = True;
	    }
	}

	else if ((*pch == 'K') || (*pch == 'k'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_KILL_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_KILL) :
		                  (cval & ~WM_SHOW_FB_KILL);
		fHit = True;
	    }
	}

	else if ((*pch == 'M') || (*pch == 'm'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_MOVE_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_MOVE) :
		                  (cval & ~WM_SHOW_FB_MOVE);
		fHit = True;
	    }
	}

	else if ((*pch == 'N') || (*pch == 'n'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_NONE_STR, len))
	    {
		/* don't bother adding or subtracting nothing */
		fHit = True;
	    }
	}

	else if ((*pch == 'P') || (*pch == 'p'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_PLACEMENT_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_PLACEMENT) :
		                  (cval & ~WM_SHOW_FB_PLACEMENT);
		fHit = True;
	    }
	}

	else if ((*pch == 'Q') || (*pch == 'q'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_QUIT_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_QUIT) :
		                  (cval & ~WM_SHOW_FB_QUIT);
		fHit = True;
	    }
	}

	else if ((*pch == 'R') || (*pch == 'r'))
	{
	    if (StringsAreEqual (pch, SHOW_FB_RESIZE_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_RESIZE) :
		                  (cval & ~WM_SHOW_FB_RESIZE);
		fHit = True;
	    }
	    else if (StringsAreEqual (pch, SHOW_FB_RESTART_STR, len))
	    {
		cval = fAddNext ? (cval | WM_SHOW_FB_RESTART) :
		                  (cval & ~WM_SHOW_FB_RESTART);
		fHit = True;
	    }
	}
	pch = pchNext;
    }



    /*
     * If we didn't match anything then set to default.
     */
    if (!fHit)
    {
	cval =  WM_SHOW_FB_DEFAULT;
    }


    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t) &cval;

} /* END OF FUNCTION WmCvtStringToShowFeedback */



/*************************************<->*************************************
 *
 *  WmCvtStringToUsePPosition (args, numArgs, fromVal, toVal)
 *
 *
 *  Description:
 *  -----------
 *  This function converts a string to a keyboard focus policy description.
 *
 *
 *  Inputs:
 *  ------
 *  args = NULL (don't care)
 *
 *  numArgs = 0 (don't care)
 *
 *  fromVal = resource value to convert
 *
 * 
 *  Outputs:
 *  -------
 *  toVal = descriptor to use to return converted value
 *
 *************************************<->***********************************/

void WmCvtStringToUsePPosition (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal)
{
    unsigned char      *pch = (unsigned char *) (fromVal->addr);
    unsigned char      *pchNext;
    int        len;
    static int cval;
    Boolean    fHit = False;


#define USE_PPOSITION_NONZERO_STR	(unsigned char *)"nonzero"
#define USE_PPOSITION_ON_STR		(unsigned char *)"on"
#define USE_PPOSITION_OFF_STR		(unsigned char *)"off"

    /*
     * Convert the use PPosition resource value:
     */

    if (*pch && NextToken (pch, &len, &pchNext))
    {
	if ((*pch == 'N') || (*pch == 'n'))
	{
	    if (StringsAreEqual (pch, USE_PPOSITION_NONZERO_STR, len))
	    {
		cval = USE_PPOSITION_NONZERO;
		fHit = True;
	    }
	}
	else if (StringsAreEqual (pch, USE_PPOSITION_OFF_STR, len))
	{
	    cval = USE_PPOSITION_OFF;
	    fHit = True;
	}
	else if (StringsAreEqual (pch, USE_PPOSITION_ON_STR, len))
	{
	    cval = USE_PPOSITION_ON;
	    fHit = True;
	}
    }

    if (!fHit)
    {
	cval =  USE_PPOSITION_NONZERO;
    }

    (*toVal).size = sizeof (int);
    (*toVal).addr = (caddr_t)&cval;


} /* END OF FUNCTION WmCvtStringToUsePPosition */


/*************************************<->*************************************
 *
 *  NextToken (pchIn, pLen, ppchNext)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *
 *  Inputs:
 *  ------
 *  pchIn = pointer to start of next token
 *
 * 
 *  Outputs:
 *  -------
 *  pLen  =    pointer to integer containing number of characters in next token
 *  ppchNext = address of pointer to following token
 *
 *  Return =   next token or NULL
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

unsigned char *NextToken (unsigned char *pchIn, int *pLen, 
	unsigned char **ppchNext)
{
    unsigned char *pchR = pchIn;
    register int   i;

#ifndef NO_MULTIBYTE
    register int   chlen;

    for (i = 0;
	 ((chlen = mblen((char *)pchIn, MB_CUR_MAX)) > 0) && (pchIn[0] != '\0');
	 i++)
    /* find end of word: requires singlebyte whitespace terminator */
    {
	if ((chlen == 1) && isspace (*pchIn))
	{
	    break;
	}
	pchIn += chlen;
    }

#else
    for (i = 0; *pchIn && !isspace (*pchIn); i++, pchIn++)
    /* find end of word */
    {
    }
#endif

    /* skip to next word */
    ScanWhitespace (&pchIn);

    *ppchNext = pchIn;
    *pLen = i;
    if (i)
    {
	return(pchR);
    }
    else
    {
       return(NULL);
    }

} /* END OF FUNCTION NextToken */   



/*************************************<->*************************************
 *
 *  StringsAreEqual (pch1, pch2, len)
 *
 *
 *  Description:
 *  -----------
 *  XXDescription ...
 *
 *
 *  Inputs:
 *  ------
 *  pch1 =
 *  pch2 =
 *  len  =
 *
 * 
 *  Outputs:
 *  -------
 *  Return = (Boolean) True iff strings match (case insensitive)
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

Boolean StringsAreEqual (unsigned char *pch1, unsigned char *pch2, int len)
{
#ifndef NO_MULTIBYTE
    int       chlen1;
    int       chlen2;
    wchar_t   wch1;
    wchar_t   wch2;

    while (len  && 
	   ((chlen1 = mbtowc (&wch1, (char *) pch1, MB_CUR_MAX)) > 0) &&
           ((chlen2 = mbtowc (&wch2, (char *) pch2, MB_CUR_MAX)) == chlen1) )
    {
        if (chlen1 == 1)
	/* singlebyte characters -- make case insensitive */
	{
	    if ((isupper (*pch1) ? tolower(*pch1) : *pch1) !=
	        (isupper (*pch2) ? tolower(*pch2) : *pch2))
            {
		break;
            }
	}
        else
	/* multibyte characters */
        {
	    if (wch1 != wch2)
            {
		break;
            }
        }
        pch1 += chlen1;
        pch2 += chlen2;
        len--;
    }

#else
    while (len  && *pch1 && *pch2 &&
	   ((isupper (*pch1) ? tolower(*pch1++) : *pch1++) ==
	    (isupper (*pch2) ? tolower(*pch2++) : *pch2++)))
    {
        len--;
    }
#endif

    return (len == 0);

} /* END OF StringsAreEqual */   


/*************************************<->*************************************
 *
 *  long
 *  DecStrToL (str, ptr)
 *
 *
 *  Description:
 *  -----------
 *  Converts a decimal string to a long.
 *
 *
 *  Inputs:
 *  ------
 *  str = character string
 *
 * 
 *  Outputs:
 *  -------
 *  *ptr = pointer to character terminating str or str
 *  Return = long value
 *
 *
 *  Comments:
 *  --------
 *  Leading whitespace is ignored.
 *  Returns long value with *ptr pointing at character terminating the decimal
 *    string.
 *  Returns 0 with *ptr == str if no integer can be formed.
 * 
 *************************************<->***********************************/

long DecStrToL (unsigned char *str, unsigned char **ptr)
{
    long  val = 0;

    *ptr = str;
#ifndef NO_MULTIBYTE
    while ((mblen ((char *)str, MB_CUR_MAX) == 1) && isspace (*str))
#else
    while (isspace (*str))
#endif
    /* Ignore leading whitespace */
    {
        str++;
    }

    /* If we can start, we will reset *ptr */
#ifndef NO_MULTIBYTE
    if ((mblen ((char *)str, MB_CUR_MAX) == 1) && isdigit (*str))
    {
        while ((mblen ((char *)str, MB_CUR_MAX) == 1) && isdigit (*str))
#else
    if (isdigit (*str))
    {
        while (isdigit (*str))
#endif
        {
	    val = val * 10 + (*str - '0');
	    str++;
        }

        *ptr = str;
    }

    return (val);

} /* END OF FUNCTION DecStrToL */

