#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <Window.h>
#include <View.h>
#include <CheckBox.h>
#include <Button.h>
#include <RadioButton.h>
#include <Slider.h>
#include <StringView.h>
#include <ListView.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include "ColorView.h"
#include "PassControl.h"
#include "SingleWindowBase.h"

const uint32 PREFS_SAVE = 'PrSv';
const uint32 PREFS_CANCEL = 'PrCl';
const uint32 PREFS_RL_LANG = 'PrR`';
const uint32 PREFS_SET_PROSP_LANG = 's4pL';

//-----------------------------------------------------

class PanelItem : public BListItem
{ 
	public: 
		PanelItem( BString name, bool enabled, BFont* font );
		void SetName( BString );
		BString Name();
		virtual void DrawItem( BView *owner, BRect frame, bool complete = false );
		virtual void Update( BView *owner, const BFont *font );

	private: 
		BFont* ourFont;
		BString name;
		bool enabled;
		float rightHeight;
};

//-----------------------------------------------------

class PanelSelectorView : public BListView {

	public:
		PanelSelectorView( BRect, const char*, list_view_type = B_SINGLE_SELECTION_LIST,
			uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP,
			uint32 = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE );
		~PanelSelectorView();
		virtual void SelectionChanged();
};

//-----------------------------------------------------

class GSlider : public BSlider {

	public:
		GSlider( BRect frame, int32 min, int32 max, BTextControl*, BTextView* );
		virtual char* UpdateText( void ) const;
		
	private:
		char upText[200];
		BTextControl* thang;
		BTextView* otherThang;
};

//-----------------------------------------------------

class GenPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		GenPrefView(BRect frame, bool globalOnly = false );
		void Save();

	private:
	
		void CreateLanguageList();
		void CreateEncodingsList();
		void SetEncoding( uint32 enc );
		void RefreshLangStrings();
			
		BStringView* globalLabel;
		EnStringView* userSpecLabel;
			
		BCheckBox*  BuddyListAllWorkspaces;
		BCheckBox*  PlaySounds;
		BCheckBox*  UseDeskbar;
		BCheckBox*  ShowDeskbarEntry;
		BCheckBox*  DoIdleTime;
		BCheckBox*  showInDeskbar;
		BMenuField* LanguageList;
		BMenuField* EncodingsList;
		BPopUpMenu*	LanguageMenu;
		BPopUpMenu*	EncodingsMenu;
		BString prospectiveLang;
		
		BMenuItem* e1;
		BMenuItem* e2;
		BMenuItem* e3;
		BMenuItem* e4;
		BMenuItem* e5;
		BMenuItem* e6;
		BMenuItem* e7;
		BMenuItem* e8;
		BMenuItem* e9;
		BMenuItem* e10;
		BMenuItem* e11;
		BMenuItem* e12;
		uint32 encoding;
		
		bool globalOnly;
};


//-----------------------------------------------------

class ConnectionPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		ConnectionPrefView(BRect frame, bool globalOnly = false );
		void Save();

	private:
	
		void RefreshLangStrings();
		
		void EnableProxyControls();
	
		BTextControl* conHost;
		BTextControl* conPort;
		BRadioButton* proxyNo;
		BRadioButton* proxyHTTPS;
		BRadioButton* proxySOCKS5;
		BTextControl* proxyHost;
		BTextControl* proxyPort;
		BCheckBox* needsAuth;
		BTextControl* authUser;
		PassControl* authPass;
		BStringView* httpsWarning;
		BBox* serverBox;
		BBox* proxyModeBox;
		BBox* proxySettingsBox;
		bool globalOnly;		
};


//-----------------------------------------------------

class ChatWindowPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		ChatWindowPrefView(BRect frame, bool globalOnly = false );
		void Save();

	private:
		void RefreshLangStrings();
		void MagSizeChanged();
	
		BCheckBox* ChatWindowAllWorkspaces;
		BCheckBox* ChatWindowPopup;
		BCheckBox* ChatShowTimestamps;
		BCheckBox* ChatFontColorSizes;
		BCheckBox* ChatShowLinks;
		BCheckBox* ChatPrefixNewMessages;
		BCheckBox* ChatIRCMeThing;
		BTextView* sampleSizeView;
		BTextControl* sizeEdit;
		GSlider* magSlider;
		bool globalOnly;
};

//-----------------------------------------------------

class AwayPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		AwayPrefView(BRect frame, bool globalOnly = false );
		void Save();

	private:
		void RefreshLangStrings();
		//BCheckBox* BuddyListAllWorkspaces;
		bool globalOnly;
};

//-----------------------------------------------------

class KeysPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		KeysPrefView(BRect frame, bool globalOnly = false );
		void Save();

	private:
		EnStringView* tabLabel;
		EnStringView* enterLabel;
		BRadioButton* tabIsTab;
		BRadioButton* tabIsFocusChange;
		BRadioButton* enterInsertsLineBreak;
		BRadioButton* enterPressesDefButton;
		void RefreshLangStrings();
		bool globalOnly;
};

//-----------------------------------------------------

class ColorPrefView : public BView 
{
	friend class PrefsWindow;

	public:
		ColorPrefView(BRect frame, bool globalOnly = false );
		void OpenColorPicker( BMessage* msg );
		void SetNewColor( BMessage* msg );
		void Save();

	private:
		void RefreshLangStrings();
		ColorView* bgColor;
		ColorView* fontColor;
		ColorView* fromYouColor;
		ColorView* toYouColor;
		ColorView* actionColor;
		ColorView* eventColor;
		bool globalOnly;
};

//-----------------------------------------------------

class PrefsWindow : public SingleWindowBase 
{
	public:
		PrefsWindow( BRect frame, bool globalOnly = false ); 
		virtual	bool QuitRequested();
		virtual void MessageReceived( BMessage *message );
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
		
		void SetActivePanel( int32 panel );
		int32 ActivePanel();
		
	private:
	
		void RefreshLangStrings();
		void SaveLangIfNeeded( bool force = false );
	
		GenPrefView* genPrefs;
		ConnectionPrefView* conPrefs;
		ChatWindowPrefView* chatPrefs;
		AwayPrefView* awayPrefs;
		ColorPrefView* colorPrefs;
		KeysPrefView* keyPrefs;
		PanelSelectorView* panelsview;
		BButton* btnCancel;
		BButton* btnSave;
		BScrollView* scroll;
		int32 curPanel;
		BFont ourFont;
};

//-----------------------------------------------------

// factory function
static SingleWindowBase* CreatePrefsWindow( BMessage* msg ) {
	return new PrefsWindow(BRect(0,0,515,260), msg->FindBool("globalonly"));
};

//-----------------------------------------------------


#endif
