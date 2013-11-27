#ifndef _BUDDY_LIST_EDITOR_H_
#define _BUDDY_LIST_EDITOR_H_

#include <Window.h>
#include <ScrollView.h>
#include <OutlineListView.h>
#include <Box.h>
#include <TextControl.h>
#include <Bitmap.h>
#include <String.h>
#include "NameStatusView.h"
#include "GenList.h"
#include "AIMUser.h"
#include "SingleWindowBase.h"
#include "constants.h"

//-----------------------------------------------------

class BLSetupItem : public BListItem
{ 
	public: 
		BLSetupItem( AIMUser u, BFont font, bool ig = false );
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		virtual void Update( BView *owner, const BFont *font );
		AIMUser GetUser();
		void SetUser( AIMUser );
		bool IsGroupItem()
		{ return isGroup; };

	private: 
		float rightHeight;
		BFont ourFont;
		bool isGroup;
		AIMUser user;
};

//-----------------------------------------------------

class BListSetupView : public BOutlineListView {

	public:
		BListSetupView( BRect, const char*, list_view_type = B_SINGLE_SELECTION_LIST,
			uint32 = B_FOLLOW_ALL_SIDES,
			uint32 = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE );
		~BListSetupView();

		virtual bool InitiateDrag( BPoint point, int32 index, bool wasSelected );
		virtual void MouseMoved( BPoint point, uint32 transit, const BMessage* message );
		virtual void TargetedByScrollView(BScrollView *view);
		virtual void MouseUp( BPoint point );
		virtual void SelectionChanged();

		void Clear();
	
	private:
	
		void Verify();
	
		void DrawInsertionLine( int32 pos, bool on );
		void GroupsCanGoWhere( int32 movingWhat );
		bool IsAllowed( int32 pos );
		
		int32 curSel, oldItemOver, startItem;
		rgb_color kDrag, kWhite, kSel;
		bool dragging;
		bool groupDrag;
		GenList<int32> goodGroupPositions;
		BScrollView* scroller;
};

//-----------------------------------------------------

class ListSetupView : public BView
{
	friend class BuddyListSetupWindow;

	public:
		ListSetupView( BRect rect );
		
	private:
		BListSetupView* listview;
		BScrollView* scroll;
		BButton* addGroupButton;
		BButton* addBuddyButton;
		BButton* editButton;
		BButton* deleteButton;
		BButton* importButton;
		BButton* doneButton;
		NameStatusView* statView;
};

//-----------------------------------------------------

class BuddyListSetupWindow : public SingleWindowBase
{
	public:
		BuddyListSetupWindow( BRect frame, bool openImporter );
		~BuddyListSetupWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		void Populate();

	private:
	
		void RefreshLangStrings();
	
		BLSetupItem* GetCurrentItem();
		void DoSelectionEnabling( int type );
		void DoEdit();
		void DoDelete();
		void DoAddGroup();
		void DoAddBuddy();
		void DoImport();
		void DoExport();
		void DoRealAddGroup( BMessage* msg );
		void DoRealAddBuddy( BMessage* msg );
		void DoRealChangeBuddy( BMessage* msg );
		void DoRealChangeGroup( BMessage* msg );
		void DoRealDeleteBuddy( BMessage* msg );
		void DoRealDeleteGroup( BMessage* msg );

		ListSetupView *genView;
		BFont ourFont;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreateBuddyListSetupWindow( BMessage* msg ) {
	return new BuddyListSetupWindow(BRect(0,0,311,282), msg->HasBool("doimport"));
};

//-----------------------------------------------------

#endif
