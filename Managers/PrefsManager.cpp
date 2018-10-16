#include "PrefsManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "PrefsManager.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "Say.h"
#include "xmlparse.h"

//-----------------------------------------------------

PreferencesManager::PreferencesManager() {

	status_t error;
	prefsDirectory = new BDirectory();

	// Try and open the BeAIM prefs folder, and create a new one if it doesn't exist
	error = prefsDirectory->SetTo( "/boot/home/config/settings/BeAIM" );
	if( error == B_ENTRY_NOT_FOUND )
		prefsDirectory->CreateDirectory( "/boot/home/config/settings/BeAIM", prefsDirectory );

	readMode = 0;
	ReadGlobalFile();
}

//-----------------------------------------------------

PreferencesManager::~PreferencesManager() {

	WriteGlobalFile();
}

//-----------------------------------------------------

void PreferencesManager::Clear( bool global ) {

	// clear lots of stuff out
	profile = "";
	awayMessages.Clear();
	prefBool.Clear();
	prefInt.Clear();
	prefString.Clear();

	// if global, then clear even more stuff out
	if( global ) {
		gPrefBool.Clear();
		gPrefInt.Clear();
		gPrefString.Clear();
	}
}

//-----------------------------------------------------

void PreferencesManager::SetAwayMessage( BString name, BString oldname, BString message ) {

	int pos;

	awayStruct aw;
	aw.name = name;
	aw.message = message;

	// if there's an oldname, look for it
	if( oldname.Length() ) {
		pos = FindAwayMessage(oldname);
		if( pos != -1 ) {
			awayMessages[(unsigned)pos] = aw;

			// if this away message in is use, we have to modify it!
			if( client->AwayMode() == AM_STANDARD_MESSAGE && oldname == client->CurrentAwayMessageName() )
				client->SetAwayMode( AM_STANDARD_MESSAGE, name );
		} else
			awayMessages.Add(aw);
	} else
		awayMessages.Add(aw);
}

//-----------------------------------------------------

bool PreferencesManager::GetAwayMessage( BString name, BString& message ) {

	awayStruct aw;

	// search for the away message w/ that name
	bool kg = awayMessages.First(aw);
	while( kg && aw.name != name )
		kg = awayMessages.Next(aw);

	// bail if we can't find it
	if( !kg )
		return false;

	// return the string
	message = aw.message;
	return true;
}

//-----------------------------------------------------

int PreferencesManager::CountAwayMessages() {
	return awayMessages.Count();
}

//-----------------------------------------------------

bool PreferencesManager::GetFirstAwayMessage( awayStruct& aw ) {
	return awayMessages.First(aw);
}

//-----------------------------------------------------

bool PreferencesManager::GetNextAwayMessage( awayStruct& aw ) {
	return awayMessages.Next(aw);
}

//-----------------------------------------------------

int PreferencesManager::FindAwayMessage( BString name ) {

	for( int i = 0; i < (int)awayMessages.Count(); ++i )
		if( awayMessages[i].name == name )
			return i;
	return -1;
}

//-----------------------------------------------------

void PreferencesManager::RemoveAwayMessage( BString name ) {

	int pos = FindAwayMessage(name);
	if( pos != -1 )
		awayMessages.Delete(pos);
}

//-----------------------------------------------------

BString PreferencesManager::CustomAwayMessage() {
	return customAwayMessage;
}

//-----------------------------------------------------

void PreferencesManager::SetCustomAwayMessage( BString cam ) {
	customAwayMessage = cam;
}

//-----------------------------------------------------

