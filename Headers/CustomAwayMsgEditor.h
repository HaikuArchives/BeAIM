#ifndef _CUSTOM_AWAY_MSG_EDITOR_H
#define _CUSTOM_AWAY_MSG_EDITOR_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <TextControl.h>
#include <String.h>
#include "GenList.h"
#include "HTMLView.h"
#include "SingleWindowBase.h"

//-----------------------------------------------------

class CustomAwayMsgView : public BView
{
	friend class CustomAwayMsgWindow;

	public:
		CustomAwayMsgView( BRect rect );
		
	private:
		BStringView* insLabel;
		HTMLView* textview;
		BScrollView* scroll;
 		BButton* btnSave;
		BButton* btnCancel;
};

//-----------------------------------------------------

class CustomAwayMsgWindow : public SingleWindowBase
{
	public:
		CustomAwayMsgWindow( BRect frame );
		~CustomAwayMsgWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:

		void RefreshLangStrings();
		bool Save();
	
		CustomAwayMsgView *genView;
		bool enterIsNewline;
		bool tabIsTab;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateCustomAwayMsgWindow( BMessage* ) {
	return new CustomAwayMsgWindow(BRect(0,0,272,177));
};

//-----------------------------------------------------


#endif
