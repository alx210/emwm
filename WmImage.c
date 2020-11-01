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
static char rcsid[] = "$XConsortium: WmImage.c /main/7 1996/11/14 13:50:30 rswiston $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"

#define MWM_NEED_IIMAGE
#include "WmIBitmap.h"

#ifdef MOTIF_ONE_DOT_ONE
#include <stdio.h>
#include <pwd.h>
#define MATCH_CHAR 'P'		/* Default match character - defined in Xmos.p */
#else
#include <Xm/XmosP.h> 
/* Copied from XmosI.h */
extern String _XmOSInitPath( 
                        String file_name,
                        String env_pathname,
                        Boolean *user_path) ;
#endif
#ifdef WSM
#include <Xm/IconFile.h>
#include <Dt/GetDispRes.h>
#endif

#define MATCH_XBM 'B'		/* .xbm character: see XmGetPixmap */
#define MATCH_PATH "XBMLANGPATH"

/*
 * include extern functions
 */

#include "WmImage.h"
#include "WmGraphics.h"
#include "WmResource.h"
#include "WmResParse.h"
#include "WmMenu.h"
#include "WmError.h"

#ifdef MOTIF_ONE_DOT_ONE
extern char    *getenv ();
#endif


/******************************<->*************************************
 *
 *  MakeClientIconPixmap (pCD, iconBitmap, iconMask)
 *
 *
 *  Description:
 *  -----------
 *  This function takes a client supplied icon image pixmap and mask 
 *  bitmap and makes it into a colored pixmap suitable for use as an 
 *  icon image.
 *
 *
 *  Inputs:
 *  ------
 *  pCD = pointer to client data (icon colors and tiles)
 *
 *  iconBitmap = icon bitmap 
 *  iconMask   = mask bitmap
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = icon pixmap (NULL if error)
 *
 ******************************<->***********************************/

Pixmap MakeClientIconPixmap (
	ClientData *pCD, 
	Pixmap iconBitmap, 
	Pixmap iconMask)
{
  Window root;
  int x;
  int y;
  unsigned int  bitmapWidth;
  unsigned int  bitmapHeight;
  unsigned int  border;
  unsigned int  depth;
  WmScreenData *pSD;

  if (pCD)
    pSD = pCD->pSD;
  else
    pSD = wmGD.pActiveSD;
    
  /*
   * Get pixmap attributes and ensure that it is usable.
   */
   
  if (!XGetGeometry (DISPLAY, iconBitmap, &root, &x, &y,
      &bitmapWidth, &bitmapHeight, &border, &depth))
  {
    Warning (((char *)GETMESSAGE(38, 1, "Invalid icon pixmap")));
    return ((Pixmap)NULL);
  }

  /*
   * Color the bitmap, center it in a pixmap ....
   */
  return (MakeIconPixmap (pCD, iconBitmap, iconMask, 
			  bitmapWidth, bitmapHeight, depth));

} /* END OF FUNCTION MakeClientIconPixmap */



/*************************************<->*************************************
 *
 *  GetNamedPixmap (pCD, iconName)
 *
 *
 *  Description:
 *  -----------
 *  This function gets the named pixmap.  bitmapDirectory is searched first
 *  then, if not set, /usr/lib/X11/bitmaps is searched.  Finally, if that
 *  also fails, XMBLANGPATH is used.
 *
 *  Inputs:
 *  ------
 *  iconName = pointer to the icon name (bitmap file path name or NULL)
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = icon pixmap or XmUNSPECIFIED_PIXMAP
 * 
 *************************************<->***********************************/

