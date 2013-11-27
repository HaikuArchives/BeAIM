#include <Application.h>
#include <Box.h>
#include <stdio.h>
#include "Say.h"
#include "InfoWindow.h"
#include "PeopleEdit.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "HTMLStuff.h"
#include "constants.h"
#include "DLanguageClass.h"
#include "Linkify.h"

const int awayOffset = 75;

//-----------------------------------------------------

InfoWindow::InfoWindow( BRect frame, AIMUser uName )
				: BWindow(frame, "", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	char windowTitle[DISPLAY_NAME_MAX+50];
	userName = uName;

	// Make the message to request the info
	BMessage* infMessage = new BMessage( BEAIM_GET_USER_INFO );
	infMessage->AddString( "userid", uName.UserString() );

	// do some name stuff
	sprintf( windowTitle, "%s: %s", Language.get("IW_USER_INFO"), uName.UserString() );
	SetTitle( windowTitle );

	// invoke the instantiation routine to create a new view 
	BRect rect = Bounds();
	rect.bottom -= 15;
	iView = new InfoView( users->IsABuddy(uName), rect, "InfoView", infMessage, users->GetBuddyEncoding(uName) ); 

	// add the view to the window 
	AddChild(iView);

	// make the stats view
	BRect statFrame = Bounds();
	statFrame.bottom = Bounds().bottom;
	statFrame.top = statFrame.bottom - 14;
	statView = new GStatusView( (char*)LangWithSuffix("IM_WAITING_FOR_INFO", B_UTF8_ELLIPSIS), statFrame );
	statView->SetSpinner(true);
	AddChild( statView );
	
	// Tell the main app that the window has been opened
	BMessage* sendMessage = new BMessage( BEAIM_IM_WINDOW_OPENED );
	sendMessage->AddInt32( "wtype", (int32)USER_INFO_TYPE );
	sendMessage->AddString( "userid", uName.UserString() );
	sendMessage->AddPointer( "new_window", this );
	PostAppMessage( sendMessage );
	
	gotInfo = false;
	gotAway = false;
	needsAway = false;
	askedAway = false;
	
	// get all the language stuff taken care of
	RefreshLangStrings();
}

//-----------------------------------------------------

void InfoWindow::RefreshLangStrings() {

	iView->label1->SetText( (char*)LangWithSuffix("STAT_USER_CLASS", ":") );
	iView->label3->SetText( (char*)LangWithSuffix("STAT_MEMBER_SINCE", ":") );
	iView->label5->SetText( (char*)LangWithSuffix("STAT_ONLINE_TIME", ":") );
	iView->label7->SetText( (char*)LangWithSuffix("STAT_WARNING_LEVEL", ":") );
	if( iView->statNotIdle )
		iView->label9->SetText( (char*)LangWithSuffix("STAT_STATUS", ":") );
	else
		iView->label9->SetText( (char*)LangWithSuffix("STAT_IDLE_TIME", ":") );
	iView->infoBox->SetLabel( Language.get("IW_USER_INFO") );
	iView->profileLabel->SetText( Language.get("IM_PROFILE_LABEL") );
	iView->awayLabel->SetText( Language.get("IM_AWAY_MSG_LABEL") );
}

//-----------------------------------------------------

bool InfoWindow::QuitRequested()
{
	// Tell the main app that the window has been closed
	BMessage* sendMessage = new BMessage( BEAIM_IM_WINDOW_CLOSED );
	sendMessage->AddInt32( "wtype", (int32)USER_INFO_TYPE );
	sendMessage->AddString( "userid", userName.UserString() );
	PostAppMessage( sendMessage );
	
	return true;
}

//-----------------------------------------------------

void InfoWindow::AskAway() {

	BMessage* awMsg = new BMessage(BEAIM_GET_AWAY_INFO);
	awMsg->AddString( "userid", userName.UserString() );
	PostAppMessage( awMsg );
	askedAway = true;
}

//-----------------------------------------------------

void InfoWindow::MessageReceived( BMessage* msg ) {

	switch( msg->what ) {
	
		case BEAIM_UPDATE_INFO:
			iView->UpdateDisplay( msg );
			gotInfo = true;
			needsAway = needsAway || (msg->HasBool("away") && msg->FindBool("away"));
			if( needsAway && !askedAway )
				AskAway();
			if( needsAway && !gotAway )
				statView->SetMessage( (char*)LangWithSuffix("IW_WAITING_FOR_AWAY",B_UTF8_ELLIPSIS) );
			else {
				statView->SetMessage( (char*)Language.get("IM_GOT_INFO") );
				statView->SetSpinner(false);
			}
			iView->Invalidate();
			break;
			
		case BEAIM_UPDATE_AWAY_INFO:
			if( gotInfo ) {
				statView->SetMessage( (char*)Language.get("IM_GOT_INFO") );
				statView->SetSpinner(false);
			} else
				statView->SetMessage( (char*)LangWithSuffix("IM_WAITING_FOR_INFO",B_UTF8_ELLIPSIS) );
			EnableAwayMode();
			iView->UpdateAwayDisplay( msg );
			iView->Invalidate();
			gotAway = true;
			break;

		case B_CANCEL:
		case BEAIM_CLOSE_WINDOW:
			QuitRequested();
			Close();
			break;
			
		case BEAIM_UPDATE_NAME_FORMAT:
			userName = AIMUser(msg->FindString("userid"));
			break;
			
		case BEAIM_ADD_BUDDY:
			AddPerson();
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived( msg );
			break;
	}
}

