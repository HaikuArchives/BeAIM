#include <Window.h>
#include <TabView.h>
#include <Box.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <SupportKit.h>
#include <MenuField.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "PrefsWindow.h"
#include "DLanguageClass.h"
#include "constants.h"
#include "Globals.h"
#include "MiscStuff.h"
#include "ColorPicker.h"
#include "Say.h"

//=====================================================

const uint32 PREFS_PROXYMODE_CHANGED = 'pRxC';
const uint32 PREFS_NEEDSAUTH_CHANGED = 'nDaC';
const uint32 PREFS_MAG_SIZE_CHANGED = 'mgCH';
const uint32 PREFS_TEMP_THINGY = 'temp';
const int32 NUM_PANELS = 5;

//=====================================================

int CompareStrings( const void* a, const void* b ) {
	BString* A = (BString*)a;
	BString* B = (BString*)b;

	if( *A < *B )
		return -1;
	if( *A == *B )
		return 0;
	if( *A == *B )
		return 1;
}

//=====================================================

GenPrefView::GenPrefView(BRect rect, bool go)
	   	   : BView(rect, "GenPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	globalOnly = go;

	// make a label
	globalLabel = new BStringView( BRect(4,4,400,21), "", "Global Settings:" );
	globalLabel->SetFont( be_bold_font );
	globalLabel->SetFontSize( 12.0 );
	AddChild( globalLabel );

	// create the language list
	CreateLanguageList();
	LanguageList = new BMenuField( BRect(12,22,rect.Width(),0), "languagelist", "Language:", LanguageMenu );
	LanguageList->ResizeToPreferred();
	LanguageList->SetDivider( 150 );
	AddChild( LanguageList );

	// (possibly) add the lang-debugging "Apply Lang" button
	if( modifiers() & B_SHIFT_KEY ) {
		AddChild( new BButton(BRect(Frame().Width()-85,22,Frame().Width()-5,0), "", "Apply Lang", new BMessage(PREFS_RL_LANG)) );
	}

	// Show the BeAIM window entry in the deskbar
	BRect tempRect = BRect(12,46,20,46+16);
	ShowDeskbarEntry = new BCheckBox( tempRect, "", "Show BeAIM window entry in the deskbar",  NULL );
	AddChild( ShowDeskbarEntry );

	// make a label
	userSpecLabel = new EnStringView( BRect(4,80,400,97), "", "User-specific Settings:" );
	userSpecLabel->SetFont( be_bold_font );
	userSpecLabel->SetFontSize( 12.0 );
	userSpecLabel->SetEnabled( !globalOnly );
	AddChild( userSpecLabel );

	// create the encodings list
	CreateEncodingsList();
	EncodingsList = new BMenuField( BRect(12,98,rect.Width(),0), "encodingslist", "Default Encoding:", EncodingsMenu );
	EncodingsList->ResizeToPreferred();
	EncodingsList->SetDivider( 150 );
	EncodingsList->SetEnabled( !globalOnly );
	AddChild( EncodingsList );

	// Show Buddy list in all workspaces
	BRect CheckRect = BRect(12,122,20,122+16);
	BuddyListAllWorkspaces = new BCheckBox( CheckRect, "", "Show the Buddy List in all workspaces",  NULL );
	BuddyListAllWorkspaces->SetEnabled( !globalOnly );
	AddChild( BuddyListAllWorkspaces );

	// Play sound effects or not
	CheckRect.top += 20;
	PlaySounds = new BCheckBox( CheckRect, "", "Play sound effects",  NULL );
	PlaySounds->SetEnabled( !globalOnly );
	AddChild( PlaySounds );

	// enable the BeAIM Deskbar Icon
	CheckRect.top += 20;
	UseDeskbar = new BCheckBox( CheckRect, "", "Enable BeAIM deskbar icon",  NULL );
	UseDeskbar->SetEnabled( !globalOnly );
	AddChild( UseDeskbar );

	// Now initialize them
	BuddyListAllWorkspaces->SetValue( prefs->ReadBool("BuddyListAllWorkspaces", true) );
	PlaySounds->SetValue( prefs->ReadBool("PlaySounds", true) );
	UseDeskbar->SetValue( prefs->ReadBool("UseDeskbarIcon", true) );
	ShowDeskbarEntry->SetValue( prefs->ReadBool("ShowDeskbarWindowEntry",true,true) );
	SetEncoding( (uint32)prefs->ReadInt32("DefaultEncoding", B_MS_WINDOWS_CONVERSION) );
}

//-----------------------------------------------------

void GenPrefView::CreateLanguageList() {

	char fileName[B_FILE_NAME_LENGTH];
	BString* addName;
	BDirectory langDir;
	BString langDirName;
	BList fileList;
	BEntry thang;
	BMessage* msg;

	// get the languages directory
	langDirName = GetAppDir();
	langDirName.Append( "Languages" );
	langDir.SetTo( langDirName.String() );

	// make sure the directory got created A-OK
	if( langDir.InitCheck() != B_OK )
		return;

	// create the pop up menu
	LanguageMenu = new BPopUpMenu(Language.Name());
	prospectiveLang = BString(Language.Name());

	// rewind the directory and get the names and stuff
	langDir.Rewind();
	while( langDir.GetNextEntry(&thang) == B_OK ) {
		thang.GetName( fileName );
		addName = new BString(fileName);
		fileList.AddItem( (void*)addName );
	}

	// sort the items
	fileList.SortItems( CompareStrings );

	// now... remove and delete the items, and build the menu
	addName = (BString*)fileList.RemoveItem((int32)0);
	while( addName ) {
		msg = new BMessage(PREFS_SET_PROSP_LANG);
		msg->AddString("prosplang", addName->String());
		LanguageMenu->AddItem( new BMenuItem(addName->String(), msg) );
		delete addName;
		addName = (BString*)fileList.RemoveItem((int32)0);
	}
}

//-----------------------------------------------------