Pixmap GetNamedPixmap (Screen *scr, String iconName,
		       Pixel fg, Pixel bg, int depth)
{
  Pixmap       pixmap = XmUNSPECIFIED_PIXMAP;


  if (iconName) {
    /*
     * If name is not absolute, then try the bitmapDirectory location.
     * Note that bitmapDirectory defaults to "/usr/lib/X11/bitmaps" if
     * not set.  The user could have set bitmapDirectory to "" though.
     */
    if ((iconName[0] != '/') && (wmGD.bitmapDirectory))
      {
	char *fullPathName;
	
	
	fullPathName = (char*) XtMalloc(strlen(wmGD.bitmapDirectory) +
					strlen(iconName) + 2);
	sprintf(fullPathName, "%s/%s", wmGD.bitmapDirectory, iconName);
	
	pixmap = XmGetPixmapByDepth(scr, fullPathName, fg, bg, depth);
	
	XtFree(fullPathName);
      }
    
    /*
     * If failed to get pixmap with bitmapDirectory, try
     * using XBMLANGPATH.
     */
    if (!PIXMAP_IS_VALID( pixmap ))
      pixmap = XmGetPixmapByDepth(scr, iconName, fg, bg, depth);

    /*
     * Warning message handling on error left to the calling routine.
     */
  }
  
  return (pixmap);
}



/*************************************<->*************************************
 *
 *  MakeNamedIconPixmap (pCD, iconName)
 *
 *
 *  Description:
 *  -----------
 *  This function makes an icon pixmap for a particular client given the
 *  name of a bitmap file.
 *
 *
 *  Inputs:
 *  ------
 *  pCD      = (nonNULL) pointer to client data
 *  iconName = pointer to the icon name (bitmap file path name or NULL)
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN = icon pixmap or NULL
 * 
 *************************************<->***********************************/

Pixmap MakeNamedIconPixmap (ClientData *pCD, String iconName)
{
  Pixmap pixmap;
  unsigned int width, height;
  int depth;


  if (iconName)
    {
      pixmap = GetNamedPixmap (ScreenOfDisplay(DISPLAY, pCD->pSD->screen),
			       iconName,
			       pCD->iconImageForeground,
			       pCD->iconImageBackground, 
			       DefaultDepth(DISPLAY, pCD->pSD->screen));
      
      if (!PIXMAP_IS_VALID( pixmap ))
	{
	  Warning (((char *)GETMESSAGE(38, 1, "Invalid icon pixmap")));
	}
      else
	{
	  XmeGetPixmapData(ScreenOfDisplay(DISPLAY, pCD->pSD->screen), pixmap,
			   NULL, &depth, NULL, NULL, NULL, NULL,
			   &width, &height);
      
	  pixmap = MakeIconPixmap (pCD, pixmap, XmUNSPECIFIED_PIXMAP,
				   width, height, depth);
	}
    }

  /*
   * If no name was given or we couldn't find the specified pixmap,
   * then use the default.
   */

  if (!iconName || (!PIXMAP_IS_VALID( pixmap )))
    {
      pixmap = MakeIconPixmap (pCD, pCD->pSD->builtinIconPixmap, XmUNSPECIFIED_PIXMAP,
			       iImage_width, iImage_height, 1);
    }
  

  return (pixmap);

} /* END OF FUNCTION MakeNamedIconPixmap */



/*************************************<->*************************************
 *
 *  MakeIconPixmap (pCD, bitmap, mask, width, height, depth)
 *
 *
 *  Description:
 *  -----------
 *  Convert the bitmap and mask into an icon pixmap.
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data (icon colors and tiles)
 *  pWS		- pointer to workspace data
 *  bitmap	- bitmap image to be converted
 *  mask	- bitmap mask, 1 for bits of "bitmap" to be kept
 *  width	- pixel width of bitmap
 *  height	- pixel height of bitmap
 *  depth	- depth of bitmap (pixmap, really)
 *
 * 
 *  Outputs:
 *  -------
 *  RETURN 	- icon pixmap or NULL
 *
 *
 *  Comments:
 *  --------
 *  o "mask" is not used.
 * 
 *************************************<->***********************************/
