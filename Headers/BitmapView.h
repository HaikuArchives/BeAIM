#ifndef BITMAP_VIEW_H
#define BITMAP_VIEW_H

#include <View.h>
#include <String.h>

// A BView derived class that holds  and displays a BBitmap.
class BitmapView : public BView
{
	public:
		BitmapView(BRect frame, BBitmap *bmap);
		~BitmapView();
		void Draw(BRect updateRect);
		void MouseDown(BPoint point);
		void SetURLMode( BString url, bool web );
		
	private:
		int urlMode;
		BString url;
		BBitmap	*themap;
		BRect bmbounds;
};

#endif
