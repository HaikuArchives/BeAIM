#ifndef _AWAY_EDITOR_H
#define _AWAY_EDITOR_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <TextControl.h>
#include <String.h>
#include "GenList.h"
#include "SingleWindowBase.h"

//-----------------------------------------------------

class AwayTextView : public BTextView {
	public:
		AwayTextView( BRect, const char*, BRect, uint32, uint32 );
		void InsertText( const char* text, int32 length,
						 int32 offset, const text_run_array *runs );
		void DeleteText(int32 start, int32 finish);
		bool IsDirty() { return isDirty; };
		void MakeDirty( bool );
		void SetText( const char* );
	private:
		bool isDirty;
};

//-----------------------------------------------------

class AwayEditorView : public BView
{
	friend class AwayEditorWindow;

	public:
		AwayEditorView( BRect rect );
		
	private:
		AwayTextView* textview;
		BListView* listview;
		BScrollView* scroll;
		BScrollView* scroll2;
		BBox* messageBox;
		BTextControl* messageName;
		BButton* btnNew;
		BButton* btnSave;
		BButton* btnDelete;
		BStringView* messagesLabel;
};

//-----------------------------------------------------

class AwayEditorWindow : public SingleWindowBase
{
	public:
		AwayEditorWindow( BRect frame );
		~AwayEditorWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
	
		void RefreshLangStrings();		
		void EnableMessageStuff( bool );
		void LoadMessage( BString name );
		void NewMessage();
		void DeleteMessage();
		bool CheckMessage();
		bool SaveMessage();
		void BuildList();
		void Revert();
	
		AwayEditorView *genView;
		BString currentMsg;
		bool isDirty, hasSelection;
		bool enterIsNewline;
		bool tabIsTab;
};

//-----------------------------------------------------

class AwayItem : public BListItem
{ 
	public: 
		AwayItem( BString n );
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		BString GetName();
		void SetName( BString );

	private: 
		BString name;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateAwayEditorWindow( BMessage* ) {
	return new AwayEditorWindow(BRect(0,0,389,200));
};

//-----------------------------------------------------

#endif
