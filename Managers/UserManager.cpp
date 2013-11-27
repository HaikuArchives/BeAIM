#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "AIMNetManager.h"
#include "UserManager.h"
#include "MiscStuff.h"
#include "Globals.h"
#include "constants.h"
#include "Say.h"

//=====================================================

UserManager::UserManager()
		   : BLooper("UserManager")
{
	front = NULL;	
	gIter = NULL;
	bIter = NULL;
	changes = NULL;
	commitAdds = NULL;
	commitRems = NULL;

	groupCount = 0;
	buddyCount = 0;
	
	// setup the 1-second pulse messages
	secondRunner = new BMessageRunner( this, new BMessage(BL_SECOND_PULSE), 1000000, -1 );
	
	// start 'er up
	Run();
}

//-----------------------------------------------------

UserManager::~UserManager() {
}

//-----------------------------------------------------

void UserManager::MessageReceived( BMessage* msg ) {

	switch( msg->what ) {
	
		case BEAIM_ADD_BUDDY:
			AddBuddy( AIMUser(msg->FindString("userid")),
					  BString(msg->FindString("group")),
					  msg->FindBool("commitnow") );
			break;
	
		case BEAIM_ADD_GROUP:
			AddGroup( BString(msg->FindString("group")) );
			break;
	
		case BEAIM_DELETE_BUDDY:
			RemoveBuddy( AIMUser(msg->FindString("userid")), msg->FindBool("commitnow") );
			break;
			
		case BEAIM_DELETE_GROUP:
			RemoveGroup( BString(msg->FindString("group")), msg->FindBool("commitnow") );
			break;

		case BEAIM_CHANGE_BUDDY:
			ChangeBuddy( BString(msg->FindString("oldname")), BString(msg->FindString("newname")), msg->FindBool("commitnow") );
			break;

		case BEAIM_CHANGE_GROUP:
			ChangeGroup( BString(msg->FindString("oldname")), BString(msg->FindString("newname")) );
			break;
			
		case BEAIM_BUDDYLIST_COMMIT:
			DispatchCommitList();
			break;
	
		case BL_SECOND_PULSE:
			DoChanges();
			break;
	}
}

//-----------------------------------------------------

void UserManager::Close() {
	Clear();
	Lock();
	Quit();
}

//-----------------------------------------------------

