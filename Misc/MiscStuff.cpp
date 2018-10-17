#include <Bitmap.h>
#include <Alert.h>
#include <Resources.h>
#include <Roster.h>
#include <SupportKit.h>
#include <Application.h>
#include <Picture.h>
#include <ctype.h>
#include <stdlib.h>
#include "constants.h"
#include "MiscStuff.h"
#include "BitmapView.h"
#include "Globals.h"
#include "Say.h"

//-----------------------------------------------------

#define BEAIM_VERSION		"1.5.6"

// Base64 encoding table
static char table64[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//-----------------------------------------------------

void GetBitmapFromResources( BBitmap*& themap, int32 resid, char *resfile )
{
	BResources res;
   	status_t err;

   	size_t  reslength;
   	const char * resbuf;
   	BMessage archive;

   	if( !resfile )
   		resfile = AppFileName;

	// 	Open resource file
	BFile file( resfile, B_READ_ONLY );

	// if the file loads OK, that is
   	if ( (file.InitCheck() || (err = res.SetTo(&file)) ) == B_NO_ERROR )  {

		// Find and load resource
   		resbuf = (char*)res.FindResource( B_RAW_TYPE, resid, &reslength );

   		if( resbuf ) {

			// Inflate and unarchive BBitmap
	   		archive.Unflatten( resbuf );
   			themap = new BBitmap( &archive );

		} else {
	   		(new BAlert("", "Error while reading resource from file.", "OK"))->Go();
		}

   	} else {	// Error
   		(new BAlert("", "Error opening resource file.", "OK"))->Go();
   		exit(1);
   	}
}

//-----------------------------------------------------

void MakeButtonPicture( BPicture*& pic, BView* view, int32 resid, char *resfile )
{
   	if( !resfile )
   		resfile = AppFileName;

	BBitmap* bitmap = NULL;
	GetBitmapFromResources( bitmap, resid, resfile );

	//tempview for creating the picture
	BView *tempView = new BView( BRect(0,0,50,50), "temp", B_FOLLOW_NONE, B_WILL_DRAW );
	view->AddChild(tempView);

	// draw the bitmap into the BPicture
	tempView->BeginPicture(new BPicture);
	tempView->DrawBitmap(bitmap);
	pic = tempView->EndPicture();

	//get rid of tempview
	view->RemoveChild(tempView);
	delete tempView;
	delete bitmap;
}

//-----------------------------------------------------

void PostAppMessage( BMessage* msg ) {
	be_app->PostMessage( msg );
}

//-----------------------------------------------------

void GetAppPath() {
	app_info winfo;
	be_app->GetAppInfo( &winfo );
}

//-----------------------------------------------------

unsigned char HexcharToInt( char hex )
{
	// is upper-case A-F
	if ( (hex >= 0x41) && (hex <= 0x46) )
		return (unsigned char)((hex - 0x41) + 0xa);

	// is lower-case a-f
	else if ( (hex >= 0x61) && (hex <= 0x66) )
		return (unsigned char)((hex - 0x61) + 0xa);

	// is numeric digit
	else if ( (hex >= 0x30) && (hex <= 0x39) )
		return (unsigned char)(hex - 0x30);
	else
		return (unsigned char)(0);
}

//-----------------------------------------------------

BString ColorToString( rgb_color color ) {

	BString ret = "#";
	char temp[15];
	sprintf( temp, "%02X", color.red );
	ret += BString(temp);
	sprintf( temp, "%02X", color.green );
	ret += BString(temp);
	sprintf( temp, "%02X", color.blue );
	ret += BString(temp);
	return ret;
}

//-----------------------------------------------------

bool isValidColor( BString str ) {

	char c;
	if( str.Length() != 7 )
		return false;
	str = str.ToLower();
	if( str[0] != '#' )
		return false;
	for( int i = 1; i < 7; ++i ) {
		c = str[i];
		if( !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) )
			return false;
	}
	return true;
}

//-----------------------------------------------------

rgb_color StringToColor( BString str ) {

	rgb_color ret;
	ret.red = 16*HexcharToInt(str[1]) + HexcharToInt(str[2]);
	ret.green = 16*HexcharToInt(str[3]) + HexcharToInt(str[4]);
	ret.blue = 16*HexcharToInt(str[5]) + HexcharToInt(str[6]);
	return ret;
}

//-----------------------------------------------------

BString BeAIMAppSig() {
	return BString("application/x-vnd.FifthAce-BeAIM");
}

//-----------------------------------------------------

BString BeAIMVersion( bool fullString ) {
	if( fullString ) {
		BString verString = Language.get( "AB_VERSION_STRING" );
		verString.ReplaceAll( "%VER", BEAIM_VERSION );
		return verString;
	} else
		return BString(BEAIM_VERSION);
}

//-----------------------------------------------------

rgb_color GetBeAIMColor( beaimColor color ) {

	rgb_color ret;

	switch( color ) {

		case BC_NORMAL_GRAY: {
			if( beaimDebug ) {
				ret.red = 180;
				ret.green = 225;
				ret.blue = 180;
			} else {
				ret.red = ret.green = ret.blue = 216;
			}
			break;
		}

		case BC_REALLY_LIGHT_GRAY: {
			if( beaimDebug ) {
				ret.red = 235;
				ret.green = 255;
				ret.blue = 235;
			} else {
				ret.red = ret.green = ret.blue = 247;
			}
			break;
		}

		case BC_WHITE: {
			if( beaimDebug ) {
				ret.red = 235;
				ret.green = 255;
				ret.blue = 235;
			} else {
				ret.red = ret.green = ret.blue = 255;
			}
			break;
		}

		case BC_SELECTION_COLOR: {

			break;
		}


		case BC_GRAY_TEXT: {

			break;
		}
	}

	return ret;
}

