#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <StringView.h>
#include "AwayEditor.h"
#include "Globals.h"
#include "MiscStuff.h"
#include "Say.h"

const uint32 AWAY_NEW = 'anEw';
const uint32 AWAY_DELETE = 'adEl';
const uint32 AWAY_SAVE = 'asAv';
const uint32 AWAY_SELECTED = 'asEl';
const uint32 AWAY_CHANGED = 'acHg';

//=====================================================

AwayEditorWindow::AwayEditorWindow( BRect frame )
	 				: SingleWindowBase(SW_AWAY_EDITOR, frame, "Away Message Editor",
	 				  B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new AwayEditorView( aRect );
	AddChild( genView );
	BuildList();
	
	// vars
	hasSelection = false;
	EnableMessageStuff(false);
	
	// prefs
	enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
	tabIsTab = prefs->ReadBool( "TabIsTab", false );	
	
	// load all the right language stuff
	RefreshLangStrings();
}

//-----------------------------------------------------

AwayEditorWindow::~AwayEditorWindow() {

}

//-----------------------------------------------------

bool AwayEditorWindow::QuitRequested()
{
	if( !CheckMessage() )
		return false;

	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_AWAY_EDITOR );
	PostAppMessage( clsMessage );
	
	return(true);
}

//-----------------------------------------------------

void AwayEditorWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	
		case AWAY_NEW:
			NewMessage();
			break;
			
		case AWAY_SAVE:
			SaveMessage();
			break;
			
		case AWAY_DELETE:
			DeleteMessage();
			break;
			
		case AWAY_CHANGED:
			isDirty = true;
			break;

		case BEAIM_RELOAD_PREF_SETTINGS:
			enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
			tabIsTab = prefs->ReadBool( "TabIsTab", false );
			break;

		case AWAY_SELECTED: {
			AwayItem* item = (AwayItem*)genView->listview->ItemAt( genView->listview->CurrentSelection() );
			if( !item || !CheckMessage() ) {
				EnableMessageStuff(false);
				return;
			}
			LoadMessage( item->GetName() );
			break;
		}
		
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

