#include "ColorPicker.h"
#include "ColorView.h"
#include "Say.h"

ColorView::ColorView( BRect frame, int id )
			   : BView( frame, "Color_view", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE ){

	SetViewColor( 216, 216, 216 );
	color.red = color.green = color.blue = 255;
	lowcolor.red = lowcolor.blue = 222;
	lowcolor.green = 219;
	enabled = true;
	cid = id;
}


ColorView::~ColorView() {
}


void ColorView::Draw( BRect updateRect ) {

	SetLowColor( lowcolor );
	SetHighColor(color);
	FillRect( Bounds(), enabled ? B_SOLID_HIGH : B_MIXED_COLORS );
	if( enabled )
		SetHighColor(0,0,0);
	else
		SetHighColor(169,166,169);
	StrokeRect(Bounds());
}

void ColorView::SetColor( rgb_color col ) {
	color = col;
	if( Window() )
		Invalidate();
}

void ColorView::SetColor( int32 r, int32 g, int32 b ) {
	color.red = r;
	color.green = g;
	color.blue = b;
	if( Window() )
		Invalidate();
}

rgb_color ColorView::GetColor() {
	return color;
}

void ColorView::SetEnabled( bool en ) {
	enabled = en;
}

void ColorView::MouseDown( BPoint point ) {
	
	if( enabled && Window() && cid != -1 ) {
		BMessage* msg = new BMessage(BEAIM_COLORVIEW_CLICKED);
		msg->AddData( "color", B_RGB_COLOR_TYPE, &color, sizeof(rgb_color) );
		msg->AddInt32( "cid", (int32)cid );
		Window()->PostMessage( msg );	
	}
	BView::MouseDown(point);
}


EnStringView::EnStringView( BRect frame, const char* name, const char* text )
			: BStringView( frame, name, text )
{
	enabled = true;	
}

void EnStringView::SetEnabled( bool en ) {
	enabled = en;
}


void EnStringView::Draw(BRect updateRect) {

	if( enabled )
		SetHighColor(0,0,0);
	else
		SetHighColor(132,130,132);

	BStringView::Draw(updateRect);
}

