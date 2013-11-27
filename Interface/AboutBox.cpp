#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <stdlib.h>
#include <StringView.h>
#include <Roster.h>
#include <Box.h>
#include <ctype.h>
#include <Button.h>
#include "AboutBox.h"
#include "MiscStuff.h"
#include "constants.h"
#include "BitmapView.h"
#include "Globals.h"
#include "DLanguageClass.h"
#include "Say.h"

//=====================================================

AboutWindow::AboutWindow( BRect frame )
	 				: SingleWindowBase( SW_ABOUT_BOX, frame, "About BeAIM", 
	 				  B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new AboutView( aRect );
	AddChild( genView );
	
	// refresh the language strings
	RefreshLangStrings();
}

//-----------------------------------------------------

AboutWindow::~AboutWindow() {

}

//-----------------------------------------------------

void AboutWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

// the cancel function
void AboutWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN ) {

		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			return;
		}
	}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

void AboutWindow::RefreshLangStrings() {

	float wholeWidth = Bounds().Width() - 5;
	
	// make the translator name thang
	BString translatorLabel;
	translatorLabel = BString( Language.Name() );
	translatorLabel.Append( " - " );
	translatorLabel.Append( Language.get("AB_TRANSLATED_BY_LABEL") );

	// set the window title
	SetTitle( Language.get("ABOUT_BEAIM") );
	
	// set the other labels in the box
	dynamic_cast<BStringView*>(genView->ChildAt(4))->SetText( Language.get("AB_MAJOR_CONTRIBS") );
	dynamic_cast<BStringView*>(genView->ChildAt(4))->ResizeToPreferred();
	dynamic_cast<BStringView*>(genView->ChildAt(6))->SetText( Language.get("AB_HOME_PAGE") );
	dynamic_cast<BStringView*>(genView->ChildAt(6))->ResizeToPreferred();
	dynamic_cast<BStringView*>(genView->ChildAt(7))->SetText( Language.get("AB_BEBITS_PAGE") );
	dynamic_cast<BStringView*>(genView->ChildAt(7))->ResizeToPreferred();

	dynamic_cast<BStringView*>(genView->ChildAt(12))->SetText( translatorLabel.String() );
	dynamic_cast<BStringView*>(genView->ChildAt(12))->ResizeToPreferred();
	dynamic_cast<BStringView*>(genView->ChildAt(13))->SetText( Language.get("TRANSLATED_BY") );
	dynamic_cast<BStringView*>(genView->ChildAt(13))->ResizeToPreferred();
	genView->verSV->SetText( BeAIMVersion().String() );
	
	// if the language isn't english, give credit to the translator
	if( BString(Language.Name()) == "English" ) {
		ResizeTo( abRegRect.Width(), abRegRect.Height() );
		genView->ChildAt(6)->MoveTo( 7, 175 );
		genView->ChildAt(7)->MoveTo( 7, 197 );
		genView->ChildAt(8)->MoveTo( 7, 219 );
		genView->ChildAt(9)->MoveTo( 145, 175 );
		genView->ChildAt(10)->MoveTo( 8, 247 );
		
		if( !genView->ChildAt(12)->IsHidden() )
			genView->ChildAt(12)->Hide();
		if( !genView->ChildAt(13)->IsHidden() )
			genView->ChildAt(13)->Hide();
		
		genView->OKButton->SetLabel( Language.get("OK_LABEL") );
		genView->OKButton->ResizeToPreferred();
		genView->OKButton->MoveTo( wholeWidth - genView->OKButton->Bounds().Width(), 
								   Bounds().bottom - 33 );
	} else {
		ResizeTo( abRegRect.Width(), abRegRect.Height() + 32 );
		genView->ChildAt(6)->MoveTo( 7, 175+32 );
		genView->ChildAt(7)->MoveTo( 7, 197+32 );
		genView->ChildAt(8)->MoveTo( 7, 219+32 );
		genView->ChildAt(9)->MoveTo( 145, 175+32 );
		genView->ChildAt(10)->MoveTo( 8, 2247+32 );
		
		if( genView->ChildAt(12)->IsHidden() )
			genView->ChildAt(12)->Show();
		if( genView->ChildAt(13)->IsHidden() )
			genView->ChildAt(13)->Show();
		
		genView->OKButton->SetLabel( Language.get("OK_LABEL") );
		genView->OKButton->ResizeToPreferred();
		genView->OKButton->MoveTo( wholeWidth - genView->OKButton->Bounds().Width(), 
								   Bounds().bottom - 33 );
	}
}

//=========================================================================

