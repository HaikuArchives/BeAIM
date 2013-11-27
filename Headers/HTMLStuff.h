#ifndef _HTML_STUFF_H
#define _HTML_STUFF_H

#include "GenList.h"
#include "TextElement.h"

class HTMLParser {

	public:
		HTMLParser();
		~HTMLParser();

		void Parse( char* text, bool full = true );
		char* ParsedString() { return parsed; };
		styleList Styles();

	private:

		// parser functions
		void HandleHTMLTag( char* tag, unsigned& offset, bool full=true );
		bool HandleSpecialChar( char* tag, unsigned& offset );
		void DoBodyTagAttributes( char* tag, unsigned offset );
		void DoFontTagAttributes( char* tag, unsigned offset );
		void DoLinkAttributes( char* tag, unsigned offset );
		bool GetTagAttribute( char* tag, char* attrib, char* value );
		bool FirstWordMatch( char* sentence, char* word );
		void HandleFinalTextChunk( char* chunk );
		void ResetFontToBase( unsigned offset );
		unsigned char HexcharToInt( char );
		void SetFontAttribute( unsigned attrib, bool state, unsigned offset );
		void SetFontColor( gColor color, unsigned offset );
		void SetFontBGColor( gColor color, unsigned offset );
		void SetFontSize( float size, unsigned offset );
		void PreCommit( unsigned offset );

		// data
		GenList<gTextElement> styles;
		GenList<gTextElement> fontStack;
		gTextElement insertstyle;
		unsigned lastoff;
		gColor backgroundColor;

		// the result of all this
		char* parsed;
};

#endif
