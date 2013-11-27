#include <Application.h>
#include <Alert.h>
#include <ctype.h>
#include "HTMLStuff.h"
#include "DataSender.h"
#include "Globals.h"
#include "Say.h"

//=========================================================================

unsigned int HexStringToInt( char *cptr )
{ 
	unsigned int i, j = 0; 
	while (cptr && *cptr && isxdigit(*cptr)) 
	{ 
		i = *cptr++ - '0'; 
		if (9 < i) 
			i -= 7; 
		j <<= 4; 
		j |= (i & 0x0f); 
	}
	return(j); 
}

//=========================================================================

DataSenderWindow::DataSenderWindow( BRect frame )
	 				: BWindow(frame, "Send Arbitrary Data", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE )
{
	// Set up the view
	BRect aRect( Bounds() );
	genView = new DataSenderView( aRect );
	AddChild( genView );
	
	genView->inputData->MakeFocus(true);
}

//-------------------------------------------------------------------------

DataSenderWindow::~DataSenderWindow() {

}

//-------------------------------------------------------------------------

bool DataSenderWindow::QuitRequested()
{
	return(true);
}

//-------------------------------------------------------------------------

void DataSenderWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case INPUTDATA_INVOKED:
			DoData( false );
			break;
			
		case INPUTSTRING_INVOKED:
			DoData( true );
			break;
			
		case CLEAR_ALL:
			data = "";
			DoData( true );
			break;
			
		case B_OK:
			DoSend();
			PostMessage( new BMessage(CLEAR_ALL) );
			break;
	
		default:
			BWindow::MessageReceived(message);
	}
}

//-------------------------------------------------------------------------

// the cancel function
void DataSenderWindow::DispatchMessage( BMessage* msg, BHandler* handler ) {

	// if it's a cancel key, post a B_CANCEL message
	if( msg->what == B_KEY_DOWN )
		if( msg->HasString("bytes") && msg->FindString("bytes")[0] == B_ESCAPE ) {
			PostMessage( new BMessage(B_CANCEL) );
			return;
		}
	
	// our work here is done... dispatch normally
	BWindow::DispatchMessage( msg, handler );
}

//-------------------------------------------------------------------------

void DataSenderWindow::DoSend() {

	if( !data.length() )
		return;

	// ask for permission
	BAlert* alert = new BAlert("title", "Are you sure you want to send this data?\n\n(Make SURE you know what you're doing... this is not average-user stuff, it's debug only!)", "Nope", "Yup", NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_WARNING_ALERT);
	alert->SetShortcut( 0, B_ESCAPE );
	if( !alert->Go() )
		return;
		
	// go ahead
	BMessage* sndMessage = new BMessage(BEAIM_SEND_ARBITRARY_DATA);
	sndMessage->AddData( "data", B_RAW_TYPE, (void*)data.c_ptr(), data.length() );
	aimnet->PostMessage( sndMessage );
}

//-------------------------------------------------------------------------

void DataSenderWindow::DoData( bool which ) {

	char bigString[2048];
	char insText[50];
	char value[50];
	unsigned char theByte;
	short base = data.length();
	char* i;
	char* q;
	int l = 1;

	// get the string
	if( which ) {
		strcpy( bigString, (char*)genView->inputString->Text() );
		genView->inputString->SetText("");
	} else {
		strcpy( bigString, (char*)genView->inputData->Text() );
		genView->inputData->SetText("");
	}
	i = bigString;
	q = i;

	// do the string
	if( which ) {
		data << bigString;
		for( short f = 0; f < (short)strlen(bigString); ++f )
			isChar[base++] = true;
	}

	// do the data
	else {
		if( i ) {
			while( *i ) {
				if( *i == ',' ) {
					strncpy( value, q, l );
					value[l-1] = '\0';
					//Say( HexStringToInt(value) );
					data << (unsigned char)HexStringToInt(value);
					isChar[base] = false;
					++base;
					l = 0;
					q = i+1;
				}
				++i; ++l;
			}
			if( i != q ) {
				//Say( HexStringToInt(q) );
				data << (unsigned char)HexStringToInt(q);
				isChar[base] = false;
				++base;
			}
		}
	}

	// clear it
	genView->sendData->SetText("");
	
	// put the data into the sendData view
	for( short i = 0; i < data.length(); ++i ) {

		// get the correct insertion offset
		int32 lastline = genView->sendData->CountLines();
		int32 offset = genView->sendData->OffsetAt( lastline );	
		insText[1] = '\0';

		// get the byte and format it correctly
		theByte = data[i];
		if( isChar[i] )
			sprintf( insText, "'%c'", theByte );
		else
			sprintf( insText, "0x%X", theByte );
			
		// insert a tab, if needed
		if( offset != 1 ) {
			if( strlen(insText) < 4 )
				genView->sendData->Insert(offset, "		", 2);
			else
				genView->sendData->Insert(offset, "	", 1);
		}			
		
		// insert the data
		genView->sendData->Insert(offset, insText, strlen(insText));
	}
}

//=========================================================================

DataSenderView::DataSenderView( BRect rect )
	   	   : BView(rect, "generic_input_view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	SetViewColor( 216, 216, 216 );
	
	rgb_color gray = {100,100,100};
	rgb_color black = {0,0,0};
	BFont fixedFont = be_fixed_font;
	fixedFont.SetSize(11);
	BRect rect = Bounds();
	BRect txrect = rect;
	txrect.right -= 35;
	
	// make the rect
	rect.InsetBy( 10, 10 );
	rect.bottom = 150;
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	
	// make the send data view	
	sendData = new BTextView( rect, "text_view", txrect, B_FOLLOW_NONE, B_WILL_DRAW );
	sendDataHolder = new BScrollView( "profholder", sendData, B_FOLLOW_NONE, 0, false, true );
	sendData->MakeEditable( false );
	sendData->SetViewColor( 240,240,240 );
	sendData->SetFontAndColor( &fixedFont, B_FONT_ALL, &gray );
	AddChild( sendDataHolder );
	
	// make the add data view
	rect.top += 155;
	rect.right += B_V_SCROLL_BAR_WIDTH;
	rect.bottom = rect.top + 20;
	inputData = new BTextControl( rect, "inputData", "Hex Data:", NULL, new BMessage(INPUTDATA_INVOKED) );
	inputData->SetDivider(60);
	inputData->TextView()->SetFontAndColor( &fixedFont, B_FONT_ALL, &black );
	AddChild( inputData );	

	// make the add string view
	rect.top += 25;
	rect.bottom = rect.top + 25;
	inputString = new BTextControl( rect, "inputString", "String:", NULL, new BMessage(INPUTSTRING_INVOKED) );
	inputString->SetDivider(60);
	AddChild( inputString );
	
	// make the Clear button
	rect.top += 28;
	rect.bottom = rect.top + 25;
	rect.left = 141;
	rect.right = rect.left + 65;
	clearButton = new BButton( rect, "clearButton", "Clear", new BMessage(CLEAR_ALL) );
	AddChild( clearButton );

	// make the OK button
	rect.left += 125;
	rect.right = rect.left + 65;
	okButton = new BButton( rect, "okButton", "Send", new BMessage(B_OK) );
	AddChild( okButton );	
	
	// make the cancel button
	rect.left += 75;
	rect.right = rect.left + 65;
	cancelButton = new BButton( rect, "cancelButton", "Cancel", new BMessage(B_QUIT_REQUESTED) );
	AddChild( cancelButton );
}

//=========================================================================
