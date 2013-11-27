// BeAIM - BuddyList.cpp
// The Buddy List module

#include <Application.h>
#include <View.h>
#include <TabView.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Alert.h>
#include <ScrollView.h>
#include <PopUpMenu.h>
#include <SupportKit.h>
#include <stdlib.h>
#include <stdio.h>
#include <Screen.h>
#include "BuddyList.h"
#include "MiscStuff.h"
#include "DLanguageClass.h"
#include "Say.h"
#include "Globals.h"

//=====================================================

const int LOGO_HEIGHT = 40;
const int STATUS_HEIGHT = 13;
const uint32 SHOW_BUBBLE = 'shBL';
const uint32 HIDE_BUBBLE = 'hdBL';

BBitmap* BLMainItem::awayIcon = NULL;
BBitmap* BLMainItem::alertIcon = NULL;
BBitmap* BLMainItem::blockIcon = NULL;
BBitmap* BLMainItem::enterIcon = NULL;
BBitmap* BLMainItem::exitIcon = NULL;

//=====================================================

BuddyListWindow::BuddyListWindow(BRect frame)
			   : LessAnnoyingWindow(frame, "Buddy List", B_DOCUMENT_WINDOW_LOOK,
			   						B_NORMAL_WINDOW_FEEL, B_NOT_ZOOMABLE )
{
	BRect r; 
	BBitmap* smallLogo = NULL;
	BMenuItem* menuItem;
	BMessage* tmpMessage;
	r = Bounds();	
	
	// font stuff
	ownerFont = be_plain_font;
	ownerFont.SetSize(12.0);
	ownerFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );

	// offline group stuff
	showOfflineGroup = false;
	offlineGroup = NULL;
	
	// set the min/max sizes for this window
	SetSizeLimits( 152, 5000, 120, 5000 );	
	
	// Add the menu bar stuff
	menubar = new BMenuBar(r, "menu_bar");
	beMenu = new BMenu("BeAIM");
	tmpMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	tmpMessage->AddInt32( "wtype", SW_PREFERENCES );
 	beMenu->AddItem(menuItem = new BMenuItem("Preferences" B_UTF8_ELLIPSIS, tmpMessage, 'P' ));
	menuItem->SetTarget(BeAIMAppSig().String());
	tmpMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	tmpMessage->AddInt32( "wtype", SW_PROFILE_EDITOR );
	beMenu->AddItem(menuItem = new BMenuItem("Edit Profile" B_UTF8_ELLIPSIS, tmpMessage, 'F' ));
	menuItem->SetTarget(BeAIMAppSig().String());
	beMenu->AddSeparatorItem();
	beMenu->AddItem(menuItem=new BMenuItem("About BeAIM" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED), 'A' ));
	menuItem->SetTarget(BeAIMAppSig().String());
	beMenu->AddSeparatorItem();
	beMenu->AddItem(menuItem=new BMenuItem("Logout", new BMessage(BEAIM_LOGOUT), 'L' ));
	menuItem->SetTarget(BeAIMAppSig().String());
	beMenu->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q' ));

	peopleMenu = new BMenu("People");
	peopleMenu->AddItem(new BMenuItem("Contact User" B_UTF8_ELLIPSIS, new BMessage(BEAIM_SEND_NEW_MESSAGE), 'C' ));
	peopleMenu->AddItem(new BMenuItem("Get User Info" B_UTF8_ELLIPSIS, new BMessage(BEAIM_GET_USER_INFO), 'I' ));
	peopleMenu->AddItem(new BMenuItem("Search by email address" B_UTF8_ELLIPSIS, new BMessage(BEAIM_SEARCH_BY_EMAIL), 'E' ));
	peopleMenu->AddSeparatorItem();
	tmpMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	tmpMessage->AddInt32( "wtype", SW_BUDDYLIST_EDITOR );
	peopleMenu->AddItem(menuItem=new BMenuItem("Edit Buddy List" B_UTF8_ELLIPSIS, tmpMessage, 'B') );
	menuItem->SetTarget(BeAIMAppSig().String());
	tmpMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	tmpMessage->AddInt32( "wtype", SW_BLOCKLIST_EDITOR );
	peopleMenu->AddItem(menuItem=new BMenuItem("Edit Blocked Users" B_UTF8_ELLIPSIS, tmpMessage, 'K') );
	menuItem->SetTarget(BeAIMAppSig().String());
	
	statMenu = new BMenu( "Away" );
	BMessage* awMsg = new BMessage(BEAIM_GOING_AWAY);
	awMsg->AddBool( "away", false );
	availItem = new BMenuItem("Available", awMsg, 'A');
	availItem->SetMarked(true);
	statMenu->AddItem( availItem );
	statMenu->AddSeparatorItem();
	
	// Load the away messages from the PrefsManager
	LoadAwayMessages();
	
	// Add the 'custom' item
	BMessage* custMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	custMessage->AddInt32( "wtype", SW_CUSTOM_AWAY_EDITOR );
	customItem = new BMenuItem( "Custom Away Message" B_UTF8_ELLIPSIS, custMessage, 'C' );
	customItem->SetTarget(BeAIMAppSig().String());
	statMenu->AddItem( customItem );
	
	// add the editor item
	awMsg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	awMsg->AddInt32( "wtype", SW_AWAY_EDITOR );
	statMenu->AddSeparatorItem();
	statMenu->AddItem(menuItem=new BMenuItem("Edit Away Messages" B_UTF8_ELLIPSIS, awMsg, 'M'));
	menuItem->SetTarget(BeAIMAppSig().String());
	menubar->AddItem(beMenu);
	menubar->AddItem(peopleMenu);
	menubar->AddItem(statMenu);
	AddChild(menubar);	

	// Add the logo thinger
	r.top = menubar->Bounds().bottom + 1.0;
	r.bottom = r.top + LOGO_HEIGHT;
	GetBitmapFromResources( smallLogo, 942 );
	logoView = new BitmapView( r, smallLogo );
	AddChild( logoView );

	// Set up the actual buddy list view
	r = Bounds();
	r.top = menubar->Bounds().bottom +logoView->Bounds().bottom + 2.0;
	r.bottom -= (STATUS_HEIGHT+1);
	genView = new BuddyListMainView( r );
	budView = genView->buddylistview;
	AddChild( genView );
	
	// add the kewl name view
	r = Bounds();
	r.top = r.bottom - STATUS_HEIGHT;
	r.right -= 14;
	myName = new NameStatusView( r );
	myName->SetName( client->User() );
	myName->SetWarningLevel( client->WarningLevel() );
	AddChild( myName );
	
	// add shortcuts to handle prev and next commands from the buddylist
	AddShortcut( ',', 0, new BMessage(BEAIM_PREV_CHAT_WINDOW) );
	AddShortcut( '.', 0, new BMessage(BEAIM_NEXT_CHAT_WINDOW) );
	
	// add the "secret" SendData shortcut
	AddShortcut( '\\', B_SHIFT_KEY, new BMessage(BEAIM_SEND_ARBITRARY_DATA) );
	
	// load the prefs and the groups and the language strings
	LoadPrefs();
	LoadGroups();
	RefreshLangStrings();
	
	// create the bubble and start the bubble-watcher thread
	bubblewin = new BLBubbleWindow();
	bubblewin->Run();
	bubbleThread = spawn_thread( StartBubbleWatcher, "BubbleWatcher", B_NORMAL_PRIORITY, this );
	if( bubbleThread >= 0 )
		resume_thread(bubbleThread);
	
	// finally, let the netcode know that we're ready to receive messages
	PostAppMessage( new BMessage(BEAIM_CLIENT_READY) );
}

//-----------------------------------------------------

BuddyListWindow::~BuddyListWindow()
{
}

//-----------------------------------------------------

bool BuddyListWindow::QuitRequested()
{
	BMessage* curMsg;

	// grab the current message
	curMsg = CurrentMessage();

	// save the window position
	BRect winPos = Frame();
	prefs->WriteInt32( "buddylist.x1", int32(winPos.LeftTop().x) );
	prefs->WriteInt32( "buddylist.y1", int32(winPos.LeftTop().y) );
	prefs->WriteInt32( "buddylist.x2", int32(winPos.RightBottom().x) );
	prefs->WriteInt32( "buddylist.y2", int32(winPos.RightBottom().y) );

	// kill the bubble-watcher thread
	if( bubbleThread >= 0 )
		kill_thread(bubbleThread);		

	// close the bubble window
	bubblewin->Lock();
	bubblewin->Quit();

	// (possibly) kill the entire app
	if( !curMsg->HasBool("dontkill") )
		be_app->PostMessage(B_QUIT_REQUESTED);

	return(true);
}

