#include "Globals.h"
#include "CheckListItem.h"
#include "MiscStuff.h"

BBitmap* CheckListItem::checkedBitmap = NULL;
BBitmap* CheckListItem::unCheckedBitmap = NULL;
BBitmap* CheckListItem::disCheckedBitmap = NULL;
BBitmap* CheckListItem::disUnCheckedBitmap = NULL;

//=====================================================

CheckListItem::CheckListItem( BString itm, BFont* font, bool chk, bool en )
		    : BListItem()
{
	ourFont = font; 
	item = itm;
	checked = chk;
	enabled = en;
	
	// get the bitmaps
	if( checkedBitmap == NULL )
		GetBitmapFromResources( checkedBitmap, 8150, AppFileName );
	if( unCheckedBitmap == NULL )
		GetBitmapFromResources( unCheckedBitmap, 8151, AppFileName );
	if( disCheckedBitmap == NULL )
		GetBitmapFromResources( disCheckedBitmap, 8152, AppFileName );
	if( disUnCheckedBitmap == NULL )
		GetBitmapFromResources( disUnCheckedBitmap, 8153, AppFileName );
}

//-----------------------------------------------------

void CheckListItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;
	BRect checkRect = BRect(0,0,11,11);

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

	// set the font and draw the string
	if( enabled )
		owner->SetHighColor(0,0,0);
	else
		owner->SetHighColor(105,106,105);
	owner->SetFont( &ownerFont );
	owner->MovePenTo(frame.left + 22, frame.bottom - 2);
	owner->DrawString( item.String() );
	
	// draw the checkmark
	checkRect.OffsetTo( frame.left + 4, frame.top + 2 );	
	if( checked )
		owner->DrawBitmap( enabled ? checkedBitmap : disCheckedBitmap, checkRect );
	else
		owner->DrawBitmap( enabled ? unCheckedBitmap : disUnCheckedBitmap, checkRect );
	
	// reset the font and colors
	owner->SetLowColor( 255, 255, 255 );
	owner->SetHighColor( 0, 0, 0 );
}

//-----------------------------------------------------

void CheckListItem::SetChecked( bool ck ) {
	if( enabled )
		checked = ck;
}

//-----------------------------------------------------

void CheckListItem::SetEnabled( bool en ) {
	enabled = en;
}

//=====================================================


