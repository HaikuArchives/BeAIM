#ifndef _TEXT_ELEMENT_H_
#define _TEXT_ELEMENT_H_

#include "GenList.h"

//===========================================

const unsigned int TE_BLANK = 0;
const unsigned int TE_FONTSTYLE = 1;
const unsigned int TE_FONTSIZE = 2;
const unsigned int TE_FONTCOLOR = 4;
const unsigned int TE_FONTBGCOLOR = 8;
const unsigned int TE_FONTFACE = 16;
const unsigned int TE_LINK = 32;

const unsigned int ST_BLANK = 0;
const unsigned int ST_BOLD = 1;
const unsigned int ST_ITALIC = 2;
const unsigned int ST_UNDERLINE = 4;
const unsigned int ST_STRIKEOUT = 8;
const unsigned int ST_SUBSCRIPT = 16;
const unsigned int ST_SUPERSCRIPT = 32;

//===========================================

class gColor {

	public:

		gColor() { dr = dg = db = 255; used = false; };
		gColor( unsigned char R, unsigned char G, unsigned char B ) {
			Set( R,G,B );
			used = true;
		};
		gColor( const gColor& c ) {
			dr = c.dr; dg = c.dg; db = c.db;
			used = c.used;
		}
		gColor* operator=( const gColor& c ) {
			dr = c.dr; dg = c.dg; db = c.db;
			used = c.used;
			return this;
		}
		bool operator==( const gColor& c ) {
			if( used != c.used ) return false;
			if( !used ) return true;
			return( (c.dr == dr) && (c.dg == dg) && (c.db == db) );
		}

		unsigned char R() { return dr; };
		unsigned char G() { return dg; };
		unsigned char B() { return db; };
		bool Used() { return used; };

		void SetR( unsigned char R ) { dr = R; used = true; };
		void SetG( unsigned char G ) { dg = G; used = true; };
		void SetB( unsigned char B ) { db = B; used = true; };
		void SetUsed( bool u ) { used = u; };

		void Set( unsigned char R, unsigned char G, unsigned char B ) {
			dr = R; dg = G; db = B; used = true;
		};

	private:
		unsigned char dr;
		unsigned char dg;
		unsigned char db;
		bool used;
};

//===========================================

class gLink {

	public:

		gLink();
		gLink( const char* addr );
		gLink( gLink& g );
		~gLink() { Clear(); };

		gLink* operator=( gLink& g );
		bool operator==( gLink& g );

		void SetLink( const char* addr );
		const char* Link() { return address; };
		bool Used() { return used; };
		void SetUsed( bool u ) { used = u; };
		void Clear();

	private:
		char* address;
		bool used;
};

//===========================================

class gTextElement {

	public:
		
		gTextElement();
		~gTextElement();
		gTextElement* operator=( gTextElement& g );
		void Clear();

		void SetFontSize( float size );
		float FontSize() { return fontSize; };
		void SetFontColor( gColor color );
		gColor FontBGColor() { return fontBGColor; };
		void SetBGFontColor( gColor color );
		gColor FontColor() { return fontColor; };		
		void SetFontStyle( unsigned int style );
		void CombineFontStyle( unsigned int style, bool on );
		unsigned int FontStyle() { return fontStyle; };
		void SetLink( gLink lnk );
		void ClearLink();
		gLink Link() { return link; };
		void SetFontFace( const char* face );
		const char* FontFace() { return fontFace; };

		unsigned int Mask() { return mask; };
		void ClearMask() { mask = 0; };		
		unsigned int Offset() { return offset; };
		void SetOffset( unsigned off ) { offset = off; };

		bool HasUniqueFont();
		void MakeFontBackup( gTextElement& g );
		void RestoreFontBackup( gTextElement& g );
		void ResetFont();

	private:

		unsigned int mask;
		unsigned int offset;
				
		float fontSize;		
		gColor fontColor;
		gColor fontBGColor;
		char* fontFace;
		unsigned int fontStyle;
		gLink link;
		bool urlOn;
};

//===========================================

//typedef GenList<gTextElement>* styleList;

struct styleList {
	styleList();
	void Clear();
	GenList<gTextElement>* theStyles;
	gColor bgColor;
};

//===========================================

#endif
