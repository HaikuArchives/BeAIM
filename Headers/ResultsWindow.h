#ifndef RESULTS_WINDOW_H
#define RESULTS_WINDOW_H

#include <Window.h>
#include <Button.h>
#include <StringView.h>
#include <ListView.h>
#include <ScrollView.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include "SingleWindowBase.h"
#include "GStatusView.h"

//-----------------------------------------------------

const uint32 ENABLE_CONTEXT_BUTTONS = 'eCtx';

//-----------------------------------------------------

class SearchItem : public BStringItem
{
	public:
		SearchItem( char *s, bool f = false ) : BStringItem(s) { fake = f; };
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		bool fake;
};

//-----------------------------------------------------

class ResultsListView : public BListView
{
	public:
		ResultsListView( BRect );
		~ResultsListView();
		virtual void MouseDown( BPoint cursor );
		virtual void SelectionChanged();

	private:
		BPopUpMenu* userPopup;
};

//-----------------------------------------------------

class ResultsView : public BView
{
	friend class SearchResultsWindow;

	public:
		ResultsView( BRect frame, const char *name );

	private:
		BStringView* resLabel;
		ResultsListView* lview;
		BScrollView* container;
		BButton* closeButton;
		BButton* addToListButton;
		BButton* messageButton;
		BButton* infoButton;
};


//-----------------------------------------------------

class SearchResultsWindow : public SingleWindowBase
{
	public:

		SearchResultsWindow( BRect frame );
		virtual	bool QuitRequested();
		virtual void MessageReceived( BMessage* );
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );

	private:

		void AddPerson();
		void GetInfo();
		void SetupNewSearch( BMessage* msg );
		void ParseIncomingResults( BMessage* );
		void RefreshLangStrings();

		ResultsView* resView;
		GStatusView* statView;
		BTextView* resLabel;
		bool empty;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateSearchResultsWindow( BMessage* ) {
	return new SearchResultsWindow(BRect(0,0,360,185));
};

//-----------------------------------------------------

#endif
