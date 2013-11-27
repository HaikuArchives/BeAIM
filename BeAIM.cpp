#include <Alert.h>
#include <string.h>
#include <stdio.h>
#include <AppKit.h>
#include <AppFileInfo.h>
#include <MessageRunner.h>
#include "DLanguageClass.h"
#include "BeAIM.h"
#include "constants.h"
#include "Globals.h"
#include "AIMNetManager.h"
#include "MiscStuff.h"
#include "TrayIcon.h"
#include "IdleConstants.h"
#include "Say.h"

//-----------------------------------------------------

int main( int argc, void** argv )
{	
	strcpy( AppFileName, (char*)argv[0] );

	BeAIMApplication myApplication;
	if( myApplication.InitCheck() != B_OK ) {
		Say( "Error initializing BeAIM. Exiting." );
		return 1;
	}
	
	myApplication.Run();

	return 0;
}

//-----------------------------------------------------

BeAIMApplication::BeAIMApplication()
		  		  : BApplication(BeAIMAppSig().String())
{
	// Do startup stuff
	DoBeAIMStartupStuff();
	if( !initialized )
		return;

	// send no-op signals every minute
	noOpRunner = new BMessageRunner( this, new BMessage(BEAIM_SEND_NOOP), 120000000, -1 );
	// check for typing every half-second
	typeRunner = new BMessageRunner( this, new BMessage(BEAIM_TYPE_PULSE), 25000000, -1);

	snacRunner = new BMessageRunner( this, new BMessage(BEAIM_CLEAR_SNACPILE), 500000, -1);

	// create the signon window and show it
	windows->OpenSignOnWindow();

	/*
	client->SetUser( "Fentible42" );
	windows->OpenBuddyList();
	BMessage* punk = new BMessage();
	punk->AddInt32( "wtype", SW_PREFERENCES );
	windows->OpenSingleWindow( punk, 0 );
	*/

	/*
	user->SetUser( "MegaPiffle" );
	BRect rect(0,0,330,286); 
	rect.OffsetBy( 100, 100 );
	InfoWindow* ret = new InfoWindow( rect, "MtDew42" );
	ret->Show();
	
	snooze( 1000000 );	
	BMessage* normal = new BMessage(BEAIM_UPDATE_INFO);
	normal->AddString( "userid", "MtDEW42" );
	normal->AddString( "profile", "<html><body bgcolor=\"#66FF66\">This <b>is</b> a <i>profile@profile.com</i></body></html>" );
	normal->AddInt32( "idletime", 1023 );
	normal->AddString( "userclass", "Internet" );
	normal->AddBool( "away", false );
	ret->PostMessage( normal );
	
	snooze( 1000000 );	
	normal = new BMessage(BEAIM_UPDATE_AWAY_INFO);
	normal->AddString( "userid", "MtDEW42" ); 
	normal->AddString( "away_message", "<html><body bgcolor=\"#FF0000\"><font color=\"#FFFFFF\">Hello, %n, This <b>is</b> an www.away.com message.<br>time: %t<br>date: %d</font></body></html>" );
	ret->PostMessage( normal );
	*/

	// Fake Incoming Message...
	/*
	client->SetUser( "Fentible42" );
	windows->OpenBuddyList();
	BMessage* msgRecv = new BMessage(BEAIM_INCOMING_IM);
	msgRecv->AddString( "userid", "Fragorama" );
	msgRecv->AddInt32( "wtype", 1 );
	msgRecv->AddInt32( "idletime", 0 );
	//msgRecv.AddString( "message", "<font color=\"#BB00BB\" size=5>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!<br>Test!</font>" );
	//msgRecv.AddString( "message", "Embedding a <a href=\"http://www.yahoo.com\">Link</a><a href=\"http://www.amazon.com\"> in the text</a>... <a href=\"http://www.puddle.org\">oh yeah!</a>" );
	//msgRecv.AddString( "message", "This<b>is<i>a</i>test</b>of the most importance!" );
	//msgRecv->AddString( "message", "Hey, have you taken a look at <a href=\"http://www.bebits.com/\">BeBits</a> lately? It's <i>really</i> cool!" );
	//msgRecv->AddString( "message", "this is a test of random stuff" );
	msgRecv->AddString( "message", "<HTML><BODY BGCOLOR=\"#ffffff\"><FONT>sdfds <B></FONT><FONT>sdfdsfs </B></FONT><FONT>sdfsdfs <I></FONT><FONT>sdfsdf <B></FONT><FONT>sdfsdf </B></FONT><FONT>sdfdsf </I></FONT><FONT>sdfsdf </FONT></BODY></HTML>" );
	msgRecv->AddBool("autorespond", true );
	msgRecv->AddInt32( "warninglevel", 0 );
	windows->ForwardIncomingMessage( msgRecv );
//	msgRecv = new BMessage(BEAIM_OFFGOING_BUDDY);
//	msgRecv->AddString( "userid", "Fragorama" );
//	msgRecv->AddInt32( "wtype", USER_MESSAGE_TYPE );
//	windows->ForwardIncomingMessage( msgRecv );
	*/


	//Fake Oncoming Buddies...
	/*
	client->SetUser( "WormNazi" );
	client->SetLoggedIn( true );
	//user->ReadUserFile();
	windows->OpenBuddyList();
	BMessage* msgRecv;
	msgRecv = new BMessage(BEAIM_ONCOMING_BUDDY);
	msgRecv->AddString( "userid", "MtDew42" );
	msgRecv->AddInt32( "idletime", 0 );
	msgRecv->AddInt32( "warninglevel", 25 );
	PostAppMessage( msgRecv );
	msgRecv = new BMessage(BEAIM_ONCOMING_BUDDY);
	msgRecv->AddString( "userid", "CrzdCowboy" );
	msgRecv->AddInt32( "idletime", 30 );
	msgRecv->AddInt32( "warninglevel", 0 );
	msgRecv->AddBool( "away", true );
	PostAppMessage( msgRecv );
	msgRecv = new BMessage(BEAIM_ONCOMING_BUDDY);
	msgRecv->AddString( "userid", "i am zeph" );
	msgRecv->AddInt32( "idletime", 0 );
	msgRecv->AddInt32( "warninglevel", 0 );
	PostAppMessage( msgRecv );
	msgRecv = new BMessage(BEAIM_ONCOMING_BUDDY);
	msgRecv->AddString( "userid", "WormNazi" );
	msgRecv->AddInt32( "idletime", 0 );
	msgRecv->AddInt32( "warninglevel", 0 );
	PostAppMessage( msgRecv );
	msgRecv = new BMessage(BEAIM_ONCOMING_BUDDY);
	msgRecv->AddString( "userid", "Squaaack" );
	msgRecv->AddInt32( "idletime", 0 );
	msgRecv->AddInt32( "warninglevel", 0 );
	*/
}

