#ifndef _CHECK_LIST_ITEM_H_
#define _CHECK_LIST_ITEM_H_

#include <ListView.h>
#include <Bitmap.h>
#include <String.h>

//-----------------------------------------------------

class CheckListItem : public BListItem
{ 
	public: 
		CheckListItem( BString item, BFont* font, bool checked, bool enabled=true );
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		void SetChecked( bool );
		void SetEnabled( bool );

	private: 
	
		static BBitmap* checkedBitmap;
		static BBitmap* unCheckedBitmap;
		static BBitmap* disCheckedBitmap;
		static BBitmap* disUnCheckedBitmap;
	
		BFont* ourFont;
		BString item;
		bool checked;
		bool enabled;
};

//-----------------------------------------------------

#endif
