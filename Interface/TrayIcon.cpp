#include <stdio.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <SupportKit.h>
#include <Bitmap.h>
#include <Deskbar.h>
#include "TrayIcon.h"
#include "Say.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "constants.h"

//---------------------------------------------------------

const char* DESKBAR_SIGNATURE = "application/x-vnd.Be-TSKB";
int32 deskbarIDNumber;

//---------------------------------------------------------

TrayIcon::TrayIcon(BRect frame, BBitmap* active, BBitmap* inactive)
	: BView(frame, beaimDebug ? "AIMDeskbar_d" : "AIMDeskbar", B_FOLLOW_NONE, B_WILL_DRAW | B_WILL_ACCEPT_FIRST_CLICK)
{
	ActiveIcon = active;
	InactiveIcon = inactive;
	deskbarIDNumber = 0;
}


//---------------------------------------------------------

TrayIcon::TrayIcon( BMessage *archive )
	: BView( archive )
{
	BMessage			bits;
	archive->FindMessage("ActiveIcon", &bits);
	ActiveIcon = new BBitmap(&bits);
	
	bits.MakeEmpty();
	archive->FindMessage("InactiveIcon", &bits);
	InactiveIcon = new BBitmap(&bits);
}

//---------------------------------------------------------

TrayIcon::~TrayIcon()
{
	if(ActiveIcon) delete ActiveIcon;
	if(InactiveIcon) delete InactiveIcon;
}

//---------------------------------------------------------

void TrayIcon::AddTrayIcon()
{	
	BDeskbar deskBar;
	deskBar.RemoveItem( beaimDebug ? "AIMDeskbar_d" : "AIMDeskbar" );
	
	BBitmap*	active;
	BBitmap*	inactive;
	GetBitmapFromResources(active, 962);
	GetBitmapFromResources(inactive, 962);
		
	// Archive a view into the msg.
	TrayIcon* replicant = new TrayIcon(BRect(0, 0, 15, 15), active, inactive);
	//replicant->Archive(&archiveMsg);
	
	deskBar.AddItem( replicant, &deskbarIDNumber );
}

//---------------------------------------------------------

void TrayIcon::RemoveTrayIcon()
{
	BDeskbar deskBar;
	deskBar.RemoveItem( deskbarIDNumber );
}

//---------------------------------------------------------

void TrayIcon::AttachedToWindow() 
{ 
	// set view color based on parent
	if (Parent()) 
		SetViewColor(Parent()->ViewColor()); 

	// call inherited class
	BView::AttachedToWindow();
}

//---------------------------------------------------------

void TrayIcon::Draw(BRect updateRect)
{
	BView::Draw(updateRect);
	SetDrawingMode(B_OP_OVER);

	// just draw the active icon for now until we can impliment status
	if(ActiveIcon) DrawBitmap(ActiveIcon);
}

//---------------------------------------------------------

void TrayIcon::MouseDown( BPoint where )
{
	BString appSig = BeAIMAppSig();

	uint32 mouseButtons;
	Window()->CurrentMessage()->FindInt32("buttons", (long*)&mouseButtons);
	BMessage* msg = NULL;
	switch (mouseButtons)
	{
		// left: send a message to BeAIM telling it to toggle minimization
		case B_PRIMARY_MOUSE_BUTTON: {
			BMessage message(BEAIM_TOGGLE_HIDDEN);
			BMessenger(appSig.String()).SendMessage(&message);
			break;
		}

		// right: popup a menu (which does very little at the moment...)
		case B_SECONDARY_MOUSE_BUTTON: {
			BPopUpMenu* popup = new BPopUpMenu("Status",false,false);
			popup->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED) ));
			popup->AddItem(new BMenuItem("Logout", new BMessage(BEAIM_LOGOUT) ));
			popup->AddSeparatorItem();
			popup->AddItem(new BMenuItem("About BeAIM" B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED)));

			msg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
			msg->AddInt32( "wtype", SW_PREFERENCES );
			msg->AddBool( "globalonly", false );
			popup->AddItem(new BMenuItem("Preferences" B_UTF8_ELLIPSIS, msg));

			msg = new BMessage(BEAIM_OPEN_SINGLE_WINDOW);
			msg->AddInt32( "wtype", SW_PROFILE_EDITOR );
			popup->AddItem(new BMenuItem("Edit Profile" B_UTF8_ELLIPSIS, msg));
			
			ConvertToScreen(&where); 
			popup->SetTargetForItems(BMessenger(appSig.String(), -1, NULL));
			popup->Go( where,true,false,true );
		}
	}
}	

//---------------------------------------------------------

void TrayIcon::MessageReceived(BMessage* msg)
{
	switch(msg->what) {
	
		case B_SET_PROPERTY:
			//inMessage->FindInt32("status",(int32 *) &my_status);
			//Invalidate(Bounds());
			break;

		default:
			BView::MessageReceived(msg);
			break;
	}
}	

//---------------------------------------------------------

TrayIcon* TrayIcon::Instantiate(BMessage* archive)
{
	if(!validate_instantiation(archive, "TrayIcon"))
		return NULL;
		
	return new TrayIcon(archive);
}

//---------------------------------------------------------

status_t TrayIcon::Archive( BMessage *archive, bool deep ) const
{
	// archive this bad boy
	BView::Archive(archive, deep);
	
	// save the icons
	BMessage	bits(B_ARCHIVED_OBJECT);
	if(ActiveIcon) {
		ActiveIcon->Archive(&bits);
		archive->AddMessage("ActiveIcon", &bits);
		bits.MakeEmpty();
	}
	
	if(InactiveIcon) {
		InactiveIcon->Archive(&bits);
		archive->AddMessage("InactiveIcon", &bits);
	}
	
	archive->AddString("add_on", BeAIMAppSig().String());
	return B_OK;
}

//---------------------------------------------------------