//-----------------------------------------------------

BeAIMApplication::~BeAIMApplication() {

}

//-----------------------------------------------------

bool BeAIMApplication::QuitRequested() {

	DoBeAIMShutdownStuff();
	return true;
}

//-----------------------------------------------------

// Central message "switchboard" of BeAIM
void BeAIMApplication::MessageReceived( BMessage* message )
{
	switch(message->what)
	{
		// forward these to the login window
		case BEAIM_SET_LOGIN_STEP_COUNT:
		case BEAIM_LOGIN_STEP:
		case BEAIM_LOGIN_FAILURE:
			windows->SendSignOnMessage( message );
			break;

		case BEAIM_ONCOMING_BUDDY:
		case BEAIM_OFFGOING_BUDDY:
			users->SetBuddyStatus( message );
			break;

		case BEAIM_BUDDY_STATUS_CHANGE:
			windows->ForwardIncomingMessage( message );
		case BEAIM_LOAD_AWAY_MENU:
			windows->SendBuddyListMessage( message );
			break;

		case BEAIM_TYPE_PULSE:
			windows->BroadcastMessage(message, false);
			break;

		case BEAIM_SEND_MESSAGE:
		case BEAIM_CLIENT_READY:
		case BEAIM_WARN_SOMEONE:
		case BEAIM_SET_USER_BLOCKINESS:
		case BEAIM_CANCEL_SIGN_ON:
		case BEAIM_GET_USER_INFO:
		case BEAIM_GET_AWAY_INFO:
		case BEAIM_SIGN_ON:
		case BEAIM_BUDDYLIST_COMMIT:
		case BEAIM_SEARCH_BY_EMAIL:
		case BEAIM_GOING_AWAY:
		case BEAIM_SEND_IDLE_PULSE:
			aimnet->PostMessage( message );
			break;
			
		// don't send any no-ops unless we're logged in...
		case BEAIM_SEND_NOOP:
		case BEAIM_CLEAR_SNACPILE:
			if( client->LoggedIn() )
				aimnet->PostMessage( message );
			break;
			
		case BEAIM_ITF_IDLE_PULSE:
			client->Idle();
			break;
			
		case BEAIM_ITF_NO_LONGER_IDLE:
			client->NotIdle();
			break;

		// these messages mostly originate from the UserManager
		case BEAIM_ADD_BUDDY:
		case BEAIM_ADD_GROUP:
		case BEAIM_DELETE_BUDDY:
		case BEAIM_DELETE_GROUP:
		case BEAIM_MOVE_BUDDY:
		case BEAIM_MOVE_GROUP:
		case BEAIM_CHANGE_GROUP:
		case BEAIM_CHANGE_BUDDY:
			windows->SendBuddyListMessage( message );
			windows->SendSingleWindowMessage( message, SW_BUDDYLIST_EDITOR );
			break;

		case BEAIM_LOGOUT:
			Logout();
			break;
			
		case BEAIM_SEND_ARBITRARY_DATA:
			windows->MakeDataSenderWindow();
			break;

		case BEAIM_OPEN_IM_WINDOW:
		case BEAIM_OPEN_USER_WINDOW:
			windows->OpenUserWindow( message );
			break;

		case BEAIM_IM_WINDOW_OPENED:
			windows->UserWindowOpened( message );
			break;

		case BEAIM_IM_WINDOW_CLOSED:
			windows->UserWindowClosed( message );
			break;

		case BEAIM_TOGGLE_HIDDEN:
			windows->ToggleHidden(false);
			break;

/*
		case BEAIM_TOGGLE_HIDDEN: {
			static int thingy = 0;
			thingy++;
			if( !(thingy % 2) ) {
				BMessage* msg = new BMessage(BEAIM_OFFGOING_BUDDY);
				msg->AddString( "userid", "MtDew42" );
				PostAppMessage(msg);
			} else {
				BMessage* msg = new BMessage(BEAIM_ONCOMING_BUDDY);
				msg->AddString( "userid", "MtDew42" );
				PostAppMessage(msg);
			}
			break;
		}
*/

		case BEAIM_SIGN_ON_SUCCESSFUL:
			client->SetLoggedIn( true );
			windows->OpenBuddyList();
			windows->CloseSignOnWindow();
			break;
			
		case BEAIM_UPDATE_INFO:
		case BEAIM_UPDATE_AWAY_INFO:
		case BEAIM_INCOMING_IM:
		case BEAIM_TYPING_STATUS:
			windows->ForwardIncomingMessage( message );
			break;

		case BEAIM_EMAIL_SEARCH_RESULTS:
			//windows->ForwardEmailSearchResults( message );
			windows->SendSingleWindowMessage( message, SW_EMAIL_RESULTS );
			break;

		case BEAIM_NEXT_CHAT_WINDOW:
		case BEAIM_PREV_CHAT_WINDOW:
			windows->SwitchUserWindow( message );
			break;

		case BEAIM_RELOAD_PREF_SETTINGS:
			DoGlobalPrefs();
			break;
			
		case BEAIM_TOGGLE_DESKBAR_ICON:
			DoDeskbarIcon( message->FindBool("enabled") );
			break;
			
		case BEAIM_OPEN_SINGLE_WINDOW:
			if( message->HasInt32("posnum") )
				windows->OpenSingleWindow( message, message->FindInt32("posnum") );
			else
				windows->OpenSingleWindow( message );
			break;
			
		case BEAIM_SINGLE_WINDOW_CLOSED:
			windows->SingleWindowClosed( message );
			break;

		case BEAIM_DISCONNECTED:
			//if( !(message->HasBool("done")) )
			GotDisconnected( message->HasBool("quietly") );
			break;
			
		case BEAIM_PERHAPS_IMPORT:
			PerhapsImport();
			break;

		case BEAIM_SET_MY_SCREEN_NAME:
			client->SetUser( message->FindString("new_userid") );
			break;

		case BEAIM_RELOAD_BUDDYLIST:
		case BEAIM_GOT_WARNED:
			windows->BroadcastMessage( message );
			break;

		case BEAIM_JUMP_TO_BUDDYLIST:
			windows->ToggleHidden(true);
			break;
			
		default:
			BApplication::MessageReceived( message );
	}
}

