#ifndef _GSTATUS_VIEW_H_
#define _GSTATUS_VIEW_H_

#include <View.h>
#include "BarberPole.h"

class GStatusView : public BView 
{
	public:
		GStatusView( char* msg, BRect frame );
		~GStatusView();
		virtual void Draw( BRect );
		virtual void MouseDown( BPoint cursor );
		void SetMessage( char* msg );
		void SetSpinner( bool );
		void AttachedToWindow();
		
	private:
		BarberPole* spinner;
		char showString[500];
};


#endif