//-----------------------------------------------------

void BuddyListWindow::LoadAwayMessages() {

	BMenuItem* tempItem;
	BMessage* awMsg;
	awayStruct aw;
	char shortcut;
	int i = 0;
	bool kg;
	
	// first, clear out everything in the existing menu
	for( unsigned i = 0; i < awayMenuItems.Count(); ++i ) {
		tempItem = awayMenuItems[i];
		statMenu->RemoveItem(tempItem);
		delete tempItem;
	}
	awayMenuItems.Clear();
	
	// Get the first of the new away messages...
	kg = prefs->GetFirstAwayMessage(aw);
	
	// special case: are there no messages defined?
	if( !kg ) {
		BMenuItem* noMsgs = new BMenuItem( "No Away Messages Defined", NULL );
		noMsgs->SetEnabled(false);
		statMenu->AddItem( noMsgs, 2 );
		awayMenuItems.Add( noMsgs );
	}
	
	// OK, there are some. Add 'em.
	while( kg ) {
	
		shortcut = 0;
		if( i < 9 )
			shortcut = '1' + i;
	
		awMsg = new BMessage(BEAIM_GOING_AWAY);
		tempItem = new BMenuItem( aw.name.String(), awMsg, shortcut );
		awMsg->AddPointer( "menuitem", (void*)tempItem );
		statMenu->AddItem( tempItem, i+2 );
		awayMenuItems.Add( tempItem );
		if( client->AwayMode() == AM_STANDARD_MESSAGE ) {
			if( client->CurrentAwayMessageName() == aw.name.String() )
				tempItem->SetMarked(true);
		}
		kg = prefs->GetNextAwayMessage(aw);
		i++;
	}
}

//-----------------------------------------------------

void BuddyListWindow::GoAway( BMessage* msg ) {

	BMenuItem* markedItem;
	BMenuItem* menuItem;
	
	// get the needed info
	msg->FindPointer( "menuitem", (void**)&menuItem );
	bool away = !msg->HasBool("away");
	if( away )
		msg->FindPointer( "menuitem", (void**)&menuItem );

	// Find the currently marked item, and unmark it (if needed)
	markedItem = statMenu->FindMarked();
	availItem->SetMarked(!away);
	if( away && markedItem && markedItem == menuItem )
		return;
	if( markedItem )
		markedItem->SetMarked(false);
	if( away )
		menuItem->SetMarked(true);
	
	// Are we going away or coming back?
	if( away )
		client->SetAwayMode( AM_STANDARD_MESSAGE, BString(menuItem->Label()) );
	else
		client->SetAwayMode( AM_NOT_AWAY );
}

//-----------------------------------------------------

