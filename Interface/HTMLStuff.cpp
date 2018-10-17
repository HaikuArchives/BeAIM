#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "HTMLStuff.h"
#include "Say.h"

//-----------------------------------------------------

HTMLParser::HTMLParser() {

	lastoff = 0;
	parsed = 0;
}

//-----------------------------------------------------

HTMLParser::~HTMLParser() {

	delete parsed;
}

//-----------------------------------------------------

void HTMLParser::Parse( char* text, bool full ) {

	// vars
	char tag[1024];
	char *p, *last, *out = NULL;
	int catpos;
	fontStack.Clear();
	insertstyle.Clear();
	lastoff = 0;
	backgroundColor.Set(255,255,255);
	char specChar[10];

	// allocate some space for the final parsed string
	parsed = new char[ strlen(text)+1 ];
	parsed[0] = '\0';
	unsigned offset = 0;

	// if there is no input, return
	if( text == NULL )
		return;

	// iterator starts at the beginning
	p = text;
	last = p;

	// loop through all the characters
	while( *p ) {

		// beginning of a special char?
		if( *p == '&' ) {
			int sCharCount = 0;

			// If there was any text before this, print it
			if( (int)(p - last) ) {
				delete out;
				out = new char[(p - last)+1];
				strncpy( out, last, (int)(p - last) );
				out[ (int)(p - last) ] = '\0';
				HandleFinalTextChunk( out );
			} last = p;

			// get the rest of the special char, as far as we can go
			while( *p && (isalpha(*p) || *p == '&') && sCharCount < 9 ) {
				specChar[sCharCount++] = *p;
				++p;
			} specChar[sCharCount] = '\0';

			// we're only (possibly) good if the last char is a semicolon
			if( *p == ';' ) {

				// if the char was handled correctly, reset last to reflect that
				if( HandleSpecialChar( specChar, offset ) )
					last += (sCharCount+1);
			}
		}

		// Beginning of an HTML tag?
		if( *p == '<' )
		{
			// If there was any text before this, print it
			if( (int)(p - last) ) {
				delete out;
				out = new char[(p - last)+1];
				strncpy( out, last, (int)(p - last) );
				out[ (int)(p - last) ] = '\0';
				HandleFinalTextChunk( out );
			}

			// Re-assign last, in case this isn't a tag after all
			if( offset && last != text )
				--offset;
			last = p;

			// get the tag name
			++p;
			catpos = 0;

			// ignore the rest of the tag
			while( *p && (*p != '>') ) {

				if( *p == '<' )
					break;

				tag[catpos++] = *p;
				++p;
			}

			// did we hit the end without finding a matching '>'?
			// if so, it was a less-than (<) sign, not a tag opener
			if( !(*p) || (*p == '<') ) {
				delete out;
				out = new char[(p - last)+1];
				strncpy( out, last, (int)(p - last) );
				out[ (int)(p - last) ] = '\0';
				offset += (int)(p - last);
				HandleFinalTextChunk( out );
				if( *p == '<' )
					--p;
				else
					return;
			}

			// otherwise, it was a tag ending and we should process it
			else {
				tag[catpos++] = '\0';
				HandleHTMLTag( tag, offset, full );
				tag[0] = '\0';
			}

			// reset last
			last = p + 1;
		}

		// next character
		if( *p ) {
			++p;
			++offset;
		}
	}

	// Was the last part a statement, perhaps? If so, add it
	if( last != p ) {
		delete out;
		out = new char[(p - last)+1];
		out[0] = '\0';
		strncpy( out, last, (int)(p - last) );
		out[(int)(p - last)] = '\0';
		HandleFinalTextChunk( out );
	}

	// Take care of details
	PreCommit( offset );
	fontStack.Clear();
}

//-----------------------------------------------------

bool HTMLParser::HandleSpecialChar( char* tag, unsigned& offset ) {

	bool handled = false;

	// is it an ampersand?
	if( strcasecmp(tag, "&amp") == 0 ) {
		HandleFinalTextChunk( "&" );
		handled = true;
	}

	// is it an less-than sign?
	else if( strcasecmp(tag, "&lt") == 0 ) {
		HandleFinalTextChunk( "<" );
		handled = true;
	}

	// is it a greater-than sign?
	else if( strcasecmp(tag, "&gt") == 0 ) {
		HandleFinalTextChunk( ">" );
		handled = true;
	}

	// is it a space?
	else if( strcasecmp(tag, "&nbsp") == 0 ) {
		HandleFinalTextChunk( " " );
		handled = true;
	}

	else if (strcasecmp(tag, "&quot") == 0) {
		HandleFinalTextChunk("\"");
		handled = true;
	}

	if( handled )
		++offset;

	return handled;
}

