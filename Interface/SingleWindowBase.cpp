#include "SingleWindowBase.h"
#include "constants.h"
#include "MiscStuff.h"
#include "Say.h"

//-----------------------------------------------------

SingleWindowBase::SingleWindowBase( int swType, BRect frame, const char* title, window_type type, uint32 flags, uint32 workspaces )
				: BWindow( frame, title, type, flags, workspaces )
{
	singleWindowType = swType;
}

//-----------------------------------------------------

SingleWindowBase::SingleWindowBase( int swType, BRect frame, const char* title, window_look look, window_feel feel, uint32 flags, uint32 workspace )
				: BWindow( frame, title, look, feel, flags, workspace )
{
	singleWindowType = swType;
}

//-----------------------------------------------------

bool SingleWindowBase::QuitRequested() {
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", singleWindowType );
	PostAppMessage( clsMessage );
	return true;
}

//-----------------------------------------------------
