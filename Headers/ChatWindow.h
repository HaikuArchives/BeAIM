#ifndef _BEAIM_CHATWINDOW_H__
#define _BEAIM_CHATWINDOW_H__

#include <View.h>
#include <Button.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Box.h>
#include <StringView.h>
#include <storage/Path.h>
#include "GenList.h"
#include "MakSplitterView.h"
#include "NameStatusView.h"
#include "LessAnnoyingWindow.h"
#include "HTMLView.h"
#include "FancyView.h"
#include "constants.h"
#include "AIMUser.h"

//=====================================================

struct chatRecord {
	BString message;
	time_t time;
	bool wasFromOutside;
	bool wasAuto;
};

//=====================================================

enum eventStatus {
	ES_NOT_A_BUDDY,
	ES_OFFLINE,
	ES_ONLINE,
	ES_AWAY
};

//=====================================================

const uint32 CHATWINDOW_SHOW_IN_ALL_WORKSPACES = 'shAw';
const uint32 CHATWINDOW_SHOW_IN_THIS_WORKSPACE = 'shTw';
const uint32 CHATWINDOW_POPUP_ON_RECEIVE       = 'pPRc';
const uint32 CHATWINDOW_SHOW_FONT_COLORS_SIZES = 'aWot';
const uint32 CHATWINDOW_SHOW_TIMESTAMPS        = 'atsp';
const uint32 CHATWINDOW_ENABLE_LINKS 		   = 'lYnX';
const uint32 CHATWINDOW_SAVE_TRANSCRIPT		   = 'tRnZ';
const uint32 CHATWINDOW_REAL_SAVE_TRANSCRIPT   = 'rTnZ';
const uint32 CHATWINDOW_FONT_BASE   = 'cfs0';
const uint32 CHATWINDOW_FONT_SIZE_1 = 'cfs1';
const uint32 CHATWINDOW_FONT_SIZE_2 = 'cfs2';
const uint32 CHATWINDOW_FONT_SIZE_3 = 'cfs3';
const uint32 CHATWINDOW_FONT_SIZE_4 = 'cfs4';
const uint32 CHATWINDOW_FONT_SIZE_5 = 'cfs5';
const uint32 CHATWINDOW_FONT_SIZE_6 = 'cfs6';
const uint32 CHATWINDOW_FONT_SIZE_7 = 'cfs7';
const uint32 CHATWINDOW_FONT_NORMAL = 'cfNM';
const uint32 CHATWINDOW_FONT_ITALIC = 'cfIT';
const uint32 CHATWINDOW_FONT_BOLD   = 'cfBL';

//=====================================================

class PointerStringView : public BStringView {

	public:
		PointerStringView( BRect frame, const char *name, const char *text, 
					   uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
					   uint32 flags = B_WILL_DRAW );
		virtual void MouseMoved( BPoint where, uint32 code, const BMessage *msg );
};

//=====================================================

class BottomView : public BView {

	friend class ChatWindow;

	public:
		BottomView( BRect frame );
		void Draw(BRect updateRect);
	private:
		BButton* sendButton;
};

//=====================================================

class ChatWindow : public LessAnnoyingWindow {

 	public:
		ChatWindow( AIMUser userName, BRect frame, float div = 0.8 );

		virtual void MessageReceived(BMessage *message);
		virtual void FrameResized( float width, float height );
		virtual bool QuitRequested(); 
		virtual void MenusBeginning();
		virtual void DispatchMessage( BMessage* msg, BHandler* handler );
		virtual void WindowActivated( bool activated );
	
		void Popup();
		void LoadPrefs();
		float Divider();
					
	private:
		
		void RefreshLangStrings();
	
		void _InitWindow(void);
		void ProcessIncomingIM( BMessage* );
		void AddMyStatement( BString statement );
		void SetStyles( BMessage* );
		void AddPerson( BMessage* );
		void UpdateLastMessage();
		void SendMessage();
		void DoAutoRespond();
		void FigureOutAwayOptions();
		void WarnEm();
		void SetCanWarn( bool );
		void DoBlockStuff( BMessage* );
		void VerifyDivider( bool fix=true );
		bool DoIRCMeThing( BString, bool wasMine, bool wasAuto );
		void DoBuddyEvent( bool firstTime = false );
		void SetEncoding( uint32 encoding );
		void AddStatementThingy( BString message, time_t time, bool wasFromOutside, bool wasAuto );
		void RebuildChatContents();
		void SaveTranscript( BMessage* );
		void EnableWindow( bool );
		void EnableSendButton();
	
		BMenuBar* menubar;
		FancyTextView* textview;
		HTMLView* editview;
		BScrollView* textscrollview;
		BScrollView* editscrollview;
		MakSplitterView* splitter;
		BMenuItem* warnItem;
		BMenuItem* blockItem;
		BottomView* btView;
		PointerStringView* lastView;
		NameStatusView* otherNameView;
		
		BMenuItem* miCopy;
		BMenuItem* miCut;
		BMenuItem* miPaste;	
		BMenuItem* miUndo;
		BMenuItem* miSelectAll;
		BMenuItem* miPopupOnReceive;
		BMenuItem* miAllWorkspaces;
		BMenuItem* miThisWorkspace;
		BMenuItem* miShowTimestamps;
		BMenuItem* miShowFontColorSizes;
		BMenuItem* miEnableLinks;
		BMenuItem* miSaveTranscript;

		BMenuItem* miNormal;
		BMenuItem* miBold;
		BMenuItem* miItalic;
		BMenuItem* miS1;
		BMenuItem* miS2;
		BMenuItem* miS3;
		BMenuItem* miS4;
		BMenuItem* miS5;
		BMenuItem* miS6;
		BMenuItem* miS7;
		
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

		BMenu* windowMenu;
		BMenu* nameMenu;
		BMenu* editMenu;
		
		float oldDivider;
		float divider;
		float topUIMargin;
		float midUIMargin;
		float botUIMargin;
		float totalUIMargin;
		
		bool popupOnReceive;
		bool showFontColorSizes;
		bool linksEnabled;
		bool hasNewMessage;
		bool prefixNewMessages;
		bool enterIsNewline;
		bool showTimestamps;
		bool doIRCMeThing;
		bool tabIsTab;
		bool firstWasMe;
		bool canWarn;
		bool firstTextNotify;
		
		bool didAutoRespond;
		bool windowEnabled;
		
		BString lastTyped;

		BFont chatFont;
		AIMUser myName;
		AIMUser otherName;
		unsigned short wLevel;
		eventStatus userEventStatus;
		
		uint32 useEncoding;
		
		GenList<chatRecord> chatRecords;
		
		rgb_color kThis;
		rgb_color kThat;
		rgb_color kAction;
		rgb_color kEvent;
};

//=====================================================

#endif
