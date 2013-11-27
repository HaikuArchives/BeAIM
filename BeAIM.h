#ifndef BEAIM_MAIN_H
#define BEAIM_MAIN_H

#include <Application.h>
#include <Window.h>
#include <string.h>
#include "BuddyList.h"
#include "GenList.h"
#include "constants.h"
#include "LoginBox.h"

//-----------------------------------------------------

// main application object
class BeAIMApplication : public BApplication 
{
	public:
		BeAIMApplication();
		~BeAIMApplication();
		virtual void MessageReceived( BMessage* message );
		virtual bool QuitRequested();
		virtual void AboutRequested(); 
		bool InitCheck();
		
	private:

		// these startup and shutdown for the entire app
		void DoBeAIMStartupStuff();
		void DoBeAIMShutdownStuff();
		void DoDeskbarIcon( bool install );
		bool SetDeskbarVisibility( bool vis );
		void DoGlobalPrefs();
		void GotDisconnected( bool quietly=false );
		void PerhapsImport();
		void Logout();

		// timer thingy to send the no-ops every minute
		BMessageRunner* noOpRunner;
		BMessageRunner* typeRunner;
		BMessageRunner* snacRunner;
	
		bool deskbarIconInstalled;
		bool initialized;
};

//-----------------------------------------------------

#endif