void UserManager::Clear()
{
	lock.Lock();
	GroupThang* nk1;
	GroupThang* tmp1;
	BuddyThang* nk2;
	BuddyThang* tmp2;
	ChangeThang* nk3;
	ChangeThang* tmp3;
	CommitThang* nk4;
	CommitThang* tmp4;
	
	// cycle through all the groups
	nk1 = front;
	while( nk1 ) {

		nk2 = nk1->buddyFront;
		while( nk2 ) {
		
			// move on and delete the current buddy
			tmp2 = nk2;
			nk2 = nk2->next;
			delete tmp2;
		}

		// move on and delete the current group
		tmp1 = nk1;
		nk1 = nk1->next;
		delete tmp1;
	}
	
	// delete all the pending buddy changes
	nk3 = changes;
	while( nk3 ) {
		tmp3 = nk3;
		nk3 = nk3->next;
		delete tmp3;
	}
	
	// delete all the pending commit adds
	nk4 = commitAdds;
	while( nk4 ) {
		tmp4 = nk4;
		nk4 = nk4->next;
		delete tmp4;
	}
	
	// delete all the pending commit removes
	nk4 = commitRems;
	while( nk4 ) {
		tmp4 = nk4;
		nk4 = nk4->next;
		delete tmp4;
	}
	
	// reset stuff
	front = NULL;	
	gIter = NULL;
	bIter = NULL;
	changes = NULL;
	commitAdds = NULL;
	commitRems = NULL;
	groupCount = 0;
	buddyCount = 0;
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::Print() {

	printf( "GroupCount: %d\n", groupCount );
	printf( "BuddyCount: %d\n\n", buddyCount );
	int bob = 0;

	GroupThang* stuff = front;
	while( stuff ) {
		printf( "%2d>  Group: %s   (%d members)\n", bob++, stuff->group.String(), stuff->buddyCount );
		
		int tango = 0;
		BuddyThang* bstuff = stuff->buddyFront;
		while( bstuff ) {
			printf( "         %2d>  Buddy: %s\n", tango++, bstuff->buddy.UserString() );
			bstuff = bstuff->next;
		}
		
		
		stuff = stuff->next;
	}
}

//-----------------------------------------------------

bool UserManager::IsAGroup( BString group ) {

	lock.Lock();
	GroupThang* iter = front;
	while( iter ) {

		if( iter->group == group ) {
			lock.Unlock();
			return true;
		}

		// next one...	
		iter = iter->next;
	}
	lock.Unlock();
	return false;
}

//-----------------------------------------------------

bool UserManager::AddGroup( BString group, int pos, short gid ) {

	lock.Lock();
	bool ssicommit = false;
	GroupThang* newGroup;

	printf( "AddGroup: %s\n", group.String() );

	// make sure it ain't there already...
	if( IsAGroup(group) ) {
		newGroup = FindGroupPtr(group);
		if (newGroup)
			if (gid != newGroup->groupid)
				newGroup->groupid = gid;
		lock.Unlock();
		return false;
	}

	// make the groupthang
	newGroup = new GroupThang;
	if (gid == 0)
		ssicommit = true;
	while (gid == 0) {
		srand(rand() * time(NULL) + int32(newGroup));
		gid = rand();
	}
	newGroup->group = group;
	newGroup->buddyCount = 0;
	newGroup->buddyFront = NULL;
	newGroup->groupid = gid;
	
	// tell the world
	BMessage* msg = new BMessage( BEAIM_ADD_GROUP );
	msg->AddString( "group", group.String() );
	PostAppMessage( msg );

	// attach it... if you can... mua ha ha ha ha ha ha ha!!!
	AttachGroup( newGroup, pos );
	
	if (ssicommit)
		aimnet->SSIAddGroup(group, gid);
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

void UserManager::AttachGroup( GroupThang* newGroup, int pos ) {

	lock.Lock();
	GroupThang* inserter;
	GroupThang* bak;
	int counter;

	// nothing in the list?
	if( !front ) {
		newGroup->next = NULL;
		front = newGroup;
		++groupCount;
		lock.Unlock();
		return;
	}
	
	// off the end of the list or no pos specified?
	if( pos == -1 || pos > groupCount )
		pos = groupCount;
		
	// off the front edge?
	if( pos < 0 )
		pos = 0;
		
	// find the right position to insert
	inserter = front;
	bak = NULL;
	counter = 0;
	while( inserter && counter < pos ) {
		bak = inserter;
		inserter = inserter->next;
		++counter;
	}
	
	// do the insertion
	newGroup->next = inserter;
	if( bak )
		bak->next = newGroup;
	else
		front = newGroup;
	++groupCount;
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::RemoveGroup( BString group, bool commitNow ) {

	lock.Lock();
	GroupThang* nukeGroup;
	BuddyThang* delIter;
	BuddyThang* tmp;
	BMessage* msg;
	
	printf( "RemoveGroup: %s\n", group.String() );
	
	// detach the group
	DetachGroup( group, nukeGroup );
	
	if( nukeGroup ) {

		// now go through and delete all the buddies in that group
		delIter = nukeGroup->buddyFront;
		while( delIter ) {
			aimnet->SSIRemoveUser(delIter->buddy.AsUser());
			// SSI delete user here!
			AddCommitListItem( delIter->buddy, false );
			--buddyCount;
			tmp = delIter;
			delIter = delIter->next;
			delete tmp;	
		}
	}

	// SSI delete group here!
	aimnet->SSIRemoveGroup(nukeGroup->group);

	// now do the commitment, if needed
	if( commitNow )
		DispatchCommitList();
		
	// tell the world about it
	msg = new BMessage( BEAIM_DELETE_GROUP );
	msg->AddString( "group", group.String() );
	PostAppMessage( msg );
	
	delete nukeGroup;
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::DetachGroup( BString group, GroupThang*& retGroup ) {

	lock.Lock();
	GroupThang* iter = front;
	GroupThang* bak = NULL;
	bool found = false;
	
	// be pessimistic at first...
	retGroup = NULL;
	
	// try and find the group in question
	while( iter ) {
	
		// found it?
		if( group == iter->group ) {
			found = true;
			break;
		}
		
		// go on to the next one
		bak = iter;
		iter = iter->next;
	}
	
	// bail if the group just ain't there
	if( !found ) {
		lock.Unlock();
		return;	
	}
		
	// now detach the group in question
	if( !bak )
		front = iter->next;
	else
		bak->next = iter->next;
	
	// clean up and go home...
	retGroup = iter;
	--groupCount;
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::ChangeGroup( BString from, BString to ) {

	lock.Lock();
	GroupThang* changeGroup;
	BMessage* msg;
	
	// first, make sure the target group doesn't already exist
	if( IsAGroup(to) ) {
		lock.Unlock();
		return;
	}
	
	// find the group we're supposed to change
	changeGroup = FindGroupPtr( from );
	if( !changeGroup ) {
		lock.Unlock();
		return;
	}
	
	// do the change already
	changeGroup->group = to;
	
	// tell the world about it
	msg = new BMessage(BEAIM_CHANGE_GROUP);
	msg->AddString( "oldname", from.String() );
	msg->AddString( "newname", to.String() );
	PostAppMessage(msg);
	
	lock.Unlock();
}

//-----------------------------------------------------

int UserManager::GroupPos( BString group ) {

	lock.Lock();
	int counter = 0;
	GroupThang* iter = front;
	
	while( iter ) {
		if( iter->group == group ) {
			lock.Unlock();
			return counter;
		}
		iter = iter->next;
		++counter;
	}
	
	lock.Unlock();
	return -1;
}

//-----------------------------------------------------

void UserManager::MoveGroup( BString group, int pos ) {

	lock.Lock();
	GroupThang* moveGroup = NULL;
	int thePos = GroupPos( group );
	BMessage* msg;
	
	// bail if we're moving it to the same pos, or it's not there...
	if( thePos == -1 || pos == thePos ) {
		lock.Unlock();
		return;
	}

	// detach and reattach the group in its new pos
	DetachGroup( group, moveGroup );
	if( moveGroup )
		AttachGroup( moveGroup, pos );

	printf( "> Moving %s to group pos %d\n", group.String(), pos );
	GroupThang* yak = front;
	while( yak ) {
		printf( "      %s\n", yak->group.String() );
		yak = yak->next;
	}
		
	// tell the world about it
	msg = new BMessage(BEAIM_MOVE_GROUP);
	msg->AddString( "group", group.String() );
	PostAppMessage(msg);		

	lock.Unlock();
}

//-----------------------------------------------------

bool UserManager::GetGroups( BString& group, bool first, int* count, short * gid ) {

	lock.Lock();

	// start over?
	if( first )
		gIter = front;

	// if we're at the end return false
	if( gIter == NULL ) {
		lock.Unlock();
		return false;
	}

	// otherwise move on and return true
	group = gIter->group;
	if( count )
		*count = gIter->buddyCount;
	if (gid)
		*gid = gIter->groupid;
	gIter = gIter->next;
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

bool UserManager::GetAllBuddies( AIMUser& buddy, bool first, short * uid, short * gid ) {
	
	lock.Lock();

	// no buddies, no joy
	if( !buddyCount ) {
		lock.Unlock();
		return false;
	}
		
	// starting over? look for the first group w/ buddies in it
	if( first ) {
		gIter = front;
		while( gIter && !gIter->buddyCount )
			gIter = gIter->next;
		if( gIter )
			bIter = gIter->buddyFront;
	}
	
	// return false if the buddy pointer is NULL (eg, this is the end)
	if( !bIter ) {
		lock.Unlock();
		return false;
	}

	// so now we should have a valid buddy pointer, so get the name out
	buddy = bIter->buddy.AsUser();

	if (uid)
		*uid = bIter->uid;
	if (gid)
		*gid = bIter->gid;

	// finally, attempt to get the next buddy pointer
	bIter = bIter->next;
	if( !bIter ) {
		gIter = gIter->next;
		while( gIter && !gIter->buddyCount )
			gIter = gIter->next;
		if( gIter )
			bIter = gIter->buddyFront;
	}
	
	// success! (for this buddy anyway, even if we couldn't find the next one)
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

bool UserManager::GetGroupBuddies( AIMUser& buddy, BString group, bool first, short * uid, short * gid ) {

	lock.Lock();
	GroupThang* groopie;

	// get the correct group pointer
	groopie = FindGroupPtr(group);
	if( !groopie ) {
		lock.Unlock();
		return false;
	}
		
	// no buddies, no joy
	if( !groopie->buddyCount ) {
		lock.Unlock();
		return false;
	}
		
	// starting over?
	if( first )
		bIter = groopie->buddyFront;

	// now return the buddy, eh?
	if( !bIter ) {
		lock.Unlock();
		return false;
	}
	buddy = bIter->buddy.AsUser();
	if (uid)
		*uid = bIter->uid;
	if (gid)
		*gid = bIter->gid;
	bIter = bIter->next;
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

bool UserManager::AddBuddy( AIMUser user, BString group, bool commitNow, int pos ) {

	lock.Lock();
	BuddyThang* newBuddy;
	BMessage* msg;	
	BString grp;
	short uid = 0, gid = 0;
	bool ssicommit = false;
	bool boom = false;

	// make sure it ain't there already...
	if( IsABuddy(user) ) {
		// put the trust in the most-recent update
		newBuddy = FindBuddyPtr(user);
		if (newBuddy) {
			uid = user.SSIUserID();
			gid = user.SSIGroupID();
			if ((newBuddy->uid != uid) || (newBuddy->gid != gid)) {
				newBuddy->uid = user.SSIUserID();
				newBuddy->gid = user.SSIGroupID();
			}
		}
		lock.Unlock();
		return false;
	}

	// make the groupthang
	newBuddy = new BuddyThang;
	newBuddy->buddy = AIMBuddy(user);
	newBuddy->parent = NULL;

	uid = user.SSIUserID();
	gid = user.SSIGroupID();

	if (uid == 0)
		ssicommit = true;

	while (uid == 0) {
		srand(rand() * time(NULL) + int32(newBuddy));
		uid = rand();
		user.SetSSIUserID(uid);
	}

	while (users->GetGroups(grp, boom, NULL, &gid)) {
		boom = false;
		if (group == grp)
			break;
//		else
//			gid = 0;
	}
	user.SetSSIGroupID(gid);

	newBuddy->gid = user.SSIGroupID();
	newBuddy->uid = user.SSIUserID();
	printf("SSI UserID: %d\n", user.SSIUserID());
	printf("SSI GroupID: %d\n", user.SSIGroupID());
//	if (group == "")
//		group = FindSSIGroup(user.GetSSIGroupID());

	// attach it... if you can... mua ha ha ha ha ha ha ha!!!
	AttachBuddy( newBuddy, group, pos );

	if (ssicommit)
		aimnet->SSIAddUser(user, group);
	// commit the change if requested
	AddCommitListItem( user, true );
	if( commitNow )
		DispatchCommitList();	
	// tell the world about it
	msg = new BMessage( BEAIM_ADD_BUDDY );
	msg->AddString( "userid", user.UserString() );
	msg->AddString( "group", group.String() );
	PostAppMessage( msg );
	
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

GroupThang* UserManager::FindGroupPtr( BString group ) {

	lock.Lock();
	GroupThang* findGroup = NULL;

	// try and find the group in question
	findGroup = front;
	while( findGroup ) {
		if( findGroup->group == group ) {
			lock.Unlock();
			return findGroup;
		}
		findGroup = findGroup->next;
	}
	
	lock.Unlock();
	return NULL;
}

//-----------------------------------------------------

void UserManager::AttachBuddy( BuddyThang* newBuddy, BString group, int pos ) {

	lock.Lock();
	GroupThang* findGroup = NULL;
	BuddyThang* inserter;
	BuddyThang* bak;
	int counter;

	// try and find the group in question
	if( !(findGroup = FindGroupPtr(group)) ) {
		lock.Unlock();
		return;
	}

	// nothing in the list?
	if( findGroup->buddyFront == NULL ) {
		newBuddy->next = NULL;
		newBuddy->parent = findGroup;
		findGroup->buddyFront = newBuddy;
		++findGroup->buddyCount;
		++buddyCount;
		lock.Unlock();
		return;
	}

	// off the end of the list or no pos specified?
	if( pos == -1 || pos > findGroup->buddyCount )
		pos = findGroup->buddyCount;

	// off the front edge?
	if( pos < 0 )
		pos = 0;

	// find the right position to insert
	inserter = findGroup->buddyFront;
	bak = NULL;
	counter = 0;
	while( inserter && counter < pos ) {
		bak = inserter;
		inserter = inserter->next;
		++counter;
	}

	// do the insertion
	newBuddy->next = inserter;
	newBuddy->parent = findGroup;
	if( bak )
		bak->next = newBuddy;
	else
		findGroup->buddyFront = newBuddy;
	++findGroup->buddyCount;
	++buddyCount;
	lock.Unlock();
}

//-----------------------------------------------------

bool UserManager::IsABuddy( AIMUser user, short * uid, short * gid ) {

	lock.Lock();
	AIMUser testBuddy;
	short u, g;
	
	// iterate through the list looking for an existing buddy
	bool kg = GetAllBuddies( testBuddy, true, &u, &g );
	while( kg ) {
		if( testBuddy == user ) {
			printf("%d::%d\n", u, g);
			if (uid)
				*uid = u;
			if (gid)
				*gid = g;
			lock.Unlock();
			return true;
		}
		kg = GetAllBuddies( testBuddy, false, &u, &g );
	}
	lock.Unlock();
	return false;
}

//-----------------------------------------------------

void UserManager::RemoveBuddy( AIMUser user, bool commitNow ) {

	aimnet->SSIRemoveUser(user);

	BuddyThang* nukeBuddy = NULL;
	DetachBuddy( user, nukeBuddy );
	delete nukeBuddy;
	BMessage* msg;
	
	printf( "RemoveBuddy: %s\n", user.UserString() );
	
	// tell the world about it
	msg = new BMessage( BEAIM_DELETE_BUDDY );
	msg->AddString( "userid", user.UserString() );
	PostAppMessage( msg );
	
	// commit the change if requested
	AddCommitListItem( user, false );
	if( commitNow )
		DispatchCommitList();
}

//-----------------------------------------------------

void UserManager::DetachBuddy( AIMUser user, BuddyThang*& detachedBuddy ) {

	lock.Lock();
	GroupThang* groupIter;
	BuddyThang* iter = NULL;
	BuddyThang* bak = NULL;
	bool found = false;
	
	// be pessimistic at first
	detachedBuddy = NULL;

	// look for the buddy, group, and bak pointers	
	found = FindBuddyAndGroupPtr( user, groupIter, iter, &bak );
	
	// bail if the buddy in question wasn't found
	if( !found ) {
		lock.Unlock();
		return;
	}

	// now detach the buddy and return
	detachedBuddy = iter;
	if( bak )
		bak->next = iter->next;
	else
		groupIter->buddyFront = iter->next;
	--groupIter->buddyCount;
	--buddyCount;
	
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::MoveBuddy( AIMUser user, BString group, int pos ) {

	lock.Lock();
	BuddyThang* bPtr;
	BString curGroup;
	BMessage* msg;
	int curPos;
	
	// make sure we're not moving to the same spot!
	if( !BuddyPos(user, curGroup, curPos) || (curGroup == group && curPos == pos) ) {
		lock.Unlock();
		return;
	}

	// make sure we're moving to a valid group!
	if( !IsAGroup(group) ) {
		lock.Unlock();
		return;
	}	
	
	// now detach and reattach the buddy in its new position
	DetachBuddy( user, bPtr );
	if( bPtr )
		AttachBuddy( bPtr, group, pos );
	
	// tell the world about it
	msg = new BMessage(BEAIM_MOVE_BUDDY);
	msg->AddString( "user", user.UserString() );
	PostAppMessage(msg);

	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::ChangeBuddy( AIMUser from, AIMUser to, bool commitNow ) {

	lock.Lock();
	BuddyThang* buddy;
	BMessage* msg;
	
	// first, make sure the target doesn't already exist
	if( IsABuddy(to) ) {
		lock.Unlock();
		return;
	}
	
	// now find the buddy in question
	buddy = FindBuddyPtr( from );
	if( !buddy ) {
		lock.Unlock();
		return;
	}
	
	// do the change
	buddy->buddy = AIMBuddy(to);
	AddCommitListItem( from, false );
	AddCommitListItem( to, true );
	
	// finally, commit if we're supposed to
	if( commitNow )
		DispatchCommitList();

	// tell the world about it
	msg = new BMessage(BEAIM_CHANGE_BUDDY);
	msg->AddString( "oldname", from.UserString() );
	msg->AddString( "newname", to.UserString() );
	PostAppMessage(msg);

	lock.Unlock();
}

//-----------------------------------------------------

bool UserManager::BuddyPos( AIMUser user, BString& group, int& pos ) {

	lock.Lock();
	GroupThang* gPtr;
	BuddyThang* bPtr;
	bool success = false;

	// try and get the group and buddy pointers, and the index within the group
	success = FindBuddyAndGroupPtr( user, gPtr, bPtr, NULL, &pos );
	if( !success ) {
		return false;
		lock.Unlock();
	}
		
	// assign the group name and get outta here
	group = gPtr->group;
	lock.Unlock();
	return true;
}

//-----------------------------------------------------

int UserManager::GetBuddyStatus( AIMUser buddy ) {

	lock.Lock();
	int ret = -1;
	BuddyThang* bPtr;
	
	// try and find the buddy pointer
	bPtr = FindBuddyPtr(buddy);
	if( bPtr == NULL ) {
		lock.Unlock();
		return -1;
	}
	
	// return the status
	ret = bPtr->buddy.Status();	
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

bool UserManager::FindBuddyAndGroupPtr( AIMUser user, GroupThang*& gPtr, BuddyThang*& bPtr,
								BuddyThang** bakPtr, int* cnt )
{
	lock.Lock();
	BuddyThang* bak = NULL;
	int count = 0;
	bool found = false;

	// navigate through the groups
	gPtr = front;
	while( gPtr ) {
	
		// go through and look for the right buddy pointer
		bak = NULL;
		bPtr = gPtr->buddyFront;
		count = 0;
		while( bPtr ) {
		
			// break outta the loop if this is the right buddy
			if( user == bPtr->buddy.AsUser() ) {
				found = true;
				break;
			}
		
			// move on...
			bak = bPtr;
			bPtr = bPtr->next;
			++count;
		}
		
		// don't go on unless we have to
		if( found )
			break;

		// next group	
		gPtr = gPtr->next;
	}
	
	// if we found stuff, set any variables that need it
	if( found ) {
		if( bakPtr )
			*bakPtr = bak;
		if( cnt )
			*cnt = count;
	}
	
	lock.Unlock();
	return found;
}

//-----------------------------------------------------

BuddyThang* UserManager::FindBuddyPtr( AIMUser user ) {

	lock.Lock();
	BuddyThang* buddy;
	GroupThang* group;
	
	if( !(FindBuddyAndGroupPtr(user,group,buddy)) )
		buddy = NULL;
	lock.Unlock();
	return buddy;
}

//-----------------------------------------------------

void UserManager::SetupChange( AIMUser target ) {
//	int CHANGE_TIME = 3;

	lock.Lock();
	ChangeThang* iter;	
	ChangeThang* bak;
	ChangeThang* newRec;

	// look and see if this user is due to change, if so, use that record
	iter = changes;
	bak = NULL;
	while( iter ) {
		if( iter->target == target ) {
			printf( "     (using existing record)\n" );
			iter->counter = CHANGE_TIME;
			lock.Unlock();
			return;
		}
		bak = iter;
		iter = iter->next;
	}

	// guess not... make add a new record
	newRec = new ChangeThang;
	newRec->target = target;
	newRec->counter = CHANGE_TIME;
	newRec->next = NULL;
	
	// add it
	if( bak )
		bak->next = newRec;
	else
		changes = newRec;
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::DoChanges() {

	lock.Lock();
	ChangeThang* iter;
	ChangeThang* bak;
	BuddyThang* buddy;
	BMessage* sndMessage;

	// go through all the queued-up changes	
	bak = NULL;
	iter = changes;
	while( iter ) {
	
		// decrement the counter thang
		--iter->counter;
		
		// ding! time's up
		if( iter->counter == 0 ) {
			
			// now update that buddy's status (if the pointer can be found)
			buddy = FindBuddyPtr( iter->target );
			if( buddy ) {
				buddy->buddy.SetStatus( -1, -1, BS_NONE );
				sndMessage = new BMessage(BEAIM_BUDDY_STATUS_CHANGE);
				sndMessage->AddString( "userid", buddy->buddy.UserString() );
				sndMessage->AddInt32( "status", buddy->buddy.Status() );
				sndMessage->AddInt32( "warninglevel", buddy->buddy.WarningLevel() );
				PostAppMessage( sndMessage );		
			}
			
			// now remove that change request from the list
			(bak ? bak->next : changes) = iter->next;
		}
	
		// next change!
		bak = iter;
		iter = iter->next;
	}	
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::AddCommitListItem( AIMUser buddy, bool add ) {

	lock.Lock();
	CommitThang* iter;
	CommitThang* bak;
	CommitThang* newCommitment;
	
	// make a new commit item
	newCommitment = new CommitThang;
	newCommitment->buddy = buddy;
	newCommitment->next = NULL;
	
	// go to the end of whichever commit list we're working with
	iter = add ? commitAdds : commitRems;
	bak = NULL;
	while( iter ) {
		bak = iter;
		iter = iter->next;
	}
	
	// now add it
	if( bak )
		bak->next = newCommitment;
	else
		(add ? commitAdds : commitRems) = newCommitment;
	
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::DispatchCommitList() {

	lock.Lock();
	CommitThang* aIter;
	CommitThang* rIter;
	CommitThang* bak;
	bool conflicted;
	AIMUser addBuddy;
	BString addString, remString, tempString;
	int32 tempLength;
	BMessage* msg;
	
	// cycle through all the names on the add list
	aIter = commitAdds;
	while( aIter ) {
	
		addBuddy = aIter->buddy;
		conflicted = false;
		
		// go through and make sure this name isn't on the remove list, too...
		// if so, nuke it from the remove list and don't send it w/ the add list,
		// since an add and a remove cancel each other out	
		rIter = commitRems;
		bak = NULL;
		while( rIter ) {

			// if the addUser is also on the remove list, remove it
			if( rIter->buddy == addBuddy ) {
				(bak ? bak->next : commitRems) = rIter->next;
				conflicted = true;
				break;
			}
		
			bak = rIter;
			rIter = rIter->next;
		}
		
		// only send it w/ the add list if it wasn't on the remove list
		if( !conflicted ) {
			tempString = addBuddy.UserString();
			tempString.Append( char(9), 1 );
			if( addString.FindFirst(tempString) == B_ERROR )
				addString.Append( tempString );
		}
	
		// check out the next item
		aIter = aIter->next;
	}
	
	// add all the names on the remove list to the remove string... we don't have
	// to check for conflicts here, since all the duplicated names got deleted from
	// this list during the last step
	rIter = commitRems;
	while( rIter ) {
		tempString = rIter->buddy.UserString();
		tempString.Append( char(9), 1 );
		if( remString.FindFirst(tempString) == B_ERROR )
			remString.Append( tempString );
		rIter = rIter->next;
	}
	
	// remove the trailing tabs and send 'em down
	if( (tempLength = addString.Length()) ) {
		addString = addString.Truncate( tempLength - 1 );
		msg = new BMessage(BEAIM_BUDDYLIST_COMMIT);
		msg->AddString( "list", addString.String() );
		msg->AddBool( "add", true );
		PostAppMessage( msg );
		printf( "COMMIT ADDS   : %s\n", addString.String() );
	}
	if( (tempLength = remString.Length()) ) {
		remString = remString.Truncate( tempLength - 1 );
		msg = new BMessage(BEAIM_BUDDYLIST_COMMIT);
		msg->AddString( "list", remString.String() );
		msg->AddBool( "add", false );
		PostAppMessage( msg );
		printf( "COMMIT REMOVES: %s\n", remString.String() );
	}
	
	// now clear it all out
	ClearCommitList();
	
	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::ClearCommitList() {

	lock.Lock();
	CommitThang* iter;
	CommitThang* tmp;
	
	// nuke the add list
	iter = commitAdds;
	while( iter ) {
		tmp = iter;
		iter = iter->next;
		delete tmp;
	}
	
	// nuke the remove list
	iter = commitRems;
	while( iter ) {
		tmp = iter;
		iter = iter->next;
		delete tmp;
	}

	// now null 'em out
	commitAdds = NULL;
	commitRems = NULL;
	lock.Unlock();
}

//-----------------------------------------------------

int UserManager::CountGroups() {
	return groupCount;
}

//-----------------------------------------------------

int UserManager::CountBuddies() {
	return buddyCount;
}

//-----------------------------------------------------

int UserManager::GetGroupCount( BString group ) {

	lock.Lock();
	GroupThang* gr00p;
	int ret;

	// try to find the group in question
	gr00p = FindGroupPtr( group );
	if( !gr00p ) {
		lock.Unlock();
		return -1;
	}

	// return the group count
	ret = gr00p->buddyCount;
	lock.Unlock();
	return ret;
}

//-----------------------------------------------------

bool UserManager::FindOnlineInsertionPoint( AIMUser user, BString& group, int& iPoint ) {

	lock.Lock();
	GroupThang* gPtr;
	BuddyThang* bPtr;
	
//	printf( "Finding insertion point for: %s\n", user.UserString() );
	
	// get the pointer and group, if possible
	if( !(FindBuddyAndGroupPtr(user, gPtr, bPtr)) ) {
		lock.Unlock();
		return false;
	}
	
	// get the group name all squared away
	group = gPtr->group;
	
//	printf( "    group = %s\n", group.String() );
	
	// find the correct insertion point
	iPoint = 0;
	bPtr = gPtr->buddyFront;
	while( bPtr ) {
		if( bPtr->buddy.AsUser() == user )
			break;
		//if( bPtr->buddy.Status() != BS_OFFLINE )
		if( bPtr->buddy.Status() != BS_OFFLINE && bPtr->buddy.GotUpdated() )
			++iPoint;
//		printf( "   iterating past: %s   (%s, %s)\n", bPtr->buddy.UserString(), bPtr->buddy.Status() == BS_OFFLINE ? "offline" : "not offline", bPtr->buddy.GotUpdated() ? "updated" : "not updated" );
		bPtr = bPtr->next;
	}
	
//	printf( "Final ipoint = %d\n", iPoint );

	lock.Unlock();
	return true;
}

//-----------------------------------------------------

bool UserManager::FindOfflineInsertionPoint( AIMUser user, int& iPoint ) {

	lock.Lock();
	GroupThang* gIter;
	BuddyThang* bIter;
	iPoint = 0;

	// no buddies, no joy
	if( !front ) {
		lock.Unlock();
		return false;
	}

	// look for the buddy in question, incrementing along the way. Yay!
	gIter = front;
	while( gIter ) {
	
		// don't bother w/ this stuff unless there are actually buddies in there...
		bIter = gIter->buddyFront;
		if( gIter->buddyCount )
		{
			// look through all the buddies in the current group	
			while( bIter ) {

				// eureka! found it.
				if( bIter->buddy.AsUser() == user ) {
					lock.Unlock();
					return true;
				}
			
				// increment and get the next buddy pointer
				bIter = bIter->next;
				++iPoint;
			}
		}
		
		// get the next group
		gIter = gIter->next;
	}
	
	// if the name was in the list, we wouldn't have made it this far...
	lock.Unlock();
	return false;
}

//-----------------------------------------------------

void UserManager::SetBuddyStatus( BMessage* msg ) {

	lock.Lock();
	BuddyThang* buddyPtr;
	AIMBuddy* theBuddy;
	BMessage* sndMessage;
	bool online;
	AIMUser user;
	int mainStatus;
	bool away;
	int32 warningLevel;
	
	printf( "[**] set buddy status\n" );
	
	// get the userid for which this is intended and set some defaults
	user = AIMUser(msg->FindString("userid"));
	mainStatus = BS_ACTIVE;
	away = false;
	warningLevel = 0;

	// either an oncoming buddy or a status update...
	if( msg->what == BEAIM_ONCOMING_BUDDY ) {

		// now get the *real* info from the message
		if( unsigned(msg->FindInt32("idletime")) >= 10 )
			mainStatus = BS_IDLE;
		if( bool(msg->FindBool("away")) == true )
			away = true;
		if( msg->HasInt32("warninglevel") )
			warningLevel = msg->FindInt32("warninglevel");
	}

	// an offgoing buddy, no doubt
	else mainStatus = BS_OFFLINE;

	// try and find the buddy in question
	buddyPtr = FindBuddyPtr(user);
	if( !buddyPtr ) {
		lock.Unlock();
		return;	
	}

	// get a pointer to the actual buddy object
	theBuddy = &buddyPtr->buddy;
	online = !(theBuddy->Status() & BS_OFFLINE);
	SetBuddyWarningLevel( user.UserString(), warningLevel );

	// time to muck about with time stuff...
	uint32 rtc = real_time_clock();
	theBuddy->SetLoginTime( rtc - msg->FindInt32("sessionlen") );
	if( mainStatus == BS_IDLE )
		theBuddy->SetIdleStartTime( rtc - 60*msg->FindInt32("idletime") );

	// coming online?
	if( !online && mainStatus != BS_OFFLINE && mainStatus != -1 ) {
		theBuddy->SetStatus( mainStatus, away ? BS_AWAY : -1, BS_ENTERING );
		theBuddy->SetGotUpdated( false );
		SetupChange( user );
		sounds->PlaySound( WS_ENTER );
	}

	// going offline?
	else if( online && mainStatus == BS_OFFLINE ) {
		theBuddy->SetStatus( mainStatus, 0, BS_LEAVING );
		theBuddy->SetGotUpdated( false );
		SetupChange( user );
		sounds->PlaySound( WS_EXIT );
	}
	
	// otherwise, it's just a small status update (idle, not idle, warned, etc).
	else
		theBuddy->SetStatus( mainStatus, away ? BS_AWAY : 0, -1 );
	
	// make a BMessage, fill it with some exciting goodies, and send it out
	sndMessage = new BMessage(BEAIM_BUDDY_STATUS_CHANGE);
	sndMessage->AddString( "userid", user.UserString() );
	sndMessage->AddInt32( "status", (int32)theBuddy->Status() );
	sndMessage->AddInt32( "warninglevel", warningLevel );
	PostAppMessage( sndMessage );

	lock.Unlock();
}

//-----------------------------------------------------

void UserManager::BuddyDisplayGotUpdated( AIMUser user )
{
	lock.Lock();
	BuddyThang* buddy;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return;
	}
	
	// do the change
	buddy->buddy.SetGotUpdated( true );	
	lock.Unlock();
}

//-----------------------------------------------------

int32 UserManager::GetBuddyWarningLevel( AIMUser user )
{
	lock.Lock();
	BuddyThang* buddy;
	int32 retVal;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return -1;
	}
	
	// do the change
	retVal = buddy->buddy.WarningLevel();	
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

void UserManager::SetBuddyWarningLevel( AIMUser user, int32 wLevel )
{
	lock.Lock();
	BuddyThang* buddy;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return;
	}
	
	// do the change
	buddy->buddy.SetWarningLevel( wLevel );
	lock.Unlock();
}

//-----------------------------------------------------

uint32 UserManager::GetBuddyEncoding( AIMUser user )
{
	lock.Lock();
	BuddyThang* buddy;
	uint32 retVal;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return DEFAULT_ENCODING_CONSTANT;
	}
	
	// do the change
	retVal = buddy->buddy.Encoding();	
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

void UserManager::SetBuddyEncoding( AIMUser user, uint32 enc )
{
	lock.Lock();
	BuddyThang* buddy;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return;
	}
	
	// do the change
	buddy->buddy.SetEncoding( enc );
	lock.Unlock();
}

//-----------------------------------------------------

uint32 UserManager::GetBuddyLoginTime( AIMUser user )
{
	lock.Lock();
	BuddyThang* buddy;
	uint32 retVal;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return 0;
	}
	
	// do the change
	retVal = buddy->buddy.GetLoginTime();	
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

uint32 UserManager::GetBuddyIdleStartTime( AIMUser user )
{
	lock.Lock();
	BuddyThang* buddy;
	uint32 retVal;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( !buddy ) {
		lock.Unlock();
		return 0;
	}
	
	// do the change
	retVal = buddy->buddy.GetIdleStartTime();	
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

bool UserManager::GetGroupOfBuddy( AIMUser user, BString& group )
{
	lock.Lock();
	BuddyThang* buddy;
	bool retVal = false;

	// now find the buddy in question
	buddy = FindBuddyPtr( user );
	if( buddy ) {
		retVal = true;
		group = buddy->parent->group;
	}
	
	lock.Unlock();
	return retVal;
}

//-----------------------------------------------------

void UserManager::BlockUser( AIMUser user, bool report )
{
	lock.Lock();
	
	// make sure the user isn't already blocked
	if( IsUserBlocked(user) ) {
		lock.Unlock();
		return;
	}

	// add the new blocked user
	blockedUsers.Add( user );

	// do some extra processing if this is a buddy we're blocking...
	if( IsABuddy(user) )
	{
		// try and find the buddy in question
		BuddyThang* buddyPtr = FindBuddyPtr(user);
		if( !buddyPtr ) {
			lock.Unlock();
			return;	
		}
		
		// set that buddy's status to offline
		buddyPtr->buddy.SetStatus( BS_OFFLINE, BS_NONE, BS_NONE );
	}

	lock.Unlock();
	
	// fire off a message to change the status
	if( report ) {
		BMessage* blockMsg = new BMessage(BEAIM_SET_USER_BLOCKINESS);
		blockMsg->AddString( "userid", user.UserString() );
		blockMsg->AddBool( "block", true );
		blockMsg->AddBool( "alreadydone", true );
		blockMsg->AddInt32( "wtype", USER_OTHER_INFO );
		windows->BroadcastMessage( blockMsg, true );
	}
}

//-----------------------------------------------------

void UserManager::UnblockUser( AIMUser user )
{
	lock.Lock();
	bool found = false;

	// find the user and delete it from the list (if it's there at all)
	for( unsigned i = 0; i < blockedUsers.Count(); ++i ) {
		if( blockedUsers[i] == user ) {
			blockedUsers.Delete(i);
			found = true;
			break;
		}	
	}

	lock.Unlock();
	
	if( !found )
		return;

	// fire off a message to change the status
	BMessage* blockMsg = new BMessage(BEAIM_SET_USER_BLOCKINESS);
	blockMsg->AddString( "userid", user.UserString() );
	blockMsg->AddBool( "block", false );
	blockMsg->AddBool( "alreadydone", true );
	blockMsg->AddInt32( "wtype", USER_OTHER_INFO );
	windows->ForwardIncomingMessage( blockMsg );
	windows->SendSingleWindowMessage( blockMsg, SW_BLOCKLIST_EDITOR );
	PostAppMessage( blockMsg );
}
		
//-----------------------------------------------------
		
bool UserManager::IsUserBlocked( AIMUser user )
{
	lock.Lock();
	bool retVal = false;

	for( unsigned i = 0; i < blockedUsers.Count(); ++i ) {
		if( blockedUsers[i] == user ) {
			retVal = true;
			break;
		}
	}

	lock.Unlock();
	return retVal;
}
		
//-----------------------------------------------------
		
bool UserManager::GetNextBlockedUser( AIMUser& user, bool first )
{
	lock.Lock();
	bool retVal = false;
	
	// starting over?
	if( first )
		blockedUsers.First();
		
	// grab the current item, if possible
	retVal = blockedUsers.Current(user);
	if( retVal )
		blockedUsers.Next();

	lock.Unlock();
	return retVal;
}

bool UserManager::GetBuddySSIInfo (AIMUser user, short & uid, short & gid)
{
	BuddyThang * thang;
	thang = FindBuddyPtr(user);
	if (!thang)
		return false;
	uid = thang->uid;
	gid = thang->gid;
	return true;
}

bool UserManager::GetGroupSSIInfo (BString group, short & gid)
{
	GroupThang * thang;
	thang = FindGroupPtr(group);
	if (!thang)
		return false;
	gid = thang->groupid;
	return true;
}

//=====================================================

AIMBuddy::AIMBuddy()
		: AIMUser()
{
	mainStatus = BS_OFFLINE;
	modStatus = BS_NONE;
	tempStatus = BS_NONE;
	warningLevel = 0;
	gotUpdated = true;
	loginTime = 0;
	idleStartTime = 0;
	encoding = DEFAULT_ENCODING_CONSTANT;
	uid = gid = 0;
}

//-----------------------------------------------------

AIMBuddy::AIMBuddy( BString user )
		: AIMUser(user)
{
	mainStatus = BS_OFFLINE;
	modStatus = BS_NONE;
	tempStatus = BS_NONE;
	warningLevel = 0;
	gotUpdated = true;
	loginTime = 0;
	idleStartTime = 0;
	encoding = DEFAULT_ENCODING_CONSTANT;
	uid = gid = 0;
}

//-----------------------------------------------------

AIMBuddy::AIMBuddy( const AIMUser& id )
		: AIMUser(id)
{
	mainStatus = BS_OFFLINE;
	modStatus = BS_NONE;
	tempStatus = BS_NONE;
	warningLevel = 0;
	gotUpdated = true;
	loginTime = 0;
	idleStartTime = 0;
	encoding = DEFAULT_ENCODING_CONSTANT;
	uid = AIMUser(id).SSIUserID();
	gid = AIMUser(id).SSIGroupID();
}

//-----------------------------------------------------

AIMBuddy::AIMBuddy( const AIMBuddy& id )
		: AIMUser()
{
	SetTo( id );
	uid = gid = 0;
}

//-----------------------------------------------------

AIMBuddy::~AIMBuddy() {
}

//-----------------------------------------------------

AIMUser AIMBuddy::AsUser() {

	AIMUser user;
	user.SetUsername(userName);
	user.SetSSIUserID(uid);
	user.SetSSIGroupID(gid);
	return user;
}

//-----------------------------------------------------

void AIMBuddy::SetStatus( int mainStat, int modStat, int tempStat ) {

	if( mainStat != -1 )
		mainStatus = mainStat;
	if( modStat != -1 )
		modStatus = modStat;
	if( tempStat != -1 )
		tempStatus = tempStat;
}

//-----------------------------------------------------

AIMBuddy* AIMBuddy::operator=( const AIMBuddy& id ) {		
	SetTo( id );
	return this;
}

//-----------------------------------------------------

void AIMBuddy::SetTo( const AIMBuddy& id ) {

	SetUsername( id.userName );

	mainStatus = id.mainStatus;
	modStatus = id.modStatus;
	tempStatus = id.tempStatus;
	warningLevel = id.warningLevel;
	blocked = id.blocked;
	capsMask = id.capsMask;
	gotUpdated = true;
	uid = id.uid;
	gid = id.gid;
}

//-----------------------------------------------------

int32 AIMBuddy::WarningLevel() {
	return warningLevel;
}

//-----------------------------------------------------

void AIMBuddy::SetWarningLevel( int32 wLevel ) {
	warningLevel = wLevel;
}

//-----------------------------------------------------

uint32 AIMBuddy::GetLoginTime() {
	return loginTime;
}
		
//-----------------------------------------------------
		
uint32 AIMBuddy::GetIdleStartTime() {
	return idleStartTime;
}

//-----------------------------------------------------		

void AIMBuddy::SetLoginTime( uint32 lt ) {
	loginTime = lt;
}
		
//-----------------------------------------------------
		
void AIMBuddy::SetIdleStartTime( uint32 ist ) {
	idleStartTime = ist;
}
		
//-----------------------------------------------------

uint32 AIMBuddy::Encoding() {
	return encoding;
}

//-----------------------------------------------------

void AIMBuddy::SetEncoding( uint32 enc ) {
	encoding = enc;
}

//-----------------------------------------------------
