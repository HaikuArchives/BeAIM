#include "Globals.h"
#include "AIMNetManager.h"
#include "Say.h"

//-----------------------------------------------------

bool debugDumpIn = true;
bool debugDumpOut = true;

//-----------------------------------------------------

AIMNetManager::AIMNetManager() {

	preLoginPhase = 0;			// not doing the prelogin yet
	connected = false;
	clientReady = false;
	
	requestID = 0x2720;
	flapRead = 0;
	correctFLAPSize = 0;

	snacPile = -1;
	
	authNetlet.nid = -1;
	mainNetlet.nid = -1;
}

//-----------------------------------------------------

AIMNetManager::~AIMNetManager() {

}

//-----------------------------------------------------

void AIMNetManager::MessageReceived( BMessage* msg ) {

	// let the network code handle the net messages
	if( msg->HasBool("netonly") ) {
		NetworkManager::MessageReceived(msg);
		return;
	}

	// handle all the rest of them here
	switch( msg->what ) {
		
		case BEAIM_SIGN_ON:
			Login( msg );
			break;
			
		case BEAIM_LOGOUT:
			Logout();
			break;
			
		case BEAIM_SEND_MESSAGE:
			SendMessage( msg );
			break;
			
		case BEAIM_GET_USER_INFO:
			RequestPersonInfo( msg );
			break;
			
		case BEAIM_GET_AWAY_INFO:
			RequestAwayInfo( msg );
			break;
			
		case BEAIM_CANCEL_SIGN_ON:
			CancelSignOn();
			break;
			
		case BEAIM_SIGN_OFF:
			//Logout();
			break;
			
		case BEAIM_SEND_ARBITRARY_DATA:
			SendArbitraryData( msg );
			break;
			
		case BEAIM_NOW_IDLE:
			SendIdleMessage( true );
			break;
			
		case BEAIM_NOW_ACTIVE:
			SendIdleMessage( false );
			break;

		case BEAIM_SEND_NOOP:
			SendNoOp();
			break;

		case BEAIM_BUDDYLIST_COMMIT:
			UpdateServerBuddyList( msg );
			break;

		case BEAIM_SEARCH_BY_EMAIL:
			SearchByEmail( msg );
			break;
			
		case BEAIM_GOING_AWAY:
			SetAwayStatus( msg );
			break;
			
		case BEAIM_WARN_SOMEONE:
			SendWarning( msg );
			break;
			
		case BEAIM_SET_USER_BLOCKINESS:
			SetUserBlockiness( msg );
			break;
			
		case BEAIM_CLIENT_READY:
			SetClientReady();
			break;
			
		case BEAIM_CLEAR_SNACPILE:
			if (snacPile != -1) snacPile = 0;
			break;

		default:
			NetworkManager::MessageReceived(msg);
			break;
	}
}

//-----------------------------------------------------

void AIMNetManager::Connected( nlID nid )
{
	printf( "AIMNetManager::Connected\n" );

	// if this is the auth netlet, continue the login process
	if( nid.type == NLT_AUTH )
		DoLogin( NULL );

	// if this is the main netlet, well, do pretty much the same thing
	if( nid.type == NLT_MAIN )
		DoLogin( NULL );
}

//-----------------------------------------------------

void AIMNetManager::ConnectError( nlID nid, int what, BString error )
{
	if( nid == mainNetlet || nid == authNetlet ) {
		switch( what ) {
			case NETLET_COULD_NOT_CONNECT:
				LoginFailure( Language.get("LF_NO_CONNECT") );
				break;
		
			case NETLET_COULD_NOT_CONNECT_TO_PROXY:
				LoginFailure( Language.get("LF_NO_PROXY_CONNECT") );
				break;
	
			case NETLET_PROXY_BAD_AUTH:
				LoginFailure( Language.get("LF_PROXY_AUTH_FAILURE") );
				break;
		
			default:
				printf( "AIMNetManager::ConnectError (unknown)\n" );
		};
	}
}

//-----------------------------------------------------

void AIMNetManager::Disconnected( nlID nid )
{
	PostAppMessage( new BMessage(BEAIM_DISCONNECTED) );
}

//-----------------------------------------------------

void AIMNetManager::DataReceived( nlID nid )
{
	// decide what to do based on the connection type, eh?
	switch( nid.type ) {

		// authorization server connection
		case NLT_AUTH: {
			DoLogin();
			ProcessAuthServerCommands();
			return;
		}

		case NLT_MAIN: {

			// if we're still logging in, keep doing that
			if( preLoginPhase ) {
				DoLogin();
				return;
			}
			
			// otherwise, it's just a standard AIM command/message
			ProcessMainServerCommands();
		}

	}
}

//-----------------------------------------------------

void AIMNetManager::CancelSignOn() {
	
	// disconnect both of the netlets that could be active at this point
	printf( "canceling sign-on...\n" );
	printf( "auth.nid = %d        main.nid = %d\n", authNetlet.nid, mainNetlet.nid );
	Logout();
}

//-----------------------------------------------------

void AIMNetManager::Logout() {

	// disconnect both of the netlets that could be active at this point
	Disconnect( authNetlet );
	Disconnect( mainNetlet );
}

//-----------------------------------------------------

void AIMNetManager::ReloadSSIList()
{
	SNAC_Object snac;
	BString group;
	AIMUser user;
	bool kg = false, kg2 = false;
	short gid, id_group = 1, id_user;
	snac.Update(0x13, 0x09, 0, 0, 0x1954);
	kg = users->GetGroups(group, true);
	while( kg ) {
		users->GetGroupSSIInfo(group, gid);
		users->AddGroup(group, -1, id_group);
		snac.data << ToAIMWord(group.Length()) << group.String();
		snac.data << ToAIMWord(id_group) << ToAIMWord(0);
		snac.data << ToAIMWord(0x0001) << ToAIMWord(0);
		id_user = 1;
		kg2 = users->GetGroupBuddies(user, group, true);
		while(kg2) {
			user.SetSSIUserID(id_user);
			user.SetSSIGroupID(id_group);
			snac.data << ToAIMWord(user.Username().Length()) << user.UserString();
			snac.data << ToAIMWord(id_group) << ToAIMWord(id_user++);
			snac.data << ToAIMWord(0) << ToAIMWord(0);
			// update the master list
			users->AddBuddy(user);
			kg2 = users->GetGroupBuddies(user, group, false);
		}
		kg = users->GetGroups(group, false);
		id_group++;
	}
	SendSNACPacket(mainNetlet, snac);
}