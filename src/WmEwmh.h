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


#ifndef WmEwmh_h
#define WmEwmh_h

void SetupWmEwmh(void);
void ProcessEwmh(ClientData*);
void ProcessEwmhWindowType(ClientData*);
void SetEwmhActiveWindow(ClientData*);
void HandleEwmhCPropertyNotify(ClientData*, XPropertyEvent*);
void HandleEwmhClientMessage(ClientData*, XClientMessageEvent*);
Boolean HandleEwmhRootClientMessage(WmScreenData*, XClientMessageEvent*);
void ConfigureEwmhFullScreen(ClientData*, Boolean);
void UpdateEwmhClientList(WmScreenData*);
void UpdateEwmhClientState(ClientData*);
void UpdateEwmhWorkspaceProperties(WmScreenData*);
void UpdateEwmhActiveWorkspace(WmScreenData*, WorkspaceID);

#endif /* WmEwmh_h */
