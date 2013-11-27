#include "Globals.h"
#include "MiscStuff.h"
#include "AIMNetManager.h"
#include "Say.h"
#include <stdlib.h>
#include <unistd.h>
#include "rsamd5.h"

#define BEAIM_NEW_LOGIN

//-----------------------------------------------------

// The version numbers reported by BeAIM
const unsigned short BEAIM_REPORT_MAJOR_VERSION 	= 5;		// was 0
const unsigned short BEAIM_REPORT_MINOR_VERSION	= 1;		// was 6
const unsigned short BEAIM_REPORT_BUILD			= 2313;		// was 42
const unsigned short BEAIM_PROTOCOL_VERSION		= 0x0001;		// was 1

//-----------------------------------------------------

// Login function
void AIMNetManager::Login( BMessage* message ) {

	// Host and port
	BString aim_host;
	int32 aim_port = 0;
	clientReady = false;
	
	// Set the number of steps in this particular login
	BMessage* sendMessage = new BMessage(BEAIM_SET_LOGIN_STEP_COUNT);
	sendMessage->AddInt32( "count", 29 );
	PostAppMessage( sendMessage );
	
	// Get the host and port from the prefs
	prefs->ReadString( "AIMHost", aim_host, "login.oscar.aol.com", true, false );
	aim_port = prefs->ReadInt32( "AIMPort", 5190, true );
	
	// Since this is all event based, we have to make copies of the sn/pw
	loginScreenName = BString(message->FindString("userid"));
	loginPassword = BString((char*)message->FindString("password"));

	// take care of the proxy stuff	
	netProxyMode proxyMode = (netProxyMode)prefs->ReadInt32( "ProxyMode", (int32)NPM_NO_PROXY, true );
	BString proxyHost, authUser, authPass;
	int32 proxyPort = prefs->ReadInt32( "ProxyPort", 80, true );
	prefs->ReadString( "ProxyHost", proxyHost, "127.0.0.1", true, false );
	prefs->ReadString( "ProxyUser", authUser, "", true, false );
	prefs->ReadString( "ProxyPass", authPass, "", true, false );
	bool auth = prefs->ReadBool( "ProxyAuth", false, true );
	SetProxyInfo( proxyMode, proxyHost, proxyPort, auth, authUser, authPass );
	
	// first: grab a netlet (the authorization netlet)
	authNetlet = MakeNetlet(NLT_AUTH);
	printf( "--> MADE AUTH NETLET: id=%d\n", authNetlet.nid );
	
	// Time to begin... connect to the authorization server
	preLoginPhase = 1;
	sendSequence = recvSequence = 0;
	LoginStep( Language.get("LS_CONNECT_AUTH_SERVER") );
	Connect( authNetlet, aim_host, aim_port );
}

//-----------------------------------------------------

// Function to update the status of the logon
void AIMNetManager::LoginStep( const char* message, bool updateText ) {

	BMessage* sendMessage;
	
	// stick an ellipsis on the end of the message
	BString theMessageThang = BString(message);
	theMessageThang.Append( BString(B_UTF8_ELLIPSIS) );

	// Stick the message into a BMessage and send it
	sendMessage = new BMessage( BEAIM_LOGIN_STEP );
	sendMessage->AddString( "status", theMessageThang.String() );
	sendMessage->AddBool( "update_text", updateText );
	PostAppMessage( sendMessage );
}

//-----------------------------------------------------

// Whoops... something went wrong. Cancel the login.
void AIMNetManager::LoginFailure( BString reason ) {

	BMessage* sendMessage;

	// then send a message saying that the login has failed, with a reason.
	sendMessage = new BMessage( BEAIM_LOGIN_FAILURE );
	sendMessage->AddString( "reason", reason.String() );
	PostAppMessage( sendMessage );

	// first, do a bit of cleanup in here.
	Disconnect( authNetlet );
	preLoginPhase = 0;
}

//-----------------------------------------------------

