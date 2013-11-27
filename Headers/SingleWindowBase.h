#ifndef _SINGLE_WINDOW_BASE_H_
#define _SINGLE_WINDOW_BASE_H_

#include <Window.h>
#include <Message.h>

class SingleWindowBase : public BWindow { 

	public:
		SingleWindowBase( int swType, BRect frame, const char* title, window_type type, uint32 flags, uint32 workspaces = B_CURRENT_WORKSPACE );
		SingleWindowBase( int swType, BRect frame, const char* title, window_look look, window_feel feel, uint32 flags, uint32 workspace = B_CURRENT_WORKSPACE );
		virtual bool QuitRequested();
		
		int SingleWindowType() {
			return singleWindowType;
		};

	private:
		int singleWindowType;
};

#endif
