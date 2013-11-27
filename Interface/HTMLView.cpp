#include "HTMLView.h"
#include <Application.h>
#include <Alert.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "constants.h"
#include "Say.h"
#include "GenList.h"

//----------------------------------------------------------------------------------------

HTMLView::HTMLView( bool mode, BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags )
		: BTextView( frame, name, textRect, resizingMode, flags )
{
	SetViewColor( 255, 255, 255 );
	TypeMode = mode;
	insertstyle.offset = 0;
	message = NULL;
	Empty = true;
	urlOn = false;
	showFontColorSizes = true;
	beingDragged = false;
};

//----------------------------------------------------------------------------------------

void HTMLView::SetBeingDragged( bool bd ) {
	beingDragged = bd;
}

//----------------------------------------------------------------------------------------

void HTMLView::MouseMoved( BPoint where, uint32 code, const BMessage *msg ) {
	
	if( !beingDragged ) {
		BView::MouseMoved( where, code, msg );
		if( code == B_EXITED_VIEW )
			be_app->SetCursor(B_HAND_CURSOR);
	}
}

//----------------------------------------------------------------------------------------

void HTMLView::SetBaseFontAndColor( BFont& font, rgb_color& color, bool SetNow ) {
	baseFont = font;
	baseColor = color;
	if( SetNow ) {
		insertstyle.font = font;
		insertstyle.color = color;
		SetFontAndColor( &font, B_FONT_ALL, &color );
	}
}

//----------------------------------------------------------------------------------------

// use later to do on-the-fly formatting, perhaps HTML?
void HTMLView::InsertText( const char *text, int32 length, int32 offset, const text_run_array *runs ) {
	Window()->PostMessage( new BMessage(TEXT_WAS_MODIFIED) );
	BTextView::InsertText( text, length, offset, runs );
}

//----------------------------------------------------------------------------------------

void HTMLView::DeleteText( int32 start, int32 finish ) {
	Window()->PostMessage( new BMessage(TEXT_WAS_MODIFIED) );
	BTextView::DeleteText( start, finish );
}
	
//----------------------------------------------------------------------------------------

void HTMLView::SetFontAttribute( int32 attrib, bool state ) {
		
	uint16 curMask = insertstyle.font.Face();
	if( state )
		curMask = ((curMask & B_REGULAR_FACE) ? 0 : curMask) | attrib;
	else
		curMask = (curMask == attrib) ? B_REGULAR_FACE : curMask & (~attrib);			
	insertstyle.font.SetFace( curMask );
	styleChanged = true;
}

//----------------------------------------------------------------------------------------

void HTMLView::SetFontColor( rgb_color color ) {
	insertstyle.color = color;
	styleChanged = true;	
}

//----------------------------------------------------------------------------------------

void HTMLView::SetFontSize( int32 size ) {
	insertstyle.font.SetSize( (float)size );
	styleChanged = true;
}

//----------------------------------------------------------------------------------------

void HTMLView::SetFontColor( int32 r, int32 g, int32 b ) {
	rgb_color newColor;
	newColor.red = r;
	newColor.green = g;
	newColor.blue = b;
	insertstyle.color = newColor;
	styleChanged = true;
}

//----------------------------------------------------------------------------------------

void HTMLView::InsertHTMLText( char* text ) {

	// Add the style to the list (if it has changed)
	if( styleChanged ) {
		insertstyle.offset = curOffset + (int)!Empty;	// if first line, dont add 1
		insertStyles.Add( insertstyle );
		styleChanged = false;
	}
	
	// "add" the text (to the internal array, anyway)
	curOffset += strlen( text );
	strcat( insertText, text );		
}

//----------------------------------------------------------------------------------------

void HTMLView::ResetFontToBase() {	
	insertstyle.font = baseFont;
	insertstyle.color = baseColor;
	insertstyle.offset = 0;
	Select( 0, 0 );
	SetFontAndColor( &baseFont, B_FONT_ALL, &baseColor );
	styleChanged = true;
}

//----------------------------------------------------------------------------------------

void HTMLView::SetShowFontColorSizes( bool show ) {
	showFontColorSizes = show;
}

//----------------------------------------------------------------------------------------

void HTMLView::ClearFontStates() {

	// Clear all the saved font states, and NULL out the insert string
	ResetFontToBase();	
	insertStyles.Clear();
	curOffset = 0;
	styleChanged = false;	

	// Only add a beginning line break if the textview is empty
	if( !Empty ) {
		insertText[0] = '\n';
		insertText[1] = '\0';
	} else
		insertText[0] = '\0';
}

//----------------------------------------------------------------------------------------

