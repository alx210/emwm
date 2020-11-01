/* $XConsortium: WmOL.h /main/4 1995/11/01 11:45:27 rswiston $ */
/* 
 * (c) Copyright 1989 Sun Microsystems, Inc.
 * (c) Copyright 1993 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
 */ 

#ifndef _WM_OL_H
#define _WM_OL_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifndef NO_OL_COMPAT

#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

/*
 * Property Names 
 */
#define OL_WIN_ATTR		"_OL_WIN_ATTR"
#define OL_DECOR_ADD		"_OL_DECOR_ADD"
#define OL_DECOR_DEL		"_OL_DECOR_DEL"

/*
 * Decoration atoms
 */
#define OL_DECOR_RESIZE		"_OL_DECOR_RESIZE"
#define OL_DECOR_FOOTER		"_OL_DECOR_FOOTER"
#define OL_DECOR_HEADER		"_OL_DECOR_HEADER"
#define OL_DECOR_PIN		"_OL_DECOR_PIN"
#define OL_DECOR_CLOSE		"_OL_DECOR_CLOSE"
#define OL_DECOR_ICON_NAME	"_OL_DECOR_ICON_NAME"
#define OL_PIN_IN		"_OL_PIN_IN"
#define OL_PIN_OUT		"_OL_PIN_OUT"

/*
 * Window types
 */
#define OL_WT_BASE		"_OL_WT_BASE"
#define OL_WT_CMD		"_OL_WT_CMD"
#define OL_WT_NOTICE		"_OL_WT_NOTICE"
#define OL_WT_HELP		"_OL_WT_HELP"
#define OL_WT_OTHER		"_OL_WT_OTHER"

/*
 * Menu Types
 */
#define OL_MENU_LIMITED		"_OL_MENU_LIMITED"
#define OL_MENU_FULL		"_OL_MENU_FULL"

/*
 * Structure of Open Look window attribute property
 */
typedef struct _OLWinAttr {
    unsigned long	flags;
    Atom		win_type;
    Atom		menu_type;
    unsigned long	pin_initial_state;
    unsigned long	cancel;
} OLWinAttr;
#define OLWINATTRLENGTH (sizeof(OLWinAttr)/sizeof(unsigned long))

typedef struct _old_OLWinAttr {
    Atom		win_type;
    Atom		menu_type;
    unsigned long	pin_initial_state;
} old_OLWinAttr;
#define OLDOLWINATTRLENGTH (sizeof(old_OLWinAttr)/sizeof(unsigned long))

/* pin states */
#define PIN_OUT		0
#define PIN_IN		1

/*
 * Values for flags in OLWinAttr
 */
#define WA_WINTYPE      (1<<0)
#define WA_MENUTYPE     (1<<1)
#define WA_PINSTATE     (1<<2)
#define WA_CANCEL       (1<<3)

#define ENTIRE_CONTENTS         (10000000L)

/*
 * Bit Flags for OL Window Decoration
 */
#define OLDecorHeader		(1L<<0)
#define OLDecorFooter		(1L<<1)
#define OLDecorPushPin		(1L<<2)
#define OLDecorCloseButton	(1L<<3)
#define OLDecorResizeable	(1L<<4)
#define OLDecorIconName		(1L<<5)
#define OLDecorWarpToPin	(1L<<6)

/*
 * Public Functions
 */
extern Boolean HasOpenLookHints( ClientData *pCD );
extern OLWinAttr * GetOLWinAttr( ClientData *pCD );
extern Boolean GetOLDecorFlags(ClientData *pCD, Atom property,
	unsigned long *pDecor);


/*
 * Macros (public pseudo-functions)
 */
#define GetOLDecorAdd(pcd,ptr) (GetOLDecorFlags(pcd,wmGD.xa_OL_DECOR_ADD,ptr))
#define GetOLDecorDel(pcd,ptr) (GetOLDecorFlags(pcd,wmGD.xa_OL_DECOR_DEL,ptr))


#endif /* NO_OL_COMPAT */
/* Do not add anything after the following #endif */
#endif /* _WM_OL_H */