//-----------------------------------------------------

void BeAIMApplication::DoBeAIMStartupStuff()
{
	BString loadedLang;

	// set some variables and stuff
	deskbarIconInstalled = false;
	initialized = false;

	// Create the prefs manager first, as its needed to load the lang name
	prefs = new PreferencesManager;

	// make the window manager
	windows = new WindowManager;

	// sounds, baby, yeah!
	sounds = new classSoundMaster;
	sounds->AddSound( WS_ENTER, "Enter", "/boot/home/config/sounds/BeAIM/AIMEnter", false );
	sounds->AddSound( WS_EXIT, "Exit","/boot/home/config/sounds/BeAIM/AIMExit", true );
	sounds->AddSound( WS_MSGSEND, "MsgSend", "/boot/home/config/sounds/BeAIM/AIMSend", true );
	sounds->AddSound( WS_MSGRECEIVE, "MsgReceive", "/boot/home/config/sounds/BeAIM/AIMReceive", true );	
	sounds->AddSound( WS_NEWMSG, "NewMsg", "/boot/home/config/sounds/BeAIM/AIMNewMessage", true );	
	sounds->AddSound( WS_WARNED, "Warned", "/boot/home/config/sounds/BeAIM/AIMGotWarned", true );
	sounds->AddSound( WS_BEEP, "Beep", "/boot/home/config/sounds/BeAIM/AIMDrip", true );

	// try and load the language file...
	prefs->ReadString( "Language", loadedLang, "English", true, false );
	Language.SetName( (char*)loadedLang.String() );
	if( Language.InitCheck() != B_OK ) {
		windows->ShowMessage( "BeAIM could not load the correct language file!", B_INFO_ALERT, "OK", WS_BEEP, false );
		return;
	}

	// Create the other necessary global objects
	aimnet = new AIMNetManager;
	users = new UserManager;
	client = new ClientManager;
	
	
	// yay, we're initialized!
	initialized = true;
}

