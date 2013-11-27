#ifndef LOGIN_BOX_H
#define LOGIN_BOX_H

#include <Window.h>
#include <Button.h>
#include <Box.h>
#include <CheckBox.h>
#include <TextControl.h>
#include <StringView.h>
#include <StatusBar.h>
#include <MessageFilter.h>
#include <MenuField.h>
#include "PassControl.h"
#include "BitmapView.h"
#include "ColorView.h"
#include "constants.h"
#include "AIMUser.h"

const uint32 PEOPLE_SELECTOR_MSG = 'psbs';
const uint32 LOGIN_COUNTDOWN_MSG = 'BOOM';
const uint32 LOGIN_STOP_COUNTDOWN = 'sTc?';
const uint32 LOGIN_SAVE_PW_CHECKBOX = 'sPwC';
const uint32 LOGIN_AUTOLOGIN_CHECKBOX = 'saLC';
const uint32 LOGIN_EDIT_PW = 'sEwP';

//=====================================================================

// The LoginView class
class LoginView : public BView 
{
	friend class LoginWindow;	// kludge

	public:
		LoginView(BRect rect);
		void EnableControls( bool );

	private:
		void MakeUserList();
		void LoadUserSettings( int32 index );
		void LoadUserSettings( AIMUser userName );
		void SaveUserSettings( AIMUser userName, bool saveNewUser=false );
		void AdjustPreLoginSettings();
	
		PassControl* Password;
		BCheckBox* SavePassword;
		BCheckBox* AutoLogin;
		BTextControl* ScreenName;
		BButton* SignOnButton;
		BButton* SetupButton;
		BMenuField* userList;
		BMenu* userMenu;
};

//=====================================================================

// The LoginProgressView class
class LoginProgressView : public BView 
{
	friend class LoginWindow;	// kludge

	public:
		LoginProgressView(BRect rect);

	private:
		BButton* CancelButton;
		BStringView* Caption;
		BStatusBar* Progress;
};

//=====================================================================

// The LoginCountdownView class
class LoginCountdownView : public BView 
{
	friend class LoginWindow;	// kludge

	public:
		LoginCountdownView(BRect rect);

	private:
		
		// counter thread stuff
		void DoCountdown();
		void StopCountdown();
		void Tick();
		static int32 counter_thread_entry( void *arg );
		int32 do_count();
		thread_id count_thread;
	
		BButton* cancelButton;
		BStringView* Caption;
		BBox* surroundBox;
		int counter;
};

//=====================================================================

// The Window class
class LoginWindow : public BWindow 
{
	public:
		LoginWindow(BRect frame);
		~LoginWindow();
		
		virtual	bool QuitRequested();
		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
		
		void SetCountdownMode( bool );
		void SetProgressMode( bool );
		void DoLogin();		

	private:
		void LoadLoginSettings();
		void SaveLoginSettings( bool saveNewUser=false );		
		void RefreshLangStrings();
	
		LoginView* logView;
		BitmapView* bview;
		LoginProgressView* progressView;
		LoginCountdownView* countView;
		EnStringView* versionLabel;
};

//=====================================================================

#endif //LOGIN_BOX_H
