/* $TOG: disp.c /main/6 1997/06/18 17:34:28 samborn $ */
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

WSMDispInfo * GDispInfo = NULL;

/*	Function Name: _WSMGetDispInfo
 *	Description: Gets the display information structure associated with
 *                   the display passed into this function.
 *	Arguments: dpy - The Display to get the info for.
 *	Returns: a WSMDispInfo structure for the display passed.
 */

WSMDispInfo *
_WSMGetDispInfo(Display *dpy)
{
    enum { XA_WSM_NAME_CONNECT, XA_WSM_NAME_EXTENSIONS, XA_WSM_NAME_CONFIG_FMT,
	   XA_WSM_NAME_GET_STATE, XA_WSM_NAME_SET_STATE, XA_WSM_NAME_REG_WINDOW,
	   XA_WSM_NAME_WM_GET_BACKGROUND_WINDOW,
	   XA_WSM_NAME_WM_SET_BACKGROUND_WINDOW, XA_WSM_NAME_WM_WINDOWS,
	   XA_WSM_NAME_WM_FOCUS, XA_WSM_NAME_WM_POINTER, XA_TARGETS,
	   XA_MULTIPLE, XA_TIMESTAMP, NUM_ATOMS };
    static char *atom_names[] = {
           WSM_NAME_CONNECT, WSM_NAME_EXTENSIONS, WSM_NAME_CONFIG_FMT,
	   WSM_NAME_GET_STATE, WSM_NAME_SET_STATE, WSM_NAME_REG_WINDOW,
	   WSM_NAME_WM_GET_BACKGROUND_WINDOW,
	   WSM_NAME_WM_SET_BACKGROUND_WINDOW, WSM_NAME_WM_WINDOWS,
	   WSM_NAME_WM_FOCUS, WSM_NAME_WM_POINTER, "TARGETS",
	   "MULTIPLE", "TIMESTAMP" };

    WSMDispInfo * disp_info;
    Atom atoms[XtNumber(atom_names)];

    if (GDispInfo == NULL) {
	disp_info = GDispInfo = (WSMDispInfo *) XtMalloc(sizeof(WSMDispInfo));
    }
    else {
	disp_info = GDispInfo;

	 while (True) {
	    if (disp_info->disp == dpy)
		return(disp_info);

	    if (disp_info->next == NULL)
		break;

	    disp_info = disp_info->next;
	}

	/*
	 * This display is not on the global list, add it.
	 */

	disp_info->next = (WSMDispInfo *) XtMalloc(sizeof(WSMDispInfo));
	disp_info = disp_info->next;
    }

    disp_info->disp = dpy;
    
    /*
     * These are our names.
     * 
     * ||| Should use XtConvertAndStore().
     */

    XInternAtoms(dpy, atom_names, XtNumber(atom_names), False, atoms);

    disp_info->connect = atoms[XA_WSM_NAME_CONNECT];
    disp_info->extensions = atoms[XA_WSM_NAME_EXTENSIONS];
    disp_info->config_fmt = atoms[XA_WSM_NAME_CONFIG_FMT];
    disp_info->get_state = atoms[XA_WSM_NAME_GET_STATE];
    disp_info->set_state = atoms[XA_WSM_NAME_SET_STATE];
    disp_info->reg_window = atoms[XA_WSM_NAME_REG_WINDOW];

    disp_info->get_background = atoms[XA_WSM_NAME_WM_GET_BACKGROUND_WINDOW];
    disp_info->set_background = atoms[XA_WSM_NAME_WM_SET_BACKGROUND_WINDOW];

    disp_info->wm_windows = atoms[XA_WSM_NAME_WM_WINDOWS];
    disp_info->wm_focus   = atoms[XA_WSM_NAME_WM_FOCUS];
    disp_info->wm_pointer = atoms[XA_WSM_NAME_WM_POINTER];

    /*
     * These global resources are unlikely to change.
     */

    disp_info->targets = atoms[XA_TARGETS];
    disp_info->multiple = atoms[XA_MULTIPLE];
    disp_info->timestamp = atoms[XA_TIMESTAMP];

    disp_info->screen_info = NULL;
    disp_info->next = NULL;

    return(disp_info);
}

/*	Function Name: _WSMGetScreenInfo
 *	Description: Gets the screen information structure associated with
 *                   the display and screen passed into this function.
 *	Arguments: dpy - The Display to get the info for.
 *                 screen_number - The screen number.
 *	Returns: a WSMScreenInfo structure for the display passed.
 */

