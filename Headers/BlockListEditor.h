#ifndef _BLOCKLIST_EDITOR_H
#define _BLOCKLIST_EDITOR_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <String.h>
#include "AIMUser.h"
#include "GenList.h"
#include "SingleWindowBase.h"

class BlockItem : public BStringItem
{ 
	public: 
		BlockItem( AIMUser u ) : BStringItem(u.UserString()) {};
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
};

class BlockListEditorView : public BView
{
	friend class BlockListEditorWindow;

	public:
		BlockListEditorView( BRect rect );
		
	private:
		BListView* list;
		BScrollView* scroll;
 		BButton* addButton;
		BButton* removeButton;
		BButton* closeButton;
};


class BlockListEditorWindow : public SingleWindowBase
{
	public:
		BlockListEditorWindow( BRect frame );
		~BlockListEditorWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
		
		void DoLoad();
		void DoRemove();
		void DoAdd();
		void DoRealAdd( BMessage* msg );
		void HandleExternalBlockiness( BMessage* msg );
		void RefreshLangStrings();
	
		BlockListEditorView *genView;
};

//-----------------------------------------------------

const BRect blRegRect = BRect(0,0,300,127);

// factory function
static SingleWindowBase* CreateBlockListEditorWindow( BMessage* ) {
	return new BlockListEditorWindow(blRegRect);
};

//-----------------------------------------------------

#endif