void HTMLView::AddStatement() {

	int32 number_of_runs = (int32)insertStyles.Count();
	int32 lineCount, lastOffset;
	bool doAutoScroll = true;
	
	// Get the scrollbar position... this is the part where we do autoscroll, but only 
	//   if the scrollbar is already at the bottom. That way it won't get annoying.
	if( scroll == NULL ) scroll = ScrollBar( B_VERTICAL );
	if( scroll ) {
		float scrMin, scrMax;
		scroll->GetRange( &scrMin, &scrMax );
		if( scroll->Value() != scrMax )
			doAutoScroll = false;
	}	

	// allocate some room for the text_run_array
	text_run_array* styles = (text_run_array*)malloc( sizeof(text_run_array) +
							(sizeof(text_run) * (number_of_runs - 1)));							

	// fill in the individual text_runs
	styles->count = number_of_runs;
	for( int32 i = 0; i < number_of_runs; ++i )
		styles->runs[i] = insertStyles[i];
		
	// Get the offset of the last line
	lineCount = CountLines();
	lastOffset = OffsetAt( lineCount );

	// Add the silly thing
	Insert( lastOffset, insertText, strlen(insertText), styles );
	
	// do the autoscroll, if needed
	if( doAutoScroll ) {
		lineCount = CountLines();
		lastOffset = OffsetAt( lineCount );
		ScrollToOffset( lastOffset );	
	}
		
	// Prepare to start over
	Empty = false;
	ClearFontStates();
	free( styles );
}

//----------------------------------------------------------------------------------------

// Format the message into HTML, and return it
char* HTMLView::GetFormattedMessage() {

	BString theText = Text();
	
	// replace all linebreaks with <br>'s
	theText.ReplaceAll( "\n", "<br>" );

	message = (char*)malloc( theText.Length() + 1 );
	theText.CopyInto( message, 0, theText.Length() );
	message[theText.Length()] = '\0';
	
	return message;
}

//----------------------------------------------------------------------------------------

// Return the message as raw text
char* HTMLView::GetRawTextMessage() {

	// Copy the text out of the buffer, and into string FullText
	message = (char*)malloc( TextLength() + 1 );
	strcpy( message, Text() );
	
	return message;
}

//----------------------------------------------------------------------------------------

int HTMLView::HexcharToInt( char hex )
{
	if ( (hex >= 0x41) && (hex <= 0x46) )
    	return (hex - 0x41) + 0xa; 				// is upper-case A-F
	else if ( (hex >= 0x61) && (hex <= 0x66) )
		return (hex - 0x61) + 0xa;				// is lower-case a-f
	else if ( (hex >= 0x30) && (hex <= 0x39) )
		return (hex - 0x30);					// is numeric digit
	else
		return 0;
}

//----------------------------------------------------------------------------------------

bool HTMLView::FirstWordMatch( char* sentence, char* word ) {

	//while( *(sentence++) && (*sentence != ' ') && *(word++) ) {
	while( *(sentence++) && isalpha(*sentence) && *(word++) ) {
		if( tolower(*sentence) != tolower(*word) )
			return false;
	}
	return true;
}

//----------------------------------------------------------------------------------------

bool HTMLView::GetTagAttribute( char* tag, char* attrib, char* value ) {

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

	// no equal sign - this attribute doesn't have a value (like <TD nowrap>		
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
			if( *p )						// get past the closing quote
				++p;
		}
	}
	
	// true indicates that there is more waiting	
	return true;
}

//----------------------------------------------------------------------------------------

void HTMLView::DoLinkAttributes( char* tag ) {

	char attrib[100];		// general size assumptions for attribs and values
	char value[512];		// the URL might get a bit lengthy though...
	
	// Loop through, getting all the attibutes, and process them
	bool more = GetTagAttribute( tag, attrib, value );
	while( more ) {	

		// starts with 'h' (probably href)
		if( (*attrib == 'H') || (*attrib == 'h') ) {
			if( FirstWordMatch( attrib, "href" ) ) {
				strcpy( URL, value );
				urlOn = true;
			}
		}
													
		more = GetTagAttribute( NULL, attrib, value );
	}
}
	
//----------------------------------------------------------------------------------------

void HTMLView::DoFontTagAttributes( char* tag ) {

	char attrib[100];		// general size assumptions for attribs and values
	char value[100];
	bool Changed;
	fontInfo savedInfo;
	
	// first save the relevant font info in case we need to push it on the stack
	savedInfo.color = insertstyle.color;
	savedInfo.size = insertstyle.font.Size();
	
	// Loop through, getting all the attibutes, and process them
	bool more = GetTagAttribute( tag, attrib, value );
	while( more ) {	

		// starts with 's' (probably size)
		if( (*attrib == 'S') || (*attrib == 's') ) {
			if( FirstWordMatch( attrib, "size" ) ) {
				Changed = true;
				int fontSize = atoi( value );
				if( fontSize == 1 )
					SetFontSize( 8 );
				else if( fontSize == 2 )
					SetFontSize( 10 );		
				else if( fontSize == 3 )
					SetFontSize( 12 );	
				else if( fontSize == 4 )
					SetFontSize( 14 );	
				else if( fontSize == 5 )
					SetFontSize( 18 );
				else if( fontSize == 6 )
					SetFontSize( 24 );
				else if( (fontSize >= 7) && (fontSize < 42) )
					SetFontSize( 38 );	
			}
		}
		
		// starts with 'c' (probably color)
		if( (*attrib == 'C') || (*attrib == 'c') ) {
			if( FirstWordMatch( attrib, "color" ) ) {
				Changed = true;
				
				// must be in the form #xxxxxx
				if( strlen(value) != 7 || value[0] != '#' )
					continue;
				rgb_color fontColor;
				fontColor.red = 16*HexcharToInt(value[1]) + HexcharToInt(value[2]);
				fontColor.green = 16*HexcharToInt(value[3]) + HexcharToInt(value[4]);
				fontColor.blue = 16*HexcharToInt(value[5]) + HexcharToInt(value[6]);
				SetFontColor( fontColor );
			}
		}		
													
		more = GetTagAttribute( NULL, attrib, value );
	}
	
	// if the styles have changed at all, push the old style onto the stack
	fontStack.Push( savedInfo );
}

