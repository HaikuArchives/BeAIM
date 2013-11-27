#ifndef _WINDOW_MANAGER_H_
#define _WINDOW_MANAGER_H_

#include <Window.h>
#include <Message.h>
#include <Locker.h>
#include <Alert.h>
#include "GenList.h"
#include "constants.h"
#include "DataSender.h"
#include "AIMUser.h"
#include "SingleWindowBase.h"
#include "LessAnnoyingWindow.h"

//-----------------------------------------------------

// holds window positions
struct winPosRect {
	AIMUser user;
	BRect frame;
	float divider;
};

//-----------------------------------------------------

// Used to define the list of open IM windows
struct IMWindowItem {
	IMWindowItem() { window = NULL; };
	IMWindowItem( AIMUser sn, int tp, BWindow* w ) {
		name = sn;
		window = w;
		type = tp;
	};
	AIMUser name;
	int type;
	BWindow* window;
};

//-----------------------------------------------------

// used to define a list of single windows
struct singleWindowDefType {
	SingleWindowBase* (*factoryFunction)( BMessage* );
	SingleWindowBase* window;
	int type;
};

//-----------------------------------------------------

// the structure passed to the message-display thread
struct showMessageInfo {
	alert_type alert;
	BString message, closeString;
};

//-----------------------------------------------------

// WindowManager - the class that handles the opening and closing of various
//   BeAIM windows, like the IM Windows.
class WindowManager {

	public:
		WindowManager();

		void Cleanup( bool full=true );
		void Clear();
		
		void OpenBuddyList( BPoint* point = NULL );
		void OpenSignOnWindow( BPoint* point = NULL );
		void CloseSignOnWindow( BPoint* point = NULL );
		void MakeDataSenderWindow();
		
		BWindow* GetBuddyList();
		BWindow* OpenUserWindow( BMessage* message );
		void UserWindowOpened( BMessage* message );
		void UserWindowClosed( BMessage* message );
		void SwitchUserWindow( BMessage* message );
		void ForwardIncomingMessage( BMessage* message );
		void BroadcastMessage( BMessage* message, bool toApp = false );
		
		void OpenSingleWindow( BMessage* msg, int32 whichWindow = 0 );
		void CloseSingleWindow( BMessage* msg );
		void SingleWindowClosed( BMessage* msg );
		void SendSingleWindowMessage( BMessage* msg, int );
		BWindow* GetSingleWindow( int32 whichWindow );

		void SendBuddyListMessage( BMessage* message );
		void SendSignOnMessage( BMessage* message );
		void MakeAddBuddyWindow( AIMUser, BWindow*, bool commitNow, BString group="" );
		
		void MakeDialogFrame( BRect&, BPoint );
		void MakeDialogFrame( BRect&, BWindow* );
		void CenterFrame( BRect& );
		
		void ToggleHidden( bool forceShow );
		
		void SetWindowPos( winPosRect& );
		bool GetNextWindowPos( winPosRect&, bool );
		void ClearWindowPositions();
		void CloseUserWindows( bool all=true );
		void CloseSingleWindows();
		
		void OpenTranscriptSavePanel( AIMUser user, BWindow* target );
		
		void OpenInputWindow( char* title, char* label, BMessage* = NULL, BWindow* window = NULL,
							  char* initial = NULL, int maxLen = 0, bool toApp = false, char* fname = "value" );
							  
		void ShowMessage( BString message, alert_type = B_INFO_ALERT, BString closeString = "--[iOK]--", int32 sound = WS_BEEP, bool async = true );

	private:

		void CorrectFrame( BRect& );
		void CalcNewIMRect();
		void SaveWindowPos( AIMUser user, BRect frame, float divider );
		BWindow* MakeMessageWindow( AIMUser userName );
		BWindow* MakeInfoWindow( AIMUser userName, BWindow* posWindow = NULL );
		void LoadSingleWindowStuff();
		int FindSingleWindow( int type );
	
		GenList<winPosRect> winPositions;
		GenList<singleWindowDefType> singleWindows;
	
		BRect IMWindowRect;
		GenList<IMWindowItem> IMWindows;
		LessAnnoyingWindow* bList;
		BWindow* signOn;
		BFilePanel* transSavePanel;
		BLocker lock;
};

//-----------------------------------------------------

#endif
