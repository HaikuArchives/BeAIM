#ifndef _ABOUT_BOX_H_
#define _ABOUT_BOX_H_

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <String.h>
#include <StringView.h>
#include "GenList.h"
#include "SingleWindowBase.h"

//-----------------------------------------------------

class LinkStringView : public BStringView
{
	public:
		LinkStringView( BRect frame, const char* name, const char* text );
		void MouseDown(BPoint point);
		void Draw(BRect updateRect);
		void SetURLMode( BString url, bool web );
		
	private:
		int urlMode;
		BString url;
};

//-----------------------------------------------------

class AboutView : public BView
{
	friend class AboutWindow;

	public:
		AboutView( BRect rect );

	private:
		BButton* OKButton;
		BStringView* bySV;
		BStringView* bySV2;
		BStringView* verSV;
		BStringView* majorSV;
};

//-----------------------------------------------------

class AboutWindow : public SingleWindowBase
{
	public:
		AboutWindow( BRect frame );
		~AboutWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	

	private:
	
		void RefreshLangStrings();

		AboutView *genView;
};

//-----------------------------------------------------

const BRect abRegRect = BRect(0,0,201,273);

// factory function
static SingleWindowBase* CreateAboutWindow( BMessage* ) {
	return new AboutWindow(abRegRect);
};

//-----------------------------------------------------

#endif
