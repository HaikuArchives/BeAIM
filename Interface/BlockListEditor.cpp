#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <stdlib.h>
#include "StringView.h"
#include "BlockListEditor.h"
#include "constants.h"
#include "MiscStuff.h"
#include "UserManager.h"
#include "DLanguageClass.h"
#include "Globals.h"

const uint32 LIST_SEL_MSG = '##%s';
const uint32 BL_REMOVE_ITEM = 'blRI';
const uint32 BL_ADD_ITEM = 'blAI';
const uint32 BL_ADD_BLOCK = 'blAB';

//=========================================================================

BlockListEditorWindow::BlockListEditorWindow( BRect frame )
	 				: SingleWindowBase(SW_BLOCKLIST_EDITOR, frame, "Block List", B_TITLED_WINDOW,
	 				  B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new BlockListEditorView( aRect );
	AddChild( genView );

	// use all the right language strings and stuff
	RefreshLangStrings();

	// fill the list
	DoLoad();
}

//-------------------------------------------------------------------------

BlockListEditorWindow::~BlockListEditorWindow() {

}

//-------------------------------------------------------------------------

bool BlockListEditorWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_BLOCKLIST_EDITOR );
	PostAppMessage( clsMessage );
	return true;
}

//-------------------------------------------------------------------------

void BlockListEditorWindow::RefreshLangStrings()
{
	SetTitle( Language.get("EDIT_BLOCK_LIST") );
	
	// set the strings for all the buttons
	genView->addButton->SetLabel( LangWithSuffix("ADD_LABEL", B_UTF8_ELLIPSIS) );
	genView->addButton->ResizeToPreferred();
	genView->removeButton->SetLabel( Language.get("REMOVE_LABEL") );
	genView->removeButton->ResizeToPreferred();
	genView->closeButton->SetLabel( Language.get("CLOSE_LABEL") );
	genView->closeButton->ResizeToPreferred();
	
	// now make them all the size of the largest button
	float largest = genView->addButton->Bounds().Width();
	if( genView->removeButton->Bounds().Width() > largest )
		largest = genView->removeButton->Bounds().Width();
	if( genView->closeButton->Bounds().Width() > largest )
		largest = genView->closeButton->Bounds().Width();
	genView->addButton->ResizeTo( largest, genView->addButton->Bounds().Height() );
	genView->removeButton->ResizeTo( largest, genView->removeButton->Bounds().Height() );
	genView->closeButton->ResizeTo( largest+4, genView->closeButton->Bounds().Height() );
	
	// make the window the right size as well
	genView->ResizeTo( 211 + largest, Bounds().Height() );
	ResizeTo( genView->Bounds().Width(), Bounds().Height() );
}

//-------------------------------------------------------------------------

void BlockListEditorWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case BL_REMOVE_ITEM:
			DoRemove();
			break;
			
		case BL_ADD_ITEM:
			DoAdd();
			break;
	
		case LIST_SEL_MSG: {
			bool hasSel = bool(genView->list->CurrentSelection() != -1);
			if( genView->removeButton->IsEnabled() != hasSel )
				genView->removeButton->SetEnabled(hasSel);
			break;
		}
		
		case BEAIM_SET_USER_BLOCKINESS:
			HandleExternalBlockiness( message );
			break;
		
		case BL_ADD_BLOCK:
			DoRealAdd( message );
			break;
	
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;
	
		default:
			BWindow::MessageReceived(message);
	}
}

//-------------------------------------------------------------------------

// the cancel function
void BlockListEditorWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

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

void BlockListEditorWindow::DoRemove() {

	BString userName;
	int32 sel = genView->list->CurrentSelection();
	
	if( sel < 0 )
		return;

	BStringItem* item = (BStringItem*)genView->list->ItemAt(sel);
	genView->list->RemoveItem(sel);
	userName = BString(item->Text());

	users->UnblockUser( userName );
	
	BString message;
	message = BString( Language.get("BLE_GOT_UNBLOCKED") );
	message.ReplaceAll( "%USER", userName.String() );
	windows->ShowMessage( (char*)message.String() );
}

//-------------------------------------------------------------------------