//-----------------------------------------------------

// the cancel function
void InfoWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

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

void InfoWindow::AddPerson() {
 	
 	// open the "add buddy" window
 	windows->MakeAddBuddyWindow( userName, this, true );
}

//-----------------------------------------------------

void InfoWindow::EnableAwayMode() {

	// resize for the away stuff if necessary
	iView->ResizeBy( 0, awayOffset );
	ResizeBy( 0, awayOffset );
	
	// move some interface elements
	iView->profHolder->MoveBy( 0, awayOffset );
	iView->profileLabel->MoveBy( 0, awayOffset );
	iView->addToListButton->MoveBy( 0, awayOffset );
	iView->closeButton->MoveBy( 0, awayOffset );
	
	// make some others visible
	iView->awayLabel->Show();
	iView->awayHolder->Show();
}

//=====================================================

InfoView::InfoView( bool isBud, BRect rect, const char *name, BMessage* rqm, uint32 enc )
	   	   : BView(rect, name, B_FOLLOW_ALL, B_WILL_DRAW)
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	reqMsg = rqm;
	isBuddy = isBud;
	encoding = enc;
	statNotIdle = true;
}

//-----------------------------------------------------

void InfoView::AttachedToWindow() {

	bool isAway = false;
	int awayOff = awayOffset;
	if( !isAway )
		awayOff = 0;

	BRect labelFrame1, labelFrame2;
	BFont chatFont;

	// Make the box frame
	BRect boxFrame = Bounds();
	boxFrame.InsetBy(10,5);
	boxFrame.bottom = 125;

	// make the box
	infoBox = new BBox( boxFrame );
	
	// set up the label frames
	labelFrame1 = boxFrame;
	labelFrame1.InsetBy( 12, 15 );
	labelFrame1.bottom = labelFrame1.top + 20;
	labelFrame2 = labelFrame1;
//	labelFrame2.right += 10;
	labelFrame2.left = Bounds().Width() / 2 - 13;

	// make lots of labels
	label1 = new BStringView( labelFrame1, "label1", "User Class:" );
	label1->SetFont(be_bold_font);
	label1->SetFontSize(11);
	labelFrame1.top += 18;
	labelFrame1.bottom += 18;
	infoBox->AddChild( label1 );

	label2 = new BStringView( labelFrame2, "label2", "--" );
	label2->SetFont(be_plain_font);
	label2->SetFontSize(11);
	labelFrame2.top += 18;
	labelFrame2.bottom += 18;
	infoBox->AddChild( label2 );
	
	label3 = new BStringView( labelFrame1, "label3", "Member Since:" );
	label3->SetFont(be_bold_font);
	label3->SetFontSize(11);
	labelFrame1.top += 18;
	labelFrame1.bottom += 18;
	infoBox->AddChild( label3 );

	label4 = new BStringView( labelFrame2, "label4", "--" );
	label4->SetFont(be_plain_font);
	label4->SetFontSize(11);
	labelFrame2.top += 18;
	labelFrame2.bottom += 18;
	infoBox->AddChild( label4 );
	
	label5 = new BStringView( labelFrame1, "label5", "Online Time:" );
	label5->SetFont(be_bold_font);
	label5->SetFontSize(11);
	labelFrame1.top += 18;
	labelFrame1.bottom += 18;
	infoBox->AddChild( label5 );

	label6 = new BStringView( labelFrame2, "label6", "--" );
	label6->SetFont(be_plain_font);
	label6->SetFontSize(11);
	labelFrame2.top += 18;
	labelFrame2.bottom += 18;
	infoBox->AddChild( label6 );	
	
	label7 = new BStringView( labelFrame1, "label7", "Warning Level:" );
	label7->SetFont(be_bold_font);
	label7->SetFontSize(11);
	labelFrame1.top += 18;
	labelFrame1.bottom += 18;
	infoBox->AddChild( label7 );

	label8 = new BStringView( labelFrame2, "label8", "--" );
	label8->SetFont(be_plain_font);
	label8->SetFontSize(11);
	labelFrame2.top += 18;
	labelFrame2.bottom += 18;
	infoBox->AddChild( label8 );

	label9 = new BStringView( labelFrame1, "label9", "Status:" );
	label9->SetFont(be_bold_font);
	label9->SetFontSize(11);
	labelFrame1.top += 18;
	labelFrame1.bottom += 18;
	infoBox->AddChild( label9 );

	label10 = new BStringView( labelFrame2, "label10", "--" );
	label10->SetFont(be_plain_font);
	label10->SetFontSize(11);
	labelFrame2.top += 18;
	labelFrame2.bottom += 18;
	infoBox->AddChild( label10 );	
	
	// resize for the away stuff if necessary
	if( isAway ) {
		ResizeBy( 0, awayOff );
		Window()->ResizeBy( 0, awayOff );
	}

	// make the away label
	BRect awayLabelRect = boxFrame;
	awayLabelRect.bottom = awayLabelRect.top + 20;
	awayLabelRect.OffsetBy( 0, boxFrame.Height() + 3 );
	awayLabel = new BStringView( awayLabelRect, "awaylabel", "Away Message:" );
	awayLabel->SetFont(be_bold_font);
	awayLabel->SetFontSize(12);		
	AddChild( awayLabel );
	if( !isAway )
		awayLabel->Hide();

	// make the textview
	BRect trect, txrect;
	trect = boxFrame;
	trect.OffsetBy( 0, boxFrame.Height() + 25 );
	trect.right -= B_V_SCROLL_BAR_WIDTH;
	trect.bottom = trect.top + 48;
	txrect = trect;
	txrect.OffsetTo( 0, 0 );
	txrect.InsetBy( 2, 2 );
	awaymessage = new FancyTextView( trect, "text_view", txrect, B_FOLLOW_NONE );
	awayHolder = new BScrollView( "profholder", awaymessage, B_FOLLOW_NONE, 0, false, true ); 
	if( !isAway )
		awayHolder->Hide();

	// Whew... make the profile label
	BRect profLabelRect = boxFrame;
	profLabelRect.bottom = profLabelRect.top + 20;
	profLabelRect.OffsetBy( 0, boxFrame.Height() + 3 + awayOff );
	profileLabel = new BStringView( profLabelRect, "proflabel", "Profile:" );
	profileLabel->SetFont(be_bold_font);
	profileLabel->SetFontSize(12);	
	AddChild( profileLabel );	
	
	// make the textview
	trect = boxFrame;
	trect.OffsetBy( 0, boxFrame.Height() + 25 + awayOff );
	trect.right -= B_V_SCROLL_BAR_WIDTH;
	trect.bottom = trect.top + 80;
	txrect = trect;
	txrect.OffsetTo( 0, 0 );
	txrect.InsetBy( 2, 2 );
	profile = new FancyTextView( trect, "text_view", txrect, B_FOLLOW_NONE );
	profHolder = new BScrollView( "profholder", profile, B_FOLLOW_NONE, 0, false, true ); 
	
	// Set the textview's base font and attributes
	rgb_color black;
	black.red = black.green = black.blue = 0;
	chatFont.SetSize( 12.0 );
	profile->SetBaseFontAndColor( chatFont, black );
	profile->MakeEditable( false );
	profile->SetStylable( true );
	profile->SetAutoScrollEnabled( false );
	awaymessage->SetBaseFontAndColor( chatFont, black );
	awaymessage->MakeEditable( false );
	awaymessage->SetStylable( true );
	awaymessage->SetAutoScrollEnabled( false );
	
	// make some buttons
	addToListButton = new BButton( BRect(7,trect.bottom+10,120,trect.bottom+34), "add",
							Language.get("ADD_BLIST_LABEL"), new BMessage(BEAIM_ADD_BUDDY) );
	addToListButton->ResizeToPreferred();
	addToListButton->SetEnabled( !isBuddy );
	
	closeButton = new BButton( BRect(Bounds().right-68, trect.bottom + 10,
							   Bounds().right-8, trect.bottom + 34), "close", Language.get("CLOSE_LABEL"),
							   new BMessage(BEAIM_CLOSE_WINDOW) );
	closeButton->ResizeToPreferred();
	closeButton->MoveTo( Bounds().Width() - closeButton->Bounds().Width() - 8, closeButton->Frame().top );
	closeButton->MakeDefault( true );

	// add the views
	AddChild( infoBox );
	AddChild( profHolder );
	AddChild( awayHolder );
	AddChild( addToListButton );
	AddChild( closeButton );
	
	profile->MakeEditable( false );
	profile->SetStylable( true );
	
	// finally, send the message
	PostAppMessage( reqMsg );
}

