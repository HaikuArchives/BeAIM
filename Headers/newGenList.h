#ifndef _STORMYRAT_GENLIST_H_
#define _STORMYRAT_GENLIST_H_

/*
*  GenList v 3.0.0
*
*  A general purpose, template-based, doubly-linked list that can act as an array, a stack,
*      a queue, or any other kind of general purpose list
*  Greg Nichols
*  initial version September 29, 1998
*
*  Overview:
*  GenList is a truly handy little template. I wrote it because I had a lot of situations where
*  a flexible array-style thingy would come in handy, but the STL seemed overly large and complex
*  for the project I was writing. So, I got creative one afternoon, and thus was GenList born.
*  You can use it as an array, a stack, a queue, etc... they're all basically the same thing
*  anyway; just depends how you access 'em.
*
*  Performance:
*  The efficiency isn't the best in the world, but it's not bad either (and certainly not as bad
*  as it used to be!) GenList tries to be semi-intelligent in how it accesses data at a given
*  index, so for many typical uses it's quite speedy. So, you can do stuff like access the same
*  index many times in a row and do 1..x iteration as you would in a for loop without any performance
*  problems (as of version 2.5.0). However, accessing an arbitrary element is much slower in some
*  situations, as GenList has to traverse a good chunk of the list to get to the right element. 
*  For typical uses of GenList in typical code, this isn't an issue: GenList has some simple but
*  effective optimizations so that as little traversal as possible is done, and it works great.
*  But if your code uses really large lists which are accessed in an unpredictable order, then a
*  big chunk of time is going to be spent traversing the list... for better performance you may want
*  to check out other solutions (probably a tree of some sort). As this is a doubly-linked list,
*  inserting or deleting from either end is pretty darned efficient. All in all, GenList is more
*  than good enough for general purpose usage. And it's self-contained... no other source required!
*
*  Known Issues:
*  No bugs are known at this time. Insert() and Delete() both have some potential performance issues;
*  instead of usging IterateToIndex(), they always traverse the entire list (starting from the front)
*  to find the node or position needed for insertion/deletion. Ideally, they should use IterateToIndex()
*  to get this information.
*  
*  Usage and Licensing:
*  You can do whatever you want with this code, with one restriction: if you use it, you must
*  give me credit in the same source file. In other words, don't try and pretend that you wrote
*  it. Other than that, you can hack it, steal bits and pieces of it, mangle it, delete it, and
*  spam it to newsgroups... I don't care. Just be nice. :-)
*  
*  Version history:
*  1.0.0 - Sep 29, 1998 - initial release
*  2.0.0 - Apr 01, 1999 - added iteration functions: Current(), First(), Last(), Next(), Prev()
*  2.1.0 - Apr 15, 1999 - added a check to speed list access for loop-iteration type situations
*  2.1.1 - May 25, 2000 - modified operator[] to use Pos, so the current position would be saved
*                         after an array-style access. And there was much rejoicing.
*  2.1.2 - Jul 02, 2000 - after inserts/deletes, LastIndex was sometimes becoming invalid.
*  						  Added code to reset it when the list gets changed significantly.
*  3.0.0 - Aug 02, 2000 - Greatly enhanced access speed in most situations via IterateToIndex()
*						  function. Added TransferFrom() and CopyFrom() functions to get data from
*						  other GenList's of the same type. Took out PeekTop() - use Last() instead.
*/

template<class T>
class GenList {

  // structures and stuff
  struct GenNode;
  typedef GenNode* GenPoint;

  public:

    // C++ stuff
    GenList();
    ~GenList();
    T& operator[] (unsigned int pos);
    T& GetItem (unsigned int pos);		// Check size first... there will be a problem if the index is invalid

    // Common functions
    unsigned int Count()			// returns the 'cached' count
    { return CacheCount; };				
    void ForceCount();				// Normally, the add/delete functions should keep CacheCount in
    								// sync, which avoids the overhead of a full list traversal to count items.
    								// But if you feel the urge, this will do that and update CacheCount.

