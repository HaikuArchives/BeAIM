#include <Application.h>
#include <Bitmap.h>
#include <Alert.h>
#include <Box.h>
#include <Message.h>
#include <MenuItem.h>
#include <stdlib.h>
#include "SupportKit.h"
#include "LoginBox.h"
#include "MiscStuff.h"
#include "constants.h"
#include "Globals.h"
#include "Say.h"

//=====================================================================

LoginWindow::LoginWindow(BRect frame)
 				: BWindow(frame, "Login", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	BBitmap* mainlogo = NULL;
	BRect makeRect;
	
	// make and add the bitmap view
	GetBitmapFromResources( mainlogo, 42 );
	makeRect = Bounds();
	makeRect.top = -4;
	makeRect.bottom = 81;
	bview = new BitmapView( makeRect, mainlogo );
	AddChild( bview );

	// and the version string	
	versionLabel = new EnStringView( BRect(0,76,198,87), "", BeAIMVersion().String() );
	versionLabel->SetFont(be_plain_font);
	versionLabel->SetViewColor( 255, 200, 200 );
	versionLabel->SetFontSize(9);
	versionLabel->SetAlignment(B_ALIGN_RIGHT);
	versionLabel->SetEnabled( false );
	bview->AddChild( versionLabel );
	
	// make and add the login view
	makeRect = Bounds();
	makeRect.top = 82;
	logView = new LoginView(makeRect);
	AddChild( logView );
	
	// make and add the countdown view
	countView = new LoginCountdownView( BRect( 5,144,196,144+31) );
	logView->AddChild( countView );
	countView->Hide();
	logView->ScreenName->MakeFocus(true);
	
	// make and add the progress view
	makeRect = Bounds();
	makeRect.top = 82;
	makeRect.bottom = 163;
	progressView = new LoginProgressView(makeRect);
	progressView->Hide();
	AddChild( progressView );
	
	// get all the right language strings
	RefreshLangStrings();
	
	// load the login settings
	LoadLoginSettings();
}

//---------------------------------------------------------------------

LoginWindow::~LoginWindow() {
}

//---------------------------------------------------------------------

bool LoginWindow::QuitRequested()
{
	SaveLoginSettings( false );
	if( !client->LoggedIn() )
		be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}

//---------------------------------------------------------------------

void LoginWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case BEAIM_SIGN_ON:
			// added to not try to login while it's already happening
			countView->StopCountdown();
			DoLogin();
			break;
			
		case BEAIM_CANCEL_SIGN_ON:
			PostAppMessage( new BMessage(BEAIM_CANCEL_SIGN_ON) );
			SetProgressMode( false );
			break;
			
		// set the number of steps in the progress bar
		case BEAIM_SET_LOGIN_STEP_COUNT:
			progressView->Progress->SetMaxValue( (float)message->FindInt32("count")+1 );
			progressView->Progress->Update( 1.0 );
			progressView->Caption->SetText( "Starting connection routine" B_UTF8_ELLIPSIS );
			break;
			
		case BEAIM_LOGIN_STEP:
			progressView->Progress->Update( 1 );	// move the indicator on the progress bar
			if( message->FindBool("update_text") )
				progressView->Caption->SetText( message->FindString("status") );
			break;

		case BEAIM_LOGIN_FAILURE: {
			// turn off the deskbar icon
			BMessage* noDeskbarIcon = new BMessage(BEAIM_TOGGLE_DESKBAR_ICON);
			noDeskbarIcon->AddBool( "enabled", false );
			PostAppMessage(noDeskbarIcon);
			
			// fix the progress bar, and display the reason
			SetProgressMode( false );
			Say( (char*)message->FindString("reason") );
			break;
		}
			
		case BEAIM_OPEN_PREFS: {
			BMessage* msg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
			msg->AddInt32( "wtype", SW_PREFERENCES );
			msg->AddBool( "globalonly", true );
			msg->AddInt32( "posnum", 1 );
			PostAppMessage( msg );
			break;
		}
			
		case PEOPLE_SELECTOR_MSG:
			logView->LoadUserSettings( message->FindInt32("index") );
			break;
			
		case LOGIN_COUNTDOWN_MSG:
			countView->Tick();
			break;
		
		case LOGIN_STOP_COUNTDOWN:
			countView->StopCountdown();
			break;
			
		case LOGIN_EDIT_PW:		
		case LOGIN_SAVE_PW_CHECKBOX:
			logView->AdjustPreLoginSettings();
			break;

		case B_CANCEL:
			QuitRequested();
			Close();
			break;
			
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;
						
		default:
			BWindow::MessageReceived(message);
	}
}