void BuddyListWindow::MessageReceived( BMessage* message ) {

	// Some variables needed for various messages
	/*
	BMessenger messenger(APP_SIGNATURE);
	BOutlineListView* source = NULL;
	BListItem* currentItem = NULL;
	PeopleItem* currentBuddyItem = NULL;	
	BString ScreenName;
	BMessage* sendMessage;
	int32 Selected, status;
	*/

	switch(message->what) {

		case BEAIM_RELOAD_BUDDYLIST:
			//Clear();
			//LoadGroups();
			break;

		case BEAIM_ONCOMING_BUDDY:
		case BEAIM_BUDDY_STATUS_CHANGE:
			printf( "-------> buddy status change msg: %s\n", message->FindString("userid") );
			SetStatus( AIMUser((char*)message->FindString("userid")),
								message->FindInt32("status"),
								message->FindInt32("warninglevel") );
			break;

		case BEAIM_MOVE_BUDDY:
			MoveBuddy( AIMUser((char*)message->FindString("user")) );
			Verify();
			break;

		case BEAIM_MOVE_GROUP:
			MoveGroup( message->FindString("group") );
			Verify();
			break;

		// The user has double-clicked on a buddy. Send a message to open up an IM window.
		case BEAIM_BUDDY_INVOKED:
			Invoked();
			break;
			
		case BEAIM_ADD_GROUP:
			DoAddGroup( message );
			break;
		
		case BEAIM_ADD_BUDDY:
			//DoAddBuddy( message );
			RefreshGroupCounts(true);
			break;
		
		case BEAIM_CHANGE_BUDDY:
		case BEAIM_DELETE_BUDDY:
			DoDeleteBuddy( message );
			break;
			
		case BEAIM_DELETE_GROUP:
			DoDeleteGroup( message );
			break;
			
		case BEAIM_LOAD_AWAY_MENU:
			LoadAwayMessages();
			break;
			
		case BEAIM_GOING_AWAY:
			GoAway( message );
			break;

		case BEAIM_RELOAD_PREF_SETTINGS:
			LoadPrefs();
			break;

		case BEAIM_AWAY_STATUS_CHANGED:
			if( message->HasBool("custom") ) {
				BMenuItem* marked = statMenu->FindMarked();
				if( marked )
					marked->SetMarked(false);
				customItem->SetMarked(true);
			}
			myName->SetAway( message->FindBool("away") );
			break;
		
		case BEAIM_GOT_WARNED:
			if( message->HasInt32("warninglevel") )
				myName->SetWarningLevel( (unsigned short)message->FindInt32("warninglevel") );
			break;
			
		case SHOW_BUBBLE:
			ShowBubble( message->FindPoint("where"), true );
			break;
			
		case HIDE_BUBBLE:
			ShowBubble( message->FindPoint("where"), false );
			break;
			
		case BEAIM_CHANGE_GROUP:
			DoChangeGroup( message );
			break;

		case BEAIM_SEND_ARBITRARY_DATA:
		case BEAIM_PREV_CHAT_WINDOW:
		case BEAIM_NEXT_CHAT_WINDOW:
			be_app->PostMessage( message );
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		case BEAIM_SEND_NEW_MESSAGE: {
			BMessage* sendMessage = new BMessage(BEAIM_OPEN_USER_WINDOW);
			sendMessage->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
			windows->OpenInputWindow( (char*)Language.get("CONTACT_USER"),
									  (char*)LangWithSuffix("SCREEN_NAME_LABEL", ":"),
									  sendMessage, this, 
									  NULL, DISPLAY_NAME_MAX, true, "userid" );
			break;
		}

		case BEAIM_SEARCH_BY_EMAIL: {
			BMessage* sendMessage = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
			sendMessage->AddInt32( "wtype", SW_EMAIL_RESULTS );
			windows->OpenInputWindow( (char*)Language.get("SEARCH_BY_EMAIL"),
									  (char*)LangWithSuffix("EMAIL_LABEL", ":"),
									  sendMessage, this, 
									  NULL, 100, true, "email" );
			break;
		}

		case BEAIM_GET_USER_INFO: {
			BMessage* sendMessage = new BMessage(BEAIM_OPEN_USER_WINDOW);
			sendMessage->AddInt32( "wtype", (int32)USER_INFO_TYPE );
			sendMessage->AddPointer( "poswindow", this );
			windows->OpenInputWindow( (char*)Language.get("GET_USER_INFO"),
									  (char*)LangWithSuffix("SCREEN_NAME_LABEL", ":"),
									  sendMessage, this, 
									  NULL, DISPLAY_NAME_MAX, true, "userid" );		
			break;
		}

		case BEAIM_SET_USER_BLOCKINESS:
			QuickRemoveBuddy( AIMUser((char*)message->FindString("userid")) );
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}

//-----------------------------------------------------

void BuddyListWindow::QuickRemoveBuddy( AIMUser user ) {

	// nuke 'em
	BLMainItem* budItem = FindBuddy( user );
	if( !budItem )
		return;
	budView->RemoveItem( budItem );
	RefreshGroupCounts( true );
}

//-----------------------------------------------------

void BuddyListWindow::ShowBubble( BPoint where, bool show, bool forceClose ) {

	BRect blRect;
	AIMUser overUser;
	bool switchCheck = false;
	float bWidth, bHeight;

	// force-close the bubble?
	if( forceClose ) {
		if( bubblewin->Lock() ) {
			if( bubblewin->IsBeingShown() )
				bubblewin->SetBeingShown( false );
			bubblewin->Unlock();
		}
		return;
	}

	// get the bounds rect of the buddy list in screen coordinates
	blRect = budView->Bounds();
	budView->ConvertToScreen( &blRect );
	
	// is the mouse over the buddy list?
	if( blRect.Contains(where) ) {
	
		// convert the point to view coordinates
		BPoint viewPoint = budView->ConvertFromScreen(viewPoint);
		int32 overIndex = budView->IndexOf(viewPoint);
	
		// if we're over a screen name, get the name at that location
		if( overIndex >= 0 ) {
			BListItem* genItem = budView->ItemAt(overIndex);
			BLMainItem* budItem = dynamic_cast<BLMainItem*>(genItem);
			if( budItem )
				overUser = budItem->GetUser();
		}
	}

	// send the bubble back to oblivion?
	if( !show ) {

		// if we're still over the same user, don't show the bubble
		if( overUser == lastUserOver )
			return;

		// turn off the bubble if we're not over a user anymore
		if( overUser.IsEmpty() ) {
			bubblewin->SetBeingShown( false );
			return;
		}
		
		// try and lock the window so we can get the next user's bubble
		if( bubblewin->Lock() )
			switchCheck = true;
		else
			return;
	}
//		if( bubblewin->Lock() )
//			switchCheck = true;
//		else
//			return;
	// otherwise, bring the bubble forth!
	// don't show it unless this is the active window, or the mouse
	// isn't over a username
	if( !IsActive() || overUser.IsEmpty() )
		return;

	// format the bubble for this particular user
	FormatBubble( overUser );

	// calculate the correct point to display (so the screen doesn't clip it)
	BScreen screen;
	//BPoint showPoint = BPoint( where.x + 15, where.y + 10 );
	BPoint showPoint = where + BPoint(15,10);
	BRect screenFrame = screen.Frame();
	bubblewin->GetDimensions( bWidth, bHeight );
	if( (showPoint.y + bHeight) > (screenFrame.bottom - 3) )
		showPoint.y = showPoint.y - (bHeight + 13);	
	if( (showPoint.x + bWidth + 4) > (screenFrame.right) )
		showPoint.x = showPoint.x - ((showPoint.x + bWidth + 4) - screenFrame.right );	

	// show the bubble! yee-hah!
	bubblewin->SetBeingShown( true, &showPoint );
	
	// unlock the window if we locked it before
	if( switchCheck )
		bubblewin->Unlock();
	
	// save the "current user" for the next time
	lastUserOver = overUser;
}

//-----------------------------------------------------

void BuddyListWindow::FormatBubble( AIMUser user ) {

	uint32 curTime, testTime;
	int32 warningLevel;
	int status;
	char tempString[150];
	char timeString[100];

	bubblewin->Lock();

	bubblewin->ClearDisplayString();
	bubblewin->AddDisplayString( user.Username() );

	// grab the user's status	
	status = users->GetBuddyStatus( user );
	
	// handle the offline case
	if( status & BS_OFFLINE ) {
		sprintf( tempString, "    %s", Language.get("STAT_OFFLINE") );
		bubblewin->AddDisplayString( tempString );
		return;
	}
	
	// get the user's warning level... not much, but it's something
	warningLevel = users->GetBuddyWarningLevel( user );
	sprintf( tempString, "    %s:  %d%%", Language.get("STAT_WARNING_LEVEL"), (int)warningLevel );
	bubblewin->AddDisplayString( tempString );
	
	// schlep in the status string
	sprintf( tempString, "    %s: ", Language.get("STAT_STATUS") );
	if( status & BS_ACTIVE )
		strcat( tempString, Language.get("STAT_ACTIVE") );
	else if( status & BS_IDLE )
		strcat( tempString, Language.get("STAT_IDLE") );
	if( status & BS_AWAY ) {
		strcat( tempString, ", " );
		strcat( tempString, Language.get("STAT_AWAY") );
	}
	bubblewin->AddDisplayString( tempString );
	
	// online time for this user
	sprintf( tempString, "    %s: ", Language.get("STAT_ONLINE_TIME")  );
	testTime = users->GetBuddyLoginTime( user );
	curTime = real_time_clock();
	MakeElapsedTimeString( int32(curTime - testTime), timeString );
	strcat( tempString, timeString );
	bubblewin->AddDisplayString( tempString );
	
	// idle time for this user (if applicable)
	if( status & BS_IDLE ) {
		sprintf( tempString, "    %s: ", Language.get("STAT_IDLE_TIME")  );
		testTime = users->GetBuddyIdleStartTime( user );
		curTime = real_time_clock();
		MakeElapsedTimeString( int32(curTime - testTime), timeString );
		strcat( tempString, timeString );
		bubblewin->AddDisplayString( tempString );
	}
	
	// make the screen name bold
	bubblewin->SetNumBoldChars( user.Username().Length() );

	bubblewin->Unlock();
}

//-----------------------------------------------------

void BuddyListWindow::Invoked() {

	BListItem* currentItem = NULL;
	BLMainItem* currentBuddyItem = NULL;
	BMessage* sendMessage;
	AIMUser ScreenName;
	
	int32 Selected = budView->CurrentSelection();
	currentItem = budView->ItemAt( Selected );
	if( !currentItem )
		return;
	if( currentItem->OutlineLevel() == 0 && budView->CountItemsUnder(currentItem,true) ) {
		if( budView->IsExpanded(budView->FullListIndexOf(currentItem)) )
			budView->Collapse(currentItem);
		else	
			budView->Expand(currentItem);
		return;
	}
	if( (currentBuddyItem = dynamic_cast<BLMainItem*>(currentItem)) == 0 )
		return;
	ScreenName = currentBuddyItem->GetUser();
	sendMessage = new BMessage( BEAIM_OPEN_USER_WINDOW );
	sendMessage->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
	sendMessage->AddString( "userid", ScreenName.UserString() );
	PostAppMessage( sendMessage );
}

//-----------------------------------------------------

void BuddyListWindow::LoadPrefs() {

	// load and implement the "all workspaces" setting
	bool allWorkspaces = prefs->ReadBool("BuddyListAllWorkspaces", true);
	if( allWorkspaces )
		//SetWorkspaces( B_ALL_WORKSPACES );
		SetAllWorkspaces( true );
	else
		//SetWorkspaces( B_CURRENT_WORKSPACE );
		SetAllWorkspaces( false );
}

//-----------------------------------------------------

void BuddyListWindow::LoadGroups() {
	
	bool kg;
	BString group;
	BLGroupItem* grpItem;
	int count;

	kg = users->GetGroups( group, true, &count );
	while( kg ) {
	
		grpItem = new BLGroupItem(group, &ownerFont);
		grpItem->SetCounts( -1, count );
		budView->AddItem( grpItem );
		kg = users->GetGroups( group, false, &count );
	}
	
	SetOfflineGroupVisibility(showOfflineGroup);
}

//-----------------------------------------------------

void BuddyListWindow::Clear() {

	GenList<BListItem*> items;
	BListItem* tempItem;

	// first, cache everything in the list
	for( int32 i = 0; i < budView->FullListCountItems(); ++i )
		items.Add( budView->FullListItemAt(i) );
		
	// clear everything
	budView->MakeEmpty();
	
	// now, delete all the items that were in the list
	while( items.Pop(tempItem) )
		delete tempItem;
}

//-----------------------------------------------------

void BuddyListWindow::SetOfflineGroupVisibility( bool show ) {

 	showOfflineGroup = show;

 	// take it off
 	if( !showOfflineGroup ) {
		if( offlineGroup )
			;// ... whatever
 	}
 	
 	// put it on
 	else if( !offlineGroup ) {
 	
 		int32 buddyUnderIndex = 0;
 		BLMainItem* addItem;
 		AIMUser user;
 		bool kg;
 	
 		// setup the group item
 		offlineGroup = new BLGroupItem( Language.get("OFFLINE_NAME"), &ownerFont, true);
 		offlineGroup->SetExpanded(false);
 		budView->AddItem( offlineGroup );
 		
 		// add all the buddies that are currently offline
 		kg = users->GetAllBuddies( user, true );
 		while( kg ) {
 			//pals.SendMessage( (char*)user.Username().String() );
 			if( users->GetBuddyStatus(user) == BS_OFFLINE ) {
	 			addItem = new BLMainItem( user, &ownerFont );
	 			addItem->SetStatus( BS_OFFLINE );
	 			budView->AddItem( addItem, budView->FullListIndexOf(offlineGroup) + (++buddyUnderIndex) );
 			}
 			kg = users->GetAllBuddies( user, false );
 		}
 		
 		// finally, set the counts
 		offlineGroup->SetCounts( buddyUnderIndex, users->CountBuddies() );
 	}
}

//-----------------------------------------------------

void BuddyListWindow::SetStatus( AIMUser user, int status, int32 wLevel ) {

	BLMainItem* mTemp = NULL;
	bool found = false;
	BLGroupItem* gItem;
	int insertPos;
	BString group;

	// search for the item in the list
	for( int32 i = 0; i < budView->FullListCountItems(); ++i ) {
	
		// try and get the item
		mTemp = dynamic_cast<BLMainItem*>( budView->FullListItemAt(i) );
		if( !mTemp )
			continue;
	
		// if we've found the user, break outta the loop
		if( user == mTemp->GetUser() ) {
			found = true;
			break;
		}
	}

	// create the item if it wasn't there and set the status
	if( !found )
		mTemp = new BLMainItem( user, &ownerFont );
	mTemp->SetStatus(status);
	mTemp->SetWarningLevel(wLevel);	

	// if the user is online, they need to be in the right spot
	if( status != BS_OFFLINE ) {

		// if the item was just created, it needs to be correctly inserted
		if( !found ) {
			insertPos = 0;
			users->FindOnlineInsertionPoint( user, group, insertPos );
			gItem = FindGroup( group );
			insertPos += (budView->FullListIndexOf(gItem) + 1);
			budView->AddItem( mTemp, insertPos );
			RefreshGroupCounts(false);
			users->BuddyDisplayGotUpdated( user );
			return;
		}
		
		/*
		// or if it was in the offline group, move it into position
		else if( showOfflineGroup && budView->Superitem(mTemp) == offlineGroup ) {
			insertPos = 0;
			users->FindOnlineInsertionPoint( user, group, insertPos );
			gItem = FindGroup( group );
			insertPos += (budView->FullListIndexOf(gItem) + 1);
			budView->RemoveItem( mTemp );
			budView->AddItem( mTemp, insertPos );
			RefreshGroupCounts(false);
			return;
		}
		*/
		
		// otherwise, just refresh it
		else {
			budView->InvalidateItem( budView->FullListIndexOf(mTemp) );
			users->BuddyDisplayGotUpdated( user );
			return;
		}
	}
	
	// so the user is *offline*... stick them into the offline group (if enabled)
	else {

		// didn't find it, had to make one
		if( !found ) {

			// if we're not showing the offline group, and we didn't find it anyway, delete it and return
			//if( !showOfflineGroup ) {
				delete mTemp;
				RefreshGroupCounts(false);
				return;		
			//}

			/*
			// if we *are* showing the offline group, but didn't find it, insert it and return
			else if( showOfflineGroup ) {
				insertPos = 0;
				users->FindOfflineInsertionPoint( user, insertPos );
				budView->AddItem( mTemp, budView->FullListIndexOf(offlineGroup)+insertPos );
				RefreshGroupCounts(false);
			}
			*/
		}

		// found it just fine, thank you
		else {
		
			/*
			// if we *are* showing the offline group, and need to move mTemp there, do so
			if( showOfflineGroup && budView->Superitem(mTemp) != offlineGroup ) {
				insertPos = 0;
				users->FindOfflineInsertionPoint( user, insertPos );
				budView->RemoveItem( mTemp );
				budView->AddItem( mTemp, budView->FullListIndexOf(offlineGroup)+insertPos+1 );
				RefreshGroupCounts(false);
			}
			*/
			
			// if we are *not* showing the offline group, just remove and delete the item
			//else if( !showOfflineGroup ) {
				budView->RemoveItem( mTemp );
				RefreshGroupCounts(false);
				delete mTemp;
			//}
		}
	}
}

//-----------------------------------------------------

void BuddyListWindow::DoAddBuddy( BMessage* msg ) {

	int insertPoint;
	BListItem* gpItem = NULL;
	BListItem* budItem = NULL;
	AIMUser user;
	BString group;
	int status;

	// initialize stuff
	user = AIMUser(msg->FindString("userid"));
	group = BString(msg->FindString("group"));
	status = users->GetBuddyStatus( user );
	
	// insert into the offline group if that's turned on
	if( status == BS_OFFLINE ) {
		if( showOfflineGroup && offlineGroup ) {
			budItem = new BLMainItem( user, &ownerFont );
			users->FindOfflineInsertionPoint( user, insertPoint );
			insertPoint += (budView->FullListIndexOf(offlineGroup) + 1);
			budView->AddItem( budItem, insertPoint );
		}
	}
	
	// or if they're online, stick 'em in the list
	else {
		budItem = new BLMainItem( user, &ownerFont );
		users->FindOnlineInsertionPoint( user, group, insertPoint );
		gpItem = FindGroup( group );
		insertPoint += (budView->FullListIndexOf(gpItem) + 1);
		budView->AddItem( budItem, insertPoint );
	}
	
	// there's an extra buddy on the list somewhere, so...
	RefreshGroupCounts( true );
}

//-----------------------------------------------------

void BuddyListWindow::DoAddGroup( BMessage* msg ) {

	int32 insertPoint;
	BLGroupItem* gpItem;
	BString group = BString(msg->FindString("group"));
	
	// make sure the group isn't already there
	if( FindGroup(group) )
		return;
	
	// make a new item for it
	gpItem = new BLGroupItem( msg->FindString("group"), &ownerFont );

	// get the insertion point for the new group
	if( showOfflineGroup && offlineGroup )
		insertPoint = budView->FullListIndexOf(offlineGroup);
	else
		insertPoint = budView->FullListCountItems();

	// now add it
	budView->AddItem( gpItem, insertPoint );
}

//-----------------------------------------------------

void BuddyListWindow::DoDeleteBuddy( BMessage* msg ) {

	AIMUser target;
	if( msg->what == BEAIM_DELETE_BUDDY )
		target = AIMUser(msg->FindString("userid"));
	else if( msg->what == BEAIM_CHANGE_BUDDY )
		target = AIMUser(msg->FindString("oldname"));
	
	// nuke 'em
	BLMainItem* budItem = FindBuddy( target );
	if( !budItem )
		return;
	budView->RemoveItem( budItem );
	RefreshGroupCounts( true );
}

//-----------------------------------------------------

void BuddyListWindow::DoDeleteGroup( BMessage* msg ) {

	BString group = BString(msg->FindString("group"));
	BLGroupItem* grpItem = FindGroup(group);
	GenList<BListItem*> removedItems;
	BListItem* tempItem;
	
	// handle the online list
	if( grpItem ) {
	
		// remove all the online members of that group
		int32 underCount = budView->CountItemsUnder(grpItem, true);
		int32 gIndex = budView->FullListIndexOf(grpItem);
		for( int32 i = 0; i < underCount; ++i ) {
			tempItem = budView->FullListItemAt( gIndex + 1 );
			removedItems.Add( tempItem );
		}
		
		// now remove the groupitem itself
		budView->RemoveItem( grpItem );
		removedItems.Add( grpItem );
	}

	// now do the offline stuff
	if( showOfflineGroup && offlineGroup ) {
		int32 base = msg->FindInt32("offlinebase");
		int32 count = msg->FindInt32("offlinecount");
	
		// if there are any offline members, remove them
		if( count ) {
			int32 offBase = budView->FullListIndexOf(offlineGroup) + 1;
			for( int32 i = 0; i < count; ++i ) {
				tempItem = budView->FullListItemAt(offBase+base);
				budView->RemoveItem( tempItem );
				removedItems.Add( tempItem );
			}
		}			
	}

	// finally, delete all the items that have been removed
	while( removedItems.Pop(tempItem) )
		delete tempItem;
		
	// stuff has changed, so recount
	RefreshGroupCounts( true );
}


//-----------------------------------------------------

void BuddyListWindow::DoChangeGroup( BMessage* msg ) {

	BString from = BString(msg->FindString("oldname"));
	BString to = BString(msg->FindString("newname"));
	BLGroupItem* grpItem = FindGroup(from);

	// bail if the group wasn't found
	if( !grpItem )
		return;
		
	// now change the name
	grpItem->SetGroup( to );
	budView->InvalidateItem( budView->FullListIndexOf(grpItem) );
}

//-----------------------------------------------------

BLGroupItem* BuddyListWindow::FindGroup( BString group ) {
	return (BLGroupItem*)budView->EachItemUnder( NULL, true, FindGroupIter, (void*)&group );
}

//-----------------------------------------------------

BListItem* BuddyListWindow::FindGroupIter( BListItem* item, void* name ) {

	BString* searchName = (BString*)name;
	BLGroupItem* gItem = dynamic_cast<BLGroupItem*>(item);
	if( gItem && !gItem->IsOfflineGroup() && *searchName == gItem->Group() )
		return item;
	return NULL;
}

//-----------------------------------------------------

BLMainItem* BuddyListWindow::FindBuddy( AIMUser user, BString group, bool checkOffline ) {

	if( checkOffline ) {
		if( !showOfflineGroup )
			return NULL;
		return (BLMainItem*)budView->EachItemUnder( offlineGroup, true, FindBuddyIter, (void*)&user );
	}
	
	BLGroupItem* gItem = FindGroup( group );
	if( !gItem )
		return NULL;

	return (BLMainItem*)budView->EachItemUnder( gItem, true, FindBuddyIter, (void*)&user );
}

//-----------------------------------------------------

BLMainItem* BuddyListWindow::FindBuddy( AIMUser user ) {
	return (BLMainItem*)budView->EachItemUnder( NULL, false, FindBuddyIter, (void*)&user );
}

//-----------------------------------------------------

BListItem* BuddyListWindow::FindBuddyIter( BListItem* item, void* name ) {

	AIMUser* searchName = (AIMUser*)name;	
	BLMainItem* uItem = dynamic_cast<BLMainItem*>(item);
	if( uItem && *searchName == uItem->GetUser() )
		return item;
	return NULL;
}

//-----------------------------------------------------

void BuddyListWindow::RefreshGroupCounts(  bool force ) {
	forceCountMode = force;
	budView->EachItemUnder( NULL, true, DoCountsForEachGroup, (void*)this );
}

//-----------------------------------------------------

BListItem* BuddyListWindow::DoCountsForEachGroup( BListItem* item, void* window ) {

	BuddyListWindow* theWindow = (BuddyListWindow*)window;
	bool changed = false;
	BLGroupItem* gItem;
	int32 count;

	// convert it to a BLGroupItem* so we can work with counts and stuff
	gItem = dynamic_cast<BLGroupItem*>(item);
	if( !gItem )
		return NULL;

	// refresh the online count if it needs refreshing
	count = theWindow->budView->CountItemsUnder( item, true );
	if( count != gItem->OnlineCount() ) {
		changed = true;
		gItem->SetCounts( count, -1 );
	}

	// if this is the offline group, special casing is required...
	if( gItem == theWindow->offlineGroup && theWindow->showOfflineGroup ) {
		int32 total = users->CountBuddies();
		if( total != gItem->TotalCount() ) {
			gItem->SetCounts( -1, total );
			changed = true;
		}
	}

	// if we need to get a new total count as well, do that
	if( theWindow->forceCountMode ) {
		int32 total = users->GetGroupCount(gItem->Group());
		if( total != gItem->TotalCount() ) {
			gItem->SetCounts( -1, total );
			changed = true;
		}
	}

	// redraw the item and return
	if( changed )
		theWindow->budView->InvalidateItem( theWindow->budView->FullListIndexOf(gItem) );
	return NULL;
}

//-----------------------------------------------------

void BuddyListWindow::MoveBuddy( AIMUser user ) {

	BLMainItem* uItem;
	BLGroupItem* gItem;
	int insertPos;
	BString group;
	
	// look for the item and bail if it's not there
	uItem = FindBuddy( user );
	if( !uItem )
		return;
	budView->RemoveItem(uItem);
	
	// grab the status (affects how we do stuff in the list)
	int status = users->GetBuddyStatus( user );

	// the user is online
	if( status != BS_OFFLINE ) {

		// insert it in the new spot	
		users->FindOnlineInsertionPoint( user, group, insertPos );
		
		//BeDC dc("movebuddy", DC_BLACK);
		//dc.SendFormat( "Moving %s to group %s, pos %d", (char*)user.UserString(), group.String(), insertPos );
		gItem = FindGroup( group );
		insertPos += (budView->FullListIndexOf(gItem) + 1);
		budView->AddItem( uItem, insertPos );
	}
	
	// the user is offline
	else if( showOfflineGroup && offlineGroup ) {

		// insert it in the new spot	
		users->FindOfflineInsertionPoint( user, insertPos );
		insertPos += (budView->FullListIndexOf(offlineGroup) + 1);
		budView->AddItem( uItem, insertPos );
	}
	
	// now refresh the group counts
	RefreshGroupCounts( true );
	
	//dc.SendMessage( (char*)(status == BS_OFFLINE ? "off" : "on") );
	//dc.SendInt( (int32)status );
	//int status = users->GetBuddyStatus( user );
}

//-----------------------------------------------------

void BuddyListWindow::MoveGroup( BString group ) {

	int32 base, tCount, gCount, target;
	GenList<BListItem*> subItems;
	BListItem* tempItem;

	BLGroupItem* groupItem = FindGroup(group);
	if( !groupItem )	
		return;
	
	// remove the item and all its subitems
	base = budView->FullListIndexOf(groupItem);
	for( int32 i = 1; i <= budView->CountItemsUnder(groupItem, true); ++i )
		subItems.Add( budView->FullListItemAt(base+i) );
	budView->RemoveItem(groupItem);
	
	// find the proper insertion point for the group stuff
	target = users->GroupPos(group);
	if( !target )
		tCount = 0;
	else {
		gCount = 0;
		for( tCount = 0; tCount < budView->FullListCountItems(); ++tCount ) {
			if( budView->FullListItemAt(tCount)->OutlineLevel() == 0 ) {
				if( 	++gCount > target )
					break;
			}		
		}
	}

	// re-insert the actual item, then the subitems (backwards to maintain order)
	budView->AddItem( groupItem, tCount );
	while( subItems.Pop(tempItem) )
		budView->AddUnder( tempItem, groupItem );
/*
	// now, handle the offline stuff
	if( showOfflineGroup && offlineGroup ) {
		GenList<BLMainItem*> movingVan;
		int32 gmCount, iPoint = 0;
		AIMUser first, last;
		BLMainItem* offItem;
		bool foundLast = false;

		// find out how many offline users to move, and the first and last users
		users->GetOfflineUserInfoForGroup( group, first, last, gmCount );
		if( !gmCount )
			return;

		// go through the offline list looking for the the users from the group
		// that just got moved, and remove them from the list (backwards to maintain order)
		int32 base = budView->FullListIndexOf(offlineGroup) + 1;
		for( int32 i = budView->CountItemsUnder(offlineGroup,true) - 1; i >= 0; --i ) {
			offItem = (BLMainItem*)budView->FullListItemAt(base+i);

			// if we haven't found the last one yet, and it matches, then this is it
			if( !foundLast && last == offItem->GetUser() ) {
				movingVan.Insert( offItem, 0 );
				budView->RemoveItem( offItem );
				foundLast = true;
				if( gmCount == 1 )		// don't continue if this will be the *only* match
					break;
			}

			// if we've found the last match and we're still in the loop, then
			// the current item has to be in the group that we're working with
			else if( foundLast ) {
				movingVan.Insert( offItem, 0 );
				budView->RemoveItem( offItem );

				// if this the first match? If so, break out of the loop
				if( first == offItem->GetUser() )
					break;
			}
		}
		
		// do all the insertions for this offline group
		users->FindOfflineInsertionPoint( first, iPoint );
		while( movingVan.Pop(offItem) )
			budView->AddItem( offItem, base+iPoint );
	}
*/
}

//-----------------------------------------------------

void BuddyListWindow::Verify() {
/*
	GenList<AIMUser> daBuddies;
	GenList<bool> isOnline;
	GenList<BString> daGroups;
	GenList<int32> groupCounts;
	BListItem* nTempItem;
	BLMainItem* bTempItem;
	BLGroupItem* gTempItem;
	AIMUser tempBuddy;
	BString tempGroup;
	int32 tempCount;
	int32 totalCovered;
	int status;
	bool kg;
	
	// this is only a debugging thing...
	if( !beaimDebug )
		return;
	
	// get the list of buddies
	kg = users->GetAllBuddies(tempBuddy, true);
	while( kg ) {
		daBuddies.Add(tempBuddy);
		kg = users->GetAllBuddies(tempBuddy, false);
	}
	
	// get the list of whether they are online or not
	for( int i = 0; i < (int32)daBuddies.Count(); ++i ) {
		tempBuddy = daBuddies[i];
		status = users->GetBuddyStatus(tempBuddy);
		isOnline.Add( bool(status != BS_OFFLINE) );
	}
	
	// get the list of groups
	kg = users->GetGroups(tempGroup, true);
	while( kg ) {
		daGroups.Add(tempGroup);
		kg = users->GetGroups(tempGroup, false);
	}
	
	// get the list of whether they are online or not
	for( int i = 0; i < (int32)daGroups.Count(); ++i ) {
		tempGroup = daGroups[i];
		groupCounts.Add( users->GetGroupCount(tempGroup) );
	}
	
	// finally got all the stuff. Now go through the long, painful process of verifying the list.
	
	// first, verify the groups.
	tempCount = 0;
	for( int32 i = 0; i < budView->FullListCountItems(); ++i ) {
		nTempItem = budView->FullListItemAt(i);
		if( nTempItem->OutlineLevel() == 0 && nTempItem != offlineGroup ) {
			gTempItem = (BLGroupItem*)nTempItem;
			if( gTempItem->Group() != daGroups[tempCount++] ) {
				BString err = "error: group ";
				err.Append( gTempItem->Group() );
				err.Append( " where group " );
				err.Append( daGroups[tempCount-1] );
				err.Append( " should be!" );
				Say( err );
				return;
			}
		}
	}

	totalCovered = 0;

	tempCount = 0;
	for( int32 i = 0; i < (int32)daBuddies.Count(); ++i ) {
		nTempItem = budView->FullListItemAt(i);
		if( nTempItem == offlineGroup )
			break;
		if( nTempItem->OutlineLevel() == 1 ) {
			bTempItem = (BLMainItem*)nTempItem;
			while( tempCount < (int32)daBuddies.Count() && isOnline[tempCount] == false )
				++tempCount;
			if( tempCount >= (int32)daBuddies.Count() ) {
				Say( "counting error!" );
				return;
			}
			if( bTempItem->GetUser() != daBuddies[tempCount] ) {
				BString err = "online error: buddy ";
				err.Append( bTempItem->GetUser().Username() );
				err.Append( " where buddy " );
				err.Append( daBuddies[tempCount].Username() );
				err.Append( " should be!" );
				Say( err );
				return;
			}
			++tempCount;
		}
	}

	tempCount = 0;
	if( offlineGroup && showOfflineGroup ) {
		for( int32 i = budView->FullListIndexOf(offlineGroup)+1; i < budView->FullListCountItems(); ++i ) {
			nTempItem = budView->FullListItemAt(i);
			bTempItem = (BLMainItem*)nTempItem;
			while( tempCount < (int32)daBuddies.Count() && isOnline[tempCount] == true )
				++tempCount;
			if( tempCount >= (int32)daBuddies.Count() ) {
				Say( "counting error!" );
				return;
			}
			if( bTempItem->GetUser() != daBuddies[tempCount] ) {
				BString err = "offline error: buddy ";
				err.Append( bTempItem->GetUser().Username() );
				err.Append( " where buddy " );
				err.Append( daBuddies[tempCount].Username() );
				err.Append( " should be!" );
				Say( err );
				return;
			}
			++tempCount;
		}
	}
	*/
}

//-----------------------------------------------------

int32 BuddyListWindow::StartBubbleWatcher(void *arg) {
	((BuddyListWindow*)arg)->BubbleWatcher();
	return 0;
}

//-----------------------------------------------------

void BuddyListWindow::BubbleWatcher() {

	bool bubbleOn = false;
	unsigned long delayCounter = 0;
	BPoint where, lastwhere, where2;
	BMessage* msg;
	ulong buttons;

	// loop forever... and ever...
	while( true ) {

		// is the bubble currently showing?
		if( bubblewin->Lock() ) {
			bubbleOn = bubblewin->IsBeingShown();
			bubblewin->Unlock();
		}

		// not being shown yet, so we're in "detect" mode...
		if( !bubbleOn ) {
	
			// try and lock the window
			if( bubblewin->Lock() ) {

				// grab the current mouse coordinates and frame
				bubblewin->GetMouseInfo( where, buttons );
				bubblewin->Unlock();
		
				// reset the delay counter if the mouse has been moved or is being clicked,
				// or if the mouse isn't even over the buddylist window
				if( lastwhere != where || buttons ) {
					delayCounter = 0;
				}

				// mouse didn't get moved or clicked...
				else {
				
					// has it been still long enough to show the bubble?
					if( delayCounter++ == 5 ) {

						// tell the buddy list to show the bubble
						msg = new BMessage(SHOW_BUBBLE);
						msg->AddPoint( "where", where );
						PostMessage( msg );
						delete msg;
					}
					
				}
			}
		}
		
		// OK, so the bubble *is* being shown... time to get rid of it?
		else {
		
			// try and lock the window
			if( bubblewin->Lock() ) {
				
				// grab the current mouse coordinates
				bubblewin->GetMouseInfo( where2, buttons );
				bubblewin->Unlock();
				
				// turn it off!! ahhhhhhhhhh!!!!!!!
				if( buttons || where2 != where ) {

					// tell the buddy list to show the bubble
					msg = new BMessage(HIDE_BUBBLE);
					msg->AddPoint( "where", where2 );
					PostMessage( msg );
					delete msg;	
				}
				
				// this is the new location to check against
				where = where2;
			}
			
			// just in case
			delayCounter = 0;
		}
		
		// get ready for the next go-round...
		lastwhere = where;
		snooze(100000);
	}
}

//-----------------------------------------------------

void BuddyListWindow::RefreshLangStrings() {

	// the buddy list name
	SetTitle( Language.get("BUDDY_LIST_NAME") );

	// menu items and such
	availItem->SetLabel( Language.get("AVAILABLE_ITEM") );
	customItem->SetLabel( LangWithSuffix("CUSTOM_AWAY_MSG", B_UTF8_ELLIPSIS) );
	peopleMenu->Superitem()->SetLabel( Language.get("PEOPLE_MENU_NAME") );
	statMenu->Superitem()->SetLabel( Language.get("AWAY_MENU_NAME") );
	beMenu->ItemAt(0)->SetLabel( LangWithSuffix("PREFERENCES", B_UTF8_ELLIPSIS) );
	beMenu->ItemAt(1)->SetLabel( LangWithSuffix("EDIT_PROFILE", B_UTF8_ELLIPSIS) );
	beMenu->ItemAt(3)->SetLabel( LangWithSuffix("ABOUT_BEAIM", B_UTF8_ELLIPSIS) );
	beMenu->ItemAt(5)->SetLabel( Language.get("LOGOUT_NAME") );
	beMenu->ItemAt(6)->SetLabel( Language.get("QUIT_NAME") );
	peopleMenu->ItemAt(0)->SetLabel( LangWithSuffix("CONTACT_USER", B_UTF8_ELLIPSIS) );
	peopleMenu->ItemAt(1)->SetLabel( LangWithSuffix("GET_USER_INFO", B_UTF8_ELLIPSIS) );
	peopleMenu->ItemAt(2)->SetLabel( LangWithSuffix("SEARCH_BY_EMAIL", B_UTF8_ELLIPSIS) );
	peopleMenu->ItemAt(4)->SetLabel( LangWithSuffix("EDIT_BUDDY_LIST", B_UTF8_ELLIPSIS) );
	peopleMenu->ItemAt(5)->SetLabel( LangWithSuffix("EDIT_BLOCK_LIST", B_UTF8_ELLIPSIS) );
	statMenu->ItemAt(statMenu->CountItems()-1)->SetLabel( LangWithSuffix("EDIT_AWAY_MSGS", B_UTF8_ELLIPSIS) );
	
	// no away messages?
	if( !prefs->CountAwayMessages() )
		statMenu->ItemAt(2)->SetLabel( Language.get("NO_AWAY_MSGS") );
		
	// offline group
	if( offlineGroup ) {
		int offPos = budView->IndexOf( offlineGroup );
		if( offPos >= 0 ) {
			offlineGroup->SetGroup( Language.get("OFFLINE_NAME") );
			budView->InvalidateItem(offPos);
		}
	}
}

//-----------------------------------------------------

void BuddyListWindow::WorkspaceActivated( int32 workspace, bool active ) {
	LessAnnoyingWindow::WorkspaceActivated( workspace, active );

	// if we're leaving the workspace, force-close the bubble
	ShowBubble( BPoint(), false, true );
}

//=====================================================

BuddyListMainView::BuddyListMainView(BRect rect)
				 : BView(rect, "BuddyList", B_FOLLOW_ALL, B_WILL_DRAW )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	BFont baseFont; 
	baseFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	baseFont.SetSize( 12.0 );
	
	// Add the outline control
	BRect r = Bounds();
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.top += 1;
	buddylistview = new BuddyListView( r );
	buddylistview->SetFont( &baseFont );
	buddylistview->SetInvocationMessage( new BMessage(BEAIM_BUDDY_INVOKED) );	
	buddylistview->SetViewColor( GetBeAIMColor(BC_WHITE) );
	
	// set up the scroller
	scroller = new BScrollView( "scroller", buddylistview, B_FOLLOW_ALL, 0, false, true, B_PLAIN_BORDER );
	AddChild( scroller );
}