//-----------------------------------------------------

BString Base64Encode( BString input ) {
	unsigned char ibuf[3];
	unsigned char obuf[4];
	int i;
	int inputparts;
	BString output;
	char* intext;

	intext = const_cast<char*>(input.String());
	while(*intext) {
		for( i = inputparts = 0; i < 3; i++ ) {
			if(*intext) {
				inputparts++;
				ibuf[i] = *intext;
				intext++;
			}
			else
				ibuf[i] = 0;
		}

		obuf[0] = (ibuf[0] & 0xFC) >> 2;
		obuf[1] = ((ibuf[0] & 0x03) << 4) | ((ibuf[1] & 0xF0) >> 4);
		obuf[2] = ((ibuf[1] & 0x0F) << 2) | ((ibuf[2] & 0xC0) >> 6);
		obuf[3] = ibuf[2] & 0x3F;

		switch(inputparts) {
			case 1:		// only one byte read
				output.Append( table64[obuf[0]], 1 );
				output.Append( table64[obuf[1]], 1 );
				output.Append( "==" );
				break;
			case 2:		// two bytes read
				output.Append( table64[obuf[0]], 1 );
				output.Append( table64[obuf[1]], 1 );
				output.Append( table64[obuf[2]], 1 );
				output.Append( "=" );
				break;
			default:
				output.Append( table64[obuf[0]], 1 );
				output.Append( table64[obuf[1]], 1 );
				output.Append( table64[obuf[2]], 1 );
				output.Append( table64[obuf[3]], 1 );
				break;
		}
	}
	return output;
}

//-----------------------------------------------------

void MakeElapsedTimeString( int32 elapsed, char theTime[100] ) {

	char stemp[100];
	int32 temp, temp2;

	theTime[0] = '\0';

	// calculate days, hours, and minutes
	temp = temp2 = int32(elapsed / 86400);
	if( temp ) {
		if( temp == 1 )
			sprintf( theTime, "%ld %s, ", temp, Language.get("DT_DAY") );
		else
			sprintf( theTime, "%ld %s, ", temp, Language.get("DT_DAYS") );
		elapsed -= (temp*86400);
	}
	temp = int32(elapsed / 3600);
	if( temp ) {
		if( temp == 1 )
			sprintf( stemp, "%ld %s", temp, Language.get("DT_HOUR") );
		else
			sprintf( stemp, "%ld %s", temp, Language.get("DT_HOURS") );
		strcat( theTime, stemp );
		elapsed -= (temp*3600);
		if( int(elapsed / 60) ) {
			strcat( theTime, ", " );
		}
	}
	temp2 = temp;
	temp = int32(elapsed / 60);
	if( temp ) {
		if( temp == 1 )
			sprintf( stemp, "%ld %s", temp, Language.get("DT_MINUTE") );
		else
			sprintf( stemp, "%ld %s", temp, Language.get("DT_MINUTES") );
		strcat( theTime, stemp );
		if( !temp2 && temp < 10 ) {
			strcat( theTime, ", " );
		}
	}
	if( !temp2 && !temp ) {
		temp = int32(elapsed);
		if( temp == 1 )
			sprintf( stemp, "%ld %s", temp, Language.get("DT_SECOND") );
		else
			sprintf( stemp, "%ld %s", temp, Language.get("DT_SECONDS") );
		strcat( theTime, stemp );
	} else if( !temp2 && temp < 10 ) {
		temp = int32( elapsed - (temp*60) );
		if( temp == 1 )
			sprintf( stemp, "%ld %s", temp, Language.get("DT_SECOND") );
		else
			sprintf( stemp, "%ld %s", temp, Language.get("DT_SECONDS") );
		strcat( theTime, stemp );
	}
}

//-----------------------------------------------------

BString ConvertToUTF8( BString text, uint32 enc ) {
	BString ret;
	int32 srcLen = text.Length();
	int32 destLen = text.Length()*2+1;
	int32 state = 0;

	if( enc == DEFAULT_ENCODING_CONSTANT )
		enc = (uint32)prefs->ReadInt32("DefaultEncoding", B_MS_WINDOWS_CONVERSION);

	char* tmpMessage = new char[destLen];
	convert_to_utf8( enc, text.String(), &srcLen, tmpMessage, &destLen, &state );
	tmpMessage[destLen] = '\0';
	ret = BString( tmpMessage );
	delete[] tmpMessage;
	return ret;
}

//-----------------------------------------------------

BString ConvertFromUTF8( BString text, uint32 enc ) {
	BString ret;
	int32 srcLen = text.Length();
	int32 destLen = text.Length()*2+1;
	int32 state = 0;

	if( enc == DEFAULT_ENCODING_CONSTANT )
		enc = (uint32)prefs->ReadInt32("DefaultEncoding", B_MS_WINDOWS_CONVERSION);

	char* tmpMessage = new char[destLen];
	convert_from_utf8( enc, text.String(), &srcLen, tmpMessage, &destLen, &state );
	tmpMessage[destLen] = '\0';
	ret = BString( tmpMessage );
	delete[] tmpMessage;
	return ret;
}

//-----------------------------------------------------

BString GetAppDir() {
	BString thisDir = BString(AppFileName);
	if( thisDir.FindLast("/") == B_ERROR ) {
		thisDir = BString("./");
	}
	else {
		int32 pos = thisDir.FindLast("/");
		thisDir.Truncate( pos+1 );
	}
	return thisDir;
}

//-----------------------------------------------------
