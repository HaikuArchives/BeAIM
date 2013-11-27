#ifndef _IMPORTER_H_
#define _IMPORTER_H_

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <TextControl.h>
#include <FilePanel.h>
#include <String.h>
#include <CheckBox.h>
#include "GenList.h"
#include "AimBuddyConv.h"
#include "SingleWindowBase.h"

//-----------------------------------------------------

class ImporterConfirmView : public BView
{
	friend class ImporterConfirmWindow;

	public:
		ImporterConfirmView( BRect rect );
		
	private:
		BScrollView* scroll;
		BTextView* textview;
		BCheckBox* replaceBox;
};

//-----------------------------------------------------

class ImporterConfirmWindow : public BWindow
{
	public:
		ImporterConfirmWindow( BRect frame, BWindow* owner, entry_ref, int );
		~ImporterConfirmWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
	
		void RefreshLangStrings();
		void FillList();
		void Save();

		BWindow* owner;
		group* thelist;
		int type;
		entry_ref ref;
		ImporterConfirmView *genView;
};

//-----------------------------------------------------

class ImporterView : public BView
{
	friend class ImporterWindow;

	public:
		ImporterView( BRect rect );
	
	private:
		BButton* closeButton;
		
		BStringView* mainImportLabel;
		BStringView* bltLabel;
		BStringView* otherUserLabel;
		BStringView* gaimLabel;
		
		BButton* importButton1;
		BButton* importButton2;
		BButton* importButton3;
		
		BBox* divider;
};

//-----------------------------------------------------

class ImporterWindow : public SingleWindowBase
{
	public:
		ImporterWindow( BRect frame );
		~ImporterWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void Show();
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
	
		void RefreshLangStrings();
	
		void Import1( BMessage* msg );
		void Import2( BMessage* msg );
		void Import3( BMessage* msg );
		
		bool GetSettingsRef( entry_ref& ref );
		void SetupConfView( entry_ref, int );
		
		ImporterConfirmWindow* confWin;
		ImporterView *genView;
		BFilePanel* panel1;
		BFilePanel* panel2;
		BFilePanel* panel3;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateImporterWindow( BMessage* ) {
	return new ImporterWindow(BRect(0,0,340,169));
};

//-----------------------------------------------------

#endif
