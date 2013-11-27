#ifndef PASSCONTROL_H
#define PASSCONTROL_H

#include <TextControl.h>
#include <Font.h>
#include <Rect.h>
#include <Message.h>

#include <string.h>
#include <String.h>

#include "myFilter.h"

class PassControl : public BTextControl {
	public:
		PassControl(BRect frame, const char *name, const char *label,
								const char *initial_text, BMessage *message );
		virtual	~PassControl(); // deletes filter
		virtual void SetText(const char *text);
		BString actualText() const; // returns actual
		
		void InsertText( BMessage* );
		void DeleteText( bool delMode );
		void CutText();

	private:
		myFilter* filter;
		BString	actual;
		int32 length;
};

#endif
