#include "FancyView.h"
#include <Alert.h>
#include <Application.h>
#include <Roster.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TextElement.h"
#include "Globals.h"

rgb_color kLinkColor = {25,0,225};

//=========================================================

void linkInfo::Set( int32 off, int32 ln, char* nlink ) {

	delete link;
	link = 0;
	offset = off;
	len = ln;
	
	if( nlink ) {
		link = new char[strlen(nlink) + 1];
		strcpy( link, nlink );
	}
}

//=========================================================

FancyTextView::FancyTextView( BRect frame, const char *name, BRect textRect, uint32 resizingMode, uint32 flags )
		: BTextView( frame, name, textRect, resizingMode, flags )
{
	SetViewColor( 255, 255, 255 );
	insertstyle.offset = 0;
	message = NULL;
	Empty = true;
	showFontColorSizes = true;
	showLinks = true;
	autoScrollEnabled = true;
	beingDragged = false;
	overLink = false;
	textMag = 1.0;
	scroll = NULL;
};

//---------------------------------------------------------

void FancyTextView::Clear() {
	Empty = true;
	SetText("");
}

//---------------------------------------------------------

void FancyTextView::SetBaseFontAndColor( BFont& font, rgb_color& color, bool SetNow ) {
	baseFont = font;
	baseColor = color;
	if( SetNow ) {
		insertstyle.font = font;
		insertstyle.color = color;
		SetFontAndColor( &font, B_FONT_ALL, &color );
	}
}

//---------------------------------------------------------

// use later to do on-the-fly formatting, perhaps?
void FancyTextView::InsertText( const char *text, int32 length, int32 offset, const text_run_array *runs ) {

	BTextView::InsertText( text, length, offset, runs );
}
	
//---------------------------------------------------------

void FancyTextView::SetFontAttribute( int32 attrib, bool state ) {
		
	uint16 curMask = insertstyle.font.Face();
	if( state )
		curMask = ((curMask & B_REGULAR_FACE) ? 0 : curMask) | attrib;
	else
		curMask = (curMask == attrib) ? B_REGULAR_FACE : curMask & (~attrib);			
	insertstyle.font.SetFace( curMask );
	styleChanged = true;
}

//---------------------------------------------------------

void FancyTextView::SetFontColor( rgb_color color ) {
	insertstyle.color = color;
	styleChanged = true;	
}

//---------------------------------------------------------

void FancyTextView::SetFontSize( float size ) {
	insertstyle.font.SetSize( (float)size * textMag );
	styleChanged = true;
}

//---------------------------------------------------------

void FancyTextView::SetFontColor( int32 r, int32 g, int32 b ) {
	rgb_color newColor;
	newColor.red = r;
	newColor.green = g;
	newColor.blue = b;
	insertstyle.color = newColor;
	styleChanged = true;
}

//---------------------------------------------------------

void FancyTextView::InsertSomeText( char* text ) {

	textRunStyle masterRec;

	// get the insertion offset for the text
	int32 lastline = CountLines();
	int32 offset = OffsetAt( lastline );

	// Add the style to the list (if it has changed)
	if( styleChanged ) {
		insertstyle.offset = curOffset;
		insertStyles.Add( insertstyle );
		masterRec.run = insertstyle;
		masterRec.run.offset += offset;
		masterRec.variable = false;
		masterRec.link = false;
		masterStyles.Add( masterRec );
		styleChanged = false;
	}

	// "add" the text (to the internal array, anyway)
	curOffset += StrLen( text );
	strcat( insertText, text );
}

//---------------------------------------------------------

void FancyTextView::ResetFontToBase() {	
	insertstyle.font = baseFont;
	insertstyle.font.SetSize( baseFont.Size() * textMag );
	insertstyle.color = baseColor;
	insertstyle.offset = 0;
	Select( 0, 0 );
	SetFontAndColor( &baseFont, B_FONT_ALL, &baseColor );
	styleChanged = true;
}

//---------------------------------------------------------

void FancyTextView::SetShowFontColorSizes( bool show ) {
	showFontColorSizes = show;
	if( !Empty )
		RebuildStyles();
}

//---------------------------------------------------------

