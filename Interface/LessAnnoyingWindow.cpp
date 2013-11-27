#include <Application.h>
#include "LessAnnoyingWindow.h"
#include "Say.h"

#define THRESHOLD_TIME 50000

LessAnnoyingWindow::LessAnnoyingWindow(BRect frame, const char* title, window_look look,
										window_feel feel, uint32 flags)
									: BWindow(frame, title, look, feel, flags )
{
	// defaults
	allWorkspaces = false;
	lastDeactivated = 0;
	lastActivated = 0;
	
	// make the workspaces array
	for( int i = 0; i < 32; ++i )
		wsWasActive[i] = false;
}

void LessAnnoyingWindow::SetAllWorkspaces( bool aw ) {
	allWorkspaces = aw;
	if( allWorkspaces )
		SetWorkspaces( B_ALL_WORKSPACES );
	else
		SetWorkspaces( B_CURRENT_WORKSPACE );
}

bool LessAnnoyingWindow::GetAllWorkspaces() {
	return allWorkspaces;
}

void LessAnnoyingWindow::WorkspaceActivated( int32 workspace, bool active ) {

	// don't do any of this if we're only in current workspace mode...
	if( !allWorkspaces )
		return;
		
//	printf( "switching %s workspace %ld\n", active ? "TO" : "FROM", workspace );

	// save out the window's active state right before the switch
	if( !active ) {
//		printf( "... current window state: %s\n", IsActive() ? "ACTIVE" : "INACTIVE" );
		if( !WasJustActivated() && IsActive() )
			wsWasActive[workspace] = true;
		else
			wsWasActive[workspace] = false;
	}
	
	// OK, we're switching *to* this workspace...
	else {

//		printf( "... the window will now be %s\n", wsWasActive[workspace] ? "ACTIVATED" : "DEACTIVATED" );

		// if the window was active when we last left this workspace, activate it again
		if( !wsWasActive[workspace] )
			Activate(false);
	}
}

void LessAnnoyingWindow::WindowActivated( bool active ) {
	bigtime_t now = real_time_clock_usecs();
	if( active )
		lastActivated = now;
	else
		lastDeactivated = now;
//	printf( "Window %s\n", active ? "ACTIVATED" : "DEACTIVATED" );
}

bool LessAnnoyingWindow::WasJustActivated() {
	bigtime_t now;

	// find out how long it's been since the window was activated
	now = real_time_clock_usecs();
	bigtime_t diff = now - lastActivated;

//	printf( "WasJustActivated: %s   (diff=%ld)\n", diff < THRESHOLD_TIME ? "YUP" : "NOPE", (int32)diff );

	if( diff < THRESHOLD_TIME )
		return true;
	return false;
}

bool LessAnnoyingWindow::WasJustDeactivated() {
	bigtime_t now;

	// find out how long it's been since the window was deactivated
	now = real_time_clock_usecs();
	bigtime_t diff = now - lastDeactivated;

//	printf( "WasJustDeactivated: %s   (diff=%ld)\n", diff < THRESHOLD_TIME ? "YUP" : "NOPE", (int32)diff );

	if( diff < THRESHOLD_TIME )
		return true;
	return false;
}

