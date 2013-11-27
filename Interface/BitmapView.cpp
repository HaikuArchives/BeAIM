#include "BitmapView.h"
#include <Application.h>
#include <Bitmap.h>
#include <Roster.h>
#include "MiscStuff.h"
#include "Globals.h"
#include "Say.h"


BitmapView::BitmapView( BRect frame, BBitmap *bmap )
			   : BView( frame, "bitmap_view", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE ){

	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	themap = bmap;
	bmbounds = themap->Bounds();
	urlMode = 0;
}


BitmapView::~BitmapView() {
	delete themap;
}


void BitmapView::Draw( BRect updateRect ) {

	// calculate the offsets to center the bitmap
	float LOffset = (Bounds().Width() - bmbounds.Width()) / 2;
	float TOffset = (Bounds().Height() - bmbounds.Height()) / 2;	

	// Make a new rect w/ those coordinates
	BRect drawRect( LOffset, TOffset, Bounds().Width() - LOffset, Bounds().Height() - TOffset );

	if ( Window()->Lock() )
	{
		DrawBitmap( themap, drawRect );
		Window()->Unlock();
	}
}


void BitmapView::SetURLMode( BString u, bool web ) {
	
	urlMode = web ? 1 : 2;
	url = u;
}


void BitmapView::MouseDown(BPoint point) {
	
	status_t result;

	if( urlMode ) {
	
		if( urlMode == 1 ) {
			char* link = url.LockBuffer(0);
			result = be_roster->Launch( "text/html", 1, &link );
			url.UnlockBuffer();
			if( (result != B_NO_ERROR) && (result != B_ALREADY_RUNNING) ) {
				windows->ShowMessage( Language.get("ERR_NO_HTML_HANDLER") );
			}
		}
		
		if( urlMode == 2 ) {
			char* link = url.LockBuffer(0);
			result = be_roster->Launch( "text/x-email", 1, &link );
			url.UnlockBuffer();
			if( (result != B_NO_ERROR) && (result != B_ALREADY_RUNNING) ) {
				windows->ShowMessage( Language.get("ERR_NO_MAIL_HANDLER") );
			}
		}
	}
	
	BView::MouseDown(point);
}
