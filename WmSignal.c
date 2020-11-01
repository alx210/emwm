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
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: WmSignal.c /main/6 1996/10/17 16:20:07 drk $"
#endif
#endif
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h" /* This should be the first include */
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


/*
 * include extern functions
 */

#include "WmFeedback.h"
#include "WmFunction.h"


/*
 * Function Declarations:
 */

#include "WmSignal.h" 


/*
 * Global Variables:
 */


#ifdef WSM

/*************************************<->*************************************
 *
 *  AbortWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a fatal signal. We reset
 *  the keyboard focus to pointer root before aborting.
 *
 *************************************<->***********************************/
static void
AbortWmSignalHandler (int sig)
{
    struct sigaction sa;

    /*
     * Set input focus back to pointer root
     */
    XSetInputFocus(DISPLAY, PointerRoot, RevertToPointerRoot, CurrentTime);
    XSync (DISPLAY, False);
    XCloseDisplay (DISPLAY);

    /*
     * Invoke the default handler
     */
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
    (void) sigaction (sig, &sa, (struct sigaction *) 0);

    kill (getpid(), sig);

} /* END OF FUNCTION AbortSignalHandler */


/*************************************<->*************************************
 *
 *  RestoreDefaultSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

void
RestoreDefaultSignalHandlers (void)
{
    struct sigaction sa;
    struct sigaction osa;

    /*
     * Restore default action for signals we're interested in.
     */

    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;

    if ((sigaction (SIGINT, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGINT, &sa, (struct sigaction *) 0);
    }

    if ((sigaction (SIGHUP, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGHUP, &sa, (struct sigaction *) 0);
    }

    (void) sigaction (SIGQUIT, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGTERM, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGPIPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGCHLD, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGILL, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGFPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGBUS, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);
#ifdef SIGSYS
    (void) sigaction (SIGSYS, &sa, (struct sigaction *) 0);
#endif
}
#endif /* WSM */


/*************************************<->*************************************
 *
 *  SetupWmSignalHandlers ()
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the signal handlers for the window manager.
 *
 *************************************<->***********************************/

void SetupWmSignalHandlers (int dummy)
{
    void (*signalHandler) ();

#ifdef WSM
    struct sigaction 	sa;
    struct sigaction 	osa;

    /*
     * Catch software signals that we ask the user about
     * before quitting.
     */
    (void) sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = QuitWmSignalHandler;

    if ((sigaction (SIGINT, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGINT, &sa, (struct sigaction *) 0);
    }

    if ((sigaction (SIGHUP, (struct sigaction *) 0, &osa) != 0) ||
	(osa.sa_handler != SIG_IGN))
    {
	(void) sigaction (SIGHUP, &sa, (struct sigaction *) 0);
    }

    (void) sigaction (SIGQUIT, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGTERM, &sa, (struct sigaction *) 0);

    /*
     * Ignore child death
     */

#ifdef SA_NOCLDWAIT
    sa.sa_flags = SA_NOCLDWAIT;  /* Don't create zombies */
#else
    sa.sa_flags = 0;
#endif
    sa.sa_handler = SIG_IGN;
    (void) sigaction (SIGCHLD, &sa, (struct sigaction *) 0);
    sa.sa_flags = 0;

    /* 
     * Catch other fatal signals so we can reset the 
     * keyboard focus to pointer root before aborting
     */
    sa.sa_handler = AbortWmSignalHandler;

    (void) sigaction (SIGILL, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGFPE, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGBUS, &sa, (struct sigaction *) 0);
    (void) sigaction (SIGSEGV, &sa, (struct sigaction *) 0);
#ifdef SIGSYS
    (void) sigaction (SIGSYS, &sa, (struct sigaction *) 0);
#endif

#else /* not WSM - original mwm code*/

    signalHandler = (void (*)())signal (SIGINT, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGINT, QuitWmSignalHandler);
    }

    signalHandler = (void (*)())signal (SIGHUP, SIG_IGN);
    if (signalHandler != (void (*)())SIG_IGN)
    {
	signal (SIGHUP, QuitWmSignalHandler);
    }

    signal (SIGQUIT, QuitWmSignalHandler);

    signal (SIGTERM, QuitWmSignalHandler);

    signal (SIGCHLD, ChildProcSignalHandler);
#endif /* WSM */


} /* END OF FUNCTION SetupWmSignalHandlers */


/*************************************<->*************************************
 *
 *  QuitWmSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a signal that is to terminate the
 *  window manager.
 *
 *************************************<->***********************************/

void QuitWmSignalHandler (int dummy)
{
    if (wmGD.showFeedback & WM_SHOW_FB_KILL)
    {
	ConfirmAction (ACTIVE_PSD, QUIT_MWM_ACTION);
	XFlush(DISPLAY);
    }
    else
    {
	Do_Quit_Mwm(False);
    }

} /* END OF FUNCTION QuitWmSignalHandler */


/*************************************<->*************************************
 *
 *  ChildProcSignalHandler ()
 *
 *
 *  Description:
 *  -----------
 *  This function is called on receipt of a signal that the child of the 
 *  window manager has terminated.
 *  This child signal handler is primarily used to wait() for child
 *  processes that were inherited from the user's .xsession if the last
 *  command in the .xsession is a "exec mwm".  These inherited child
 *  processes were not spawned the WmFunction.c:F_Exec() routine and
 *  therefore would not be reaped by the wait() in that routine until
 *  a menu item using F_Exec() was invoked by the user.
 *
 *************************************<->***********************************/

void ChildProcSignalHandler (int dummy)
{
   pid_t pid;
   int status;
   void (*intStat) ();
   void (*quitStat) ();
   
   intStat = (void (*)())signal (SIGINT, SIG_IGN);
   quitStat = (void (*)())signal (SIGQUIT, SIG_IGN);

   pid = wait(&status);
   signal(SIGCHLD, ChildProcSignalHandler); 

   signal (SIGINT, intStat);
   signal (SIGQUIT, quitStat);
}
