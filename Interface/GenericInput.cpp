#include <Application.h>
#include <Box.h>
#include "GenericInput.h"
#include "DLanguageClass.h"
#include "Say.h"

//=========================================================================

GenericInputWindow::GenericInputWindow( BRect frame, char* title, char* name, BMessage* msg, BWindow* window,
										char* initial, int maxLen, char* fname, char* OK, char* Cancel )
	 				: BWindow(frame, "", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE ),
	 				  BInvoker( msg, window )
{
	BRect btnRect(62,70,122,90);
	unsigned int extra;
	
	// window feel stuff
	SetFeel( B_MODAL_SUBSET_WINDOW_FEEL );
	AddToSubset( window );
	
	// use the constructor parameters
	sndMessage = msg;
	SetTitle( title );
	strcpy( fieldName, fname );

	// Set up the view
	BRect aRect( Bounds() );
	genView = new GenericInputView( aRect );
	AddChild( genView );
	
	// set up the label
	label = new BStringView( BRect(10,7,222,23), "", name );
	label->SetFont(be_bold_font);
	label->SetFontSize(11);
	genView->AddChild( label );

	// set up the edit control
	personName = new BTextControl( BRect(8,25,222,45), "gen_in", "", "", NULL );
	personName->SetViewColor( 216, 216, 216 );
	personName->SetDivider( 0 );
	genView->AddChild( personName );
	if( initial )
		personName->SetText( initial );
	if( maxLen )
		personName->TextView()->SetMaxBytes( maxLen );
	
	// Now call the function that will let derived classes add extra controls if they want
	MakeExtraControls( genView, extra );
	if( extra ) {
		genView->ResizeBy( 0, extra );
		ResizeBy( 0, extra );
		btnRect.OffsetBy( 0, extra );
	}

	// Make the groovy little line
	BBox* divider = new BBox(BRect(11,55,222,57),"divider", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, B_FANCY_BORDER);	
	AddChild( divider );
	
	// adjust the OK/Cancel buttons for language independence
	BString okLabel = BString(OK);
	BString cancelLabel = BString(Cancel);
	if( okLabel == BString("--[iOK]--") )
		okLabel = Language.get("OK_LABEL");
	if( cancelLabel == BString("--[iCANCEL]--") )
		cancelLabel = Language.get("CANCEL_LABEL");
		
	// Make the Save and Cancel buttons
	SaveButton = new BButton( btnRect, "savebutton", okLabel.String(), new BMessage((unsigned long)B_OK));
	SaveButton->MakeDefault( true );
	btnRect.OffsetBy( 70, 0 );
	CancelButton = new BButton( btnRect, "cancelbutton", cancelLabel.String(), new BMessage(B_CANCEL) );
	
	
	// move the buttons and stuff
	float realWidth = Bounds().Width() - 6;
	SaveButton->ResizeToPreferred();
	CancelButton->ResizeToPreferred();
	CancelButton->MoveTo( realWidth-CancelButton->Bounds().Width(), CancelButton->Frame().top );
	realWidth -= (CancelButton->Bounds().Width() + 4);
	SaveButton->MoveTo( realWidth-SaveButton->Bounds().Width(), SaveButton->Frame().top );	
	
	genView->AddChild( SaveButton );
	genView->AddChild( CancelButton );
	
	// set focus to the text field
	personName->MakeFocus(true);
}

//-------------------------------------------------------------------------

GenericInputWindow::~GenericInputWindow() {

}

//-------------------------------------------------------------------------

void GenericInputWindow::MakeExtraControls( BView* view, unsigned int& extraHeight ) {
	extraHeight = 0;
}

//-------------------------------------------------------------------------

bool GenericInputWindow::QuitRequested()
{
	return(true);
}

//-------------------------------------------------------------------------

void GenericInputWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_OK:
			Save();	
			if(sndMessage) {
				Invoke(sndMessage);
			}
			
		case B_CANCEL:
			Close();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-------------------------------------------------------------------------

void GenericInputWindow::Save() {

	if( sndMessage ) {
		sndMessage->AddString( fieldName, personName->Text() );
	}
}

//-------------------------------------------------------------------------

// the cancel function
void GenericInputWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//=========================================================================

GenericInputView::GenericInputView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );
}
