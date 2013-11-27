#ifndef _AIM_CONSTANTS_H_
#define _AIM_CONSTANTS_H_

// SNAC data genus constants
const unsigned short GENERIC_SERVICE_GENUS = 0x0001;
const unsigned short LOCATION_SERVICE_GENUS = 0x0002;
const unsigned short BUDDY_LIST_GENUS = 0x0003;
const unsigned short MESSAGING_GENUS = 0x0004;
const unsigned short ADVERTISEMENTS_GENUS = 0x0005;
const unsigned short INVITATION_GENUS = 0x0006;
const unsigned short ADMINISTRATIVE_GENUS = 0x0007;
const unsigned short POPUP_NOTICE_GENUS = 0x0008;
const unsigned short BOS_SPECIFIC_GENUS = 0x0009;
const unsigned short USER_LOOKUP_GENUS = 0x000A;
const unsigned short STATS_GENUS = 0x000B;
const unsigned short TRANSLATION_GENUS = 0x000C;
const unsigned short CHAT_NAVIGATION_GENUS = 0x000D;
const unsigned short CHAT_GENUS = 0x000E;
const unsigned short SSI_GENUS = 0x0013;
const unsigned short MD5LOGIN_GENUS = 0x0017;
const unsigned short UNKNOWN_GENUS = 0x0045;

// max screen name size is 32, but in practice, AOL only allows 16...
// future expandability, maybe? In any case, we'll go the full route... :-)
const int MAX_SCREEN_NAME_SIZE = 32;

// Login constants
#define AIM_LOGIN_SERVER "login.oscar.aol.com"
#define AIM_LOGIN_PORT 5190

// capabilities block
const int CAPS_BUDDYICON = 0x01;
const int CAPS_VOICE = 0x02;
const int CAPS_IMIMAGE = 0x04;
const int CAPS_CHAT = 0x08;
const int CAPS_GETFILE = 0x10;
const int CAPS_SENDFILE = 0x20;

//-----------------------------------------------------
// global AIM data stuff

// holds capabilities block definitions
extern unsigned char AIMCaps[6][16];

#endif