// Handles the login "steps"
void AIMNetManager::DoLogin( BMessage* info ) {
	char channel;

	printf( "dologin()... %d\n", preLoginPhase );

	// do stuff based on the login phase
	switch( preLoginPhase ) {
		
		// Just after the initial connection
		case 1:
			LoginStep( Language.get("LS_CONNECTED_AUTH_SERVER") );
			sendSequence = 0;
			recvSequence = 0;
			break;

		// Connection acknowledgement received from authorization server
		case 2:
			snooze(500000);
			printf( "waitink for ack packet...\n" );
			channel = Receive(authNetlet);
			printf( "ack packet recv'd on channel %d\n", channel );
			SendLoginData();
			break;

		// Connected to BOS server
		case 4:
			LoginStep( Language.get("LS_CONNECTED_AIM_SERVER") );
			printf( "DoLogin(): connected to BOS server\n" );
			break;

		// BOS connection acknowledgment... send the sign on request
		case 5:
			channel = Receive(mainNetlet);
			printf( "BOS connection ack packet recv'd on channel %d  (size: %d bytes)\n", (int)channel, recvBuf.length() );
			SendBOSRequest();
			break;

		// Host Ready response received... send the rate request
		case 6:
			LoginStep( Language.get("LS_RECV_HOST_READY") );
			printf( "host ready response received\n" );
			ReceiveBOSResponse();
			SendBOSRequest();
			break;
			
		// Rate response received... send rate acknowledgement (and other stuff)
		case 7:
			LoginStep( Language.get("LS_RECV_RATE") );
			ReceiveBOSResponse();
			SendBOSRequest();
			ProcessMainServerCommands();
			break;
		
		// Process the responses to various other stuff we have sent up	
		case 8:
		case 9:
		case 10:
			snooze(500000);
			LoginStep( "", false );
			printf( "got something! yeah!\n" );
			ReceiveBOSResponse();
			ProcessMainServerCommands();
			break;
		
		// server is done sending responses... send up more stuff
		case 11:
			snooze(500000);
			printf( "boo-rah!\n" );
			LoginStep( Language.get("LS_RECV_HOST") );
			//ReceiveBOSResponse();
			SendBOSRequest();
			ProcessMainServerCommands();
			break;
		
		// minimum report interval
		case 12:
			snooze(500000);
			LoginStep( Language.get("LS_RECV_REPORT_INT") );
			ReceiveBOSResponse();
			ProcessMainServerCommands();
			break;
			
		// mystery packet (comes at the end of login)
		case 13:
			snooze(500000);
			LoginStep( Language.get("LS_FINALIZING") );
			ReceiveBOSResponse();
			snacPile = 0;
			ProcessMainServerCommands();
			NowOnline();
			return;
	};

	// Time for the next step
	++preLoginPhase;	
}

//-----------------------------------------------------

void AIMNetManager::SendLoginData() {

	DataContainer loginPacket = "";
	DataContainer clientProfile;

	// the new, shiny MD5 based login
	loginPacket << ToAIMWord(0x0000) << ToAIMWord(BEAIM_PROTOCOL_VERSION);
	SendPacket( authNetlet, loginPacket, 0x01 );
	SNAC_Object snac;
	snac.Update(0x17, 0x06, 0, 0, 0);
	snac.data << ToTLV(0x01, loginScreenName.Length(), loginScreenName.String());
	snac.data << ToAIMWord(0x004B) << ToAIMWord(0x0000);
	snac.data << ToAIMWord(0x005A) << ToAIMWord(0x0000);
	SendSNACPacket(authNetlet, snac);
	LoginStep( Language.get("LS_SENDING_AUTH_DATA") );	
}

//-----------------------------------------------------

DataContainer AIMNetManager::EncryptPassword( BString password )
{
	char encoding_table[] = {
		0xf3, 0xb3, 0x6c, 0x99,
		0x95, 0x3f, 0xac, 0xb6,
		0xc5, 0xfa, 0x6b, 0x63,
		0x69, 0x6c, 0xc3, 0x9f
	};

	DataContainer encoded;
	int32 i;
	
	// encode the password
	for( i = 0; i < password.Length(); i++ )
		encoded << char( password[i] ^ encoding_table[i] );
		
	return encoded;
}

//-----------------------------------------------------

