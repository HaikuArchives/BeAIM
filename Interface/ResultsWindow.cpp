#include <Application.h>
#include <stdio.h>
#include <PopUpMenu.h>
#include <SupportKit.h>
#include "Say.h"
#include "ResultsWindow.h"
#include "PeopleEdit.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "constants.h"

//-----------------------------------------------------

SearchResultsWindow::SearchResultsWindow( BRect frame )
				: SingleWindowBase(SW_EMAIL_RESULTS, frame, "Search Results", B_TITLED_WINDOW,
								   B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	BRect r;

	// create and add the results view
	r = Bounds();
	r.bottom -= 15;
	resView = new ResultsView( r, "ResultsView" );
	AddChild(resView);
	empty = true;
	
	r = Bounds();
	r.top = r.bottom - 14;
	statView = new GStatusView( "" B_UTF8_ELLIPSIS, r );
	AddChild( statView );
	
	RefreshLangStrings();
}

//-----------------------------------------------------

bool SearchResultsWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_EMAIL_RESULTS );
	PostAppMessage( clsMessage );
	return(true);
}

//-----------------------------------------------------

void SearchResultsWindow::MessageReceived( BMessage* msg ) {

	switch( msg->what ) {

		case B_CANCEL:
		case BEAIM_CLOSE_WINDOW:
			QuitRequested();
			Close();
			break;
			
		// this is a new search
		case BEAIM_OPEN_EMAIL_SEARCH:
			SetupNewSearch( msg );
			break;
			
		// incoming search results
		case BEAIM_EMAIL_SEARCH_RESULTS:
			ParseIncomingResults( msg );
			break;
			
		case ENABLE_CONTEXT_BUTTONS: {
			bool enabled = msg->FindBool("enabled");
			resView->addToListButton->SetEnabled(enabled);
			resView->infoButton->SetEnabled(enabled);
			break;
		}
		
		case BEAIM_GET_USER_INFO:
			GetInfo();
			break;
			
		case BEAIM_ADD_BUDDY:
			AddPerson();
			break;

		// this is a bit confusing here... the open single window message starts a search.
		// maybe I'll clarify it later. Then again, considering my track record on clarifying
		// things later, probably not.
		case BEAIM_OPEN_SINGLE_WINDOW:
			SetupNewSearch( msg );
			break;
			
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}

//-----------------------------------------------------

// the cancel function
void SearchResultsWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

void SearchResultsWindow::ParseIncomingResults( BMessage* msg ) {

	BString dispMsg;
	char countMsg[10];

	// add a "fake" item saying that the search failed
	if( msg->HasBool("failed") && msg->FindBool("failed") == true ) {
		resView->lview->MakeEmpty();
		statView->SetSpinner(false);
		statView->SetMessage( (char*)Language.get("SR_NONE_FOUND") );
		return;
	}
	
	// if it's empty, clear it out
	if( empty ) {
		resView->lview->RemoveItems( 0, resView->lview->CountItems() );
		empty = false;
	}

	// add a new item
	BStringItem* item;
	item = new SearchItem( (char*)msg->FindString( "userid" ) );
	resView->lview->AddItem( item );
	
	// set the status view
	statView->SetSpinner(false);
	int32 count = resView->lview->CountItems();

	sprintf( countMsg, "%ld", count );
	dispMsg = BString( Language.get("SR_MATCHES_FOUND") );
	dispMsg.ReplaceAll( "%COUNT", countMsg );
	statView->SetMessage( (char*)dispMsg.String() );
}

//-----------------------------------------------------

void SearchResultsWindow::SetupNewSearch( BMessage* msg ) {
	
	char resTag[512];
	
	// set the new tag
	sprintf( resTag, "%s: %s", Language.get("SR_SEARCH_RESULTS"), (char*)msg->FindString("email") );
	resView->resLabel->SetText( resTag );
	
	// clear out the list
	resView->lview->MakeEmpty();
	empty = true;
	
	// fix the status bar
	statView->SetSpinner(true);
	statView->SetMessage( (char*)LangWithSuffix("SR_WAITING", B_UTF8_ELLIPSIS) );
	
	// send off the search request
	BMessage* searchMessage = new BMessage(BEAIM_SEARCH_BY_EMAIL);
	searchMessage->AddString( "email", (char*)msg->FindString("email") );
	PostAppMessage( searchMessage );
}

//-----------------------------------------------------

void SearchResultsWindow::GetInfo() {

	BMessage* sendMessage;
	BStringItem* item;
	
	// get the index of the item the user clicked on
	int32 selected = resView->lview->CurrentSelection();
	if( selected < 0 )
		return;
	item = dynamic_cast<BStringItem*>( resView->lview->ItemAt(selected) );

	// make a message to do the dirty work, and send it off
	sendMessage = new BMessage(BEAIM_OPEN_USER_WINDOW);
	sendMessage->AddString( "userid", item->Text() );
	sendMessage->AddInt32( "wtype", (int32)USER_INFO_TYPE );
	sendMessage->AddPointer( "poswindow", this );
	PostAppMessage( sendMessage );
}

//-----------------------------------------------------

void SearchResultsWindow::AddPerson() {

	BStringItem* item;
	AIMUser user;

	// get the index of the item the user clicked on
	int32 selected = resView->lview->CurrentSelection();
	if( selected < 0 )
		return;
	item = dynamic_cast<BStringItem*>( resView->lview->ItemAt(selected) );
	user = AIMUser( (char*)item->Text() );

	// open the "add buddy" window
	windows->MakeAddBuddyWindow( user, this, true );
}

//-----------------------------------------------------

void SearchResultsWindow::RefreshLangStrings() {

	BButton* button;

	// set the title
	SetTitle( Language.get("SR_SEARCH_RESULTS") );

	// do the buttons
	button = resView->closeButton;
	button->SetLabel( Language.get("CLOSE_LABEL") );
	button->ResizeToPreferred();
	button->MoveTo( Bounds().Width() - (button->Frame().Width() + 7), button->Frame().top );
	button = resView->addToListButton;
	button->SetLabel( Language.get("ADD_BLIST_LABEL") );
	button->ResizeToPreferred();
	button = resView->infoButton;
	button->SetLabel( Language.get("CW_GET_INFO") );
	button->ResizeToPreferred();
	button->MoveTo( resView->addToListButton->Frame().right + 4, button->Frame().top );
}

//=====================================================

ResultsView::ResultsView(BRect rect, const char *name)
	   	   : BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );	
	
	// make the columnlistview
	BRect other_rect = Bounds();
	other_rect.InsetBy( 9, 9 );
	other_rect.top += 20;
	other_rect.bottom -= 35;
	other_rect.right -= B_V_SCROLL_BAR_WIDTH;
	lview = new ResultsListView( other_rect );
	lview->SetFontSize(12);
	container = new BScrollView( "scroller", lview, B_FOLLOW_ALL, 0, false, true );
	
	// make the search results label
	BRect boxRect = Bounds();
	boxRect.InsetBy( 7, 3 );
	boxRect.bottom = boxRect.top + 20;
	resLabel = new BStringView( boxRect, "resLabel", "Search Results:" );
	resLabel->SetFont(be_bold_font);
	resLabel->SetFontSize(12);
	
	// make some buttons
	closeButton = new BButton( BRect(252,138,312,158), "close", "Close", new BMessage(BEAIM_CLOSE_WINDOW) );
	closeButton->MakeDefault( true );
	addToListButton = new BButton( BRect(7,138,120,158), "close", "Add to Buddy List", new BMessage(BEAIM_ADD_BUDDY) );
	addToListButton->SetEnabled( false ); 
	infoButton = new BButton( BRect(126,138,190,158), "close", "Get Info", new BMessage(BEAIM_GET_USER_INFO) );
	infoButton->SetEnabled( false );

	// add the views
	AddChild( resLabel );
	AddChild( container );
	AddChild( addToListButton );
	AddChild( infoButton );
	AddChild( closeButton );
}

