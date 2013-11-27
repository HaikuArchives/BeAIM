#ifndef _USER_MANAGER_H_
#define _USER_MANAGER_H_

#include <String.h>
#include <Looper.h>
#include <Locker.h>
#include <MessageRunner.h>
#include <stdio.h>
#include "GenList.h"
#include "AIMUser.h"

//-----------------------------------------------------

const uint32 BL_SECOND_PULSE = 'blSP';
const int CHANGE_TIME = 15;

//-----------------------------------------------------

struct BuddyThang;
struct GroupThang;
struct ChangeThang;

//-----------------------------------------------------

class AIMBuddy : public AIMUser {

	public:
		AIMBuddy();
		AIMBuddy( BString );
		AIMBuddy( const AIMUser& );
		AIMBuddy( const AIMBuddy& );
		~AIMBuddy();
		
		AIMBuddy* operator=( const AIMBuddy& );
		
		AIMUser AsUser();
		
		void SetStatus( int mainStat = -1, int modStat = -1, int tempStat = -1 );
		int Status()
			{ return mainStatus | modStatus | tempStatus; };
			
		int32 WarningLevel();
		void SetWarningLevel( int32 );
		uint32 Encoding();
		void SetEncoding( uint32 );
		int32 CapabilitiesMask();
		void SetCapabilitiesMask( int32 );
		bool IsBlocked();
		void SetBlocked( bool );
		void SetGotUpdated( bool gu ) { gotUpdated = gu; };
		bool GotUpdated() { return gotUpdated; };
		
		uint32 GetLoginTime();
		uint32 GetIdleStartTime();
		void SetLoginTime( uint32 );
		void SetIdleStartTime( uint32 );
		
		void SetTo( const AIMBuddy& );
		
	private:
		BString groupName;
		int mainStatus;
		int modStatus;
		int tempStatus;
		int32 warningLevel;
		int32 capsMask;
		bool blocked;
		bool gotUpdated;
		uint32 loginTime;
		uint32 idleStartTime;
		uint32 encoding;
		short uid;
		short gid;
};

//-----------------------------------------------------

struct BuddyThang {
	AIMBuddy buddy;
	BuddyThang* next;
	GroupThang* parent;
	short uid;
	short gid;
};

//-----------------------------------------------------

struct GroupThang {
	BString	group;
	BuddyThang* buddyFront;
	GroupThang* next;
	short groupid;
	int buddyCount;
};

//-----------------------------------------------------

struct ChangeThang {
	AIMUser target;
	int counter;
	ChangeThang* next;
};

//-----------------------------------------------------

struct CommitThang {
	AIMUser buddy;
	CommitThang* next;
};

//-----------------------------------------------------

class UserManager : public BLooper {


	public:
		UserManager();
		~UserManager();
		
		// this is a looper, after all...
		void MessageReceived( BMessage* msg );

		// group functions
		bool AddGroup( BString group, int pos = -1, short gid = 0 );
		bool IsAGroup( BString group );
		void RemoveGroup( BString group, bool commitNow = true );
		void MoveGroup( BString group, int pos );
		int  GroupPos( BString group );
		void ChangeGroup( BString from, BString to );
		
		// buddy functions
		bool AddBuddy( AIMUser user, BString group = "", bool commitNow = true, int pos = -1 );
		bool IsABuddy( AIMUser user, short * uid = NULL, short * gid = NULL );
		void RemoveBuddy( AIMUser user, bool commitNow = true );
		void MoveBuddy( AIMUser user, BString group, int pos = -1 );
		bool BuddyPos( AIMUser user, BString& group, int& pos );
		void ChangeBuddy( AIMUser from, AIMUser to, bool commitNow = true );
		
		// buddy get/set info functions
		void SetBuddyStatus( BMessage* msg );
		int GetBuddyStatus( AIMUser user );
		bool GetGroupOfBuddy( AIMUser, BString& group );
		int32 GetBuddyWarningLevel( AIMUser );
		void SetBuddyWarningLevel( AIMUser, int32 );
		uint32 GetBuddyLoginTime( AIMUser );
		uint32 GetBuddyIdleStartTime( AIMUser );
		uint32 GetBuddyEncoding( AIMUser );
		bool GetBuddySSIInfo(AIMUser, short &, short &);
		bool GetGroupSSIInfo(BString, short &);
		void SetBuddyEncoding( AIMUser, uint32 );
		
		// iteration functions
		bool GetGroups( BString& group, bool first, int* groupCount = NULL, short * gid = NULL );
		bool GetGroupBuddies( AIMUser& user, BString group, bool first, short * uid = NULL, short * gid = NULL );
		bool GetAllBuddies( AIMUser& user, bool first, short * uid = NULL, short * gid = NULL );

		// functions that let the buddy list know where to put the buddies
		bool FindOnlineInsertionPoint( AIMUser user, BString& group, int& iPoint );
		bool FindOfflineInsertionPoint( AIMUser user, int& iPoint );
		void BuddyDisplayGotUpdated( AIMUser user );

		// user blocking functions
		void BlockUser( AIMUser, bool report = true );
		void UnblockUser( AIMUser );
		bool IsUserBlocked( AIMUser );
		bool GetNextBlockedUser( AIMUser&, bool );

		// commit list functions
		void ClearCommitList();
		void DispatchCommitList();

		// count functions
		int CountGroups();
		int CountBuddies();
		int GetGroupCount( BString );

		// miscellaneous stuff
		void Print();
		void Clear();
		void Close();
	
	private:

		// does the actual work of add/remove/move
		void AttachGroup( GroupThang*, int pos );
		void DetachGroup( BString, GroupThang*& );
		void AttachBuddy( BuddyThang*, BString, int pos );
		void DetachBuddy( AIMUser, BuddyThang*& );

		// searches within the list
		GroupThang* FindGroupPtr( BString group );
		BuddyThang* FindBuddyPtr( AIMUser user );
		bool FindBuddyAndGroupPtr( AIMUser user, GroupThang*&, BuddyThang*&, BuddyThang** = NULL, int* = NULL );
		
		// handles the periodic changes in status (oncoming, offgoing, etc)
		void SetupChange( AIMUser target );
		void DoChanges();
		
		// another commit list function... this one is private though
		void AddCommitListItem( AIMUser name, bool add );
	
		BLocker lock;
		int groupCount;
		int buddyCount;
		
		BMessageRunner* secondRunner;
		
		GroupThang* front;
		GroupThang* gIter;
		BuddyThang* bIter;
		ChangeThang* changes;
		CommitThang* commitAdds;
		CommitThang* commitRems;
		GenList<AIMUser> blockedUsers;
};

//-----------------------------------------------------

#endif
