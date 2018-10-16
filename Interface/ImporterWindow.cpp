#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <stdlib.h>
#include <StringView.h>
#include "ImporterWindow.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "constants.h"
#include "Say.h"

const uint32 BEAIM_IMPORT_1 = 'bAi1';
const uint32 BEAIM_IMPORT_2 = 'bAi2';
const uint32 BEAIM_IMPORT_3 = 'bAi3';
const uint32 BEAIM_GOT_FILE_1 = 'bAf1';
const uint32 BEAIM_GOT_FILE_2 = 'bAf2';
const uint32 BEAIM_GOT_FILE_3 = 'bAf3';
const uint32 BEAIM_CLOSED_CONF = 'clC?';

//=====================================================

ImporterWindow::ImporterWindow( BRect frame )
	 				: SingleWindowBase(SW_BUDDYLIST_IMPORTER, frame, "Import Buddylist",
	 				  B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new ImporterView( aRect );
	AddChild( genView );

	// pointers
	panel1 = NULL;
	panel2 = NULL;
	panel3 = NULL;
	confWin = NULL;

	// window feel stuff
	SetFeel( B_MODAL_SUBSET_WINDOW_FEEL );
	AddToSubset( windows->GetSingleWindow(SW_BUDDYLIST_EDITOR) );

	// do the language stuff
	RefreshLangStrings();
}

//-----------------------------------------------------

void ImporterWindow::Show() {

	// first off, call the base class function
	BWindow::Show();

	// we're doing this here instead of the constructor, to prevent the half-second or so
	// "pause" before the window shows up; give the user some immediate visual response, even
	// if the window doesn't actually *work* right away. The pause is probably from the file
	// panels getting lists of stuff.
	if( !panel1 || !panel2 || !panel3 ) {

		// panel pointers
		panel1 = new BFilePanel( B_OPEN_PANEL, (BMessenger*)this, NULL, 0, false, new BMessage(BEAIM_GOT_FILE_1) );
		panel1->SetTarget(this);
		panel2 = new BFilePanel( B_OPEN_PANEL, (BMessenger*)this, NULL, 0, false, new BMessage(BEAIM_GOT_FILE_2) );
		panel2->SetTarget(this);
		panel3 = new BFilePanel( B_OPEN_PANEL, (BMessenger*)this, NULL, 0, false, new BMessage(BEAIM_GOT_FILE_3) );
		panel3->SetTarget(this);

		// beaim settings dir for #2
		entry_ref homeRef;
		if( GetSettingsRef(homeRef) ) {
			panel2->SetPanelDirectory(&homeRef);
		}
	}
}

//-----------------------------------------------------

bool ImporterWindow::GetSettingsRef( entry_ref& ref ) {

	BEntry entry;
	BDirectory dir;
	if( dir.SetTo("/boot/home/config/settings/BeAIM") != B_OK )
		return false;
	if( dir.GetEntry( &entry ) != B_OK )
		return false;
	if( entry.GetRef( &ref ) != B_OK )
		return false;
	return true;
}

//-----------------------------------------------------

ImporterWindow::~ImporterWindow() {

	delete panel1;
	delete panel2;
	delete panel3;
}

//-----------------------------------------------------

bool ImporterWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_BUDDYLIST_IMPORTER );
	PostAppMessage( clsMessage );
	return(true);
}

//-----------------------------------------------------

void ImporterWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case BEAIM_CLOSED_CONF:
			confWin = NULL;
			break;

		case BEAIM_GOT_FILE_1:
			Import1(message);
			break;

		case BEAIM_GOT_FILE_2:
			Import2(message);
			break;

		case BEAIM_GOT_FILE_3:
			Import3(message);
			break;

		case BEAIM_IMPORT_1:
			panel1->Show();
			break;

		case BEAIM_IMPORT_2:
			panel2->Show();
			break;

		case BEAIM_IMPORT_3:
			panel3->Show();
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

// the cancel function
void ImporterWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			return;
		}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

void ImporterWindow::Import1( BMessage* msg ) {

	entry_ref ref;
	status_t err;

	// grab the ref in the file
	if ((err = msg->FindRef("refs", &ref)) != B_OK)
		return;

	SetupConfView( ref, IMPORT_TYPE_BLT );
}

//-----------------------------------------------------

void ImporterWindow::Import2( BMessage* msg ) {

	entry_ref ref;
	status_t err;

	// grab the ref in the file
	if ((err = msg->FindRef("refs", &ref)) != B_OK)
		return;

	SetupConfView( ref, IMPORT_TYPE_BEAIM1X );
}

//-----------------------------------------------------

void ImporterWindow::Import3( BMessage* msg ) {

	entry_ref ref;
	status_t err;

	// grab the ref in the file
	if ((err = msg->FindRef("refs", &ref)) != B_OK)
		return;

	SetupConfView( ref, IMPORT_TYPE_GAIM );
}

//-----------------------------------------------------

