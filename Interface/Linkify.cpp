#include "Linkify.h"
#include <stdio.h>
#include <ctype.h>
#include "GenList.h"

struct linkTag {
	int32 start, stop;
	bool on;
};

bool IsValidURLChar( char c, bool end ) {
	if( isalnum(c) )
		return true;
	switch( c ) {
		case '.':
		case ',':
		case ':':
		case '!':
		case '-':
		case '?':
		case '*':
		case '(':
		case ')':
		case '_':
			if( end )
				return false;
			return true;		
		case '+':
		case '/':
		case '&':
		case '=':
		case '$':
		case '@':
		case '~':
		case '\'':
			return true;
	}
	return false;
}

bool IsValidMailChar( char c ) {
	if( isalnum(c) )
		return true;
	switch( c ) {
		case '_':
		case '-':
		case '.':
 			return true;		
	}
	return false;
}

//_ @ - . alphanum 

bool IsLinkableURL( GenList<linkTag>& links, int32 lsStart, int32 lsStop ) {

	linkTag temp;
	bool linkOn = false;

	for( unsigned i = 0; i < links.Count(); ++i ) {
		temp = links[i];
		if( (lsStart >= temp.start && lsStart <= temp.stop) ||
			(lsStop  >= temp.start && lsStop  <= temp.stop) )
			return false;
	}
	for( unsigned i = 0; i < links.Count(); ++i ) {
		temp = links[i];
		if( temp.start > lsStart ) {
			if( linkOn )
				return false;
			break;
		}
		linkOn = links[i].on;
	}
	return true;
}

void InsertNewLink( GenList<linkTag>& links, int32 where, int32 howlong, bool on ) {

	unsigned i = 0;

	linkTag nLink;
	nLink.on = on;
	nLink.start = where;
	nLink.stop = where+howlong;

	for( i = 0; i < links.Count(); ++i ) {
		if( links[i].start > where ) {
			break;
		}
	}

	links.Insert( nLink, i );
}

void OffsetLinksAfterPoint( GenList<linkTag>& links, int32 afterWhat, int32 howMuch ) {
	for( unsigned i = 0; i < links.Count(); ++i ) {
		if( links[i].start > afterWhat )
			links[i].start += howMuch;
		if( links[i].stop > afterWhat )
			links[i].stop += howMuch;
	}
	return;
}

void Linkify( BString& input ) {

	//printf( "input: %s\n", input.String() );

	BString tagTemp, linkTemp;
	int32 start = -1, k;
	GenList<linkTag> links;
	int32 off = 0, off2 = 0, off3 = 0;
	bool on;
	
	// go through all the characters so we can find the links
	for( int32 i = 0; i < input.Length(); ++i ) {
	
		if( input[i] == '<' )
			start = i;
		else if( input[i] == '>' ) {
			if( start != -1 && (i-start) > 1 ) {
				tagTemp = input;
				tagTemp.Truncate(i);
				tagTemp.Remove(0, start+1);
				on = true;
				k = 0;
				
				// now verify that it's a link tag
				while( tagTemp.Length() && isspace(tagTemp[0]) )
					tagTemp.Remove(0,1);
				if( !tagTemp.Length() )
					continue;
				if( tagTemp[0] == '/' ) {
					on = false;
					tagTemp.Remove(0,1);
				} while( tagTemp.Length() && isspace(tagTemp[0]) )
					tagTemp.Remove(0,1);				
				if( !tagTemp.Length() )
					continue;				
				while( k < tagTemp.Length() && isalpha(tagTemp[k]) )
					++k;
				tagTemp = tagTemp.Truncate(k);
				tagTemp = tagTemp.ToLower();
				if( tagTemp == "a" ) {
					linkTag nLink;
					nLink.on = on;
					nLink.start = start;
					nLink.stop = i+1;
					links.Add(nLink);
				}
			}
		}
	}
	
	// now find and linkify all the "http://" thingers
	off = off2 = off3 = 0;
	off = input.IFindFirst("http://", off2);
	while( off != B_ERROR ) {
		off2 = off;
		if( off == 0 || !isalnum(input[off-1]) ) {
			while( off2 < input.Length() && IsValidURLChar(input[off2], false) )
				++off2;
			while( off2 > off && !IsValidURLChar(input[off2-1], true) )
				--off2;
			linkTemp = input;
			linkTemp.Truncate( off2 );
			linkTemp.Remove( 0, off );
			if( IsLinkableURL( links, off, off2 ) ) {
				//printf( "found link: %s\n", linkTemp.String() );
				input.Insert( "</a>", off2 );
				input.Insert( "\">", off );
				input.Insert( linkTemp, off );
				input.Insert( "<a href=\"", off );
				OffsetLinksAfterPoint( links, off2, 15+linkTemp.Length() );
				InsertNewLink( links, off, 11+linkTemp.Length(), true );
				InsertNewLink( links, off+11+2*linkTemp.Length(), 4, false );
			}
		} else
			off2 += 7;
		off = input.IFindFirst("http://", off2);
	}

	// now find and linkify all the "www" thingers
	off = off2 = off3 = 0;
	off = input.IFindFirst("www", off2);
	while( off != B_ERROR ) {
		off2 = off;
		if( off == 0 || !isalnum(input[off-1]) ) {
			while( off2 < input.Length() && IsValidURLChar(input[off2], false) )
				++off2;
			while( off2 && off2 > off && !IsValidURLChar(input[off2-1], true) )
				--off2;
			linkTemp = input;
			linkTemp.Truncate( off2 );
			linkTemp.Remove( 0, off );
			if( IsLinkableURL( links, off, off2 ) ) {
				//printf( "found link: %s\n", linkTemp.String() );
				input.Insert( "</a>", off2 );
				input.Insert( "\">", off );
				input.Insert( linkTemp, off );
				input.Insert( "<a href=\"http://", off );
				OffsetLinksAfterPoint( links, off2, 22+linkTemp.Length() );
				InsertNewLink( links, off, 18+linkTemp.Length(), true );
				InsertNewLink( links, off+18+2*linkTemp.Length(), 4, false );
			}
		} else
			off2 += 3;
		off = input.IFindFirst("www", off2);
	}
		
	// now find and linkify all the e-mail addresses
	off = off2 = off3 = 0;
	off = input.IFindFirst("@", off2);
	while( off && off != B_ERROR ) {
		int dotCount = 0;
		if( !off ) {
			off = input.IFindFirst("@", off2);
			continue;
		}
		off3 = off2 = off;
		while( off && IsValidMailChar(input[off-1]) )
			--off;
		++off2;
		while( off2 < input.Length() && IsValidMailChar(input[off2]) ) {
			if( input[off2] == '.' )
				dotCount++;
			++off2;
		}
		linkTemp = input;
		linkTemp.Truncate( off2 );
		linkTemp.Remove( 0, off );
		if( dotCount && IsLinkableURL( links, off, off2 ) ) {
			//printf( "found link: [%s]\n", linkTemp.String() );
			input.Insert( "</a>", off2 );
			input.Insert( "\">", off );
			input.Insert( linkTemp, off );
			input.Insert( "<a href=\"mailto:", off );
			OffsetLinksAfterPoint( links, off2, 22+linkTemp.Length() );
			InsertNewLink( links, off, 18+linkTemp.Length(), true );
			InsertNewLink( links, off+18+2*linkTemp.Length(), 4, false );
		}
		off = input.IFindFirst("@", off2);
	}
}

