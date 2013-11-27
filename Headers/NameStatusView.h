#ifndef _NAME_STATUS_VIEW_H_
#define _NAME_STATUS_VIEW_H_

#include <View.h>
#include <String.h>
#include "AIMUser.h"

class NameStatusView : public BView 
{
	public:
		NameStatusView( BRect frame );
		~NameStatusView();
		void SetName( AIMUser );
		void SetWarningLevel( unsigned short );
		virtual void Draw( BRect );
		void SetAway( bool );
		void SetTempString( BString );
		
	private:
		AIMUser myName;
		BString displayString;
		BString tempString;
		unsigned short wLevel;
		bool away;
		BFont rFont, bFont;
};



#endif
