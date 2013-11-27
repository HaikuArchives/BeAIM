#include "myFilter.h"
#include "PassControl.h"
#include "Say.h"


myFilter::myFilter()
		: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN, NULL)
{
}

filter_result myFilter::Filter(BMessage *message, BHandler **target)
{
	int32 mods;

	int8 byte;
	message->FindInt8("byte", &byte);

	// handle the "cut" command specially...
	if (!message->FindInt32("modifiers", &mods) && (mods & B_COMMAND_KEY)){
		if( byte == 'x' )
			((PassControl *)((BView *)*target)->Parent())->CutText();
		return B_DISPATCH_MESSAGE;
	}

	// Let the control handle all other meta keys
	if (!message->FindInt32("modifiers", &mods) && (mods & B_COMMAND_KEY))
		return B_DISPATCH_MESSAGE;
	

	switch(byte)
	{
		case B_DELETE:
			((PassControl *)((BView *)*target)->Parent())->DeleteText( true );
			return B_DISPATCH_MESSAGE;

		case B_BACKSPACE:
		{
			((PassControl *)((BView *)*target)->Parent())->DeleteText( false );	
			return B_DISPATCH_MESSAGE;
		}

		case B_ENTER:
			return B_SKIP_MESSAGE;
		case B_TAB:
			*target = ((BView *)*target)->Parent(); // send it to the parent bview so you
													// can navigate
			return B_DISPATCH_MESSAGE;
		default:
			if (byte < B_SPACE)
				return B_DISPATCH_MESSAGE;
			((PassControl *)((BView *)*target)->Parent())->InsertText(message); // ditto
			type_code type;
			int32 count;
			message->GetInfo("byte", &type, &count);

#if 0
			// Just a little experimentation with the bullet character instead of asterisk
			// Of course, the bullet looks pretty pitiful in some fonts.
			message->ReplaceInt8("byte", 0, 0xC2);	// This should always work, since there's always a byte.
			
			if (count > 1)
				message->ReplaceInt8("byte", 1, 0xB7);
			else
			{
				count++;
				message->AddInt8("byte", 0xB7);
			}

			// If it's a big multi-byte, strip off the (now extraneous) trailing byte(s).
			while (count-- > 2)
				message->RemoveData("byte", count);
			
#else
			message->ReplaceInt8("byte", 42); // 42? heh. this is the ascii code for "*". 
												// funny how it really _was_ the answer here

			// If it's multi-byte, strip off the (now extraneous) trailing bytes.
			while (count-- > 1)
				message->RemoveData("byte", count);
#endif
			return B_DISPATCH_MESSAGE;
	}
}