void AIMNetManager::ParseLoginResponse( DataContainer data ) {

	printf( "parsing login response\n" );

	char channel;
	DataContainer nextServer;
	int32 nextPort = prefs->ReadInt32("AIMPort", 5190, true);

#ifndef BEAIM_NEW_LOGIN	
	// Get the authorization response packet
	channel = Receive( authNetlet );
	if( channel != 0x04 ) {
		printf( "Hmmmm.... channel should have been 0x04, but was %c instead! Bad.\n", channel );
		return;
	}
#endif

	// Decode it!
	unsigned short Tag, Len, i = 10;
	unsigned short errNum;

	// Go through all the TLV's in this packet
	while( i < recvBuf.length() ) {

		// Get the Tag, Length, and Value out of recvBuf
		Tag = GetWord( recvBuf, i );	i += 2;
		Len = GetWord( recvBuf, i );	i += 2;
		DataContainer Value( recvBuf.getrange(i,Len) );
		i += Len;
		
		printf( "%%%%%%%%%%%%%% NEW TLV!!!\n" );
		printf( "    Tag: %X\n", Tag );
		printf( "    Len: %X\n\n", Len );

		// Go through the possible tag values
		switch( Tag ) {

			// The "official" screen name
			case 0x01:
				Value << char(0);
				printf( "official screen name: %s\n", Value.c_ptr() );
				break;
			
			// error message URL
			case 0x04:
				Value << char(0);
				printf( "error... go to %s for more info\n", Value.c_ptr() );
				break;
				
			// BOS IP address (redirect)
			case 0x05:
				Value << char(0);
				
				{
					// format seems to be addr:port
					const char* colon = strchr(Value.c_ptr(), ':');
					if (colon && atoi(colon+1)>0) nextPort = atoi(colon+1);
					nextServer = colon? Value.getrange(0,colon-Value.c_ptr()):Value;
					nextServer << char(0);
					
					printf( "The BOS IP to redirect to is %s\n", nextServer.c_ptr() );
					printf( "The BOS port to redirect to is %s\n", colon? colon+1:"not specified");
				}
				break;

			// Received the authorization cookie
			case 0x06:
				cookie = Value;
				printf( "got the authorization cookie. Woohoo!\n" );
				break;

			// email address
			case 0x11:
				Value << char(0);
				printf( "Your email address (according to OSCAR) is %s\n", Value.c_ptr() );
				break;

			// registration status
			case 0x13:
				errNum = (Value[0] << 8) + Value[1];
				printf( "registration status is %u, whatever that means.\n", errNum );
				break;

			// Error number
			case 0x08:
				errNum = (Value[0] << 8) + Value[1];
				switch( errNum ) {

					// Invalid screen name
					case 0x01:
						LoginFailure( Language.get("LF_INVALID_SN") );
						printf( "ERROR: invalid screen name!\n" );
						break;
						
					// Invalid password
					case 0x05:
						printf( "ERROR: invalid password!\n" );
						LoginFailure( Language.get("LF_INVALID_PW") );
						break;
						
					// Too many logins
					case 0x18:
						LoginFailure( Language.get("LF_TOO_FAST") );
						printf( "ERROR: logged on too many times in a set interval!\n" );
						break;

					// something else
					default:
						LoginFailure( Language.get("LF_DUNNO") );
						printf( "ERROR: something weird has gone wrong, and we don't know what! Aah!\n" );
						break;
				}
				connected = false;
				break;
				
			// unknown TLV
			default:
				printf( "Hmmmm... no idea what this TLV is. Ignoring it.\n" );
				break;
		}
	}
	
	// ahhh... all done with the authorization. Disconnect from this server.
	connected = false;
	recvSequence = 0;

	// disconnect the authorization netlet, and get the main one
	Disconnect( authNetlet );
	authNetlet.nid = -1;
	authNetlet.type = -1;
	mainNetlet = MakeNetlet(NLT_MAIN);
	printf( "--> MADE MAIN NETLET: id=%d\n", mainNetlet.nid );
	
	// Now, we have the cookie and the next server, so attempt to connect to that.
	if( preLoginPhase ) {
		LoginStep( Language.get("LS_CONNECTING_AIM_SERVER") );
		printf( "next server is... %s\n", nextServer.c_ptr() );
		snooze(500000);		
		Connect( mainNetlet, nextServer.c_ptr(), nextPort );
	}
}

//-----------------------------------------------------

