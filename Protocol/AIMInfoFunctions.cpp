#include "AIMNetManager.h"
#include "AIMConstants.h"
#include "Globals.h"

//-----------------------------------------------------

// Send message function
void AIMNetManager::SendMessage( BMessage* msg ) {

	char Message[2048];
	SNAC_Object sendMsg;
	bool isAutoResponse = msg->FindBool( "autorespond" );

	// Wrap the message in some generic HTML (AOL style)
	strcpy( Message, "<HTML><BODY BGCOLOR=\"#ffffff\">" );
	strcat( Message, msg->FindString("message") );
	strcat( Message, "</HTML>" );
	
	// Make the SNAC object for the message
	sendMsg.Update( 0x04, 0x06, 0x00, 0x00, requestID );
	sendMsg.data << ToAIMWord(0x00) << ToAIMWord(0x00)
				 << ToAIMWord(0x00) << ToAIMWord(0x00)
				 << ToAIMWord(0x01)
				 << char(strlen((char*)msg->FindString("send_to")))		// sn length
				 << (char*)msg->FindString("send_to");					// sn
				 
	// handle the auto-respond case and stuff
	if( isAutoResponse ) {
		sendMsg.data << ToAIMWord(0x04)
					 << ToAIMWord(0x00)
					 << ToAIMWord(0x02);
	} else
		sendMsg.data << ToAIMWord(0x02);
				 
	// finish up with the packet
	sendMsg.data << ToAIMWord(strlen(Message) + 0x0D)					// msg len + 0x0D
				 << char(0x05) << ToAIMWord(0x0100)
				 << ToAIMWord(0x0101) << ToAIMWord(0x0101)
				 << ToAIMWord(strlen(Message) + 0x04)					// msg len + 0x04
				 << ToAIMWord(0x00) << ToAIMWord(0x00)
				 << Message;
				 
	// log it, so we know what happened in case the message never gets there
	LogMessageReqID( BString((char*)msg->FindString("send_to")), RID_SEND_MESSAGE );

	// Now that we've gone to all the trouble of making that, send it
	SendSNACPacket( mainNetlet, sendMsg );
}

//-----------------------------------------------------

// Receive message function
void AIMNetManager::ReceiveMessage( SNAC_Object& snac ) {

	short i = 10;
	unsigned short temp, temp2;
	unsigned int languageValue;
	bool autoRespond = false;
	BMessage* incomingMsg = new BMessage(BEAIM_INCOMING_IM);
	DataContainer msg;
	
	// Grab the other person's info
	DecodeBuddyStuff( snac.data, i, incomingMsg );

	// This will be either 0x0002 or 0x0004 (autorespond mode?)
	temp = (unsigned short)GetWord( snac.data, i );
	i += 2;
	if( temp == 0x0004 ) {
		temp = (unsigned short)GetWord( snac.data, i );		// 0x0000
		temp = (unsigned short)GetWord( snac.data, i+2 );	// 0x0002
		autoRespond = true;
		i += 4;
	}

	if( autoRespond )
		printf( "AutoResponded IM...\n" );
	incomingMsg->AddBool( "autorespond", autoRespond );

	// Reality check
	if( temp != 0x0002 )
		printf( "RECV_MESSAGE ERROR: temp should be 0x0002, not 0x%X!\n", temp );
	
	// Get the message length (with 0x0D tacked on), and fixed value 0x0501
	temp = (unsigned short)GetWord( snac.data, i );
	temp = (unsigned short)GetWord( snac.data, i+2 );
	i += 4;
	
	// Another reality check
	if( temp != 0x0501 )
		printf( "RECV_MESSAGE ERROR: temp should be 0x0501, not 0x%X!\n", temp );
	
	// Grab the mysterious language value
	temp = (unsigned short)GetWord( snac.data, i );
	temp2 = (unsigned short)GetWord( snac.data, temp + i + 8 );
	languageValue = (temp2 << 16) + (unsigned short)GetWord( snac.data, temp + i + 6 );
	incomingMsg->AddInt32( "languagevalue", (int32)languageValue );
	i += 2;	
	
	printf( "Mysterious Language value: 0x%X\n", (unsigned)languageValue );
	
	// Get the message itself
	msg = snac.data.getrange( temp + i + 8 );
	msg << char(0);
	incomingMsg->AddString( "message", (char*)msg.c_ptr() );
	
	printf( "actual message: %s\n", msg.c_ptr() );
	
	// Send the message off its final resting place... the great bit-bucket in the sky
	incomingMsg->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
	PostAppMessage( incomingMsg );
}

//-----------------------------------------------------