//---------------------------------------------------------------------

// the cancel function
void LoginWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN ) {
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}
	}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//---------------------------------------------------------------------

void LoginWindow::SetCountdownMode( bool mode ) {

	// remake the window to show the countdown display
	if( mode ) {
		logView->ResizeTo( 201, 260 );
		ResizeTo( 201, 260 );
		countView->Show();
		logView->EnableControls(false);
		countView->cancelButton->MakeFocus(true);
		countView->DoCountdown();
	}
	
	// turn off the countdown display and stop the countdown
	else { 
		logView->ResizeTo( 201, 222 );
		ResizeTo( 201, 222 );
		countView->Hide();
		logView->EnableControls(true);
		logView->ScreenName->MakeFocus(true);
	}
}

//---------------------------------------------------------------------

void LoginWindow::SetProgressMode( bool mode ) {

	// remake the window to show the login progfess
	if( mode ) {
		ResizeTo( 201, 163 );
		while( !logView->IsHidden() )
			logView->Hide();
		while( progressView->IsHidden() )
			progressView->Show();
		progressView->Progress->Update( -(progressView->Progress->CurrentValue()) );
		progressView->CancelButton->MakeFocus(true);
		progressView->CancelButton->MakeDefault(true);
	}

	// make the window show the login information stuff again
	else { 
		logView->EnableControls(true);
		while( logView->IsHidden() )
			logView->Show();
		logView->ResizeTo( 201, 222 );
		ResizeTo( 201, 222 );
		while( !progressView->IsHidden() )
			progressView->Hide();
		logView->ScreenName->MakeFocus(true);
		logView->SignOnButton->MakeDefault(true);
		client->SetLoggedIn( false );
	}
}

//---------------------------------------------------------------------

void LoginWindow::LoadLoginSettings() {

	// grab the name of the person who last logged in...
	// and if there was one, load their settings
	if( prefs->LastLogin().Username().Length() )
		logView->LoadUserSettings( prefs->LastLogin() );
		
	// Now... if the auto-login check box is checked, then do the autologin
	if( logView->AutoLogin->Value() ) {
		if( logView->Password->actualText() != "" )
			SetCountdownMode( true );
		else
			logView->Password->MakeFocus(true);
	}
}

//---------------------------------------------------------------------

void LoginWindow::SaveLoginSettings( bool saveNewUser ) {

	AIMUser userName;
	
	// save the window position
	BRect winPos = Frame();
	prefs->WriteInt32( "login.x", int32(winPos.LeftTop().x), true );
	prefs->WriteInt32( "login.y", int32(winPos.LeftTop().y), true );

	// Get and save the username, and let the user view save its own stuff
	userName = AIMUser( logView->ScreenName->Text() );
	prefs->SetLastLogin( userName );
	logView->SaveUserSettings( userName, saveNewUser );
	prefs->WriteGlobalFile();
}

//---------------------------------------------------------------------

void LoginWindow::DoLogin() {

	BMessage* sendMessage = NULL;

	// setup the window for the login
	SetProgressMode( true );
	
	// set the user and save the login settings
	client->SetUser( logView->ScreenName->Text() );
	SaveLoginSettings( true );

	// send the login message
	sendMessage = new BMessage(BEAIM_SIGN_ON);
	sendMessage->AddString( "userid", logView->ScreenName->Text() );
	sendMessage->AddString( "password", (char*)logView->Password->actualText().String() );
	PostAppMessage( sendMessage );
}

//---------------------------------------------------------------------

void LoginWindow::RefreshLangStrings() {

	float temp;

	// start with the title
	SetTitle( Language.get("LW_LOGIN") );
	
	// do the buttons now
	logView->SetupButton->SetLabel( Language.get("LW_SETUP") );
	logView->SetupButton->ResizeToPreferred();
	logView->SignOnButton->SetLabel( Language.get("LW_LOGIN") );
	logView->SignOnButton->ResizeToPreferred();
	logView->SignOnButton->MoveTo( Bounds().Width() - (logView->SignOnButton->Bounds().Width()+5),
								   logView->SignOnButton->Frame().top );
								   
	// now do some labels and stuff
	logView->ScreenName->SetLabel( LangWithSuffix("LW_SCREEN_NAME",":") );
	logView->Password->SetLabel( LangWithSuffix("LW_PASSWORD",":") );
	logView->SavePassword->SetLabel( Language.get("LW_SAVEPW") );
	logView->AutoLogin->SetLabel( Language.get("LW_AUTOLOGIN") );
	
	// do the progress-view cancel button (needs to be centered)
	progressView->CancelButton->SetLabel( Language.get("CANCEL_LABEL") );
	progressView->CancelButton->ResizeToPreferred();
	temp = Bounds().Width() - progressView->CancelButton->Bounds().Width();
	progressView->CancelButton->MoveTo( temp / 2, progressView->CancelButton->Frame().top );

	// set the version string correctly
	versionLabel->SetText( BeAIMVersion().String() );
	
	// finally do the countdown view cancel button
	countView->cancelButton->SetLabel( Language.get("CANCEL_LABEL") );
}

