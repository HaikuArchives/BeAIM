/*
classSound.cpp

By: Sean Heber (bigzaphod@legions.com)
Come and visit a cool site:  http://www.legions.com/

You are free to use this class for anything you want.  All that I ask is that
you please credit me and/or e-mail me and let me know what you're doing with it.

Also, if you are distributing source with your application, please keep this
comment and add a note with any changes you may have made to the class and how
it works.

As is standard, I take no responsibility if this causes any damage at all.  No matter what.  Forever.

(Note that this is not being released under the GPL or anything like that, so feel
 free to use it in a commercial program if you want.  I'll be honored if you do :-)

Have fun, drink lots of Dew, and never forget THE answer: 42.
*/

#include <PlaySound.h>
#include <Entry.h>
#include <MediaDefs.h>

#include "classSound.h"

// The constructor actually loads and stores the sound file into
// memory.
classSound::classSound( char *path, char *name )
{
	OK = false;
	AllowMult = false;  // By default don't allow multiple

	entry.SetTo( path, true );

	if( entry.Exists( ) && entry.InitCheck( ) == B_OK )  // Make sure everything is good before I go any farther
		if( entry.GetRef( &ref ) == B_OK )
			OK = true;
}

// Destroy the player and things (only if it was ever made)
classSound::~classSound( )
{
	if( OK )
	{
		stop_sound( id );
	}
}

// Just plays the sound.  It also checks to make sure that it CAN and is ALLOWED to play the sound
void classSound::PlaySound( ) { 
	if( !OK || ( IsPlaying( ) && !AllowMult ) )
		return;

	id = play_sound( &ref, true, true, true );   // Actually allow Be to start playing (does NOT block!)
}

// Just check to see if the sound it playing
bool classSound::IsPlaying( )
{
	sem_info info;
	return ( OK && ( get_sem_info( id, &info ) == B_OK ) );
}

// Set the multiple flag.
void classSound::AllowMultiple( bool allow )
{
	AllowMult = allow;
}

// Just checks to make sure all is good.
bool classSound::IsOK( )
{
	return OK;
}