void PreferencesManager::WriteAwayMessages( FILE* ptr ) {

	BString temp, temp2;
	awayStruct aw;
	bool kg;

	if( !awayMessages.Count() )
		return;

	// write out all the away messages
	fprintf( ptr, "	<awaymessages>\n" );
	kg = awayMessages.First(aw);
	while( kg ) {

		temp = aw.name;
		temp2 = aw.message;
		EntitizeNormalString(temp);
		EntitizeCDataString(temp2);

		fprintf( ptr, "		<message name=\"%s\"><![CDATA[%s]]></message>\n",
				 temp.String(), temp2.String() );
		kg = awayMessages.Next(aw);
	}
	fprintf( ptr, "	</awaymessages>\n" );

	fprintf( ptr, "	<customawaymessage><![CDATA[%s]]></customawaymessage>\n",
			 customAwayMessage.String() );
}

//-----------------------------------------------------

void PreferencesManager::WriteBuddyList( FILE* ptr ) {

	BString group;
	AIMUser user;
	bool kg = false, kg2 = false;
	uint32 enc;
	short gid;

	// write out the buddy list
	fprintf( ptr, "	<buddylist>\n" );

	kg = users->GetGroups(group, true);
	while( kg ) {

		users->GetGroupSSIInfo(group, gid);

		// write out a group
		fprintf( ptr, "		<group name=\"%s\" groupid=\"%d\">\n", group.String(), gid );
		printf("group %s\t%d\n", group.String(), gid);
		// go through each buddy in the group
		kg2 = users->GetGroupBuddies(user, group, true);
		while(kg2) {
			fprintf( ptr, "			<buddy name=\"%s\" userid=\"%d\" groupid=\"%d\"", user.UserString(), user.SSIUserID(), user.SSIGroupID() );
			if( (enc = users->GetBuddyEncoding(user)) != DEFAULT_ENCODING_CONSTANT )
				fprintf( ptr, " encoding=\"%lu\"", enc );
			fprintf( ptr, "></buddy>\n" );
			printf("%s\t\t%d\t%d\n", user.UserString(), user.SSIUserID(), user.SSIGroupID());
			//fprintf( ptr, "			</buddy>\n" );

			kg2 = users->GetGroupBuddies(user, group, false);
		}

		// write the end tag and get the next group (if any)
		fprintf( ptr, "		</group>\n" );
		kg = users->GetGroups(group, false);
	}

	// done!
	fprintf( ptr, "	</buddylist>\n" );
}

//-----------------------------------------------------

void PreferencesManager::WritePrefs( FILE* ptr ) {

	fprintf( ptr, "	<config>\n" );

	// print out the bools
	for( unsigned i = 0; i < prefBool.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"bool\" name=\"%s\" value=\"%s\"/>\n",
					prefBool[i].name.String(),
					prefBool[i].val ? "true" : "false" );
	}

	// print out the ints
	for( unsigned i = 0; i < prefInt.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"int32\" name=\"%s\" value=\"%ld\"/>\n",
					prefInt[i].name.String(),
					prefInt[i].val );
	}

	// print out the strings
	for( unsigned i = 0; i < prefString.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"string\" name=\"%s\" value=\"%s\"/>\n",
					prefString[i].name.String(),
					prefString[i].val.Length() ? prefString[i].val.String() : "" );
	}

	fprintf( ptr, "	</config>\n" );
}

//-----------------------------------------------------

void PreferencesManager::WriteUserFile( AIMUser user ) {

	if( !user.Username().Length() )
		return;
	lock.Lock();

	BString fName = GetUserFileName(user);
	if( beaimDebug )
		fName = fName.Append(".temp");
	//Say( fName );
	FILE* ptr = fopen( fName.String(), "w" );

	// print out some comments and stuff
	fprintf( ptr, "<!-- BeAIM XML user file for %s -->\n\n", user.UserString() );

	// print out the opening tag
	fprintf( ptr, "<beaim>\n" );

	// write out various bits o' data (order changed to make it work -- gile)
	WriteBuddyList( ptr );
	WriteBlockedUsers( ptr );
	WriteWindowPositions( ptr );
	WritePrefs( ptr );

	// print out the user's profile (if any)
	if( profile.Length() )
		fprintf( ptr, "	<profile><![CDATA[%s]]></profile>\n", profile.String() );

	// write out the away messages
	WriteAwayMessages( ptr );

	// now the closing tag
	fprintf( ptr, "</beaim>\n" );
	fclose(ptr);
	lock.Unlock();
}

