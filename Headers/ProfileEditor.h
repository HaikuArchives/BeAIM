#ifndef _PROFILE_EDITOR_H
#define _PROFILE_EDITOR_H

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

class ProfileEditorView : public BView
{
	friend class ProfileEditorWindow;

	public:
		ProfileEditorView( BRect rect );
		
	private:
		HTMLView* textview;
		BScrollView* scroll;
 		BButton* btnSave;
		BButton* btnCancel;
		BStringView* label;
};

//-----------------------------------------------------

class ProfileEditorWindow : public SingleWindowBase
{
	public:
		ProfileEditorWindow( BRect frame );
		~ProfileEditorWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
	
		void Save();
		void RefreshLangStrings();
	
		ProfileEditorView *genView;
		bool enterIsNewline;
		bool tabIsTab;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateProfileEditorWindow( BMessage* ) {
	return new ProfileEditorWindow(BRect(0,0,272,177));
};

//-----------------------------------------------------


#endif
