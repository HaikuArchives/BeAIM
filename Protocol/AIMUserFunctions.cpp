#include "AIMNetManager.h"
#include "AIMDataTypes.h"
#include "DLanguageClass.h"
#include "Globals.h"
#include "Say.h"

//-----------------------------------------------------

void AIMNetManager::DecodeBuddyStuff( DataContainer data, short& i, BMessage* values ) {

	char snLength;
	char* sn;
	unsigned short warningLevel, numTLVs, typeCode,
				   tlvLen, userClass, idleTime, temp;
	time_t memberSince = 0;
	uint32 sessionLen;
	int32 iTemp;
	DataContainer dTemp;

	struct tm* t_s; // to get localtime(time_t t)

	// Get the screen name
	snLength = data[i++];
	sn = new char[snLength+1];
	strncpy( sn, data.getrange(i,snLength).c_ptr(), snLength );
	sn[snLength] = '\0';
	i += snLength;
	if( values )
		values->AddString( "userid", sn );

	printf( "BUDDY_TLV... screen name (%d chars): %s\n", (int)snLength, sn );
	delete sn;

	// Get the sender's warning level
	warningLevel = (unsigned short)( rint( (double)GetWord(data,i) / 10.0 ) );
	i += 2;
	if( values )
		values->AddInt32( "warninglevel", (int32)warningLevel );

	printf( "BUDDY_TLV... warning level %d\n", warningLevel );

	// Get the number of TLV's in the packet
	numTLVs = (unsigned short)GetWord( data, i );
	i += 2;

	printf( "BUDDY_TLV... %d TLV's in this packet\n", numTLVs );

	// Go through and grab the rest of the TLV's
	for( unsigned short j = 0; j < numTLVs; ++j ) {

		// Grab the type code and length
		typeCode =	(unsigned short)GetWord( data, i );
		tlvLen = (unsigned short)GetWord( data, i+2 );
		i += 4;

		// we'll assume at first that the user is *not* away...
		values->AddBool( "away", false );

		// grab data based on the type code
		switch( typeCode ) {

			// classes... 0x10=free, 0x04=AOL... 0x11 is the "new" AIM
			case 0x01:
				userClass = (unsigned short)GetWord( data, i );
				i += 2;
				if( userClass == 0x10 )
					values->AddString( "userclass", Language.get("STAT_UC_INTERNET") );
				else if( values && userClass == 0x11 )
					values->AddString( "userclass", Language.get("STAT_UC_UNCONF_INTERNET") );
				else if( values && userClass == 0x04 )
					values->AddString( "userclass", Language.get("STAT_UC_AOL") );
				else if( userClass == 0x30 || userClass == 0x31 ) {
					values->AddString( "userclass", Language.get("STAT_UC_INTERNET") );
					values->ReplaceBool( "away", true );
				} else if( values )
					values->AddString( "userclass", Language.get("STAT_UC_UNKNOWN") );

				if( userClass == 0x10 || userClass == 0x11 )
					printf( "BUDDY_TLV... Class = Non-AOL (probably AIM)\n" );
				else if( userClass == 0x04 )
					printf( "BUDDY_TLV... Class = AOL user\n" );
				else if( userClass == 0x30 || userClass == 0x31 )
					printf( "BUDDY_TLV... Class = Away??\n" );
				else
					printf( "BUDDY_TLV... Class = Huh? (unknown, val=0x%X)\n", userClass );
				break;

			case 0x02:	// member since date
				temp = (unsigned short)GetWord( data, i );
				memberSince = (temp << 16) + (unsigned short)GetWord( data, i+2 );
				{DataContainer tempDC = data.getrange( i, 4 );
				printf( "membersince: " );
				Dump( tempDC );}
				i += 4;
				if( values )
					values->AddData( "membersince", B_TIME_TYPE, (void*)&memberSince, sizeof(time_t) );

				t_s = localtime( &memberSince );
				printf( "BUDDY_TLV: Member since date: %s", asctime(t_s) );
				break;

			case 0x03:	// online since date
				printf( "BUDDY_TLV: Online since date (ignoring)\n" );
				i += 4;
				break;

			case 0x04:	// idle time
				idleTime = (unsigned short)GetWord( data, i );
				i += 2;
				if( values )
					values->AddInt32( "idletime", (int32)idleTime );

				printf( "BUDDY_TLV: Idle time: %u\n", idleTime );
				break;

			case 0x0d:
				dTemp = data.getrange( i, tlvLen );
				iTemp = GrabCapabilityStuff(dTemp);
				if( values )
					values->AddInt32( "capsmask", iTemp );
				printf( "BUDDY_TLV: Capabilities mask: %ld\n", iTemp );
				i += tlvLen;
				break;

			case 0x0f:		// session length (AIM)
			case 0x10:		// session length (AOL)
				temp = (unsigned short)GetWord( data, i );
				sessionLen = (temp << 16) + (unsigned short)GetWord( data, i+2 );
				i += 4;
				if( values )
					values->AddInt32( "sessionlen", (int32)sessionLen );
				printf( "BUDDY_TLV: Session Length (secs): %u\n", (unsigned)sessionLen );
				break;

			default:	// something unknown... just skip it
				printf( "BUDDY_TLV: Unknown TLV: type 0x%X length (%d)\n", typeCode, tlvLen );
				i += tlvLen;
		}
	}
}

