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



#include <Xm/Xm.h>
#include "WmGlobal.h"
#include "WmWsmLib/wsm_proto.h"

/*------------------------------------------------------------------*
 |                              GetPCD                              |
 *------------------------------------------------------------------*/
/*ARGSUSED*/
ClientData *
GetPCD (
     int    scr,		/* unused */
     Window win)
{
  ClientData   *pCD = NULL;
  Window        root, parent, *children;
  unsigned int  nchildren;

  if (XQueryTree(DISPLAY, win & WIN_MASK, &root, &parent,
		 &children, &nchildren))

    if (XFindContext (DISPLAY, parent, wmGD.windowContextType,
		      (XPointer *)&pCD))
      {
	Boolean foundIt = False;

	while ((parent != root) && (!foundIt))
	  {
	    win = parent;
	    if (!XQueryTree(DISPLAY, win, &root, &parent,
			    &children, &nchildren))
	      break;
	    foundIt =
	      (XFindContext (DISPLAY, parent, wmGD.windowContextType,
			     (XPointer *)&pCD) == 0);
	  }

	if (!foundIt) pCD = NULL;
      }

  return (pCD);
}