void GenPrefView::CreateEncodingsList() {

	// create the pop up menu
	EncodingsMenu = new BPopUpMenu( "Encoding" );
	EncodingsMenu->SetRadioMode( true );
	BMessage* encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_ISO2_CONVERSION );
	EncodingsMenu->AddItem(e1 = new BMenuItem("Central European (ISO 8859-2)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_ISO5_CONVERSION );
	EncodingsMenu->AddItem(e2 = new BMenuItem("Cyrillic (ISO 8859-5)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_KOI8R_CONVERSION );
	EncodingsMenu->AddItem(e3 = new BMenuItem("Cyrillic (KOI8-R)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_MS_DOS_866_CONVERSION );
	EncodingsMenu->AddItem(e4 = new BMenuItem("Cyrillic (MS-DOS 866)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_MS_WINDOWS_1251_CONVERSION );
	EncodingsMenu->AddItem(e5 = new BMenuItem("Cyrillic (Windows 1251)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_ISO7_CONVERSION );
	EncodingsMenu->AddItem(e6 = new BMenuItem("Greek (ISO 8859-7)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_SJIS_CONVERSION );
	EncodingsMenu->AddItem(e7 = new BMenuItem("Japanese (Shift-JIS)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_EUC_CONVERSION );
	EncodingsMenu->AddItem(e8 = new BMenuItem("Japanese (EUC)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_UNICODE_CONVERSION );
	EncodingsMenu->AddItem(e9 = new BMenuItem("Unicode", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_ISO1_CONVERSION );
	EncodingsMenu->AddItem(e10 = new BMenuItem("Western (ISO 8859-1)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_MAC_ROMAN_CONVERSION );
	EncodingsMenu->AddItem(e11 = new BMenuItem("Western (Mac Roman)", encMsg) );
	encMsg = new BMessage(PREFS_TEMP_THINGY);
	encMsg->AddInt32( "encoding", (int32)B_MS_WINDOWS_CONVERSION );
	EncodingsMenu->AddItem(e12 = new BMenuItem("Western (MS Windows)", encMsg) );
}

//-----------------------------------------------------

void GenPrefView::SetEncoding( uint32 enc ) {

	// set the encoding
	encoding = enc;
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
}

//-----------------------------------------------------

void GenPrefView::Save() {

	if( (bool)ShowDeskbarEntry->Value() != prefs->ReadBool("ShowDeskbarWindowEntry",true,true) ) {
		char message[100];
		bool showEntry = (bool)ShowDeskbarEntry->Value();
		prefs->WriteBool("ShowDeskbarWindowEntry", showEntry, true );
		sprintf( message, "The BeAIM window entry in the deskbar will not be %s until you restart BeAIM.",
						  showEntry ? "shown" : "hidden" );
		Say( message );
	}

	// save the prefs
	if( !globalOnly ) {
		prefs->WriteBool("BuddyListAllWorkspaces", BuddyListAllWorkspaces->Value() );
		prefs->WriteBool("PlaySounds", PlaySounds->Value() );
		prefs->WriteBool("UseDeskbarIcon", UseDeskbar->Value() );
		prefs->WriteInt32("DefaultEncoding", (int32)encoding );
	}
}

//-----------------------------------------------------

void GenPrefView::RefreshLangStrings() {

	EncodingsList->SetLabel( Language.get("PREFS_GDEFENC") );
	LanguageList->SetLabel( Language.get("PREFS_GLANG") );
	BuddyListAllWorkspaces->SetLabel( Language.get("PREFS_GSBLAW") );
	BuddyListAllWorkspaces->ResizeToPreferred();
	PlaySounds->SetLabel( Language.get("PREFS_GPSE") );
	PlaySounds->ResizeToPreferred();
	UseDeskbar->SetLabel( Language.get("PREFS_GEBDI") );
	UseDeskbar->ResizeToPreferred();
	ShowDeskbarEntry->SetLabel( Language.get("PREFS_GSBWE") );
	ShowDeskbarEntry->ResizeToPreferred();
	globalLabel->SetText( Language.get("PREFS_GGLOBAL") );
	globalLabel->ResizeToPreferred();
	userSpecLabel->SetText( Language.get("PREFS_GUSERSPEC") );
	userSpecLabel->ResizeToPreferred();
}

//=====================================================

ConnectionPrefView::ConnectionPrefView(BRect rect, bool go)
	   	   : BView(rect, "ConPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );

	char tmpCharString[50];
	BString tmpString;
	int32 tmpInt32;
	globalOnly = go;

	// Add the server group box
	serverBox = new BBox( BRect(7,2,230,78) );
	serverBox->SetLabel("AIM Server");
	AddChild( serverBox );

	// set up the server text boxes
	conHost = new BTextControl( BRect(10,20,205,40), "hostname", "Server:", "", NULL );
	conHost->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	conHost->SetDivider( 70 );
	conPort = new BTextControl( BRect(10,45,205,65), "portname", "Port:", "", NULL );
	conPort->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	conPort->SetDivider( 70 );
	serverBox->AddChild( conHost );
	serverBox->AddChild( conPort );

	// Add the proxy mode group box
	proxyModeBox = new BBox( BRect(238,4,Bounds().Width()-8,82) );
	proxyModeBox->SetLabel("Proxy Mode");
	AddChild( proxyModeBox );

	// add the radio button stuff
	proxyNo = new BRadioButton( BRect(8,17,140,0), "proxyno", "No Proxy Server", new BMessage(PREFS_PROXYMODE_CHANGED) );
	proxyHTTPS = new BRadioButton( BRect(8,35,140,0), "proxyhttps", "HTTPS Proxy Server", new BMessage(PREFS_PROXYMODE_CHANGED) );
	proxySOCKS5 = new BRadioButton( BRect(8,53,140,0), "proxysocks", "SOCKS5 Proxy Server", new BMessage(PREFS_PROXYMODE_CHANGED) );
	proxyModeBox->AddChild( proxyNo );
	proxyModeBox->AddChild( proxyHTTPS );
	proxyModeBox->AddChild( proxySOCKS5 );

	// Add the proxy settings group box
	proxySettingsBox = new BBox( BRect(8,90,Bounds().Width()-8,186) );
	proxySettingsBox->SetLabel("Proxy Settings");
	AddChild( proxySettingsBox );

	// set up the server text boxes
	proxyHost = new BTextControl( BRect(10,20,175,40), "proxyhost", "Server:", "", NULL );
	proxyHost->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	proxyHost->SetDivider( 70 );
	proxyPort = new BTextControl( BRect(10,45,175,65), "proxyport", "Port:", "", NULL );
	proxyPort->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	proxyPort->SetDivider( 70 );
	proxySettingsBox->AddChild( proxyHost );
	proxySettingsBox->AddChild( proxyPort );

	// setup the authorization info stuff
	needsAuth = new BCheckBox( BRect(199,17,378,0), "needsauth", "Authentication required", new BMessage(PREFS_NEEDSAUTH_CHANGED) );
	authUser = new BTextControl( BRect(199,40,375,60), "authuser", "User:", "", NULL );
	authUser->SetDivider( 80 );
	authPass = new PassControl( BRect(199,65,375,80), "authpass", "Pass:", "", NULL );
	authPass->SetDivider( 80 );
	proxySettingsBox->AddChild( needsAuth );
	proxySettingsBox->AddChild( authUser );
	proxySettingsBox->AddChild( authPass );

	// add the silly warning about how HTTPS can boot ya
	httpsWarning = new BStringView( BRect(7,187,375,97), "httpswarning",
		"Note: Some HTTPS proxy servers will disconnect BeAIM without warning." );
	AddChild( httpsWarning );

	// now initialize everything
	prefs->ReadString( "AIMHost", tmpString, "login.oscar.aol.com", true, false );
	conHost->SetText( tmpString.String() );
	tmpInt32 = prefs->ReadInt32( "AIMPort", 5190, true );
	sprintf( tmpCharString, "%lu", tmpInt32 );
	conPort->SetText( tmpCharString );
	prefs->ReadString( "ProxyHost", tmpString, "127.0.0.1", true, false );
	proxyHost->SetText( tmpString.String() );
	tmpInt32 = prefs->ReadInt32( "ProxyPort", 80, true );
	sprintf( tmpCharString, "%lu", tmpInt32 );
	proxyPort->SetText( tmpCharString );
	needsAuth->SetValue( prefs->ReadBool( "ProxyAuth", false, true ) );
	prefs->ReadString( "ProxyUser", tmpString, "ooga", true, false );
	authUser->SetText( tmpString.String() );
	prefs->ReadString( "ProxyPass", tmpString, "booga", true, false );
	authPass->SetText( tmpString.String() );

	// set the radio buttons
	switch( (netProxyMode)prefs->ReadInt32( "ProxyMode", (int32)NPM_NO_PROXY, true ) ) {
		case NPM_HTTPS_PROXY:
			proxyHTTPS->SetValue(1);
			break;
		case NPM_SOCKS5_PROXY:
			proxySOCKS5->SetValue(1);
			break;
		case NPM_NO_PROXY:
		default:
			proxyNo->SetValue(1);
			break;
	}

	// enable/disable stuff
	EnableProxyControls();
}

//-----------------------------------------------------

void ConnectionPrefView::Save() {

	int32 port = 0;
	if( proxyNo->Value() )
		prefs->WriteInt32( "ProxyMode", (int32)NPM_NO_PROXY, true );
	else if( proxyHTTPS->Value() )
		prefs->WriteInt32( "ProxyMode", (int32)NPM_HTTPS_PROXY, true );
	else if( proxySOCKS5->Value() )
		prefs->WriteInt32( "ProxyMode", (int32)NPM_SOCKS5_PROXY, true );
	prefs->WriteString( "AIMHost", BString(conHost->Text()), true );
	sscanf( (char*)conPort->Text(), "%lu", &port );
	prefs->WriteInt32( "AIMPort", port, true );
	prefs->WriteString( "ProxyHost", BString(proxyHost->Text()), true );
	sscanf( (char*)proxyPort->Text(), "%lu", &port );
	prefs->WriteInt32( "ProxyPort", port, true );
	prefs->WriteBool( "ProxyAuth", needsAuth->Value(), true );
	prefs->WriteString( "ProxyUser", BString(authUser->Text()), true );
	prefs->WriteString( "ProxyPass", BString(authPass->actualText()), true );
}

//-----------------------------------------------------

void ConnectionPrefView::EnableProxyControls() {

	// enable/disable stuff based on what the proxy settings are
	if( proxyNo->Value() ) {
		if( proxyHost->IsEnabled() )
			proxyHost->SetEnabled(false);
		if( proxyPort->IsEnabled() )
			proxyPort->SetEnabled(false);
		if( needsAuth->IsEnabled() )
			needsAuth->SetEnabled(false);
		if( authUser->IsEnabled() )
			authUser->SetEnabled(false);
		if( authPass->IsEnabled() )
			authPass->SetEnabled(false);
	}
	else {
		if( !proxyHost->IsEnabled() )
			proxyHost->SetEnabled(true);
		if( !proxyPort->IsEnabled() )
			proxyPort->SetEnabled(true);
		if( !needsAuth->IsEnabled() )
			needsAuth->SetEnabled(true);
		if( needsAuth->Value() ) {
			if( !authUser->IsEnabled() )
				authUser->SetEnabled(true);
			if( !authPass->IsEnabled() )
				authPass->SetEnabled(true);
		}
		else {
			if( authUser->IsEnabled() )
				authUser->SetEnabled(false);
			if( authPass->IsEnabled() )
				authPass->SetEnabled(false);
		}
	}

	// show/hide the warning about HTTPS servers based on whether
	// HTTPS is the current proxy mode
	if( proxyHTTPS->Value() ) {
		if( httpsWarning->IsHidden() )
			httpsWarning->Show();
	} else {
		if( !httpsWarning->IsHidden() )
			httpsWarning->Hide();
	}

	// change the default proxy port number based on what the current proxy mode is
	if( proxyHTTPS->Value() )
		proxyPort->SetText( "80" );
	else if( proxySOCKS5->Value() )
		proxyPort->SetText( "1080" );
}

//-----------------------------------------------------

void ConnectionPrefView::RefreshLangStrings() {

	// set the box labels
	serverBox->SetLabel( Language.get("PREFS_CAIMSERVER_LABEL") );
	proxyModeBox->SetLabel( Language.get("PREFS_CPROXYSERVER_LABEL") );
	proxySettingsBox->SetLabel( Language.get("PREFS_CPROXYSETINGS_LABEL") );

	// proxy modes
	proxyNo->SetLabel( Language.get("PREFS_CPROXYNONE") );
	proxyHTTPS->SetLabel( Language.get("PREFS_CPROXYHTTPS") );
	proxySOCKS5->SetLabel( Language.get("PREFS_CPROXYSOCKS5") );

	// stuffage
	conHost->SetLabel( Language.get("PREFS_CSERVER_NAME") );
	conPort->SetLabel( Language.get("PREFS_CPORT_NAME") );
	proxyHost->SetLabel( Language.get("PREFS_CSERVER_NAME") );
	proxyPort->SetLabel( Language.get("PREFS_CPORT_NAME") );
	authUser->SetLabel( Language.get("PREFS_CUSER_NAME") );
	authPass->SetLabel( Language.get("PREFS_CPASSWD_NAME") );
	needsAuth->SetLabel( Language.get("PREFS_CAUTH_REQD") );
	needsAuth->ResizeToPreferred();
	httpsWarning->SetText( Language.get("PREFS_CHTTPS_WARNING") );
	httpsWarning->ResizeToPreferred();
}

//=====================================================

ChatWindowPrefView::ChatWindowPrefView(BRect rect, bool go)
	   	   : BView(rect, "ConPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	BString colTemp, defTemp;
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	globalOnly = go;

	// Show chat windows in all workspaces
	BRect CheckRect = BRect(4,4,20,20);
	ChatWindowAllWorkspaces = new BCheckBox( CheckRect, "", "Show IM windows in all workspaces by default",  NULL );
	ChatWindowAllWorkspaces->ResizeToPreferred();
	ChatWindowAllWorkspaces->SetEnabled( !globalOnly );
	AddChild( ChatWindowAllWorkspaces );

	// Popup the chat window when a message is received
	CheckRect.top += 20;
	ChatWindowPopup = new BCheckBox( CheckRect, "", "Popup IM windows when messages are received by default",  NULL );
	ChatWindowPopup->ResizeToPreferred();
	ChatWindowPopup->SetEnabled( !globalOnly );
	AddChild( ChatWindowPopup );

	// Show font color and size attributes in the chat windows
	CheckRect.top += 20;
	ChatShowTimestamps = new BCheckBox( CheckRect, "", "Show timestamps for each message",  NULL );
	ChatShowTimestamps->ResizeToPreferred();
	ChatShowTimestamps->SetEnabled( !globalOnly );
	AddChild( ChatShowTimestamps );

	// Show font color and size attributes in the chat windows
	CheckRect.top += 20;
	ChatFontColorSizes = new BCheckBox( CheckRect, "", "Show font color and size attributes by default",  NULL );
	ChatFontColorSizes->ResizeToPreferred();
	ChatFontColorSizes->SetEnabled( !globalOnly );
	AddChild( ChatFontColorSizes );

	// Show URL/mailto links by default
	CheckRect.top += 20;
	ChatShowLinks = new BCheckBox( CheckRect, "", "Enable web/email links",  NULL );
	ChatShowLinks->ResizeToPreferred();
	ChatShowLinks->SetEnabled( !globalOnly );
	AddChild( ChatShowLinks );

	// Prefix incoming messages with '>' in the title bar
	CheckRect.top += 20;
	ChatPrefixNewMessages = new BCheckBox( CheckRect, "", "Prefix incoming messages with '>' in the title bar when window is inactive",  NULL );
	ChatPrefixNewMessages->ResizeToPreferred();
	ChatPrefixNewMessages->SetEnabled( !globalOnly );
	AddChild( ChatPrefixNewMessages );

	// Prefix incoming messages with '>' in the title bar
	CheckRect.top += 20;
	ChatIRCMeThing = new BCheckBox( CheckRect, "", "Display IRC-style /me actions",  NULL );
	ChatIRCMeThing->ResizeToPreferred();
	ChatIRCMeThing->SetEnabled( !globalOnly );
	AddChild( ChatIRCMeThing );

	// create the size-edit thang
	sizeEdit = new BTextControl( BRect(330,184,394,182), "blah!", "", "", new BMessage(PREFS_MAG_SIZE_CHANGED) );
	sizeEdit->SetEnabled( !globalOnly );
	sizeEdit->TextView()->SetMaxBytes(4);

	sizeEdit->SetDivider( 0 );

	// create the sample-size view
	BRect tRect1 = BRect( 335, 152, 393, 175 );
	BRect tRect2 = tRect1;
	tRect2.OffsetTo(0,0);
	tRect2.InsetBy(3,1);
	sampleSizeView = new BTextView( tRect1, "sampleview", tRect2, B_FOLLOW_NONE, B_WILL_DRAW );
	sampleSizeView->MakeEditable( false );
	sampleSizeView->MakeSelectable( false );
	sampleSizeView->SetText( "BeAIM" );
	sampleSizeView->SetViewColor( 230, 230, 230 );
	if( globalOnly ) {
		rgb_color gray;
		gray.red = gray.green = gray.blue = 128;
		sampleSizeView->SetFontAndColor( NULL, B_FONT_ALL, &gray );
	}

	// create the slider
	BRect slidRect = BRect( 6, 152, 323, 168 );
	magSlider = new GSlider( slidRect, 50, 150, sizeEdit, sampleSizeView );
	magSlider->SetEnabled( !globalOnly );

	// add stuff
	AddChild( magSlider );
	AddChild( new BScrollView( "scroll", sampleSizeView ) );
	AddChild( sizeEdit );

	// now initialize them
	ChatWindowAllWorkspaces->SetValue( prefs->ReadBool("ChatWindowAllWorkspaces", false) );
	ChatWindowPopup->SetValue( prefs->ReadBool("PopupOnMessage", false) );
	ChatFontColorSizes->SetValue( prefs->ReadBool("ShowFontColorSizes", true) );
	ChatShowLinks->SetValue( prefs->ReadBool("ShowLinks", true) );
	ChatPrefixNewMessages->SetValue( prefs->ReadBool( "PrefixNewMessages", false ) );
	ChatIRCMeThing->SetValue( prefs->ReadBool( "IRCMeThing", true ) );
	ChatShowTimestamps->SetValue( prefs->ReadBool( "ShowTimestamps", false ) );
	magSlider->SetValue( prefs->ReadInt32("TextMagnification", 100) );
}

//-----------------------------------------------------

void ChatWindowPrefView::Save() {

	// save the prefs
	if( !globalOnly ) {
		prefs->WriteBool("ChatWindowAllWorkspaces", ChatWindowAllWorkspaces->Value() );
		prefs->WriteBool("PopupOnMessage", ChatWindowPopup->Value() );
		prefs->WriteBool("ShowFontColorSizes", ChatFontColorSizes->Value() );
		prefs->WriteBool("ShowLinks", ChatShowLinks->Value() );
		prefs->WriteBool("PrefixNewMessages", ChatPrefixNewMessages->Value() );
		prefs->WriteBool("IRCMeThing", ChatIRCMeThing->Value() );
		prefs->WriteBool("ShowTimestamps", ChatShowTimestamps->Value() );
		prefs->WriteInt32("TextMagnification", magSlider->Value() );
	}
}

//-----------------------------------------------------

void ChatWindowPrefView::RefreshLangStrings() {

	ChatWindowAllWorkspaces->SetLabel( Language.get("PREFS_IMWINALLWS") );
	ChatWindowAllWorkspaces->ResizeToPreferred();
	ChatWindowPopup->SetLabel( Language.get("PREFS_IMPOPUPIMWIN") );
	ChatWindowPopup->ResizeToPreferred();
	ChatFontColorSizes->SetLabel( Language.get("PREFS_IMSHOWFONTCOL") );
	ChatFontColorSizes->ResizeToPreferred();
	ChatShowLinks->SetLabel( Language.get("PREFS_IMENABLELINKS") );
	ChatShowLinks->ResizeToPreferred();
	ChatPrefixNewMessages->SetLabel( Language.get("PREFS_IMPREFIXMSGS") );
	ChatPrefixNewMessages->ResizeToPreferred();
	ChatIRCMeThing->SetLabel( Language.get("PREFS_IMIRCMEACTIONS") );
	ChatIRCMeThing->ResizeToPreferred();
	ChatShowTimestamps->SetLabel( Language.get("PREFS_IMSHOWTIMESTAMPS") );
	ChatShowTimestamps->ResizeToPreferred();
	magSlider->SetLabel( Language.get("PREFS_IMTEXTMAG") );
}

//-----------------------------------------------------

void ChatWindowPrefView::MagSizeChanged() {

	BString magSize = sizeEdit->Text();
	BString test;
	bool good = true;
	int32 num;

	// strip all the spaces
	magSize.ReplaceAll( " ", "" );

	// take care of the percentages and stuff
	test = magSize;
	if( test[test.CountChars()-1] == '%' ) {
		test.RemoveAll( "%" );
		if( test.Length() != magSize.Length() - 1 ) {
			good = false;
		}
	}

	// now strip all the digits... there should be nothing left
	if( good ) {
		test.RemoveSet( "1234567890% " );
		if( test.Length() ) {
			good = false;
		}
	}

	// now do the number check...
	if( good ) {
		test = magSize;
		test.RemoveSet( " %" );
		num = atoi(test.String());
		if( num < 50 || num > 150 ) {
			good = false;
		}
	}

	// finally, complain if it was bad
	if( !good )	{
		Say( (char*)Language.get("ERR_BAD_MAG_VAL") );
		num = magSlider->Value();
	}

	// finally, replace the text with a nicely formatted percentage
	BString finalString = "";
	finalString << num;
	finalString.Append( "%" );
	sizeEdit->SetText( finalString.String() );
	magSlider->SetValue( num );
}

//=====================================================

AwayPrefView::AwayPrefView(BRect rect, bool go)
			: BView(rect, "AwayPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	globalOnly = go;

	// Show chat windows in all workspaces
/*	BRect CheckRect = BRect(4,4,20,20);
	ChatWindowAllWorkspaces = new BCheckBox( CheckRect, "", "Show chat windows in all workspaces by default",  NULL );
	ChatWindowAllWorkspaces->ResizeToPreferred();
	ChatWindowAllWorkspaces->SetEnabled( !globalOnly );
	AddChild( ChatWindowAllWorkspaces );

	// Popup the chat window when a message is received
	CheckRect.top += 20;
	ChatWindowPopup = new BCheckBox( CheckRect, "", "Popup chat windows when messages are received by default",  NULL );
	ChatWindowPopup->ResizeToPreferred();
	ChatWindowPopup->SetEnabled( !globalOnly );
	AddChild( ChatWindowPopup );*/

/*
	// now initialize them
	ChatWindowAllWorkspaces->SetValue( user->ReadBool("ChatWindowAllWorkspaces", false) );
	ChatWindowPopup->SetValue( user->ReadBool("PopupOnMessage", false) );
	ChatFontColorSizes->SetValue( user->ReadBool("ShowFontColorSizes", true) );
	ChatShowLinks->SetValue( user->ReadBool("ShowLinks", true) );
	ChatPrefixNewMessages->SetValue( user->ReadBool( "PrefixNewMessages", false ) );
	*/
}

//-----------------------------------------------------

void AwayPrefView::Save() {

	// save the prefs
	if( !globalOnly ) {
	/*	user->WriteBool("ChatWindowAllWorkspaces", ChatWindowAllWorkspaces->Value() );
		user->WriteBool("PopupOnMessage", ChatWindowPopup->Value() );
		user->WriteBool("ShowFontColorSizes", ChatFontColorSizes->Value() );
		user->WriteBool("ShowLinks", ChatShowLinks->Value() );
		user->WriteBool("PrefixNewMessages", ChatPrefixNewMessages->Value() );*/
	}
}

//-----------------------------------------------------

void AwayPrefView::RefreshLangStrings() {

}


//=====================================================

KeysPrefView::KeysPrefView(BRect rect, bool go)
			: BView(rect, "KeyPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	globalOnly = go;

	// make a label
	tabLabel = new EnStringView( BRect(4,4,400,21), "", ":" );
	tabLabel->SetFont( be_bold_font );
	tabLabel->SetFontSize( 12.0 );
	tabLabel->SetEnabled( !globalOnly );
	AddChild( tabLabel );

	// add some radio buttons
	tabIsTab = new BRadioButton( BRect(11,25,140,0), "tabistab", "", NULL );
	tabIsTab->SetEnabled( !globalOnly );
	AddChild( tabIsTab );
	tabIsFocusChange = new BRadioButton( BRect(11,43,140,0), "tabisfocus", "", NULL );
	tabIsFocusChange->SetEnabled( !globalOnly );
	AddChild( tabIsFocusChange );

	// make another label
	enterLabel = new EnStringView( BRect(4,4+70,400,21), "", ":" );
	enterLabel->SetFont( be_bold_font );
	enterLabel->SetFontSize( 12.0 );
	enterLabel->SetEnabled( !globalOnly );
	AddChild( enterLabel );

	// add a dummy view to make the radio buttons work right
	BView* addView = new BView( BRect(0,90,Bounds().Width(),132), "", B_FOLLOW_NONE, 0 );
	addView->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	AddChild( addView );

	// add some more radio buttons
	enterInsertsLineBreak = new BRadioButton( BRect(11,2,140,0), "enterislb", "", NULL );
	enterInsertsLineBreak->SetEnabled( !globalOnly );
	addView->AddChild( enterInsertsLineBreak );
	enterPressesDefButton = new BRadioButton( BRect(11,20,140,0), "enterisdb", "", NULL );
	enterPressesDefButton->SetEnabled( !globalOnly );
	addView->AddChild( enterPressesDefButton );

	// now initialize all that stuff
	bool tempVal = prefs->ReadBool( "TabIsTab", false );
	(tempVal ? tabIsTab : tabIsFocusChange)->SetValue( true );
	tempVal = prefs->ReadBool( "EnterInsertsNewline", false );
	(tempVal ? enterInsertsLineBreak : enterPressesDefButton)->SetValue( true );
}

//-----------------------------------------------------

void KeysPrefView::Save() {

	// save the prefs
	if( !globalOnly ) {
		prefs->WriteBool("EnterInsertsNewline", enterInsertsLineBreak->Value() );
		prefs->WriteBool("TabIsTab", tabIsTab->Value() );
	}
}

//-----------------------------------------------------

void KeysPrefView::RefreshLangStrings() {

	tabLabel->SetText( Language.get("PREFS_KTABFUNC") );
	tabLabel->ResizeToPreferred();
	tabIsTab->SetLabel( Language.get("PREFS_KTABISTAB") );
	tabIsTab->ResizeToPreferred();
	tabIsFocusChange->SetLabel( Language.get("PREFS_KTABISFS") );
	tabIsFocusChange->ResizeToPreferred();
	enterLabel->SetText( Language.get("PREFS_KENTERFUNC") );
	enterLabel->ResizeToPreferred();
	enterInsertsLineBreak->SetLabel( Language.get("PREFS_KENTERISLB") );
	enterInsertsLineBreak->ResizeToPreferred();
	enterPressesDefButton->SetLabel( Language.get("PREFS_KENTERISDEF") );
	enterPressesDefButton->ResizeToPreferred();
}

//=====================================================

ColorPrefView::ColorPrefView(BRect rect, bool go)
			: BView(rect, "ColorPrefsTab", B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW )
{
	EnStringView* tempSV;
	BString colTemp, defTemp;
	BRect sframe, cframe;
 	SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	globalOnly = go;

	// make a label
	tempSV = new EnStringView( BRect(4,4,400,21), "", "IM Window Colors:" );
	tempSV->SetFont( be_bold_font );
	tempSV->SetFontSize( 12.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe = BRect( 6, 28, 36, 43 );
	sframe = BRect( 43, 31, 264, 43 );
	bgColor = new ColorView( cframe, 1 );
	bgColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "Default background color" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( bgColor );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe.OffsetBy( 0, 20 );
	sframe.OffsetBy( 0, 20 );
	fontColor = new ColorView( cframe, 2);
	fontColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "Default font color" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( fontColor );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe.OffsetBy( 0, 20 );
	sframe.OffsetBy( 0, 20 );
	fromYouColor = new ColorView( cframe, 3);
	fromYouColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "Username: messages from you" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( fromYouColor );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe.OffsetBy( 0, 20 );
	sframe.OffsetBy( 0, 20 );
	toYouColor = new ColorView( cframe, 4);
	toYouColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "Username: messages to you" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( toYouColor );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe.OffsetBy( 0, 20 );
	sframe.OffsetBy( 0, 20 );
	eventColor = new ColorView( cframe, 5);
	eventColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "Events (when the buddy logs on/off)" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( eventColor );
	AddChild( tempSV );

	// Color, baby, yeah!
	cframe.OffsetBy( 0, 20 );
	sframe.OffsetBy( 0, 20 );
	actionColor = new ColorView( cframe, 6);
	actionColor->SetEnabled( !globalOnly );
	tempSV = new EnStringView( sframe, "", "IRC-style /me actions" );
	tempSV->SetFontSize( 10.0 );
	tempSV->SetEnabled( !globalOnly );
	AddChild( actionColor );
	AddChild( tempSV );

	// now initialize them
	prefs->ReadString( "ChatBGColor", colTemp, defTemp = "#FFFFFF" );
	bgColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "ChatFontColor", colTemp, defTemp = "#000000" );
	fontColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "ChatFromYouColor", colTemp, defTemp = "#FF0000" );
	fromYouColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "ChatToYouColor", colTemp, defTemp = "#0000FF" );
	toYouColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "IRCMeThingColor", colTemp, defTemp = "#5707A9" );
	actionColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
	prefs->ReadString( "BuddyEventColor", colTemp, defTemp = "#006E6E" );
	eventColor->SetColor( isValidColor(colTemp) ? StringToColor(colTemp) : StringToColor(defTemp) );
}

//-----------------------------------------------------

void ColorPrefView::Save() {

	// save the prefs
	if( !globalOnly ) {
		prefs->WriteString("ChatBGColor", ColorToString(bgColor->GetColor()) );
		prefs->WriteString("ChatFontColor", ColorToString(fontColor->GetColor()) );
		prefs->WriteString("ChatFromYouColor", ColorToString(fromYouColor->GetColor()) );
		prefs->WriteString("ChatToYouColor", ColorToString(toYouColor->GetColor()) );
		prefs->WriteString("IRCMeThingColor", ColorToString(actionColor->GetColor()) );
		prefs->WriteString("BuddyEventColor", ColorToString(eventColor->GetColor()) );
	}
}

//-----------------------------------------------------

void ColorPrefView::OpenColorPicker( BMessage* msg ) {

	int cid;
	BRect frame(0,0,312,110);
	BPoint lt = Window()->Frame().LeftTop();
	rgb_color* startColor;
	BString title;
	ssize_t size;

	cid = msg->FindInt32("cid");
	msg->FindData( "color", B_RGB_COLOR_TYPE, (const void**)&startColor, &size );

	switch( cid ) {
		case 1: title = BString( Language.get("PREFS_CDEFBGCOLOR") );
			break;
		case 2: title = BString( Language.get("PREFS_CDEFFONTCOLOR") );
			break;
		case 3: title = BString( Language.get("PREFS_CUNMSGFINYOU") );
			break;
		case 4: title = BString( Language.get("PREFS_CUNMSGFINYOU") );
			break;
		case 5: title = BString( Language.get("PREFS_CEVENTCOLORS") );
			break;
		case 6: title = BString( Language.get("PREFS_CIRCSTYLE") );
			break;
	}

	frame.OffsetTo( lt.x + 20, lt.y + 40 );
	ColorPickerWindow* stuff = new ColorPickerWindow( frame, title.String(), cid, Window() );
	stuff->AddToSubset( Window() );
	stuff->SetTheColor( *startColor );
	stuff->Show();
}

//-----------------------------------------------------

void ColorPrefView::SetNewColor( BMessage* msg ) {

	int cid;
	rgb_color* startColor;
	ssize_t size;

	cid = msg->FindInt32("cid");
	msg->FindData( "color", B_RGB_COLOR_TYPE, (const void**)&startColor, &size );

	switch( cid ) {

		case 1: bgColor->SetColor( *startColor );
			break;

		case 2: fontColor->SetColor( *startColor );
			break;

		case 3: fromYouColor->SetColor( *startColor );
			break;

		case 4: toYouColor->SetColor( *startColor );
			break;

		case 5: eventColor->SetColor( *startColor );
			break;

		case 6: actionColor->SetColor( *startColor );
			break;
	}
}

//-----------------------------------------------------

void ColorPrefView::RefreshLangStrings() {

	dynamic_cast<EnStringView*>(ChildAt(0))->SetText( Language.get("PREFS_CIMWCOLORS") );
	dynamic_cast<EnStringView*>(ChildAt(2))->SetText( Language.get("PREFS_CDEFBGCOLOR") );
	dynamic_cast<EnStringView*>(ChildAt(4))->SetText( Language.get("PREFS_CDEFFONTCOLOR") );
	dynamic_cast<EnStringView*>(ChildAt(6))->SetText( Language.get("PREFS_CUNMSGFINYOU") );
	dynamic_cast<EnStringView*>(ChildAt(8))->SetText( Language.get("PREFS_CUNMSGTOYOU") );
	dynamic_cast<EnStringView*>(ChildAt(10))->SetText( Language.get("PREFS_CEVENTCOLORS") );
	dynamic_cast<EnStringView*>(ChildAt(12))->SetText( Language.get("PREFS_CIRCSTYLE") );
}

//=====================================================

PrefsWindow::PrefsWindow(BRect frame, bool globalOnly)
			: SingleWindowBase(SW_PREFERENCES, frame, "BeAIM Preferences", B_TITLED_WINDOW,
							   B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS )
{
	// setup the font info
	ourFont = be_plain_font;
	ourFont.SetSize(12.0);

	// Add the basic background
	BView* bgView = new BView( Bounds(), "PrefView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE );
	bgView->SetViewColor( GetBeAIMColor(BC_NORMAL_GRAY) );
	AddChild( bgView );

	// make the new listview
	BRect listRect = BRect( 5, 5, 108, Bounds().Height() - 45 );
	panelsview = new PanelSelectorView( listRect, "list_view" );
	panelsview->SetViewColor( GetBeAIMColor(BC_WHITE) );
	panelsview->SetFontSize(11);

	// make the scrollview
	bgView->AddChild(scroll = new BScrollView("panelsview", panelsview,
			B_FOLLOW_NONE, 0, false, false, B_PLAIN_BORDER));

	// populate the list
	panelsview->AddItem( new PanelItem( "General", true, &ourFont ) );
	panelsview->AddItem( new PanelItem( "IM windows", !globalOnly, &ourFont ) );
	panelsview->AddItem( new PanelItem( "Colors", !globalOnly, &ourFont ) );
	panelsview->AddItem( new PanelItem( "Keys", !globalOnly, &ourFont ) );
	panelsview->AddItem( new PanelItem( "Connection", true, &ourFont ) );

	// Make the groovy little line
	BRect boxRect = Bounds();
	boxRect.InsetBy( 4, 0 );
	boxRect.bottom -= 38;
	boxRect.top = boxRect.bottom - 1;
	BBox* divider = new BBox(boxRect,"divider", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW, B_FANCY_BORDER);
	bgView->AddChild( divider );

	// setup a rect and make the panels
	BRect r = BRect( panelsview->Frame().right + 9, 5, Bounds().right - 1, Bounds().bottom - 41 );
	bgView->AddChild( genPrefs = new GenPrefView(r,globalOnly) );
	bgView->AddChild( chatPrefs = new ChatWindowPrefView(r,globalOnly) );
	bgView->AddChild( colorPrefs = new ColorPrefView(r,globalOnly) );
	bgView->AddChild( awayPrefs = new AwayPrefView(r,globalOnly) );
	bgView->AddChild( keyPrefs = new KeysPrefView(r,globalOnly) );
	bgView->AddChild( conPrefs = new ConnectionPrefView(r,globalOnly) );
	genPrefs->Hide();
	chatPrefs->Hide();
	colorPrefs->Hide();
	awayPrefs->Hide();
	conPrefs->Hide();
	keyPrefs->Hide();

	// Add the save and cancel buttons
	BRect buttonrect = Bounds();
	buttonrect.OffsetBy( -5, -5 );
	buttonrect.top = buttonrect.bottom - 26;
	buttonrect.left = buttonrect.right - 60;
	btnCancel = new BButton(buttonrect, "Cancel", "Cancel", new BMessage(PREFS_CANCEL), B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);
	buttonrect.right = buttonrect.left - 8;
	buttonrect.left = buttonrect.right - 60;
	btnSave = new BButton(buttonrect, "Save", "Save", new BMessage(PREFS_SAVE), B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);
	btnSave->MakeDefault( true );
	bgView->AddChild(btnSave);
	bgView->AddChild(btnCancel);

	// do the language strings and stuff
	RefreshLangStrings();

	// Show the view
	curPanel = -1;
	panelsview->Select( globalOnly ? NUM_PANELS-1 : 0 );
}

//-----------------------------------------------------

void PrefsWindow::MessageReceived( BMessage* message ) {

	switch( message->what ) {

		case BEAIM_COLORVIEW_CLICKED:
			colorPrefs->OpenColorPicker(message);
			break;

		case BEAIM_NEW_COLOR_PICKED:
			colorPrefs->SetNewColor(message);
			break;

		case PREFS_NEEDSAUTH_CHANGED:
		case PREFS_PROXYMODE_CHANGED:
			conPrefs->EnableProxyControls();
			break;

		case PREFS_MAG_SIZE_CHANGED:
			chatPrefs->MagSizeChanged();
			break;

		case PREFS_TEMP_THINGY:
			if( message->HasInt32("encoding") )
				genPrefs->encoding = (uint32)message->FindInt32("encoding" );
			break;

		case PREFS_SAVE:

			// have all the views save themselves
			genPrefs->Save();
			conPrefs->Save();
			chatPrefs->Save();
			colorPrefs->Save();
			awayPrefs->Save();
			keyPrefs->Save();

			// make sure the language is up to date
			SaveLangIfNeeded();

			// send a message to all windows telling them to reload settings
			windows->BroadcastMessage( new BMessage(BEAIM_RELOAD_PREF_SETTINGS), true );

			// ... fall thru to Close();

		case B_CANCEL:
		case PREFS_CANCEL:
			PostMessage( new BMessage(B_QUIT_REQUESTED) );
			break;

		case PREFS_RL_LANG:
			SaveLangIfNeeded(true);
			break;

		case BEAIM_REFRESH_LANG_STRINGS:
			RefreshLangStrings();
			break;

		case PREFS_SET_PROSP_LANG:
			genPrefs->prospectiveLang = BString(message->FindString("prosplang"));
			break;

		default:
			BWindow::MessageReceived( message );
	}
}

//-----------------------------------------------------

void PrefsWindow::SaveLangIfNeeded( bool force ) {

	BString curLang = BString(Language.Name());

	// if the prospective language doesn't match the current one,
	// then load the new language
	if( force || (genPrefs->prospectiveLang != curLang) ) {
		Language.SetName( (char*)genPrefs->prospectiveLang.String() );
		prefs->WriteString("Language", Language.Name(), true );
		force = true;
	}

	// if the lang changed (or is being forced), reload that bad boy
	if( force )
		windows->BroadcastMessage( new BMessage(BEAIM_REFRESH_LANG_STRINGS), true );
}

//-----------------------------------------------------

int32 PrefsWindow::ActivePanel() {
	return curPanel;
}

//-----------------------------------------------------

void PrefsWindow::SetActivePanel( int32 panel ) {

	BView* panelPtr = NULL;

	// do some validation
	if( panel < 0 )
		panel = 0;
	if( panel >= NUM_PANELS )
		panel = NUM_PANELS - 1;
	if( panel == curPanel )
		return;

	for( int32 i = 0; i < NUM_PANELS; ++i ) {

		// get a pointer to the panel at index i
		switch(i) {
			case 0:
				panelPtr = genPrefs;
				break;
			case 1:
				panelPtr = chatPrefs;
				break;
			case 2:
				panelPtr = colorPrefs;
				break;
			case 3:
				panelPtr = keyPrefs;
				break;
			case 4:
				panelPtr = conPrefs;
				break;
			default:
				panelPtr = NULL;
		}

		// if it's not supposed to be visible, hide it;
		// if it is, show it
		if( panelPtr ) {
			if( !panelPtr->IsHidden() && i != panel )
				panelPtr->Hide();
			else if( panelPtr->IsHidden() && i == panel )
				panelPtr->Show();
		}
	}
}

//-----------------------------------------------------

bool PrefsWindow::QuitRequested()
{
	BMessage* clsMessage = new BMessage(BEAIM_SINGLE_WINDOW_CLOSED);
	clsMessage->AddInt32( "wtype", SW_PREFERENCES );
	PostAppMessage( clsMessage );

	return(true);
}

//-----------------------------------------------------

// the cancel function
void PrefsWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}

	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-----------------------------------------------------

void PrefsWindow::RefreshLangStrings() {

	float wholeWidth = Bounds().Width() - 5;

	// window title
	SetTitle( Language.get("PREFERENCES") );

	// relabel and move the buttons
	btnCancel->SetLabel( Language.get("CANCEL_LABEL") );
	btnCancel->ResizeToPreferred();
	btnCancel->MoveTo( wholeWidth - btnCancel->Bounds().Width(), btnCancel->Frame().top );
	wholeWidth -= (btnCancel->Bounds().Width() + 5);
	btnSave->SetLabel( Language.get("SAVE_LABEL") );
	btnSave->ResizeToPreferred();
	btnSave->MoveTo( wholeWidth - btnSave->Bounds().Width(), btnSave->Frame().top );

	// do the panels
	((PanelItem*)panelsview->ItemAt(0))->SetName( Language.get("PREFS_PAN_GENERAL") );
	((PanelItem*)panelsview->ItemAt(1))->SetName( Language.get("PREFS_PAN_IMWIN") );
	((PanelItem*)panelsview->ItemAt(2))->SetName( Language.get("PREFS_PAN_COLORS") );
	((PanelItem*)panelsview->ItemAt(3))->SetName( Language.get("PREFS_PAN_KEYS") );
	((PanelItem*)panelsview->ItemAt(4))->SetName( Language.get("PREFS_PAN_CONNECTION") );
	panelsview->Invalidate();

	// now let each panel do its own language strings
	genPrefs->RefreshLangStrings();
	conPrefs->RefreshLangStrings();
	chatPrefs->RefreshLangStrings();
	colorPrefs->RefreshLangStrings();
	awayPrefs->RefreshLangStrings();
	keyPrefs->RefreshLangStrings();
}

//=====================================================

GSlider::GSlider( BRect frame, int32 min, int32 max, BTextControl* th, BTextView* oth )
	   : BSlider( frame, "blah", "Text Magnification:", NULL, min, max )
{
	SetValue( 100 );
	SetLimitLabels( "50%", "150%" );
	SetHashMarks( B_HASH_MARKS_BOTTOM );
	SetHashMarkCount( 5 );
	thang = th;
	otherThang = oth;

}

//-----------------------------------------------------

char* GSlider::UpdateText( void ) const {

	BFont font;

	// make a new percentage string
	char stuffage[100];
	sprintf( stuffage, "%ld%%", Value() );

	// set the text thing to the current percentage value
	if( thang )
		thang->SetText( stuffage );

	// set the correct font size in the preview thang
	float newSize = 13.0 * float(float(Value()) / 100.0);
	font.SetFamilyAndStyle( "Swis721 BT", "Roman" );
	font.SetSize( newSize );
	if (otherThang)
		otherThang->SetFontAndColor( &font );

	return NULL;
}

//=====================================================

PanelSelectorView::PanelSelectorView( BRect frame, const char* name, list_view_type type,
						uint32 resizingMode, uint32 flags ) : BListView( frame, name, type,
						resizingMode, flags )

{
}

//-----------------------------------------------------

PanelSelectorView::~PanelSelectorView() {
}

//-----------------------------------------------------

void PanelSelectorView::SelectionChanged()
{
	PrefsWindow* owner;
	int32 curSel = CurrentSelection();

	// make sure that *something* is selected
	if( curSel == -1 ) {
		Select( CountItems() - 1);
		return;
	}

	// now tell the window about the new selection
	owner = (PrefsWindow*)Window();
	owner->SetActivePanel( curSel );
}

//=====================================================

PanelItem::PanelItem( BString n, bool en, BFont* font )
		  : BListItem()
{
	name = n;
	ourFont = font;
	enabled = en;
	rightHeight = -1;
	ourFont->SetSize( 11.0 );
}

//-----------------------------------------------------

BString PanelItem::Name() {
	return name;
}

//-----------------------------------------------------

void PanelItem::DrawItem( BView *owner, BRect frame, bool complete ) {

	rgb_color color;

	// Make the selection color (shouldn't be hardcoded!)
	rgb_color kHighlight;
	kHighlight.red = kHighlight.blue = 222;
	kHighlight.green = 219;

	// Grab the owner's font, to be fiddled with if needed
	BFont ownerFont( *ourFont );

	// does the background need to be redrawn?
	frame.left++;
	if( IsSelected() || complete ) {

		// pick the appropriate background color
        if( IsSelected() ) {
        	color = kHighlight;
        	owner->SetLowColor( 222, 219, 222 );
        } else {
			color = owner->ViewColor();
			owner->SetLowColor( 255, 255, 255 );
		}

		// draw the background
		owner->SetHighColor(color);
		owner->FillRect(frame);
	}

	// Set colors and font attributes
	if( enabled )
		owner->SetHighColor(0,0,0);
	else
		owner->SetHighColor(105,106,105);

	// set the font and draw the string
	owner->SetFont( &ownerFont );
	owner->MovePenTo(frame.left + 3, frame.bottom - 3);
	owner->DrawString( name.String() );

	// reset the font and colors
	owner->SetLowColor( 255, 255, 255 );
	owner->SetHighColor( 0, 0, 0 );
}

//-----------------------------------------------------

void PanelItem::Update( BView *owner, const BFont *font ) {

	BListItem::Update(owner, font);
	if( rightHeight == -1 )
		rightHeight = Height() + 2;
	SetHeight(rightHeight);
}

//-----------------------------------------------------

void PanelItem::SetName( BString newName ) {
	name = newName;
}

//-----------------------------------------------------
