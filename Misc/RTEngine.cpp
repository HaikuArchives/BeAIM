#include <Window.h>
#include "RTEngine.h"


RTEngine::RTEngine( BRect frame )
		: BView(frame,"",B_FOLLOW_NONE,B_WILL_DRAW)
{
	font = be_plain_font;
	font.SetSize(25.0);
	font.SetShear(100);
	SetFont( &font );
	SetViewColor( 255, 235, 235 );
	
	leftMargin = 5;
	rightMargin = 5;
}

void RTEngine::Draw(BRect updateRect) {

	char theString[] = "AaBbCc";
	DrawString( theString, BPoint(leftMargin,50) );
	
	//BRect boundThinger;
	//escapement_delta delta;
	//font.GetBoundingBoxesForStrings( (const char**)&theString, 1, B_SCREEN_METRIC, &delta, &boundThinger );
	
	//float len = font.StringWidth("AaBbCc");
	//StrokeLine( BPoint(leftMargin,44), BPoint(leftMargin+len,44) );
}
