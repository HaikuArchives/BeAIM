#ifndef _BEAIM_CONSTANTS_H__
#define _BEAIM_CONSTANTS_H__

// BeAIM messages
const uint32 BEAIM_SIGN_ON					= 'Snon';
const uint32 BEAIM_LOGIN						= 'LoGn';
const uint32 BEAIM_LOGOUT					= 'LoGt';
const uint32 BEAIM_OPEN_IM_WINDOW			= 'Oimw';
const uint32 BEAIM_ATTEMPT_SEND_IM			= 'Sasm';
const uint32 BEAIM_NOT_IMPLEMENTED			= 'Nimp';
const uint32 BEAIM_BUDDY_INVOKED				= 'Bivk';
const uint32 BEAIM_IM_WINDOW_OPENED			= 'Imwo';
const uint32 BEAIM_IM_WINDOW_CLOSED			= 'Imwc';
const uint32 BEAIM_SEND_MESSAGE				= 'SnIm';
const uint32 BEAIM_CANCEL_SIGN_ON			= 'Cnsn';
const uint32 BEAIM_LOGIN_STEP				= 'Sstp';
const uint32 BEAIM_LOGIN_FAILURE				= 'lFlr';
const uint32 BEAIM_ONCOMING_BUDDY			= 'Bdin';
const uint32 BEAIM_OFFGOING_BUDDY			= 'Bdot';
const uint32 BEAIM_BUDDY_STATUS_CHANGE		= 'Bsch';
const uint32 BEAIM_SIGN_ON_SUCCESSFUL		= 'SonS';
const uint32 BEAIM_INCOMING_IM				= 'inIM';
const uint32 BEAIM_SET_MY_SCREEN_NAME		= 'stMy';
const uint32 BEAIM_SET_LOGIN_STEP_COUNT		= 'SlSc';
const uint32 BEAIM_EDIT_BUDDY				= 'eDtB';
const uint32 BEAIM_SIGN_OFF					= 'sNof';
const uint32 BEAIM_OPEN_PREFS				= 'oPrf';
const uint32 BEAIM_RELOAD_PREF_SETTINGS		= 'RlPs';
const uint32 BEAIM_ADD_BUDDY					= '43ap';
const uint32 BEAIM_CHANGE_BUDDY				= '43cp';
const uint32 BEAIM_TRY_ADD_GROUP				= '42ag';
const uint32 BEAIM_ADD_GROUP					= '43ag';
const uint32 BEAIM_TRY_CHANGE_GROUP			= '42cg';
const uint32 BEAIM_CHANGE_GROUP				= '43cg';
const uint32 BEAIM_DELETE_BUDDY				= '42dl';
const uint32 BEAIM_DELETE_GROUP				= '43dl';
const uint32 BEAIM_SEND_NEW_MESSAGE			= 'nMSG';
const uint32 BEAIM_GET_USER_INFO				= 'gPif';
const uint32 BEAIM_GET_AWAY_INFO				= 'gAif';
const uint32 BEAIM_UPDATE_INFO				= 'uINF';
const uint32 BEAIM_UPDATE_AWAY_INFO			= 'aINF';
const uint32 BEAIM_OPEN_USER_WINDOW			= 'oUSw';
const uint32 BEAIM_CLOSE_WINDOW				= 'cLsw';
const uint32 BEAIM_BUDDYLIST_COMMIT			= 'PlsC';
const uint32 BEAIM_OPEN_EMAIL_SEARCH			= 'sE-O';
const uint32 BEAIM_SEARCH_BY_EMAIL			= 'sE-m';
const uint32 BEAIM_TOGGLE_HIDDEN				= 'tGHD';
const uint32 BEAIM_EMAIL_SEARCH_RESULTS		= 'tGsR';
const uint32 BEAIM_ESEARCH_RESULTS_CLOSED	= 'esc!';
const uint32 BEAIM_NEXT_CHAT_WINDOW			= 'nC%s';
const uint32 BEAIM_PREV_CHAT_WINDOW			= 'nC%A';
const uint32 BEAIM_SEND_ARBITRARY_DATA 		= 'aBdt';
const uint32 BEAIM_NOW_IDLE					= 'Dew!';
const uint32 BEAIM_NOW_ACTIVE				= 'Dew?';
const uint32 BEAIM_SEND_IDLE_PULSE			= 'Dew.';
const uint32 BEAIM_TOGGLE_DESKBAR_ICON		= 'SQRL';
const uint32 BEAIM_GOING_AWAY				= 'GkAy';
const uint32 BEAIM_DISCONNECTED				= 'bDcN';
const uint32 BEAIM_OPEN_AWAY_EDITOR			= 'AweT';
const uint32 BEAIM_AWAY_EDITOR_CLOSED		= 'AweC';
const uint32 BEAIM_SCREEN_NAME_UPDATE		= 'snUP';
const uint32 BEAIM_OPEN_SINGLE_WINDOW		= 'SW$c';
const uint32 BEAIM_SINGLE_WINDOW_CLOSED		= 'oSW$';
const uint32 BEAIM_LOAD_AWAY_MENU			= 'lDaM';
const uint32 BEAIM_AWAY_STATUS_CHANGED		= 'bCW]';
const uint32 BEAIM_LOGIN_STATUS_CHANGED		= 'lCW[';
const uint32 BEAIM_UPDATE_NAME_FORMAT		= 'udnf';
const uint32 BEAIM_PERHAPS_IMPORT			= 'p~ip';
const uint32 BEAIM_GOT_WARNED				= 'AHH!';
const uint32 BEAIM_WARN_SOMEONE				= 'wRsM';
const uint32 BEAIM_SET_USER_BLOCKINESS		= 'bloK';
const uint32 BEAIM_JUMP_TO_BUDDYLIST			= 'jBL}';
const uint32 BEAIM_MOVE_BUDDY				= 'mVbY';
const uint32 BEAIM_MOVE_GROUP				= 'mVgP';
const uint32 BEAIM_RELOAD_BUDDYLIST			= 'rLbL';
const uint32 BEAIM_SEND_NOOP					= 'nOoP';
const uint32 BEAIM_CLIENT_READY				= 'rDy!';
const uint32 BEAIM_REFRESH_LANG_STRINGS		= 'rlNg';
const uint32 BEAIM_CHANGE_ENCODING			= 'cEnc';
const uint32 BEAIM_TYPING_STATUS			= 'tYPE';
const uint32 BEAIM_TYPE_PULSE				= 'TyPP';
const uint32 BEAIM_CLEAR_SNACPILE			= 'CSpN';