AboutView::AboutView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect sframe;
	BStringView* tempSV;
	LinkStringView* tempSV2;
	BBitmap* mainlogo = NULL;
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	
	// Set up the views
	GetBitmapFromResources( mainlogo, 42 );
	BRect bframe = Bounds();
	bframe.bottom = 77;
	BitmapView* bview = new BitmapView( bframe, mainlogo );
	AddChild( bview );
	
	// BeAIM version
	sframe = BRect( 0, 70, 198, 85 );
	verSV = new BStringView( sframe, "", BeAIMVersion().String() );
	verSV->SetFont(be_plain_font);
	verSV->SetFontSize(9);
	verSV->SetAlignment(B_ALIGN_RIGHT);
	AddChild( verSV );

	// main credits (that is, me!)
	sframe = BRect( 7, 84, 198, 109 );
	bySV = new BStringView( sframe, "", "Greg Nichols" );
	bySV->SetFont(be_bold_font);
	bySV->SetFontSize(15);
	bySV->ResizeToPreferred();
	AddChild( bySV );	

	sframe = BRect( 7, 105, 198, 134 );
	bySV2 = new BStringView( sframe, "", "Kyle Donaldson" );
	bySV2->SetFont(be_bold_font);
	bySV2->SetFontSize(15);
	bySV2->ResizeToPreferred();
	AddChild( bySV2 );	

	
	// other code contributors title
	sframe = BRect( 7, 130, 198, 0 );
	majorSV = new BStringView( sframe, "", "Major contributors:" );
	majorSV->SetFont(be_bold_font);
	majorSV->SetFontSize(12);
	majorSV->ResizeToPreferred();
	AddChild( majorSV );	
	
	// major code contributors text
	sframe = BRect( 13, 144, 198, 0 );
	tempSV = new BStringView( sframe, "", "Sean Heber, Justin Mierta" );
	tempSV->SetFont(be_plain_font);
	tempSV->SetFontSize(11);
	tempSV->ResizeToPreferred();
	AddChild( tempSV );		
	
	// home page link
	sframe = BRect( 7, 175, 140, 0 );
	tempSV2 = new LinkStringView( sframe, "", "BeAIM Home Page" );
	tempSV2->SetURLMode( "http://www.fifthace.com/beaim/", true );
	tempSV2->SetFont(be_plain_font);
	tempSV2->SetFontSize(11);
	AddChild( tempSV2 );	
	
	// BeBits app page link
	sframe = BRect( 7, 197, 140, 0 );
	tempSV2 = new LinkStringView( sframe, "", "BeAIM page on BeBits" );
	tempSV2->SetURLMode( "http://www.bebits.com/app/1", true );
	tempSV2->SetFont(be_plain_font);
	tempSV2->SetFontSize(11);
	AddChild( tempSV2 );	
	
	// BeBits app page link
	sframe = BRect( 7, 219, 140, 0 );
	tempSV2 = new LinkStringView( sframe, "", "gile@csh.rit.edu" );
	tempSV2->SetURLMode( "mailto:gile@csh.rit.edu", false );
	tempSV2->SetFont(be_plain_font);
	tempSV2->SetFontSize(11);
	tempSV2->ResizeToPreferred();
	AddChild( tempSV2 );	
	
	// Set up "Get it at BeBits" logo
	GetBitmapFromResources( mainlogo, 8294 );
	bframe = BRect(0,0,49,62);
	bframe.OffsetTo( 145, 175 );
	bview = new BitmapView( bframe, mainlogo );
	bview->SetURLMode( "http://www.bebits.com/app/1", true );
	AddChild( bview ); 
	
	// the groovy line thang
	BBox* divider = new BBox(BRect(8,247,193,229),"divider", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, B_FANCY_BORDER);
	AddChild( divider );
	
	// and finally, the OK button
	OKButton = new BButton(BRect(132,260,192,260), "loginbutton", "Close", new BMessage(B_QUIT_REQUESTED)); 
	OKButton->MakeDefault(true);
	AddChild( OKButton );
	
	// translated by...
	sframe = BRect( 7, 162, 198, 0 );
	majorSV = new BStringView( sframe, "", "Translated by:" );
	majorSV->SetFont(be_bold_font);
	majorSV->SetFontSize(12);
	majorSV->ResizeToPreferred();
	majorSV->Hide();
	AddChild( majorSV );
	
	// and the name of the translator
	sframe = BRect( 13, 176, 198, 0 );
	tempSV = new BStringView( sframe, "", "Bob Joneski" );
	tempSV->SetFont(be_plain_font);
	tempSV->SetFontSize(11);
	tempSV->ResizeToPreferred();
	tempSV->Hide();
	AddChild( tempSV );	
}

//=========================================================================

LinkStringView::LinkStringView( BRect frame, const char* name, const char* text )
			: BStringView( frame, name, text )
{
	urlMode = 0;
}

//-----------------------------------------------------

void LinkStringView::Draw(BRect updateRect) {
	SetHighColor(0,0,255);
	BStringView::Draw(updateRect);
}

//-----------------------------------------------------

void LinkStringView::MouseDown(BPoint point) {
	
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

//-----------------------------------------------------

void LinkStringView::SetURLMode( BString u, bool web ) {
	urlMode = web ? 1 : 2;
	url = u;
}

//-----------------------------------------------------