void AIMNetManager::SendBOSRequest() {

	DataContainer sendBuf;
	SNAC_Object snac;

	// The BOS Sign On Command... basically just resend the cookie
	if( preLoginPhase == 5 ) {
		sendBuf << ToAIMWord( 0x0000 );
		sendBuf << ToAIMWord( 0x0001 );
		sendBuf << ToTLV( 0x06, cookie.length(), cookie );
		LoginStep( Language.get("LS_SENDING_LOGIN_DATA") );
		SendPacket( mainNetlet, sendBuf, 0x01 );
		return;
	}
	
	// Send a rate request, to find out how fast we can send SNAC's
	if( preLoginPhase == 6 ) {
		printf( "// Send a rate request, to find out how fast we can send SNAC's\n ");
		snac.Update( 0x01, 0x06, 0, 0, 0x1015 );
		LoginStep( Language.get("LS_SEND_RATE") );
		SendSNACPacket( mainNetlet, snac );
		return;
	}
	
	// Send lots of stuff requests... order doesn't matter here, so
	// we can "chunk" them together
	if( preLoginPhase == 7 ) {

		LoginStep( Language.get("LS_SEND_INFO_REQS") );
		
		// Send an acknowledgement that we got the rate response
		snac.Update( 0x01, 0x08, 0x00, 0x00, 0x1016 );
		snac.data << ToAIMWord( 0x0001 ) << ToAIMWord( 0x0002 )
				  << ToAIMWord( 0x0003 ) << ToAIMWord( 0x0004 );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );

		// Set privacy flags
		snac.Update( 0x01, 0x14, 0x00, 0x00, 0x1017 );
		snac.data << ToAIMWord( 0x0000 ) << ToAIMWord( 0x0003 );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );

		// Send request for our user info
		printf( "// Send request for our user info\n" );
		snac.Update( 0x01, 0x0E, 0x00, 0x00, 0x101a );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );
		
		// Send BOS rights request
		snac.Update( 0x09, 0x02, 0x00, 0x00, 0x101a );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );
		
		// Send BuddyList rights request
		snac.Update( 0x13, 0x02, 0x00, 0x00, 0x101b );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );
		
		// Send locate service rights request
		snac.Update( 0x02, 0x02, 0x00, 0x00, 0x101c );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( "", false );
		
		// Send IM Parameter info request
		snac.Update( 0x04, 0x04, 0x00, 0x00, 0x101d );
		SendSNACPacket( mainNetlet, snac );
		LoginStep( Language.get("LS_RECV_INFO_REQS") );
		
		return;
	}
	
	// send up another big load of stuff
	if( preLoginPhase == 11 ) {

		LoginStep( Language.get("LS_SENDING_LOGIN_DATA") );

		// Optional(?) SNAC number 1
		snac.Update( 0x09, 0x04, 0x00, 0x00, 0x101e );
		snac.data << ToAIMWord( 0x00 ) << ToAIMWord( 0x001f );
		SendSNACPacket( mainNetlet, snac );

		// Optional(?) SNAC number 2
		snac.Update( 0x09, 0x07, 0x00, 0x00, 0x101f );
		SendSNACPacket( mainNetlet, snac );
		
		// Send up the buddy list
		GenList<DataContainer> BList;
		DataContainer tempList;
		LoadBuddyList( BList );
		LoginStep( Language.get("LS_SENDING_BUDDY_LIST") );
		printf( "Sending buddy list...\n" );
		
		while( BList.Dequeue(tempList) ) {
			snac.Update( 0x03, 0x04, 0x00, 0x00, 0x1020 );
			snac.data << tempList;
			SendSNACPacket( mainNetlet, snac );
		}
		printf( "Done with buddy list.\n" );

		// Send up the user profile
		BString profile = prefs->Profile();
		LoginStep( Language.get("LS_SENDING_USER_PROFILE") );
		SetProfileStuff( profile, BString() );
		
		// send up the block list (if there is one)
		AIMUser ttemp;
		if( users->GetNextBlockedUser( ttemp, true ) ) {
			LoginStep( Language.get("LS_SENDING_BLOCK_LIST") );
			SetUserBlockiness( NULL, true );
		}

		// send set initial ICBM parameter
		snac.Update( 0x04, 0x02, 0x00, 0x00, 0x1022 );
		snac.data << ToAIMWord(0x01)   << ToAIMWord(0x00)   << ToAIMWord(0x0001 | 0x0002 | 0x0008)
				  << ToAIMWord(0x1f3f) << ToAIMWord(0x03e7) << ToAIMWord(0x03e7)
				  << ToAIMWord(0x00)   << ToAIMWord(0x64);
		SendSNACPacket( mainNetlet, snac );
		
		// send client online/ready
		LoginStep( Language.get("LS_SENDING_CLIENT_READY") );
		snac.Update( 0x01, 0x02, 0x00, 0x00, 0x1023 );
		snac.data << ToAIMWord(0x01) << ToAIMWord(0x02) << ToAIMWord(0x01)
				  << ToAIMWord(0x13) << ToAIMWord(0x09) << ToAIMWord(0x01)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x03)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x01)
				  << ToAIMWord(0x04) << ToAIMWord(0x01) << ToAIMWord(0x01)
				  << ToAIMWord(0x01) << ToAIMWord(0x02) << ToAIMWord(0x01)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x08)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x01)
				  << ToAIMWord(0x06) << ToAIMWord(0x01) << ToAIMWord(0x01)
				  << ToAIMWord(0x01) << ToAIMWord(0x0a) << ToAIMWord(0x01)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x0b)
				  << ToAIMWord(0x01) << ToAIMWord(0x01) << ToAIMWord(0x01);
		SendSNACPacket( mainNetlet, snac );
		
