#include <Application.h>
#include <Bitmap.h>
#include <Alert.h>
#include <MenuItem.h>
#include <String.h>
#include <stdlib.h>
#include "Globals.h"
#include "PeopleEdit.h"
#include "UserManager.h"
#include "MiscStuff.h"
#include "AIMUser.h"

//-----------------------------------------------------

PersonAddEditWindow::PersonAddEditWindow( BRect frame, BWindow* mk, AIMUser usr, BString grp, editType mode )
	 				: BWindow(frame, "", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	user = usr;
	group = grp;
	maker = mk;
	now = true;

	// Set the correct title based on the edit mode
	switch( mode ) {
		case ET_ADDBUDDY:
			ResizeBy( 0, 23 );		
			SetTitle( Language.get("BLE_ADD_BUDDY") );
			msgID = BEAIM_ADD_BUDDY;
			break;
		case ET_ADDGROUP:
			SetTitle( Language.get("BLE_ADD_GROUP") );
			msgID = BEAIM_ADD_GROUP;
			break;
		case ET_BUDDYEDIT:
			ResizeBy( 0, 23 );		
			SetTitle( "Edit Buddy" );
			msgID = BEAIM_CHANGE_BUDDY;
			break;
		case ET_GROUPEDIT:
			SetTitle( Language.get("BLE_EDIT_GROUP") );
			msgID = BEAIM_CHANGE_GROUP;
			break;
	}
	
	// Set up the view
	BRect aRect( Bounds() );
	personView = new PersonAddEditView( aRect, msgID, group );
	AddChild( personView );

	// Copy in the text, if needed
	if( mode == ET_BUDDYEDIT || (mode == ET_ADDBUDDY && user.Username().Length()) )
		personView->personName->SetText( (char*)user.UserString() );
	else if( mode == ET_GROUPEDIT )
		personView->personName->SetText( (char*)group.String() );

	// if you want the text to be selected when the dialog starts, move this line 
	//   up before the text is copied in
	personView->personName->MakeFocus( true );

	// let's be modal! yay!
	SetFeel( B_MODAL_SUBSET_WINDOW_FEEL );
	AddToSubset( mk );
	
	// make all the language stuff nice and happy
	RefreshLangStrings();
}

//-----------------------------------------------------

void PersonAddEditWindow::RefreshLangStrings() {

	int32 fullWidth = (int32)Bounds().Width();
	BButton* button;

	// do the cancel button
	button = personView->CancelButton;
	button->SetLabel( Language.get("CANCEL_LABEL") );
	button->ResizeToPreferred();
	button->MoveTo( fullWidth - (button->Bounds().Width()+7), button->Frame().top );
	fullWidth -= (int32)button->Bounds().Width();
	
	// do the save button
	button = personView->SaveButton;
	button->SetLabel( Language.get("SAVE_LABEL") );
	button->ResizeToPreferred();
	button->MoveTo( fullWidth - (button->Bounds().Width()+11), button->Frame().top );
}

//-----------------------------------------------------

PersonAddEditWindow::~PersonAddEditWindow() {

}

//-----------------------------------------------------

bool PersonAddEditWindow::QuitRequested()
{
	return(true);
}

//-----------------------------------------------------

void PersonAddEditWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case B_OK:
			if( Save() )
				Close();
			break;
						
		case B_CANCEL:
			Close();
			break;
	
		default:
			BWindow::MessageReceived(message);
	}
}

//-----------------------------------------------------

