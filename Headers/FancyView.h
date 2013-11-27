#ifndef BEAIM_FANCY_TEXT_VIEW_H
#define BEAIM_FANCY_TEXT_VIEW_H

#include <TextView.h>
#include <ScrollBar.h>
#include "TextElement.h"
#include "GenList.h"

#define NORMAL_FONT_SIZE 3
#define FANCY_MESSAGE_MAX 2048

const uint32 FANCY_OVER_LINK = 'LiNk';
const uint32 FANCY_NOT_OVER_LINK = 'NiNk';

//=========================================================

struct linkInfo {
	linkInfo() { link = 0; offset = len = 0; };
	~linkInfo() { delete[] link; link = 0; };
	linkInfo( int32 off, int32 len, char* nlink )
	{ link = 0; Set( off, len, nlink ); };
	linkInfo( linkInfo& g )
	{ link = 0; Set(g.offset, g.len, g.link); };
	linkInfo* operator=( linkInfo& g )
	{ Set(g.offset, g.len, g.link); return this; };
	void Set( int32, int32, char* link );
	int32 offset, len;
	char* link;
};

//=========================================================

struct textRunStyle {
	text_run run;
	bool variable;
	bool link;
};

//=========================================================

class FancyTextView : public BTextView {

	public:
		FancyTextView( BRect, const char*, BRect, uint32 = B_FOLLOW_ALL, uint32 = B_WILL_DRAW | B_PULSE_NEEDED );
		virtual void InsertText(const char*, int32, int32, const text_run_array* ); 

		void AddStyledText( char*, styleList&, bool=true );

		void SetShowFontColorSizes( bool );
		void SetShowLinks( bool );
		void SetAutoScrollEnabled( bool );
		
		void SetBaseFontAndColor( BFont& font, rgb_color& color, bool=true );
		void SetFontAttribute( int32 attrib, bool state );
		void SetFontColor( rgb_color color );
		void SetFontSize( float );
		void SetFontColor( int32, int32, int32 );
		void ClearInsertStuff();
		void ResetFontToBase();
		void AddStatement();
		void InsertSomeText( char* text );
		bool IsEmpty() { return (TextLength() == 0); };
		BFont GetBaseFont() { return baseFont; };
		void SetTextMagnification( float tm );
		float TextMagnification() { return textMag; };
		void SetBeingDragged( bool );
		
		void Clear();
		
		virtual void MouseMoved( BPoint where, uint32 code, const BMessage *msg );
		virtual void MouseDown( BPoint where );

	private:
	
		void MakeRGBColor( gColor&, rgb_color& );
		void RebuildStyles();
		void RebuildLinks();
		uint32 StrLen( char* );
		void Open( char* link );

		bool Empty;
		BFont baseFont;
		rgb_color baseColor;
		text_run insertstyle;
		BScrollBar* scroll;
		bool showFontColorSizes;
		bool showLinks;
		bool autoScrollEnabled;
		bool beingDragged;
		float textMag;
		
		// to hold styles until everything is inserted
		GenList<text_run> insertStyles;
		
		// holds the text_run stuff
		GenList<textRunStyle> masterStyles;

		// hold links
		GenList<linkInfo> links;

		int32 curOffset;
		char insertText[FANCY_MESSAGE_MAX];
		bool styleChanged;
		bool overLink;
		char* message;
};

//=========================================================

#endif
