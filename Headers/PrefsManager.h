#ifndef _PREFS_MANAGER_H_
#define _PREFS_MANAGER_H_

#include <String.h>
#include <File.h>
#include <Directory.h>
#include <Locker.h>
#include <time.h>
#include <stdio.h>
#include "WindowManager.h"
#include "GenList.h"
#include "AIMUser.h"

//-----------------------------------------------------

struct preLogStruct {
	AIMUser name;
	BString password;
	bool savePassword;
	bool autoLogin;
};
		
//-----------------------------------------------------

struct awayStruct {
	BString name;
	BString message;
};

//-----------------------------------------------------

struct prefInt32Struct {
	BString name;
	int32 val;
};
struct prefBoolStruct {
	BString name;
	bool val;
};
struct prefStringStruct {
	BString name;
	BString val;
};

//-----------------------------------------------------

class PreferencesManager {

	public:
		PreferencesManager();
		~PreferencesManager();

		void Clear( bool global = false );

		// profile functions		
		BString Profile();
		void SetProfile( BString );
		
		// away message functions
		void SetAwayMessage( BString name, BString oldname, BString message );
		bool GetAwayMessage( BString name, BString& message );
		int  FindAwayMessage( BString name );
		void RemoveAwayMessage( BString name );
		bool GetFirstAwayMessage( awayStruct& );
		bool GetNextAwayMessage( awayStruct& );
		void SetCustomAwayMessage( BString );
		BString CustomAwayMessage();
		int  CountAwayMessages();
		
		// preference functions
		void WriteBool( BString name, bool value, bool=false );
		bool ReadBool( BString name, bool def, bool=false );
		void WriteString( BString name, BString val, bool=false );
		void ReadString( BString name, BString& buf, BString def, bool=false, bool=true );
		void WriteInt32( BString name, int32 value, bool=false );
		int32 ReadInt32( BString name, int32 def, bool=false );
		
		// prelogin functions
		bool GetNextUser( AIMUser&, bool first = false );
		void GetUserLoginParameters( AIMUser user, BString& pw, bool& autoLogin, bool& savePassword );
		void SaveLoginParameters( AIMUser user, BString pw, bool autoLogin, bool savePassword, bool ain=false );
		void SetLastLogin( AIMUser );
		AIMUser LastLogin();
		
		// file functions
		void WriteUserFile( AIMUser );
		void ReadUserFile( AIMUser );
		void WriteGlobalFile();
		void ReadGlobalFile();
		
		// Mmmmmmm...... entitizing....
		void EntitizeNormalString( BString& );
		void EntitizeCDataString( BString& );
		void UnEntitizeNormalString( BString& );
		void UnEntitizeCDataString( BString& );

	private:

		unsigned readMode;
		bool readGlobal;
		BString rGroup, rBuddy, rAwayName, rAwayMessage;
		winPosRect wpRect;
		
		// prefs output functions
		BString GetUserFileName( AIMUser );
		void WriteAwayMessages( FILE* ptr );
		void WriteBuddyList( FILE* ptr );
		void WritePrefs( FILE* ptr );
		void WritePreLoginData( FILE* ptr );
		void WriteWindowPositions( FILE* ptr );
		void WriteBlockedUsers( FILE* ptr );
		
		// expat XML handler functions
		static void startElement( void *userData, const char *name, const char **atts );
		static void endElement( void *userData, const char *name );
		static void characterData( void *userData, const char *data, int len );
		void ReadOption( const char *name, const char **atts );

		// preferences
		GenList<prefInt32Struct> prefInt;
		GenList<prefBoolStruct> prefBool;
		GenList<prefStringStruct> prefString;
		GenList<prefInt32Struct> gPrefInt;
		GenList<prefBoolStruct> gPrefBool;
		GenList<prefStringStruct> gPrefString;
		
		// away status and stuff
		GenList<awayStruct> awayMessages;
		BString customAwayMessage;
		
		// prelogin vars
		GenList<preLogStruct> preLogins;
		preLogStruct plTemp;
		AIMUser lastLogin;

		// vars
		BDirectory *prefsDirectory;
		BString profile;
		BLocker lock;
};

//-----------------------------------------------------

#endif
