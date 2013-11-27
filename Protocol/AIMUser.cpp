#include <ctype.h>
#include "AIMUser.h"
#include "AIMConstants.h"
#include "AIMNetManager.h"
#include "UserManager.h"
#include "Globals.h"
#include "Say.h"

//-----------------------------------------------------

AIMUser::AIMUser() {
	uid = gid = 0;
}

//-----------------------------------------------------

AIMUser::AIMUser( BString user ) {
	SetUsername(user);
	uid = 0;
	gid = 0;
}

//-----------------------------------------------------

AIMUser::AIMUser( const char* user ) {
	SetUsername(BString(user));
	uid = 0;
	gid = 0;
}

//-----------------------------------------------------

AIMUser::AIMUser( const AIMUser& id ) {
	SetUsername( id.userName );
	uid = id.uid;
	gid = id.gid;
}

//-----------------------------------------------------

AIMUser::~AIMUser() {
}

//-----------------------------------------------------

AIMUser* AIMUser::operator=( const AIMUser& id ) {

	SetUsername( id.userName );
	uid = gid = 0;
	return this;
}

//-----------------------------------------------------

bool AIMUser::operator==( const AIMUser& id ) {
	if( compare(id) == 0 )
		return true;
	return false;
}

//-----------------------------------------------------

bool AIMUser::operator!=( const AIMUser& id ) {
	if( compare(id) != 0 )
		return true;
	return false;
}

//-----------------------------------------------------

bool AIMUser::operator<( const AIMUser& id ) {
	if( compare(id) == -1 )
		return true;
	return false;
}

//-----------------------------------------------------

bool AIMUser::operator>( const AIMUser& id ) {
	if( compare(id) == 1 )
		return true;
	return false;
}

//-----------------------------------------------------

int AIMUser::compare( const AIMUser& id ) {

	BString us, them;
	char* a;
	char* b;

	us = userName;
	them = id.userName;

	a = (char*)us.String();
	b = (char*)them.String();

	// now, do the comparison
	while( *a && *a == ' ' ) ++a;
	while( *b && *b == ' ' ) ++b;
	if( !(*a) && !(*b) )
		return 0;
	if( !(*a) || !(*b) ) {
		if( !(*a) )
			return -1;
		else
			return 1;
	}

	while( *a && *b ) {
		while( *a && *a == ' ' ) ++a;
		while( *b && *b == ' ' ) ++b;
		if( !(*a) && !(*b) )
			return 0;
		if( !(*a) || !(*b) ) {
			if( !(*a) )
				return -1;
			else
				return 1;
		}
		if( tolower(*a) != tolower(*b) ) {
			if( tolower(*a) < tolower(*b) )
				return -1;
			else
				return 1;
		}
		++a;
		++b;
	}

	if( !(*a) && !(*b) )
		return 0;
	if( !(*a) || !(*b) ) {
		if( !(*a) )
			return -1;
		else
			return 1;
	}
	return 0;
}

//-----------------------------------------------------

BString AIMUser::Username() {
	return userName;
}

//-----------------------------------------------------

const char* AIMUser::UserString() {
	return userName.String();
}

//-----------------------------------------------------

void AIMUser::SetUsername( BString user ) {
	userName = user;
}

//-----------------------------------------------------

void AIMUser::SetSSIUserID(short id)
{
	uid = id;
}

//-----------------------------------------------------

void AIMUser::SetSSIGroupID(short id)
{
	gid = id;
}

//-----------------------------------------------------

short AIMUser::SSIUserID(void)
{
	short userid, groupid;
	if ((uid == 0) && (users)) {
		if (users->GetBuddySSIInfo(*this, userid, groupid)) {
			uid = userid;
			gid = groupid;
		}
	}
	return uid;
}

//-----------------------------------------------------

short AIMUser::SSIGroupID(void)
{
	short groupid, userid;
	if ((gid == 0) && (users)) {
		if (users->GetBuddySSIInfo(*this, userid, groupid))
			gid = groupid;
	}
	return gid;
}

//-----------------------------------------------------

bool AIMUser::ExactCompare( const AIMUser& id ) {

	if( !userName.Compare(id.userName) )
		return true;
	return false;
}
	
//-----------------------------------------------------
		
bool AIMUser::IsValid() {

	BString tempName = userName;

	// strip out all the spaces and make sure it's not too long
	tempName.RemoveSet(" ");
	if( tempName.Length() > MAX_SCREEN_NAME_SIZE )
		return false;
		
	// does it begin with a letter?
	if( !isalpha(tempName[0]) )
		return false;
		
	// remove all alphabetic and numberic characters... if there
	// is anything left, then it's not valid
	tempName.RemoveSet( "abcdefghijklmnopqrstuvwxyz" );
	tempName.RemoveSet( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	tempName.RemoveSet( "0123456789" );
	if( tempName.Length() )
		return false;
		
	return false;
}

//-----------------------------------------------------

bool AIMUser::IsEmpty() {

	// it's empty if there is nothing but whitespace in the name
	BString tempName = userName;
	tempName.RemoveSet(" ");
	return bool(tempName.Length() == 0);
}

//-----------------------------------------------------