Pixmap MakeIconPixmap (ClientData *pCD, Pixmap bitmap, Pixmap mask, unsigned int width, unsigned int height, unsigned int depth)
{
    Pixmap       iconPixmap;
    GC           imageGC, topGC, botGC;
    XGCValues    gcv;
#ifdef WSM
    unsigned long gc_mask;
    XmPixelSet   *pPS = NULL;
#endif /* WSM */
    unsigned int imageWidth;
    unsigned int imageHeight;
    int          dest_x, dest_y;
#ifndef NO_CLIP_CENTER
    int          src_x, src_y;
#endif /* NO_CLIP_CENTER */
    Pixel        fg;
    Pixel        bg;
    static RList *top_rects = NULL;
    static RList *bot_rects = NULL;
    WmScreenData	*pSD;

    if ((top_rects == NULL) && 
	(top_rects = AllocateRList 
	((unsigned)2 * ICON_INTERNAL_SHADOW_WIDTH)) == NULL)
    {
	/* Out of memory! */
	Warning (((char *)GETMESSAGE(38, 3, "Insufficient memory to bevel icon image")));
	return ((Pixmap)NULL);
    }

    if ((bot_rects == NULL) &&
	(bot_rects = AllocateRList 
	((unsigned)2 * ICON_INTERNAL_SHADOW_WIDTH)) == NULL)
    {
	/* Out of memory! */
	Warning (((char *)GETMESSAGE(38, 4, "Insufficient memory to bevel icon image")));
	return ((Pixmap)NULL);
    }

    if (pCD)
    {
	pSD = pCD->pSD;
    }
    else 
    {
	pSD = wmGD.pActiveSD;
    }

    /* don't make icon pixmap if bitmap is too small */

    if ((width < pSD->iconImageMinimum.width) ||
	(height < pSD->iconImageMinimum.height))
    {
	/* bitmap is too small */
	return ((Pixmap)NULL);
    }
#ifndef NO_CLIP_CENTER
    
    /* copy the center of the icon if too big */
    if (width > pSD->iconImageMaximum.width)
    {
	src_x = (width - pSD->iconImageMaximum.width)/2;
    }
    else
    {
	src_x = 0;
    }
    if (height > pSD->iconImageMaximum.height)
    {
	src_y = (height - pSD->iconImageMaximum.height)/2;
    }
    else
    {
	src_y = 0;
    }
#endif /* NO_CLIP_CENTER */

    /*
     * SLAB frameStyle adds a single pixel of background color around
     * the image to set it off from the beveling.
     */
    imageWidth = pSD->iconImageMaximum.width + 
		  2 * ICON_INTERNAL_SHADOW_WIDTH +
		  ((wmGD.frameStyle == WmSLAB) ? 2 : 0);
    imageHeight = pSD->iconImageMaximum.height +
		  2 * ICON_INTERNAL_SHADOW_WIDTH +
		  ((wmGD.frameStyle == WmSLAB) ? 2 : 0);

    /* create a pixmap (to be returned) */

    iconPixmap = XCreatePixmap (DISPLAY, pSD->rootWindow, 
		     imageWidth, imageHeight,
		     DefaultDepth(DISPLAY, pSD->screen));

    /*
     * If a client is not specified use icon component colors, otherwise
     * use the client-specific icon colors.
     */

    if (pCD)
    {
	bg = pCD->iconImageBackground;
	fg = pCD->iconImageForeground;
    }
    else
    {
	bg = pSD->iconAppearance.background;
	fg = pSD->iconAppearance.foreground;
    }

    /* create a GC to use */
#ifdef WSM
    gc_mask = GCForeground | GCBackground | GCGraphicsExposures;
    if (mask)
    {
	if (pSD->pPrimaryPixelSet != NULL)
	{
	    pPS = pSD->pPrimaryPixelSet;
	    gcv.background = pPS->bg;
	    /* set fg to bg color to clear it first */
	    gcv.foreground = pPS->bg;
	}
	else
	{
	    gcv.background = ICON_APPEARANCE(pCD).background;
	    /* set fg to bg color to clear it first */
	    gcv.foreground = ICON_APPEARANCE(pCD).background;
	}
    } 
    else 
    {
	gcv.foreground = bg;	/* clear it first! */
	gcv.background = bg;
    }
    gcv.graphics_exposures = False;

    imageGC = XCreateGC (DISPLAY, iconPixmap, gc_mask, &gcv);
#else /* WSM */
    gcv.foreground = bg;	/* clear it first! */
    gcv.background = bg;
    gcv.graphics_exposures = False;

    imageGC = XCreateGC (DISPLAY, iconPixmap, (GCForeground|GCBackground),
		  &gcv);
#endif /* WSM */

    /*
     * Format the image. 
     */

    /* fill in background */

    XFillRectangle(DISPLAY, iconPixmap, imageGC, 0, 0, 
		   imageWidth, imageHeight);

    /* center the image */

    if (width > pSD->iconImageMaximum.width)
    {
	width = pSD->iconImageMaximum.width;
    }
    if (height > pSD->iconImageMaximum.height)
    {
	height = pSD->iconImageMaximum.height;
    }
    /* center the image */

    dest_x = (imageWidth - width) / 2;
    dest_y = (imageHeight - height) / 2;

#ifdef WSM
    if (mask)
    {
	if (pPS != NULL)
	{
	    gcv.foreground = pPS->fg;
	}
	else
	{
	    gcv.foreground = ICON_APPEARANCE(pCD).foreground;
	}
    }
    else
    {
	gcv.foreground = fg;
    }
    gc_mask = GCForeground;
    if (mask)
    {
	gcv.clip_mask = mask;
#ifndef NO_CLIP_CENTER
	gcv.clip_x_origin = dest_x - src_x;
	gcv.clip_y_origin = dest_y - src_y;
#else /* NO_CLIP_CENTER */
	gcv.clip_x_origin = dest_x;
	gcv.clip_y_origin = dest_y;
#endif /* NO_CLIP_CENTER */
	gc_mask |= GCClipXOrigin | GCClipYOrigin | GCClipMask;
    } 

    XChangeGC (DISPLAY, imageGC, gc_mask, &gcv);
#else /* WSM */
    /* set the foreground */
    XSetForeground (DISPLAY, imageGC, fg);
#endif /* WSM */

    /* copy the bitmap to the pixmap */
#ifndef DISALLOW_DEEP_ICONS
    if ((depth > 1) &&
        (depth == DefaultDepth(DISPLAY, pSD->screen)))
    {
#ifndef NO_CLIP_CENTER
        XCopyArea (DISPLAY, bitmap, iconPixmap, imageGC, src_x, src_y,
                width, height, dest_x, dest_y);
#else /* NO_CLIP_CENTER */
        XCopyArea (DISPLAY, bitmap, iconPixmap, imageGC, 0, 0,
                width, height, dest_x, dest_y);
#endif /* NO_CLIP_CENTER */
    }
    else
#endif /* DISALLOW_DEEP_ICONS */
#ifndef NO_CLIP_CENTER
    XCopyPlane (DISPLAY, bitmap, iconPixmap, imageGC, src_x, src_y, width, 
		height, dest_x, dest_y, 1L);
#else /* NO_CLIP_CENTER */
    XCopyPlane (DISPLAY, bitmap, iconPixmap, imageGC, 0, 0, width, height, 
		dest_x, dest_y, 1L);
#endif /* NO_CLIP_CENTER */

    /* free resources */
    XFreeGC (DISPLAY, imageGC);

    if (pCD)
    {
	/*
	 * Shadowing
	 */

#ifdef WSM
        if (mask && (pPS != NULL))
	{
	topGC = GetHighlightGC (pSD, pPS->ts, pPS->bg,
				  pCD->iconImageTopShadowPixmap);

	botGC = GetHighlightGC (pSD, pPS->bs, pPS->bg,
				  pCD->iconImageBottomShadowPixmap);
	}
	else
	{
#endif /* WSM */
	topGC = GetHighlightGC (pSD, pCD->iconImageTopShadowColor, 
				  pCD->iconImageBackground,
				  pCD->iconImageTopShadowPixmap);

	botGC = GetHighlightGC (pSD, pCD->iconImageBottomShadowColor, 
				  pCD->iconImageBackground,
				  pCD->iconImageBottomShadowPixmap);
#ifdef WSM
	}
#endif /* WSM */

       /*
        *  CR5208 - Better fix than from OSF!
        *           Zero out the rectangle count in our
        *           static structures so that BevelRectangle
        *           won't extend the RList causing a memory leak.
        *           Old fix allocated and freed rectangle structure
        *           each time through.
        */
	top_rects->used = 0;	/* reset count */
	bot_rects->used = 0;

	BevelRectangle (top_rects, 
			bot_rects, 
			0, 0,
			imageWidth, imageHeight,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH,
			ICON_INTERNAL_SHADOW_WIDTH);

	XFillRectangles (DISPLAY, iconPixmap, topGC, top_rects->prect, 
							top_rects->used);
	XFillRectangles (DISPLAY, iconPixmap, botGC, bot_rects->prect,
							bot_rects->used);
    }
    return (iconPixmap);

} /* END OF FUNCTION MakeIconPixmap */