    void Clear( bool removeItems = true );
    bool IsEmpty() { return bool(Front == NULL); };

    // List functions
    void Add( T );
    void Insert( T, unsigned int );
    void Delete( unsigned int );

    // Stack functions
    void Push( T );				// returns true if it worked, false if it didn't
    bool Pop( T& );				// same
    bool Pop();    				// same as above, except it just throws away what it pops

    // Queue functions
    void Enqueue( T );
    bool Dequeue( T& );			// returns true if it worked, false if it didn't
    
    // Iteration functions
	bool Current( T& );			// grabs the current val, if there is one
    bool First();				// iterates to the first node
    bool First( T& );			// grabs the first val, if there is one
    bool Last();				// etc.
    bool Last( T& );
    bool Next();
    bool Next( T& );
    bool Prev();
    bool Prev( T& );

	// copy and assign functions
	void CopyFrom( const GenList<T>& OtherList );
	void TransferFrom( GenList<T>& OtherList );

  private:

	// the heavy-duty iterator function... returns true if it could iterate
	// to the requested index; otherwise returns (surprise!) false
	bool IterateToIndex( unsigned int target );

    struct GenNode {
       GenNode() { Next = NULL; Prev = NULL; };
       T Data;
       GenPoint Next;
       GenPoint Prev;
    };

    unsigned int CacheCount;
	unsigned int MidPoint;
    GenPoint Front;
    GenPoint Back;
    GenPoint Pos;
    
    // This var keeps track of the last index accessed via [] or GetItem. 
    // Helps increase access speed (hopefully) down in operator[].
    unsigned int LastIndex;
        
    // resets the two vars above so as not to run into problems after mucking w/ the list
    void ResetLastIndex() {
       LastIndex = 0;
       Pos = Front;
    };
};

//-----------------------------------------------------

template< class T >
GenList<T>::GenList() {

	Front = NULL;
	Back = NULL;
	Pos = NULL;
	CacheCount = 0;
	MidPoint = 0;
	LastIndex = 0;
}

//-----------------------------------------------------

template< class T >
GenList<T>::~GenList() {

	Clear();
}

//-----------------------------------------------------

template< class T >
void GenList<T>::Clear( bool removeItems ) {

	GenPoint DelIter = Front, bak;
	if( removeItems ) {
		while( DelIter ) {
			bak = DelIter;
			DelIter = DelIter->Next;
			delete bak;
			bak = NULL;
		}
	}

	ResetLastIndex();
	Front = NULL;
	Back = NULL;
	CacheCount = 0;
	MidPoint = 0;
}

//-----------------------------------------------------

template<class T>
void GenList<T>::ForceCount() {

   CacheCount = 0;
   GenPoint iter = Front;

   while( iter ) {
      ++CacheCount;
      iter = iter->Next;
   }

   MidPoint = CacheCount >> 1;
}

//-----------------------------------------------------

template< class T >
void GenList<T>::Enqueue( T NewThang ) {

    // EnQueue does the same thing as Add
    Add( NewThang );
}

//-----------------------------------------------------

template< class T >
bool GenList<T>::Dequeue( T& NewThang ) {

    // Anything to Dequeue?
    if( IsEmpty() )
        return false;

    GenPoint Save = Front;
    NewThang = Front->Data;

    if( Front == Back )
        Front = Back = NULL;
    else {
        Front = Front->Next;
        Front->Prev = NULL;
    }

    delete Save;
    --CacheCount;
	MidPoint = CacheCount >> 1;
    ResetLastIndex();
    return true;
}

//-----------------------------------------------------

template< class T >
void GenList<T>::Push( T NewThang ) {

    // Push does the same thing as Add
    Add( NewThang );
}

//-----------------------------------------------------

template< class T >
bool GenList<T>::Pop( T& NewThang ) {

    if( IsEmpty() )
        return false;

    GenPoint Save = Back;
    NewThang = Back->Data;

    if( Front == Back )
        Front = Back = NULL;
    else {
        Back = Back->Prev;
        Back->Next = NULL;
    }

    delete Save;
    --CacheCount;
	MidPoint = CacheCount >> 1;
    ResetLastIndex();
    return true;
}