//-----------------------------------------------------

void BuddyListMainView::Draw( BRect )
{
	BRect r = Bounds();
	SetHighColor( 156, 154, 156 );		// dark grey
	StrokeRect( r );
}

//=========================================================================

BuddyListView::BuddyListView( BRect frame )
			 : BOutlineListView( frame, "online_list", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL )
{
	userPopup = new BPopUpMenu( "userpopup", false );
	userPopup->AddItem( new BMenuItem( LangWithSuffix("BLST_MENU_CONTACT",B_UTF8_ELLIPSIS), NULL ) );
	userPopup->AddItem( new BMenuItem( LangWithSuffix("BLST_MENU_GETINFO",B_UTF8_ELLIPSIS), NULL ) );
}

//-------------------------------------------------------------------------

BuddyListView::~BuddyListView() {
	delete userPopup;
}

//-------------------------------------------------------------------------

void BuddyListView::MouseDown( BPoint cursor ) {

	uint32 buttons;
	int32 clickIndex;
	BLMainItem* clickItem;
	BMessage* popupMsg;
	BMenuItem* selected;
	int32 option;
	
	// make sure there ain't no bubble being shown!
	BuddyListWindow* parentWindow = dynamic_cast<BuddyListWindow*>( Window() );
	if( parentWindow )
		parentWindow->ShowBubble( BPoint(), false, true );

	// get the mouse position
	GetMouse( &cursor, &buttons, false );
		
	// it was a right mouse button click, popup the menu
	if( buttons & B_SECONDARY_MOUSE_BUTTON ) {
		
		// get the index of the item the user clicked on, and select it
		clickIndex = IndexOf(cursor);
		if( clickIndex < 0 )			// not an item?
			return;
		Select(clickIndex);

		// get a pointer to the item, and check to see if it's a person or not
		clickItem = dynamic_cast<BLMainItem*>( ItemAt(clickIndex) );
		if( !clickItem )
			return;

		// it's all good... display the popup menu
		ConvertToScreen( &cursor );
		selected = userPopup->Go( cursor );

		// if the user selected something, take action based on the choice
		if( selected ) {
			option = userPopup->IndexOf(selected);
			popupMsg = new BMessage(BEAIM_OPEN_USER_WINDOW);
			popupMsg->AddString( "userid", clickItem->GetUser().UserString() );
			popupMsg->AddPointer( "poswindow", Window() );
			switch( option ) {
				case 0: popupMsg->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
					break;
				case 1: popupMsg->AddInt32( "wtype", (int32)USER_INFO_TYPE );
					break;
			}
			PostAppMessage( popupMsg );			
		}
	} else
		BOutlineListView::MouseDown( cursor );
}

