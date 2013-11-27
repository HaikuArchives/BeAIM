#ifndef _LESS_ANNOYING_WINDOW_H_
#define _LESS_ANNOYING_WINDOW_H_

#include <Window.h>
#include <OS.h>


/*
Notes:

This class is a hack. There's a bug in BeOS that has the following effect...
when you set a window to be in all workspaces via the SetFlags() function,
then that window will *always* be active whenever you first switch to a given
workspace, regardless of whether or not it was active the last time you used
that workspace. (Or something like that; I reported it to Be once and got
ignored). Anyway, this class has some workarounds to try and fix it, and does
a reasonably good job, but it's still a major hack. If Be would fix their
bugs once in a while, then it wouldn't be needed, but they don't, so it is.

*/

class LessAnnoyingWindow : public BWindow 
{
	public:
		LessAnnoyingWindow( BRect frame,
							const char* title,
							window_look look,
							window_feel feel,
							uint32 flags); 

		void SetAllWorkspaces( bool );
		bool GetAllWorkspaces();
		bool WasJustActivated();
		bool WasJustDeactivated();
		
		void WorkspaceActivated( int32 workspace, bool active );
		void WindowActivated( bool active );

	private:
		bool allWorkspaces;
		bool wsWasActive[32];
		bigtime_t lastDeactivated;
		bigtime_t lastActivated;
};

#endif
