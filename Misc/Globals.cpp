#include "Globals.h"
#include "constants.h"

// The network manager 
//NetworkManager* aimnet;
AIMNetManager * aimnet;

// The Window manager
WindowManager* windows;

// The client manager... the big cheese of BeAIM!
ClientManager* client;

// The user manager... buddylist, block stuff, etc.
UserManager* users;

// prefs manager... storage and stuff
PreferencesManager* prefs;

// BeAIM's noisemakers  :-)
classSoundMaster* sounds;

// vars that should be somewhere else
char AppFileName[1024];

// testing/debug mode
bool beaimDebug = false;

// official or beta release?
bool beaimBeta = false;