//=====================================================

BLMainItem::BLMainItem( AIMUser u, BFont* font )
		  : BListItem( (uint32)1 )
{
	ourFont = font;
	user = u;
	status = BS_OFFLINE;
	warningLevel = 0;

	if( awayIcon == NULL )
		GetBitmapFromResources( awayIcon, 3827, AppFileName );
	if( alertIcon == NULL )
		GetBitmapFromResources( alertIcon, 3828, AppFileName );
	if( blockIcon == NULL )
		GetBitmapFromResources( blockIcon, 3829, AppFileName );
	if( enterIcon == NULL )
		GetBitmapFromResources( enterIcon, 3830, AppFileName );
	if( exitIcon == NULL )
		GetBitmapFromResources( exitIcon, 3831, AppFileName );
		
	MakeDisplay();
}

//-----------------------------------------------------

AIMUser BLMainItem::GetUser() {
	return user;
}

//-----------------------------------------------------

void BLMainItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;
	BRect awayRect = BRect(0,0,11,11);

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight; 
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;
	
	// Grab the owner's font, to be fiddled with if needed
	BFont ownerFont( ourFont );
	
	// does the background need to be redrawn?
	frame.left++;
	if( IsSelected() || complete ) {
		
		// pick the appropriate background color
        if( IsSelected() ) {
        	color = kHighlight;
        	owner->SetLowColor( 222, 219, 222 );
        } else {
			color = owner->ViewColor();
			owner->SetLowColor( 255, 255, 255 );
		}
	
		// draw the background		
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}

	// set various color/font state attributes
	ownerFont.SetFace( B_REGULAR_FACE );
	owner->SetHighColor(0,0,0);
	if( status & BS_ENTERING )
		ownerFont.SetFace( B_BOLD_FACE );
	if( status & BS_LEAVING || status & BS_OFFLINE )
		ownerFont.SetFace( B_ITALIC_FACE );
	if( status & BS_LEAVING || status & BS_IDLE || status & BS_OFFLINE )
		owner->SetHighColor(105,106,105);

	// set the font and draw the string
	owner->SetFont( &ownerFont );
	owner->MovePenTo(frame.left + 5, frame.bottom - 2);
	owner->DrawString( display.String() );
	
	// draw the icon, if any
	awayRect.OffsetTo( frame.left - 13, frame.top + 2 );	
	
	// draw the enter/exit icon if needed
	//if( status & BS_ALERT )
	//	owner->DrawBitmap( alertIcon, awayRect );	
	if( status & BS_AWAY )
		owner->DrawBitmap( awayIcon, awayRect );
	else if( status & BS_ENTERING )
		owner->DrawBitmap( enterIcon, awayRect );
	else if( status & BS_LEAVING )
		owner->DrawBitmap( exitIcon, awayRect );
	//else if( status & BS_BLOCK )
	//	owner->DrawBitmap( blockIcon, awayRect );	

	// reset the font and colors
	owner->SetLowColor( 255, 255, 255 );
	owner->SetHighColor( 0, 0, 0 );
}