//-----------------------------------------------------

void HTMLParser::HandleHTMLTag( char* tag, unsigned& offset, bool full ) {

	char* p;
	bool off = false;
	bool handled = false;

	// Trim all the whitespace off the left side of the tag
	p = tag;
	while( *p == ' ' )
		++p;

	// if there was nothing but spaces in the tag, print it as it was
	if( !(*p) ) {
		HandleFinalTextChunk( "<" );
		HandleFinalTextChunk( tag );
		HandleFinalTextChunk( ">" );
		offset += strlen(tag) + 2;
		return;
	}

	// is this an end tag?
	if( *p == '/' ) {
		off = true;

		// make sure there is something after the slash
		if( !(*(++p)) )
			return;
	}

	// starts with 'b' (body, bold, or line break)
	if( (*p == 'B') || (*p == 'b') ) {

		// if it is just a b (for bold)
		if( full && (!(*(p+1)) || *(p+1) == ' ') ) {
			SetFontAttribute( ST_BOLD, !off, offset );
			handled = true;
		}

		// if it is a body tag
		else if( FirstWordMatch( p, "body" ) ) {
			if( !off )
				DoBodyTagAttributes( p, offset );
			handled = true;
		}

		// if it is a line break
		else if( full && FirstWordMatch( p, "br" ) ) {
			HandleFinalTextChunk( "\n" );
			++offset;
			handled = true;
		}
	}

	// starts with 'i' (probably italic)
	else if( full && ((*p == 'I') || (*p == 'i')) ) {
		if( !(*(p+1)) || *(p+1) == ' ' ) {
			SetFontAttribute( ST_ITALIC, !off, offset );
			handled = true;
		}
	}

	// starts with 'f' (probably <FONT> )
	else if( full && ((*p == 'F') || (*p == 'f')) ) {
		if( FirstWordMatch( p, "font" ) ) {
			if( !off )
				DoFontTagAttributes( p, offset );
			else {
				if( !fontStack.IsEmpty() ) {
					PreCommit( offset );
					gTextElement savedInfo;
					fontStack.Pop( savedInfo );
					insertstyle.RestoreFontBackup( savedInfo );
				} else
					ResetFontToBase( offset );
			}
			handled = true;
		}
	}

	// starts with 'a' (probably <a href="..."> )
	else if( full && ((*p == 'A') || (*p == 'a')) ) {
		if( FirstWordMatch( p, "a" ) ) {
			if( !off )
				DoLinkAttributes( p, offset );
			else {
				PreCommit( offset );
				insertstyle.ClearLink();
			}
			handled = true;
		}
	}

	// starts with 'h' (probably <HTML> or <hr>)
	else if( (*p == 'H') || (*p == 'h') ) {

		if( FirstWordMatch( p, "html" ) )
			handled = true;

		// if it is a horizontal rule (<hr>... treat it like a line break. Bad but good enough for now.
		else if( full && FirstWordMatch( p, "hr" ) ) {
			HandleFinalTextChunk( "\n" );
			++offset;
			handled = true;
		}
	}

	// starts with 'u' (underline)
	else if( full && ((*p == 'U') || (*p == 'u')) ) {
		if( !(*(p+1)) || *(p+1) == ' ' )
			SetFontAttribute( ST_UNDERLINE, !off, offset );
			handled = true;
	}

	// starts with 's' (<sub> or <sup> )
	else if( full && ((*p == 'S') || (*p == 's')) ) {
		if( FirstWordMatch(p,"sub") || FirstWordMatch(p,"sup") )
			handled = true;
	}

	// starts with 'p' (<pre> tag)
	else if( full && ((*p == 'P') || (*p == 'p')) ) {
		if( FirstWordMatch( p, "pre" ) )
			handled = true;
	}

	// if it wasn't handled, it was probably some silly user
	//   saying stuff like <this>, so print it out raw
	if( !handled ) {
		HandleFinalTextChunk( "<" );
		HandleFinalTextChunk( tag );
		HandleFinalTextChunk( ">" );
		offset += strlen(tag) + 2;
	}
}

//-----------------------------------------------------