//-----------------------------------------------------

template< class T >
bool GenList<T>::Pop() {

    if( IsEmpty() )
        return false;

    GenPoint Save = Back;

    if( Front == Back )
        Front = Back = NULL;
    else {
        Back = Back->Prev;
        Back->Next = NULL;
    }

    delete Save;
    --CacheCount;
	MidPoint = CacheCount >> 1;
    ResetLastIndex();
    return true;
}

//-----------------------------------------------------

template< class T >
void GenList<T>::Add( T NewThang ) {

   GenPoint NewNode = new GenNode;
   NewNode->Data = NewThang;

   // Is the list empty?
   if( IsEmpty() ) {
       Front = Back = NewNode;
       CacheCount = 1;
	   MidPoint = 0;
       return;
   }

   // Make this the last node
   Back->Next = NewNode;
   NewNode->Prev = Back;
   Back = NewNode;

   // Find the new count
   ++CacheCount;
   MidPoint = CacheCount >> 1;
}

//-----------------------------------------------------

template<class T>
void GenList<T>::Insert( T NewThang, unsigned int insPos ) {

   GenPoint NewNode = new GenNode;
   NewNode->Data = NewThang;
   unsigned int PosCount = 0;
   GenPoint iter = Front, bak = NULL;

   if( insPos > CacheCount )
      insPos = CacheCount;

   // Is the list empty?
   if( IsEmpty() ) {
       Front = Back = NewNode;
       CacheCount = 1;
	   MidPoint = 0;
       ResetLastIndex();
       return;
   }

   // Are we trying to make this the front node?
   if( insPos == 0 ) {
      NewNode->Next = Front;
      Front->Prev = NewNode;
      Front = NewNode;
      ++CacheCount;
	  MidPoint = CacheCount >> 1;
      ResetLastIndex();
      return;
   }

   // Find the insert position
   while( PosCount < insPos && iter ) {
      bak = iter;
      iter = iter->Next;
      ++PosCount;
   }

   // Insert the thing, baby!
   NewNode->Next = iter;
   bak->Next = NewNode;
   NewNode->Prev = bak;
   if(iter) iter->Prev = NewNode;

   // Was that the back node?
   if( bak == Back )
       Back = NewNode;

   // Correct the cached count
   ++CacheCount;
   MidPoint = CacheCount >> 1;
   ResetLastIndex();
}

//-----------------------------------------------------