//-----------------------------------------------------

void BLMainItem::SetUser( AIMUser u ) {
	user = u;
	MakeDisplay();
}

//-----------------------------------------------------

void BLMainItem::SetStatus( int st ) {
	status = st;
}

//-----------------------------------------------------

int32 BLMainItem::WarningLevel() {
	return warningLevel;
}

//-----------------------------------------------------

void BLMainItem::SetWarningLevel( int32 wl ) {
	warningLevel = wl;
	MakeDisplay();
}

//-----------------------------------------------------

void BLMainItem::MakeDisplay() {

	char wls[15];

	display = user.Username();
	if( warningLevel ) {
		sprintf( wls, "  (%ld%%)", warningLevel );
		display.Append( wls );
	}
}

//=====================================================

BLGroupItem::BLGroupItem( BString grp, BFont* font, bool off )
		    : BListItem( (uint32)0 )	// indented to the zero level
{
	ourFont = font; 
	isOffline = off;
	groupName = grp;
	online = 0;
	total = 0;
	
	SetCounts( online, total );
}

//-----------------------------------------------------

void BLGroupItem::SetCounts( int32 on, int32 ttl ) {

	char countThinger[20];

	if( on != -1 )
		online = on;
	if( ttl != -1 )
		total = ttl;
		
	sprintf( countThinger, "  (%ld/%ld)", online, total );
	drawString = groupName;
	drawString.Append( countThinger );
}

