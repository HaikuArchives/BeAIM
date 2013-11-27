#ifndef _GENERIC_INPUT_H
#define _GENERIC_INPUT_H

#include <Invoker.h>
#include <Window.h>
#include <Button.h>
#include <TextControl.h>
#include <StringView.h>

class GenericInputView : public BView
{
	friend class GenericInputWindow;

	public:
		GenericInputView( BRect rect );
};


class GenericInputWindow : public BWindow, public BInvoker
{
	public:
		GenericInputWindow( BRect frame, char* title, char* label, BMessage* = NULL, BWindow* = NULL, char* initial = NULL,
							int maxLen = 0, char* fname = "value", char* OK = "--[iOK]--", char* cancel = "--[iCANCEL]--" );
		~GenericInputWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
		
		virtual void MakeExtraControls( BView*, unsigned int& extraHeight );		
		virtual	bool QuitRequested();
		virtual void Save();

	private:
		GenericInputView *genView;
		BTextControl* personName;
		BButton* SaveButton;
		BButton* CancelButton;
		BStringView* label;	
		BMessage* sndMessage;
		char fieldName[25];
};

void DoGenericInput( char* title, char* label, BMessage* = NULL, BWindow* window = NULL,
					 char* initial = NULL, int maxLen = 0 );

#endif