/*************************************<->*************************************
 *
 *  Pixmap
 *  MakeCachedLabelPixmap (pSD, menuW, bitmapIndex)
 *
 *
 *  Description:
 *  -----------
 *  Creates and returns a label pixmap.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  menuW = menu widget (for foreground and background colors)
 *  bitmapIndex = bitmap cache index
 *
 * 
 *  Outputs:
 *  -------
 *  Return = label pixmap or NULL.
 *
 *
 *  Comments:
 *  --------
 *  Assumes bitmapIndex is valid.
 * 
 *************************************<->***********************************/

Pixmap MakeCachedLabelPixmap (WmScreenData *pSD, Widget menuW, int bitmapIndex)
{
    BitmapCache  *bitmapc;
    PixmapCache  *pixmapc;
    int           i;
    Arg           args[5];
    Pixel         fg, bg;
    Pixmap        pixmap = (Pixmap)NULL;
    GC            gc;
    XGCValues     gcv;

    if (bitmapIndex < 0)
    {
	return ((Pixmap)NULL);
    }
    bitmapc = &(pSD->bitmapCache[bitmapIndex]);

    /*
     * Get the foreground and background colors from the menu widget.
     * Search for a label pixmap matching those colors.
     */

    i = 0;
    XtSetArg (args[i], XtNforeground, &fg); i++;
    XtSetArg (args[i], XtNbackground, &bg); i++;
    XtGetValues (menuW, (ArgList)args, i);

    pixmapc = bitmapc->pixmapCache;
    while (pixmapc)
    {
        if ((pixmapc->pixmapType == LABEL_PIXMAP) &&
	    (pixmapc->foreground == fg) &&
	    (pixmapc->background == bg))
        {
	    pixmap = pixmapc->pixmap;
	    break;
        }
	pixmapc = pixmapc->next;
    }

    if (!pixmap)
    /* 
     * A matching pixmap was not found in the pixmap cache for this bitmap.
     * Create and save the label pixmap with appropriate colors.
     */
    {
        /* 
         * Create a pixmap of the appropriate size, root, and depth.
         * Only BadAlloc error possible; BadDrawable and BadValue are avoided.
         */

        pixmap = XCreatePixmap (DISPLAY, pSD->rootWindow, 
				bitmapc->width, bitmapc->height,
                                DefaultDepth (DISPLAY, pSD->screen));

        /*
	 * Create a GC and copy the bitmap to the pixmap.
         * Only BadAlloc and BadDrawable errors are possible; others are avoided
         */

        gcv.foreground = bg;
        gcv.background = bg;
	gcv.graphics_exposures = False;
        gc = XCreateGC(DISPLAY, pixmap, (GCForeground|GCBackground), &gcv);
   
        /*
	 * Fill in the background, set the foreground, copy the bitmap to the 
         * pixmap, and free the gc.
         */

        XFillRectangle (DISPLAY, pixmap, gc, 0, 0,
		        bitmapc->width, bitmapc->height);
        XSetForeground (DISPLAY, gc, fg);
        XCopyPlane (DISPLAY, bitmapc->bitmap, pixmap, gc, 0, 0,
                    bitmapc->width, bitmapc->height, 0, 0, 1L);
        XFreeGC (DISPLAY, gc);

        /*
	 * If have sufficient memory, save the pixmap info in the pixmapCache.
	 */

	if ((pixmapc = (PixmapCache *) XtMalloc(sizeof(PixmapCache))) != NULL)
	{
	    pixmapc->pixmapType = LABEL_PIXMAP;
	    pixmapc->foreground = fg;
	    pixmapc->background = bg;
	    pixmapc->pixmap = pixmap;
	    pixmapc->next = bitmapc->pixmapCache;
	    bitmapc->pixmapCache = pixmapc;
	}
    }

    return (pixmap);

} /* END OF FUNCTION MakeCachedLabelPixmap */



