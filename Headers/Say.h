#ifndef _SAY_H_
#define _SAY_H_

#include <Alert.h>
#include <stdio.h>
#include <String.h>
#include <string.h>
#include <Rect.h>


// String version
inline void Say( const char* string ) {
	(new BAlert("", string, "OK"))->Go();
}

// String with title version
inline void Say( const char* title, char* string ) {
	char* Msg = new char[strlen(title) + strlen(string) + 4];
	sprintf( Msg, "%s: %s", title, string );
	Say( Msg );
	delete[] Msg;
}

// Int with title version
inline void Say( const char* title, int what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %d", title, what );
	Say( Msg );
	delete[] Msg;
}

// Int without title version
inline void Say( int what ) {
	char Msg[25];
	sprintf( Msg, "%d", what );
	Say( Msg );
}

// Int32 with title version
inline void Say( char* title, int32 what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %ld", title, what );
	Say( Msg );
	delete[] Msg;
}

// Int32 without title version
inline void Say( int32 what ) {
	char Msg[25];
	sprintf( Msg, "%ld", what );
	Say( Msg );
}

// Float with title version
inline void Say( const char* title, float what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %f", title, what );
	Say( Msg );
	delete[] Msg;
}

// Float without title version
inline void Say( float what ) {
	char Msg[25];
	sprintf( Msg, "%f", what );
	Say( Msg );
}

// Char with title version
inline void Say( char* title, char what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %c", title, what );
	Say( Msg );
	delete[] Msg;
}

// Char without title version
inline void Say( const char what ) {
	char Msg[25];
	sprintf( Msg, "%c", what );
	Say( Msg );
}

// Bool with title version
inline void Say( const char* title, bool what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %s", title, what ? "true" : "false" );
	Say( Msg );
	delete[] Msg;
}

// Bool without title version
inline void Say( bool what ) {
	Say( what ? "true" : "false" );
}

// unsigned int with title version
inline void Say( const char* title, unsigned int what ) {
	char* Msg = new char[strlen(title) + 25];
	sprintf( Msg, "%s: %u", title, what );
	Say( Msg );
	delete[] Msg;
}

// unsigned int without title version
inline void Say( unsigned int what ) {
	char Msg[25];
	sprintf( Msg, "%u", what );
	Say( Msg );
}

// BRect without title version
inline void Say( BRect what ) {
	char Msg[25];
	sprintf( Msg, "{%f, %f, %f, %f}", what.left, what.top, what.right, what.bottom );
	Say( Msg );
}

// BRect with title version
inline void Say( const char* title, BRect what ) {
	char Msg[25];
	sprintf( Msg, "%s: {%f, %f, %f, %f}", title, what.left, what.top, what.right, what.bottom );
	Say( Msg );
}

// BString without title version
inline void Say( BString what ) {
	if( what.Length() ) {
		char* Msg = new char[what.Length()+1];
		sprintf( Msg, "%s\n", what.LockBuffer(0) );
		what.UnlockBuffer();
		Say( Msg );
		delete[] Msg;
	} else
		Say( "" );
}

// unsigned int with title version
inline void Say( char* title, BString what ) {
	char* Msg = new char[what.Length()+25];
	if( what.Length() ) {
		sprintf( Msg, "%s: %s", title, what.LockBuffer(0) );
		what.UnlockBuffer();
	} else
		sprintf( Msg, "%s:", title );
	Say( Msg );
	delete[] Msg;
}

#endif