//----------------------------------------------------------------------------------------

void HTMLView::HandleHTMLTag( char* tag ) {

	char* p;
	bool off = false;
	bool handled = false;

	// Trim all the whitespace off the left side of the tag
	p = tag;
	while( *p == ' ' )
		++p;
		
	// return if there is nothing left
	if( !(*p) )
		return;

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
		if( !(*(p+1)) || *(p+1) == ' ' ) {
			SetFontAttribute( B_BOLD_FACE, !off );
			handled = true;
		}

		// if it is a body tag		
		else if( FirstWordMatch( p, "body" ) )
			handled = true;
			
		// if it is a line break		
		else if( FirstWordMatch( p, "br" ) ) {
			InsertHTMLText( "\n" );
			handled = true;
		}
	}
		
	// starts with 'i' (probably italic)
	else if( (*p == 'I') || (*p == 'i') ) {
		if( !(*(p+1)) || *(p+1) == ' ' ) {
			SetFontAttribute( B_ITALIC_FACE, !off );
			handled = true;
		}
	}
	
	// starts with 'f' (probably <FONT> )
	else if( (*p == 'F') || (*p == 'f') ) {
		if( FirstWordMatch( p, "font" ) ) {
			if( !off && showFontColorSizes )
				DoFontTagAttributes( p );
			else {
				if( !fontStack.IsEmpty() ) {
					fontInfo savedInfo;
					fontStack.Pop( savedInfo );
					SetFontColor( savedInfo.color );
					SetFontSize( savedInfo.size );
				} else
					ResetFontToBase();
			}			
			handled = true;
		}
	}
	
	// starts with 'a' (probably <a href="..."> )
	else if( (*p == 'A') || (*p == 'a') ) {
		if( FirstWordMatch( p, "a" ) ) {
			if( !off )
				DoLinkAttributes( p );
			else {
				// Insert the URL (kind of cheap, but it works for now)
				InsertHTMLText( " [" );
				InsertHTMLText( URL );
				InsertHTMLText( "]" );
				URL[0] = '\0';
				urlOn = false;
			}
			handled = true;
		}
	}	
	
	// starts with 'h' (probably <HTML> )
	else if( (*p == 'H') || (*p == 'h') ) {
		if( FirstWordMatch( p, "html" ) )
			handled = true;
	}
	
	// starts with 'u' (underline)
	else if( (*p == 'U') || (*p == 'u') ) {
		if( !(*(p+1)) || *(p+1) == ' ' )
			handled = true;
	}
	
	// starts with 's' (<sub> or <sup> )
	else if( (*p == 'S') || (*p == 's') ) {
		if( FirstWordMatch(p,"sub") || FirstWordMatch(p,"sup") )
			handled = true;
	}	
	
	// if it wasn't handled, it was probably some silly user
	//   saying stuff like <this>, so print it out raw
	if( !handled ) {
		InsertHTMLText( "<" );
		InsertHTMLText( tag );
		InsertHTMLText( ">" );
	}
}

//----------------------------------------------------------------------------------------

void HTMLView::ParseHTMLStatement( char* text )
{
	// vars
	char tag[150];
	char *p, *last, *out = NULL;
	int catpos;
	fontStack.Clear();

	// if there is no input, return
	if( text == NULL )
		return;

	// iterator starts at the beginning
	p = text;
	last = p;

	// loop through all the characters
	while( *p ) {
	
		// Beginning of an HTML tag?		
		if( *p == '<' )
		{	
			// If there was any text before this, print it
			if( (int)(p - last) ) {
				delete out;
				out = new char[(p - last)+1];
				strncpy( out, last, (int)(p - last) );
				out[ (int)(p - last) ] = '\0';
				InsertHTMLText( out );
			}
			
			// Re-assign last, in case this isn't a tag after all
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
				InsertHTMLText( out );
				if( *p == '<' )
					--p;
				else
					return;
			}
			
			// otherwise, it was an end tag and we should process it
			else {
				tag[catpos++] = '\0';
				HandleHTMLTag( tag );
				tag[0] = '\0';
			}
			
			// reset last
			last = p + 1;
		}
	
		// next character
		if( *p )
			++p;	
	}
	
	// Was the last part a statement, perhaps? If so, add it
	if( last != p ) {
		delete out;
		out = new char[(p - last)+1];
		out[0] = '\0';
		strncpy( out, last, (int)(p - last) );
		out[(int)(p - last)] = '\0';
		InsertHTMLText( out );	
	}
	
	// Clear out the font stack
	fontStack.Clear();
}
