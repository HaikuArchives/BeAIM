#include <Application.h>
#include <Box.h>
#include <Button.h>
#include "DLanguageClass.h"
#include "StringView.h"
#include "BuddyListEditor.h"
#include "PeopleEdit.h"
#include "Say.h"
#include "Globals.h"
#include "MiscStuff.h"

const int DO_DELETE = 'doDL';
const int DO_ADD_BUDDY = 'doaB';
const int DO_ADD_GROUP = 'doaG';
const int DO_IMPORT = 'doIP';
const int DO_EXPORT = 'doXP';
const int EDITOR_INVOKED = 'eDiK';
const int SEL_CHANGED = 'sLc#';

BRect peopleEditFrame(0,0,250,85);

//=====================================================

BuddyListSetupWindow::BuddyListSetupWindow( BRect frame, bool openImporter )
	 				: SingleWindowBase( SW_BUDDYLIST_EDITOR, frame, Language.get("EDIT_BUDDY_LIST"),
					  B_DOCUMENT_WINDOW, B_NOT_ZOOMABLE )
{
	ourFont = be_plain_font;
	ourFont.SetSize(12.0);

	// Set up the view
	BRect aRect( Bounds() );
	genView = new ListSetupView( aRect );
	AddChild( genView );
	Populate();

	// load the language strings
	RefreshLangStrings();

	// disable stuff
	DoSelectionEnabling(0);

	// if we're supposed to open the importer window too, do that
	if( openImporter )
		DoImport();

	// set the size limits for the window
	SetSizeLimits( 150, 5000, 200, 5000 );
}

//-----------------------------------------------------

BuddyListSetupWindow::~BuddyListSetupWindow() {

}

//-----------------------------------------------------

void BuddyListSetupWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case DO_ADD_GROUP:
			DoAddGroup();
			break;

		case DO_ADD_BUDDY:
			DoAddBuddy();
			break;

		case DO_DELETE:
			DoDelete();
			break;

		case EDITOR_INVOKED:
			DoEdit();
			break;

		case DO_IMPORT:
			DoImport();
			break;

		case DO_EXPORT:
			DoExport();
			break;

		case BEAIM_ADD_GROUP:
			DoRealAddGroup( msg );
			break;

		case BEAIM_ADD_BUDDY:
			DoRealAddBuddy( msg );
			break;

		case BEAIM_DELETE_BUDDY:
			DoRealDeleteBuddy( msg );
			break;

		case BEAIM_DELETE_GROUP:
			DoRealDeleteGroup( msg );
			break;

		case BEAIM_CHANGE_GROUP:
			DoRealChangeGroup( msg );
			break;

		case BEAIM_CHANGE_BUDDY:
			DoRealChangeBuddy( msg );
			break;

		case SEL_CHANGED:
			DoSelectionEnabling( msg->FindInt32("curseltype") );
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}

//-----------------------------------------------------

