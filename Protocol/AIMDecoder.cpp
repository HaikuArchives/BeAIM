#include "AIMNetManager.h"
#include "AIMConstants.h"
#include "UserManager.h"
#include "Globals.h"
#define _BSD_SOURCE
#include <sys/md5.h>

#define AIM_STRING "AOL Instant Messenger (SM)"

//-----------------------------------------------------

void AIMNetManager::DecodeSNACPacket( SNAC_Object& snac ) {

	printf( "SNAC packet received...  genus: %X   specie: %X\n", snac.genus, snac.specie );

	// send each genus to the appropriate sub-handlers
	switch( snac.genus ) {

		case GENERIC_SERVICE_GENUS:
			DecodeGenericServiceSNAC( snac );
			break;

		case LOCATION_SERVICE_GENUS:
		case USER_LOOKUP_GENUS:
			DecodeUserSNAC( snac );
			break;

		case BUDDY_LIST_GENUS:
			DecodeBuddyListSNAC( snac );
			break;

		case BOS_SPECIFIC_GENUS:
			DecodeBOSSNAC( snac );
			break;

		case ADMINISTRATIVE_GENUS:
			DecodeAdministrativeSNAC( snac );
			break;

		case MESSAGING_GENUS:
			DecodeMessagingSNAC( snac );
			break;

		case SSI_GENUS:
			DecodeSSISNAC(snac);
			break;

		case INVITATION_GENUS:
		case STATS_GENUS:
		case POPUP_NOTICE_GENUS:
			DecodeOtherSNAC( snac );
			break;

		case MD5LOGIN_GENUS:
			DecodeMD5LoginSNAC(snac);
			break;

		default:
			printf( "**********************\nWhoa... Unknown GENUS!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
	}
	printf("done SNACing...\n");
}

//-----------------------------------------------------

void AIMNetManager::DecodeUserSNAC( SNAC_Object& snac ) {

	// snac from the location service
	if( snac.genus == LOCATION_SERVICE_GENUS ) {

		switch( snac.specie ) {

			// error... user not online
			case 0x01:
				printf( "LOCATION_SERVICE_GENUS error! (uh-oh)\n" );
				FailedPersonInfo( snac );
				break;

			// rights response
			case 0x03:
				printf( "location rights response received\n" );
				break;

			// user information
			case 0x06:
				printf( "user information received\n" );
				ReceivePersonInfo( snac );
				break;

			// watcher notification (?)
			case 0x08:
				printf( "watcher notification (?) received\n" );
				break;

			default:
				printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );

		}
	}

	// snac from the user lookup service
	if( snac.genus == USER_LOOKUP_GENUS ) {

		switch( snac.specie ) {

			// error
			case 0x01:
				printf( "USER_LOOKUP_GENUS error! (usually: search failed)\n" );
				ReceiveFailedSearch( snac );
				break;

			// response to search by email address
			case 0x03:
				printf( "response to search by email address\n" );
				ReceiveSearchResults( snac );
				break;

			default:
				printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );

		}
	}

}

//-----------------------------------------------------

