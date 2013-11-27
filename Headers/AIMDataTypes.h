#ifndef _AIM_DATATYPES_H_
#define _AIM_DATATYPES_H_

// Note:
// Some of this code was stolen from laim. Some bits are original.
// I'm publishing it all, so I hope there are no license issues...

#include "DataContainer.h"

//=========================================================================

class SNAC_Object {

	public:
	
		// makes a blank SNAC_Object, to be fiddled with later
		SNAC_Object();
	
		// constructs a SNAC_Object w/ all the relevant fields filled in
		SNAC_Object( unsigned short, unsigned short, char = 0, char = 0, long = 0 );
		
		// pulls out its own data
		SNAC_Object( DataContainer );
		
		// Updates the header data in this object
		void Update( unsigned short, unsigned short, char = 0, char = 0, long = 0 );
		
		// Pulls data out of the passed string
		void Decode( DataContainer );
		
		// returns SNAC packet, w/ proper header
		DataContainer Packet();
		
		// returns the various elements of this object
		unsigned short GetGenus() { return genus; };
		unsigned short GetSpecie() { return specie; };
		unsigned short GetRequestID() { return req_id; };

		// for debugging
		void Dump();
		

		// These should probably be private, but for ease of use they're gonna stay public

		unsigned short genus;		// family 2bytes
		unsigned short specie;		// subtype 2bytes
		unsigned char flag[2];		// optional flags 2bytes
		long req_id;				// request ID ?? 4 bytes

		DataContainer data;			// packet data
};

//=========================================================================

#endif
