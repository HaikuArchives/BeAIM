#ifndef _PEOPLE_EDIT_H
#define _PEOPLE_EDIT_H

#include <Window.h>
#include <Button.h>
#include <RadioButton.h>
#include <TextControl.h>
#include <MenuField.h>
#include "UserManager.h"
#include "AIMUser.h"
#include "constants.h"

const uint32 ADDEDIT_SELECTION_CHANGED = 'aesc';

//-----------------------------------------------------

class PersonAddEditView : public BView 
{
	friend class PersonAddEditWindow;

	public:
		PersonAddEditView( BRect rect, uint32 id, BString grp = "" );
		virtual void KeyDown(const char *bytes, int32 numBytes);

	private:
		BMenuField* menuField;
		BMenu* groupMenu;
		BTextControl* personName;
		BButton* SaveButton;
		BButton* CancelButton;
		uint32 msgID;		
};

//-----------------------------------------------------

// The Window class
class PersonAddEditWindow : public BWindow
{
	public:
		PersonAddEditWindow( BRect frame, BWindow* mk, AIMUser user, BString group, editType mode );
		~PersonAddEditWindow();

		virtual	bool QuitRequested();
		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
		
		void SetInitialName( BString name );		
		bool Save();
		void SetNow( bool n ) {
			now = n;
		}

	private:
	
		void RefreshLangStrings();
	
		bool now;
		PersonAddEditView *personView;
		AIMUser user;
		BString group;
		uint32 msgID;
		BWindow* maker;
};

//-----------------------------------------------------

#endif
