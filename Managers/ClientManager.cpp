#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ClientManager.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "Say.h"
#include "xmlparse.h"

//-----------------------------------------------------

ClientManager::ClientManager() {

	loggedIn = false;
	awayMode = AM_NOT_AWAY;
	warningLevel = 0;
	idleTimer = 0;
}

//-----------------------------------------------------

ClientManager::~ClientManager()
{
}

//-----------------------------------------------------

void ClientManager::SetUser( AIMUser user ) {

	lock.Lock();
	
	// same actual screen name? We're probably re-logging in or something
	if( currentUser == user ) {
		currentUser = user;
		Clear(false);
	}

	// if there was someone there before, save their settings and stuff
	else if( currentUser.Username().Length() ) {
		prefs->WriteUserFile( currentUser );
		Clear(true);
	}

	// otherwise, we're loading a first-time user	
	// set currentUser and load the user file
	currentUser = user;
	prefs->ReadUserFile( currentUser );

	// now, everybody reload everything...
	PostAppMessage( new BMessage(BEAIM_RELOAD_PREF_SETTINGS) );
	lock.Unlock();
}

//-----------------------------------------------------

AIMUser ClientManager::User() {
	return currentUser;
}


//-----------------------------------------------------

void ClientManager::Clear( bool full ) {

	// clear the window manager stuff
	windows->Cleanup( full );

	// save the settings of the current user, if any
	if( full && currentUser.Username().Length() )
		prefs->WriteUserFile( currentUser );

	// finally, clear the buddylist and the prefs
	users->Clear();
	prefs->Clear();
	if( full )
		windows->Clear();
}

//-----------------------------------------------------

int ClientManager::AwayMode() {
	return awayMode;
}

//-----------------------------------------------------

void ClientManager::SetAwayMode( int am, BString messageInfo ) {

	BString finalMsg;
	bool away = true;
	
	// reset the away message if we are coming back
	if( am == AM_NOT_AWAY ) {
		currentAwayMessage = "";
		away = false;
	}

	// no need to do anything if we're coming back and we're already back
	if( am == AM_NOT_AWAY && awayMode == AM_NOT_AWAY )
		return;

	// grab what will become the (new?) away message
	if( am == AM_CUSTOM_MESSAGE ) {
		finalMsg = messageInfo;
		prefs->SetCustomAwayMessage( finalMsg );
	} else if( am == AM_STANDARD_MESSAGE )
		prefs->GetAwayMessage( messageInfo, finalMsg );

	// if it's a standard message, has anything changed?
	if( am == AM_STANDARD_MESSAGE && am == awayMode &&
		awayMessageName == messageInfo && currentAwayMessage == finalMsg )
		return;

	// save the name of the current message
	if( am == AM_STANDARD_MESSAGE )
		awayMessageName = messageInfo;

	// save the mode and actual message
	currentAwayMessage = finalMsg;
	awayMode = am;

	// Finally... go away!
	BMessage* awMsg = new BMessage(BEAIM_GOING_AWAY);
	awMsg->AddBool( "away", away );
	PostAppMessage( awMsg );
	
	// broadcast that the away status changed
	BMessage* awchangedMsg = new BMessage(BEAIM_AWAY_STATUS_CHANGED);
	awchangedMsg->AddBool( "away", away );
	if( awayMode == AM_CUSTOM_MESSAGE )
		awchangedMsg->AddBool( "custom", true );
	windows->BroadcastMessage( awchangedMsg );
}

//-----------------------------------------------------

BString ClientManager::CurrentAwayMessage() {
	BString msg = currentAwayMessage;
	return msg;
}

//-----------------------------------------------------

BString ClientManager::CurrentAwayMessageName() {
	return awayMessageName;
}

//-----------------------------------------------------

void ClientManager::ReplaceTagVars( BString& origMsg ) {

	time_t now;
	char theTime[50];
	char theDate[50];

	// make the time/date strings
	time(&now);
	strftime( theTime, 50, "%I:%M:%S %p", localtime(&now) );
	strftime( theDate, 50, "%b %e, %Y", localtime(&now) );

	// handle the "variables" in AIM away messages...
	origMsg.ReplaceAll( "%n", (char*)currentUser.UserString() );
	origMsg.ReplaceAll( "%t", theTime );
	origMsg.ReplaceAll( "%d", theDate );
}

//-----------------------------------------------------

void ClientManager::SetLoggedIn( bool li ) {

	loggedIn = li;
	
	// tell the world about it
	BMessage* msg = new BMessage(BEAIM_LOGIN_STATUS_CHANGED);
	msg->AddBool( "loggedin", loggedIn );
	windows->BroadcastMessage( msg, true );
	
	// does the user need to import any buddies or anything?
//	if( loggedIn && users->CountGroups() == 0 )
//		PostAppMessage( new BMessage(BEAIM_PERHAPS_IMPORT) ); 
}

//-----------------------------------------------------

int32 ClientManager::WarningLevel() {
	return warningLevel;
}

//-----------------------------------------------------

void ClientManager::SetWarningLevel( int32 wLevel ) {

	// set the new level
	warningLevel = wLevel;

	// tell the rest of the client about the new warning level
	BMessage* msg = new BMessage(BEAIM_GOT_WARNED);
	msg->AddInt32( "warninglevel", (int32)wLevel );
	msg->AddBool("noappsend", true);
	PostAppMessage( msg );
}

//-----------------------------------------------------

void ClientManager::Idle() {

	// increment the idle timer
	idleTimer++;
	
	// more than 10 minutes? get idle
	if( idleTimer == 10 ) {
		aimnet->PostMessage( new BMessage( BEAIM_NOW_IDLE ) );
	}
}

//-----------------------------------------------------

void ClientManager::NotIdle() {

	// if we're not officially idle, nothing to do
	if( idleTimer < 10 )
		return;
	
	// get un-idle
	idleTimer = 0;
	aimnet->PostMessage( new BMessage( BEAIM_NOW_ACTIVE ) );
}
		
//-----------------------------------------------------		