BLSetupItem* BuddyListSetupWindow::GetCurrentItem() {

	BLSetupItem* selItem;

	int32 selected = genView->listview->CurrentSelection();
	if( selected == -1 )
		return NULL;
	selItem = dynamic_cast<BLSetupItem*>(genView->listview->ItemAt( selected ));
	return selItem;
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoDelete() {

	BLSetupItem* selItem;
	BString message;
	BAlert* alert;
	AIMUser user;
	bool isBuddy;

	// get the current item
	if( !(selItem = GetCurrentItem()) )
		return;
	user = selItem->GetUser();

	// buddy or group? build the delete message
	isBuddy = !selItem->IsGroupItem();
	if( isBuddy ) {
		message = BString( Language.get("BLE_DELBUDDY_WARN") );
		message.IReplaceAll( "%BUDDY", user.UserString() );
	} else {
		message = BString( Language.get("BLE_DELGROUP_WARN") );
		message.IReplaceAll( "%GROUP", user.UserString() );
	}
	message.Append( "\n\n" );
	message.Append( Language.get("BLE_AREYOUSURE") );

	// ask for permission, then remove the sucker
	alert = new BAlert("title", message.String(), Language.get("CANCEL_LABEL"), Language.get("REMOVE_LABEL"), NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT);
	alert->SetShortcut( 0, B_ESCAPE );
	if( !alert->Go() )
		return;

	// it's a group
	if( !isBuddy ) {
		users->RemoveGroup( user.UserString(), false );
	}

	// it's a buddy
	else {
		users->RemoveBuddy( user, false );
	}
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoAddGroup() {

	PersonAddEditWindow* addWindow = NULL;

	// make a frame for the new window
	windows->MakeDialogFrame( peopleEditFrame, this );

	// create the window and open it
	addWindow = new PersonAddEditWindow( peopleEditFrame, (BWindow*)this, AIMUser(), BString(), ET_ADDGROUP );
	addWindow->Show();
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoAddBuddy() {

	BLSetupItem* curGroup;
	BString group;
	int32 curSel;

	// try to get the currently selected group item
	curSel = genView->listview->CurrentSelection();
	curGroup = (BLSetupItem*)genView->listview->ItemAt(curSel);
	if( curGroup ) {
		if( curGroup->OutlineLevel() )
			curGroup = (BLSetupItem*)genView->listview->Superitem(curGroup);
		if( curGroup )
			group = curGroup->GetUser().Username();
	}

	// do it like everybody else does... let the windowmanager handle it
	windows->MakeAddBuddyWindow( AIMUser(), this, false, group );
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoImport()
{
	BMessage* nMsg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
	nMsg->AddInt32("wtype", SW_BUDDYLIST_IMPORTER );
	nMsg->AddInt32("posnum", 2);
	PostAppMessage( nMsg );
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoExport() {

}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealAddGroup( BMessage* msg ) {

	BLSetupItem* item = new BLSetupItem( msg->FindString("group"), &ourFont, true );
	genView->listview->AddItem( item, genView->listview->FullListCountItems() );
	genView->listview->Select( genView->listview->IndexOf(item) );
	genView->listview->ScrollToSelection();
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealAddBuddy( BMessage* msg ) {

	BLSetupItem* gpItem = NULL;
	BLSetupItem* budItem = NULL;
	BOutlineListView* list = genView->listview;
	BString group = msg->FindString("group");
	bool found = false;

	// find the right group item
	for( int32 i = 0; i < list->CountItems(); ++i ) {
		gpItem = (BLSetupItem*)list->ItemAt(i);
		if( !gpItem->OutlineLevel() && gpItem->GetUser().Username() == group ) {
			found = true;
			break;
		}
	}

	// return if not found
	if( !found )
		return;

	// make and attach the new buddy item
	budItem = new BLSetupItem( AIMUser(msg->FindString("userid")), &ourFont );
	//list->AddUnder( budItem, gpItem );
	list->AddItem( budItem, list->IndexOf(gpItem) + list->CountItemsUnder(gpItem,true) + 1 );
	list->Select( list->IndexOf(budItem) );
	list->ScrollToSelection();
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealDeleteBuddy( BMessage* msg ) {
	short u, g;
	AIMUser user = AIMUser(msg->FindString("userid"));
	BOutlineListView* list = genView->listview;
	BLSetupItem* budItem = NULL;
	bool found = true;

	users->IsABuddy(user, &u, &g);
	user.SetSSIUserID(u);
	user.SetSSIGroupID(g);

	// find the right buddy item
	for( int32 i = 0; i < list->FullListCountItems(); ++i ) {
		budItem = (BLSetupItem*)list->FullListItemAt(i);
		if( budItem->OutlineLevel() && budItem->GetUser() == user ) {
			found = true;
			break;
		}
	}

	// if we found the right item, delete it
	if( found )
		list->RemoveItem(budItem);
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealDeleteGroup( BMessage* msg ) {

	BLSetupItem* gpItem = NULL;
	BLSetupItem* budItem = NULL;
	BOutlineListView* list = genView->listview;
	BString group = msg->FindString("group");
	GenList<BLSetupItem*> nukeList;
	bool found = false;

	// find the right group item
	for( int32 i = 0; i < list->CountItems(); ++i ) {
		gpItem = (BLSetupItem*)list->ItemAt(i);
		if( !gpItem->OutlineLevel() && gpItem->GetUser().Username() == group ) {
			found = true;
			break;
		}
	}

	// return if it ain't there
	if( !gpItem )
		return;

	// cache all the subitems (aka buddies) in the list
	int32 base = list->FullListIndexOf(gpItem) + 1;
	for( int32 i = 0; i < list->CountItemsUnder(gpItem,true); ++i ) {
		budItem = (BLSetupItem*)list->FullListItemAt(base+i);
		nukeList.Add( budItem );
	}

	// remove and delete the main item
	list->RemoveItem( gpItem );
	delete gpItem;

	// finally, delete all the stored items
	while( nukeList.Pop(budItem) )
		delete budItem;
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealChangeGroup( BMessage* msg ) {
	BOutlineListView* list = genView->listview;
	BString from = msg->FindString("oldname");
	BString to = msg->FindString("newname");
	BLSetupItem* gpItem = NULL;
	bool found = false;

	// find the right group item
	for( int32 i = 0; i < list->CountItems(); ++i ) {
		gpItem = (BLSetupItem*)list->ItemAt(i);
		if( !gpItem->OutlineLevel() && gpItem->GetUser().Username() == from ) {
			found = true;
			break;
		}
	}

	// return if it ain't there
	if( !gpItem )
		return;

	// change the name
	gpItem->SetUser( AIMUser(to) );
	list->InvalidateItem( list->FullListIndexOf(gpItem) );
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoRealChangeBuddy( BMessage* msg ) {
	BOutlineListView* list = genView->listview;
	AIMUser from = AIMUser( msg->FindString("oldname") );
	AIMUser to = AIMUser( msg->FindString("newname") );
	BLSetupItem* budItem = NULL;
	bool found = false;

	// find the right buddy item
	for( int32 i = 0; i < list->FullListCountItems(); ++i ) {
		budItem = (BLSetupItem*)list->FullListItemAt(i);
		if( budItem->OutlineLevel() && budItem->GetUser() == from ) {
			found = true;
			break;
		}
	}

	// return if it ain't there
	if( !budItem )
		return;

	// change the name
	budItem->SetUser( to );
	list->InvalidateItem( list->FullListIndexOf(budItem) );
}

//-----------------------------------------------------

// the cancel function
void BuddyListSetupWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if we're closing, "save" any changes that have been made
	if( msg->what == B_QUIT_REQUESTED ) {
		users->PostMessage( new BMessage(BEAIM_BUDDYLIST_COMMIT) );
// the SSI reloader is broken... :(
//		aimnet->ReloadSSIList();
	}

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			return;
		}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

void BuddyListSetupWindow::Populate() {

	BListSetupView* lView;
	BLSetupItem* tmpItem;
	int32 counter = 0;
	AIMUser tmpUser;
	BString group;
	bool kg, kg2;

	// handle some silly font details
	BFont ourFont = be_plain_font;
	ourFont.SetSize(12.0);

	// first, nuke everything in there
	lView = genView->listview;
	lView->Clear();

	// now fill 'er up!
	kg = users->GetGroups( group, true );
	while( kg ) {

		// create the item for the group
		tmpItem = new BLSetupItem( group, &ourFont, true );
		lView->AddItem( tmpItem, counter++ );

		// grab all the users from this group
		kg2 = users->GetGroupBuddies( tmpUser, group, true );
		while( kg2 ) {
			tmpItem = new BLSetupItem( tmpUser.Username(), &ourFont, false );
			lView->AddItem( tmpItem, counter++ );
			kg2 = users->GetGroupBuddies( tmpUser, group, false );
		}

		kg = users->GetGroups(group, false);
	}
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoEdit() {

	BLSetupItem* selItem;
	PersonAddEditWindow* editWindow = NULL;
	BRect itemFrame;
	BPoint point;
	AIMUser user;

	// get the current item
	if( !(selItem = GetCurrentItem()) )
		return;

	int32 selected = genView->listview->CurrentSelection();
	itemFrame = genView->listview->ItemFrame( selected );
	point = itemFrame.LeftTop();
	point.y -= genView->scroll->ScrollBar(B_VERTICAL)->Value();
	point.x += 20;
	point.y += 5;
	ConvertToScreen( &point );

	//  it's a buddy
	// note: disallowing buddy editing for this release... it's a pain and doesn't make a lot of sense
	if( selItem->OutlineLevel() ) {
		return;
//		BString buddyGroup;
//
//		windows->MakeDialogFrame( peopleEditFrame, point );
//		user = AIMUser(selItem->GetUser());
//		users->GetGroupOfBuddy( user, buddyGroup );
//		editWindow = new PersonAddEditWindow( peopleEditFrame, (BWindow*)this, user, buddyGroup, ET_BUDDYEDIT );
	}

	// it's a group
	else {
		windows->MakeDialogFrame( peopleEditFrame, point );
		editWindow = new PersonAddEditWindow( peopleEditFrame, (BWindow*)this, AIMUser(), selItem->GetUser().UserString(), ET_GROUPEDIT );
	}

	editWindow->Show();
}

//-----------------------------------------------------

void BuddyListSetupWindow::DoSelectionEnabling( int type ) {

	switch( type ) {

		case 0:
			genView->editButton->SetEnabled(false);
			genView->deleteButton->SetEnabled(false);
			break;

		case 1:
			genView->editButton->SetEnabled(false);
			genView->deleteButton->SetEnabled(true);
			break;

		case 2:
			genView->editButton->SetEnabled(true);
			genView->deleteButton->SetEnabled(true);
			break;
	}
}


//-----------------------------------------------------

void BuddyListSetupWindow::RefreshLangStrings() {

	int32 maxWidth = 0;
	float buttonHeight;
	BButton* button;

	// set the title first...
	SetTitle( Language.get("EDIT_BUDDY_LIST") );

	float leftSide = genView->addBuddyButton->Frame().left;

	// set buttons and stuff
	button = genView->addBuddyButton;
	button->SetLabel( Language.get("BLE_ADD_BUDDY") );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	button = genView->addGroupButton;
	button->SetLabel( Language.get("BLE_ADD_GROUP") );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	button = genView->editButton;
	button->SetLabel( Language.get("EDIT_LABEL") );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	button = genView->deleteButton;
	button->SetLabel( Language.get("REMOVE_LABEL") );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	button = genView->importButton;
	button->SetLabel( LangWithSuffix("BLE_IMPORT", B_UTF8_ELLIPSIS) );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	button = genView->doneButton;
	button->SetLabel( Language.get("CLOSE_LABEL") );
	button->ResizeToPreferred();
	if( button->Bounds().IntegerWidth() > maxWidth )
		maxWidth = button->Bounds().IntegerWidth();

	// resize the window
	ResizeTo( 228 + maxWidth, Bounds().Height() );
	genView->ResizeTo( Bounds().Width(), Bounds().Height() );

	// resize the buttons
	buttonHeight = button->Bounds().Height();
	genView->addBuddyButton->ResizeTo( maxWidth, buttonHeight );
	genView->addGroupButton->ResizeTo( maxWidth, buttonHeight );
	genView->editButton->ResizeTo( maxWidth, buttonHeight );
	genView->deleteButton->ResizeTo( maxWidth, buttonHeight );
	genView->importButton->ResizeTo( maxWidth, buttonHeight );
	genView->doneButton->ResizeTo( maxWidth, buttonHeight );

	// now move 'em back to where they're supposed to be
	genView->addBuddyButton->MoveTo( leftSide, genView->addBuddyButton->Frame().top );
	genView->addGroupButton->MoveTo( leftSide, genView->addGroupButton->Frame().top );
	genView->editButton->MoveTo( leftSide, genView->editButton->Frame().top );
	genView->deleteButton->MoveTo( leftSide, genView->deleteButton->Frame().top );
	genView->importButton->MoveTo( leftSide, genView->importButton->Frame().top );
	genView->doneButton->MoveTo( leftSide, genView->doneButton->Frame().top );

	// muck around w/ the scrollview
	genView->scroll->ResizeTo( leftSide - 12, genView->scroll->Frame().Height() );
}

//=====================================================

ListSetupView::ListSetupView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );

	// make the new listview
	BRect listRect = BRect( 7, 7, 200, 260 );
	listview = new BListSetupView( listRect, "list_view" );
	listview->SetViewColor( GetBeAIMColor(BC_WHITE) );
	listview->SetFontSize(11);

	// make the scrollview
	AddChild(scroll = new BScrollView("list_scroll_view", listview,
			B_FOLLOW_ALL_SIDES, 0, false, true, B_PLAIN_BORDER));

	// make some buttons
	AddChild( addBuddyButton = new BButton( BRect(223,5,305,0), "add_buddy", "Add Buddy", new BMessage(DO_ADD_BUDDY), B_FOLLOW_RIGHT | B_FOLLOW_TOP ) );
	AddChild( addGroupButton = new BButton( BRect(223,30,305,0), "add_group", "Add Group", new BMessage(DO_ADD_GROUP), B_FOLLOW_RIGHT | B_FOLLOW_TOP ) );
	AddChild( editButton = new BButton( BRect(223,65,305,0), "edit_item", "Edit", new BMessage(EDITOR_INVOKED), B_FOLLOW_RIGHT | B_FOLLOW_TOP ) );
	AddChild( deleteButton = new BButton( BRect(223,90,305,0), "delete_item", "Delete", new BMessage(DO_DELETE), B_FOLLOW_RIGHT | B_FOLLOW_TOP ) );
	AddChild( importButton = new BButton( BRect(223,125,305,0), "import", "Import" B_UTF8_ELLIPSIS, new BMessage(DO_IMPORT), B_FOLLOW_RIGHT | B_FOLLOW_TOP ) );
	AddChild( doneButton = new BButton( BRect(223,238,305,0), "done", "Done", new BMessage(B_QUIT_REQUESTED), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM ) );

	// add the name view
	BRect r = Bounds();
	r.top = r.bottom - 13;
	r.right -= 14;
	statView = new NameStatusView( r );
	AddChild( statView );
}

//=====================================================

BLSetupItem::BLSetupItem( AIMUser u, BFont font, bool ig )
		   : BListItem( ig ? 0 : 1 )
{
	rightHeight = -1;
	user = u;
	ourFont = font;
	isGroup = ig;

	if( isGroup )
		ourFont.SetFace( B_BOLD_FACE );
}

//-----------------------------------------------------

void BLSetupItem::Update( BView *owner, const BFont *font ) {

	BListItem::Update(owner, font);
	if( rightHeight == -1 )
		rightHeight = Height() + 2;
	SetHeight(rightHeight);
}

//-----------------------------------------------------

AIMUser BLSetupItem::GetUser() {
	return user;
}

//-----------------------------------------------------

void BLSetupItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;
	float xDraw = frame.left + 4;
	float yDraw = frame.bottom - 4;

	frame.top++;
	frame.bottom--;

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight;
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;

	// does the background need to be redrawn?
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

	// set the font and draw the string
	owner->SetHighColor(0,0,0);
	owner->SetFont( &ourFont );
	owner->MovePenTo(xDraw, yDraw);
	owner->DrawString( user.Username().String() );

	// reset the font and colors
	owner->SetLowColor( 255, 255, 255 );
}

//-----------------------------------------------------

void BLSetupItem::SetUser( AIMUser u ) {
	user = u;
}

//=====================================================

BListSetupView::BListSetupView( BRect frame, const char* name, list_view_type type,
						uint32 resizingMode, uint32 flags ) : BOutlineListView( frame, name, type,
						resizingMode, flags )

{
	oldItemOver = -1;
	curSel = -1;
	startItem = -1;
	scroller = NULL;
	dragging = false;

	kWhite.red = kWhite.blue = kWhite.green = 255;
	kDrag.red = kDrag.green = 128;
	kDrag.blue = 255;
	kSel.red = kSel.blue = 222;
	kSel.green = 219;

	SetInvocationMessage( new BMessage(EDITOR_INVOKED) );
}

//-----------------------------------------------------

BListSetupView::~BListSetupView() {
}

//-----------------------------------------------------

bool BListSetupView::InitiateDrag( BPoint point, int32 index, bool wasSelected ) {

	BListItem* dragItem1;
	BLSetupItem* dragItem2;

	startItem = index;

	dragItem1 = ItemAt(index);
	if( !dragItem1 )
		return false;
	dragItem2 = dynamic_cast<BLSetupItem*>(dragItem1);
	if( !dragItem2 )
		return false;

	groupDrag = dragItem2->IsGroupItem();
	if( groupDrag )
		GroupsCanGoWhere( startItem );

	BMessage* msg = new BMessage(M_OK);
	BRect rect = ItemFrame(index);

	dragging = true;
	DragMessage( msg, rect );
	return true;
}

//-----------------------------------------------------

	// default version of this is empty, so no base-class calls required...
void BListSetupView::MouseMoved( BPoint point, uint32 transit, const BMessage* message ) {

	int32 itemOver;
	BRect frame;

	// if it's not in the view or there's no drag stuff, we ain't interested
	if( !message || !dragging )
		return;

	// get the item the thing is being dragged over
	itemOver = IndexOf( point );

	// if we've exited the view, erase the line
	if( transit == B_EXITED_VIEW ) {
		Window()->Lock();
		DrawInsertionLine( oldItemOver, false );
		Window()->Unlock();
		oldItemOver = -1;
		return;
	}

	// calculate the correct insertion point
	if( itemOver >= 0 ) {
		frame = ItemFrame(itemOver);
		if( point.y >= frame.top + (int(frame.Height()) / 2) )
			++itemOver;
	}

	// not gonna redraw the line if it's correct already... flicker bad!
	if( itemOver == oldItemOver )
		return;

	// Erase the old line and draw the new one
	Window()->Lock();
	DrawInsertionLine( oldItemOver, false );
	if( IsAllowed(itemOver) )
		DrawInsertionLine( itemOver, true );
	Window()->Unlock();
	oldItemOver = itemOver;
}

//-----------------------------------------------------

void BListSetupView::SelectionChanged() {

	BListItem* temp;
	BMessage* msg = new BMessage(SEL_CHANGED);

	// save the current selection
	curSel = CurrentSelection();

	// get the currently selected item
	temp = ItemAt( curSel );
	if( temp ) {

		// buddy or group?
		if( temp->OutlineLevel() )
			msg->AddInt32("curseltype", 1);
		else
			msg->AddInt32("curseltype", 2);
	}

	// no selection
	else msg->AddInt32("curseltype", 0);

	// send it to the parent window to deal with
	if( Window() )
		Window()->PostMessage( msg );
}

//-----------------------------------------------------

void BListSetupView::DrawInsertionLine( int32 pos, bool on ) {

	BRect frame;
	BPoint start;
	BPoint end;

	// sanity check... can't draw an invalid pos
	if( pos < 0 )
		return;

	// is this the last position?
	bool last = bool(pos == CountItems());
	if( last )
		--pos;

	// calculate the points and stuff
	frame = ItemFrame(pos);

	// put the coordinates in the right place
	if( last ) {
		start = frame.LeftBottom();
		end = frame.RightBottom();
	} else {
		start = frame.LeftTop();
		end = frame.RightTop();
	}
	if( !pos ) {
		start.y++;
		end.y++;
	}

	end.x -= 4;
	if( groupDrag )
		start.x += 11;
	else
		start.x += 21;

	// draw the line
	if( on ) {
		SetHighColor( kDrag );
		SetPenSize(2.0);
		StrokeLine( start, end );
	 	SetPenSize(1.0);
	}

	// erase the line
	else
	{
		// erasing above the first element?
		if( pos == 0 ) {
			SetHighColor( kWhite );
			start.y = end.y = 0;
			StrokeLine( start, end );
			if( startItem == 0 )
				SetHighColor( kSel );
			start.y = end.y = 1;
			StrokeLine( start, end );
		}

		// erasing below the last element?
		else if( last ) {
			SetHighColor( kWhite );
			StrokeLine( start, end );
			if( startItem == CountItems() - 1 )
				SetHighColor( kSel );
			--start.y;
			--end.y;
			StrokeLine( start, end );
		}

		// just erasing a normal element...
		else {
			SetHighColor( kWhite );
			SetPenSize( 2.0 );
			StrokeLine( start, end );
			SetPenSize( 1.0 );
		}
	}
}

//-----------------------------------------------------

void BListSetupView::MouseUp( BPoint point ) {

	BLSetupItem* theItem = NULL;
	BLSetupItem* theGroup = NULL;
	BLSetupItem* prevGroup = NULL;
	BLSetupItem* toMoveItem = NULL;
	int32 indexUnder = -1;
	BString theGroupName;
	BString prevGroupName;
	GenList<BListItem*> subItems;
	BListItem* tempItem;
	int32 insertPos = 0;

	// make sure we're supposed to be doing this
	if( !dragging )
		return;

	// OK, we're done w/ the dragging part
	dragging = false;

	// first, erase the old line
	Window()->Lock();
	DrawInsertionLine( oldItemOver, false );
	Window()->Unlock();

	// grab the item we're moving
	toMoveItem = (BLSetupItem*)ItemAt( startItem );

	// do nothing if either index is invalid
	if( !toMoveItem || startItem < 0 || oldItemOver < 0 || !IsAllowed(oldItemOver) ) {
		SelectionChanged();
		return;
	}

	// just moving a user...
	if( !groupDrag ) {
		theItem = (BLSetupItem*)ItemAt( oldItemOver );

		// First... if there *is* no item, it means that it's being moved to
		// the last spot. So we gotta get the last group item.
		if( !theItem ) {
			theGroup = (BLSetupItem*)ItemUnderAt( NULL, true, CountItemsUnder(NULL,true)-1 );
			indexUnder = CountItemsUnder(theGroup, false);
		}

		// we landed immediately after a group object...
		else if( !ItemAt(oldItemOver-1)->OutlineLevel() ) {
			theGroup = (BLSetupItem*)ItemAt(oldItemOver-1);

			// if the group is expanded, we put it at the top
			if( theGroup->IsExpanded() )
				indexUnder = 0;

			// otherwise, put it at the bottom of the group
			else
				indexUnder = CountItemsUnder(theGroup, false);
		}

		// (probably) the most common case... landed somewhere in the middle.
		else {
			// the last item in the group
			if( !theItem->OutlineLevel() ) {
				theGroup = (BLSetupItem*)Superitem(ItemAt(IndexOf(theItem)-1));
				indexUnder = CountItemsUnder(theGroup, false);
			}

			// really somewhere in the middle
			else {
				theGroup = (BLSetupItem*)Superitem(theItem);
				indexUnder = IndexOf(theItem) - IndexOf(theGroup) - 1;
			}
		}

		// move the items around in the OutlineListView
		int32 mvIndexUnder = indexUnder;
		if( Superitem(toMoveItem) == theGroup && FullListIndexOf(toMoveItem) < (FullListIndexOf(theGroup)+1+indexUnder) )
			--mvIndexUnder;
		RemoveItem( toMoveItem );
		AddItem( toMoveItem, FullListIndexOf(theGroup) + 1 + mvIndexUnder );
		if( Superitem(toMoveItem)->IsExpanded() ) {
			Select( FullListIndexOf(toMoveItem) );
			ScrollToSelection();
		} else {
			Select( FullListIndexOf(Superitem(toMoveItem)) );
			ScrollToSelection();
		}

		// now, finally, do the actual move
		users->MoveBuddy( toMoveItem->GetUser(), theGroup->GetUser().Username(), indexUnder );

		Verify();
		BListView::MouseUp(point);
	}


	// moving a group, not a user
	else {

		// moving to the first position?
		if( oldItemOver == 0 ) {
			prevGroupName = "";
			prevGroup = NULL;
		}

		// guess not... get the group that comes before the new location for this group
		else {
			prevGroup = (BLSetupItem*)ItemAt(oldItemOver-1);
			if( prevGroup->OutlineLevel() )
				prevGroup = (BLSetupItem*)Superitem(prevGroup);
			prevGroupName = prevGroup->GetUser().Username();
		}

		// store all of the subitems for this group and remove the actual item
		startItem = FullListIndexOf( toMoveItem );
		for( int32 i = 1; i <= CountItemsUnder(toMoveItem,true); ++i ) {
			tempItem = FullListItemAt(startItem + i);
			subItems.Add(tempItem);
		}
		RemoveItem( toMoveItem );

		// calculate the new insertion point for this item, if it's not being inserted up top
		if( prevGroup )
			insertPos = FullListIndexOf(prevGroup) + CountItemsUnder(prevGroup,true) + 1;

		// re-insert the actual item, then the subitems (backwards to maintain order)
		AddItem( toMoveItem, insertPos );
		while( subItems.Pop(tempItem) )
			AddUnder( tempItem, toMoveItem );

		// then do the actual move
		BString theGroupName = toMoveItem->GetUser().Username();
		int32 igPos;
		if( prevGroupName == "" )
			igPos = 0;
		else
			igPos = users->GroupPos(prevGroupName) + 1;
		if( igPos > users->GroupPos(theGroupName) )
			--igPos;
		users->MoveGroup( theGroupName, igPos );

		// select the group item
		Select( FullListIndexOf(toMoveItem) );
		ScrollToSelection();

	//	Verify();
		BListView::MouseUp(point);
	}
}

//-----------------------------------------------------

void BListSetupView::Verify() {

	bool kg;
	BString group;
	AIMUser user;
	BLSetupItem* stuff;
	//BeDC ver( "verifier", DC_BLUE );

	// this is only a debugging thing...
	//if( !beaimDebug )
		return;

	// verify the groups
	//ver.AddSeparator();
	kg = users->GetGroups( group, true );
	for( int32 j = 0; j < FullListCountItems(); ++j ) {
		stuff = dynamic_cast<BLSetupItem*>( FullListItemAt(j) );
		if( stuff->OutlineLevel() == 0 ) {
			//ver.SendFormat( "group - %s - %s", (char*)stuff->GetUser().UserString(), (char*)group.String() );
			if( stuff->GetUser().Username() != group ) {
				Say( "error in group location", (char*)group.String() );
				return;
			}
			kg = users->GetGroups( group, false );
		}
	}

	// verify the users
	//ver.AddSeparator();
	kg = users->GetAllBuddies( user, true );
	for( int32 j = 0; j < FullListCountItems(); ++j ) {
		stuff = dynamic_cast<BLSetupItem*>( FullListItemAt(j) );
		if( stuff->OutlineLevel() == 1 ) {
			//ver.SendFormat( "user - %s - %s", (char*)stuff->GetUser().UserString(), (char*)user.UserString() );
			if( stuff->GetUser() != user ) {
				Say( "error in buddy location", (char*)user.UserString() );
				return;
			}
			kg = users->GetAllBuddies( user, false );
		}
	}
}

//-----------------------------------------------------

bool BListSetupView::IsAllowed( int32 pos ) {

	// groups must actually move
	if( groupDrag ) {
		for( unsigned i = 0; i < goodGroupPositions.Count(); ++i ) {
			if( goodGroupPositions[i] == pos )
				return true;
		}
		return false;
	}

	// OK, we're done group-dragging... on to the username stuff
	// a username can't be the first item
	if( pos == 0 )
		return false;

	// can't go in the same place it started from either
	if( pos == startItem )
		return false;

	// or in the next spot for that matter
	if( pos == startItem+1 )
		return false;

	return true;
}

//-----------------------------------------------------

void BListSetupView::GroupsCanGoWhere( int32 movingWhat ) {

	// this function calculates a list of the spots where it
	// is safe to move the group at movingWhat.

	Window()->Lock();
	int32 count = CountItems();

	goodGroupPositions.Clear();
	for( int32 i = 0; i < count; ++i )
		if( !ItemAt(i)->OutlineLevel() )
			goodGroupPositions.Add(i);

	// we need more than one item for this to work...
	if( goodGroupPositions.Count() < 2 ) {
		goodGroupPositions.Clear();
		Window()->Unlock();
		return;
	}

	goodGroupPositions.Add( CountItems() );
	int32 gCount = goodGroupPositions.Count();
	for( int32 i = gCount-1; i >= 0; --i ) {
		if( goodGroupPositions[i] == movingWhat )
			goodGroupPositions.Delete(i);
		else if( i && goodGroupPositions[i-1] == movingWhat )
			goodGroupPositions.Delete(i);
	}

	Window()->Unlock();
}

//-----------------------------------------------------

void BListSetupView::Clear() {

	GenList<BListItem*> tempList;
	BListItem* temp;

	// first, grab all the list items
	for( int i = 0; i < CountItems(); ++i )
		tempList.Add( ItemAt(i) );

	// now clear them out, all at once, to avoid flicker
	MakeEmpty();

	// finally, do the actual deletions (MakeEmpty() doesn't).
	while( tempList.Pop(temp) )
		delete temp;

	// MakeEmpty also doesn't update the scroll bars so we gotta do it
	if( scroller )
		scroller->ScrollBar(B_VERTICAL)->SetRange(0,0);
}

//-----------------------------------------------------

// so we can save a copy of the scrollview pointer... the base class
// also stores this, but it's private so we can't get at it. Silly.
void BListSetupView::TargetedByScrollView( BScrollView* view ) {
	scroller = view;
}

//=====================================================