/*************************************<->*************************************
 *
 *  int
 *  GetBitmapIndex (pSD, name)
 *
 *
 *  Description:
 *  -----------
 *  Retrieve bitmap from cache.
 *
 *
 *  Inputs:
 *  ------
 *  pSD = pointer to screen data
 *  name = bitmap file name or NULL pointer
 *  bitmapCache[]
 *  bitmapCacheSize
 *  bitmapCacheCount
 *
 * 
 *  Outputs:
 *  -------
 *  bitmapCache[]
 *  bitmapCacheSize
 *  bitmapCacheCount
 *  Return   = bitmap cache index or -1
 *
 *
 *  Comments:
 *  --------
 *  None
 * 
 *************************************<->***********************************/

#define BITMAP_CACHE_INC 5

#ifdef WSM
int GetBitmapIndex (WmScreenData *pSD, char *name, Boolean bReportError)
#else /* WSM */
int GetBitmapIndex (WmScreenData *pSD, char *name)
#endif /* WSM */
{
    char         *path;
    BitmapCache  *bitmapc;
    unsigned int  n;
    int           x, y;

    /*
     * Search a nonempty bitmap cache for a pathname match.
     */
    path = BitmapPathName (name);
    for (n = 0, bitmapc = pSD->bitmapCache;
	 n < pSD->bitmapCacheCount;
	 n++, bitmapc++)
    {
        if ((!path && !bitmapc->path) ||
            (path && bitmapc->path && 
	     !strcmp (path, bitmapc->path)))
        {
	    return (n);
	}
    }

    /*
     * The bitmap path name was not found in bitmapCache.
     * Find the next BitmapCache entry, creating or enlarging bitmapCache if 
     * necessary.
     */
    if (pSD->bitmapCacheSize == 0)
    /* create */
    {
        pSD->bitmapCacheSize = BITMAP_CACHE_INC;
        pSD->bitmapCache =
	    (BitmapCache *) XtMalloc (BITMAP_CACHE_INC * sizeof (BitmapCache));
    }
    else if (pSD->bitmapCacheCount == pSD->bitmapCacheSize)
    /* enlarge */
    {
        pSD->bitmapCacheSize += BITMAP_CACHE_INC;
        pSD->bitmapCache = (BitmapCache *) 
	    XtRealloc ((char*)pSD->bitmapCache, 
		     pSD->bitmapCacheSize * sizeof (BitmapCache));
    }

    if (pSD->bitmapCache == NULL)
    {
        MWarning (((char *)GETMESSAGE(38, 5, "Insufficient memory for bitmap %s\n")), name);
	pSD->bitmapCacheSize = 0;
	pSD->bitmapCacheCount = 0;
	return (-1);
    }

    bitmapc = &(pSD->bitmapCache[pSD->bitmapCacheCount]);

    /*
     * Fill the entry with the bitmap info.
     * A NULL path indicates the builtin icon bitmap.
     * Indicate that no pixmapCache exists yet.
     */

    if (path)
    {
        if ((bitmapc->path = (String)
                 XtMalloc ((unsigned int)(strlen (path) + 1))) == NULL)
        {
            MWarning (((char *)GETMESSAGE(38, 6, "Insufficient memory for bitmap %s\n")), name);
	    return (-1);
        }
        strcpy (bitmapc->path, path);

        if (XReadBitmapFile (DISPLAY, pSD->rootWindow, path, 
			     &bitmapc->width, &bitmapc->height, 
			     &bitmapc->bitmap, &x, &y)
            != BitmapSuccess)
        {
#ifdef WSM
	  if (bReportError)
#endif /* WSM */
            MWarning (((char *)GETMESSAGE(38, 7, "Unable to read bitmap file %s\n")), path);
	    XtFree ((char *)bitmapc->path);
	    return (-1);
        }

        if (bitmapc->width == 0 || bitmapc->height == 0)
        {
#ifdef WSM
	  if (bReportError)
#endif /* WSM */
            MWarning (((char *)GETMESSAGE(38, 8, "Invalid bitmap file %s\n")), path);
	    XtFree ((char *)bitmapc->path);
	    return (-1);
        }
    }
    else
    /* builtin icon bitmap */
    {
        bitmapc->path   = NULL;
        bitmapc->bitmap = pSD->builtinIconPixmap;
        bitmapc->width  = iImage_width;
        bitmapc->height = iImage_height;
    }

    bitmapc->pixmapCache = NULL;

    return (pSD->bitmapCacheCount++);

} /* END OF FUNCTION GetBitmapIndex */



