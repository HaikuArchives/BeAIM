#ifndef BEAIM_HTML_VIEW_H
#define BEAIM_HTML_VIEW_H

#include <TextView.h>
#include <ScrollBar.h>
#include <String.h>
#include "GenList.h"
#include "constants.h"

#define HTML_BOLD 1
#define HTML_ITALIC 2
#define HTML_UNDERLINE 3

#define NORMAL_FONT_SIZE 3

const uint32 TEXT_WAS_MODIFIED = 'tXm$';

// structure used to save some style info on the stack
struct fontInfo {
	rgb_color color;
	float size;
};

class HTMLView : public BTextView {

	public:
		HTMLView( bool, BRect, const char*, BRect, uint32 = B_FOLLOW_ALL, uint32 = B_WILL_DRAW | B_PULSE_NEEDED );
		virtual void InsertText(const char*, int32, int32, const text_run_array* );
		virtual void MouseMoved( BPoint where, uint32 code, const BMessage *msg );
		virtual void DeleteText(int32 start, int32 finish);
	
		void SetShowFontColorSizes( bool );
		void SetBaseFontAndColor( BFont& font, rgb_color& color, bool=true );
		void SetFontAttribute( int32 attrib, bool state );
		void SetFontColor( rgb_color color );
		void SetFontSize( int32 );
		void SetFontColor( int32, int32, int32 );
		void ClearFontStates();
		void ResetFontToBase();
		void AddStatement();	
		void InsertHTMLText( char* text );
		char* GetFormattedMessage();
		char* GetRawTextMessage();
		bool IsEmpty() { return (TextLength() == 0); };
		int HexcharToInt( char );
		void ParseHTMLStatement( char* );
		void HandleHTMLTag( char* );
		bool FirstWordMatch( char*, char* );
		void DoFontTagAttributes( char* );
		void DoLinkAttributes( char* );
		bool GetTagAttribute( char*, char*, char* );
		BFont GetBaseFont() { return baseFont; };
		
		void SetBeingDragged( bool );
	
	private:
	
		bool TypeMode;
		bool Empty;
		BFont baseFont;
		rgb_color baseColor;
		text_run insertstyle;
		GenList<fontInfo> fontStack;
		char URL[512];
		bool urlOn;
		BScrollBar* scroll;
		bool showFontColorSizes;
		
		// to hold styles until everything is inserted
		GenList<text_run> insertStyles;	
			
		int32 curOffset;
		char insertText[MESSAGE_MAX];
		bool styleChanged;
		char* message;
		bool beingDragged;
};

#endif
