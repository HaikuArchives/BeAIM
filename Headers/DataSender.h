#ifndef _DATA_SENDER_H_
#define _DATA_SENDER_H_

#include <Window.h>
#include <ScrollView.h>
#include <Button.h>
#include <TextView.h>
#include <TextControl.h>
#include <ScrollView.h>
#include "DataContainer.h"

const uint32 INPUTDATA_INVOKED = '#$@#';
const uint32 INPUTSTRING_INVOKED = '&#32';
const uint32 CLEAR_ALL = 'cler';

class DataSenderView : public BView
{
	friend class DataSenderWindow;

	public:
		DataSenderView( BRect rect );
		
	private:

		BTextControl* inputString;
		BTextControl* inputData;
		
		BTextView* sendData;
		BScrollView* sendDataHolder;
		
		BButton* clearButton;
		BButton* okButton;
		BButton* cancelButton;
};


class DataSenderWindow : public BWindow
{
	public:
		DataSenderWindow( BRect frame );
		~DataSenderWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();

	private:
	
		void DoData( bool which );
		void DoSend();
	
		DataSenderView *genView;
		
		DataContainer data;
		bool isChar[2048];
};

#endif
