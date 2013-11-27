#ifndef _STORMYRAT_GENLIST_H_
#define _STORMYRAT_GENLIST_H_

// GenList v 2.1

// A general purpose, template-based, doubly-linked list that can act as an array, a stack, or a queue
// Greg Nichols
// 9-29-98

// Here's the deal:
// You can do whatever you want with this code, with one restriction: if you use it, you must
// give me credit in the same source file. In other words, don't try and pretend that you wrote
// it. Other than that, you can hack it, steal bits and pieces of it, mangle it, delete it, and
// spam it to newsgroups... I don't care. It ain't an entirely pretty piece of code, and it could
// definitely use some efficency tuning, but for general purpose lists it saves lots of time.
// Enjoy.

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
    T& GetItem (unsigned int pos);	// Check size first... there will be a problem if the index is invalid
    bool PeekTop( T& );					// Peeks at the top(end) of the list, if it can

    // Common functions
    unsigned int Count()			// returns the 'cached' count
    { return CacheCount; };				
    void ForceCount();				// Normally, the add/delete functions should keep CacheCount in
    								// sync, which avoids the overhead of a full list traversal to count items.
    								// But if you feel the urge, this will do that and update CacheCount.
    void Clear();
    bool IsEmpty() { return Front == NULL; };

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

  private:

    struct GenNode {
       GenNode() { Next = NULL; Prev = NULL; };
       T Data;
       GenPoint Next;
       GenPoint Prev;
    };

    unsigned int CacheCount;
    GenPoint Front;
    GenPoint Back;
    GenPoint Pos;
    
    // These two vars keep track of the last item accessed via [] or GetItem. 
    // They increase access speed when items are being accessed sequentially,
    // as they would be in a for loop.
    unsigned int LastIndex;
    GenPoint LastIter;
};


//---------------------------------------------------------------------------

template< class T >
GenList<T>::GenList() {

	Front = NULL;
	Back = NULL;
	Pos = NULL;
	CacheCount = 0;
	LastIndex = 0;
	LastIter = Front;
}

//---------------------------------------------------------------------------

template< class T >
GenList<T>::~GenList() {

	Clear();
}

//---------------------------------------------------------------------------

template< class T >
void GenList<T>::Clear() {

   GenPoint DelIter = Front, bak;
   while( DelIter ) {
      bak = DelIter;
      DelIter = DelIter->Next;
      delete bak;
      bak = NULL;
   }

   Front = NULL;
   Back = NULL;
   CacheCount = 0;
}

//---------------------------------------------------------------------------

template<class T>
void GenList<T>::ForceCount() {

   CacheCount = 0;
   GenPoint iter = Front;

   while( iter ) {
      ++CacheCount;
      iter = iter->Next;
   }
}

//---------------------------------------------------------------------------

template< class T >
void GenList<T>::Enqueue( T NewThang ) {

    // EnQueue does the same thing as Add
    Add( NewThang );
}

//---------------------------------------------------------------------------

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
    return true;
}


//---------------------------------------------------------------------------

template< class T >
void GenList<T>::Push( T NewThang ) {

    // Push does the same thing as Add
    Add( NewThang );
}

//---------------------------------------------------------------------------

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
    return true;
}

//---------------------------------------------------------------------------

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
    return true;
}

//---------------------------------------------------------------------------

template< class T >
void GenList<T>::Add( T NewThang ) {

   GenPoint NewNode = new GenNode;
   NewNode->Data = NewThang;

   // Is the list empty?
   if( IsEmpty() ) {
       Front = Back = NewNode;
       CacheCount = 1;
       return;
   }

   // Make this the last node
   Back->Next = NewNode;
   NewNode->Prev = Back;
   Back = NewNode;

   // Find the new count
   ++CacheCount;
}

//---------------------------------------------------------------------------

template<class T>
void GenList<T>::Insert( T NewThang, unsigned int pos ) {

   GenPoint NewNode = new GenNode;
   NewNode->Data = NewThang;
   unsigned int PosCount = 0, listCount;
   GenPoint iter = Front, bak = NULL;

   listCount = CacheCount;
   if( pos > listCount )
      pos = listCount;

   // Is the list empty?
   if( IsEmpty() ) {
       Front = Back = NewNode;
       CacheCount = 1;
       return;
   }

   // Are we trying to make this the front node?
   if( pos == 0 ) {
      NewNode->Next = Front;
      Front->Prev = NewNode;
      Front = NewNode;
      ++CacheCount;
      return;
   }

   // Find the insert position
   while( PosCount < pos && iter ) {
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

   // Find the new count
   ++CacheCount;
}

//---------------------------------------------------------------------------

template<class T>
T& GenList<T>::operator[]( unsigned int pos ) {

    unsigned int theCount = 0;
    GenPoint iter = Front;

    if( CacheCount == 0 )
        pos = 0;
    else if( pos >= CacheCount )
        pos = CacheCount - 1;
    
    // check and see if sequential access will work this time
    if( pos == LastIndex+1 ) {
        ++LastIndex;
        iter = (LastIter = LastIter->Next);
    }

    // otherwise start from the beginning and keep going. Horribly inefficient.
    else while( iter && theCount < pos ) {
       iter = iter->Next;
       ++theCount;
    }

    LastIndex = pos;
    LastIter = iter;
    return iter->Data;
}

//---------------------------------------------------------------------------

template<class T>
T& GenList<T>::GetItem( unsigned int pos ) {
    return operator[]( pos );
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::PeekTop( T& Data ) {

	if( Back ) {
		Data = Back->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

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

   // Find the new count
   --CacheCount;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Current( T& Data ) {

	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::First() {
	Pos = Front;
	return bool( Pos != 0 );
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::First( T& Data ) {

	Pos = Front;
	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Last() {
	Pos = Back;
	return bool( Pos != 0 );
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Last( T& Data ) {

	Pos = Back;
	if( Pos ) {
		Data = Pos->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Next() {
	if( Pos ) {
		Pos = Pos->Next;
		return bool( Pos != 0 );
	} else
		return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Next( T& Data ) {

	if( Pos && Pos->Next ) {
		Pos = Pos->Next;
		Data = Pos->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Prev() {
	if( Pos ) {
		Pos = Pos->Prev;
		return bool( Pos != 0 );
	} else
		return false;
}

//---------------------------------------------------------------------------

template<class T>
bool GenList<T>::Prev( T& Data ) {

	if( Pos && Pos->Prev ) {
		Pos = Pos->Prev;
		Data = Pos->Data;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------

#endif