//		snac.Update(0x13, 0x07, 0, 0, 0x1024);
//		SendSNACPacket(mainNetlet, snac);
		
		// ... and we're almost officially online!
		return;
	}
}

//-----------------------------------------------------

void AIMNetManager::ReceiveBOSResponse() {

	// Get the actual data
	char channel = Receive( mainNetlet );
	SNAC_Object snac;
	
	// The Host-Ready response
	if( preLoginPhase == 6 || preLoginPhase == 7 ) {
		if( channel != 0x02 ) {
			printf( "received response on the wrong channel (%c instead of 0x02)! bad.\n", channel );
			return;
		}
		snac = SNAC_Object(recvBuf);
		DecodeSNACPacket( snac );
		return;
	}
	
	// the "default" case
	snac = SNAC_Object( recvBuf );
	DecodeSNACPacket( snac );
}

//-----------------------------------------------------

void AIMNetManager::LoadBuddyList( GenList<DataContainer>& buddies ) {
	
	const int maxList = 50;
	AIMUser user;
	DataContainer adder;
	int count = 0;
	bool kg;
	
	// add 'em all, in blocks of [maxlist]
	kg = users->GetAllBuddies(user, true);
	while( kg ) {

		// add the length of the name, then the name
		adder << char(user.Username().Length());
		adder << (char*)user.UserString();
		
		printf( "ADD:  [%d] %s\n", int(char(user.Username().Length())), (char*)user.UserString() );
		
		// finish the block if needed
		if( !(++count % maxList) ) {
			buddies.Add( adder );
			adder = "";		
		}

		// next buddy, please
		kg = users->GetAllBuddies(user, false);
	}
	
	// if there's anything left in adder, add it
	if( adder.length() > 0 )
		buddies.Add( adder );
}

//-----------------------------------------------------

void AIMNetManager::ParseMyUserInfoPacket( DataContainer data ) {

	unsigned short Tag, Len = 0, i = 0, wLevel, temp2;
	DataContainer funStuff;
	time_t onSince;
	char snLength;

	// grab the length, and then the screen name
	snLength = data[i++];
	funStuff = DataContainer( data.getrange(i,snLength) );
	i += snLength;

	wLevel = (unsigned short)( rint( (double)GetWord(data,i) / 10.0 ) );
	client->SetWarningLevel( wLevel );
	printf( "warning level? %u\n", wLevel );
	
	temp2 = GetWord( data, i );	i += 2;

	// Go through all the TLV's in this packet
	while( i < data.length() ) {

		// Get the Tag, Length, and Value out of recvBuf
		Tag = GetWord( data, i );	i += 2;
		Len = GetWord( data, i );	i += 2;
		DataContainer Value( data.getrange(i,Len) );
		i += Len;
		
		// Go through the possible tag values
		switch( Tag ) {

			// join date
			case 0x02:
				//printf( "join date\n" );
				break;
				
			// on since date
			case 0x03:
				//printf( "on since date\n" );
				temp2 = (unsigned short)GetWord( Value, 0 );
				onSince = (temp2 << 16) + (unsigned short)GetWord( Value, 2 );
				break;
				
			default:
				printf( "ParseMyUserInfoPacket Unknown TLV: type 0x%02x, len %d bytes\n", Tag, Len );
				break;
		}
	}
}

//-----------------------------------------------------

// woo-hoo! We're all logged in now. Tell the app.
void AIMNetManager::NowOnline() {

	preLoginPhase = 0;
	printf( "You are now online... officially!\n" );
	
	// Send a message saying that the signon was successful
	BMessage* goodSignOn = new BMessage(BEAIM_SIGN_ON_SUCCESSFUL);
	PostAppMessage( goodSignOn );
}

//-----------------------------------------------------