WSMScreenInfo *
_WSMGetScreenInfo(Display *dpy, int screen_number)
{
    WSMDispInfo * disp_info = _WSMGetDispInfo(dpy);
    WSMScreenInfo *screen_info;
    char temp[BUFSIZ];

    if (disp_info->screen_info == NULL) {
	/*
	 * Didn't find any screen info on this display, allocate it
	 * and then drop down to the code below that fills in the data.
	 */

	screen_info = (WSMScreenInfo *) XtMalloc(sizeof(WSMScreenInfo));
	disp_info->screen_info = screen_info;
    }
    else {
	screen_info = disp_info->screen_info;

	/*
	 * Hunt through the screen info structs on this display until we
	 * find one that matches the screen num passed.  If we don't find
	 * one then allocate a new one, put it on the end of the list and
	 * drop into the code below that fills in the data.
	 */

	while (True) {
	    if (screen_info->screen_num == screen_number)
		return(screen_info);
	    
	    if (screen_info->next == NULL)
		break;
	    
	    screen_info = screen_info->next;
	}

	/*
	 * This screen is not on the display's screen list, add it.
	 */

	screen_info->next = (WSMScreenInfo *) XtMalloc(sizeof(WSMScreenInfo));
	screen_info = screen_info->next;
    }

    screen_info->screen_num = screen_number;

    sprintf(temp, WM_SELECTION_FORMAT, screen_number);
    screen_info->wm_selection = XInternAtom(dpy, temp, False);

    sprintf(temp, WSM_SELECTION_FORMAT, screen_number);
    screen_info->wsm_selection = XInternAtom(dpy, temp, False);

    screen_info->next = NULL;
    screen_info->global.num_attrs = 0;
    screen_info->window.num_attrs = 0;
    screen_info->icon.num_attrs = 0;
    screen_info->request_callback = NULL;
    screen_info->request_data = NULL;

    return(screen_info);
}





/*	Function Name: _WSMClearConfigScreenInfo
 *	Description: Resets the screen information structure associated with
 *                   the display and screen passed into this function.
 *	Arguments: dpy - The Display to get the info for.
 *                 screen_number - The screen number.
 *	Returns:
 */

void
_WSMClearConfigScreenInfo(Display *dpy, int screen_number)
{
    WSMDispInfo * disp_info = _WSMGetDispInfo(dpy);
    WSMScreenInfo *screen_info;

    if (disp_info->screen_info != NULL)
      {
	screen_info = disp_info->screen_info;

	/*
	 * Hunt through the screen info structs on this display until we
	 * find one that matches the screen num passed. 
	 */

	while (True) {
	  if (screen_info->screen_num == screen_number)
	    break;
	  
	  if (screen_info->next == NULL)
	    return;
	  
	  screen_info = screen_info->next;
	}

	if (screen_info->global.num_attrs != 0){
	  XtFree((XtPointer)screen_info->global.attr_list);
	  screen_info->global.num_attrs = 0;
	}
	
	if (screen_info->window.num_attrs != 0){
	  XtFree((XtPointer)screen_info->window.attr_list);
	  screen_info->window.num_attrs = 0;
	}
	
	if (screen_info->icon.num_attrs != 0){
	  XtFree((XtPointer)screen_info->icon.attr_list);
	  screen_info->icon.num_attrs = 0;
	}
      }

}



/*	Function Name: _WSMGetConfigFormat
 *	Description: Gets the Configuration format for the dpy, scr and
 *                   type specified.
 *	Arguments: dpy - The display.
 *                 screen_number - The screen number.
 *                 type - The type of format required.
 *	Returns: A pointer to the config format, or NULL if none was
 *               found.
 */

WSMConfigFormatData *
_WSMGetConfigFormat(Display *dpy, int screen_number, WSMConfigFormatType type)
{
    WSMScreenInfo *screen_info = _WSMGetScreenInfo(dpy, screen_number);

    if (screen_info == NULL)
	return(NULL);

    switch(type) {
    case WSM_GLOBAL_FMT:
	return(&(screen_info->global));
    case WSM_WINDOW_FMT:
	return(&(screen_info->window));
    case WSM_ICON_FMT:
	return(&(screen_info->icon));
    default:
	break;
    }

    return(NULL);
}

/*	Function Name: _WSMGetSelectionAtom
 *	Description: Gets the Selection atom for the manager that we want to
 *                   send to.
 *	Arguments: dpy - The display.
 *                 screen_number - The screen number.
 *                 send_to - Whether to send this to the WM or WSM.
 *	Returns: A pointer to the config format, or NULL if none was
 *               found.
 */

Atom
_WSMGetSelectionAtom(Display *dpy, int screen_num, WSMClientType send_to)
{
    WSMScreenInfo *screen_info = _WSMGetScreenInfo(dpy, screen_num);

    if (screen_info == NULL)
	return(None);

    switch(send_to) {
    case WSM_WORKSPACE_MANAGER:
	return(screen_info->wsm_selection);
    case WSM_WINDOW_MANAGER:
	return(screen_info->wm_selection);
    default:
	break;
    }

    return(None);
}
