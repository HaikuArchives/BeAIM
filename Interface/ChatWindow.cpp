#include <Application.h>
#include <Message.h>
#include <Roster.h>
#include <Window.h>
#include <MenuBar.h>
#include <Menu.h>
#include <Entry.h>
#include <Alert.h>
//#include <Path.h>
#include <MenuItem.h>
#include <Box.h>
#include <FilePanel.h>
#include <ScrollView.h>
#include <TextView.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SupportKit.h>
#include <ctype.h>
#include "Globals.h"
#include "constants.h"
#include "MiscStuff.h"
#include "ChatWindow.h"
#include "HTMLStuff.h"
#include "Globals.h"
#include "Linkify.h"
#include "Say.h"

const int STATUS_HEIGHT = 13;

ChatWindow::ChatWindow( AIMUser userName, BRect frame, float div )
			: LessAnnoyingWindow(frame, userName.UserString(), B_DOCUMENT_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, 0 )
{
	uint32 enc;

	otherName = userName;
	popupOnReceive = false;
	showFontColorSizes = false;
	hasNewMessage = false;
	prefixNewMessages = false;
	enterIsNewline = false;
	tabIsTab = false;
	didAutoRespond = false;
	doIRCMeThing = true;
	firstWasMe = false;
	canWarn = false;
	windowEnabled = true;
	showTimestamps = false;
	wLevel = 0;
	oldDivider = -1;
	divider = div;
	userEventStatus = ES_NOT_A_BUDDY;

	// colors
	kThis.red = 255;
	kThis.green = kThis.blue = 0;
	kThat.blue = 255;
	kThat.green = kThat.red = 0;
	kAction.red = 0x9D;
	kAction.green = 0x07;
	kAction.blue = 0xF8;
	kEvent.red = 0;
	kEvent.green = 0x4D;
	kEvent.blue = 0x4A;

	// preparatory stuff
	_InitWindow();
	srand(unsigned(time(NULL)));
	LoadPrefs();

	// Get the screen name for this client
	myName = client->User();

	// set the encoding for this thing
	enc = users->GetBuddyEncoding(otherName);
	SetEncoding( enc );

	// get their initial status if they're on the buddylist
	if( users->IsABuddy(userName) )
		DoBuddyEvent( true );

	// Tell the main app that the window has been opened
	BMessage* sendMessage = new BMessage( BEAIM_IM_WINDOW_OPENED );
	sendMessage->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
	sendMessage->AddString( "userid", otherName.UserString() );
	sendMessage->AddPointer( "new_window", this );
	PostAppMessage( sendMessage );
}

//-----------------------------------------------------