//-----------------------------------------------------

void InfoView::UpdateAwayDisplay( BMessage* message ) {

	// away message
	if( message->HasString("away_message" ) ) {

		HTMLParser parse;
		styleList styles;	
		BString origMsg = BString((char*)message->FindString("away_message"));

		// translate it into UTF-8
		origMsg = ConvertToUTF8( origMsg, encoding );

		// handle the away message variables
		client->ReplaceTagVars( origMsg );

		// automatically link the web and mail addresses that weren't linked
		Linkify( origMsg );

		// Now parse the bad boy and display the results
		awaymessage->SetText("");
		awaymessage->ClearInsertStuff();
		awaymessage->ResetFontToBase();
		parse.Parse( const_cast<char*>(origMsg.String()) );
		styles = parse.Styles();
		awaymessage->SetViewColor( styles.bgColor.R(), styles.bgColor.G(), styles.bgColor.B() );
		awaymessage->Invalidate();
		awaymessage->AddStyledText( parse.ParsedString(), styles );
		styles.Clear();
		awaymessage->AddStatement();
	} 
}

//-----------------------------------------------------

void InfoView::UpdateDisplay( BMessage* message ) {
	
	time_t *membersince;
	char theTime[100], stemp[100];
	BString origProfile;
	unsigned idletime;
	ssize_t size;
	bool away = false;
	
	// away?
	if( message->HasBool("away") && message->FindBool("away") )
		away = true;
	
	// username
	if( message->HasString("userid") ) {
		
		BMessage* msg = new BMessage(BEAIM_UPDATE_NAME_FORMAT);
		msg->AddString("userid", message->FindString("userid"));
		Window()->PostMessage( msg );
		
		char windowTitle[DISPLAY_NAME_MAX+35];
		sprintf( windowTitle, "%s: %s", Language.get("IW_USER_INFO"), message->FindString("userid") );
		if( away ) {
			strcat( windowTitle, "  (" );
			strcat( windowTitle, Language.get("STAT_AWAY") );
			strcat( windowTitle, ")" );
		}
		Window()->SetTitle( windowTitle );
	}

	// user class
	if( message->HasString("userclass") ) {
		label2->SetText( message->FindString( "userclass" ) );
		printf( "IW: userclass done.\n" );
		fflush(stdout);
	}

	// member since
	if( message->HasData("membersince", B_TIME_TYPE) ) {
		message->FindData( "membersince", B_TIME_TYPE, (const void**)(&membersince), &size );
		strftime( theTime, 100, "%B %e, %Y", localtime(membersince) );
		label4->SetText( theTime );
	}
	
	// onsince	
	/*if( message->HasData("onsince", B_TIME_TYPE) ) {
	
		message->FindData( "onsince", B_TIME_TYPE, (const void**)(&onsince), &size );
		MakeElapsedTimeString( *onsince, theTime );
		label6->SetText( theTime );
	}*/
	
	// session length
	if( message->HasInt32("sessionlen") ) { 
		MakeElapsedTimeString( message->FindInt32("sessionlen"), theTime );
		label6->SetText( theTime );
	}
	
	// warning level
	if( message->HasInt32("warninglevel") ) {
		sprintf( stemp, "%u%%", (unsigned)(message->FindInt32("warninglevel")) );
		label8->SetText( stemp );
	}
	
	// idle time
	if( message->HasInt32( "idletime" ) ) {
		idletime = (unsigned)message->FindInt32("idletime");
		if( idletime ) {
			label9->SetText( (char*)LangWithSuffix("STAT_IDLE_TIME", ":") );
			MakeElapsedTimeString( (idletime*60), stemp );
			label10->SetText( stemp );
			statNotIdle = false;
		} else {
			label10->SetText( Language.get("STAT_ACTIVE") );
		}
	}
		
	// profile
	if( message->HasString("profile" ) )
		origProfile = BString((char*)message->FindString("profile"));

	if( origProfile.Length() ) {
		HTMLParser parse;
		styleList styles;

		// first, convert it to UTF-8
		origProfile = ConvertToUTF8( origProfile, encoding );

		// handle the away message variables
		client->ReplaceTagVars( origProfile );
		
		// automatically link the web and mail addresses that weren't linked
		Linkify( origProfile );

		profile->SetText("");
		profile->ClearInsertStuff();
		profile->ResetFontToBase();
		parse.Parse( const_cast<char*>(origProfile.String()) );
		styles = parse.Styles();
		profile->SetViewColor( styles.bgColor.R(), styles.bgColor.G(), styles.bgColor.B() );
		profile->Invalidate();
		profile->AddStyledText( parse.ParsedString(), styles );
		styles.Clear();
		profile->AddStatement();
	} 
	
	// put up a generic "no profile" message
	else
		profile->SetText( Language.get("IW_NO_PROFILE") );
	BView::Invalidate();
}

//=====================================================