void FancyTextView::SetShowLinks( bool show ) {
	showLinks = show;
	if( !Empty )
		RebuildLinks();
}

//---------------------------------------------------------

void FancyTextView::SetAutoScrollEnabled( bool enabled ) {
	autoScrollEnabled = enabled;
}

//---------------------------------------------------------

void FancyTextView::RebuildStyles() {

	textRunStyle thisRec, nextRec;
	bool keepGoing;
	const unsigned blockSize = 150;
	uint32 start = 0, stop = 0, counter = 0;
	float baseFontSize = baseFont.Size() * textMag;
	
	// allocate some room for the text_run_array
	text_run_array* newStyles = (text_run_array*)malloc( sizeof(text_run_array) +
								(sizeof(text_run) * (blockSize - 1)));
	newStyles->count = 0;

	// go through all the master records (in blocks of size spec'd by blockSize)
	keepGoing = masterStyles.First(thisRec);
	while( keepGoing ) {

		// grab the next record
		keepGoing = masterStyles.Next(nextRec);

		// assign the stop value
		stop = keepGoing ? nextRec.run.offset : TextLength();
					
		// When to LEAVE the coloring... if this block is:
		// 1. not variable
		//  -- or --
		// 2. showFontColorSizes is ON
		//  -- or --
		// 3. This is a link, and links are on

		// leave the original formatting there
		if( !thisRec.variable || showFontColorSizes ) {
			newStyles->runs[counter] = thisRec.run;
			newStyles->runs[counter].offset -= start;
		}

		// otherwise, turn it off
		else {
			newStyles->runs[counter] = thisRec.run;
			newStyles->runs[counter].offset -= start;
			newStyles->runs[counter].font.SetSize(baseFontSize);
			if( !(thisRec.link && showLinks) )
				newStyles->runs[counter].color = baseColor;
			else
				newStyles->runs[counter].color = kLinkColor;
		}

		// if we've reached the end of the blocksize, commit the changes
		if( (counter + 1) == blockSize ) {
			newStyles->count = blockSize;
			SetRunArray( start, stop, newStyles );
			start = nextRec.run.offset;
			counter = 0;
		} else
			++counter;

		// assign the next record
		thisRec = nextRec;
	}
	
	// if there are any remaining styles, commit them too
	if( counter ) {
		newStyles->count = counter;
		SetRunArray( start, stop, newStyles );
	}
	
	// free the text_run_array
	free(newStyles);
}

//---------------------------------------------------------

void FancyTextView::RebuildLinks() {

	textRunStyle thisRec, nextRec;
	bool keepGoing;
	uint32 start, stop;

	// set the link style basics
	text_run_array* linkStyle = (text_run_array*)malloc( sizeof(text_run_array) );
	linkStyle->count = 1;
	linkStyle->runs[0].offset = 0;

	// go through all the master records looking for links
	keepGoing = masterStyles.First(thisRec);
	while( keepGoing ) {

		// grab the next record
		keepGoing = masterStyles.Next(nextRec);

		// assign the start/stop values
		start = thisRec.run.offset;
		stop = keepGoing ? nextRec.run.offset : TextLength();
		
		// if it's a link, deal with it...
		if( thisRec.link ) {

			// set the "appropriate" style info, and commit it
			linkStyle->runs[0] = thisRec.run;
			linkStyle->runs[0].offset = 0;
			if( !(!thisRec.variable || showFontColorSizes) ) {
				linkStyle->runs[0].font.SetSize(baseFont.Size() * textMag);
				linkStyle->runs[0].color = baseColor;
			}
			if( showLinks )
				linkStyle->runs[0].color = kLinkColor;
			SetRunArray( start, stop, linkStyle );
		}

		// assign the next record
		thisRec = nextRec;
	}
	
	// free the text_run_array
	free( linkStyle );
}

//---------------------------------------------------------