//-----------------------------------------------------

BString BLGroupItem::Group() {
	return groupName;
}

//-----------------------------------------------------

void BLGroupItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight; 
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;
	
	// Grab the owner's font, to be fiddled with if needed
	BFont ownerFont( *ourFont );
	
	// does the background need to be redrawn?
	frame.left++;
	if( IsSelected() || complete ) {
		
		// pick the appropriate background color
        if( IsSelected() ) {
        	color = kHighlight;
        	owner->SetLowColor( 222, 219, 222 );
        } else {
			color = owner->ViewColor();
			owner->SetLowColor( 255, 255, 255 );
		}
	
		// draw the background		
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}
	
	// Set colors and font attributes based on buddy status
	if( isOffline ) {
		owner->SetHighColor(105,106,105);
		ownerFont.SetFace( B_ITALIC_FACE );
	} else {	
		owner->SetHighColor(0,77,74);
	}
	if( IsExpanded() )
		ownerFont.SetFace(B_BOLD_FACE);
	else
		ownerFont.SetFace(B_REGULAR_FACE);

	// set the font and draw the string
	owner->SetFont( &ownerFont );
	owner->MovePenTo(frame.left + 5, frame.bottom - 2);
	owner->DrawString( drawString.String() );
	
	// reset the font and colors
	owner->SetLowColor( 255, 255, 255 );
	owner->SetHighColor( 0, 0, 0 );
}

