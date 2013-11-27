#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include <Window.h>
#include <Button.h>
#include <StringView.h>
#include <ScrollView.h>
#include "FancyView.h"
#include "GStatusView.h"
#include "constants.h"
#include "AIMUser.h"

//-------------------------------------------------------------------------

class InfoView : public BView 
{
	friend class InfoWindow;

	public:
		InfoView( bool isBuddy, BRect frame, const char *name, BMessage* rqm, uint32 enc );
		void AttachedToWindow();
		void UpdateDisplay( BMessage* );
		void UpdateAwayDisplay( BMessage* );
		
	private:
	
		BButton* closeButton;
		BButton* addToListButton;
		BButton* messageButton;
		
		BStringView *label1, *label2;
		BStringView *label3, *label4;
		BStringView *label5, *label6;
		BStringView *label7, *label8;
		BStringView *label9, *label10;
		
		FancyTextView* profile;
		BBox* infoBox;
		BScrollView* profHolder;	
		FancyTextView* awaymessage;
		BScrollView* awayHolder;
		BStringView* profileLabel;
		BStringView* awayLabel;
		BMessage* reqMsg;
		bool isBuddy;
		bool statNotIdle;
		uint32 encoding;
};

//-------------------------------------------------------------------------

class InfoWindow : public BWindow 
{
	public:
		InfoWindow( BRect frame, AIMUser userName );
		virtual	bool QuitRequested();
		virtual void MessageReceived( BMessage* );
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
				
	private:

		void RefreshLangStrings();
		void AddPerson();
		void EnableAwayMode();
		void AskAway();

		InfoView* iView;
		GStatusView* statView;		
		bool gotInfo;
		bool gotAway;
		bool needsAway;
		bool askedAway;
		AIMUser userName;
};

//-------------------------------------------------------------------------

#endif
