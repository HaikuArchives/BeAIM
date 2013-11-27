#include <Screen.h>
#include <SupportKit.h>
#include <Application.h>
#include "WindowManager.h"
#include "ChatWindow.h"
#include "InfoWindow.h"
#include "GenericInput.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "DLanguageClass.h"
#include "Say.h"

// window files
#include "ResultsWindow.h"
#include "AwayEditor.h"
#include "PrefsWindow.h"
#include "ProfileEditor.h"
#include "CustomAwayMsgEditor.h"
#include "ImporterWindow.h"
#include "BlockListEditor.h"
#include "BuddyList.h"
#include "BuddyListEditor.h"
#include "PeopleEdit.h"
#include "AboutBox.h"
#include "LoginBox.h"

//-----------------------------------------------------

WindowManager::WindowManager()
{
	IMWindowRect = BRect( 25, 45, 350, 350 );
	bList = NULL;
	signOn = NULL;
	
	// create the transcript save panel for the chat windows
	transSavePanel = new BFilePanel( B_SAVE_PANEL, NULL, NULL, 0, false,
									 new BMessage(CHATWINDOW_REAL_SAVE_TRANSCRIPT) );
	transSavePanel->SetButtonLabel( B_DEFAULT_BUTTON, Language.get("SAVE_LABEL") );
	transSavePanel->SetButtonLabel( B_CANCEL_BUTTON, Language.get("CANCEL_LABEL") );
	
	// load the single window records
	LoadSingleWindowStuff();
}

//-----------------------------------------------------