//-----------------------------------------------------

BString PreferencesManager::GetUserFileName( AIMUser user ) {

	// remove all spaces and change the letters to lowercase
	BString fName = user.Username();
	fName.ToLower();
	fName.RemoveSet( " " );
	fName = fName.Prepend( "/boot/home/config/settings/BeAIM/xu-" );
	return fName;
}

//-----------------------------------------------------

void PreferencesManager::EntitizeNormalString( BString& str ) {

	str = str.ReplaceAll( "&", "&amp;" );
	str = str.ReplaceAll( "<", "&lt;" );
	str = str.ReplaceAll( ">", "&gt;" );
	str = str.ReplaceAll( "'", "&apos;" );
	str = str.ReplaceAll( "\"", "&quot;" );
}

//-----------------------------------------------------

void PreferencesManager::UnEntitizeNormalString( BString& str ) {

	str = str.ReplaceAll( "&amp;", "&" );
	str = str.ReplaceAll( "&lt;", "<" );
	str = str.ReplaceAll( "&gt;", ">" );
	str = str.ReplaceAll( "&apos;", "'" );
	str = str.ReplaceAll( "&quot;", "\"" );
}

//-----------------------------------------------------

void PreferencesManager::EntitizeCDataString( BString& str ) {
	str = str.ReplaceAll( "]]>", "]]&gt;" );
}

//-----------------------------------------------------

BString PreferencesManager::Profile() {
	return profile;
}

//-----------------------------------------------------

void PreferencesManager::SetProfile( BString pr ) {

	profile = pr;
	EntitizeCDataString(profile);

	// send the message
	BMessage* gMsg = new BMessage( BEAIM_GOING_AWAY );
	if( client->AwayMode() != AM_NOT_AWAY )
		gMsg->AddBool( "away", true );
	PostAppMessage( gMsg );
}

//-----------------------------------------------------

