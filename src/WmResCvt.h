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

void AddWmResourceConverters (void);
void WmCvtStringToAMBehavior (XrmValue *args, Cardinal numArgs, 
				 XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToCFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToCDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToCFunc (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToFrameStyle (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToIDecor (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToIPlace (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToKFocus (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToSize (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToShowFeedback (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
void WmCvtStringToUsePPosition (XrmValue *args, Cardinal numArgs, XrmValue *fromVal, XrmValue *toVal);
unsigned char *NextToken (unsigned char *pchIn, int *pLen, unsigned char **ppchNext);
Boolean StringsAreEqual (unsigned char *pch1, unsigned char *pch2, int len);
long DecStrToL (unsigned char *str, unsigned char **ptr);
