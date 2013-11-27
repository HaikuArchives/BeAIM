#ifndef COLOR_VIEW_H
#define COLOR_VIEW_H

#include <View.h>
#include <StringView.h>

const uint32 BEAIM_COLORVIEW_CLICKED = 'c&CK';

// A BView derived class that holds and displays a color.
class ColorView : public BView
{
	public:
		ColorView(BRect frame, int);
		~ColorView();
		void Draw(BRect updateRect);
		void MouseDown(BPoint point);
		void SetColor( rgb_color );
		void SetColor( int32, int32, int32 );
		rgb_color GetColor();
		void SetEnabled( bool );
		
	private:
		rgb_color color;
		rgb_color lowcolor;
		bool enabled;
		int cid;
};


class EnStringView : public BStringView
{
	public:
		EnStringView( BRect frame, const char* name, const char* text );
		void Draw(BRect updateRect);
		void SetEnabled( bool );
		
	private:
		bool enabled;
};

#endif
