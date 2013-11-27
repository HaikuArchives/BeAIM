#ifndef MISC_STUFF_H
#define MISC_STUFF_H

#include "constants.h"
#include <String.h>
#include <View.h>
#include <String.h>
#include "DLanguageClass.h"

void GetBitmapFromResources( BBitmap*& themap, int32 resid, char *resfile = NULL );
void MakeButtonPicture( BPicture*& themap, BView* view, int32 resid, char *resfile = NULL );
void PostAppMessage( BMessage* msg );
bool isValidColor( BString );
BString ColorToString( rgb_color );
rgb_color StringToColor( BString );
BString BeAIMAppSig();
BString BeAIMVersion( bool fullString = true );
rgb_color GetBeAIMColor(beaimColor);
BString Base64Encode( BString );
void MakeElapsedTimeString( int32 elapsed, char theTime[100] );
BString ConvertToUTF8( BString, uint32 );
BString ConvertFromUTF8( BString, uint32 );
BString GetAppDir();

#define LangWithSuffix(x,y) BString( BString(Language.get(x)).Append( BString(y)) ).String()

#endif