void FancyTextView::ClearInsertStuff() {

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

//---------------------------------------------------------

void FancyTextView::AddStatement() {

	int32 number_of_runs = (int32)insertStyles.Count();
	int32 lineCount, lastOffset;
	bool doAutoScroll = true;
	int32 emptyOffset = 0;
	text_run temp;

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

	// if the view isn't empty, then we have to offset everything by one
	//  to account for the newline inserted at the start of each statement
	emptyOffset = 0;
	if( !Empty )
		emptyOffset = 1;
	
	// fill in the individual text_runs
	styles->count = number_of_runs;
	insertStyles.First(temp);
	for( int32 i = 0; i < number_of_runs; ++i ) {
		if( i )
			temp.offset += emptyOffset;
		styles->runs[i] = temp;
		insertStyles.Next(temp);
	}

	// Get the offset of the last line
	lineCount = CountLines();
	lastOffset = OffsetAt( lineCount );

	// Add the silly thing... if there are no text_runs, don't use styles
	if( number_of_runs )
		Insert( lastOffset, insertText, strlen(insertText), styles );
	else
		Insert( lastOffset, insertText, strlen(insertText) );
	
	// do the autoscroll, if needed
	if( autoScrollEnabled && doAutoScroll ) {
		lineCount = CountLines();
		lastOffset = OffsetAt( lineCount );
		ScrollToOffset( lastOffset );	
	}
		
	// Prepare to start over
	Empty = false;
	ClearInsertStuff();
	free( styles );
}

//---------------------------------------------------------

void FancyTextView::AddStyledText( char* text, styleList& insStyles, bool saveStyles ) {

	// vars
	text_run single, blank;
	gTextElement element;
	bool keepGoing, linkWaiting = false;
	unsigned mask, smask;
	rgb_color color1;
	gColor color2;
	int32 fontMask, offset = 0;
	linkInfo nLink;
	textRunStyle masterRec;
	GenList<gTextElement>* tStyles = insStyles.theStyles;

	// make the "blank" insert style
	blank.font = baseFont;
	blank.color = baseColor;

	// get the insertion offset for the text
	int32 lastline = CountLines();
	offset = OffsetAt( lastline );
	if( !Empty )
		++offset;
		
	// if there aren't any styles, set it to the "blank" style
	if( !tStyles->Count() ) {
		single = blank;
		single.offset = curOffset;
		masterRec.run = single;
		masterRec.run.offset += offset;
		masterRec.variable = saveStyles;
		masterRec.link = false;
		insertStyles.Add( single );
		masterStyles.Add( masterRec );
	}

	// go through all the styles
	keepGoing = tStyles->First( element );
	while( keepGoing ) {

		// set the offset and get the mask
		single = blank;
		single.offset = element.Offset() + curOffset;
		mask = element.Mask();

		// is it blank?
		if( mask == TE_BLANK ) {
			single.font = baseFont;
			single.color = baseColor;
		}

		// handle font attributes (bold, italic, etc.)
		if( mask & TE_FONTSTYLE ) {
			fontMask = 0;
			smask = element.FontStyle();
			if( smask & ST_BOLD )
				fontMask |= B_BOLD_FACE;
			if( smask & ST_ITALIC )
				fontMask |= B_ITALIC_FACE;
			if( smask & ST_UNDERLINE )
				fontMask |= B_UNDERSCORE_FACE;
			single.font.SetFace( fontMask );
		} else
			single.font.SetFace( B_REGULAR_FACE );

		// handle font size	
		if( showFontColorSizes )
			if( mask & TE_FONTSIZE )
				single.font.SetSize( element.FontSize() * textMag );
			else
				single.font.SetSize( blank.font.Size() * textMag );

		// handle font color
		if( showFontColorSizes && (mask & TE_FONTCOLOR) ) {
			color2 = element.FontColor();
			MakeRGBColor( color2, color1 );
			single.color = color1;		
		}

		// handle links
		if( mask & TE_LINK ) {
			if( linkWaiting ) {
				nLink.len = offset + single.offset - nLink.offset;
				links.Add( nLink );
				linkWaiting = false;
			}		
			//single.color = kLinkColor;
			nLink.offset = single.offset + offset;
			nLink.Set( single.offset + offset, 0, (char*)element.Link().Link() );
			linkWaiting = true;
		} else {
			if( linkWaiting ) {
				nLink.len = offset + single.offset - nLink.offset;
				links.Add( nLink );
				linkWaiting = false;
			}
		}

		// add the style, and keep going if we can
		masterRec.run = single;
		masterRec.run.offset += offset;
		masterRec.variable = saveStyles;
		masterRec.link = bool(mask & TE_LINK);
		masterStyles.Add( masterRec );
		if( bool(mask & TE_LINK) && showLinks )
			single.color = kLinkColor;
		insertStyles.Add( single );
		keepGoing = tStyles->Next( element );
	}	

	// insert it
	styleChanged = false;
	InsertSomeText( text );
}

//---------------------------------------------------------

void FancyTextView::MakeRGBColor( gColor& color2, rgb_color& color1 ) {
	color1.red = color2.R();
	color1.green = color2.G();
	color1.blue = color2.B();
}

//---------------------------------------------------------

uint32 FancyTextView::StrLen( char* str ) {
	return (uint32)strlen(str);
}

//---------------------------------------------------------

void FancyTextView::MouseMoved( BPoint where, uint32 code, const BMessage *msg ) {

	int32 moveOffset = OffsetAt(where);
	bool getNext;
	linkInfo thisLink;
	
	// if links aren't on, forget about all this
	if( !showLinks ) {
		BTextView::MouseMoved(where, code, msg);
		return;
	}	
	
	// are we still over a link?
	if( overLink && !(links.Current(thisLink) && moveOffset >= thisLink.offset && moveOffset < thisLink.offset + thisLink.len) ) {
		Window()->PostMessage( new BMessage(FANCY_NOT_OVER_LINK) );	
		overLink = false;
	}
	
	
	// start from the bottom, since that's where the link is likely to be
	getNext = links.Last( thisLink );
	while( getNext ) {
			
		// check and see if we are over a link
		if( moveOffset >= thisLink.offset && moveOffset < thisLink.offset + thisLink.len ) {
			be_app->SetCursor(B_HAND_CURSOR);
			BMessage* msg = new BMessage(FANCY_OVER_LINK);
			msg->AddString("url", thisLink.link);
			Window()->PostMessage( msg );
			overLink = true;
			return;
		}
		getNext = links.Prev( thisLink );
	}
	
	if( !beingDragged )
		BTextView::MouseMoved(where, code, msg);
}

//---------------------------------------------------------

void FancyTextView::MouseDown( BPoint where ) {

	bool getNext, found = false;
	linkInfo thisLink;
	int32 clickOffset = OffsetAt(where);

	// if links aren't on, forget about all this
	if( !showLinks ) {
		BTextView::MouseDown(where);
		return;
	}

	// make sure this window has the focus
	MakeFocus( true );

	// start from the bottom, since that's where the link is likely to be
	getNext = links.Last( thisLink );
	while( getNext ) {

		// check and see if this was the link we clicked on
		if( clickOffset >= thisLink.offset && clickOffset < thisLink.offset + thisLink.len ) {
			found = true;
			break;
		}
		getNext = links.Prev( thisLink );
	}

	if( found ) {
		Select( thisLink.offset, thisLink.offset + thisLink.len );
		Window()->UpdateIfNeeded();
		Open( thisLink.link );
	}

	// call the original implementation...
	BTextView::MouseDown(where);
}

//---------------------------------------------------------

void FancyTextView::Open( char* link ) {

	status_t result;
	
	// the "header" characters
	char header[15];

	// check for "http://"
	strncpy( header, link, 7 );
	header[7] = '\0';
	if( strcasecmp(header, "http://") == 0 ) {
		result = be_roster->Launch( "text/html", 1, &link );
		if( (result != B_NO_ERROR) && (result != B_ALREADY_RUNNING) ) {
			windows->ShowMessage( Language.get("ERR_NO_HTML_HANDLER") );
		}
		return;
	}
	
	// check for "mailto:"
	strncpy( header, link, 7 );
	header[7] = '\0';
	if( strcasecmp(header, "mailto:") == 0 ) {
		be_roster->Launch("text/x-email", 1, &link);
	}
}

//---------------------------------------------------------

void FancyTextView::SetBeingDragged( bool bd ) {
	beingDragged = bd;
}

//---------------------------------------------------------

void FancyTextView::SetTextMagnification( float tm ) {
	textMag = tm;
}

//---------------------------------------------------------

