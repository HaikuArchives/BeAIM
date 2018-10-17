#ifndef BUDDY_LIST_H
#define BUDDY_LIST_H

#include <Window.h>
#include <TabView.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <OutlineListView.h>
#include <TextView.h>
#include <PictureButton.h>
#include <Alert.h>
#include <string.h>
#include "LessAnnoyingWindow.h"
#include "BitmapView.h"
#include "NameStatusView.h"
#include "AIMUser.h"
#include "constants.h"
#include "GenList.h"

//=====================================================

class BLBubbleWindow : public BWindow
{
	public:
		BLBubbleWindow();
		~BLBubbleWindow();

		bool IsBeingShown();
		void SetBeingShown( bool, BPoint* = NULL );
		void AddDisplayString( BString );
		void ClearDisplayString();
		void GetMouseInfo( BPoint&, ulong& );
		void GetDimensions( float&, float& );
		void SetNumBoldChars( int );

	private:
		BTextView* textview;
		BString displayString;
		bool isBeingShown;
		float biggestWidth, heightConst;
		int numLines;
		BFont theFont;
};

//=====================================================

class BLGroupItem : public BListItem
{
	public:
		BLGroupItem( BString grp, BFont* font, bool off = false );
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		BString Group();
		void SetGroup( BString );
		void SetCounts( int32 on, int32 ttl );
		int32 OnlineCount()
		{ return online; };
		int32 TotalCount()
		{ return total; };
		bool IsOfflineGroup()
		{ return isOffline; };

	private:
		BFont* ourFont;
		BString groupName;
		BString drawString;
		bool isOffline;
		int32 online, total;
};

//=====================================================

class BLMainItem : public BListItem
{
	public:
		BLMainItem( AIMUser u, BFont* font );
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		AIMUser GetUser();
		void SetUser( AIMUser );
		void SetStatus( int );
		int32 WarningLevel();
		void SetWarningLevel( int32 );

	private:
		void MakeDisplay();

		BFont* ourFont;
		AIMUser user;
		int status;
		int32 warningLevel;
		BString display;

		static BBitmap* awayIcon;
		static BBitmap* alertIcon;
		static BBitmap* blockIcon;
		static BBitmap* enterIcon;
		static BBitmap* exitIcon;
};

//=====================================================

class BuddyListView : public BOutlineListView {

	public:
		BuddyListView( BRect );
		~BuddyListView();
		virtual void MouseDown( BPoint cursor );

	private:
		BPopUpMenu* userPopup;
};

//=====================================================

class BuddyListMainView : public BView
{
	friend class BuddyListWindow;

	public:
		BuddyListMainView( BRect rect );
		virtual void Draw( BRect );

	private:
		BuddyListView* buddylistview;
		BScrollView* scroller;
};

//=====================================================

class BuddyListWindow : public LessAnnoyingWindow
{
	public:
		BuddyListWindow(BRect frame);
		~BuddyListWindow();

		virtual	bool QuitRequested();
		virtual void MessageReceived( BMessage *message );
		virtual void WorkspaceActivated( int32 workspace, bool active );

		void Populate();
		void LoadGroups();
		void Clear();

		void ShowBubble( BPoint, bool, bool forceClose=false );

	private:

		void SetOfflineGroupVisibility( bool );
		void RefreshGroupCounts( bool force );

		void RefreshLangStrings();

		void Verify();

		BLMainItem* FindBuddy( AIMUser user, BString group, bool checkOffline );
		BLMainItem* FindBuddy( AIMUser user );
		void MoveBuddy( AIMUser user );
		void MoveGroup( BString group );
		static BListItem* FindBuddyIter( BListItem*, void* );
		BLGroupItem* FindGroup( BString group );
		static BListItem* FindGroupIter( BListItem*, void* );
		void SetStatus( AIMUser user, int status, int32 wl );
		static BListItem* DoCountsForEachGroup( BListItem*, void* );
		void DoAddBuddy( BMessage* );
		void DoAddGroup( BMessage* );
		void DoDeleteBuddy( BMessage* );
		void DoDeleteGroup( BMessage* );
		void DoChangeGroup( BMessage* );
		void QuickRemoveBuddy( AIMUser );

		void LoadPrefs();
		void LoadAwayMessages();
		void GoAway( BMessage* msg );
		void Invoked();

		// bubble stuff
		void FormatBubble( AIMUser user );
		static int32 StartBubbleWatcher(void *arg);
		void BubbleWatcher();
		thread_id bubbleThread;
		BLBubbleWindow* bubblewin;
		AIMUser lastUserOver;

		GenList<BMenuItem*> awayMenuItems;

		BMenuBar *menubar;
		BitmapView* logoView;
		BMenuItem* availItem;
		BMenuItem* customItem;
		NameStatusView* myName;
		BMenu* statMenu;
		BMenu *peopleMenu;
		BMenu *beMenu;
		BuddyListMainView* genView;
		BuddyListView* budView;
		BLGroupItem* offlineGroup;
		bool showOfflineGroup;
		bool forceCountMode;

		BFont ownerFont;
};

//=====================================================

#endif //BUDDY_LIST_H