void PreferencesManager::ReadUserFile( AIMUser user ) {

	//Say( "read user file: ", (char*)user.UserString() );

	XML_Parser parser;
	char data[512];
	size_t len;
	bool done;

	// open the file
	readGlobal = false;
	BString fName = GetUserFileName(user);
	FILE* ptr = fopen( fName.String(), "r" );
	if( !ptr )
		return;

	printf( "reading XML: %s\n", fName.String() );

	// create the XML parser and set handlers
	parser = XML_ParserCreate(NULL);
	XML_SetUserData( parser, this );
	XML_SetElementHandler( parser, startElement, endElement );
	XML_SetCharacterDataHandler( parser, characterData );

	do {
		len = fread( data, 1, 512, ptr );
		done = bool(len < 512);

		if (!XML_Parse(parser, data, len, done)) {
			fprintf(stderr,
				"%s at line %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser) );
			return;
		}
	} while( !done );

	// clean up
	XML_ParserFree(parser);
	fclose(ptr);
}

//-----------------------------------------------------

void PreferencesManager::startElement( void *userData, const char *name, const char **atts ) {

	PreferencesManager* theClass = (PreferencesManager*)userData;
	int i;

	// is it a userdata tag?
	if( strcasecmp(name,"userdata") == 0 ) {
		theClass->plTemp.name = atts[1];
		theClass->plTemp.savePassword = false;
		theClass->plTemp.autoLogin = false;
	}

	// is it the savepassword tag?
	else if( strcasecmp(name,"savepassword") == 0 )
		theClass->plTemp.savePassword = true;

	// is it the password itself?
	else if( strcasecmp(name,"password") == 0 )
		theClass->plTemp.password = BString(atts[1]);

	// is it the autologin tag?
	else if( strcasecmp(name,"autologin") == 0 )
		theClass->plTemp.autoLogin = true;

	// is it the "last" tag?
	else if( strcasecmp(name,"last") == 0 )
		theClass->lastLogin = theClass->plTemp.name;

	// is it a profile?
	else if( strcasecmp(name,"profile") == 0 )
		theClass->readMode = 1;

	// is it a buddylist?
	else if( strcasecmp(name,"buddylist") == 0 ) {
		theClass->readMode = 2;
	}

	// is it a group?
	else if( strcasecmp(name,"group") == 0 ) {
		short gid = 0;
		theClass->rGroup = atts[1];
		theClass->readMode = 3;
		for (i = 2; atts[i]; i++) {
			if (!strcasecmp(atts[i], "groupid"))
				gid = (short) atoi(atts[++i]);
		}
		users->AddGroup( BString(atts[1]), -1, gid );
		printf("group %s, gid %d\n", atts[1], gid);
	}

	// is it a buddy?
	else if( strcasecmp(name,"buddy") == 0 ) {
		theClass->rBuddy = atts[1];
		uint32 enc = 0;
		AIMUser wtf = atts[1];
		for (i = 2; atts[i]; i++) {
			if (!strcasecmp(atts[i], "encoding"))
				enc = (uint32)atoi(atts[++i]);
			else if (!strcasecmp(atts[i], "groupid"))
				wtf.SetSSIGroupID((short) atoi(atts[++i]));
			else if (!strcasecmp(atts[i], "userid"))
				wtf.SetSSIUserID((short) atoi(atts[++i]));
		}
		users->AddBuddy( wtf, theClass->rGroup, false );
		if (enc)
			users->SetBuddyEncoding(wtf, enc);
		theClass->readMode = 4;
		printf("user %s, uid %d, gid %d\n", wtf.UserString(), wtf.SSIUserID(), wtf.SSIGroupID());
	}

	// is it the list of away messages?
	else if( strcasecmp(name,"awaymessages") == 0 )
		theClass->readMode = 5;

	// is it an away message?
	else if( strcasecmp(name,"message") == 0 ) {
		theClass->rAwayName = atts[1];
		theClass->UnEntitizeNormalString( theClass->rAwayName );
		theClass->rAwayMessage = "";
		theClass->readMode = 6;
	}

	// is it a custom away message?
	else if( strcasecmp(name,"customawaymessage") == 0 ) {
		theClass->customAwayMessage = "";
		theClass->readMode = 19;
	}

	// is it an option (pref setting)?
	else if( strcasecmp(name,"option") == 0 ) {
		theClass->ReadOption( name, atts );
	}

	// is it a blocked user?
	else if( strcasecmp(name,"blockeduser") == 0 ) {
		users->BlockUser( AIMUser(atts[1]), false );
	}

	// is it a saved window position?
	else if( strcasecmp(name,"userwinpos") == 0 ) {
		float left, top, right, bottom, div;
		theClass->wpRect.user = BString(atts[1]);
		sscanf( atts[3], "%f", &left );
		sscanf( atts[5], "%f", &top );
		sscanf( atts[7], "%f", &right );
		sscanf( atts[9], "%f", &bottom );
		sscanf( atts[11], "%f", &div );
		theClass->wpRect.frame.Set( left, top, right, bottom );
		theClass->wpRect.divider = div;
		windows->SetWindowPos( theClass->wpRect );
	}
}

//-----------------------------------------------------

void PreferencesManager::endElement( void *userData, const char *name ) {

	PreferencesManager* theClass = (PreferencesManager*)userData;

	// done reading in user data
	if( strcasecmp(name, "userdata") == 0 ) {
		theClass->preLogins.Add( theClass->plTemp );
	}

	// done reading in buddies
	if( strcasecmp(name, "buddylist") == 0 ) {
		BMessage* msg = new BMessage(BEAIM_RELOAD_BUDDYLIST);
		msg->AddBool( "noappsend", true );
		users->ClearCommitList();
		PostAppMessage( msg );
	}

	else if( theClass->readMode == 1 )
		theClass->readMode = 0;

	else if( theClass->readMode == 6 ) {
		awayStruct aw;
		aw.name = theClass->rAwayName;
		aw.message = theClass->rAwayMessage;
		theClass->awayMessages.Add( aw );
		theClass->readMode = 0;
	}

	else if( theClass->readMode == 19 ) {
		theClass->readMode = 0;
	}
}

//-----------------------------------------------------

void PreferencesManager::characterData( void *userData, const char *data, int len ) {

	PreferencesManager* theClass = (PreferencesManager*)userData;
	BString temp = "";

	if( theClass->readMode == 1 ) {
		theClass->profile = theClass->profile.Append( data, len );
	}

	else if( theClass->readMode == 6 ) {
		theClass->rAwayMessage = theClass->rAwayMessage.Append( data, len );
	}

	else if( theClass->readMode == 19 ) {
		theClass->customAwayMessage = theClass->customAwayMessage.Append( data, len );
	}
}

//-----------------------------------------------------

void PreferencesManager::ReadOption( const char *name, const char **atts ) {

	// there must be a type and value
	if( !atts[0] || !atts[1] || !atts[2] || !atts[3] || !atts[4] || !atts[5] )
		return;
	if(  strcasecmp(atts[0],"type") != 0 ||
		 strcasecmp(atts[2],"name") != 0 ||
		 strcasecmp(atts[4],"value") != 0 )
		return;

	// it's a boolean option
	if( strcasecmp(atts[1], "bool") == 0 ) {
		if (!strcasecmp(atts[5], "true"))
			WriteBool(atts[3], true, readGlobal);
		else // default to false
			WriteBool(atts[3], false, readGlobal);
	}
	// it's an int32 option
	else if( strcasecmp(atts[1], "int32") == 0 )
		WriteInt32(atts[3], atoi(atts[5]), readGlobal);
	// it's a string option
	else if( strcasecmp(atts[1], "string") == 0 )
		WriteString(atts[3], atts[5], readGlobal);
}

//-----------------------------------------------------

void PreferencesManager::WriteBool( BString name, bool value, bool global ) {

	GenList<prefBoolStruct>* list = global ? &gPrefBool : &prefBool;
	prefBoolStruct addStruct;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name ) {
			(*list)[i].val = value;
			return;
		}
	}

	addStruct.name = name;
	addStruct.val = value;
	list->Add( addStruct );
}