// check and see if the message needs saving, and whether to ditch it or not
bool AwayEditorWindow::CheckMessage() {

	if( !hasSelection )
		return true;

	BAlert* alert;
	if( isDirty || genView->textview->IsDirty() ) {
	
		alert = new BAlert("", Language.get("AME_SAVE_CHANGES"), Language.get("CANCEL_LABEL"),
							   Language.get("IGNORE_LABEL"), Language.get("SAVE_LABEL"),
							   B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut( 0, B_ESCAPE );
		int32 index = alert->Go();

		if( index == 2 )
			if( SaveMessage() )
				return true;
			else return false;
			
		if( index == 1 ) {
			if( currentMsg == BString(Language.get("AME_NEW_MESSAGE")) )
				DeleteMessage();
		}
		
		if( index == 0 ) {
			Revert();
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------

void AwayEditorWindow::Revert() {
	genView->listview->SetSelectionMessage( NULL );
	genView->listview->Select( prefs->FindAwayMessage(currentMsg) );
	genView->listview->SetSelectionMessage( new BMessage(AWAY_SELECTED) );
}

//-----------------------------------------------------

void AwayEditorWindow::NewMessage() {

	if( !CheckMessage() )
		return;
	
	currentMsg = BString(Language.get("AME_NEW_MESSAGE"));
	prefs->SetAwayMessage( currentMsg, "", "" );
	
	EnableMessageStuff( true );
	genView->messageName->SetText("");
	genView->textview->SetText("");
	genView->messageName->MakeFocus(true);
	genView->listview->AddItem( new AwayItem(currentMsg) );
	genView->listview->SetSelectionMessage( NULL );
	genView->listview->Select( prefs->FindAwayMessage(currentMsg) );
	genView->listview->SetSelectionMessage( new BMessage(AWAY_SELECTED) );
	genView->textview->MakeDirty(false);
	isDirty = false;
	hasSelection = true;
}

//-----------------------------------------------------

bool AwayEditorWindow::SaveMessage() {

	if( client->AwayMode() == AM_STANDARD_MESSAGE ) {
		if( client->CurrentAwayMessageName() == currentMsg ) {
			windows->ShowMessage( Language.get("AME_ERR1"), B_INFO_ALERT );
			return false;
		}
	}

	BString text = genView->messageName->Text();
	if( text == BString(Language.get("AME_NEW_MESSAGE")) || text == BString("") ) {
		windows->ShowMessage( Language.get("AME_ERR2"), B_WARNING_ALERT );
		Revert();
		genView->messageName->MakeFocus(true);
		return false;
	}

	if( (text != currentMsg) && (prefs->FindAwayMessage(text) != -1) ) {
		windows->ShowMessage( Language.get("AME_ERR3"), B_WARNING_ALERT );
		Revert();
		genView->messageName->MakeFocus(true);
		return false;	
	}

	int32 selected = genView->listview->CurrentSelection();
	BString message = genView->textview->Text();
	message.ReplaceAll( "\n", "<br>" );
	prefs->SetAwayMessage( genView->messageName->Text(), currentMsg, message );
	BuildList();
	
	isDirty = false;
	genView->textview->MakeDirty(false);
	genView->listview->Select( selected );
	PostAppMessage( new BMessage(BEAIM_LOAD_AWAY_MENU) );
	return true;
}

//-----------------------------------------------------

void AwayEditorWindow::LoadMessage( BString name ) {
		
	BString message;
	if( !(prefs->GetAwayMessage(name, message)) )
		return;

	// correct the line breaks and stuff
	message.ReplaceAll( "<br>", "\n" );

	// set the textviews and stuff
	genView->messageName->SetModificationMessage( NULL );
	genView->messageName->SetText( name.String() );
	genView->messageName->SetModificationMessage( new BMessage(AWAY_CHANGED) );
	genView->textview->SetText( message.String() );
	EnableMessageStuff( true );
	hasSelection = true;
	currentMsg = name;
}

//-----------------------------------------------------

void AwayEditorWindow::EnableMessageStuff( bool enabled ) {

	if( enabled ) {
		genView->btnSave->SetEnabled( true );
		genView->btnDelete->SetEnabled( true );
		genView->messageName->SetEnabled( true );
		genView->textview->MakeEditable( true );
		genView->textview->SetViewColor( 255, 255, 255 );
		genView->textview->Invalidate();
	} else {
		genView->btnSave->SetEnabled( false );
		genView->btnDelete->SetEnabled( false );
		genView->messageName->SetEnabled( false );
		genView->textview->MakeEditable( false );
		genView->textview->SetViewColor( 239, 239, 239 );
		genView->textview->Invalidate();
		genView->textview->SetText( "" );
		genView->messageName->SetModificationMessage(NULL);
		genView->messageName->SetText( "" );
		genView->messageName->SetModificationMessage(new BMessage(AWAY_CHANGED));
		genView->textview->MakeFocus(false);
		genView->messageName->MakeFocus(false);
	}
	isDirty = false;
}

//-----------------------------------------------------

void AwayEditorWindow::RefreshLangStrings() {

	// do the title first
	SetTitle( Language.get("EDIT_AWAY_MSGS") );
	
	// set the labels on the controls
	genView->messagesLabel->SetText( Language.get("AME_MESSAGES") );
	genView->messageName->SetLabel( LangWithSuffix("AME_MESSAGE_NAME", ":") );
	genView->messageBox->SetLabel( Language.get("AME_MESSAGE_INFO") );
	
	// ... and the buttons
	genView->btnNew->SetLabel( Language.get("NEW_LABEL") );
	genView->btnNew->ResizeToPreferred();
	genView->btnSave->SetLabel( Language.get("SAVE_LABEL") );
	genView->btnSave->ResizeToPreferred();
	genView->btnDelete->SetLabel( Language.get("REMOVE_LABEL") );
	genView->btnDelete->ResizeToPreferred();
	
	// ... and move them
	genView->btnSave->MoveTo( genView->btnNew->Frame().right + 30, genView->btnSave->Frame().top );
	genView->btnDelete->MoveTo( genView->btnSave->Frame().right + 2, genView->btnDelete->Frame().top );
	
		// finally, resize everything to match the larger (too large, IMHO) buttons
	genView->messageBox->ResizeTo( genView->btnDelete->Frame().right + 7, genView->messageBox->Frame().Height() );
	genView->scroll->ResizeTo( genView->btnDelete->Frame().right - genView->scroll->Frame().left - 1,
								 genView->textview->Frame().Height() );
	genView->messageName->ResizeTo( genView->btnDelete->Frame().right - genView->messageName->Frame().left - 1,
								 genView->messageName->Frame().Height() );
	ResizeTo( genView->messageBox->Frame().right + 7, Bounds().Height() );
}

//-----------------------------------------------------

// the cancel function
void AwayEditorWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// the enter key does a bunch of stuff, depending on prefs...
	if( msg->what == B_KEY_DOWN && msg->HasString("bytes") ) {
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

void AwayEditorWindow::BuildList() {

	// clear out the list
	genView->listview->MakeEmpty();
	
	// populate the list
	awayStruct aw;
	bool kg = prefs->GetFirstAwayMessage(aw);
	while( kg ) {
		genView->listview->AddItem( new AwayItem(aw.name) );
		kg = prefs->GetNextAwayMessage(aw);
	}
}

//-----------------------------------------------------

void AwayEditorWindow::DeleteMessage() {

	if( client->AwayMode() == AM_STANDARD_MESSAGE ) {
		if( client->CurrentAwayMessageName() == currentMsg ) {
			windows->ShowMessage( Language.get("AME_ERR1"), B_INFO_ALERT );
			return;
		}
	}

	prefs->RemoveAwayMessage( currentMsg );
	currentMsg = "";
	hasSelection = false;
	isDirty = false;
	genView->textview->MakeDirty(false);
	BuildList();
	EnableMessageStuff(false);
	PostAppMessage( new BMessage(BEAIM_LOAD_AWAY_MENU) );
}

//=====================================================

AwayEditorView::AwayEditorView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );
			
	// make the stringview
	messagesLabel = new BStringView( BRect(7,8,110,20), "", "Messages" );
	messagesLabel->SetFont(be_bold_font);
	messagesLabel->SetFontSize(11);
	AddChild( messagesLabel );

	// make the new listview
	BRect listRect = BRect( 8, 22, 110, 192 );	
	listview = new BListView( listRect, "list_view" );
	listview->SetSelectionMessage( new BMessage(AWAY_SELECTED) );
	listview->SetFontSize(11);
										 
	// make the scrollview
	AddChild(scroll2 = new BScrollView("list_scroll_view", listview,
			B_FOLLOW_NONE, 0, false, true));

	// put in the groupbox
	messageBox = new BBox( BRect(135,7,382,193) );
	messageBox->SetLabel("Message Info");
	messageBox->SetFontSize(11);
	AddChild( messageBox );

	// make the messageName text control
	messageName = new BTextControl( BRect(8,20,235,40), "", "Name:", "", NULL );
	messageName->SetViewColor( 216, 216, 216 );
	messageName->SetDivider( 60 );
	messageName->SetFontSize( 11 );
	messageName->SetModificationMessage( new BMessage(AWAY_CHANGED) );
	messageName->TextView()->SetMaxBytes(30);	
	messageBox->AddChild( messageName );
	
	// Set up the view rectangles
	BRect textframe = BRect( 10, 48, 235, 150 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;

	// make the new textview
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	textrect.InsetBy( 2.0, 2.0 );	
	textview = new AwayTextView( textframe, "text_view", textrect,
								 B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );
										 
	// make the scrollview
	messageBox->AddChild(scroll = new BScrollView("text_scroll_view", textview,
			B_FOLLOW_NONE, 0, false, true, B_PLAIN_BORDER));
				
	// set attributes
	textview->MakeEditable( true );
	textview->SetStylable( false );
	textview->SetMaxBytes( 768 );
	BFont chatFont;
	rgb_color black;
	black.red = black.green = black.blue = 0;
	chatFont.SetSize( 12.0 );
	textview->SetFontAndColor( &chatFont, B_FONT_ALL, &black );	
	
	// make the new buttons
	BRect buttonrect( 8, 155, 59, 0 );
	btnNew = new BButton(buttonrect, "New", "New", new BMessage(AWAY_NEW), B_FOLLOW_NONE, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW);
	messageBox->AddChild( btnNew );
	buttonrect = BRect( 129, 155, 180, 0 );
	btnSave = new BButton(buttonrect, "Save", "Save", new BMessage(AWAY_SAVE), B_FOLLOW_NONE, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW);
	messageBox->AddChild( btnSave );
	buttonrect = BRect( 186, 155, 237, 0 );
	btnDelete = new BButton(buttonrect, "Delete", "Delete", new BMessage(AWAY_DELETE), B_FOLLOW_NONE, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW);
	messageBox->AddChild( btnDelete );
}

//=====================================================

AwayTextView::AwayTextView( BRect frame, const char* name, BRect textRect,
						    uint32 resizingMode, uint32 flags )
						    : BTextView( frame, name, textRect, resizingMode, flags )
{
	isDirty = false;
}

//-----------------------------------------------------

void AwayTextView::InsertText( const char* text, int32 length, int32 offset,
							   const text_run_array *runs )
{
	isDirty = true;
	BTextView::InsertText( text, length, offset, runs );
}

void AwayTextView::DeleteText(int32 start, int32 finish) {
	isDirty = true;
	BTextView::DeleteText( start, finish );
}

//-----------------------------------------------------

void AwayTextView::MakeDirty( bool dirty ) {
	isDirty = dirty;
}

//-----------------------------------------------------

void AwayTextView::SetText( const char* text ) {
	BTextView::SetText(text);
	isDirty = false;
}

//=====================================================

AwayItem::AwayItem( BString n ) {
	name = n;
}

//-----------------------------------------------------

BString AwayItem::GetName() {
	return name;
}

//-----------------------------------------------------

void AwayItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight; 
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;
	
	// Grab the owner's font
	BFont baseFont;
	owner->GetFont( &baseFont );
	BFont ownerFont( baseFont );
	
	// does the background need to be redrawn?
	if( IsSelected() || complete ) {
		
		// pick the appropriate background color
        if( IsSelected() ) {
        	color = kHighlight;
        	owner->SetLowColor( 222, 219, 222 );
        } else {
			color = owner->ViewColor();
			owner->SetLowColor( 255, 255, 255 );
		}
	
		// draw the background		
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}

	// set the font and draw the string
	owner->SetHighColor(0,0,0);
	owner->SetFont( &ownerFont );
	owner->MovePenTo(frame.left + 4, frame.bottom - 2);
	owner->DrawString( name.LockBuffer(35) ); 
	name.UnlockBuffer();
	
	// reset the font and colors
	owner->SetFont( &baseFont );
	owner->SetLowColor( 255, 255, 255 );
}

//-----------------------------------------------------

void AwayItem::SetName( BString n ) {
	name = n;
}

//-----------------------------------------------------