template<class T>
T& GenList<T>::operator[]( unsigned int target ) {

	// bounds checking
    if( CacheCount == 0 )
        target = 0;
    else if( target >= CacheCount )
        target = CacheCount - 1;

	// go to the correct index
	IterateToIndex( target );

	// this assumes Pos isn't null, which is a bad assumption... but there's
	// not much that can be done here in the way of error handling
	return Pos->Data;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::IterateToIndex( unsigned int target ) {

	long Diff;
	short navMode = 0;

	// figure out the difference between the last index and the desired index
	Diff = target - LastIndex;
	
	// Position needs to be defined for this to work
	if( Pos == NULL )
		ResetLastIndex();

	// here's where we get tricky to try and improve access speed...
	// if the difference is more than three in either direction, try to speed
	// things up by figuring out the best starting point to navigate to that index
	// otherwise, it's (probably) faster to just go there from the current position.
	// The 4 and -4 are just a guess... need to figure out at what point the target
	// is far enough away to warrant the extra time it takes to try and get there
	// faster, as opposed to just doing it. Need to do some timed tests, methinks...
	if( Diff > 4 || Diff < -4 ) {

		// are we closer to the current position, rather than an endpoint?
		if( (unsigned)(Diff > 0 ? Diff : -Diff) < MidPoint ) {
			if( Diff > 0 )				// go forward
				navMode = 1;
			else						// go backward
				navMode = 2;
		}

		// start at one of the endpoints
		else {
			if( target <= MidPoint ) {	// start at the beginning
				navMode = 1;
				LastIndex = 0;
				Pos = Front;
			}
			else {						// start at the end
				navMode = 2;
				LastIndex = CacheCount - 1;
				Pos = Back;
			}
		}
	}

	// staying in the same place?
	else if( Diff == 0 )
		navMode = 0;

	// go forward from the current position
	else if( Diff > 0 )
		navMode = 1;

	// go backwards from the current position
	else if( Diff < 0 )
		navMode = 2;

	// now we either go forward or backwards until we 
	// find the target position or something bad happens.
	if( navMode != 0 ) {
		if( navMode == 1 ) {
			while( Pos && LastIndex < target ) {
				Pos = Pos->Next;
				++LastIndex;
			}
		} else {
			while( Pos && LastIndex > target ) {
				Pos = Pos->Prev;
				--LastIndex;
			}
		}
	}

	// return value is the success code... we were successful if 
	// we made it all the way to the requested index
	if( LastIndex == target )
		return true;
	return false;
}

//-----------------------------------------------------

template<class T>
T& GenList<T>::GetItem( unsigned int pos ) {
    return operator[]( pos );
}

//-----------------------------------------------------

template<class T>
void GenList<T>::Delete( unsigned int pos ) {

   GenPoint iter = Front, bak = NULL;
   unsigned int PosCount = 0, lCount = CacheCount;

   if( (lCount == 0) || pos > (lCount - 1) )
      return;

   // Deleting front node?
   if( pos == 0 ) {
      bak = Front;
      Front = Front->Next;
      if(Front) Front->Prev = NULL;
      delete bak;
      --CacheCount;
	  MidPoint = CacheCount >> 1;
      ResetLastIndex();
      return;
   }

   // Find the delete position
   while( iter && PosCount < pos  ) {
      bak = iter;
      iter = iter->Next;
      ++PosCount;
   }

   if( iter ) {
      bak->Next = iter->Next;
      if(iter->Next) iter->Next->Prev = bak;
   } else
      bak->Next = NULL;
   
   // Was that the back node?
   if( Back == iter )
      Back = bak;

   delete iter;

   // Correct the cached count
   --CacheCount;
   MidPoint = CacheCount >> 1;
   ResetLastIndex();
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Current( T& Data ) {

	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::First() {
	Pos = Front;
	return bool( Pos != 0 );
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::First( T& Data ) {

	Pos = Front;
	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Last() {
	Pos = Back;
	return bool( Pos != 0 );
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Last( T& Data ) {

	Pos = Back;
	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Next() {
	if( Pos ) {
		Pos = Pos->Next;
		return bool( Pos != 0 );
	} else
		return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Next( T& Data ) {

	if( Pos && Pos->Next ) {
		Pos = Pos->Next;
		Data = Pos->Data;
		return true;
	}
	return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Prev() {
	if( Pos ) {
		Pos = Pos->Prev;
		return bool( Pos != 0 );
	} else
		return false;
}

//-----------------------------------------------------

template<class T>
bool GenList<T>::Prev( T& Data ) {

	if( Pos && Pos->Prev ) {
		Pos = Pos->Prev;
		Data = Pos->Data;
		return true;
	}
	return false;
}

//-----------------------------------------------------

template<class T>
void GenList<T>::CopyFrom( const GenList<T>& OtherList ) {

	GenPoint iter = OtherList.Front;

	// clear this list out first
	Clear();

	// now, one by one, grab the items from the other
	// list and insert them in this one
	while( iter ) {
		Add( iter->Data );
		iter = iter->Next;
	}
}

//-----------------------------------------------------

template<class T>
void GenList<T>::TransferFrom( GenList<T>& OtherList ) {

	// clear ourselves out first
	Clear();

	// grab all the relevant bits from OtherList...
	Front = OtherList.Front;
	Back = OtherList.Back;
	Pos = OtherList.Pos;
	CacheCount = OtherList.CacheCount;
	MidPoint = OtherList.MidPoint;
	LastIndex = OtherList.LastIndex;

	// now, clear the all the info from OtherList (but don't delete the data)
	OtherList.Clear(false);
}

//-----------------------------------------------------

#endif