void HTMLParser::DoBodyTagAttributes( char* tag, unsigned offset ) {

	char attrib[1024];		// general size assumptions for attribs and values
	char value[1024];

	// Loop through, getting all the attibutes, and process them
	bool more = GetTagAttribute( tag, attrib, value );
	while( more ) {

		// starts with 'b' (probably bgcolor)
		if( (*attrib == 'B') || (*attrib == 'b') ) {
			if( FirstWordMatch( attrib, "bgcolor" ) ) {

				// must be in the form #xxxxxx... no color constants supported yet
				if( strlen(value) != 7 || value[0] != '#' ) {
					more = GetTagAttribute( NULL, attrib, value );
					continue;
				}
				backgroundColor.SetR( 16*HexcharToInt(value[1]) + HexcharToInt(value[2]) );
				backgroundColor.SetG( 16*HexcharToInt(value[3]) + HexcharToInt(value[4]) );
				backgroundColor.SetB( 16*HexcharToInt(value[5]) + HexcharToInt(value[6]) );
			}
		}

		more = GetTagAttribute( NULL, attrib, value );
	}
}

//-----------------------------------------------------

void HTMLParser::DoFontTagAttributes( char* tag, unsigned offset ) {

	char attrib[1024];		// general size assumptions for attribs and values
	char value[1024];
	bool Changed = false;
	gTextElement savedInfo;

	// first save the relevant font info in case we need to push it on the stack
	savedInfo.MakeFontBackup( insertstyle );

	// Loop through, getting all the attibutes, and process them
	bool more = GetTagAttribute( tag, attrib, value );
	while( more ) {

		// starts with 's' (probably size)
		if( (*attrib == 'S') || (*attrib == 's') ) {
			if( FirstWordMatch( attrib, "size" ) ) {
				Changed = true;
				int fontSize = atoi( value );
				if( fontSize == 1 )
					SetFontSize( 10, offset );
				else if( fontSize == 2 )
					SetFontSize( 11, offset );
				else if( fontSize == 3 )
					SetFontSize( 13, offset );
				else if( fontSize == 4 )
					SetFontSize( 15, offset );
				else if( fontSize == 5 )
					SetFontSize( 19, offset );
				else if( fontSize == 6 )
					SetFontSize( 25, offset );
				else if( (fontSize >= 7) && (fontSize < 42) )
					SetFontSize( 38, offset );
			}
		}

		// starts with 'c' (probably color)
		if( (*attrib == 'C') || (*attrib == 'c') ) {
			if( FirstWordMatch( attrib, "color" ) ) {
				Changed = true;

				// must be in the form #xxxxxx... no color constants supported yet
				if( strlen(value) != 7 || value[0] != '#' ) {
					more = GetTagAttribute( NULL, attrib, value );
					continue;
				}
				gColor font_Color;
				font_Color.SetR( 16*HexcharToInt(value[1]) + HexcharToInt(value[2]) );
				font_Color.SetG( 16*HexcharToInt(value[3]) + HexcharToInt(value[4]) );
				font_Color.SetB( 16*HexcharToInt(value[5]) + HexcharToInt(value[6]) );
				SetFontColor( font_Color, offset );
			}
		}

		more = GetTagAttribute( NULL, attrib, value );
	}

	// if the styles have changed at all, push the old style onto the stack
	if( Changed && savedInfo.HasUniqueFont() )
		fontStack.Push( savedInfo );
}

//-----------------------------------------------------

void HTMLParser::DoLinkAttributes( char* tag, unsigned offset ) {

	char attrib[100];		// general size assumptions for attribs and values
	char value[1024];		// the URL might get a bit lengthy though...

	// Loop through, getting all the attibutes, and process them
	bool more = GetTagAttribute( tag, attrib, value );
	while( more ) {

		// starts with 'h' (probably href)
		if( (*attrib == 'H') || (*attrib == 'h') ) {
			if( FirstWordMatch( attrib, "href" ) ) {
				PreCommit( offset );
				gLink valueLink(value);
				insertstyle.SetLink( valueLink );
			}
		}

		more = GetTagAttribute( NULL, attrib, value );
	}
}

//-----------------------------------------------------

