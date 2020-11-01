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

#ifndef _WM_DEBUG_
#define _WM_DEBUG_

#ifdef MWM_DEBUG

# ifndef PRINT
#   define PRINT printf
# endif

#else

# ifndef PRINT
#   ifdef _NO_PROTO
    static int noop () { return 0; }
#   else
    static int noop (char *fmt, ...) { return 0; }
#   endif
#   define PRINT noop
# endif

#endif /* MWM_DEBUG */


#endif /* _WM_DEBUG_ */