// Messages for menu commands

const uint32 MENU_FILE_NEW					= 'MFnw';
const uint32 MENU_FILE_OPEN					= 'MFop';
const uint32 MENU_FILE_CLOSE					= 'MFcl';
const uint32 MENU_FILE_SAVE					= 'MFsv';
const uint32 MENU_FILE_SAVEAS				= 'MFsa';
const uint32 MENU_FILE_PAGESETUP				= 'MFps';
const uint32 MENU_FILE_PRINT					= 'MFpr';
const uint32 MENU_FILE_QUIT					= 'MFqu';

// Buddy status: Is someone entering, leaving, idle...?
const int BS_NONE = 0;
const int BS_ENTERING = 1;
const int BS_LEAVING = 2;
const int BS_ACTIVE = 4;
const int BS_IDLE = 8;
const int BS_AWAY = 16;
const int BS_OFFLINE = 32;

// max string sizes
const int INTERNAL_NAME_MAX = 50;
const int DISPLAY_NAME_MAX = 20;
const int USER_NAME_MAX = 20;
const int MESSAGE_MAX = 2048;
const int PASSWORD_MAX = 15;

// the amount of time an item stays bold when it has changed
const int BUDDY_CHANGE_TIME = 15;

// user window modes
const int USER_MESSAGE_TYPE = 1;
const int USER_INFO_TYPE = 2;
const int USER_OTHER_INFO = 3;

// sound events
const int WS_ENTER = 1;
const int WS_EXIT = 2;
const int WS_MSGSEND = 3;
const int WS_MSGRECEIVE = 4;
const int WS_NEWMSG = 5;
const int WS_NOTIDLE = 6;
const int WS_WARNED = 7;
const int WS_BEEP = 8;


// single window constants
const int SW_PROFILE_EDITOR = 1;
const int SW_AWAY_EDITOR = 2;
const int SW_ABOUT_BOX = 3;
const int SW_PREFERENCES = 4;
const int SW_EMAIL_RESULTS = 5;
const int SW_CUSTOM_AWAY_EDITOR = 6;
const int SW_BUDDYLIST_IMPORTER = 7;
const int SW_BLOCKLIST_EDITOR = 8;
const int SW_BUDDYLIST_EDITOR = 9;

// away mode constants
const int AM_NOT_AWAY = 0;
const int AM_STANDARD_MESSAGE = 1;
const int AM_CUSTOM_MESSAGE = 2;

// list editor mode
enum editType {
	ET_ADDBUDDY,
	ET_ADDGROUP,
	ET_BUDDYEDIT,
	ET_GROUPEDIT	
};

// buddy icon type
enum buddyIconType {
	BIT_NONE,
	BIT_AWAY,
	BIT_BLOCK,
	BIT_ALERT
};

// block mode type
enum blockModeType {
	BMT_BLOCK_SOME = 1,
	BMT_ALLOW_SOME = 2,
	BMT_BLOCK_GROUPS = 3,
	BMT_ALLOW_BUDDIES = 4,
	BMT_ALLOW_GROUPS = 5
};

// color thingers...
// pretty much for debug only right now
enum beaimColor {
	BC_NORMAL_GRAY = 1,
	BC_REALLY_LIGHT_GRAY = 2,
	BC_WHITE = 3,
	BC_SELECTION_COLOR = 4,
	BC_GRAY_TEXT = 5
};

// proxy mode constants
enum netProxyMode {
	NPM_NO_PROXY = 0,
	NPM_HTTPS_PROXY,
	NPM_SOCKS5_PROXY
};

// represents the default encoding
const uint32 DEFAULT_ENCODING_CONSTANT = 99999999;

// default position of the window divider
const float DEFAULT_DIVIDER_POS = 0.8;

#endif
