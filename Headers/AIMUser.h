#ifndef _AIM_USER_H_
#define _AIM_USER_H_

#include <Message.h>
#include <String.h>

class AIMUser {

	public:

		// constructors/destructor
		AIMUser();
		AIMUser( BString );
		AIMUser( const char* );
		AIMUser( const AIMUser& );
		~AIMUser();
		AIMUser* operator=( const AIMUser& );
		bool operator==( const AIMUser& );
		bool operator!=( const AIMUser& );
		bool operator<( const AIMUser& );
		bool operator>( const AIMUser& );
		
		bool IsEmpty();
		bool ExactCompare( const AIMUser& );
		bool IsValid();
		void SetUsername( BString );
		BString Username();
		const char* UserString();
		
		void SetSSIUserID(short);
		void SetSSIGroupID(short);
		short SSIUserID();
		short SSIGroupID();
		
	protected:
	
		int compare( const AIMUser& );
		BString userName;
		short uid;
		short gid;
};

#endif
