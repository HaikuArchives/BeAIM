#ifndef _COLOR_PICKER_H
#define _COLOR_PICKER_H

#include <Window.h>
#include <ScrollView.h>
#include <ListView.h>
#include <Box.h>
#include <ColorControl.h>
#include <String.h>
#include "GenList.h"
#include "ColorView.h"

const uint32 BEAIM_CP_COLOR_CHANGED = 'nee!';
const uint32 BEAIM_NEW_COLOR_PICKED = 'nCLR';

class ColorPickerView : public BView
{
	friend class ColorPickerWindow;

	public:
		ColorPickerView( BRect rect );
		
	private:
 		BButton* btnSave;
		BButton* btnCancel;
		BColorControl* picker;
		ColorView* colPick;
};


class ColorPickerWindow : public BWindow
{
	public:
		ColorPickerWindow( BRect frame, const char* title, int id, BWindow* own );
		~ColorPickerWindow();

		virtual void MessageReceived(BMessage* message);
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );	
		virtual	bool QuitRequested();
		void SetTheColor( rgb_color );

	private:
	
		ColorPickerView *genView;
		BWindow* owner;
		int cid;
};

#endif