bool PersonAddEditWindow::Save() {

	// make a message
	BMessage* sendMessage = new BMessage( msgID );

	// set the appropriate fields	
	switch( msgID ) {

		case BEAIM_CHANGE_BUDDY: {
			AIMUser to = AIMUser(personView->personName->Text());
			if( users->IsABuddy(to) ) {
				windows->ShowMessage( Language.get("BLE_BUDDY_THERE") );
				return false;
			}
			sendMessage->AddString( "oldname", user.UserString() );
			sendMessage->AddString( "newname", to.UserString() );
			sendMessage->AddString( "group", (char*)personView->groupMenu->FindMarked()->Label() );
			break;
		}

		case BEAIM_CHANGE_GROUP: {
			BString to = BString(personView->personName->Text());
			if( users->IsAGroup(to) ) {
				windows->ShowMessage( Language.get("BLE_GROUP_THERE") );
				return false;
			}			
			sendMessage->AddString( "oldname", group.String() );
			sendMessage->AddString( "newname", to.String() );
			break;
		}

		case BEAIM_ADD_BUDDY: {
			AIMUser user = AIMUser(personView->personName->Text());
			if( users->IsABuddy(user) ) {
				windows->ShowMessage( Language.get("BLE_BUDDY_THERE") );
				return false;
			}
			sendMessage->AddString( "userid", user.UserString() );
			sendMessage->AddString( "group", (char*)personView->groupMenu->FindMarked()->Label() );
			sendMessage->AddBool( "commitnow", now );
			break;
		}

		case BEAIM_ADD_GROUP: {
			BString group = BString(personView->personName->Text());
			if( users->IsAGroup(group) ) {
				windows->ShowMessage( Language.get("BLE_GROUP_THERE") );
				return false;
			}
			sendMessage->AddString( "group", personView->personName->Text() );
			break;
		}
		
		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;
	}

	// send and delete
	users->PostMessage( sendMessage );
	return true;
}

//-----------------------------------------------------

void PersonAddEditWindow::SetInitialName( BString name ) {
	personView->personName->SetText(name.String());
}

//-----------------------------------------------------

// the cancel function
void PersonAddEditWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//=====================================================

PersonAddEditView::PersonAddEditView( BRect rect, uint32 id, BString gp )
	   	   : BView(rect, "person_addedit_view", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );
	msgID = id;
	BRect btnRect(62,50,122,70);
	BMenuItem* groupItem;
	menuField = NULL;

	// Construct the views
	BRect textRectThang = BRect(8,15,192,35);
	textRectThang.right = Bounds().Width() - 8;
	if( id==BEAIM_ADD_GROUP || id==BEAIM_CHANGE_GROUP )
		personName = new BTextControl( textRectThang, "user_id", (const char*)LangWithSuffix("GROUP_LABEL",":"), "", NULL );
	else
		personName = new BTextControl( textRectThang, "user_id", (const char*)LangWithSuffix("BUDDY_LABEL",":"), "", NULL );
	personName->SetViewColor( 216, 216, 216 );
	personName->SetDivider( 75 );
	//personName->TextView()->SetMaxBytes( SCREEN_NAME_MAX );
	personName->MakeFocus( true );

	// Make the group selector (if needed)
	if( msgID == BEAIM_ADD_BUDDY || msgID == BEAIM_CHANGE_BUDDY ) {

		BString group;
		bool kg = false;
		int i = 0;

		// Get the first group, and create the menu w/ it
		users->GetGroups( group, true );
		groupMenu = new BMenu( group.String() );
		groupMenu->SetRadioMode( true );
		groupMenu->SetLabelFromMarked( true );
		
		// Add all the rest of the groups
		kg = users->GetGroups( group, true );
		while( kg ) {
			groupItem = new BMenuItem(group.String(), new BMessage(ADDEDIT_SELECTION_CHANGED));
			if( gp.Length() )
				groupItem->SetMarked( (bool)(group == gp) );
			else
				groupItem->SetMarked( (bool)(i == 0) );
			groupMenu->AddItem( groupItem );
			kg = users->GetGroups( group, false );
			i++;
		}

		// Now create the menufield
		BRect menuRect( 8, 37, 192, 62 );
		menuRect.right = Bounds().Width() - 8;
		menuField = new BMenuField( menuRect, "group_select", LangWithSuffix("GROUP_LABEL",":"), groupMenu );
		menuField->SetDivider( 77 );
		btnRect.OffsetBy( 0, 23 );
		ResizeBy( 0, 23 );
	}

	// Make the Save and Cancel buttons
	SaveButton = new BButton( btnRect, "savebutton", "Save", new BMessage((unsigned long)B_OK) );
	SaveButton->MakeDefault( true );
	btnRect.OffsetBy( 70, 0 );
	CancelButton = new BButton( btnRect, "cancelbutton", "Cancel", new BMessage(B_CANCEL) );
	
	// Now add them to the window
	AddChild( personName );
	if( menuField )
		AddChild( menuField );	
	AddChild( SaveButton );
	AddChild( CancelButton );
	//AddChild( Divider );
	
	personName->MakeFocus();
}

//-----------------------------------------------------

void PersonAddEditView::KeyDown( const char *bytes, int32 numBytes ) {

}

//-----------------------------------------------------