//-----------------------------------------------------

bool PreferencesManager::ReadBool( BString name, bool def, bool global ) {

	bool ret = def;
	GenList<prefBoolStruct>* list = global ? &gPrefBool : &prefBool;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name )
			ret = (*list)[i].val;
	}
	return ret;
}

//-----------------------------------------------------

void PreferencesManager::WriteInt32( BString name, int32 value, bool global ) {

	GenList<prefInt32Struct>* list = global ? &gPrefInt : &prefInt;
	prefInt32Struct addStruct;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name ) {
			(*list)[i].val = value;
			return;
		}
	}

	addStruct.name = name;
	addStruct.val = value;
	list->Add( addStruct );
}

//-----------------------------------------------------

int32 PreferencesManager::ReadInt32( BString name, int32 def, bool global ) {

	int32 ret = def;
	GenList<prefInt32Struct>* list = global ? &gPrefInt : &prefInt;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name )
			ret = (*list)[i].val;
	}
	return ret;
}

//-----------------------------------------------------

void PreferencesManager::WriteString( BString name, BString value, bool global ) {

	GenList<prefStringStruct>* list = global ? &gPrefString : &prefString;
	prefStringStruct addStruct;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name ) {
			(*list)[i].val = value;
			return;
		}
	}

	addStruct.name = name;
	addStruct.val = value;
	list->Add( addStruct );
}