//=====================================================================

LoginView::LoginView(BRect rect)
	   	   : BView(rect, "login_view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	//SetViewColor( 255, 200, 200 );

	// Construct the views
	ScreenName = new BTextControl( BRect(5,9,179,19), "screenname", "Screen Name:", "", NULL );
	ScreenName->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	ScreenName->SetDivider( 87 );
	// current limit is 16 chars for username
	ScreenName->TextView()->SetMaxBytes(32);
	Password = new PassControl( BRect(5,34,196,54), "password", "Password:", "", new BMessage(31337) );
	Password->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	Password->SetDivider( 87 );
	Password->SetModificationMessage( new BMessage(LOGIN_EDIT_PW) );
	// current limit is 16 chars for password
	Password->TextView()->SetMaxBytes(16);
	SavePassword = new BCheckBox( BRect(5,63,196,83), "Save Password", "Save Password", new BMessage(LOGIN_SAVE_PW_CHECKBOX) );
	SavePassword->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	AutoLogin = new BCheckBox( BRect(5,80,196,100), "autologin", "Login Automatically", new BMessage(LOGIN_AUTOLOGIN_CHECKBOX) );
	AutoLogin->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	SignOnButton = new BButton(BRect(133,109,193,129), "loginbutton", "Login", new BMessage(BEAIM_SIGN_ON)); 
	SignOnButton->MakeDefault( true );
	SetupButton = new BButton(BRect(5,109,55,129), "loginbutton", "Setup", new BMessage(BEAIM_OPEN_PREFS));
	MakeUserList();	
	userList = new BMenuField( BRect(182,7,197,26), NULL, NULL, userMenu, true );

	// Now add them to the window
	AddChild( ScreenName );
	AddChild( userList );
	AddChild( Password );
	AddChild( SavePassword );
	AddChild( AutoLogin );
	AddChild( SetupButton );
	AddChild( SignOnButton );
	
	// disable some stuff
	AdjustPreLoginSettings();
}

//---------------------------------------------------------------------

void LoginView::LoadUserSettings( int32 index ) {

	AIMUser userName;

	// Grab the user's name and call the other settings function
	userName = AIMUser( BString(userList->Menu()->ItemAt(index)->Label()) );
	LoadUserSettings( userName );
}

//---------------------------------------------------------------------

void LoginView::LoadUserSettings( AIMUser userName ) {

	BString password;
	bool autoLogin = false, savePassword = false;

	// save the old stuff and grab the user's info
	SaveUserSettings( AIMUser(ScreenName->Text()) );
	prefs->GetUserLoginParameters( userName, password, autoLogin, savePassword );

	// set stuff up based on the info in the user file
	SavePassword->SetValue( savePassword );
	AutoLogin->SetValue( autoLogin );
	ScreenName->SetText( (char*)userName.UserString() );
	if( savePassword && password.Length() ) {
		Password->SetText( password.String() );
	} else {
		Password->SetText("");
	}
	
	// enable/disable stuff
	AdjustPreLoginSettings();
}

//---------------------------------------------------------------------

void LoginView::SaveUserSettings( AIMUser userName, bool saveNewUser ) {

	// sanity check
	if( !userName.Username().Length() )
		return;
		
	// save 'em
	prefs->SaveLoginParameters( userName,
								Password->actualText(),
								AutoLogin->Value(),
								SavePassword->Value(),
								saveNewUser );
}

//---------------------------------------------------------------------

void LoginView::EnableControls( bool enabled ) {

	ScreenName->SetEnabled( enabled );
	Password->SetEnabled( enabled );
	SavePassword->SetEnabled( enabled );
	AutoLogin->SetEnabled( enabled );
	userList->SetEnabled( enabled );
	SetupButton->SetEnabled( enabled );
	AdjustPreLoginSettings();
}

//---------------------------------------------------------------------