//-----------------------------------------------------

void BeAIMApplication::DoBeAIMShutdownStuff() {

	// remove the tray icon
	if( deskbarIconInstalled )
		DoDeskbarIcon( false );

	// close down the client manager... it should take care of cleaning
	// up most of the mess that BeAIM has made  :-)
	client->Clear( true );
	delete client;
	client = 0;

	// net will take care of deleting itself
	if(aimnet) aimnet->Shutdown();
	aimnet = NULL;
	delete windows;
	windows = 0;
	delete prefs;
	prefs = 0;
	delete sounds;
	sounds = 0;
	
	// close down the User manager
	users->Close();
	users = NULL;
}


//-----------------------------------------------------

void BeAIMApplication::Logout() {

	BMessage* disMessage;
	
	// close some of the windows, anyway...
//	windows->CloseSingleWindows();

	// close the buddy list and open the signon window
//	disMessage = new BMessage(B_QUIT_REQUESTED);
//	disMessage->AddBool("dontkill", true);
//	windows->SendBuddyListMessage( disMessage );

	
	// cleanup all those silly open windows	
	windows->Cleanup(false);

	windows->OpenSignOnWindow();
	
	// let the ClientManager do its thing
	client->SetLoggedIn(false);

	// tell the network stuff to disconnect
	disMessage = new BMessage( BEAIM_LOGOUT );
	aimnet->PostMessage( disMessage );
}