void BlockListEditorWindow::DoAdd() {

	BMessage* sendMessage;
	sendMessage = new BMessage(BL_ADD_BLOCK);
	windows->OpenInputWindow( (char*)Language.get("BLK_BLOCK_USER"),
							  (char*)LangWithSuffix("SCREEN_NAME_LABEL", ":"),
							  sendMessage, this, 
							  NULL, DISPLAY_NAME_MAX, false, "userid" );
}

//-------------------------------------------------------------------------

// this function updates the list if the block list is modified externally (ie, from a chat window)
void BlockListEditorWindow::HandleExternalBlockiness( BMessage* msg ) {

	Lock();
	int32 index = -1;
	AIMUser user = AIMUser(msg->FindString("userid"));
	
	// see if the item is in the list
	BStringItem* item;
	for( int32 i = 0; i < genView->list->CountItems(); ++i ) {
		item = (BStringItem*)genView->list->ItemAt(i);
		if( AIMUser((char*)item->Text()) == user )
			index = i;
	}
	Unlock();
	
	if( msg->FindBool("block") ) {
		if( index != -1 )
			return;
		genView->list->AddItem( new BlockItem(user) );
	}

	else {
		if( index == -1 )
			return;	
		genView->list->RemoveItem( index );
	}
}

//-------------------------------------------------------------------------

void BlockListEditorWindow::DoRealAdd( BMessage* msg ) {

	BString toAdd = BString(msg->FindString("userid") );
	if( users->IsUserBlocked(toAdd) )
		return;
		
	genView->list->AddItem( new BlockItem(AIMUser(toAdd)) );
	users->BlockUser( toAdd );
	
	BString message;
	message = BString( Language.get("BLE_GOT_BLOCKED") );
	message.ReplaceAll( "%USER", toAdd.String() );
	windows->ShowMessage( (char*)message.String() );	
}

//-------------------------------------------------------------------------

void BlockListEditorWindow::DoLoad() {
	AIMUser use;
	bool kg;

	kg = users->GetNextBlockedUser(use, true);
	while( kg ) {
		genView->list->AddItem( new BlockItem(use) );
		kg = users->GetNextBlockedUser(use, false);
	}
}

//=========================================================================

BlockListEditorView::BlockListEditorView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_NONE, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );

	// Set up the view rectangles
	BRect textframe = BRect( 7, 7, 197, 120 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;

	// make the new listview
	list = new BListView( textframe, "list_view" );
	list->SetSelectionMessage( new BMessage(LIST_SEL_MSG) );
	list->SetFontSize(12.0);
	AddChild(scroll = new BScrollView("text_scroll_view", list,
			B_FOLLOW_NONE, 0, false, true));

	// make the new button
	BRect buttonrect = BRect( 207, 6, 270, 0 );
	addButton = new BButton(buttonrect, "add", "Add" B_UTF8_ELLIPSIS, new BMessage(BL_ADD_ITEM), B_FOLLOW_NONE, B_NAVIGABLE | B_NAVIGABLE_JUMP | B_WILL_DRAW );
	AddChild( addButton );
	buttonrect = BRect( 207, 34, 270, 0 );
	removeButton = new BButton(buttonrect, "remove", "Remove", new BMessage(BL_REMOVE_ITEM), B_FOLLOW_NONE, B_NAVIGABLE | B_WILL_DRAW);
	removeButton->SetEnabled(false);
	AddChild( removeButton  );
	buttonrect = BRect( 207, 98, 270, 0 );
	closeButton = new BButton(buttonrect, "close", "Close", new BMessage(B_QUIT_REQUESTED), B_FOLLOW_NONE, B_NAVIGABLE | B_WILL_DRAW);
	closeButton->MakeDefault(true);
	AddChild( closeButton );
}


//=========================================================================

void BlockItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight; 
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;
	
	// Grab the owner's font, to be fiddled with if needed
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
	owner->DrawString( Text() );

	// reset the font and colors
	owner->SetFont( &baseFont );
	owner->SetLowColor( 255, 255, 255 );
	owner->SetHighColor( 0, 0, 0 );
}


