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


/********    Public Function Declarations    ********/

void ChangeToWorkspace(WmWorkspaceData *pNewWS);

void ChangeWorkspaceTitle( WmWorkspaceData *pWS, char * pchTitle);
Boolean DuplicateWorkspaceName (
			WmScreenData *pSD, 
			unsigned char *name, 
			int num);
void UpdateWorkspacePresenceProperty( 
                        ClientData *pCD) ;

WmWorkspaceData * CreateWorkspace( 
                        WmScreenData *pSD,
                        unsigned char *name) ;
void DeleteWorkspace( 
                        WmWorkspaceData *pWS) ;
Boolean GetClientWorkspaceInfo( 
                        ClientData *pCD,
                        long manageFlags) ;
Boolean WorkspaceIsInCommand( 
                        Display *dpy,
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pNumIDs) ;
Boolean ConvertNamesToIDs( 
                        WmScreenData *pSD,
                        unsigned char *pchIn,
                        WorkspaceID **ppAtoms,
                        unsigned int *pNumAtoms) ;
void CheckForPutInAllRequest( 
                        ClientData *pCD,
                        Atom *pIDs,
                        unsigned int numIDs) ;
Boolean FindWsNameInCommand( 
                        int argc,
                        char *argv[],
                        unsigned char **ppch) ;
void PutClientIntoWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
void TakeClientOutOfWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
WmWorkspaceData * GetWorkspaceData( 
                        WmScreenData *pSD,
                        WorkspaceID wsID) ;
unsigned char * GenerateWorkspaceName( 
                        WmScreenData *pSD,
                        int wsnum) ;
Boolean InWindowList( 
                        Window w,
                        Window wl[],
                        int num) ;
Boolean ClientInWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
WsClientData * GetWsClientData( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
void SetClientWsIndex( 
                        ClientData *pCD) ;
void ProcessDtWmHints (ClientData *pCD) ;
Boolean ProcessWorkspaceHints( 
                        ClientData *pCD) ;
void ProcessWorkspaceHintList( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void RemoveSingleClientFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void RemoveSubtreeFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
WorkspaceID * GetListOfOccupiedWorkspaces( 
			ClientData *pCD,
                        int *numIDs) ;
void HonorAbsentMapBehavior(
			ClientData *pCD) ;
void RemoveClientFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void AddSingleClientToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void AddSubtreeToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void AddClientToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
void AddClientToWsList( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
void RemoveClientFromWsList( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
Boolean F_CreateWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
Boolean F_DeleteWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
Boolean F_GotoWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
Boolean F_AddToAllWorkspaces( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
Boolean F_Remove( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
int GetCurrentWorkspaceIndex( 
                        WmScreenData *pSD) ;
void InsureIconForWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
Boolean GetLeaderPresence( 
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pnumIDs) ;
Boolean GetMyOwnPresence( 
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pnumIDs) ;
void ReserveIdListSpace( 
                        int numIDs) ;

void AddStringToResourceData( 
                        char *string,
                        char **pdata,
                        int *plen) ;

void AddPersistentWindows(WmWorkspaceData *pWS);

/********    End Public Function Declarations    ********/

