#include <Application.h>
#include <Box.h>
#include <Button.h>
#include "StringView.h"
#include "ProfileEditor.h"
#include "DLanguageClass.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "Say.h"

//=====================================================

ProfileEditorWindow::ProfileEditorWindow( BRect frame )
	 				: SingleWindowBase(SW_PROFILE_EDITOR, frame, "Profile Editor",
	 				  B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new ProfileEditorView( aRect );
	AddChild( genView );
	
	// load the profile
	BString profile = prefs->Profile();
	profile.ReplaceAll( "<br>", "\n" );
	if( profile.Length() )
		genView->textview->SetText( profile.String() );
	genView->textview->MakeFocus(true);
	
	// prefs
	enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
	tabIsTab = prefs->ReadBool( "TabIsTab", false );
	
	// load the lang strings
	RefreshLangStrings();
}

//-----------------------------------------------------

ProfileEditorWindow::~ProfileEditorWindow() {

}

//-----------------------------------------------------

bool ProfileEditorWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_PROFILE_EDITOR );
	be_app->PostMessage( clsMessage );
	return(true);
}

//-----------------------------------------------------

void ProfileEditorWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_OK:
			Save();
	
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

void ProfileEditorWindow::Save() {

	BString profile = genView->textview->Text();
	profile.ReplaceAll( "\n", "<br>" );
	prefs->SetProfile(profile);
}

//-----------------------------------------------------

// the cancel function
void ProfileEditorWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

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

//-----------------------------------------------------

void ProfileEditorWindow::RefreshLangStrings() {

	float wholeWidth = Bounds().Width() - 5;

	// set the window title
	SetTitle( Language.get("EDIT_PROFILE") );
	
	// set the label
	genView->label->SetText( Language.get("PE_ENTER_PROFILE") );
	genView->label->ResizeToPreferred();
	
	// relabel and move the buttons
	genView->btnCancel->SetLabel( Language.get("CANCEL_LABEL") );
	genView->btnCancel->ResizeToPreferred();
	genView->btnCancel->MoveTo( wholeWidth - genView->btnCancel->Bounds().Width(), genView->btnCancel->Frame().top );
	wholeWidth -= (genView->btnCancel->Bounds().Width() + 5);
	genView->btnSave->SetLabel( Language.get("SAVE_LABEL") );
	genView->btnSave->ResizeToPreferred();
	genView->btnSave->MoveTo( wholeWidth - genView->btnSave->Bounds().Width(), genView->btnSave->Frame().top );
	
}

//=====================================================

ProfileEditorView::ProfileEditorView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	
	// make the stringview
	label = new BStringView( BRect(7,4,110,0), "", "Enter your profile:" );
	label->SetFont(be_bold_font);
	label->SetFontSize(11);
	AddChild( label );

	// Set up the view rectangles
	BRect textframe = BRect( 7, 23, 265, 136 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;

	// make the new textview
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	textrect.InsetBy( 2.0, 2.0 );	
	textview = new HTMLView( true, textframe, "text_view", textrect, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );
	textview->SetViewColor( GetBeAIMColor(BC_WHITE) );
	//textview = new BTextView( textframe, "text_view", textrect, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );
	
									 
	// make the scrollview
	AddChild(scroll = new BScrollView("text_scroll_view", textview,
			B_FOLLOW_NONE, 0, false, true));
				
	// set attributes
	textview->MakeEditable( true );
	//textview->SetStylable( false );
	textview->SetStylable( true );
	textview->SetMaxBytes( 768 );
	BFont chatFont;
	rgb_color black;
	black.red = black.green = black.blue = 0;
	chatFont.SetSize( 12.0 );
	textview->SetFontAndColor( &chatFont, B_FONT_ALL, &black );	
	textview->SelectAll();
	
	// make the new button
	BRect buttonrect = BRect( 156, 145, 207, 0 );
	btnSave = new BButton(buttonrect, "Save", "Save", new BMessage(B_OK),
					B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW );
	btnSave->MakeDefault(true);
	AddChild( btnSave );
	buttonrect = BRect( 216, 145, 267, 0 );
	btnCancel = new BButton(buttonrect, "Cancel", "Cancel", new BMessage(B_CANCEL),
					B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW);
	AddChild( btnCancel );
}

//-----------------------------------------------------