void AIMNetManager::DecodeMD5LoginSNAC (SNAC_Object & snac)
{
	short len;
	char * key;
	char phash[16];
	char ahash[16];
	MD5_CTX context;
	SNAC_Object auth;
	DataContainer idInfo = "";
	switch (snac.specie) {
		case 0x03:
			LoginStep( Language.get("LS_AUTH_RECEIVED") );
			ParseLoginResponse(snac.data);
			break;
		case 0x07:
			len = GetWord(snac.data, 0);
			// aieee!
			key = new char[len];
			memcpy(key, snac.data.getrange(2, len).c_ptr(), len);

			printf("sweet!  auth time!  (key length = %d)\n", len);

			MD5Init(&context);
			MD5Update(&context, (const unsigned char*)loginPassword.String(), loginPassword.Length());
			MD5Final((unsigned char*)phash, &context);

			MD5Init(&context);
			MD5Update(&context, (const unsigned char*)key, len);
			MD5Update(&context, (const unsigned char*)phash, 16);
			MD5Update(&context, (const unsigned char*)AIM_STRING, strlen(AIM_STRING));
			MD5Final((unsigned char*)ahash, &context);

			delete key;

			auth.Update(0x17, 0x02, 0, 0, 0x1927);
			auth.data << ToTLV(0x01, loginScreenName.Length(), loginScreenName.String());
			auth.data << ToTLV(0x25, 16, DataContainer((const char *) ahash, 16));
			auth.data << ToTLV(0x4C, 0, "");
//			idInfo << "BeAIM, " << (char *)BeAIMVersion().String();
			idInfo << "AOL Instant Messenger, version 5.1.3036/WIN32";
			auth.data << ToTLV(0x03, idInfo.length(), idInfo);
			auth.data << ToTLV(0x16, 2, ToAIMWord(0x0109));
			auth.data << ToTLV(0x17, 2, ToAIMWord(0x0005));
			auth.data << ToTLV(0x18, 2, ToAIMWord(0x0001));
			auth.data << ToTLV(0x19, 2, ToAIMWord(0x0000));
			auth.data << ToTLV(0x1A, 2, ToAIMWord(0x0BDC));
			auth.data << ToTLV(0x14, 4, ToAIMWord(0x0000) << ToAIMWord(0x00D2));
			auth.data << ToTLV(0x0F, 2, "en");
			auth.data << ToTLV(0x0E, 2, "us");
			auth.data << ToAIMWord(0x004A) << ToAIMWord(0x0001) << DataContainer((unsigned char) 0x01);
//			auth.data << ToTLV(0x4A, 1, (char) 1);
			SendSNACPacket(authNetlet, auth);
			if (loginScreenName.IFindFirst("@") == B_ERROR) {
				snooze(500000);
			} else {
				printf("@mac.com address?\n");
				snooze(1500000);
			}
			break;
		default:
			printf("WTF, mate?  What is SNAC 0x0017/0x%04X?\n", snac.specie);
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeBuddyListSNAC( SNAC_Object& snac ) {

	switch( snac.specie ) {

		// error
		case 0x01:
			printf( "BUDDY_LIST_GENUS error! (uh-oh)\n" );
			break;

		// buddy list rights response
		case 0x03:
			printf( "buddy list rights response\n" );
			break;

		// watcher list (?) response
		case 0x07:
			printf( "Watcher list response\n" );
			break;

		// watcher (?) notification
		case 0x09:
			printf( "Watcher (?) notification\n" );
			break;

		// reject (?) notification
		case 0x0A:
			printf( "reject (?) notification\n" );
			break;

		// oncoming buddy/buddy refresh
		case 0x0B:
			printf( "oncoming buddy\n" );
			BuddyOnline(snac);
			break;

		// offgoing buddy
		case 0x0C:
			printf( "offgoing buddy\n" );
			BuddyOffline(snac);
			break;

		default:
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeMessagingSNAC( SNAC_Object& snac ) {

	BMessage * typeMsg;
	char len;
	DataContainer name;
	switch( snac.specie ) {

		// error
		case 0x01:
			printf( "MESSAGING_GENUS error! (uh-oh)\n" );
			FailedMessage(snac);
			break;

		// ICBM parameter info response
		case 0x05:
			printf( "ICBM parameter info response\n" );
			preLoginPhase = 11;
			DoLogin(NULL);
			break;

		// incoming IM
		case 0x07:
			printf( "incoming IM!\n" );
			ReceiveMessage(snac);
			break;

		// response to a warning-request
		case 0x09:
			printf( "response to a warning-request\n" );
			break;

		// missed messages
		case 0x0A:
			MishMashWarningFunction( 2, snac );
			break;

		// client error (?)
		case 0x0B:
			printf( "client error... huh?\n" );
			break;

		// host acknowledgement
		case 0x0C:
			printf( "Host ack...\n" );
			break;

		// typing notification
		case 0x14:
			printf("whee! typing notification!\n");
			typeMsg = new BMessage(BEAIM_TYPING_STATUS);
			len = snac.data.getrange(10, 1).c_ptr()[0];
			name = snac.data.getrange(11, len);
			name << char(0);
			typeMsg->AddString("userid", name.c_ptr());
			typeMsg->AddInt32("tstat_value", GetWord(snac.data, 11 + len));
			typeMsg->AddInt32("wtype", USER_OTHER_INFO);
			PostAppMessage(typeMsg);
			break;

		default:
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeAdministrativeSNAC( SNAC_Object& snac ) {

	switch( snac.specie ) {

		// error
		case 0x01:
			printf( "ADMINISTRATIVE_GENUS error! (uh-oh)\n" );
			break;

		// information reply
		case 0x03:
			printf( "admin information reply\n" );
			break;

		// change information reply
		case 0x05:
			printf( "admin change information reply\n" );
			break;

		// account confirm reply
		case 0x07:
			printf( "admin account confirm reply\n" );
			break;

		// account delete reply
		case 0x09:
			printf( "admin account delete reply\n" );
			break;

		default:
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeBOSSNAC( SNAC_Object& snac ) {

	switch( snac.specie ) {

		// error
		case 0x01:
			printf( "BOS_SPECIFIC_GENUS error! (uh-oh)\n" );
			break;

		// somebody else logged on as you
		case 0x02:
			MishMashWarningFunction( 1, snac );
			break;

		// BOS rights response
		case 0x03:
			printf( "BOS rights response\n" );
			break;

		// another error
		case 0x09:
			printf( "server BOS error... odd...\n" );
			break;

		default:
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );

	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeOtherSNAC( SNAC_Object& snac ) {

	// response to the "invite a friend" command
	if( snac.genus == INVITATION_GENUS && snac.specie == 0x03 ) {
		printf( "Invite a friend to join AIM acknowledged\n" );
		return;
	}

	// Popup notices
	else if( snac.genus == POPUP_NOTICE_GENUS ) {

		// error
		if( snac.specie == 0x01 ) {
			printf( "POPUP_NOTICE_GENUS error! (uh-oh)\n" );
			return;
		}

		// display popup
		else if( snac.specie == 0x02 ) {
			printf( "server wants to display a popup\n" );
			return;
		}

		else {
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
			return;
		}
	}

	// Stats
	else if( snac.genus == STATS_GENUS ) {

		// error
		if( snac.specie == 0x01 ) {
			printf( "STATS_GENUS error! (uh-oh)\n" );
			return;
		}

		// set minimum report interval
		else if( snac.specie == 0x02 ) {
			printf( "set minimum report interval\n" );
			return;
		}

		// stats report ack
		else if( snac.specie == 0x04 ) {
			printf( "stats report ack\n" );
			return;
		}

		else {
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
			return;
		}
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeSSISNAC(SNAC_Object & snac)
{
	int i, j, k;
	long tim;
	BString name;
	short itemid, groupid, type, len, items;
	AIMUser user;
	SNAC_Object dsnac;
	switch (snac.specie) {
		case 0x0001:
			printf("SSI Error!  Aaagghhh!\n");
			break;
		case 0x0003:
			printf("SSI Rights information!\n");
//			dsnac.Update(0x13, 0x07, 0, 0, 0x1201);
//			SendSNACPacket(mainNetlet, dsnac);
			dsnac.Update(0x13, 0x04, 0x00, 0x00, 0x1202);
			SendSNACPacket(mainNetlet, dsnac);
			break;
		case 0x0006:
			printf("SSI data!\n");
			i = 1;
			printf("SSI version: 0x%02X\n", snac.data[0]);
			items = GetWord(snac.data, i);
			i += 2;
			printf("Items in SSI SNAC: %d\n", items);
			for (j = 0; j < items; j++) {
				name = "";
				len = GetWord(snac.data, i);
				i += 2;
				for (k = 0; k < len; k++)
					name += snac.data[i++];
				groupid = GetWord(snac.data, i);
				i += 2;
				itemid = GetWord(snac.data, i);
				i += 2;
				type = GetWord(snac.data, i);
				i += 2;
				len = GetWord(snac.data, i);
				// ack! ignoring TLV data!!!
				i += 2 + len;
				printf("%d  %-32s\t0x%04X\t0x%04X\t0x%04X\t0x%04X\n", j, name.String(), groupid, itemid, type, len);
				switch (type) {
					case 0x0000:	// buddy
						if (itemid == 0x0000)
							break;
						if (savedGroup == "") {
							savedGroup = "The Squirrels";
							users->AddGroup(savedGroup, -1, 0);
						}
						user = name;
						user.SetSSIUserID(itemid);
						user.SetSSIGroupID(groupid);
						users->AddBuddy(user, savedGroup, false);
						break;
					case 0x0001:	// group
						if (groupid == 0x0000)	// ignore supergroup
							break;
						users->AddGroup(name, -1, groupid);
						savedGroup = name;
						break;
					default:
						printf("WTF is SSI type 0x%04X?\n", type);
				}
			}
//			users->DispatchCommitList();
			dsnac.Update(0x13, 0x07, 0x00, 0x00, 0x1012);
			SendSNACPacket(mainNetlet, dsnac);
			tim = GetWord(snac.data, i) << 16;
			i += 2;
			tim += GetWord(snac.data, i);
			if (tim != 0)
				savedGroup = "";
			printf("SSI last modified: %s\n", asctime(localtime(&tim)));
			break;
		case 0x000E:
			printf("SSI Server Ack.\n");
			/*switch (snac.req_id) {
				case 0x1900:
				case 0x1901:
					dsnac.Update(0x13, 0x08, 0x00, 0x00, 0x1905);
					dsnac.data << ToAIMWord(ssisav.name.Length());
					dsnac.data << ssisav.name.String();
					dsnac.data << ToAIMWord(ssisav.gid);
					dsnac.data << ToAIMWord(ssisav.uid);
					dsnac.data << ToAIMWord(ssisav.type);
					dsnac.data << ToAIMWord(0x0000);
					SendSNACPacket(mainNetlet, dsnac);
					break;
				case 0x1902:
				case 0x1903:
					dsnac.Update(0x13, 0x0A, 0x00, 0x00, 0x1905);
					dsnac.data << ToAIMWord(ssisav.name.Length());
					dsnac.data << ssisav.name.String();
					dsnac.data << ToAIMWord(ssisav.gid);
					dsnac.data << ToAIMWord(ssisav.uid);
					dsnac.data << ToAIMWord(ssisav.type);
					dsnac.data << ToAIMWord(0x0000);
					SendSNACPacket(mainNetlet, dsnac);
					break;
				case 0x1904:
					dsnac.Update(0x13, 0x12, 0x00, 0x00, 0x1905);
					SendSNACPacket(mainNetlet, dsnac);
					ssisav.name = "";
					break;
//				default:
			}*/
			break;
		default:
			printf("Ignoring SSI packet of type 0x%04X (not a good idea?)\n", snac.specie);
	}
}

//-----------------------------------------------------

void AIMNetManager::DecodeGenericServiceSNAC( SNAC_Object& snac ) {

	// in case it's needed...
	DataContainer temp;
	SNAC_Object req;
	int off, toff, i;
	short proto, ver;
	short nrrate, rclass;
	int level;

	switch( snac.specie ) {

		// error
		case 0x01:
			printf( "GENERIC_SERVICE_GENUS error! (uh-oh)\n" );
			break;

		// server ready
		case 0x03:
			off = 0;
			req.Update(0x01, 0x17, 0, 0, 0x1654);
			while (off < snac.data.length()) {
				proto = GetWord(snac.data, off);
				off += 2;
				switch (proto) {
					case 0x0001: ver = 0x0004; break;
					case 0x0013: ver = 0x0004; break;
					case 0x0004: ver = 0x0002; break;
					default: ver = 0x0001;
				}
				printf("protocol %04X supported, asking for version %d\n", proto, ver);
				req.data << ToAIMWord(proto) << ToAIMWord(ver);
			}
			SendSNACPacket(mainNetlet, req);
			break;

		// rate information received
		//
		// right now, we're ignoring this, and that's a BAD IDEA
		case 0x07:
			printf( "Rate information received\n" );
			break;

		// rate violation (bad)
		case 0x0a:
			MishMashWarningFunction( 3, snac );
			break;

		// info on our screen name
		case 0x0f:
			printf( "screen name information received.\n" );
			ParseMyUserInfoPacket( snac.data );
			break;

		// warning
		case 0x10:
			printf( "Someone just warned you!\n" );
			ReceiveWarning( snac );
			break;

		// migration notice
		case 0x12:
			printf( "Migration notice... oscar wants us to move!\n" );
			break;

		// Message of the day (?) server sends this after BOS login...
		case 0x13:
			printf( "...\n" );
			break;

		case 0x18:
			printf("the supported versions, oh my!\n");
			off = 0;
			while (off < snac.data.length()) {
				proto = GetWord(snac.data, off);
				off += 2;
				ver = GetWord(snac.data, off);
				off += 2;
				printf("version %d of protocol %04X supported by server.\n", ver, proto);
			}
			break;

		default:
			printf( "**********************\nUnknown Packet!  Gen: %X	Spc: %X\n**********************", snac.genus, snac.specie );
	}
}


