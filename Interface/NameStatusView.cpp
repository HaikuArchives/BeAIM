#include <stdio.h>
#include "DLanguageClass.h"
#include "NameStatusView.h"
#include "Say.h"

//=====================================================

NameStatusView::NameStatusView( BRect rect )
	   	  	  : BView(rect, "BuddyListStatus", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW )
{
	wLevel = 0;
	SetViewColor( 230, 230, 230 );
	SetAway(false);
	
	rFont = be_plain_font;
	rFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	rFont.SetSize(11.0);
	bFont = be_bold_font;
	bFont.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	bFont.SetFace(B_BOLD_FACE);
	bFont.SetSize(11.0);
	SetFont( &rFont );
	SetFontSize(11);
}

//-----------------------------------------------------

NameStatusView::~NameStatusView() {
}

//-----------------------------------------------------

void NameStatusView::SetTempString( BString ts ) {
	if( tempString == ts )
		return;
	tempString = ts;
	if( Window() )
		Invalidate();
}

//-----------------------------------------------------

void NameStatusView::Draw( BRect ) {

	BRect frame = Bounds();

	if( !away ) {
		SetHighColor( 156, 154, 156 );		// dark grey
		BPoint start = frame.LeftBottom();
		BPoint end = frame.RightBottom();
		start.y -= 13;
		end.y -= 13;
		StrokeLine( start, end );
		
		SetHighColor( 0, 0, 0 );
		SetLowColor( 222, 219, 222 );
		
		MovePenTo(frame.left + 4, frame.bottom-2);	
		
		if( tempString.Length() )
			DrawString( tempString.String() );
		else
			DrawString( displayString.String() );
	} 
	
	else {
		SetHighColor( 0, 0, 0 );		// dark grey
		BPoint start = frame.LeftBottom();
		BPoint end = frame.RightBottom();
		start.y -= 13;
		end.y -= 13;
		StrokeLine( start, end );
		
		SetHighColor( 255, 255, 255 );
		SetLowColor( 0, 0, 165 );
		
		MovePenTo(frame.left + 4, frame.bottom-2);
		
		if( tempString.Length() ) {
			SetFont( &rFont );
			DrawString( tempString.String() );
		} else {
			SetFont( &bFont );
			DrawString( Language.get("ERR_IN_AWAY_MODE") );
		}
	}
}

//-----------------------------------------------------

void NameStatusView::SetAway( bool aw ) {

	if( away == aw )
		return;

	away = aw;
	if( away )
		SetViewColor( 0, 0, 165 );
	else
		SetViewColor( 230, 230, 230 );
	
	if( Window() )
		Invalidate();
}

//-----------------------------------------------------

void NameStatusView::SetName( AIMUser name ) {

	if( name == myName )
		return;

	myName = name;
	displayString = myName.Username();
	if( wLevel ) {
		char nums[10];
		sprintf( nums, "%u", wLevel );
		displayString += BString("  (");
		displayString += BString(nums);
		displayString += BString("%)");
	}
	
	if( Window() )
		Invalidate();
}

//-----------------------------------------------------

void NameStatusView::SetWarningLevel( unsigned short wl ) {

	if( wLevel == wl )
		return;

	wLevel = wl;
	displayString = myName.Username();
	if( wLevel ) {
		char nums[10];
		sprintf( nums, "%u", wLevel );
		displayString += BString("  (");
		displayString += BString(nums);
		displayString += BString("%)");
	}
	
	if( Window() )
		Invalidate();
}

//-----------------------------------------------------