/*************************************<->*************************************
 *
 *  BitmapPathName (string)
 *
 *
 *  Description:
 *  -----------
 *  Constructs a bitmap file pathname from the bitmap file name and the
 *  bitmapDirectory resource value.
 *
 *
 *  Inputs:
 *  ------
 *  string = bitmap file name or NULL
 *  wmGD.bitmapDirectory = bitmapDirectory resource value
 *  HOME = environment variable for home directory
 *  XBMLANGPATH
 *  XAPPLRESDIR
 *
 * 
 *  Outputs:
 *  -------
 *  Return = string containing the bitmap file pathname or NULL.
 *
 *
 *  Comments:
 *  --------
 *  If the bitmap file does not exist, searches using XBMLANGPATH.
 *  Returns NULL path name for a NULL file name.
 * 
 *************************************<->***********************************/

char *BitmapPathName (string)
    char *string;

{
    static char  fileName[MAXWMPATH+1];
    char *retname;
    SubstitutionRec subs[1];
#ifndef MOTIF_ONE_DOT_ONE
    char *homeDir = XmeGetHomeDirName();
#endif

    if (!string || !*string)
    {
	return (NULL);
    }

    /*
     * Interpret "~/.." as relative to the user's home directory.
     * Interpret "/.." as an absolute pathname.
     * If the bitmapDirectory resource is nonNULL, interpret path as relative
     *   to it.
     * Else, or if bitmapDirectory has no such file, use a XBMLANGPATH lookup.
     */

    if ((string[0] == '~') && (string[1] == '/'))
    /* 
     * Handle "~/.." 
     */
    {
#ifdef MOTIF_ONE_DOT_ONE
	GetHomeDirName(fileName);
#else
	strcpy (fileName, homeDir);
#endif
        strncat (fileName, &(string[1]), MAXWMPATH - strlen (fileName));
	return (fileName);
    }

    if (string[0] == '/')
    {
      return(string);
    }

    if (wmGD.bitmapDirectory && *wmGD.bitmapDirectory)
    /*
     * Relative to nonNULL bitmapDirectory (which may have relative to HOME)
     */
    {
	if ((wmGD.bitmapDirectory[0] == '~') &&
	    (wmGD.bitmapDirectory[1] == '/'))
	{
#ifdef MOTIF_ONE_DOT_ONE
	    GetHomeDirName(fileName);
#else
	    strcpy (fileName, homeDir);
#endif
            strncat (fileName, &wmGD.bitmapDirectory[1],
		     MAXWMPATH - strlen (fileName));
	} else {
	    strcpy (fileName, wmGD.bitmapDirectory);
	}
        strncat (fileName, "/", MAXWMPATH - strlen (fileName));
        strncat (fileName, string, MAXWMPATH - strlen (fileName));

/* Test file for existence. */

	subs[0].substitution = "";
	if ((retname = XtFindFile(fileName, subs, 0,
				  (XtFilePredicate) NULL)) != NULL) {
	  XtFree(retname);
	  return (fileName);
	}
    }

    /* Fall back on a path search */

#ifdef MOTIF_ONE_DOT_ONE
    return (NULL);
#else
    {
	char *search_path;
	Boolean user_path;

	search_path = _XmOSInitPath(string, MATCH_PATH, &user_path);
	subs[0].match = user_path ? MATCH_XBM : MATCH_CHAR;
	subs[0].substitution = string;
	retname = XtResolvePathname(DISPLAY, "bitmaps", NULL, NULL,
				    search_path, subs, XtNumber(subs), 
					(XtFilePredicate)NULL);
	XtFree(search_path);

	if (!retname)
	  return (string);

	strncpy(fileName, retname, MAXWMPATH);
	XtFree(retname);
	return (fileName);
    }
#endif

} /* END OF FUNCTION BitmapPathName */

#ifdef WSM
/****************************   eof    ***************************/
#endif /* WSM */