//-----------------------------------------------------

void BLGroupItem::SetGroup( BString grp ) {
	groupName = grp;
	SetCounts( online, total );
}

//=====================================================

BLBubbleWindow::BLBubbleWindow()
			  : BWindow( BRect(-100,-100,-50,-50), "BeAIMBubble", B_BORDERED_WINDOW_LOOK,
			  			 B_FLOATING_ALL_WINDOW_FEEL, B_NOT_MOVABLE | B_AVOID_FOCUS )
{
	font_height fhInfo;

	// the official bubble window font
	theFont = be_plain_font;
	theFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	theFont.SetSize(11.0);
	
	// get the height constant all figured out 
	theFont.GetHeight( &fhInfo );
	heightConst = fhInfo.ascent + fhInfo.descent + fhInfo.leading + 1;

	// make the BTextView that does all the real work around here	
	textview = new BTextView( BRect(0,0,50,50), "", BRect(2,2,48,48), 
							  B_FOLLOW_ALL_SIDES, B_WILL_DRAW );
	textview->SetFont(&theFont);
	textview->MakeEditable(false);
	textview->MakeSelectable(false);
	textview->SetStylable(true);
	textview->SetWordWrap(false);
	textview->SetLowColor(255,255,220);
	textview->SetViewColor(255,255,220);
	textview->SetHighColor(0,0,0);
	textview->SetFontAndColor(&theFont);
	AddChild(textview);

	// no text just yet so no biggest width...
	biggestWidth = 10;
	numLines = 0;
	
	// the bubble isn't showing yet, obviously...
	isBeingShown = false;
}

//-----------------------------------------------------

BLBubbleWindow::~BLBubbleWindow() {
}

//-----------------------------------------------------

void BLBubbleWindow::GetMouseInfo( BPoint& point, ulong& buttons ) {
	Lock();
	textview->GetMouse( &point, &buttons );
	textview->ConvertToScreen( &point );
	Unlock();
}

//-----------------------------------------------------

void BLBubbleWindow::SetBeingShown( bool show, BPoint* point ) {

	BPoint movePoint;
	Lock();

	// hiding the bubble?
	if( !show ) {
	
		// hide the bubble
		MoveTo( -1000, -1000 );
		if( !IsHidden() )
			Hide();	
	
		// all done hiding the bubble
		isBeingShown = false;
		Unlock();
		return;	
	}
	
	// we're going to show the bubble... make sure we have a point
	if( !point ) {
		Unlock();
		return;
	}
		
	// now move the bubble to the point
	movePoint = *point;
	MoveTo( movePoint );
	
	// now display it! yay!
	SetWorkspaces(B_CURRENT_WORKSPACE);
	if( IsHidden() )
		Show();

	// all done showing the bubble
	isBeingShown = true;
	Unlock();	
}

//-----------------------------------------------------

bool BLBubbleWindow::IsBeingShown() {
	bool ret;
	Lock();
	ret = isBeingShown;
	Unlock();
	return ret;
}

//-----------------------------------------------------

void BLBubbleWindow::AddDisplayString( BString nString ) {

	float testWidth, fHeight;
	BRect fRect;
	
	// get the width... add 9 because StringWidth isn't terribly accurate...
	testWidth = textview->StringWidth(nString.String());
	testWidth += 9;
	
	// tack the new string onto the display string
	displayString.Append( nString );
	displayString.Append( "\n" );
	
	// bigger than the current big width?
	if( testWidth > biggestWidth )
		biggestWidth = testWidth;
	
	// figure out the new height
	fHeight = heightConst * (++numLines);
	fHeight += 3;
	
	// construct a new text rectangle
	fRect = BRect(0,0,biggestWidth,fHeight);
	fRect.InsetBy(3,2);

	// now set all this stuff
	Lock();
	ResizeTo( biggestWidth, fHeight );
	textview->ResizeTo( biggestWidth, fHeight );
	textview->SetTextRect( fRect );
	textview->SetText( displayString.String() );
	Unlock();
}

//-----------------------------------------------------

void BLBubbleWindow::ClearDisplayString() {

	rgb_color kBlack;
	kBlack.red = 0;
	kBlack.green = 0;
	kBlack.blue = 0;
	text_run normalRun;

	// reset the font
	theFont.SetFace(B_REGULAR_FACE);
	theFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	theFont.SetSize(11.0);

	// do that funky text_run stuff
	text_run_array* newStyles = (text_run_array*)malloc( sizeof(text_run_array) +
								(sizeof(text_run) * 1));
	normalRun.offset = 0;
	normalRun.font = theFont;
	normalRun.font.SetSize( theFont.Size() );
	normalRun.color = kBlack;
	newStyles->runs[0] = normalRun;
	newStyles->count = 1;

	// no text anymore! hahahahahaha!
	displayString = "";
	biggestWidth = 10;
	numLines = 0;
	Lock();
	ResizeTo( biggestWidth, 5 );
	textview->SetStylable(false);
	textview->ResizeTo( biggestWidth, 5 );
	textview->SetFont(&theFont);
	textview->SetStylable(true);
	textview->SetRunArray( 0, 100, newStyles );
	Unlock();
	free(newStyles);
}

//-----------------------------------------------------

void BLBubbleWindow::GetDimensions( float& width, float& height ) {

	width = height = 0;
	if( Lock() ) {
		BRect rect = Frame();
		width = rect.Width();
		height = rect.Height();
		Unlock();
	}
}

//-----------------------------------------------------

void BLBubbleWindow::SetNumBoldChars( int nbc ) {

	rgb_color kBlack;
	kBlack.red = 0;
	kBlack.green = 0;
	kBlack.blue = 0;
	
	// text runs
	text_run boldRun, normalRun;

	// allocate some room for the text_run_array
	text_run_array* newStyles = (text_run_array*)malloc( sizeof(text_run_array) +
								(sizeof(text_run) * 2));
	// the bold run thinger
	boldRun.offset = 0;
	boldRun.font = theFont;
	boldRun.color = kBlack;
	boldRun.font.SetFace( B_BOLD_FACE );
	boldRun.font.SetSize( theFont.Size() * 1.1 );
	
	// the normal run thinger
	normalRun.offset = nbc;
	normalRun.font = theFont;
	normalRun.font.SetSize( theFont.Size() );
	normalRun.color = kBlack;
	
	// set the styles in the run_array thing
	newStyles->runs[0] = boldRun;
	newStyles->runs[1] = normalRun;
	newStyles->count = 2;
	
	// finally, apply the style
	Lock();
	textview->SetRunArray( 0, displayString.Length(), newStyles );
	Unlock();
	free( newStyles );
}

//=====================================================