void LoginView::MakeUserList() {

	AIMUser name;
	bool ret = true;
	bool start = true;
	int32 msgID = 0;
	BMessage* selectMessage;
	userMenu = new BMenu("");

	// add the users
	while( ret ) {
		ret = prefs->GetNextUser( name, start );
		if( !ret )
			break;		
		if( start )
			start = false;
		if( !name.Username().Length() )
			continue;
		selectMessage = new BMessage(PEOPLE_SELECTOR_MSG);
		selectMessage->AddInt32( "index", msgID++ );
		userMenu->AddItem( new BMenuItem(name.UserString(), selectMessage) );
	}
	
	// no people? Add a "(none)" item
	if( msgID == 0 ) {
		BMenuItem* disabledItem = new BMenuItem("(none)", NULL);
		disabledItem->SetEnabled( false );
		userMenu->AddItem( disabledItem );
	}
}

//---------------------------------------------------------------------

// handle all the enable/disable silliness for the password, save password, and autologin controls
void LoginView::AdjustPreLoginSettings() {

	// can't do much in terms of autologging or whatever without a password
	if( Password->actualText().Length() == 0 ) {
		if( SavePassword->IsEnabled() ) {
			SavePassword->SetEnabled(false);
			SignOnButton->SetEnabled(false);
			AutoLogin->SetEnabled(false);
		}
		return;
	}
	
	// quick hack...
	if( !Password->IsEnabled() )
		return;
	
	// otherwise, make sure that at least Save Password is enabled
	if( !SavePassword->IsEnabled() )
		SavePassword->SetEnabled(true);
	if( !SignOnButton->IsEnabled() )
		SignOnButton->SetEnabled(true);
			
	// and only allow AutoLogin to be enabled if SavePassword is
	if( (bool)SavePassword->Value() != AutoLogin->IsEnabled() )
		AutoLogin->SetEnabled( (bool)SavePassword->Value() );
}

//=====================================================================

LoginProgressView::LoginProgressView(BRect rect)
	   	   : BView(rect, "progress_view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	
	Progress = new BStatusBar( BRect(5,-8,195,12), "progress" );
	Caption = new BStringView( BRect(5,18,195,38), "caption", "" );
	CancelButton = new BButton( BRect(75,49,125,69), "cancel", "Cancel", new BMessage(BEAIM_CANCEL_SIGN_ON) );
	
	AddChild( Caption );
	AddChild( Progress );
	AddChild( CancelButton );
}

//=====================================================================

LoginCountdownView::LoginCountdownView(BRect rect)
	   	   : BView(rect, "count_view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor( 240, 240, 240 );
	
	surroundBox = new BBox( Bounds() );
	cancelButton = new BButton( BRect(128,3,186,25), "cancel", "Cancel", new BMessage(LOGIN_STOP_COUNTDOWN) );
	Caption = new BStringView( BRect(7,4,195,24), "caption", "Auto-login in 5 seconds..." );
	
	AddChild( cancelButton );
	AddChild( surroundBox );
	AddChild( Caption );
}

//---------------------------------------------------------------------

void LoginCountdownView::DoCountdown() {

	// default is 3 seconds... should probably make an option for this
	counter = 3;

	// make the thread and do the first tick
	count_thread = spawn_thread( counter_thread_entry, "countdown_thread", B_LOW_PRIORITY, this );
	if( count_thread > 0 ) {
		resume_thread( count_thread );
		Tick();
	}
}

//---------------------------------------------------------------------

void LoginCountdownView::StopCountdown() {

	//beep();

	if( count_thread ) {
		kill_thread( count_thread );
		count_thread = 0;
	}
	
	// Remake the window to hide this view
	LoginWindow* window = dynamic_cast<LoginWindow*>( Window() );
	window->SetCountdownMode( false );
}

//---------------------------------------------------------------------

void LoginCountdownView::Tick() {

	char Label[50];
	LoginWindow* parent = dynamic_cast<LoginWindow*>(Window());
	
	if( counter > 0 ) {
		//sprintf( Label, "Auto-login in %d second%s...", counter, counter==1?"":"s" );
		sprintf( Label, "%s: %d", Language.get("LW_LOGIN"), counter );
		Caption->SetText( Label );
	} else {
	
		// Do the login
		kill_thread( count_thread );
		count_thread = 0;
		parent->DoLogin();
	}

	counter--;
}

//---------------------------------------------------------------------

int32 LoginCountdownView::counter_thread_entry( void *arg ) {
	LoginCountdownView* this_class = (LoginCountdownView*)arg;
	return this_class->do_count();
}

//---------------------------------------------------------------------

int32 LoginCountdownView::do_count() {

	// sleep for one second, then send a tick message
	while( true ) {
		snooze( 1000000 );
		Window()->PostMessage( new BMessage(LOGIN_COUNTDOWN_MSG) );
	}
}
		
//=====================================================================