//-----------------------------------------------------

void AIMNetManager::BuddyOnline( SNAC_Object& snac ) {

	short i = 0;
	BMessage* newBuddy = new BMessage( BEAIM_ONCOMING_BUDDY );

	// The buddy info comes at the beginnning... yank it out
	DecodeBuddyStuff( snac.data, i, newBuddy );

	// Send the message if the client is ready
	if( clientReady )
		PostAppMessage( newBuddy );
	else
		queuedClientMessages.Enqueue( newBuddy );
}

//-----------------------------------------------------

void AIMNetManager::BuddyOffline( SNAC_Object& snac ) {

	char name[DISPLAY_NAME_MAX];
	BMessage* oldBuddy = new BMessage( BEAIM_OFFGOING_BUDDY );
	char len;

	// get the screen name
	len = snac.data[0];
	printf( "logoff name len: %d\n", int(len) );

	snac.data << char(0);
	strcpy( name, snac.data.c_ptr() + 1 );
	printf( "offgoing name: %s\n", name );

	oldBuddy->AddString( "userid", name );
	PostAppMessage( oldBuddy );
}

//-----------------------------------------------------

// Requests the info and profile for a member
void AIMNetManager::RequestPersonInfo( BMessage* msg ) {

	printf( "requesting person info... %s\n", msg->FindString("userid") );

	// Make the SNAC object
	SNAC_Object snac( 0x02, 0x05, 0, 0, requestID );
	snac.data << ToAIMWord(0x0001)
			  << char(strlen(msg->FindString("userid")))
			  << msg->FindString("userid");

	// add an object to the pending list
	userRequestID rid;
	rid.userid = AIMUser( msg->FindString("userid") );
	rid.reqID = requestID++;
	rid.type = RID_GET_INFO;
	pendingRequests.Add( rid );

	// and send it!
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

// Requests the info and profile for a member
void AIMNetManager::RequestAwayInfo( BMessage* msg ) {

	printf( "requesting away info... %s\n", msg->FindString("userid") );

	// Make the SNAC object
	SNAC_Object snac( 0x02, 0x05, 0, 0, requestID );
	snac.data << ToAIMWord(0x03)
			  << char(strlen(msg->FindString("userid")))
			  << msg->FindString("userid");

	// add an object to the pending list
	userRequestID rid;
	rid.userid = AIMUser( msg->FindString("userid") );
	rid.reqID = requestID++;
	rid.type = RID_GET_AWAY_MSG;
	pendingRequests.Add( rid );

	// and send it!
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

// Requests the info and profile for a member
void AIMNetManager::ReceivePersonInfo( SNAC_Object& snac ) {

	// vars
	BMessage* incomingInfo = new BMessage(BEAIM_UPDATE_INFO);
	unsigned short type, length;
	DataContainer temp;
	short i = 0;

	// Grab the other person's info (from the beginning)
	DecodeBuddyStuff( snac.data, i, incomingInfo );

	// get the encoding value and the profile (if they are there)
	if( i < snac.data.length() ) {

		// There should be two TLV's in here...
		for( int j = 0; j < 2; ++j ) {

			// get the type and the length
			type = (unsigned short)GetWord( snac.data, i );
			length = (unsigned short)GetWord( snac.data, i+2 );
			i += 4;

			// get the data
			temp = snac.data.getrange( i, length );
			temp << char(0);
			i += length;

			// the profile encoding
			if( type == 0x01 ) {
				printf( "> Profile encoding:\n%s\n", temp.c_ptr() );
				incomingInfo->AddString( "encoding", temp.c_ptr() );
			}

			// the profile itself
			else if( type == 0x02 ) {
				printf( "> Profile:\n%s\n", temp.c_ptr() );
				incomingInfo->AddString( "profile", temp.c_ptr() );
			}

			// away encoding?
			else if( type == 0x03 ) {
				printf( "> Away Encoding:\n%s\n", temp.c_ptr() );
				incomingInfo->AddString( "encoding", temp.c_ptr() );
			}

			// away?
			else if( type == 0x04 ) {
				printf( "> Away Message:\n%s\n", temp.c_ptr() );
				incomingInfo->AddString( "away_message", temp.c_ptr() );
			}
		}
	}

	// Find this request in the pending list and delete it
	userRequestID btemp;
	int curPos = 0;
	bool keepGoing = pendingRequests.First(btemp);
	while( keepGoing ) {
		if( snac.GetRequestID() == btemp.reqID ) {
			pendingRequests.Delete(curPos);
			printf( "--- Found and Nuked ReqID %d ---\n", btemp.reqID );
			if( btemp.type == RID_GET_AWAY_MSG )
				incomingInfo->what = BEAIM_UPDATE_AWAY_INFO;
			break;
		}
		keepGoing = pendingRequests.Next(btemp);
		++curPos;
	}

	// bombs away...
	incomingInfo->AddInt32( "wtype", (int32)USER_INFO_TYPE );
	PostAppMessage( incomingInfo );
}

//-----------------------------------------------------

void AIMNetManager::UpdateServerBuddyList( BMessage* msg ) {

	// Set up a SNAC object, with the correct fields to ADD (not remove) a person
	SNAC_Object snac( 0x03, 0x04, 0, 0, 0x1015 );

	// other vars we're gonna need (mostly to parse the list)
	int l = 1;
	char* i = (char*)msg->FindString("list");
	char* q = i;
	char name[50];

	if( msg->FindBool("add") )
		printf( "ADDING to server buddy list\n" );
	else
		printf( "REMOVING from server buddy list\n" );
	printf( "list: %s\n", i );


	// are we adding to or removing from the server's buddy list?
	bool add = msg->FindBool("add");

	// the add command apparently has three sets of zeroes before the name list
	if( add )
		snac.data << ToAIMWord(0x00) << ToAIMWord(0x00) << ToAIMWord(0x00);

	// Go through and pull out all the names
	if( i ) {
		while( *i ) {
			if( *i == 9 ) {
				strncpy( name, q, l );
				name[l-1] = '\0';
				if( add ) snac.data << char(l-1) << name;
				else SendRemovePersonMessage( name, int(l-1) );
				l = 0;
				q = i+1;
			}
			++i; ++l;
		}
		if( i != q ) {
			if( add ) snac.data << char(l-1) << q;
			else SendRemovePersonMessage( q, int(l-1) );
		}
	}

	// if we are adding screen names, go ahead and send them
	if( add )
		SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

void AIMNetManager::SendRemovePersonMessage( AIMUser name, int len ) {

	printf( "REMOVINK: %s\n", name.UserString() );

	// Set up a SNAC object, with the correct fields to remove a person
	SNAC_Object snac( 0x03, 0x05, 0, 0, 0x1015 );
	snac.data << ToAIMWord(0x00) << ToAIMWord(0x00) << ToAIMWord(0x00);
	snac.data << char(len) << name.UserString();
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

void AIMNetManager::FailedPersonInfo( SNAC_Object& snac ) {

	printf( "Failed packet:\n" );

	// Find this request in the pending list and delete it
	userRequestID btemp;
	int curPos = 0;
	bool keepGoing = pendingRequests.First(btemp);
	long ctemp = snac.GetRequestID();
	while( keepGoing ) {
		if( btemp.reqID == (unsigned)ctemp ) {
			pendingRequests.Delete(curPos);
			break;
		}
		keepGoing = pendingRequests.Next(btemp);
		++curPos;
	}

	// OK... now fire off a failure message
	if( keepGoing ) {
		BMessage* incomingInfo = new BMessage(BEAIM_UPDATE_INFO);
		incomingInfo->AddInt32( "wtype", (int32)USER_INFO_TYPE );
		incomingInfo->AddString( "userid", (char*)btemp.userid.UserString() );

		// build an error message to send off
		BString errorMsg = "<b>";
		errorMsg.Append( Language.get("ERROR_LABEL") );
		errorMsg.Append( ":</b> " );
		if( users->IsUserBlocked(btemp.userid) )
			errorMsg.Append( Language.get("IM_ERR_BLOCKED") );
		else
			errorMsg.Append( Language.get("IM_ERR_INVALID") );

		// send it out
		incomingInfo->AddString( "profile", errorMsg.String() );
		PostAppMessage( incomingInfo );
	}
}

//-----------------------------------------------------

void AIMNetManager::SetUserBlockiness( BMessage* msg, bool all ) {
	BString theUser;
	theUser = all ? "" : msg->FindString( "userid" );
	SNAC_Object sendMsg;

	printf( "Setting user blockiness for %s: %s\n", all ? "[all]" : theUser.String(), (all || msg->FindBool("block")) ? "yup" : "nope" );

	// set the correct SNAC info
	if( all || msg->FindBool("block") )
		sendMsg.Update( 0x09, 0x07, 0x00, 0x00, 0x00 );
	else
		sendMsg.Update( 0x09, 0x08, 0x00, 0x00, 0x00 );

	// finally, tack on the screen name(s)
	if( !all ) {
		sendMsg.data << char(theUser.Length())						// sn length
					 << (char*)theUser.String();						// sn
	} else {
		AIMUser tempName;
		bool kg = users->GetNextBlockedUser( tempName, true );
		while( kg ) {
			sendMsg.data << char(tempName.Username().Length())				// sn length
						 << (char*)tempName.UserString();					// sn
			kg = users->GetNextBlockedUser( tempName, false );
		}
	}

	// Now that we've gone to all the trouble of making that, send it
	SendSNACPacket( mainNetlet, sendMsg );
}

//-----------------------------------------------------

void AIMNetManager::SetClientReady() {
	clientReady = true;
	BMessage* tmpMsg;

	// now send off all the messages that are queued up
	while( queuedClientMessages.Dequeue(tmpMsg) )
		PostAppMessage( tmpMsg );
}

//-----------------------------------------------------

void AIMNetManager::SSIAddUser(AIMUser user, BString group)
{
	short g;
	BString grp;
	bool boom = true;
	SNAC_Object snac;
	printf("SSI Add User\n");
	while (users->GetGroups(grp, boom, NULL, &g)) {
		boom = false;
		printf("%s->%d\n", grp.String(), g);
		if (group == grp)
			break;
	}
	snac.Update(0x13, 0x11, 0x00, 0x00, 0x1900);
	SendSNACPacket(mainNetlet, snac);
	printf("%d/%d", user.SSIUserID(), g);
	snac.Update(0x13, 0x08, 0x00, 0x00, 0x1903);
	snac.data << ToAIMWord(strlen(user.UserString()));
	snac.data << user.UserString() << ToAIMWord(g);
	snac.data << ToAIMWord(user.SSIUserID()) << ToAIMWord(0x0000);
	snac.data << ToAIMWord(0x0000);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x12, 0x00, 0x00, 0x1904);
	SendSNACPacket(mainNetlet, snac);
}

void AIMNetManager::SSIAddGroup(BString group, short gid)
{
	printf("SSI Add Group\n");
	SNAC_Object snac;
	snac.Update(0x13, 0x11, 0x00, 0x00, 0x1901);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x08, 0x00, 0x00, 0x1903);
	snac.data << ToAIMWord(group.Length());
	snac.data << group.String() << ToAIMWord(gid);
	snac.data << ToAIMWord(0x0000) << ToAIMWord(0x0001);
	snac.data << ToAIMWord(0x0000);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x12, 0x00, 0x00, 0x1904);
	SendSNACPacket(mainNetlet, snac);
}

void AIMNetManager::SSIRemoveUser(AIMUser user)
{
	short u, g;
	AIMUser usr;
	bool boom = true;
	printf("SSI Remove User\n");
	printf("%d\n", user.SSIUserID());
	SNAC_Object snac;
//users->IsABuddy(user, &u, &g);
//		return;
	while (users->GetAllBuddies(usr, boom, &u, &g)) {
		boom = false;
		printf("u->%s = %s\n", usr.UserString(), user.UserString());
		if (user == usr)
			break;
		else
			u = 0;
	}
	snac.Update(0x13, 0x11, 0x00, 0x00, 0x1902);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x0a, 0x00, 0x00, 0x1903);
	printf("%d/%d", u, g);
	snac.data << ToAIMWord(strlen(user.UserString()));
	snac.data << user.UserString() << ToAIMWord(g);
	snac.data << ToAIMWord(u) << ToAIMWord(0x0000);
	snac.data << ToAIMWord(0x0000);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x12, 0x00, 0x00, 0x1904);
	SendSNACPacket(mainNetlet, snac);
}

void AIMNetManager::SSIRemoveGroup(BString group)
{
	printf("SSI Remove Group\n");
	BString grp;
	short g;
	bool boom = true;
	SNAC_Object snac;
	while (users->GetGroups(grp, boom, NULL, &g)) {
		boom = false;
		printf("%s->%d\n", grp.String(), g);
		if (group == grp)
			break;
		else
			g = 0;
	}
	snac.Update(0x13, 0x11, 0x00, 0x00, 0x1903);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x0A, 0x00, 0x00, 0x1903);
	snac.data << ToAIMWord(group.Length());
	snac.data << group.String() << ToAIMWord(g);
	snac.data << ToAIMWord(g) << ToAIMWord(0x0001);
	snac.data << ToAIMWord(0x0000);
	SendSNACPacket(mainNetlet, snac);
	snac.Update(0x13, 0x12, 0x00, 0x00, 0x1904);
	SendSNACPacket(mainNetlet, snac);
}
