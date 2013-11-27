#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <stdlib.h>
#include "StringView.h"
#include "ColorPicker.h"
#include "DLanguageClass.h"
#include "Say.h"

//=========================================================================

ColorPickerWindow::ColorPickerWindow( BRect frame, const char* title, int id, BWindow* own )
	 				: BWindow(frame, title, B_TITLED_WINDOW,
	 				  B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new ColorPickerView( aRect );
	AddChild( genView );
	
	// other stuff
	SetFeel( B_MODAL_SUBSET_WINDOW_FEEL );
	owner = own;
	cid = id;
}

//-------------------------------------------------------------------------

ColorPickerWindow::~ColorPickerWindow() {
}

//-------------------------------------------------------------------------

bool ColorPickerWindow::QuitRequested()
{
	//be_app->PostMessage( new BMessage(B_QUIT_REQUESTED) );
	return(true);
}

//-------------------------------------------------------------------------

void ColorPickerWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case BEAIM_CP_COLOR_CHANGED:
			genView->colPick->SetColor( genView->picker->ValueAsColor() );
			break;
			
		case B_OK:
			if( owner ) {
				rgb_color color = genView->picker->ValueAsColor();
				BMessage* msg = new BMessage(BEAIM_NEW_COLOR_PICKED);
				msg->AddInt32( "cid", (int32)cid );
				msg->AddData( "color", B_RGB_COLOR_TYPE, &color, sizeof(rgb_color) );
				owner->PostMessage( msg );
			}
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			break;
			
		default:
			BWindow::MessageReceived(message);
	}
}

//-------------------------------------------------------------------------

// the cancel function
void ColorPickerWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			return;
		}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-------------------------------------------------------------------------

void ColorPickerWindow::SetTheColor( rgb_color clr ) {

	genView->picker->SetValue( clr );
	genView->colPick->SetColor( clr );
}

//=========================================================================

ColorPickerView::ColorPickerView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );
	
	picker = new BColorControl( BPoint(7,7), B_CELLS_32x8, 7, "colorpicker", new BMessage(BEAIM_CP_COLOR_CHANGED) );
	AddChild( picker );

	// Set up the view rectangles
	BRect textframe = BRect( 7, 23, 265, 136 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;
	
	// make the colorview
	colPick = new ColorView( BRect( 8, 75, 38, 103 ), -1 );
	AddChild( colPick );

	// make the new button
	BRect buttonrect = BRect( 195, 79, 245, 0 );
	btnSave = new BButton(buttonrect, "save", Language.get("SAVE_LABEL"), new BMessage(B_OK), B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW );
	btnSave->ResizeToPreferred();
	btnSave->MakeDefault(true);
	AddChild( btnSave );
	buttonrect = BRect( 255, 79, 305, 0 );
	btnCancel = new BButton(buttonrect, "cancel", Language.get("CANCEL_LABEL"), new BMessage(B_QUIT_REQUESTED), B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_WILL_DRAW);
	btnCancel->ResizeToPreferred();
	AddChild( btnCancel );
	
	btnCancel->MoveTo( Bounds().Width() - btnCancel->Bounds().Width() - 7, btnCancel->Frame().top );
	btnSave->MoveTo( btnCancel->Frame().left - btnSave->Bounds().Width() - 7, btnSave->Frame().top );
}

