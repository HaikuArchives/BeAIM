#include "PassControl.h"
#include "myFilter.h"
#include "Say.h"


PassControl::PassControl(BRect frame,
						const char *name,
						const char *label, 
						const char *initial_text, 
						BMessage *message)
			: BTextControl(frame, name, label, initial_text, message)

{
	length = 0;
	filter = new myFilter();
	((BTextView *)TextView())->AddFilter(filter);
	
	// font-sensitive code
	// needs for the Width() to be sufficient. this code just makes room 
	// for the label...
	SetDivider(StringWidth(label) + 2.5);
}

PassControl::~PassControl()
{
	if(((BTextView *)TextView())->RemoveFilter(filter))
	{
		delete filter;
	}
}

void PassControl::SetText(const char *text)
{
	// set the actual text
	actual = text;
	
	// make the star string
	BString star_string;
	star_string.Append( '*', actual.CountChars() );

	// call the base class function
	BTextControl::SetText(star_string.String());
}

BString PassControl::actualText() const
{ return actual.String(); }


void PassControl::InsertText( BMessage* msg ) {

	// ignore UTF-8 for now
	
	
	int32 start, end;
	char c;
	TextView()->GetSelection( &start, &end );
	
	// remove the text we're replacing (if any)
	if( start != end )
		actual.Remove( start, end-start );
		
	msg->FindInt8("byte", (int8*)&c );
	actual.Insert( c, 1, start );
}


void PassControl::DeleteText( bool delMode ) {

	int32 start, end;
	TextView()->GetSelection( &start, &end );
	
	// for start != end, delete and backspace are equivalent
	if( start != end )
		actual.Remove( start, end-start );
	
	else {
	
		// the delete key has been pressed
		if( delMode ) {
		
			// ignore if we're at the end of the text
			if( start == actual.CountChars() )
				return;
			actual.Remove( start, 1 );
		}
		
		// the backspace key has been pressed
		else {

			// ignore if we're at the start of the text
			if( start == 0 )
				return;
			actual.Remove( --start, 1 );
		}
	}
}


void PassControl::CutText() {

	int32 start, end;
	TextView()->GetSelection( &start, &end );
	
	// if start == end, then nothing happens
	if( start == end )
		return;

	// nuke the cut text	
	actual.Remove( start, end-start );
}
