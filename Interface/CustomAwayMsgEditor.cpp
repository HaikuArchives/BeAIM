#include <Application.h>
#include <Box.h>
#include <Button.h>
#include "StringView.h"
#include "CustomAwayMsgEditor.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "constants.h"

//=====================================================

CustomAwayMsgWindow::CustomAwayMsgWindow( BRect frame )
	 				: SingleWindowBase(SW_CUSTOM_AWAY_EDITOR, frame, "Custom Away Message",
	 				  B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new CustomAwayMsgView( aRect );
	AddChild( genView );

	// load the custom message
	BString customMessage = prefs->CustomAwayMessage();
	customMessage.ReplaceAll( "<br>", "\n" );
	genView->textview->SetText(customMessage.String());
	genView->textview->MakeFocus(true);

	// prefs
	enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
	tabIsTab = prefs->ReadBool( "TabIsTab", false );

	// language stuff
	RefreshLangStrings();
}

//-----------------------------------------------------

CustomAwayMsgWindow::~CustomAwayMsgWindow() {

}

//-----------------------------------------------------

bool CustomAwayMsgWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_CUSTOM_AWAY_EDITOR );
	PostAppMessage( clsMessage );
	return(true);
}

//-----------------------------------------------------

void CustomAwayMsgWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_OK:
			if( !Save() )
				break;

		case B_CANCEL:
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			break;

		case BEAIM_RELOAD_PREF_SETTINGS:
			enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
			tabIsTab = prefs->ReadBool( "TabIsTab", false );
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

void CustomAwayMsgWindow::RefreshLangStrings() {

	// do the title first
	SetTitle( Language.get("CUSTOM_AWAY_MSG") );

	// now do the label thingy
	genView->insLabel->SetText( Language.get("AME_ENTER_CUST_MSG") );

	// next, the buttons
	genView->btnSave->SetLabel( Language.get("SAVE_LABEL") );
	genView->btnCancel->SetLabel( Language.get("CANCEL_LABEL") );
	genView->btnSave->ResizeToPreferred();
	genView->btnCancel->ResizeToPreferred();

	// now move 'em
	genView->btnCancel->MoveTo( Bounds().Width() - genView->btnCancel->Bounds().Width() - 5,
								genView->btnCancel->Frame().top );
	genView->btnSave->MoveTo( genView->btnCancel->Frame().left - genView->btnSave->Bounds().Width() - 4,
								genView->btnSave->Frame().top );
}

//-----------------------------------------------------

bool CustomAwayMsgWindow::Save() {

	BString saveWhat = genView->textview->Text();

	// check to see if there is any text
	if( !saveWhat.Length() ) {
		windows->ShowMessage( Language.get("AME_ERR4") );
		return false;
	}

	// set the away message
	saveWhat.ReplaceAll( "\n", "<br>" );
	client->SetAwayMode( AM_CUSTOM_MESSAGE, saveWhat );
	return true;
}

//-----------------------------------------------------

// the cancel function
void CustomAwayMsgWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// the enter key does a bunch of stuff, depending on prefs...
	if( msg->what == B_KEY_DOWN ) {
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ENTER && genView->textview->IsFocus() ) {

			uint32 mods = modifiers();

			// shift-enter... depends on pref
			if( mods & B_SHIFT_KEY ) {
				if( !enterIsNewline ) {
					genView->textview->Insert("\n");
					genView->textview->ScrollToSelection();
					return;
				}
			}

			// plain old enter... depends on pref
			else {
				if( enterIsNewline ) {
					genView->textview->Insert("\n");
					genView->textview->ScrollToSelection();
					return;
				}
			}
		}

		// handle the almighty tab key
		else if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_TAB && genView->textview->IsFocus() ) {

			uint32 mods = modifiers();

			if( !tabIsTab ) {
				if( !(mods&B_COMMAND_KEY || mods&B_CONTROL_KEY || mods&B_MENU_KEY || mods&B_SHIFT_KEY) )
					msg->ReplaceInt32( "modifiers", mods | B_COMMAND_KEY );
			}

			else {
				if( mods & B_SHIFT_KEY )
					msg->ReplaceInt32( "modifiers", (mods | B_COMMAND_KEY) & ~B_SHIFT_KEY );
			}
		}

		else if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			uint32 mods = modifiers();
			if( mods == 0 || mods == 32) {
				PostMessage( new BMessage(B_QUIT_REQUESTED) );
				return;
			}
		}
	}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//=====================================================

CustomAwayMsgView::CustomAwayMsgView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );

	// make the stringview
	insLabel = new BStringView( BRect(7,8,250,20), "", "Enter your away message:" );
	insLabel->SetFont(be_bold_font);
	insLabel->SetFontSize(11);
	AddChild( insLabel );

	// Set up the view rectangles
	BRect textframe = BRect( 7, 23, 265, 136 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;

	// make the new textview
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	textrect.InsetBy( 2.0, 2.0 );
	textview = new HTMLView( true, textframe, "text_view", textrect, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );
	//textview = new BTextView( textframe, "text_view", textrect, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );


	// make the scrollview
	AddChild(scroll = new BScrollView("text_scroll_view", textview,
			B_FOLLOW_NONE, 0, false, true));

	// set attributes
	textview->MakeEditable( true );
	textview->SetStylable( false );
	//textview->SetStylable( true );
	textview->SetMaxBytes( 768 );
	BFont chatFont;
	rgb_color black;
	black.red = black.green = black.blue = 0;
	chatFont.SetSize( 12.0 );
	textview->SetFontAndColor( &chatFont, B_FONT_ALL, &black );
	textview->SelectAll();

	// make the new button
	BRect buttonrect = BRect( 156, 145, 207, 0 );
	btnSave = new BButton(buttonrect, "Save", "Save", new BMessage(M_OK),
					B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW );
	btnSave->MakeDefault(true);
	AddChild( btnSave );
	buttonrect = BRect( 216, 145, 267, 0 );
	btnCancel = new BButton(buttonrect, "Cancel", "Cancel", new BMessage(B_CANCEL),
					B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW);
	AddChild( btnCancel );
}

