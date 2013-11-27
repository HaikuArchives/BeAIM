#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <String.h>
#include "GenList.h"
#include "WindowManager.h"
#include "ClientManager.h"
#include "UserManager.h"
#include "PrefsManager.h"
#include "classSoundMaster.h"
#include "NetManager.h"
#include "AIMNetManager.h"
#include "DLanguageClass.h"

//extern NetworkManager* aimnet;
extern AIMNetManager * aimnet;
extern WindowManager* windows;
extern ClientManager* client;
extern classSoundMaster* sounds;
extern UserManager* users;
extern PreferencesManager* prefs;
extern char AppFileName[1024];
extern bool beaimDebug;
extern bool beaimBeta;

#endif