//-----------------------------------------------------

void PreferencesManager::ReadString( BString name, BString& buf, BString def, bool global, bool canBeEmpty ) {

	buf = def;
	GenList<prefStringStruct>* list = global ? &gPrefString : &prefString;

	for( unsigned i = 0; i < list->Count(); ++i ) {
		if( (*list)[i].name == name )
			buf = (*list)[i].val;
	}

	if( !buf.Length() && !canBeEmpty )
		buf = def;
}

//-----------------------------------------------------

AIMUser PreferencesManager::LastLogin() {
	return lastLogin;
}

//-----------------------------------------------------

void PreferencesManager::SetLastLogin( AIMUser LL ) {
	lastLogin = LL;
}

//-----------------------------------------------------

void PreferencesManager::WritePreLoginData( FILE* ptr ) {

	preLogStruct pl;
	bool kg;

	fprintf( ptr, "	<prelogin>\n" );

	kg = preLogins.First(pl);
	while( kg ) {
		fprintf( ptr, "		<userdata name=\"%s\">\n", pl.name.UserString() );
		if( pl.savePassword ) {
			fprintf( ptr, "			<savepassword/>\n" );
			fprintf( ptr, "			<password value=\"%s\"/>\n", pl.password.String() );
		}
		if( pl.autoLogin )
			fprintf( ptr, "			<autologin/>\n" );
		if( lastLogin == pl.name )
			fprintf( ptr, "			<last/>\n" );
		fprintf( ptr, "		</userdata>\n" );
		kg = preLogins.Next(pl);
	}
	fprintf( ptr, "	</prelogin>\n" );
}

//-----------------------------------------------------

void PreferencesManager::WriteGlobalFile() {

	BString globalString;
	FILE* ptr;

	globalString = "/boot/home/config/settings/BeAIM/";
	globalString.Append( beaimDebug ? "dglobal" : "global" );
	ptr = fopen( globalString.String(), "w" );
	if( !ptr )
		return;

	// print out some comments and stuff
	fprintf( ptr, "<!-- BeAIM global XML preferences file -->\n\n" );
	fprintf( ptr, "<beaim>\n" );

	// write out the prelogin data
	WritePreLoginData( ptr );

	// write the config stuff
	fprintf( ptr, "	<config>\n" );

	// print out the bools
	for( unsigned i = 0; i < gPrefBool.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"bool\" name=\"%s\" value=\"%s\"/>\n",
					gPrefBool[i].name.String(),
					gPrefBool[i].val ? "true" : "false" );
	}

	// print out the ints
	for( unsigned i = 0; i < gPrefInt.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"int32\" name=\"%s\" value=\"%ld\"/>\n",
					gPrefInt[i].name.String(),
					gPrefInt[i].val );
	}

	// print out the strings
	for( unsigned i = 0; i < gPrefString.Count(); ++i ) {
		fprintf( ptr, "		<option type=\"string\" name=\"%s\" value=\"%s\"/>\n",
					gPrefString[i].name.String(),
					gPrefString[i].val.Length() ? gPrefString[i].val.String() : "" );
	}

	fprintf( ptr, "	</config>\n" );
	fprintf( ptr, "</beaim>\n" );

	fclose(ptr);
}

//-----------------------------------------------------

void PreferencesManager::ReadGlobalFile() {

	BString globalString;
	XML_Parser parser;
	char data[1024];
	size_t len;
	bool done;

	globalString = "/boot/home/config/settings/BeAIM/";
	globalString.Append( beaimDebug ? "dglobal" : "global" );

	// open the file
	readGlobal = true;
	FILE* ptr = fopen( globalString.String(), "r" );
	if( !ptr )
		return;

	printf( "Reading XML: Global prefs\n" );

	// create the XML parser and set handlers
	parser = XML_ParserCreate(NULL);
	XML_SetUserData( parser, this );
	XML_SetElementHandler( parser, startElement, endElement );
	XML_SetCharacterDataHandler( parser, characterData );

	do {
		len = fread( data, 1, 1024, ptr );
		done = bool(len < 1024);

		if (!XML_Parse(parser, data, len, done)) {
			fprintf(stderr,
				"%s at line %d\n",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser) );
			return;
		}

	} while( !done );

	// clean up
	XML_ParserFree(parser);
	fclose(ptr);
}

