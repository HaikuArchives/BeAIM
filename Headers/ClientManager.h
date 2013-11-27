#ifndef _CLIENT_MANAGER_H_
#define _CLIENT_MANAGER_H_

#include <String.h>
#include <File.h>
#include <Directory.h>
#include <Locker.h>
#include <stdio.h>
#include "GenList.h"
#include "WindowManager.h"
#include "AIMUser.h"

//-----------------------------------------------------

class ClientManager {

	public:
	
		ClientManager();
		~ClientManager();

		void SetUser( AIMUser user );
		AIMUser User();

		// Cleanup() is called when a user gets disconnected, but they might reconnect... it
		// cleans stuff up but keeps enough info to be able to reconnect the user with minimum
		// annoyance. Clear() is called to clean everything up, like when we're closing and stuff.
		void Cleanup();
		void Clear( bool full=true );
		
		// warning level stuff
		int32 WarningLevel();
		void SetWarningLevel( int32 );

		// away message functions
		void SetAwayMode( int awayMode, BString messageInfo = "" );
		BString CurrentAwayMessage();
		BString CurrentAwayMessageName();
		int  AwayMode();
		void ReplaceTagVars( BString& );

		// other stuff
		void SetLoggedIn( bool li );
		bool LoggedIn() { return loggedIn; };
		
		// idle-ness stuff
		void Idle();
		void NotIdle();

	private:

		// away status and stuff
		BString currentAwayMessage;
		BString awayMessageName;
		int awayMode;

		// vars
		AIMUser currentUser;
		BString profile;
		bool loggedIn;
		int32 warningLevel;
		int32 idleTimer;
		BLocker lock;
};

//-----------------------------------------------------

#endif