// Initialize the window.
void ChatWindow::_InitWindow(void) {

	BRect r;
	float HeightCalc;
	BMenuItem* tempItem;

	// Add the menu bar
	r = Bounds();
	menubar = new BMenuBar(r, "menu_bar");
	AddChild(menubar);

	// add the "last message" view
	BRect lastRect = Bounds();
	lastRect.top = menubar->Bounds().bottom + 1.0;
	lastRect.bottom = lastRect.top + 13;
	lastView = new PointerStringView( lastRect, "lastmsg", Language.get("CW_NO_MESSAGES"), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT,
								B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );
	lastView->SetViewColor( 125, 125, 125 );
	lastView->SetLowColor( 125, 125, 125 );
	//lastView->SetFont( be_bold_font );
	lastView->SetFontSize(10);
	lastView->SetHighColor( 255, 255, 255 );
	lastView->SetAlignment( B_ALIGN_RIGHT );
	AddChild( lastView );

	// Set up the Divider
	VerifyDivider( false );
	topUIMargin = menubar->Bounds().bottom + lastRect.Height() + 1.0;
	midUIMargin = 9;
	botUIMargin = 51;
	totalUIMargin = topUIMargin + midUIMargin + botUIMargin;

	// Set up the view rectangles
	BRect textframe = Bounds();
	HeightCalc = textframe.bottom - textframe.top - totalUIMargin;
	textframe.right -= B_V_SCROLL_BAR_WIDTH;
	BRect editframe = textframe;
	textframe.top = topUIMargin;
	textframe.bottom = HeightCalc*divider + topUIMargin;
	editframe.top = textframe.bottom + midUIMargin;
	editframe.bottom -= (botUIMargin+1);

	// Add the text (conversation) BHTMLView
	BRect textrect = textframe;
	textrect.OffsetTo(B_ORIGIN);
	textrect.InsetBy( 2.0, 2.0 );
	textview = new FancyTextView( textframe, "text_view", textrect, B_FOLLOW_ALL,
								  B_PULSE_NEEDED | B_WILL_DRAW | B_NAVIGABLE_JUMP );
	AddChild(textscrollview = new BScrollView("text_scroll_view", textview,
				B_FOLLOW_LEFT_RIGHT, 0, false, true, B_NO_BORDER));
	textview->SetDoesUndo(true);
	rgb_color rayedski;
	rayedski.red = rayedski.green = rayedski.blue = 0;
	rayedski.red = 255;
	textview->SetViewColor( rayedski );//GetBeAIMColor(BC_REALLY_LIGHT_GRAY) );

	// Add the message editing BHTMLView
	BRect editrect = editframe;
	editrect.OffsetTo(B_ORIGIN);
	editrect.InsetBy( 2.0, 2.0 );
	editview = new HTMLView( true, editframe, "edit_view", editrect,
				B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED | B_NAVIGABLE );
	AddChild(editscrollview = new BScrollView("edit_scroll_view", editview,
				B_FOLLOW_LEFT_RIGHT, 0, false, true, B_NO_BORDER));
	editview->SetDoesUndo(true);
	editview->SetMaxBytes( 1024 );
	editview->ResetFontToBase();
	editview->MakeFocus(true);
	editview->SetViewColor( rayedski );//GetBeAIMColor(BC_WHITE) );

	// Add the splitter bar
	rgb_color rayed;
	BRect divframe = Bounds();
	divframe.top = textframe.bottom + 1;
	divframe.bottom = editframe.top - 1;
	divframe.right += 1;
	rayed.red = rayed.green = rayed.blue = 200;
	splitter = new MakSplitterView( divframe, textscrollview, editscrollview, MAK_V_SPLITTER,
									B_FOLLOW_LEFT_RIGHT, rayed );
	AddChild( splitter );

	// Add the bottom view
	BRect btRect = Bounds();
	btRect.top = btRect.bottom - botUIMargin;
	btRect.bottom -= STATUS_HEIGHT;
	btView = new BottomView( btRect );
	AddChild( btView );

	// add the name view
	r = Bounds();
	r.top = r.bottom - STATUS_HEIGHT;
	r.right -= 14;
	otherNameView = new NameStatusView( r );
	otherNameView->SetName( otherName );
	otherNameView->SetWarningLevel( client->WarningLevel() );
	if( client->AwayMode() != AM_NOT_AWAY )
		otherNameView->SetAway( true );
	AddChild( otherNameView );

	// Add the Window menu to the menubar
	windowMenu = new BMenu( "Window" );
	BMenu* workspaceView = new BMenu( "Show this window" );
	workspaceView->SetRadioMode( true );
	BMenuItem* miThisWorkspace = new BMenuItem("Only in This Workspace", new BMessage(CHATWINDOW_SHOW_IN_THIS_WORKSPACE));
	workspaceView->AddItem( miThisWorkspace );
	workspaceView->AddItem( miAllWorkspaces = new BMenuItem("In All Workspaces", new BMessage(CHATWINDOW_SHOW_IN_ALL_WORKSPACES)) );
	windowMenu->AddItem(workspaceView);
	windowMenu->AddItem(miPopupOnReceive = new BMenuItem("Popup on Message Receive", new BMessage(CHATWINDOW_POPUP_ON_RECEIVE), 'P'));
	windowMenu->AddItem(miShowTimestamps = new BMenuItem("Show Timestamps", new BMessage(CHATWINDOW_SHOW_TIMESTAMPS), 'T'));
	windowMenu->AddItem(miShowFontColorSizes = new BMenuItem("Show Font Colors and Sizes", new BMessage(CHATWINDOW_SHOW_FONT_COLORS_SIZES)));
	windowMenu->AddItem(miEnableLinks = new BMenuItem("Enable web/email links", new BMessage(CHATWINDOW_ENABLE_LINKS)));
	windowMenu->AddSeparatorItem();
	windowMenu->AddItem(miSaveTranscript = new BMenuItem("Save Chat Transcript"B_UTF8_ELLIPSIS, new BMessage(CHATWINDOW_SAVE_TRANSCRIPT), 'S'));
	windowMenu->AddSeparatorItem();
	windowMenu->AddItem(new BMenuItem("Previous Chat Window", new BMessage(BEAIM_PREV_CHAT_WINDOW), ','));
	windowMenu->AddItem(new BMenuItem("Next Chat Window", new BMessage(BEAIM_NEXT_CHAT_WINDOW), '.'));
	windowMenu->AddItem(tempItem=new BMenuItem("Jump to Buddy List", new BMessage(BEAIM_JUMP_TO_BUDDYLIST), '/'));
	tempItem->SetTarget(BeAIMAppSig().String());
	windowMenu->AddSeparatorItem();
	windowMenu->AddItem(new BMenuItem("Close", new BMessage(MENU_FILE_CLOSE), 'W'));
	menubar->AddItem(windowMenu);

	// Add the Edit menu to the menu bar
	editMenu = new BMenu("Edit");
	editMenu->AddItem(miUndo=new BMenuItem("Undo", new BMessage(B_UNDO), 'Z'));
	miUndo->SetTarget(editview);
	editMenu->AddSeparatorItem();
	editMenu->AddItem(miCut=new BMenuItem("Cut", new BMessage(B_CUT), 'X'));
	miCut->SetTarget(editview);
	editMenu->AddItem(miCopy=new BMenuItem("Copy", new BMessage(B_COPY), 'C'));
	miCopy->SetTarget(editview);
	editMenu->AddItem(miPaste=new BMenuItem("Paste", new BMessage(B_PASTE), 'V'));
	miPaste->SetTarget(editview);
	editMenu->AddSeparatorItem();
	editMenu->AddItem(miSelectAll=new BMenuItem("Select All", new BMessage(B_SELECT_ALL), 'A'));
	miSelectAll->SetTarget(editview);
	menubar->AddItem(editMenu);

	// Add the Font menu to the menubar
	/*
	menu = new BMenu("Style");
	menu->SetEnabled( false );
	menu->AddItem(miNormal=new BMenuItem("Normal", new BMessage(CHATWINDOW_FONT_NORMAL), 'N'));
	miNormal->SetTarget(editview);
	menu->AddItem(miBold=new BMenuItem("Bold", new BMessage(CHATWINDOW_FONT_BOLD), 'B'));
	miBold->SetTarget(editview);
	menu->AddItem(miItalic=new BMenuItem("Italic", new BMessage(CHATWINDOW_FONT_ITALIC), 'I'));
	miItalic->SetTarget(editview);
	menu->AddSeparatorItem();
	menu->AddItem(miS1=new BMenuItem("Size 1", new BMessage(CHATWINDOW_FONT_SIZE_1), '1'));
	miS1->SetTarget(editview);
	menu->AddItem(miS2=new BMenuItem("Size 2", new BMessage(CHATWINDOW_FONT_SIZE_2), '2'));
	miS2->SetTarget(editview);
	menu->AddItem(miS3=new BMenuItem("Size 3", new BMessage(CHATWINDOW_FONT_SIZE_3), '3'));
	miS3->SetTarget(editview);
	menu->AddItem(miS4=new BMenuItem("Size 4", new BMessage(CHATWINDOW_FONT_SIZE_4), '4'));
	miS4->SetTarget(editview);
	menu->AddItem(miS5=new BMenuItem("Size 5", new BMessage(CHATWINDOW_FONT_SIZE_5), '5'));
	miS5->SetTarget(editview);
	menu->AddItem(miS6=new BMenuItem("Size 6", new BMessage(CHATWINDOW_FONT_SIZE_6), '6'));
	miS6->SetTarget(editview);
	menu->AddItem(miS7=new BMenuItem("Size 7", new BMessage(CHATWINDOW_FONT_SIZE_7), '7'));
	miS7->SetTarget(editview);
	//menubar->AddItem(menu);
	*/

	// Add the <ScreenName> menu to the menubar
	BMenuItem* buddyItem;
	nameMenu = new BMenu( otherName.UserString() );
	nameMenu->AddItem(buddyItem = new BMenuItem("Add to Buddy List" B_UTF8_ELLIPSIS,
							  new BMessage(BEAIM_ADD_BUDDY), 'L' ));
	nameMenu->AddItem(buddyItem = new BMenuItem("Get Info" B_UTF8_ELLIPSIS,
							  new BMessage(BEAIM_GET_USER_INFO), 'I' ));
	nameMenu->AddSeparatorItem();
	nameMenu->AddItem(warnItem = new BMenuItem("Warn"B_UTF8_ELLIPSIS, new BMessage(BEAIM_WARN_SOMEONE) ));
	nameMenu->AddItem(blockItem = new BMenuItem("Block/Unblock" B_UTF8_ELLIPSIS, new BMessage(BEAIM_SET_USER_BLOCKINESS) ));

	nameMenu->AddSeparatorItem();
	BMenu* useEncoding = new BMenu( "Use Encoding" );
	useEncoding->SetRadioMode( true );
	BMessage* encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_ISO2_CONVERSION );
	useEncoding->AddItem(e1 = new BMenuItem("Central European (ISO 8859-2)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_ISO5_CONVERSION );
	useEncoding->AddItem(e2 = new BMenuItem("Cyrillic (ISO 8859-5)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_KOI8R_CONVERSION );
	useEncoding->AddItem(e3 = new BMenuItem("Cyrillic (KOI8-R)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_MS_DOS_866_CONVERSION );
	useEncoding->AddItem(e4 = new BMenuItem("Cyrillic (MS-DOS 866)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_MS_WINDOWS_1251_CONVERSION );
	useEncoding->AddItem(e5 = new BMenuItem("Cyrillic (Windows 1251)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_ISO7_CONVERSION );
	useEncoding->AddItem(e6 = new BMenuItem("Greek (ISO 8859-7)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_SJIS_CONVERSION );
	useEncoding->AddItem(e7 = new BMenuItem("Japanese (Shift-JIS)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_EUC_CONVERSION );
	useEncoding->AddItem(e8 = new BMenuItem("Japanese (EUC)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_UNICODE_CONVERSION );
	useEncoding->AddItem(e9 = new BMenuItem("Unicode", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_ISO1_CONVERSION );
	useEncoding->AddItem(e10 = new BMenuItem("Western (ISO 8859-1)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_MAC_ROMAN_CONVERSION );
	useEncoding->AddItem(e11 = new BMenuItem("Western (Mac Roman)", encMsg) );
	encMsg = new BMessage(BEAIM_CHANGE_ENCODING);
	encMsg->AddInt32( "encoding", (int32)B_MS_WINDOWS_CONVERSION );
	useEncoding->AddItem(e12 = new BMenuItem("Western (MS Windows)", encMsg) );
	nameMenu->AddItem( useEncoding );

	menubar->AddItem(nameMenu);

	// load the default prefs settings
	if( prefs->ReadBool( "ChatWindowAllWorkspaces", false ) ) {
		SetAllWorkspaces( true );
		miAllWorkspaces->SetMarked( true );
	} else {
		SetAllWorkspaces( false );
		miThisWorkspace->SetMarked( true );
	}
	if( prefs->ReadBool( "PopupOnMessage", false ) ) {
		popupOnReceive = true;
		miPopupOnReceive->SetMarked( true );
	}
	if( prefs->ReadBool( "ShowTimestamps", false ) ) {
		showTimestamps = true;
		miShowTimestamps->SetMarked( true );
	}
	if( prefs->ReadBool( "ShowFontColorSizes", true ) ) {
		showFontColorSizes = true;
		textview->SetShowFontColorSizes( true );
		miShowFontColorSizes->SetMarked( true );
	} else
		textview->SetShowFontColorSizes( false );
	if( prefs->ReadBool( "ShowLinks", true ) ) {
		linksEnabled = true;
		textview->SetShowLinks( true );
		miEnableLinks->SetMarked( true );
	} else
		textview->SetShowLinks( false );
	prefixNewMessages = prefs->ReadBool( "PrefixNewMessages", false );

	// Set the BTextView attributes
	textview->MakeEditable( false );
	textview->SetStylable( true );
	editview->MakeEditable( true );
	editview->SetStylable( false );

	// get all the right language strings and stuff
	RefreshLangStrings();

	// no warnings yet!
	SetCanWarn( false );

	// disable the send button until something has been typed...
	EnableSendButton();
	firstTextNotify = true;
	lastTyped = "";;
	// just make damned sure...
	aimnet->SetTypingStatus(otherName, 0x0000);
}

//-----------------------------------------------------

void ChatWindow::RefreshLangStrings() {

	// menu items and such
	windowMenu->Superitem()->SetLabel( Language.get("CW_WINDOW_MENU") );
	windowMenu->SubmenuAt(0)->ItemAt(0)->SetLabel( Language.get("CW_SHOW_ONLYTHISWS") );
	windowMenu->SubmenuAt(0)->ItemAt(1)->SetLabel( Language.get("CW_SHOW_ALLWS") );
	windowMenu->ItemAt(0)->SetLabel( Language.get("CW_SHOW_WINDOW") );
	windowMenu->ItemAt(1)->SetLabel( Language.get("CW_POPUP_ON_RECEIVE") );
	windowMenu->ItemAt(2)->SetLabel( Language.get("CW_SHOW_TIMESTAMPS") );
	windowMenu->ItemAt(3)->SetLabel( Language.get("CW_SHOW_FONTSCOLORS") );
	windowMenu->ItemAt(4)->SetLabel( Language.get("CW_SHOW_LINKS") );
	windowMenu->ItemAt(6)->SetLabel( LangWithSuffix("CW_SAVE_TRANSCRIPT", B_UTF8_ELLIPSIS) );
	windowMenu->ItemAt(8)->SetLabel( Language.get("CW_JUMP_PREV_WINDOW") );
	windowMenu->ItemAt(9)->SetLabel( Language.get("CW_JUMP_NEXT_WINDOW") );
	windowMenu->ItemAt(10)->SetLabel( Language.get("CW_JUMP_BUDDY_LIST") );
	windowMenu->ItemAt(12)->SetLabel( Language.get("CLOSE_LABEL") );

	editMenu->Superitem()->SetLabel( Language.get("CW_EDIT_MENU") );
	editMenu->ItemAt(0)->SetLabel( Language.get("CW_EDIT_UNDO") );
	editMenu->ItemAt(2)->SetLabel( Language.get("CW_EDIT_CUT") );
	editMenu->ItemAt(3)->SetLabel( Language.get("CW_EDIT_COPY") );
	editMenu->ItemAt(4)->SetLabel( Language.get("CW_EDIT_PASTE") );
	editMenu->ItemAt(6)->SetLabel( Language.get("CW_EDIT_SELECTALL") );
	nameMenu->ItemAt(0)->SetLabel( LangWithSuffix("ADD_BLIST_LABEL", B_UTF8_ELLIPSIS) );
	nameMenu->ItemAt(1)->SetLabel( LangWithSuffix("CW_GET_INFO", B_UTF8_ELLIPSIS) );
	nameMenu->ItemAt(3)->SetLabel( LangWithSuffix("CW_WARN", B_UTF8_ELLIPSIS) );
	nameMenu->ItemAt(6)->SetLabel( LangWithSuffix("CW_USE_ENCODING", B_UTF8_ELLIPSIS) );

	if( users->IsUserBlocked(otherName) )
		nameMenu->ItemAt(4)->SetLabel( Language.get("CW_UNBLOCK") );
	else
		nameMenu->ItemAt(4)->SetLabel( Language.get("CW_BLOCK") );

	// relabel and move the buttons
	btView->sendButton->SetLabel( Language.get("CW_SEND_BUTTON") );
	btView->sendButton->ResizeToPreferred();
	btView->sendButton->MoveTo( Bounds().Width() - 3 - btView->sendButton->Bounds().Width(), btView->sendButton->Frame().top );
}

//-----------------------------------------------------

// Adjust the size of the BTextView's text rectangles
// when the window is resized.
void ChatWindow::FrameResized(float width, float height) {

	BScrollBar* vert;

	// resize the windows themselves
	float newHeight = (height - totalUIMargin) * divider;
	textscrollview->ResizeTo( width, newHeight );
	editscrollview->ResizeTo( width, height - totalUIMargin - newHeight );
	editscrollview->MoveTo( 0, topUIMargin + midUIMargin + newHeight );
	splitter->MoveTo( 0, topUIMargin + newHeight + 1 );

	// muck with the scrollview's scrollbars
	vert = textscrollview->ScrollBar(B_VERTICAL);
	if( vert ) {
		vert->SetValue( 0 );
	}

	// set the new text rects
	BRect textrect = textview->Bounds();
	textrect.InsetBy( 2.0, 2.0 );
	textview->SetTextRect( textrect );
	textrect = editview->Bounds();
	textrect.InsetBy( 2.0, 2.0 );
	editview->SetTextRect( textrect );
}

//-----------------------------------------------------

void ChatWindow::WarnEm() {

	int32 buttonResult;
	BMessage* msg;

	// ask for permission, then remove the sucker
	BAlert* alert = new BAlert("title", Language.get("ERR_ASK_WARN"), Language.get("CANCEL_LABEL"),
								Language.get("ANONYMOUS_LABEL"), Language.get("NORMAL_LABEL"),
								B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_IDEA_ALERT);
	alert->SetShortcut( 0, B_ESCAPE );
	if( (buttonResult = alert->Go()) ) {

		msg = new BMessage(BEAIM_WARN_SOMEONE);
		msg->AddString( "userid", otherName.UserString() );

		if( buttonResult == 1 )
			msg->AddBool( "anonymous", true );
		else
			msg->AddBool( "anonymous", false );

		PostAppMessage( msg );

		// we just warned 'em; can't do it twice in a row
		SetCanWarn( false );
	}
}

//-----------------------------------------------------

void ChatWindow::MessageReceived( BMessage *message ) {

	// Some variables needed for various messages
	BMessage* sendMessage;
	BString textosity;

	switch(message->what) {

		case TEXT_WAS_MODIFIED:
			EnableSendButton();
			lastTyped = editview->Text();
			if ((!firstTextNotify) && (lastTyped.Length() == 0)) {
				aimnet->SetTypingStatus(otherName, 0x0000);
				firstTextNotify = true;
			} else if ((firstTextNotify) && (lastTyped.Length() > 0)) {
				aimnet->SetTypingStatus(otherName, 0x0002);
				firstTextNotify = false;
			}
			break;

		// !!!! translate these!
		case BEAIM_TYPING_STATUS:
			textosity = otherName.Username();
			switch (message->FindInt32("tstat_value")) {
				case 0x0000:
//					textosity << " has finshed typing...";
					break;
				case 0x0001:
					textosity << " has paused typing...";
					break;
				case 0x0002:
					textosity << " is typing...";
					break;
			}
			otherNameView->SetName(textosity);
//			snooze(30000000);
//			otherNameView->SetName(otherName.Username());
			break;

		case BEAIM_TYPE_PULSE:
			if ((lastTyped == editview->Text()) && (!firstTextNotify)) {
				aimnet->SetTypingStatus(otherName, 0x0001);
				firstTextNotify = true;
			}
//			lastTyped = editview->Text();
			break;

		case BEAIM_ATTEMPT_SEND_IM:
			aimnet->SetTypingStatus(otherName, 0x0000);
			firstTextNotify = true;
			SendMessage();
			break;

		case BEAIM_INCOMING_IM:
			ProcessIncomingIM( message );
			otherNameView->SetName(otherName.Username());
			break;

		case BEAIM_RELOAD_PREF_SETTINGS:
			LoadPrefs();
			break;

		case CHATWINDOW_SHOW_IN_THIS_WORKSPACE:
			SetAllWorkspaces( false );
			break;

		case CHATWINDOW_SHOW_IN_ALL_WORKSPACES:
			SetAllWorkspaces( true );
			break;

		case CHATWINDOW_POPUP_ON_RECEIVE:
			miPopupOnReceive->SetMarked( !miPopupOnReceive->IsMarked() );
			popupOnReceive = miPopupOnReceive->IsMarked();
			break;

		case CHATWINDOW_SHOW_FONT_COLORS_SIZES:
			miShowFontColorSizes->SetMarked( !miShowFontColorSizes->IsMarked() );
			showFontColorSizes = miShowFontColorSizes->IsMarked();
			textview->SetShowFontColorSizes( showFontColorSizes );
			RebuildChatContents();
			break;

		case CHATWINDOW_ENABLE_LINKS:
			miEnableLinks->SetMarked( !miEnableLinks->IsMarked() );
			linksEnabled = miEnableLinks->IsMarked();
			textview->SetShowLinks( linksEnabled );
			break;

		case MENU_FILE_CLOSE:
			QuitRequested();
			Close();
			break;

		case CHATWINDOW_SAVE_TRANSCRIPT:
			windows->OpenTranscriptSavePanel( otherName, this );
			break;

		case CHATWINDOW_REAL_SAVE_TRANSCRIPT:
			SaveTranscript( message );
			break;

		case CHATWINDOW_SHOW_TIMESTAMPS:
			miShowTimestamps->SetMarked( !miShowTimestamps->IsMarked() );
			showTimestamps = miShowTimestamps->IsMarked();
			RebuildChatContents();
			break;

		case BEAIM_GET_USER_INFO:
			sendMessage = new BMessage(BEAIM_OPEN_USER_WINDOW);
			sendMessage->AddString( "userid", otherName.UserString() );
			sendMessage->AddInt32( "wtype", (int32)USER_INFO_TYPE );
			sendMessage->AddPointer( "poswindow", this );
			PostAppMessage( sendMessage );
			break;

		case BEAIM_ADD_BUDDY:
			AddPerson(message);
			break;

		case BEAIM_NEXT_CHAT_WINDOW:
		case BEAIM_PREV_CHAT_WINDOW:
			message->AddString( "userid", otherName.UserString() );
			message->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
			PostAppMessage( message );
			break;

		case BEAIM_AWAY_STATUS_CHANGED:
			if( client->AwayMode() != AM_NOT_AWAY ) {
				otherNameView->SetAway(true);
				didAutoRespond = false;
				firstWasMe = false;
			} else
				otherNameView->SetAway(false);
			break;

		case CHATWINDOW_FONT_SIZE_1:
		case CHATWINDOW_FONT_SIZE_2:
		case CHATWINDOW_FONT_SIZE_3:
		case CHATWINDOW_FONT_SIZE_4:
		case CHATWINDOW_FONT_SIZE_5:
		case CHATWINDOW_FONT_SIZE_6:
		case CHATWINDOW_FONT_SIZE_7:
		case CHATWINDOW_FONT_NORMAL:
		case CHATWINDOW_FONT_ITALIC:
		case CHATWINDOW_FONT_BOLD:
			SetStyles( message );
			break;

		case MAK_START_SPLITTING:
			textview->SetBeingDragged(true);
			editview->SetBeingDragged(true);
			break;

		case MAK_DONE_SPLITTING:
			textview->SetBeingDragged(false);
			editview->SetBeingDragged(false);
			divider = textscrollview->Frame().Height() / (Bounds().Height() - totalUIMargin);
			VerifyDivider();
			break;

		case BEAIM_WARN_SOMEONE:
			WarnEm();
			break;

		case BEAIM_SET_USER_BLOCKINESS:
			DoBlockStuff(message);
			break;

		case BEAIM_BUDDY_STATUS_CHANGE:
		case BEAIM_ONCOMING_BUDDY:
			if( message->HasInt32("warninglevel") )
				otherNameView->SetWarningLevel( (unsigned short)message->FindInt32("warninglevel") );
		case BEAIM_OFFGOING_BUDDY:
			DoBuddyEvent();
			break;

		case BEAIM_LOGIN_STATUS_CHANGED:
			EnableWindow( message->FindBool("loggedin") );
			break;

		case BEAIM_CHANGE_ENCODING:
			SetEncoding( (int32)message->FindInt32("encoding") );
			users->SetBuddyEncoding( otherName, useEncoding );
			break;

		case FANCY_OVER_LINK:
			otherNameView->SetTempString( message->FindString("url") );
			break;

		case FANCY_NOT_OVER_LINK:
			otherNameView->SetTempString( "" );
			break;

		case B_QUIT_REQUESTED:
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}

//-----------------------------------------------------

// the cancel function
void ChatWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// the enter key does a bunch of stuff, depending on prefs...
	if( msg->what == B_KEY_DOWN && msg->HasString("bytes") ) {
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ENTER &&
			editview->IsFocus() )
		{

			uint32 mods = modifiers();

			// shift-enter... depends on pref
			if( mods & B_SHIFT_KEY ) {
				if( !enterIsNewline ) {
					editview->Insert("\n");
					editview->ScrollToSelection();
					return;
				}

				// we can remove the shift key from the mask, so we don't navigate backwards
				msg->ReplaceInt32( "modifiers", mods & ~B_SHIFT_KEY );
			}

			// plain old enter... depends on pref
			else {
				if( enterIsNewline ) {
					editview->Insert("\n");
					editview->ScrollToSelection();
					return;
				}
			}

			// OK, at this point the enter is meant to press the "send" button.
			// don't let that happen if the send button isn't enabled, because instead
			// it will fall through and insert a line break in the text editor. Normally
			// I suppose this would make sense but in this case it's silly.
			if( !btView->sendButton->IsEnabled() )
				return;
		}

		// handle the almighty tab key
		else if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_TAB &&
				 editview->IsFocus() )
		{

			uint32 mods = modifiers();

			if( !tabIsTab ) {
				if( !(mods&B_COMMAND_KEY || mods&B_CONTROL_KEY || mods&B_MENU_KEY || mods&B_SHIFT_KEY) )
					msg->ReplaceInt32( "modifiers", mods | B_COMMAND_KEY );
			}

			else {
				if( mods & B_SHIFT_KEY )
					msg->ReplaceInt32( "modifiers", (mods | B_COMMAND_KEY) & ~B_SHIFT_KEY );
			}
		}

		// the escape key, of course
		else if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			uint32 mods = modifiers();
			if( mods == 0 || mods == 32 && !menubar->IsFocus()) {
				PostMessage( new BMessage(B_QUIT_REQUESTED) );
				return;
			}
		}
	}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

bool ChatWindow::QuitRequested()
{
	BMessage* curMsg = CurrentMessage();
	if( !curMsg->HasBool("dontreport") ) {

		// Tell the main app that the window has been closed
		BMessage* sendMessage = new BMessage( BEAIM_IM_WINDOW_CLOSED );
		sendMessage->AddInt32( "wtype", (int32)USER_MESSAGE_TYPE );
		sendMessage->AddRect( "frame", Frame() );
		sendMessage->AddFloat( "divider", divider );
		sendMessage->AddString( "userid", otherName.UserString() );
		be_app->PostMessage( sendMessage );
	}
	return(true);
}

//-----------------------------------------------------
// The splitter bar has a tendency to go waaaaaaaay out of whack sometimes and
// end up off the edge of the window, so one of the textviews becomes invisible
// (as does the splitter bar), leaving no way to fix it. This function verifies
// that the divider position is within range and fixes it if it isn't.

void ChatWindow::VerifyDivider( bool fix ) {

	bool needsFixing = false;

	// if divider is too big or too small, then we have a problem
	if( divider < .0001 || divider > .9999 ) {
		needsFixing	= true;

		// use the old divider value if possible, otherwise use the default
		if( oldDivider < .0001 || oldDivider > .9999 )
			divider = DEFAULT_DIVIDER_POS;
		else
			divider = oldDivider;
	}

	// now save the verified-as-good divider in case the next resize messes things up
	oldDivider = divider;

	// if we're not supposed to fix it (or it doesn't need fixing) then we're done
	if( !needsFixing || !fix )
		return;

	// let FrameResized() handle all the heavy lifting of actually fixing stuff
	BRect rect = Bounds();
	FrameResized( rect.Width(), rect.Height() );
}

//-----------------------------------------------------

float ChatWindow::Divider() {
	return divider;
}

//-----------------------------------------------------

void ChatWindow::SetCanWarn( bool cw ) {

	canWarn = cw;
	warnItem->SetEnabled( canWarn );
}

//-----------------------------------------------------

void ChatWindow::MenusBeginning() {

	// Adjust the Edit menu... if the read-only TextView is selected, Cut, Paste and Undo
	// should be disabled. Also, the targets on Copy/SelectAll need be correct.

	if( textview->IsFocus() ) {
		miSelectAll->SetTarget( textview );
		miCopy->SetTarget( textview );
		miCut->SetEnabled( false );
		miPaste->SetEnabled( false );
		miUndo->SetEnabled( false );
	}

	if( editview->IsFocus() ) {
		miSelectAll->SetTarget( editview );
		miCopy->SetTarget( editview );
		miCut->SetEnabled( true );
		miPaste->SetEnabled( true );
		miUndo->SetEnabled( true );
	}
}

//-----------------------------------------------------

void ChatWindow::DoBlockStuff( BMessage* msg ) {

	if( msg->HasBool("alreadydone") ) {
		if( msg->FindBool("block") )
			blockItem->SetLabel( Language.get("CW_UNBLOCK") );
		else
			blockItem->SetLabel( Language.get("CW_BLOCK") );
		return;
	}

	bool blocked = users->IsUserBlocked(otherName);
	BString message;

	// we're unlocking the user
	if( blocked ) {
		blockItem->SetLabel( Language.get("CW_BLOCK") );
		users->UnblockUser( otherName );
		message = BString( Language.get("BLE_GOT_UNBLOCKED") );
		message.ReplaceAll( "%USER", otherName.UserString() );
		windows->ShowMessage( (char*)message.String() );
	}

	// we're blocking the user
	else {
		blockItem->SetLabel( Language.get("CW_UNBLOCK") );
		users->BlockUser( otherName );
		message = BString( Language.get("BLE_GOT_BLOCKED") );
		message.ReplaceAll( "%USER", otherName.UserString() );
		windows->ShowMessage( (char*)message.String() );
	}

	// either way, we can't warn the other user right now
	SetCanWarn(false);
}

//-----------------------------------------------------

void ChatWindow::ProcessIncomingIM( BMessage* incomingIM )
{
	bool empty = textview->IsEmpty();
	bool wasAutoResponse = incomingIM->FindBool("autorespond");
	BString message;
	time_t now;
	unsigned short wL;

	// convert the message to a BString
	message = BString(incomingIM->FindString("message"));

	// make sure we have the most recent format of the other user's screen name
	otherName = AIMUser( (char*)incomingIM->FindString("userid") );

	// if this was an auto-responded message, do the var replacements
	if( wasAutoResponse )
		client->ReplaceTagVars(message);

	// automatically link the web and mail addresses that weren't linked
	Linkify( message );

	// save the message to the chat history thing
	time(&now);
	chatRecord addRec;
	addRec.message = message;
	addRec.time = now;
	addRec.wasFromOutside = true;
	addRec.wasAuto = wasAutoResponse;
	chatRecords.Add( addRec );

	// grab the warning level out of the message
	wL = (unsigned short)(incomingIM->FindInt32("warninglevel"));
	otherNameView->SetWarningLevel( wL );

	// Let AddStatementThingy do most of the real work
	AddStatementThingy( message, now, true, wasAutoResponse );

	// Play the sound
	if( empty )
		sounds->PlaySound( WS_NEWMSG );
	else
		sounds->PlaySound( WS_MSGRECEIVE );

	// Update the "last message" timestamp
	UpdateLastMessage();

	// do the autoresponse, if necessary
	if( client->AwayMode() != AM_NOT_AWAY )
		DoAutoRespond();

	// popup the window if the user has specified that option
	if( popupOnReceive )
		Popup();

	// otherwise, update the window title if we aren't active
	else if( prefixNewMessages && !IsActive() ) {
		char newTitle[DISPLAY_NAME_MAX+10];
		sprintf( newTitle, "> %s", otherName.UserString() );
		SetTitle( newTitle );
		hasNewMessage = true;
	}

	// a message just came in; you can warn them now
	SetCanWarn( true );
}

//-----------------------------------------------------

void ChatWindow::RebuildChatContents()
{
	chatRecord rec;
	bool kg;

	// first, clear out the current contents of the textview
	textview->Clear();

	// re-insert all the records into the chat window
	kg = chatRecords.First(rec);
	while( kg ) {
		AddStatementThingy( rec.message, rec.time, rec.wasFromOutside, rec.wasAuto );
		kg = chatRecords.Next(rec);
	}
}

//-----------------------------------------------------

void ChatWindow::AddStatementThingy( BString message, time_t time,
									 bool wasFromOutside, bool wasAutoResponse )
{
	HTMLParser parse;
	styleList styles;
	char theTime[100];

	// convert the message to UTF-8 based on whatever the current encoding is
	if( wasFromOutside )
		message = ConvertToUTF8( message, useEncoding );

	// first, parse the HTML
	parse.Parse( (char*)message.String() );
	styles = parse.Styles();

	// clear out the insert-mode stuff
	textview->ClearInsertStuff();

	// add the timestamp gizmo thingy
	if( showTimestamps ) {
		textview->SetFontSize( chatFont.Size() * 0.8 );
		textview->SetFontAttribute( B_BOLD_FACE, false );
		textview->SetFontColor( 100, 100, 100 );
		strftime( theTime, 100, "[%I:%M %p] ", localtime(&time) );
		textview->InsertSomeText( theTime );
		textview->ResetFontToBase();
	}

	// do the IRC-style /me replacement, if the user is into that sort of thing
	if( doIRCMeThing && DoIRCMeThing( BString(parse.ParsedString()), !wasFromOutside, wasAutoResponse) ) {
		styles.Clear();
	}

	// nope, just a normal message
	else {

		// get the styles and stuff ready to insert the sender's name
		textview->SetFontAttribute( B_BOLD_FACE, true );
		textview->SetFontColor( wasFromOutside ? kThat : kThis );

		// insert the auto-response line, if it was an auto-response
		if( wasAutoResponse ) {
			BString autoResponseString;
			autoResponseString = BString( LangWithSuffix("CW_AUTO_RESPONSE_FROM", ": ") );
			autoResponseString.ReplaceAll( "%USER",
											wasFromOutside ?
											otherName.UserString() :
											myName.UserString() );
			textview->InsertSomeText( (char*)autoResponseString.String() );
		}

		// insert the sender's screen name
		else {
			if( wasFromOutside )
				textview->InsertSomeText( (char*)otherName.UserString() );
			else
				textview->InsertSomeText( (char*)myName.UserString() );
			textview->InsertSomeText( ": " );
		}

		// insert the parsed HTML
		textview->ResetFontToBase();
		textview->AddStyledText( parse.ParsedString(), styles );
		styles.Clear();
	}

	// At last... finalize the insertion
	textview->AddStatement();
}

//-----------------------------------------------------

bool ChatWindow::DoIRCMeThing( BString msgText, bool wasMine, bool wasAuto ) {

	BString newText;
	BString theName;

	// if this string doesn't start with /me, then bail
	if( msgText.ICompare("/me ", 4) != 0 )
		return false;

	// figure out the correct name for this stuff
	theName = wasMine ? myName.Username() : otherName.Username();

	// OK, we have a winner! Do the necessary replacements.
	newText = BString("* ");
	newText.Append( theName );
	newText.Append( msgText.String() + 3 );

	// insert the action statement, all fixed up
	textview->SetFontColor( kAction );
	if( wasAuto ) {
		BString temp = "[";
		temp.Append( Language.get("CW_AUTO_RESPONSE_FROM") );
		temp.Append( "]\n" );
		temp.ReplaceAll( "%USER", theName.String() );
		textview->InsertSomeText( (char*)temp.String() );
	}
	textview->InsertSomeText( (char*)newText.String() );
	textview->ResetFontToBase();
	return true;
}

//-----------------------------------------------------

void ChatWindow::AddMyStatement( BString statement )
{
	time_t now;

	// will this be the first message in the window?
	if( textview->IsEmpty() )
		firstWasMe = true;

	// save the message to the chat history thing
	time(&now);
	chatRecord addRec;
	addRec.message = statement;
	addRec.time = now;
	addRec.wasFromOutside = false;
	addRec.wasAuto = false;
	chatRecords.Add(addRec);

	// Let AddStatementThingy do most of the real work
	AddStatementThingy( statement, now, false, false );

	// Update the "last message" timestamp
	UpdateLastMessage();
}

//-----------------------------------------------------

void ChatWindow::DoBuddyEvent( bool firstTime ) {
	BString actionStr;
	char theTime[50];
	eventStatus evStat;
	int status;

	// if this user isn't a buddy, ignore it, unless this is the first time
	if( userEventStatus == ES_NOT_A_BUDDY && !firstTime )
		return;

	// grab the status of this user... if it's -1, this is no longer a buddy
	status = users->GetBuddyStatus( otherName );
	if( status == -1 ) {
		userEventStatus = ES_NOT_A_BUDDY;
		return;
	}

	// figure out what the current status is
	if( status & BS_OFFLINE )
		evStat = ES_OFFLINE;
	else if( status & BS_AWAY )
		evStat = ES_AWAY;
	else
		evStat = ES_ONLINE;

	// if this is the first time, that's all we need to know...
	if( firstTime ) {
		userEventStatus = evStat;
		return;
	}

	// has the status changed at all, or was this just a "boring" update?
	if( userEventStatus == evStat )
		return;

	// make the status strings
	switch( evStat ) {

		case ES_OFFLINE:
			actionStr = Language.get("CW_EV_WENT_OFFLINE");
			break;

		case ES_ONLINE:
			if( userEventStatus != ES_AWAY )
				actionStr = Language.get("CW_EV_CAME_ONLINE");
			else
				actionStr = Language.get("CW_EV_CAME_BACK");
			break;

		case ES_AWAY:
			actionStr = Language.get("CW_EV_WENT_AWAY");
			break;

		case ES_NOT_A_BUDDY:		// to stop gcc from complaining
			break;
	}

	// make the time string
	time_t now;
	time(&now);
	strftime( theTime, 100, "%I:%M %p", localtime(&now) );

	// do the substitutions
	actionStr.ReplaceAll( "%USER", otherName.UserString() );
	actionStr.ReplaceAll( "%TIME", theTime );

	// insert some fun stuff, eh?
	textview->ClearInsertStuff();
	textview->SetFontAttribute( B_BOLD_FACE, true );
	textview->SetFontColor( kEvent );
	textview->InsertSomeText( (char*)actionStr.String() );
	textview->ResetFontToBase();
	textview->AddStatement();

	// save the new status for prosperity
	userEventStatus = evStat;
}

//-----------------------------------------------------

void ChatWindow::SaveTranscript( BMessage* saveMsg ) {

	FILE* out;
	chatRecord rec;
	char theTime[100];
	HTMLParser parse;
	bool kg, done;
	entry_ref theRef;
	BEntry theEntry;
	BPath thePath;
	BString pathName;

	// get the pathname of where we're saving this sucker
	saveMsg->FindRef( "directory", &theRef );
	theEntry.SetTo( &theRef );
	theEntry.GetPath( &thePath );
	pathName = BString( thePath.Path() );
	pathName.Append( "/" );
	pathName.Append( saveMsg->FindString("name") );

	// open the file for output
	out = fopen( pathName.String(), "w" );
	if( !out ) {
		windows->ShowMessage( Language.get("ERR_NO_SAVE_TRANSCRIPT"), B_STOP_ALERT );
		return;
	}

	// write out a header
	fprintf( out, "<html><head><title>%s: %s</title></head>\n", Language.get("TRANSCRIPT_LABEL"), otherName.UserString() );
	fprintf( out, "<body bgcolor=\"white\">\n" );

	kg = chatRecords.First(rec);
	while( kg ) {
		done = false;

		// write out the timestamp
		strftime( theTime, 100, "[%I:%M:%S %p] ", localtime(&rec.time) );
		fprintf( out, "<font size=\"-2\" color=\"#444444\">%s</font>\n", theTime );

		// if they've turned on the IRC stuff, then try and write it out
		if( doIRCMeThing ) {
			BString finalString;
			parse.Parse( (char*)rec.message.String() );
			BString parsed = BString(parse.ParsedString());
			if( parsed.ICompare("/me ", 4) == 0 ) {
				BString theName = !rec.wasFromOutside ? myName.Username() : otherName.Username();
				if( rec.wasAuto ) {
					finalString = "[";
					finalString.Append( Language.get("CW_AUTO_RESPONSE_FROM") );
					finalString.Append( "]<br>\n" );
					finalString.ReplaceAll( "%USER", theName.String() );
				}
				finalString.Append( "* " );
				finalString.Append( theName );
				finalString.Append( parse.ParsedString() + 3 );
				fprintf( out, "<font size=\"-1\" color=\"%s\">%s</font>", ColorToString(kAction).String(), finalString.String() );
				done = true;
			}
		}

		// if the IRC thing didn't work, do it the normal way
		if( !done ) {

			// partially parse the string, to get the <html> and <body> tags outta there
			parse.Parse( (char*)rec.message.String(), false );

			// do the autoresponses and all
			if( rec.wasAuto ) {
				BString autoResponseString;
				autoResponseString = BString( LangWithSuffix("CW_AUTO_RESPONSE_FROM", ": ") );
				autoResponseString.ReplaceAll( "%USER",
												rec.wasFromOutside ?
												otherName.UserString() :
												myName.UserString() );
				if( rec.wasFromOutside )
					fprintf( out, "<font size=\"-1\" color=\"%s\"><b>%s</b></font>", ColorToString(kThat).String(), autoResponseString.String() );
				else
					fprintf( out, "<font size=\"-1\" color=\"%s\"><b>%s</b></font>", ColorToString(kThis).String(), autoResponseString.String() );
			}

			// do the non-auto-responses
			else {
				// print out the sender's username
				if( rec.wasFromOutside )
					fprintf( out, "<font size=\"-1\" color=\"%s\"><b>%s: </b></font>", ColorToString(kThat).String(), otherName.UserString() );
				else
					fprintf( out, "<font size=\"-1\" color=\"%s\"><b>%s: </b></font>", ColorToString(kThis).String(), myName.UserString() );
			}

			// print out the message
			fprintf( out, parse.ParsedString() );
		}

		// next chat record thing
		fprintf( out, "<br>\n" );
		kg = chatRecords.Next(rec);
	}

	fprintf( out, "</body></html>" );

	// close it again
	fclose(out);
}

//-----------------------------------------------------

void ChatWindow::Popup() {

	// Check to see if this window is already displayed in all workspaces
	// if not, move it to the current workspace
	if( !GetAllWorkspaces() )
		SetWorkspaces( B_CURRENT_WORKSPACE );

	// if the window is hidden, pop it up
	if( IsHidden() || !IsFront() ) {
		Show();
		Activate();
	}
}

//-----------------------------------------------------

void ChatWindow::LoadPrefs() {

	BString colTemp, defTemp;
	rgb_color textCol;

	prefixNewMessages = prefs->ReadBool( "PrefixNewMessages", false );
	enterIsNewline = prefs->ReadBool( "EnterInsertsNewline", false );
	tabIsTab = prefs->ReadBool( "TabIsTab", false );
	doIRCMeThing = prefs->ReadBool( "IRCMeThing", true );

	float textMag = float(prefs->ReadInt32("TextMagnification",100)) / 100;
	textview->SetTextMagnification( textMag );

	prefs->ReadString( "ChatBGColor", colTemp, defTemp = "#FFFFFF" );
	textview->SetViewColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	editview->SetViewColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "ChatFontColor", colTemp, defTemp = "#000000" );
	textCol = isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp);
	prefs->ReadString( "ChatFromYouColor", colTemp, defTemp = "#FF0000" );
	kThis = isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp);
	prefs->ReadString( "ChatToYouColor", colTemp, defTemp = "#0000FF" );
	kThat = isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp);
	prefs->ReadString( "IRCMeThingColor", colTemp, defTemp = "#5707A9" );
	kAction = isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp);
	prefs->ReadString( "BuddyEventColor", colTemp, defTemp = "#006E6E" );
	kEvent = isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp);

	if( !IsHidden() && !IsMinimized() ) {
		textview->Invalidate();
		editview->Invalidate();
	}

	chatFont.SetSize( 12.0 );
	textview->SetBaseFontAndColor( chatFont, textCol );
	chatFont.SetSize( 12.0 * textMag );
	editview->SetBaseFontAndColor( chatFont, textCol );

	// may as well rebuild the chat window contents too
	RebuildChatContents();
}

//-----------------------------------------------------

void ChatWindow::SetStyles( BMessage* message ) {
/*
	int32 start, finish, attrib;
	text_run_array* curStyles = NULL;
	BFont font, font2;
	uint16 curMask;

	// Get the current selection, and the current styles for that selection
	editview->GetSelection( &start, &finish );
	curStyles = editview->RunArray( start, finish );
	font = curStyles->runs[0].font;
	curMask = font.Face();

	bool on;

	switch( message->what ) {

		// set the font back to the normal font
		case CHATWINDOW_FONT_NORMAL:
			font = editview->GetBaseFont();
			editview->SetFontAndColor( start, finish, &font );
			break;

		// make it italic
		case CHATWINDOW_FONT_ITALIC:
			if( message->what == CHATWINDOW_FONT_ITALIC )
				attrib = B_ITALIC_FACE;

			editview->GetFontAndColor( start, &font2 );

			on = !(curMask & attrib);

			on = true;

				for( int i = 0; i < curStyles->count; ++i ) {
					curMask = curStyles->runs[i].font.Face();
					if( on ) curMask = ((curMask & B_REGULAR_FACE) ? 0 : curMask) | attrib;
					else curMask = (curMask == attrib) ? B_REGULAR_FACE : curMask & (~attrib);
					curStyles->runs[i].font.SetFace( curMask );
				}
				editview->SetRunArray( start, finish, curStyles );
				break;

		// make it bold
		case CHATWINDOW_FONT_BOLD:
			break;

		// font size change
		default:
			font.SetSize( ConvertFontSize(message->what - CHATWINDOW_FONT_BASE) );
			editview->SetFontAndColor( start, finish, &font );
			break;
	}

	delete curStyles;
	*/
}

//-----------------------------------------------------

void ChatWindow::AddPerson( BMessage* msg ) {
	windows->MakeAddBuddyWindow( otherName, this, true );
}

//-----------------------------------------------------

void ChatWindow::UpdateLastMessage() {

	char theTime[100];

	int rnd = (int)(random() % 1979);
	if( rnd == 42 )
		lastView->SetText( "Run! The squirrels are coming!" );
	else {
		time_t now;
		time(&now);
		strftime( theTime, 100, "%b %e, %Y (%I:%M %p)", localtime(&now) );
		lastView->SetText( theTime );
	}
}

//-----------------------------------------------------

void ChatWindow::SendMessage() {

	BString message;
	BMessage* sendMessage;

	// don't send if nothing is there
	if( editview->IsEmpty() )
		return;

	// grab the message
	message = BString( editview->GetFormattedMessage() );

	// "linkify" the message
	Linkify( message );

	// add the message to the textbox thinger
	AddMyStatement( message );
	editview->SetText( "" );
	editview->ResetFontToBase();

	// play the sound
	sounds->PlaySound( WS_MSGSEND );

	// convert it using whatever encoding, and send it
	message = ConvertFromUTF8( message, useEncoding );
	sendMessage = new BMessage(BEAIM_SEND_MESSAGE);
	sendMessage->AddString( "send_to", otherName.UserString() );
	sendMessage->AddString( "message", message.String() );
	sendMessage->AddBool( "autorespond", false );
	PostAppMessage( sendMessage );
}

//-----------------------------------------------------

void ChatWindow::DoAutoRespond()
{
	BMessage* sendMessage;
	BString text;
	BString preText;
	time_t now;

	// if we already *did* an autorespond, don't do it again
	if( didAutoRespond || firstWasMe )
		return;

	// get the autorespond message, translate the vars, and wrap some nice blue text around it
	text = client->CurrentAwayMessage();
	text.Prepend( "<html><font color=\"#0000ff\">" );
	text.ReplaceAll( "%n", otherName.UserString() );
	client->ReplaceTagVars( text );
	text.Append( "</font></html>" );

	// linkify the text
	Linkify(text);

	// save the message to the chat history thing
	time(&now);
	chatRecord addRec;
	addRec.message = text;
	addRec.time = now;
	addRec.wasFromOutside = false;
	addRec.wasAuto = true;
	chatRecords.Add( addRec );

	// convert it to whatever encoding we're using
	text = ConvertFromUTF8( text, useEncoding );

	// send off the auto-respond message
	sendMessage = new BMessage(BEAIM_SEND_MESSAGE);
	sendMessage->AddString( "send_to", otherName.UserString() );
	sendMessage->AddString( "message", text.String() );
	sendMessage->AddBool( "autorespond", true );
	PostAppMessage( sendMessage );

	// Let AddStatementThingy do most of the real work
	AddStatementThingy( text, now, false, true );

	// play the sound
	sounds->PlaySound( WS_MSGSEND );

	// Update the "last message" timestamp and set the autoresponded flag
	UpdateLastMessage();
	didAutoRespond = true;
}

//-----------------------------------------------------

void ChatWindow::WindowActivated( bool activated ) {

	// if we are getting re-activated and there is a new message,
	//  then take out the '>' from the titlebar
	if( activated && hasNewMessage ) {
		SetTitle( (char*)otherName.UserString() );
		hasNewMessage = false;
	}
}

//-----------------------------------------------------

void ChatWindow::EnableWindow( bool enabled ) {

	// make the "grayed out" color for the text edit thingers
	rgb_color grayedOut;
	grayedOut.red = grayedOut.green = grayedOut.blue = 210;

	// enable/disable the window
	windowEnabled = enabled;
	if( windowEnabled ) {

		// enable the various UI components
		EnableSendButton();
		editview->MakeEditable(true);
		KeyMenuBar()->SetEnabled(true);

		// loading the prefs will do most of the hard stuff...
		LoadPrefs();
	}

	// disable the window
	else {

		// disable the various UI components
		EnableSendButton();
		editview->MakeEditable(false);
		KeyMenuBar()->SetEnabled(false);

		// gray out the text editors
		textview->SetViewColor( grayedOut );
		editview->SetViewColor( grayedOut );
		if( !IsHidden() && !IsMinimized() ) {
			textview->Invalidate();
			editview->Invalidate();
		}
	}
}

//-----------------------------------------------------

void ChatWindow::EnableSendButton() {
	if( windowEnabled ) {
		if( editview->TextLength() ) {
			if( !btView->sendButton->IsEnabled() )
				btView->sendButton->SetEnabled( true );
		} else {
			if( btView->sendButton->IsEnabled() )
				btView->sendButton->SetEnabled( false );
		}
	} else {
		if( btView->sendButton->IsEnabled() )
			btView->sendButton->SetEnabled( false );
	}
}

//-----------------------------------------------------

void ChatWindow::SetEncoding( uint32 encoding ) {

	Lock();

	// set the encoding
	useEncoding = encoding;

	// interpret the default decoding
	if( encoding == DEFAULT_ENCODING_CONSTANT )
		encoding = (uint32)prefs->ReadInt32("DefaultEncoding", B_MS_WINDOWS_CONVERSION);

	// enable/disable the menu items
	switch( encoding ) {
		case B_ISO2_CONVERSION:
			e1->SetMarked(true);
			break;
		case B_ISO5_CONVERSION:
			e2->SetMarked(true);
			break;
		case B_KOI8R_CONVERSION:
			e3->SetMarked(true);
			break;
		case B_MS_DOS_866_CONVERSION:
			e4->SetMarked(true);
			break;
		case B_MS_WINDOWS_1251_CONVERSION:
			e5->SetMarked(true);
			break;
		case B_ISO7_CONVERSION:
			e6->SetMarked(true);
			break;
		case B_SJIS_CONVERSION:
			e7->SetMarked(true);
			break;
		case B_EUC_CONVERSION:
			e8->SetMarked(true);
			break;
		case B_UNICODE_CONVERSION:
			e9->SetMarked(true);
			break;
		case B_ISO1_CONVERSION:
			e10->SetMarked(true);
			break;
		case B_MAC_ROMAN_CONVERSION:
			e11->SetMarked(true);
			break;
		case B_MS_WINDOWS_CONVERSION:
			e12->SetMarked(true);
			break;
	}

	Unlock();
}

//=====================================================

BottomView::BottomView( BRect frame )
		   : BView( frame, "bottom_view", B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT,
		   								  B_WILL_DRAW | B_NAVIGABLE_JUMP )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );

	BRect btnFrame = Bounds();
	btnFrame.right -= 7;
	btnFrame.left = btnFrame.right - 60;
	btnFrame.top = 7;
	btnFrame.bottom = 27;
	sendButton = new BButton( btnFrame, "send_button", "Send", new BMessage(BEAIM_ATTEMPT_SEND_IM),
										B_FOLLOW_RIGHT | B_FOLLOW_TOP, B_NAVIGABLE | B_WILL_DRAW );
	sendButton->MakeDefault( true );

	AddChild( sendButton );
}

//-----------------------------------------------------

void BottomView::Draw( BRect updateRect ) {

	BRect frame = Bounds();

	SetHighColor( 156, 154, 156 );		// dark grey
	BPoint start = frame.LeftTop();
	BPoint end = frame.RightTop();
	StrokeLine( start, end );
}

//=====================================================

PointerStringView::PointerStringView( BRect frame, const char *name, const char *text,
					   uint32 resizingMode,
					   uint32 flags )
				 : BStringView( frame, name, text, resizingMode, flags )
{
}

//-----------------------------------------------------

void PointerStringView::MouseMoved( BPoint where, uint32 code, const BMessage *msg ) {

	be_app->SetCursor(B_HAND_CURSOR);
	BStringView::MouseMoved(where, code, msg);
}

//=====================================================