//=====================================================

void SearchItem::DrawItem( BView *owner, BRect frame, bool complete ) {

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
	
	// if it's a fake node, make it italic
	if( fake )
		ownerFont.SetFace( B_ITALIC_FACE );
	
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

//=====================================================

ResultsListView::ResultsListView( BRect frame )
		        : BListView( frame, "resultslistview" )
{
	userPopup = new BPopUpMenu( "userpopup", false );
	userPopup->AddItem( new BMenuItem( LangWithSuffix("ADD_BLIST_LABEL", B_UTF8_ELLIPSIS), NULL ) );
	userPopup->AddItem( new BMenuItem( LangWithSuffix("CW_GET_INFO", B_UTF8_ELLIPSIS), NULL ) );
}

//-----------------------------------------------------

ResultsListView::~ResultsListView() {
	delete userPopup;
}

//-----------------------------------------------------

void ResultsListView::MouseDown( BPoint cursor ) {

	uint32 buttons;
	int32 clickIndex;
	SearchItem* clickItem;
	BMenuItem* selected;
	int32 option;

	// get the mouse position
	GetMouse( &cursor, &buttons );
		
	// it was a right mouse button click, popup the menu
	if( buttons & B_SECONDARY_MOUSE_BUTTON ) {
		
		// get the index of the item the user clicked on, and select it
		clickIndex = IndexOf(cursor);
		if( clickIndex < 0 )			// not an item?
			return;
		Select(clickIndex);
		clickItem = (SearchItem*)ItemAt(clickIndex);

		// it's all good... display the popup menu
		ConvertToScreen( &cursor );
		selected = userPopup->Go( cursor );

		// if the user selected something, take action based on the choice
		if( selected ) {
			option = userPopup->IndexOf(selected);
			switch( option ) {
				case 0:
					Window()->PostMessage( new BMessage(BEAIM_ADD_BUDDY) );
					break;
				case 1:
					Window()->PostMessage( new BMessage(BEAIM_GET_USER_INFO) );
					break;				
			}
		}
	} else
		BListView::MouseDown( cursor );
}

//-----------------------------------------------------

void ResultsListView::SelectionChanged() {

	// Based on whether something is selected, send a message to disable the buttons
	int32 selected = CurrentSelection();
	BMessage* sndMessage = new BMessage(ENABLE_CONTEXT_BUTTONS);
	sndMessage->AddBool( "enabled", selected < 0 ? false : true );
	Window()->PostMessage( sndMessage );
	delete sndMessage;	
}

//-----------------------------------------------------
