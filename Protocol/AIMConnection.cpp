#include <ctype.h>
#include <Application.h>
#include "AIMNetManager.h"

//-----------------------------------------------------

void AIMNetManager::SendPacket( nlID nid, DataContainer packet, char channel ) {

	DataContainer sendData;

	// increment the sequence number
	if( sendSequence == 0xFFFF )
		sendSequence = 0;
	else
		++sendSequence;	
	
	// tack on the FLAP header
	sendData << '\x2a';
	sendData << channel;
	sendData << ToAIMWord( sendSequence );
	sendData << ToAIMWord( packet.length() );


	// Now add the packet itself
	sendData << packet;

	// bon voyage...
	Transmit( nid, sendData );
}

//-----------------------------------------------------

void AIMNetManager::SendSNACPacket( nlID nid, SNAC_Object& obj, char channel ) {

	// SNAC's are always send on channel 0x02
	// crazy crazy SNAC rate limiting... but it gets the job done
	// though, I really should do real limiting...
	if (snacPile != -1) snacPile++;
	if ((snacPile > 8) && (nid.type == NLT_MAIN)) {
		printf("SNAC RATE EXCEEDED!\n");
		snooze(1000000);
		snacPile = 0;
	}
	SendPacket( nid, obj.Packet(), channel );
}

//-----------------------------------------------------

void AIMNetManager::Transmit( nlID nid, DataContainer packet ) {

	// dump the packet
	if( debugDumpOut ) {
		printf( "\n[OUTGOING PACKET (%u bytes)]\n", packet.length() );
		Dump( packet );
		printf( "--------------]\n\n" );
	}

	// send the data on its way
	Send( nid, packet );
}


//-----------------------------------------------------

void AIMNetManager::Dump( DataContainer& packet ) {

	char temp1[256];
	char temp2[256];
	int cnt = 0, pad = 0;
	unsigned char c;

	// dump it...
	for( short i = 0; i < packet.length(); ++i ) {

		c = (unsigned char)packet[i];
		sprintf( temp1 + cnt*3 + pad, "%02X%s", c, " " );
		sprintf( temp2 + cnt + (pad >> 1), "%c", isprint(c) ? c : '.' );
		
		if( !((i+1) % 8) && ((i+1) % 16) ) {
			strcat( temp1, "  " );
			strcat( temp2, " " );
			pad = 2;
		}
		if( !((i+1) % 16) ) {
			strcat( temp1, "      " );
			strcat( temp1, temp2 );
			printf( "%s\n", temp1 );
			cnt = pad = 0;
		} else
			++cnt;
	}
	if( packet.length() % 16 ) {
		for( int i = strlen(temp1); i < 56; ++i )
			strcat( temp1, " " );
		strcat( temp1, temp2 );
		printf( "%s\n", temp1 );
	}
}

//-----------------------------------------------------
// The Receive() function assumes that in the event we only get a part of
// the 6-byte FLAP header, then we'll get the remainder in the next packet...
// perhaps this is sloppy programming but that seems like a safe assumption.

char AIMNetManager::Receive( nlID nid ) {

	DataContainer tempBuf;
	unsigned short seq, flapLen;
	char channel;
	
	//secondaryBuf = DataContainer();
	printf( "receive function entered\n" );
	
	// grab any new data that's in the queue
	if( IsDataWaiting(nid) ) {
		Read( nid, tempBuf );
		secondaryBuf << tempBuf;
	}
	
	// bail out for now if we don't have an entire FLAP header
	if( secondaryBuf.length() < 6 )
		return -1;

	// get 2 byte FLAP_id, presumably 0x2aXX
	if( secondaryBuf[0] != 0x2a )
		printf( "BIG WARNING!! FLAP start byte NOT correct! (!= 0x2a)\n" );
	channel = secondaryBuf[1];	
	printf( "start data: %c %c\n", secondaryBuf[0], secondaryBuf[1] );
	
	// get the server's sequence number
	seq = GetWord( secondaryBuf, 2 );
	printf( "sequence number: %u\n", seq );
	
	// check the sequence numbers
	if( !recvSequence )
		recvSequence = seq;
	else if( seq != ++recvSequence ) {
		// odd... this packet was out of order. Oh well.
		printf( "***WARNING: received packet out of order!\n" );
	}
	
	// Get the FLAP data length (the last 2 bytes of the header)
	flapLen = GetWord( secondaryBuf, 4 );
	printf( "FLAP data length: %u\n", flapLen );
	
	// verify that the FLAP data is all there
	if( secondaryBuf.length() < flapLen + 6 )
		return -1;
		
	// copy the packet to recvBuf, and clear it out of secondaryBuf
	recvBuf = secondaryBuf.getrange( 6, flapLen );
	printf( "GOT  data length: %u\n", recvBuf.length() );
	secondaryBuf = secondaryBuf.getrange( flapLen + 6, secondaryBuf.length() - flapLen - 6 );
	
	printf( "[%d bytes remaining in secondary buffer]\n", secondaryBuf.length() );

	// dump the packet
	if( debugDumpIn ) {
		printf( "\n[INCOMING PACKET (%u bytes, no FLAP header)]\n", recvBuf.length() );
		Dump( recvBuf );
		printf( "--------------]\n\n" );
	}

	// since all is apparently well, return the channel...
	return channel;
}
//-----------------------------------------------------