void AIMNetManager::SearchByEmail( BMessage* msg ) {

	// calculate the next request ID
	emailSearchID = requestID++;

	// Set up a SNAC object, with the correct fields to search for an address
	SNAC_Object snac( 0x0a, 0x02, 0x00, 0x00, emailSearchID );
	snac.data << (char*)msg->FindString( "email" );
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

void AIMNetManager::ReceiveSearchResults( SNAC_Object& snac ) {

	DataContainer name;
	char actualName[50];	
	unsigned short length;
	BMessage* sndMessage;
	short i = 0;

	// is this the current search? If not, ignore it
	if( snac.GetRequestID() != emailSearchID )
		return;

	// go through and suck out all the search results
	while( (snac.data.length() - i) > 4 ) {

		// get 0x0001 (a constant), and the length of the screen name
		GetWord( snac.data, i );
		length = (unsigned short)GetWord( snac.data, i+2 );
		i += 4;
		
		printf( "SR---> SN length: %d chars\n", length );
		
		// get the screen name itself
		name = snac.data.getrange( i, length );
		strncpy( actualName, name.c_ptr(), length );
		actualName[length] = '\0';
		i += length;
		
		printf( "SR---> SN name: %s\n", name.c_ptr() );
		
		// send a message with this match
		sndMessage = new BMessage( BEAIM_EMAIL_SEARCH_RESULTS );
		sndMessage->AddString( "userid", actualName );
		PostAppMessage( sndMessage );
	}
}

//-----------------------------------------------------

void AIMNetManager::ReceiveFailedSearch( SNAC_Object& snac ) {

	// is this the current search? If not, ignore it
	if( snac.GetRequestID() != emailSearchID )
		return;
		
	// post a message saying that the search failed
	BMessage* failMessage = new BMessage( BEAIM_EMAIL_SEARCH_RESULTS );
	failMessage->AddBool( "failed", true );
	PostAppMessage( failMessage );	
}

//-----------------------------------------------------

void AIMNetManager::SendArbitraryData( BMessage* msg ) {

	DataContainer data;
	ssize_t size = 0;
	char* chardata;
	
	// get that data outta there!
	msg->FindData( "data", B_RAW_TYPE, (const void**)(&chardata), &size );
	for( short i = 0; i < size; ++i )
		data << (unsigned char)chardata[i];

	// Send it!
	SendPacket( mainNetlet, data, 0x02 );
}

//-----------------------------------------------------

void AIMNetManager::SendIdleMessage( bool nowIdle ) {

	SNAC_Object snac;
	snac.Update( 0x01, 0x11, 0x00, 0x00, 0x00 );

	// are we idle now?
	if( nowIdle ) {
		snac.data << ToAIMWord(0x00);
		snac.data << (unsigned char)(2);
		snac.data << (unsigned char)('Z');	
	}
	
	// nope... guess not
	else {		
		snac.data << ToAIMWord(0x00);
		snac.data << ToAIMWord(0x00);
	}
	
	// send it
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

void AIMNetManager::SendNoOp() {

	SNAC_Object snac;
	snac.Update( 0x01, 0x16, 0x00, 0x00, 0x00 );
	
	// send the no-op command
	printf( "Sending No-Op..." );
	SendSNACPacket( mainNetlet, snac );	
}

//-----------------------------------------------------

void AIMNetManager::SetAwayStatus( BMessage* msg ) {

	BString awayMsg;
	printf( "SetAwayStatus: %s\n", msg->FindBool("away") ? "true" : "false" );
	
	// cheap hack to make sure the profile isn't length 1
	BString profile = prefs->Profile();

	// if we are setting an away message, tack on the length and message
	if( msg->FindBool("away") ) {
		awayMsg = client->CurrentAwayMessage();

		// cheap hack to make sure the away message isn't length zero,
		// assuming that we really *are* away, of course
		if( awayMsg.Length() == 0 )	
			awayMsg << BString(" ");
	}

	// set the profile stuff... yippee!
	SetProfileStuff( profile, awayMsg );
}

//-----------------------------------------------------

short AIMNetManager::TackOnCapabilityStuff( DataContainer& data ) {

	//int caps = CAPS_CHAT | CAPS_GETFILE | CAPS_SENDFILE;
	int caps = 0;
	int offset = 0;

	// tack on all the required capabilities info
	if( caps & CAPS_BUDDYICON ) {
		data << DataContainer( AIMCaps[0], sizeof(AIMCaps[0]) );
		offset += sizeof(AIMCaps[0]);
		printf( "tacking on cap: CAPS_BUDDYICON\n" );
	}
	if( caps & CAPS_VOICE ) {
		data << DataContainer( AIMCaps[1], sizeof(AIMCaps[1]) );
		offset += sizeof(AIMCaps[1]);
		printf( "tacking on cap: CAPS_VOICE\n" );
	}
	if( caps & CAPS_IMIMAGE ) {
		data << DataContainer( AIMCaps[2], sizeof(AIMCaps[2]) );
		offset += sizeof(AIMCaps[2]);
		printf( "tacking on cap: CAPS_IMIMAGE\n" );
	}
	if( caps & CAPS_CHAT ) {
		data << DataContainer( AIMCaps[3], sizeof(AIMCaps[3]) );
		offset += sizeof(AIMCaps[3]);
		printf( "tacking on cap: CAPS_CHAT\n" );
	}
	if( caps & CAPS_GETFILE ) {
		data << DataContainer( AIMCaps[4], sizeof(AIMCaps[4]) );
		offset += sizeof(AIMCaps[4]);
		printf( "tacking on cap: CAPS_GETFILE\n" );
	}
	if( caps & CAPS_SENDFILE ) {
		data << DataContainer( AIMCaps[5], sizeof(AIMCaps[5]) );
		offset += sizeof(AIMCaps[5]);
		printf( "tacking on cap: CAPS_SENDFILE\n" );
	}

	return offset;
}

//-----------------------------------------------------

int AIMNetManager::GrabCapabilityStuff( DataContainer block ) {

	int ret = 0;
	int offset = 0;

	while( offset < block.length() ) {
		for( int32 i = 0; i < (int32)(sizeof(AIMCaps)/16); ++i ) {
			if( DataContainer(AIMCaps[i],16) == block.getrange(offset,16) ) {
				switch(i) {
					case 0: ret |= CAPS_BUDDYICON;
						break;
					case 1: ret |= CAPS_VOICE;
						break;
					case 2: ret |= CAPS_IMIMAGE;
						break;
					case 3: ret |= CAPS_CHAT;
						break;
					case 4: ret |= CAPS_GETFILE;
						break;
					case 5: ret |= CAPS_SENDFILE;
						break;
					default: ret |= 0xff00;
						break;
				}
			}
		}
		offset += 16;
	}
	return ret;
}

//-----------------------------------------------------

void AIMNetManager::SetProfileStuff( BString profile, BString awayMsg ) {

	// cheap hack to make sure the profile isn't length 1
	if( profile.Length() == 1 )
		profile << BString(" ");
	
	// Make a SNAC object for the away packet and tack on some required data
	SNAC_Object snac;
	snac.Update( 0x02, 0x04, 0x00, 0x00, 0x02 );
	snac.data << ToTLV( 0x01, 0x1F, "text/aolrtf; charset=\"us-ascii\"" );

	// If there is a profile, add the whole TLV...
	// otherwise, send a profile of length 1, consisting of a single zero
	if( profile.Length() )
		snac.data << ToTLV( 0x02, profile.Length(), profile.String() );
	else
		snac.data << ToAIMWord(0x02) << ToAIMWord(0x01) << (unsigned char)(0);
	
	// if we are setting an away message, tack on the length and message
	if( awayMsg.Length() ) {
		snac.data << ToTLV( 0x03, 0x1F, "text/aolrtf; charset=\"us-ascii\"" );
		snac.data << ToTLV( 0x04, awayMsg.Length(), awayMsg.String() );
	} else
		snac.data << ToAIMWord( 0x04 ) << ToAIMWord( 0x00 );

	// stick the capability block in there.
	DataContainer caps;
	snac.data << ToAIMWord(0x05);
	int32 capLen = TackOnCapabilityStuff( caps );
	snac.data << ToAIMWord(capLen);
	snac.data << caps;
	
	// send it
	SendSNACPacket( mainNetlet, snac );
}

//-----------------------------------------------------

void AIMNetManager::ReceiveWarning( SNAC_Object& snac ) {

	BString warnMessage;
	char strNewWLevel[15];
	unsigned short wLevel, i=0;
	DataContainer screenName;
	bool anon = false;
	char snLength;

	wLevel = (unsigned short)( rint( (double)GetWord(snac.data,i) / 10.0 ) );
	i += 2;
	
	printf( "New warning level: %u%%\n", wLevel );
	printf( "... came from " );
	
	// 'twas an anonymous warning
	if( i >= snac.data.length() ) {
		anon = true;
		printf( "Anonymous\n" );
	}
	
	// grab the length, and then the screen name
	if( !anon ) {
		snLength = snac.data[i++];
		screenName = DataContainer( snac.data.getrange(i,snLength) );
		i += snLength;

		screenName << char(0);
		printf( "%s\n", screenName.c_ptr() );
	}
	
	// is our warning level going DOWN? if so, don't annoy the user by telling them about it
	if( wLevel < client->WarningLevel() ) {
		client->SetWarningLevel( wLevel );
		return;
	}

	// make the "warned" message
	client->SetWarningLevel( wLevel );
	sprintf( strNewWLevel, "%u%%", wLevel );
	warnMessage = BString( Language.get("ERR_GOT_WARNED") );
	warnMessage.ReplaceAll( "%WLEVEL", strNewWLevel );
	if( anon ) {
		BString anonLabel = "[";
		anonLabel.Append( Language.get("ANONYMOUS_LABEL") );
		anonLabel.Append( "]" );
		warnMessage.ReplaceAll( "%USER", anonLabel.String() );
	} else
		warnMessage.ReplaceAll( "%USER", screenName.c_ptr() );
	
	// now display it
	windows->ShowMessage( warnMessage, B_STOP_ALERT, Language.get("BEHAVE_LABEL"), WS_WARNED );
}

//-----------------------------------------------------

void AIMNetManager::FailedMessage( SNAC_Object& snac ) {

	BString errorMessage;
	unsigned short targetReqID;
	bool found = false, kg;
	userRequestID rid;

	targetReqID = snac.GetRequestID();
	kg = pendingNRRequests.First(rid);
	while( kg ) {
		if( targetReqID == rid.reqID ) {
			found = true;
			break;
		}
		kg = pendingNRRequests.Next(rid);
	}
	
	// if we didn't find it, stay quiet for now
	if( !found )
		return;
	
	// aha! Found it! Send an error message
	printf( ">>> Failed Message: %s\n", rid.userid.UserString() );
	
	// failure to send a message
	if( rid.type == RID_SEND_MESSAGE )
	{
		if( users->IsUserBlocked(rid.userid) )
			errorMessage = BString( Language.get("ERR_NO_SEND_BLOCKED") );
		else
			errorMessage = BString( Language.get("ERR_NO_SEND_OFFLINE") );
	}
	
	// failure to send a warning
	else if( rid.type == RID_WARN_EM )
		errorMessage = BString( Language.get("ERR_BAD_WARN_ATTEMPT") );

	// none of the above	
	else return;

	// replace %USER with the username and show the warning
	errorMessage.ReplaceAll( "%USER", rid.userid.UserString() );
	windows->ShowMessage( errorMessage, B_STOP_ALERT );
}

//-----------------------------------------------------

void AIMNetManager::SendWarning( BMessage* msg ) {

	SNAC_Object sendMsg;

	// debug stuff
	char message[100];
	sprintf( message, "Warning: %s  [mode=%s]\n", msg->FindString("userid"),
			 msg->FindBool("anonymous") ? "anonymous" : "normal" );
	printf( message );
	
	// Make the SNAC object for the warning
	sendMsg.Update( 0x04, 0x08, 0x00, 0x00, requestID );
	
	// make the warning anonymous (or not)
	if( msg->FindBool("anonymous") )
		sendMsg.data << ToAIMWord(0x01);
	else
		sendMsg.data << ToAIMWord(0x00);

	// finally, tack on the screen name
	sendMsg.data << char(strlen((char*)msg->FindString("userid")))		// sn length
				 << (char*)msg->FindString("userid");					// sn

	// log it, so we know what happened in case the warning fails
	LogMessageReqID( BString((char*)msg->FindString("userid")), RID_WARN_EM );
	
	// Now that we've gone to all the trouble of making that, send it
	SendSNACPacket( mainNetlet, sendMsg );
}

//-----------------------------------------------------

void AIMNetManager::MishMashWarningFunction( int type, SNAC_Object& snac )
{
	BMessage* msg;

	// you got booted because someone signed on w/ the same screen name
	if( type == 1 ) {
		printf( "You are being IMPERSONATED!!!! AAAAAAAHHHHHHH!!!!!\n" );
	
		// kill the connection and let 'em know why they got booted
		Logout();
		msg = new BMessage(BEAIM_DISCONNECTED);
		msg->AddBool("quietly", true);
		PostAppMessage( msg );
		windows->ShowMessage( Language.get("ERR_BEING_IMPERSONATED"), B_STOP_ALERT );
	}
	
	// you missed a few messages due to somebody else's rate violation
	else if( type == 2 ) {
		printf( "some messages were missed... the other person sent them too fast.\n" );
		windows->ShowMessage( Language.get("ERR_MISSED_MESSAGES"), B_INFO_ALERT );
	}
	
	// you did a rate violation yourself, you naughty person, you!
	else if( type == 3 ) {
		printf( "Rate violation!\n" );
		windows->ShowMessage( Language.get("ERR_RATE_VIOLATION"), B_STOP_ALERT, Language.get("BEHAVE_LABEL"), WS_WARNED );
	}
}

//-----------------------------------------------------
