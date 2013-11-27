#include "AIMNetManager.h"

//-----------------------------------------------------

bool AIMNetManager::ProcessMainServerCommands() {

	printf( "processCommands, baby, yeah!\n" );

	// Get the actual data
	char channel = Receive( mainNetlet );
	SNAC_Object snac;

	// Make a SNAC object out of the received data, and decode it
	while( channel != -1 ) {
		printf( "processing loop entered\n" );
		snac = SNAC_Object( recvBuf );
		DecodeSNACPacket( snac );
		channel = Receive( mainNetlet );
		printf( "processing loop: channel is %d\n", int(channel) );
	}
		
	return false;
}

//-----------------------------------------------------

bool AIMNetManager::ProcessAuthServerCommands() {

	printf( "processCommands, baby, yeah!\n" );

	// Get the actual data
	char channel = Receive( authNetlet );
	SNAC_Object snac;

	printf( "processing loop: channel is %d\n", int(channel) );
	// Make a SNAC object out of the received data, and decode it
	while( channel != -1 ) {
		printf( "processing loop entered\n" );
		snac = SNAC_Object( recvBuf );
		DecodeSNACPacket( snac );
		printf("hate!\n");
		channel = Receive( authNetlet );
		printf( "processing loop: channel is %d\n", int(channel) );
	}
		
	return false;
}

//-----------------------------------------------------

DataContainer AIMNetManager::ToAIMWord( unsigned short word ) {

	DataContainer ret = "";
	ret << char( word >> 8 );
	ret << char( word & 0xFF );
	
	return ret;
}

//-----------------------------------------------------

DataContainer AIMNetManager::ToTLV( unsigned short id, unsigned short len, DataContainer data )
{
	// Make a DataContainer containing a TLV
	DataContainer ret = "";
	ret << ToAIMWord( id );
	ret << ToAIMWord( len );
	ret << data;

	return ret;
}

//-----------------------------------------------------

// assumes a 2-byte input
unsigned short AIMNetManager::GetWord( char* input ) {
	
	unsigned short ret = 0;

	// Get the correct byte order
	ret =  ( (( (unsigned short) input[0]) & 0xFF) << 8);
	ret +=    ( (unsigned short) input[1]) & 0xFF;
	
	return ret;
}

//-----------------------------------------------------

// i is the value at which the word starts in input
unsigned short AIMNetManager::GetWord( DataContainer input, short i ) {
	
	unsigned short ret = 0;	
	
	// Get the correct byte order
	ret =  ( (( (unsigned short) input[i+0]) & 0xFF) << 8);
	ret +=    ( (unsigned short) input[i+1]) & 0xFF;
	
	return ret;
}

//-----------------------------------------------------

void AIMNetManager::LogMessageReqID( AIMUser userID, int type ) {

	userRequestID rid;

	printf( "Logging Message RID: %d\n", requestID );

	// if the list has 20+ items in it, shuffle one of 'em out
	if( pendingNRRequests.Count() >= 20 )
		pendingNRRequests.Dequeue( rid );

	// add an object to the pending list
	rid.userid = userID;
	rid.reqID = requestID++;
	rid.type = type;
	pendingNRRequests.Enqueue( rid );
}

//-----------------------------------------------------

void AIMNetManager::SetTypingStatus(AIMUser user, short status)
{
	SNAC_Object snac;
	printf("typing status to %s: %04X\n", user.UserString(), status);
	snac.Update(0x04, 0x14, 0, 0, 0x3466);
	snac.data << ToAIMWord(0) << ToAIMWord(0) << ToAIMWord(0) << ToAIMWord(0);
	snac.data << ToAIMWord(0x0001);
	snac.data << DataContainer((unsigned char) user.Username().Length());
	snac.data << user.UserString();
	snac.data << ToAIMWord(status);
	SendSNACPacket(mainNetlet, snac);
}