//-----------------------------------------------------

bool PreferencesManager::GetNextUser( AIMUser& user, bool first ) {

	bool retval;
	preLogStruct pl;

	retval = first ? preLogins.First(pl) : preLogins.Next(pl);
	if( retval )
		user = pl.name;

	return retval;
}

//-----------------------------------------------------

void PreferencesManager::GetUserLoginParameters( AIMUser user, BString& pw, bool& al, bool& sp ) {

	preLogStruct pl;
	bool kg;

	// first, set some defaults
	pw = "";
	sp = false;
	al = false;

	// now find and read the real values
	kg = preLogins.First(pl);
	while( kg ) {
		if( user == pl.name )
		{
			pw = pl.password;
			sp = pl.savePassword;
			al = pl.autoLogin;
			return;
		}
		kg = preLogins.Next(pl);
	}
}

//-----------------------------------------------------

void PreferencesManager::SaveLoginParameters( AIMUser user, BString pw, bool al, bool sp, bool addIfNeeded ) {

	preLogStruct pl, pl2;

	// first, setup the preLogStruct
	pl.name = user;
	pl.password = sp ? pw : BString();
	pl.savePassword = sp;
	pl.autoLogin = al;

	// now look for the appropriate index to save it in
	for( unsigned index = 0; index < preLogins.Count(); ++index ) {
		pl2 = preLogins[index];
		if( user == pl2.name )
		{
			preLogins[index] = pl;
			return;
		}
	}

	// didn't find it... add it if needed
	if( addIfNeeded )
		preLogins.Add( pl );
}

//-----------------------------------------------------

void PreferencesManager::WriteBlockedUsers( FILE* ptr ) {
	AIMUser name;
	bool kg;

	// see if there are any blocked users
	kg = users->GetNextBlockedUser( name, true );
	if( !kg )
		return;

	// write 'em all out
 	fprintf( ptr, "	<blocklist>\n" );
	while( kg ) {
		fprintf( ptr, "		<blockeduser name=\"%s\"/>\n", name.UserString() );
		kg = users->GetNextBlockedUser( name, false );
	}
	fprintf( ptr, "	</blocklist>\n" );
}

//-----------------------------------------------------

void PreferencesManager::WriteWindowPositions( FILE* ptr ) {
	winPosRect wp;
	int i = 0;
	bool kg;

	kg = windows->GetNextWindowPos( wp, true );
	if( !kg )
		return;

	// go through and write out all the window positions
	fprintf( ptr, "	<windowpositions>\n" );
	while( kg ) {

		// write out the current window position
		fprintf( ptr, "		<userwinpos name=\"%s\" ", wp.user.UserString() );
		fprintf( ptr, "x1=\"%ld\" y1=\"%ld\" ", int32(wp.frame.LeftTop().x), int32(wp.frame.LeftTop().y) );
		fprintf( ptr, "x2=\"%ld\" y2=\"%ld\" ", int32(wp.frame.RightBottom().x), int32(wp.frame.RightBottom().y) );
		fprintf( ptr, "div=\"%f\"/>\n", wp.divider );

		// write out 100 window positions at most... the most recently closed windows
		// are at the top of the list, so that should be plenty
		if( ++i >= 100 )
			break;

		// next!
		kg = windows->GetNextWindowPos( wp, false );
	}
	fprintf( ptr, "	</windowpositions>\n" );
}

//-----------------------------------------------------

