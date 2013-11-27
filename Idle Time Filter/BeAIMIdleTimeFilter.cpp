// BeAIMIdleTimeFilter.cpp

// This file contains the source to an input server addon, which just sits there
// and figures out when the user becomes idle, and how long they stay that way.
// An input server addon may seem like overkill for this function, especially when
// there's an OS function, idle_time(), which is supposed to return that information.
// However, that function is buggy... the idle counter will reset itself in many
// situations that have nothing to do with whether the user is idle or not, making it
// basically useless. Once upon a time I reported this bug (and it was confirmed, too!)
// but Be has seemingly forgotten all about it, and over a year later, it's still
// broken. Hence this input server addon. Oh well, it was fun to write, anyway.  :-)

#include <stdlib.h>
#include <time.h>
#include <Debug.h>
#include <List.h>
#include <Message.h>
#include <Messenger.h>
#include <Locker.h>
#include <OS.h>

#include "IdleConstants.h"

//-----------------------------------------------------

#include <add-ons/input_server/InputServerFilter.h>
extern "C" _EXPORT BInputServerFilter* instantiate_input_filter();

//-----------------------------------------------------

class BeAIMIdleTimeFilter : public BInputServerFilter 
{
	public:
		BeAIMIdleTimeFilter();
		virtual ~BeAIMIdleTimeFilter();
		virtual	filter_result Filter(BMessage *message, BList *outList);

	private:
	
		void SendBeAIMMessage( BMessage* msg );
		static int32 StartTickerThread(void *arg);
		void RunTickerThread(void); 
	
		time_t lastEvent;
		BLocker lock;
		int32 oldIdleSeconds;
		thread_id ticker_thread;
};

//-----------------------------------------------------

BInputServerFilter* instantiate_input_filter()
{
	return (new BeAIMIdleTimeFilter());
}

//-----------------------------------------------------

BeAIMIdleTimeFilter::BeAIMIdleTimeFilter()
{
	oldIdleSeconds = 0;
	time(&lastEvent);
	ticker_thread = spawn_thread(StartTickerThread, "BeAIM IdleTime Detector", B_LOW_PRIORITY, this);
	resume_thread(ticker_thread);
}

//-----------------------------------------------------

BeAIMIdleTimeFilter::~BeAIMIdleTimeFilter()
{
	kill_thread(ticker_thread);
}

//-----------------------------------------------------

int32 BeAIMIdleTimeFilter::StartTickerThread( void *arg )
{
 	BeAIMIdleTimeFilter *self = (BeAIMIdleTimeFilter*)arg;
	self->RunTickerThread();
	return (B_NO_ERROR);
}

//-----------------------------------------------------

void BeAIMIdleTimeFilter::RunTickerThread()
{
	time_t now;
	BMessage* msg;
	int32 idleSeconds;
	
	while (true) {
	
		// sleep for 1 second
		snooze(1000000);
		
		// get the time and compare it w/ the last event time
		time(&now);
		lock.Lock();
		idleSeconds = now - lastEvent;
		lock.Unlock();
		
		// if we just became un-idle, sent a notification message back to BeAIM
		if( idleSeconds < 2 && oldIdleSeconds >= 60 ) {
			msg = new BMessage(BEAIM_ITF_NO_LONGER_IDLE);
			SendBeAIMMessage( msg );
		}
		
		// if we've achieved a minute mark, send that back to BeAIM too
		else if( idleSeconds && !(idleSeconds % 60) && oldIdleSeconds <= idleSeconds ) {
			msg = new BMessage(BEAIM_ITF_IDLE_PULSE);
			msg->AddInt32( "idleminutes", idleSeconds / 60 );
			SendBeAIMMessage( msg );
		}

		// save the idle time
		oldIdleSeconds = idleSeconds;		
	}
}

//-----------------------------------------------------

filter_result BeAIMIdleTimeFilter::Filter(BMessage *message, BList *outList)
{
	int32 what = message->what;

	// if anything of importance has happened, save the time of the event
	if( what == B_MODIFIERS_CHANGED || what == B_KEY_DOWN || what == B_KEY_UP ||
		what == B_MOUSE_DOWN || what == B_MOUSE_UP || what == B_MOUSE_MOVED)
	{
		lock.Lock();
		time( &lastEvent );
		lock.Unlock();
	}

	return B_DISPATCH_MESSAGE;
}

//-----------------------------------------------------

void BeAIMIdleTimeFilter::SendBeAIMMessage( BMessage* msg ) {

	status_t error;
	BMessenger sender( "application/x-vnd.FifthAce-BeAIM", -1, &error );

	// if the BMessenger got setup OK, send the message
	if( error == B_OK )
		sender.SendMessage( msg );
	
	// in any case, delete the message now
	delete msg;
}

//-----------------------------------------------------