void ImporterWindow::SetupConfView( entry_ref ref, int type ) {

	BPoint lt = Frame().LeftTop();
	BRect frame( 0, 0, 282, 178 );

	frame.OffsetTo( lt.x + 20, lt.y + 40 );

	if( confWin ) {
		confWin->Activate();
	}
	else {
		confWin = new ImporterConfirmWindow( frame, this, ref, type );
		confWin->Show();
	}
	genView->closeButton->MakeFocus(true);
}

//-----------------------------------------------------

void ImporterWindow::RefreshLangStrings() {

	BStringView* label;
	float biggestWidth;

	// set the title
	SetTitle( Language.get("IB_IMPORT") );

	// set the main label
	label = genView->mainImportLabel;
	label->SetText( LangWithSuffix("IB_IMPORT",":") );
	label->ResizeToPreferred();

	// do the other labels
	label = genView->bltLabel;
	label->SetText( Language.get("IB_BLT_LABEL") );
	label->ResizeToPreferred();
	label = genView->otherUserLabel;
	label->SetText( Language.get("IB_BEAIM_UF_LABEL") );
	label->ResizeToPreferred();
	label = genView->gaimLabel;
	label->SetText( Language.get("IB_GAIM_LABEL") );
	label->ResizeToPreferred();

	// fix the widths and stuff
	biggestWidth = genView->bltLabel->Bounds().Width();
	if( genView->otherUserLabel->Bounds().Width() > biggestWidth )
		biggestWidth = genView->otherUserLabel->Bounds().Width();
	if( genView->gaimLabel->Bounds().Width() > biggestWidth )
		biggestWidth = genView->gaimLabel->Bounds().Width();
	biggestWidth += 65;

	// set the button labels
	genView->importButton1->SetLabel( Language.get("BLE_IMPORT") );
	genView->importButton1->ResizeToPreferred();
	genView->importButton2->SetLabel( Language.get("BLE_IMPORT") );
	genView->importButton2->ResizeToPreferred();
	genView->importButton3->SetLabel( Language.get("BLE_IMPORT") );
	genView->importButton3->ResizeToPreferred();
	genView->closeButton->SetLabel( Language.get("CLOSE_LABEL") );
	genView->closeButton->ResizeToPreferred();

	// move the buttons
	genView->importButton1->MoveTo( biggestWidth, genView->importButton1->Frame().top );
	genView->importButton2->MoveTo( biggestWidth, genView->importButton2->Frame().top );
	genView->importButton3->MoveTo( biggestWidth, genView->importButton3->Frame().top );
	ResizeTo( genView->importButton1->Frame().right + 7, Bounds().Height() );
	genView->ResizeTo( genView->importButton3->Frame().right + 7, Bounds().Height() );
	genView->closeButton->MoveTo( Bounds().Width() - (genView->closeButton->Frame().Width() + 7),
								  genView->closeButton->Frame().top );

	// finally, move the divider thing
	genView->divider->ResizeTo( Bounds().Width() - 22, 2 );
}

//=====================================================

ImporterView::ImporterView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect sframe;
	int top = 25;

	SetViewColor( 216, 216, 216 );

	sframe = BRect( 10, 7, 220, 22 );
	mainImportLabel = new BStringView( sframe, "", "Import a buddylist:" );
	mainImportLabel->SetFont(be_bold_font);
	mainImportLabel->SetFontSize(11);
	AddChild( mainImportLabel );

	sframe = BRect( 20, top+8, 40, top+23 );
	bltLabel = new BStringView( sframe, "", "BLT Buddylist File" );
	bltLabel->SetFont(be_plain_font);
	bltLabel->SetFontSize(11);
	AddChild( bltLabel );

	sframe = BRect( 195, top, 260, top+24 );
	importButton1 = new BButton( sframe, "", "Import", new BMessage(BEAIM_IMPORT_1) );
	AddChild( importButton1 );

	top += 30;

	sframe = BRect( 20, top+8, 220, top+23 );
	otherUserLabel = new BStringView( sframe, "", "BeAIM 1.x User File" );
	otherUserLabel->SetFont(be_plain_font);
	otherUserLabel->SetFontSize(11);
	AddChild( otherUserLabel );

	sframe = BRect( 195, top, 260, top+24 );
	importButton2 = new BButton( sframe, "", "Import", new BMessage(BEAIM_IMPORT_2) );
	AddChild( importButton2 );

	top += 30;

	sframe = BRect( 20, top+8, 220, top+23 );
	gaimLabel = new BStringView( sframe, "", "GAIM Buddylist File" );
	gaimLabel->SetFont(be_plain_font);
	gaimLabel->SetFontSize(11);
	AddChild( gaimLabel );

	sframe = BRect( 195, top, 260, top+24 );
	importButton3 = new BButton( sframe, "", "Import", new BMessage(BEAIM_IMPORT_3) );
	AddChild( importButton3 );

	top += 38;

	// Make the groovy little line
	divider = new BBox(BRect(11,top,Bounds().right-11,top+2),"divider", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, B_FANCY_BORDER);
	AddChild( divider );

	top += 12;

	// close button
	sframe = BRect( 267, top, 332, top+24 );
	closeButton = new BButton( sframe, "", "Close", new BMessage(B_QUIT_REQUESTED) );
	closeButton->MakeDefault(true);
	AddChild( closeButton );
}

