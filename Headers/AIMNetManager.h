#ifndef _AIM_NET_MANAGER_H_
#define _AIM_NET_MANAGER_H_

#include <stdio.h>
#include "NetManager.h"
#include "AIMUser.h"
#include "AIMDataTypes.h"
#include "MiscStuff.h"

//-----------------------------------------------------

// netlet type constants
const int NLT_AUTH = 1;
const int NLT_MAIN = 2;

//-----------------------------------------------------

// request id type constants
const int RID_GET_INFO = 1;
const int RID_GET_AWAY_MSG = 2;
const int RID_WARN_EM = 3;
const int RID_SEND_MESSAGE = 4;

//-----------------------------------------------------

// So we can translate request ids back to user ids
struct userRequestID {
	int type;
	unsigned int reqID;
	AIMUser userid;
};

//-----------------------------------------------------

class AIMNetManager : public NetworkManager {

	public:
		AIMNetManager();
		~AIMNetManager();
		void MessageReceived( BMessage* );
		
		void Login( BMessage* );
		void Logout();
		void CancelSignOn();
		
		void SendMessage( BMessage* );
		void RequestPersonInfo( BMessage* );
		void RequestAwayInfo( BMessage* );
		void SearchByEmail( BMessage* );
		void SendArbitraryData( BMessage* );
		void SendIdleMessage( bool );
		void SendNoOp();
		void SetAwayStatus( BMessage* );
		
		void SSIAddUser(AIMUser, BString);
		void SSIAddGroup(BString, short);
		void SSIRemoveUser(AIMUser);
		void SSIRemoveGroup(BString);

		void Dump( DataContainer& );
		
		void SetTypingStatus(AIMUser, short);		

		void ReloadSSIList();

	protected:
	
		// notification functions
		virtual void Connected( nlID netletID );
		virtual void ConnectError( nlID netletID, int what, BString error="" );
		virtual void Disconnected( nlID netletID );
		virtual void DataReceived( nlID netletID );
		
	private:
	
		// login functions
		void DoLogin( BMessage* info = NULL );
		void LoginStep( const char*, bool=true );
		void LoadBuddyList( GenList<DataContainer>& buddies );
		void SendLoginData();
		void ParseLoginResponse( DataContainer data );
		void ParseMyUserInfoPacket( DataContainer data );
		DataContainer EncryptPassword( BString password );
		void SendBOSRequest();
		void ReceiveBOSResponse();
		void NowOnline();
		void LoginFailure( BString message );
		
		// packet decoder functions
		void DecodeSNACPacket( SNAC_Object& snac );
		void DecodeUserSNAC( SNAC_Object& snac );
		void DecodeBuddyListSNAC( SNAC_Object& snac );
		void DecodeMessagingSNAC( SNAC_Object& snac );
		void DecodeAdministrativeSNAC( SNAC_Object& snac );
		void DecodeBOSSNAC( SNAC_Object& snac );
		void DecodeOtherSNAC( SNAC_Object& snac );
		void DecodeSSISNAC(SNAC_Object & snac);
		void DecodeMD5LoginSNAC(SNAC_Object & snac);
		void DecodeGenericServiceSNAC( SNAC_Object& snac );
		
		// "functional" functions
		void DecodeBuddyStuff( DataContainer, short&, BMessage* );		
		void ReceiveMessage( SNAC_Object& snac );
		void BuddyOnline( SNAC_Object& snac );
		void BuddyOffline( SNAC_Object& snac );
		void ReceivePersonInfo( SNAC_Object& snac );
		void FailedPersonInfo( SNAC_Object& snac );
		void UpdateServerBuddyList( BMessage* );
		void SendRemovePersonMessage( AIMUser name, int len );
		void ReceiveSearchResults( SNAC_Object& snac );
		void ReceiveFailedSearch( SNAC_Object& snac );
		void ReceiveWarning( SNAC_Object& snac );
		void SendWarning( BMessage* msg );
		void FailedMessage( SNAC_Object& snac );
		void SetUserBlockiness( BMessage*, bool all=false );
		void SetProfileStuff( BString profile, BString awayMsg );
		short TackOnCapabilityStuff( DataContainer& );
		int GrabCapabilityStuff( DataContainer );
		void SetClientReady();

		void MishMashWarningFunction( int type, SNAC_Object& snac );

		// Send and receive functions
		void SendPacket( nlID nid, DataContainer packet, char channel );
		void Transmit( nlID nid, DataContainer packet );
		void SendSNACPacket( nlID nid, SNAC_Object& obj, char channel = 0x02 );
		char Receive( nlID nid );
		bool ProcessMainServerCommands();
		bool ProcessAuthServerCommands();
		void LogMessageReqID( AIMUser userID, int type );
	
		// useful functions
		DataContainer ToAIMWord( unsigned short );
		DataContainer ToTLV( unsigned short, unsigned short, DataContainer );
		unsigned short GetWord( char* input );
		unsigned short GetWord( DataContainer input, short i );

		bool CheckLoggedIn();
	
		// Sequence numbering stuff
		unsigned int sendSequence;
		unsigned int recvSequence;
		
		// keeps track of various request ids
		GenList<userRequestID> pendingRequests;
		GenList<userRequestID> pendingNRRequests;
		unsigned int emailSearchID;
		unsigned int requestID;
		
		int snacPile;

		// connection info data
		int preLoginPhase;
		bool connected;
		bool clientReady;
		
		// temporary local copies of the screen name and password
		BString loginScreenName;
		BString loginPassword;
		BString savedGroup;
		
		// The received buffers
		DataContainer recvBuf;
		DataContainer secondaryBuf;
		DataContainer cookie;
		
		// queued-up messages for the client
		GenList<BMessage*> queuedClientMessages;
		
		// what the size of the current packet object *should* be
		unsigned short correctFLAPSize;
		
		// keep track of the flap header
		unsigned short flapRead;
		char flapHeader[6];
		
		// netlets
		nlID authNetlet;
		nlID mainNetlet;
};

//-----------------------------------------------------

// dump all incoming/outgoing packets?
extern bool debugDumpIn;
extern bool debugDumpOut;

//-----------------------------------------------------

#endif