bool HTMLParser::GetTagAttribute( char* tag, char* attrib, char* value ) {

	static char* p = NULL;
	int count;

	// if we were passed a valid tag, it means we are starting over.
	if( tag ) {
		p = tag;
		while( *p && isalnum(*p) )			// seek past the tag's name
			++p;
	}

	// eat whitespace
	while( *p && *p == ' ' )
		++p;

	// this should be the beginning of an attribute (like size=4)
	if( !(*p) || !isalpha(*p) )
		return false;

	// read the attribute name
	attrib[count = 0] = *(p++);		// count = 0, first char into attrib
	while( *p && isalnum(*p) ) {
		attrib[++count] = *p;
		++p;
	}
	attrib[++count] = '\0';
	if( !(*p) )
		return false;

	// now read to the equal sign (if there is one)
	while( *p && (*p != '=') && !isalnum(*p) )
		++p;
	if( !(*p) )
		return false;

	// no equal sign - this attribute doesn't have a value (like <TD nowrap>)
	if( isalnum(*p) )
		value[0] = '\0';

	// this attribute DOES have a value
	else {

		// eat whitespace until the start of the value tag
		++p;
		while( *p == ' ' ) ++p;
		if( !(*p) )
			return false;

		// a non-quoted value (with no spaces)
		if( isprint(*p) && !isspace(*p) && *p != '"' ) {
			value[count = 0] = *(p++);		// count = 0, first char into attrib
			while( *p && isprint(*p) && !isspace(*p) ) {
				value[++count] = *p;
				++p;
			}
			value[++count] = '\0';
		}

		// quoted value
		else if( *p == '"' ) {
			if( !(*(++p)) )					// get past the opening quote
				return false;
			value[count = 0] = *(p++);		// count = 0, first char into attrib

			while( *p && *p != '"' ) {		// seek to the ending quote
				value[++count] = *p;
				++p;
			}
			value[++count] = '\0';
// At least, I hope this is the right place :)
//
// NathanW: I would also note that quotation marks are still screwing up formatting
// NathanW: The provided offset in the HTML decode stuff is off by one when it advances the read pointer
//
//			if( *p )						// get past the closing quote
//				++p;
		}
	}

	// true indicates that there are more attribs waiting
	return true;
}

//-----------------------------------------------------

bool HTMLParser::FirstWordMatch( char* sentence, char* word ) {
	while( *(sentence) && isalpha(*sentence) && *(word) ) {
		if( tolower(*sentence) != tolower(*word) )
			return false;
		sentence++;
		word++;
	}
	if( (!(*sentence) || !isalpha(*sentence)) && !(*word) )
		return true;
	return false;
}

//-----------------------------------------------------

unsigned char HTMLParser::HexcharToInt( char hex )
{
	// is upper-case A-F
	if ( (hex >= 0x41) && (hex <= 0x46) )
		return (unsigned char)((hex - 0x41) + 0xa);

	// is lower-case a-f
	else if ( (hex >= 0x61) && (hex <= 0x66) )
		return (unsigned char)((hex - 0x61) + 0xa);

	// is numeric digit
	else if ( (hex >= 0x30) && (hex <= 0x39) )
		return (unsigned char)(hex - 0x30);
	else
		return (unsigned char)(0);
}

//-----------------------------------------------------

// Note: do NOT do anything with offsets here... they are not guaranteed
// to be valid yet!
void HTMLParser::HandleFinalTextChunk( char* chunk ) {

	strcat( parsed, chunk );
}

//-----------------------------------------------------

void HTMLParser::ResetFontToBase( unsigned offset ) {

	PreCommit( offset );
	insertstyle.ResetFont();
}

//-----------------------------------------------------

void HTMLParser::SetFontAttribute( unsigned attrib, bool state, unsigned offset ) {

	PreCommit( offset );
	insertstyle.CombineFontStyle( attrib, state );
}

//-----------------------------------------------------

void HTMLParser::SetFontColor( gColor color, unsigned offset ) {

	PreCommit( offset );
	insertstyle.SetFontColor(color);
}

//-----------------------------------------------------

void HTMLParser::SetFontBGColor( gColor color, unsigned offset ) {

	PreCommit( offset );
	insertstyle.SetFontColor(color);
}

//-----------------------------------------------------

void HTMLParser::SetFontSize( float size, unsigned offset ) {

	PreCommit( offset );
	insertstyle.SetFontSize(size);
}

//-----------------------------------------------------

void HTMLParser::PreCommit( unsigned offset ) {

	// if the offsets aren't equal, add the old style
	if( offset != lastoff ) {
		insertstyle.SetOffset( lastoff );
		styles.Add( insertstyle );
		lastoff = offset;
	}
}

//-----------------------------------------------------

// Note: caller is responsible for deleting the the returned styles list!
styleList HTMLParser::Styles() {

	// make a new styles list
	styleList s;
	s.bgColor = backgroundColor;
	s.theStyles = new GenList<gTextElement>;

	// fill 'er up and return it
	for( unsigned int i = 0; i < styles.Count(); ++i )
		s.theStyles->Add( styles[i] );
	return s;
}

//-----------------------------------------------------
