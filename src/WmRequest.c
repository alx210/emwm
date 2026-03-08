/*
 * Copyright (C) 2018-2026 alx@fastestcode.org
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <ctype.h>
#include "WmGlobal.h"
#include "WmBackdrop.h"
#include "WmWrkspace.h"
#include "WmRequest.h"
#include "WmResParse.h"
#include "WmError.h"

static void ProcessChangeBackdrop(WmScreenData*, const char *param);
static void ProcessChangeWorkspace(WmScreenData*, const char *param);
static void ProcessAddWorkspace(WmScreenData*, const char *param);
static void ProcessDeleteWorkspace(WmScreenData*, const char *param);
static void ProcessNameWorkspace(WmScreenData*, const char *param);


void ProcessWmRequest(WmScreenData *pSD, char *pchReq)
{
	const char sz_msgfmt[] = "Invalid window manager request: \"%s\".";
	size_t len;
	char *msg;

	if(!strncmp(pchReq, REQ_BACKDROP, REQ_NAME_LEN)) {
		ProcessChangeBackdrop(pSD, pchReq + REQ_NAME_LEN);
		return;
	}

	if(!strncmp(pchReq, REQ_WRKSPACE, REQ_NAME_LEN)) {
		ProcessChangeWorkspace(pSD, pchReq + REQ_NAME_LEN);
		return;
	}

	if(!strncmp(pchReq, REQ_ADDWKSPC, REQ_NAME_LEN)) {
		ProcessAddWorkspace(pSD, pchReq + REQ_NAME_LEN);
		return;
	}
	
	if(!strncmp(pchReq, REQ_DELWKSPC, REQ_NAME_LEN)) {
		ProcessDeleteWorkspace(pSD, pchReq + REQ_NAME_LEN);
		return;
	}

	if(!strncmp(pchReq, REQ_NAMWKSPC, REQ_NAME_LEN)) {
		ProcessNameWorkspace(pSD, pchReq + REQ_NAME_LEN);
		return;
	}

	/* only reached if the request string didn't contain a valid name */
	len = snprintf(NULL, 0, sz_msgfmt, pchReq);
	msg = XtMalloc(len + 1);
	sprintf(msg, sz_msgfmt, pchReq);
	Warning(msg);
	XtFree(msg);
}

/* Parses the workspace change request */
static void ProcessChangeWorkspace(WmScreenData *pSD, const char *param)
{
	Atom ws_id;
	int i;

	ws_id = (Atom)strtol(param, NULL, 0);

	for(i = 0; i < pSD->numWorkspaces; i++) {
		if(pSD->pWS[i].id == ws_id) break;
	}

	if(i == pSD->numWorkspaces) {
		Warning("Invalid workspace ID in Change Workspace request.");
	} else {
		ChangeToWorkspace(&pSD->pWS[i]);
	}
}

/*
 * Parses the backdrop change request parameter string and sets
 * the backdrop accordingly.
 */
static void ProcessChangeBackdrop(WmScreenData *psd, const char *param)
{
	WmWorkspaceData *pwd;
	char *psz_param = XtNewString(param);
	unsigned char *pctx = (unsigned char*)psz_param;
	char *file_name = "none";
	int n = 0;
	Atom ws_id;
	Pixel fg, bg;
	char *p;
	
	if(!psz_param) return;	

	while((p = (char*)GetString(&pctx))) {
		switch(n) {
			case 0:
			ws_id = (Atom) strtol(p, NULL, 0);
			break;
			case 1:
			fg = (Pixel) strtol(p, NULL, 0);
			break;
			case 2:
			bg = (Pixel) strtol(p, NULL, 0);
			break;
			case 3:
			file_name = p;
			break;
		}						
		n++;
	}
	
	if(n < 3) {
		Warning("Missing parameters in Change Backdrop request.");
		XtFree(psz_param);
		return;
	}
	
	pwd = GetWorkspaceData(psd, ws_id);
	if(!pwd) {
		Warning("Invalid workspace ID in Change Backdrop request.");
		XtFree(psz_param);
		return;
	}

	pwd->backdrop.background = bg;
	pwd->backdrop.foreground = fg;

	SetNewBackdrop(pwd, file_name);

	XtFree(psz_param);
}

static void ProcessAddWorkspace(WmScreenData *pSD, const char *param)
{
	char *psz_param = XtNewString(param);
	unsigned char *pctx = (unsigned char*)psz_param;
	char *title = NULL;
	
	if(strlen(psz_param))
		title = (char*)GetString(&pctx);
	
	CreateWorkspace(pSD, (unsigned char*)title);

	XtFree(psz_param);
}

static void ProcessDeleteWorkspace(WmScreenData  *pSD, const char *param)
{
	Atom ws_id;
	WmWorkspaceData *pwd;
	
	ws_id = (Atom)strtol(param, NULL, 0);
	pwd = GetWorkspaceData(pSD, ws_id);
	if(!pwd) {
		Warning("Invalid workspace ID in Delete Workspace request.");
		return;
	}
	DeleteWorkspace(pwd);
}

static void ProcessNameWorkspace(WmScreenData *pSD, const char *param)
{
	char *psz_param = XtNewString(param);
	unsigned char *pctx = (unsigned char*)psz_param;
	WmWorkspaceData *pwd;
	Atom ws_id;
	char *p;
	char *title = NULL;
	int n = 0;

	if(!psz_param) return;	

	while((p = (char*)GetString(&pctx))) {
		switch(n) {
			case 0:
			ws_id = (Atom) strtol(p, NULL, 0);
			break;
			
			case 1:
			title = p;
			break;
		}						
		n++;
	}

	if(n < 2) {
		Warning("Missing parameters in Name Workspace request.");
		XtFree(psz_param);
		return;
	}
	
	pwd = GetWorkspaceData(pSD, ws_id);
	if(!pwd) {
		Warning("Invalid workspace ID in Name Workspace request.");
		XtFree(psz_param);
		return;
	}

	ChangeWorkspaceTitle(pwd, title);

	XtFree(psz_param);
}
