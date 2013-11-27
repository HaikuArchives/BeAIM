#include <stdio.h>
#include <string.h>
#include "TextElement.h"

//=====================================================

gLink::gLink() {
	address = 0;
	used = false;
}

//-----------------------------------------------------

gLink::gLink( const char* addr ) {

	address = 0;
	used = false;
	if( addr )
		SetLink( addr );
	else
		used = false;
}

//-----------------------------------------------------

void gLink::Clear() {

	fflush(stdout);
	delete[] address;
	address = 0;
	used = false;
}

//-----------------------------------------------------

void gLink::SetLink( const char* addr ) {

	delete[] address;
	if( addr != 0 ) {
		address = new char[ strlen(addr)+1 ];
		strcpy( address, addr );
		used = true;
	} else
		Clear();
}

//-----------------------------------------------------

gLink::gLink( gLink& g ) {

	address = 0;
	used = g.used;
	if( used )
		SetLink( g.address );
}

//-----------------------------------------------------

gLink* gLink::operator=( gLink& g ) {

	used = g.used;
	if( used )
		SetLink( g.address );
	else
		Clear();
	
	return this;
}

//-----------------------------------------------------

bool gLink::operator==( gLink& g ) {

	if( used != g.used )
		return false;
	if( !used )
		return true;
		
	return bool( strcasecmp(address,g.address) == 0 );
}

//=====================================================

gTextElement::gTextElement() {
	Clear();
}

//-----------------------------------------------------

void gTextElement::Clear() {
	mask = TE_BLANK;
	offset = 0;

	// defaults
	fontSize = -1;
	fontFace = 0;
	fontStyle = ST_BLANK;
}

//-----------------------------------------------------

gTextElement::~gTextElement() {
	
	delete[] fontFace;
}

//-----------------------------------------------------

gTextElement* gTextElement::operator=( gTextElement& g ) {

	SetFontSize( g.FontSize() );
	SetFontStyle( g.FontStyle() );
	SetFontColor( g.FontColor() );
	SetFontFace( g.FontFace() );
	SetFontFace( g.FontFace() );
	SetLink( g.Link() );
	mask = g.mask;
	offset = g.offset;
	return this;
}

//-----------------------------------------------------

void gTextElement::MakeFontBackup( gTextElement& g ) {

	// clear out the structure
	ClearMask();
	link.SetUsed( false );
	
	// back up the font values
	SetFontSize( g.fontSize );
	SetFontColor( g.fontColor );
	SetFontFace( g.fontFace );
	SetLink( g.link );
	mask = g.Mask();
}

//-----------------------------------------------------

void gTextElement::RestoreFontBackup( gTextElement& g ) {

	// reset the font
	ResetFont();
	
	if( g.fontSize != fontSize )
		printf( "restoring font size\n" );
	if( !(g.fontColor == fontColor) )
		printf( "restoring font color\n" );
	if( strcasecmp(fontFace, g.fontFace) != 0 )
		printf( "restoring font face\n" );
	
	// conditionally set values
	if( g.fontSize != fontSize || g.mask & TE_FONTSIZE )
		SetFontSize( g.fontSize );
	if( !(g.fontColor == fontColor) || g.mask & TE_FONTCOLOR )
		SetFontColor( g.fontColor );
	if( strcasecmp(fontFace, g.fontFace) != 0 || g.mask & TE_FONTFACE )
		SetFontFace( g.fontFace );
	if( !(g.link == link) || g.mask & TE_LINK )
		SetLink( g.link );
}

//-----------------------------------------------------

void gTextElement::ResetFont() {

	// clear the font stuff from the bitmask
	mask &= ~TE_FONTSIZE;
	mask &= ~TE_FONTCOLOR;
	mask &= ~TE_FONTBGCOLOR;
	mask &= ~TE_FONTFACE;
}

//-----------------------------------------------------

bool gTextElement::HasUniqueFont() {

	return bool(mask & TE_FONTSIZE) ||
		   bool(mask & TE_FONTCOLOR) ||
		   bool(mask & TE_FONTBGCOLOR) ||
		   bool(mask & TE_FONTFACE);
}

//-----------------------------------------------------

void gTextElement::SetFontSize( float size ) {

	fontSize = size;
	if( fontSize != -1 )
		mask |= TE_FONTSIZE;
	else
		mask &= ~TE_FONTCOLOR;
}

//-----------------------------------------------------

void gTextElement::SetFontColor( gColor color ) {

	fontColor = color;
	if( fontColor.Used() )
		mask |= TE_FONTCOLOR;
	else
		mask &= ~TE_FONTCOLOR;
}

//-----------------------------------------------------

void gTextElement::CombineFontStyle( unsigned int style, bool on ) {

	if( on )
		fontStyle |= style;
	else
		fontStyle &= ~style;

	if( fontStyle != TE_BLANK )
		mask |= TE_FONTSTYLE;
	else
		mask &= ~TE_FONTSTYLE;
}

//-----------------------------------------------------

void gTextElement::SetFontStyle( unsigned int style ) {

	fontStyle = style;
	mask |= TE_FONTSTYLE;
}

//-----------------------------------------------------

void gTextElement::SetLink( gLink lnk ) {

	link = lnk;
	mask |= TE_LINK;
}

//-----------------------------------------------------

void gTextElement::ClearLink() {

	link.Clear();
	mask &= ~TE_LINK;
}

//-----------------------------------------------------

void gTextElement::SetFontFace( const char* face ) {

	delete[] fontFace;
	if( face != 0 ) {
		fontFace = new char[ strlen(face)+1 ];
		strcpy( fontFace, face );
	} else
		fontFace = 0;
	mask |= TE_FONTFACE;
}

//=====================================================

styleList::styleList() {
	theStyles = NULL;
}

//-----------------------------------------------------

void styleList::Clear() {
	delete theStyles;
	theStyles = NULL;
}

//=====================================================