//=====================================================

ImporterConfirmWindow::ImporterConfirmWindow( BRect frame, BWindow* own, entry_ref r, int t )
	 				: BWindow( frame, Language.get("IB_IMPORT"), B_TITLED_WINDOW,
	 				  B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	SetFeel( B_MODAL_SUBSET_WINDOW_FEEL );
	AddToSubset( own );
	AddToSubset( windows->GetSingleWindow(SW_BUDDYLIST_EDITOR) );

	// Set up the view
	BRect aRect( Bounds() );
	genView = new ImporterConfirmView( aRect );
	ResizeTo( genView->Bounds().Width(), Bounds().Height() );
	AddChild( genView );
	owner = own;
	ref = r;
	thelist = NULL;
	type = t;

	// fill the list
	FillList();
}

//-----------------------------------------------------

ImporterConfirmWindow::~ImporterConfirmWindow() {

}

//-----------------------------------------------------

bool ImporterConfirmWindow::QuitRequested()
{
	NukeList(thelist);
	thelist = NULL;
	owner->PostMessage( new BMessage(BEAIM_CLOSED_CONF) );
	return(true);
}

//-----------------------------------------------------

void ImporterConfirmWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_OK:
			Save();
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

// does the actual importing
void ImporterConfirmWindow::Save()
{
	BString igroup;
	person* temp;
	while( thelist != NULL )
	{
		igroup = BString(thelist->name);
		users->AddGroup( igroup );
		temp = thelist->scrnname;
		while( temp != NULL )
		{
			users->AddBuddy( AIMUser(temp->name), igroup, false );
			temp = temp->next;
		}
		thelist = thelist->next;
	}
}

//-----------------------------------------------------

void ImporterConfirmWindow::FillList() {

	BString toAdd;
	bool first = true;
	group* thebackup;

	// get the actual list
	if( thelist ) {
		NukeList(thelist);
		thelist = NULL;
	}
	ImportAIMList( ref, thelist, type );
	thebackup = thelist;

	person* temp;
	while (thelist!=NULL)
	{
		if( !first )
			toAdd.Append( "\n\n" );
		else
			first = false;

		toAdd.Append( "[" );
		toAdd.Append( thelist->name );
		toAdd.Append( "]" );

		temp = thelist->scrnname;
		if( temp == NULL ) {
			toAdd.Append( "\n    (" );
			toAdd.Append( Language.get("EMPTY_LABEL") );
			toAdd.Append( ")" );
		}
		while (temp!=NULL)
		{
			toAdd.Append( "\n    " );
			toAdd.Append( temp->name );
			temp = temp->next;
		}
		thelist = thelist->next;
	}
	genView->textview->SetText( toAdd.String() );
	thelist = thebackup;
}

//-----------------------------------------------------

// the cancel function
void ImporterConfirmWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			return;
		}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//=====================================================

ImporterConfirmView::ImporterConfirmView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect sframe;
	BStringView* tempSV;
	BButton* importButton;
	BButton* closeButton;
	SetViewColor( 216, 216, 216 );

	sframe = BRect( 7, 7, 300, 22 );
	tempSV = new BStringView( sframe, "", Language.get("IB_WILL_BE_IMPORTED") );
	tempSV->SetFontSize(11);
	tempSV->SetFont(be_bold_font);
	tempSV->ResizeToPreferred();
	AddChild( tempSV );
	ResizeTo( tempSV->Frame().right + 30 + 7, Bounds().Height() );

	// Set up the view rectangles
	BRect textframe = BRect( 7, 25, Bounds().Width() - 7, 136 );
	textframe.right -= B_V_SCROLL_BAR_WIDTH;

	// make the new textview
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	textrect.InsetBy( 2.0, 2.0 );
	textview = new BTextView( textframe, "text_view", textrect, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE | B_NAVIGABLE_JUMP );


	// make the scrollview
	AddChild(scroll = new BScrollView("text_scroll_view", textview,
			B_FOLLOW_NONE, 0, false, true, B_PLAIN_BORDER));

	// set attributes
	textview->MakeEditable( false );
	BFont chatFont;
	rgb_color black;
	black.red = black.green = black.blue = 0;
	chatFont.SetSize( 11.0 );
	textview->SetFontAndColor( &chatFont, B_FONT_ALL, &black );

	// close button
	int top = 145;

	sframe = BRect( 210, top, 275, top+24 );
	closeButton = new BButton( sframe, "", Language.get("CLOSE_LABEL"), new BMessage(B_QUIT_REQUESTED) );
	closeButton->ResizeToPreferred();
	closeButton->MoveTo( Bounds().Width()-closeButton->Bounds().Width()-7, top );

	sframe = BRect( 135, top, 200, top+24 );
	importButton = new BButton( sframe, "", Language.get("BLE_IMPORT"), new BMessage(M_OK) );
	importButton->ResizeToPreferred();
	importButton->MoveTo( closeButton->Frame().left-importButton->Bounds().Width()-8, top );
	importButton->MakeDefault(true);

	AddChild( importButton );
	AddChild( closeButton );
}

//=====================================================
