#include "GStatusView.h"

#include <string.h>

//=========================================================================

GStatusView::GStatusView( char* msg, BRect rect )
	   	   : BView(rect, "GStatusView", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW )
{
	SetFontSize(12);
	SetViewColor( 230, 230, 230 );
	strcpy( showString, msg );
	spinner = NULL;
}

//-------------------------------------------------------------------------

GStatusView::~GStatusView() {
	SetSpinner(false);
}

//-------------------------------------------------------------------------

void GStatusView::Draw( BRect ) {

	BRect frame = Bounds();

	SetHighColor( 156, 154, 156 );		// dark grey
	BPoint start = frame.LeftTop();
	BPoint end = frame.RightTop();
	StrokeLine( start, end );

	SetHighColor( 0, 0, 0 );
	SetLowColor( 222, 219, 222 );
	SetFont(be_plain_font);
	SetFontSize(11);

	if( spinner )
		MovePenTo(frame.left + 60, frame.bottom-2);
	else
		MovePenTo(frame.left + 4, frame.bottom-2);

	DrawString( showString );
}

//-------------------------------------------------------------------------

void GStatusView::MouseDown( BPoint point ) {

	BView::MouseDown( point );
}

//-------------------------------------------------------------------------

void GStatusView::SetMessage( char* msg ) {
	strcpy( showString, msg );
	Invalidate();
}

//-------------------------------------------------------------------------

void GStatusView::AttachedToWindow() {
	if( spinner )
		spinner->Start();
}

//-------------------------------------------------------------------------

void GStatusView::SetSpinner( bool on ) {

	// The spinner is a feisty little beast and prone to crashes...
	// just to be safe, recreate it every time we need it
	if( on ) {
		// make the spinner
		spinner = new BarberPole( BRect(4,4,50,11), "spinner", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW );
		spinner->SetHighColor(0,128,0);
		AddChild( spinner );
		if( Window() )
			spinner->Start();
	}

	// if it's not needed, delete it
	else {
		if( spinner ) {
			spinner->Stop();
			spinner->RemoveSelf();
			delete spinner;
			spinner = 0;
		}
	}
}

//=========================================================================
