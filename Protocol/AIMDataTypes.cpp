#include <stdio.h>
#include <ctype.h>
#include "AIMDataTypes.h"
#include "AIMConstants.h"

//=========================================================================

SNAC_Object::SNAC_Object()
{
	genus = 0;
	specie = 0;
	flag[0] = 0;
	flag[1] = 0;
	req_id = 0;
	data = "";
}

//-------------------------------------------------------------------------

SNAC_Object::SNAC_Object( unsigned short g, unsigned short s, char f1, char f2, long req )
{
	Update( g, s, f1, f2, req );
}

//-------------------------------------------------------------------------

SNAC_Object::SNAC_Object( DataContainer s )
{
	Decode( s );
}

//-------------------------------------------------------------------------

void SNAC_Object::Update( unsigned short g, unsigned short s, char f1, char f2, long req )
{
	genus = g;
	specie = s;
	flag[0] = f1;
	flag[1] = f2;
	req_id = req;
	data = "";
}

//-------------------------------------------------------------------------

void SNAC_Object::Decode( DataContainer s )
{
	genus = (s[0] << 8) + s[1];
	specie = (s[2] << 8) + s[3];
	flag[0] = s[4];
	flag[1] = s[5];
	req_id = (s[6] << 24) + (s[7] << 16) + (s[8] << 8) + s[9];
	
	data = s.getrange(10);
}

//-------------------------------------------------------------------------

DataContainer SNAC_Object::Packet()
{
	DataContainer r;

	r << char(0) << char(genus);
	r << char(0) << char(specie);
	r << flag[0] << flag[1];
	r << char(req_id >> 24) 
	  << char(req_id >> 16)
	  << char(req_id >> 8)
	  << char(req_id);
	r << data;

	return r;
}

//-------------------------------------------------------------------------

void SNAC_Object::Dump() {
	
	DataContainer packet = Packet();
	
	// print it all
	for( short i = 0; i < packet.length(); ++i ) {
		printf( "%d%c", (int)packet[i], char(9) );
		if( !((i+1) % 8) && i != packet.length()-1 )
			printf( "\n" );
	}
	printf( "\n" );
}

//=========================================================================