void WindowManager::ToggleHidden( bool forceShow ) {

	lock.Lock();

	// toggle the minimization of the Buddy List window...
	if( bList && bList->Lock() ) {
	
		// if the window is active, minimize it
		if( bList->WasJustDeactivated() && !forceShow )
			bList->Minimize(true);
		
		// otherwise, make it active
		else {
			bList->Minimize(false);
			bList->Activate();
		}
	
		// unlock the window again
		bList->Unlock();
	}

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::CalcNewIMRect() {

	lock.Lock();
	BScreen* screen = new BScreen;
	
	// offset it... put it back to top-left if it doesn't fit onscreen
	IMWindowRect.OffsetBy( 20, 20 );
	if( !screen->Frame().Contains(IMWindowRect) )
		IMWindowRect = BRect( 25, 25, 400, 400 );
	
	delete screen;
	lock.Unlock();
}

//-----------------------------------------------------

BWindow* WindowManager::GetBuddyList() {

	BWindow* ret;
	lock.Lock();
	ret = bList;
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

void WindowManager::OpenTranscriptSavePanel( AIMUser user, BWindow* target )
{
	BMessenger theTarget(target);
	char theTitle[500];

	// make the suggested filename
	BString fName = user.Username();
	fName.Append( ".html" );
	
	// make a new title for the window
	sprintf( theTitle, "%s: %s", Language.get("CW_SAVE_TRANSCRIPT"), user.UserString() );

	// set the target and title for the save panel and show it
	lock.Lock();
	transSavePanel->Window()->SetTitle( theTitle );
	transSavePanel->SetTarget( theTarget );
	transSavePanel->SetSaveText( fName.String() );
	transSavePanel->Show();
	lock.Unlock();
}

//-----------------------------------------------------

// for DEBUG USE ONLY!!!!!!!
void WindowManager::MakeDataSenderWindow() {

	BRect frame(0,0,415,250);
	frame.OffsetBy( 100, 100 );

	DataSenderWindow* dSend = new DataSenderWindow( frame );
	dSend->Show();
}

//-----------------------------------------------------

void WindowManager::UserWindowClosed( BMessage* message )
{
	lock.Lock();

	AIMUser ScreenName = AIMUser(message->FindString("userid"));
	int wtype = (int)message->FindInt32("wtype");
	bool Found = false;
	unsigned int i;
	
	// only save the position for *message* windows...
	if( wtype == USER_MESSAGE_TYPE )
		SaveWindowPos( ScreenName, message->FindRect("frame"), message->FindFloat("divider") );
	
	// Find the window in the list
	Found = false;
	for( i = 0; i < IMWindows.Count(); ++i ) {
		if( IMWindows[i].type == wtype && ScreenName == IMWindows[i].name )
		{
			Found = true;
			break;
		}
	}
	
	// Delete the record (if it was there)
	if( Found )
		IMWindows.Delete( i );

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::SaveWindowPos( AIMUser user, BRect frame, float divider ) {

	winPosRect adder;
	unsigned int i;

	// find the correct position record
	for( i = 0; i < winPositions.Count(); ++i )
	{
		if( user == winPositions[i].user ) {
			winPositions.Delete(i);
			break;
		}
	}

	// now insert it in the first position
	adder.user = user;
	adder.frame = frame;
	adder.divider = divider;
	winPositions.Insert(adder, 0);
}

//-----------------------------------------------------

void WindowManager::SwitchUserWindow( BMessage* message ) {

	lock.Lock();
	IMWindowItem item, target;
	bool Found = false, keepGoing;
	
	// true if we should go the the next one, false if to the prev
	bool forward = bool(message->what == BEAIM_NEXT_CHAT_WINDOW);
	
	// if the list is empty, just ignore it
	if( IMWindows.IsEmpty() ) {
		lock.Unlock();
		return;	
	}

	// was this sent from a user window or somewhere else?
	if( message->HasString("userid") ) {

		AIMUser ScreenName = AIMUser( (char*)message->FindString("userid") );
		int wtype = (int)message->FindInt32("wtype");

		// Find the window in the list
		keepGoing = IMWindows.First( item );
		while( keepGoing ) {
			if( item.type == wtype && ScreenName == item.name )
			{
				Found = true;
				break;
			}
			keepGoing = IMWindows.Next( item );
		}
	}
	
	// if it wasn't there, use the first or last one
	if( !Found ) {
		if( forward )
			IMWindows.First( target );
		else
			IMWindows.Last( target );
		target.window->Activate();

		lock.Unlock();
		return;
	}

	// now look for the next (or previous) window
	if( forward ) {
		keepGoing = IMWindows.Next( target );
		if( !keepGoing )
			keepGoing = IMWindows.First( target );
	} else {
		keepGoing = IMWindows.Prev( target );
		if( !keepGoing )
			keepGoing = IMWindows.Last( target );
	}

	// activate the target window
	target.window->Activate();

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::OpenBuddyList( BPoint* point ) {

	lock.Lock();
	BRect winRect, frame;
	BRect screenRect = BScreen().Frame();
	
	// get the window position from the global prefs
	winRect.left = float(prefs->ReadInt32( "buddylist.x1", -32000 ));
	winRect.top = float(prefs->ReadInt32( "buddylist.y1", -32000 ));
	winRect.right = float(prefs->ReadInt32( "buddylist.x2", -32000 ));
	winRect.bottom = float(prefs->ReadInt32( "buddylist.y2", -32000 ));
	
	// try to recreate the saved frame... if we can't, go with the default frame
	if( winRect.left == -32000 || winRect.top == -32000 ||
		winRect.right == -32000 || winRect.bottom == -32000 ) 
	{
		frame = BRect(0,0,158,270);
		frame.OffsetBy( screenRect.Width() - frame.Width() - 10, 29 );
	} else {
		frame = winRect;
		CorrectFrame( frame );
	}
	
	// create the buddylist window and show it
	bList = new BuddyListWindow( frame );
	bList->Show();
	
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::UserWindowOpened( BMessage* message )
{
	lock.Lock();

	BWindow* newWindow;
	char name[INTERNAL_NAME_MAX];

	// Get the data from the BMessage and make a new record out of it
	message->FindPointer( "new_window", (void**)&newWindow );
	strcpy( name, message->FindString("userid") );
	IMWindowItem newRecord( name, (int)message->FindInt32("wtype"), newWindow );
	
	// Add that record to the list
	IMWindows.Add( newRecord );
	
	lock.Unlock();
}

//-----------------------------------------------------

BWindow* WindowManager::OpenUserWindow( BMessage* message )
{
	lock.Lock();
	BWindow *userWindow = NULL, *posWindow = NULL;

	AIMUser userName = AIMUser( (char*)message->FindString("userid") );
	int wtype = (int)message->FindInt32("wtype");
	if( message->HasPointer("poswindow") )
		message->FindPointer( "poswindow", (void**)&posWindow );

	for( unsigned int i = 0; i < IMWindows.Count(); ++i ) {

		// The window the user wants is already open... activate it
		if( IMWindows[i].type == wtype && userName == IMWindows[i].name )
		{
			BWindow* theWindow = IMWindows[i].window;
			theWindow->Activate();
			lock.Unlock();
			return theWindow;
		}
	}

	// Otherwise, make a new one
	switch( wtype ) {

		// a message (IM) window
		case USER_MESSAGE_TYPE:
			userWindow = MakeMessageWindow( userName );
			break;
			
		case USER_INFO_TYPE:
			userWindow = MakeInfoWindow( userName, posWindow );
			break;
	}

	// show it and quit
	userWindow->Show();
	lock.Unlock();
	return userWindow;
}

//-----------------------------------------------------

BWindow* WindowManager::MakeMessageWindow( AIMUser userName ) {

	BRect screenRect = BScreen().Frame();
	unsigned int i = 0;
	bool Found = false;
	float divider = DEFAULT_DIVIDER_POS;
	BWindow* ret;
	BRect frame = IMWindowRect;

	// try and find a saved window position for this user
	for( i = 0; i < winPositions.Count(); ++i ) {
		if( userName == winPositions[i].user )
		{
			Found = true;
			userName = winPositions[i].user;
			frame = winPositions[i].frame;
			divider = winPositions[i].divider;
			break;
		}
	}

	// use one if we found one; otherwise, use the default
	if( !Found || !screenRect.Contains(frame) ) {
		ret = new ChatWindow( userName, IMWindowRect );
		CalcNewIMRect();
	} else
		ret = new ChatWindow( userName, frame, divider );
	
	return ret;
}

//-----------------------------------------------------

BWindow* WindowManager::MakeInfoWindow( AIMUser userName, BWindow* posWindow )
{
	lock.Lock();
	BRect rect(0,0,330,286);

	// position it appropriately
	if( posWindow )
		MakeDialogFrame( rect, posWindow );
	else
		CenterFrame( rect );

	// make it	
	InfoWindow* ret = new InfoWindow( rect, userName );
	lock.Unlock();
	return ret;
}
		
//-----------------------------------------------------

void WindowManager::OpenSignOnWindow( BPoint* point ) {

	lock.Lock();
	BPoint winPos;
	BRect frame( 0, 0, 201, 222 );
	BRect screenRect = BScreen().Frame();
	
	// get the window position from the global prefs
	winPos.x = float(prefs->ReadInt32( "login.x", -32000, true ));
	winPos.y = float(prefs->ReadInt32( "login.y", -32000, true ));
	
	// try to recreate the saved frame... if we can't, go with the default frame
	if( winPos.x == -32000 || winPos.x == -32000 )
		frame.OffsetBy( screenRect.Width() - frame.Width() - 10, 29 );
	else {
		frame.OffsetTo( winPos );
		CorrectFrame( frame );
	}

	// create the signon window and show it
	signOn = new LoginWindow( frame );
	signOn->Show();

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::CloseSignOnWindow( BPoint* point ) {

	lock.Lock();

	// lock the sign on window and close it
	signOn->Lock();
	signOn->Quit();
	
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::ForwardIncomingMessage( BMessage* message ) {

	lock.Lock();

	AIMUser ScreenName = AIMUser(message->FindString("userid"));
	BWindow* window;
	bool Found = false;
	unsigned int i;
	int wtype = USER_OTHER_INFO;

	if( message->HasInt32("wtype") )
		wtype = (int)message->FindInt32("wtype");

	// Find the window in the list
	for( i = 0; i < IMWindows.Count(); ++i ) {
		if( (wtype == USER_OTHER_INFO || IMWindows[i].type == wtype) && ScreenName == IMWindows[i].name )
		{
			Found = true;
			break;
		}
	}
	
	// Construct a BMessenger and forward the message
	if( Found && (window = IMWindows[i].window) ) {
		//BMessenger sender( IMWindows[i].window );
		//sender.SendMessage( message );
		window->Lock();
		window->PostMessage(message);
		window->Unlock();
	} else if( wtype == USER_MESSAGE_TYPE ) {	
		//BWindow* newWindow = OpenUserWindow( message );
		//BMessenger sender( newWindow );
		window = OpenUserWindow( message );
		window->Lock();
		window->PostMessage( message );	
		window->Unlock();
	}
	
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::SendBuddyListMessage( BMessage* message ) {

	if( !bList )
		return;
	BMessenger sender( bList );
	sender.SendMessage( message );
}
		
//-----------------------------------------------------		

void WindowManager::SendSignOnMessage( BMessage* message ) {

	if( !signOn )
		return;
	BMessenger sender( signOn );
	sender.SendMessage( message );
}

//-----------------------------------------------------

void WindowManager::BroadcastMessage( BMessage* message, bool toApp ) {

	lock.Lock();
 	BWindow* window;
 	
 	// send it to the buddylist and sign-on window
 	SendBuddyListMessage( message );
 	SendSignOnMessage( message );

	// deliver the message to all the single windows
	for( unsigned i = 0; i < singleWindows.Count(); ++i ) {
		if( (window = singleWindows[i].window) ) {
			BMessenger sender(window);
			sender.SendMessage( message );
		}
	}
	
	// in most cases, no point in sending to the app, anyway...
	if( toApp )
		PostAppMessage( message );
	
	// send to all the open chat windows
	for( unsigned int i = 0; i < IMWindows.Count(); ++i ) {
		BMessenger sender(IMWindows[i].window);
		sender.SendMessage( message );
	}
		
	lock.Unlock();
}
		
//-----------------------------------------------------

void WindowManager::CenterFrame( BRect& frame ) {

	lock.Lock();
	BRect centered;
	BScreen* screen = new BScreen;	
	
	centered.left = (screen->Frame().Width() - frame.Width()) / 2;
	centered.top = (screen->Frame().Height() - frame.Height()) / 2;
	centered.right = centered.left + frame.Width();
	centered.bottom = centered.top + frame.Height();
	frame = centered;
	
	delete screen;
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::MakeDialogFrame( BRect& frame, BWindow* window ) {
	
	lock.Lock();
	BPoint point;

	// make sure there IS a window	
	if( !window ) {
		lock.Unlock();
		return;
	}
	
	// get it's top left corner and call the real function
	point = window->Frame().LeftTop();
	MakeDialogFrame( frame, point );
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::MakeDialogFrame( BRect& frame, BPoint point ) {

	lock.Lock();
	
	// Move the rectangle sligtly down and right relative to point
	frame.OffsetTo( point.x + 20, point.y + 40 );
	CorrectFrame( frame );
		
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::CorrectFrame( BRect& frame ) {

	lock.Lock();
	//BScreen* screen = new BScreen;
	BRect screenRect = BScreen().Frame();
	float offx = 0, offy = 0;

	// if the the rect isn't totally onscreen, fix it
	if( !screenRect.Contains( frame ) ) {

		// offset by the amount that the box would be offscreen
		offx = frame.left + frame.Width() - screenRect.Width();
		offy = frame.top + frame.Height() - screenRect.Height();
		frame.OffsetBy( offx < 0 ? 0 : -offx - 20, offy < 0 ? 0 : -offy - 20 );
	}
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::OpenInputWindow( char* title, char* label, BMessage* msg, BWindow* window,
									 char* initial, int maxLen, bool toApp, char* fname )
{
	lock.Lock();

	// put the rect to window in the right spot
	BRect frame(0,0,230,100);
	if( window )
		MakeDialogFrame( frame, window );
	else
		CenterFrame( frame );
		
	// make the window
	GenericInputWindow* msgBox = new GenericInputWindow( frame, title, label, msg, window,
														 initial, maxLen, fname );

	// check and see if the message is supposed to go the the app (not a window)
	if( toApp )
		msgBox->SetTarget( BeAIMAppSig().String() );

	msgBox->Show();	
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::OpenSingleWindow( BMessage* msg, int32 whichWindow ) {

	lock.Lock();
	BRect winRect;
	BWindow* theWindow = NULL;
	BWindow* posWindow = NULL;
	int swPos;
	
	// which window are we basing the position on?
	switch( whichWindow ) {
		case 0: posWindow = bList;
			break;
		case 1: posWindow = signOn;
			break;
		case 2: posWindow = GetSingleWindow(SW_BUDDYLIST_EDITOR);
			break;
	}

	// find the position of the single window in the list
	swPos = FindSingleWindow( msg->FindInt32("wtype") );
	if( swPos == -1 ) {
		lock.Unlock();
		return;
	}

	// only make a new window if it's not already open
	if( (theWindow = singleWindows[swPos].window) == NULL ) {
	
		// create the window using its factory function...
		theWindow = singleWindows[swPos].window = singleWindows[swPos].factoryFunction(msg);
		
		// ...put the frame in the right spot...
		if( posWindow ) {
			winRect = theWindow->Frame();
			MakeDialogFrame( winRect, posWindow->Frame().LeftTop() );
			theWindow->MoveTo( winRect.LeftTop() );
		} else
			CenterFrame( winRect );
		
		// ...and show it
		if( theWindow )
			theWindow->Show();
	}

	// set (or reset) the window
	if( theWindow ) {
		theWindow->Activate();	
		theWindow->PostMessage( msg );
	}
	
	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::SingleWindowClosed( BMessage* msg ) {

	lock.Lock();
	
	// find the position of the single window in the list
	int swPos = FindSingleWindow( msg->FindInt32("wtype") );
	if( swPos == -1 ) {
		lock.Unlock();
		return;
	}
	
	// set the window pointer to NULL
	singleWindows[swPos].window = NULL;
	lock.Unlock();
}

//-----------------------------------------------------

BWindow* WindowManager::GetSingleWindow( int32 whichWindow ) {

	lock.Lock();
	BWindow* ret = NULL;
	
	// find the position of the single window in the list
	int swPos = FindSingleWindow( whichWindow );
	if( swPos == -1 ) {
		lock.Unlock();
		return NULL;
	}
	
	// set the window pointer to NULL
	ret = singleWindows[swPos].window;
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

void WindowManager::SendSingleWindowMessage( BMessage* msg, int wtype ) {

	lock.Lock();
	BWindow* window;

	// find the position of the single window in the list
	int swPos = FindSingleWindow( wtype );
	if( swPos == -1 ) {
		lock.Unlock();
		return;
	}
	
	// deliver the message
	window = singleWindows[swPos].window;
	if( window ) {
		window->Lock();
		window->PostMessage( msg );
		window->Unlock();
	}

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::CloseSingleWindow( BMessage* msg ) {

	lock.Lock();
	SendSingleWindowMessage( new BMessage(B_QUIT_REQUESTED), msg->FindInt32("wtype") );
	lock.Unlock();
}

//-----------------------------------------------------

bool WindowManager::GetNextWindowPos( winPosRect& wp, bool first ) {

	if( first )
		return winPositions.First(wp);
	else
		return winPositions.Next(wp);
}

//-----------------------------------------------------

void WindowManager::ClearWindowPositions() {
	winPositions.Clear();
}

//-----------------------------------------------------

// a horrendously inefficient procedure, since it has to traverse the 
// entire list whenever we're going to add a new winpos record. But
// I'll leave it there now for safety's sake...
void WindowManager::SetWindowPos( winPosRect& wp ) {
	winPositions.Add(wp);
}

//-----------------------------------------------------

void WindowManager::CloseUserWindows( bool all ) {

	lock.Lock();
	BRect frame;
	float div;
	BWindow* window = NULL;
	
	// make the close message	
	BMessage* clsMessage = new BMessage(B_QUIT_REQUESTED);
	clsMessage->AddBool("dontreport", true);
	
	// close all the windows and save their positions
	for( unsigned i = 0; i < IMWindows.Count(); ++i ) {
		if( !all && IMWindows[i].type == USER_MESSAGE_TYPE )
			continue;
		window = IMWindows[i].window;
		window->Lock();
		if( IMWindows[i].type == USER_MESSAGE_TYPE ) {
			frame = window->Frame();
			div = ((ChatWindow*)window)->Divider();
			SaveWindowPos( IMWindows[i].name, frame, div );
		}
		window->PostMessage( clsMessage );
		window->Unlock();
		IMWindows.Delete(i);
	}

	lock.Unlock();
}


//-----------------------------------------------------

void WindowManager::CloseSingleWindows() {

	lock.Lock();
	
	// close all the single windows
	for( unsigned i = 0; i < singleWindows.Count(); ++i ) {
		if( singleWindows[i].window ) {
			BMessenger sender(singleWindows[i].window);
			sender.SendMessage( new BMessage(B_QUIT_REQUESTED) );
		}
	}

	// if the buddy list is still hanging around, close that too
	if( bList ) {
		BMessage* msg = new BMessage(B_QUIT_REQUESTED);
		msg->AddBool( "dontkill", true );
		BMessenger sender(bList);
		sender.SendMessage( msg );
		bList = NULL;
	}
	
	/*
	// if the sign on window is open for some weird reason, nuke it
	if( signOn ) {
		BMessage* msg = new BMessage(B_QUIT_REQUESTED);
		msg->AddBool( "dontkill", true );
		BMessenger sender(signOn);
		sender.SendMessage( msg );
		signOn = NULL;
	}
	*/

	lock.Unlock();
}

//-----------------------------------------------------

void WindowManager::LoadSingleWindowStuff() {

	singleWindowDefType sngWindow;

	// add the profile editor
	sngWindow.factoryFunction = CreateProfileEditorWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_PROFILE_EDITOR;
	singleWindows.Add( sngWindow );
	
	// add the away message editor
	sngWindow.factoryFunction = CreateAwayEditorWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_AWAY_EDITOR;
	singleWindows.Add( sngWindow );

	// add the about box
	sngWindow.factoryFunction = CreateAboutWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_ABOUT_BOX;
	singleWindows.Add( sngWindow );

	// add the preferences editor
	sngWindow.factoryFunction = CreatePrefsWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_PREFERENCES;
	singleWindows.Add( sngWindow );

	// add the block list editor
	sngWindow.factoryFunction = CreateBlockListEditorWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_BLOCKLIST_EDITOR;
	singleWindows.Add( sngWindow );

	// add the search results window
	sngWindow.factoryFunction = CreateSearchResultsWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_EMAIL_RESULTS;
	singleWindows.Add( sngWindow );
	
	// add the search results window
	sngWindow.factoryFunction = CreateCustomAwayMsgWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_CUSTOM_AWAY_EDITOR;
	singleWindows.Add( sngWindow );
	
	// add the buddylist import window
	sngWindow.factoryFunction = CreateImporterWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_BUDDYLIST_IMPORTER;
	singleWindows.Add( sngWindow );
	
	// add the buddylist editor window
	sngWindow.factoryFunction = CreateBuddyListSetupWindow;
	sngWindow.window = NULL;
	sngWindow.type = SW_BUDDYLIST_EDITOR;
	singleWindows.Add( sngWindow );
}

//-----------------------------------------------------

int WindowManager::FindSingleWindow( int type ) {

	int retVal = -1;
	lock.Lock();
	for( int i = 0; i < (int)singleWindows.Count(); ++i ) {
		if( singleWindows[i].type == type ) {
			retVal = i;
			break;
		}
	}
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

void WindowManager::Clear() {
	Cleanup( true );
	ClearWindowPositions();
}

//-----------------------------------------------------

void WindowManager::Cleanup( bool full ) {
	CloseUserWindows( full );
	CloseSingleWindows();
}

//-----------------------------------------------------

void WindowManager::MakeAddBuddyWindow( AIMUser user, BWindow* owner, bool commitNow, BString group ) {

	PersonAddEditWindow* addWindow = NULL;
	BRect frame( 0, 0, 250, 85 );
	
	// first, make sure there are some groups...
	if( !users->CountGroups() ) {
		ShowMessage( Language.get("ERR_NO_GROUPS"), B_STOP_ALERT );
		return;
	}

	// make a frame for the new window
	MakeDialogFrame( frame, owner );

	// create the window and open it
	addWindow = new PersonAddEditWindow( frame, owner, user, group, ET_ADDBUDDY );
	addWindow->SetNow( commitNow );
	addWindow->Show();
}

//-----------------------------------------------------

void WindowManager::ShowMessage( BString message, alert_type alertType, BString closeString, int32 sound, bool async )
{

	// alert types:
	//    B_EMPTY_ALERT, B_INFO_ALERT, B_IDEA_ALERT, B_WARNING_ALERT, B_STOP_ALERT,
	
	// "lang-ize" the close string if one wasn't specified
	if( closeString == BString("--[iOK]--") )
		closeString = Language.get("OK_LABEL");

	// play the sound if one is specified
	if( sound != -1 )
		sounds->PlaySound( sound );	
		
	// create and display the alert
	BAlert* alert = new BAlert("title", message.String(), closeString.String(),
							   NULL, NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, alertType);
	alert->SetShortcut( 0, B_ESCAPE );
	if( async )
		alert->Go( NULL );
	else
		alert->Go();
}

//-----------------------------------------------------
