/*
classSound.h

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

#ifndef _SOUND_H_
#define _SOUND_H_

#include <PlaySound.h>
#include <Entry.h>
#include <MediaDefs.h>

class classSound
{
	protected:
		sound_handle id;
		
		// Big bad master OK flag
		bool OK;
		
		// File stuff
		BEntry entry;
		entry_ref ref;
		// Keeps track of the multiple setting
		bool AllowMult;

	public:
		// Pass it a path to a sound file (any format that Be supports--not MIDI)
		classSound( char *, char * = "AudioFile" );
		~classSound( );
		// Allow this sound to be played more than once over itself
		void AllowMultiple( bool );
		// Actually play the sound
		void PlaySound( );
		// Check to see if the sound is playing
		bool IsPlaying( );
		// Check to see if everything is all right
		bool IsOK( );
};

#endif