//-----------------------------------------------------

void BeAIMApplication::DoDeskbarIcon( bool install ) {

	if( install )
		TrayIcon::AddTrayIcon();	
	else
		TrayIcon::RemoveTrayIcon();	
	
	deskbarIconInstalled = install;
}

//-----------------------------------------------------

void BeAIMApplication::AboutRequested() {

	BMessage* msg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	msg->AddInt32( "wtype", SW_ABOUT_BOX );
	windows->OpenSingleWindow(msg);
}

//-----------------------------------------------------

void BeAIMApplication::PerhapsImport() {

	BAlert* alert = new BAlert("", Language.get("IB_PERHAPS_IMPORT"),
							   Language.get("NO_LABEL"),
							   Language.get("YES_LABEL"),
							   NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT);
	alert->SetShortcut( 0, B_ESCAPE );
	if( alert->Go() ) {
		BMessage* msg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
		msg->AddInt32("wtype", SW_BUDDYLIST_EDITOR);
		msg->AddBool("doimport", true);
		windows->OpenSingleWindow(msg);
	}
}

//-----------------------------------------------------

void BeAIMApplication::DoGlobalPrefs() {

	// do sound prefs
	bool soundEnabled = prefs->ReadBool("PlaySounds", true);
	if( sounds->IsEnabled() != soundEnabled )
		sounds->SetEnabled( soundEnabled );

	// do deskbar icon prefs
	bool deskbarEnabled = prefs->ReadBool("UseDeskbarIcon", true);
	if( deskbarEnabled != deskbarIconInstalled ) {
		DoDeskbarIcon( deskbarEnabled );	
	}
	
	// set the deskbar visibility prefs
	SetDeskbarVisibility( prefs->ReadBool("ShowDeskbarWindowEntry",true,true) );
}

//-----------------------------------------------------

void BeAIMApplication::GotDisconnected( bool quietly ) {

	BMessage* disMessage;

	// if the client is logged in, do lotsa stuff
	if( client->LoggedIn() ) {
	
		// such as close most of the windows, for example
		windows->CloseSingleWindows();
		disMessage = new BMessage(B_QUIT_REQUESTED);
		disMessage->AddBool("dontkill", true);
		windows->SendBuddyListMessage( disMessage );
		windows->OpenSignOnWindow();
		client->SetLoggedIn(false);
		
		// tell 'em about it?
		if( !quietly )
			windows->ShowMessage( Language.get("ERR_GOT_DISCONNECTED"), B_INFO_ALERT );
	}
}

//-----------------------------------------------------

bool BeAIMApplication::SetDeskbarVisibility( bool vis )
{
	app_info appInfo;
	BFile file;
	BAppFileInfo appFileInfo;
	uint32 appFlags;

	// get the BAppFileInfo object for this application
	be_app->GetAppInfo(&appInfo);
	file.SetTo(&appInfo.ref, B_READ_WRITE);
	appFileInfo.SetTo(&file);

	// get the app flags
	appFileInfo.GetAppFlags(&appFlags);

	// make the app visible (or not) in the deskbar
	if( vis )
		appFlags &= ~B_BACKGROUND_APP;
	else
		appFlags |= B_BACKGROUND_APP;

	// now set them!
	appFileInfo.SetAppFlags(appFlags);
}

//-----------------------------------------------------

bool BeAIMApplication::InitCheck() {
	if( initialized )
		return B_OK;
	return B_ERROR;
}

//-----------------------------------------------------
