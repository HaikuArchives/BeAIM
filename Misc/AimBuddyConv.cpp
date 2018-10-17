//spaces in names
#include <storage/Path.h>
#include "AimBuddyConv.h"
#include <iostream>
#include "string.h"
#include <fstream>
#include <stdio.h>
#include <ctype.h>
#include "Say.h"

using namespace std;

//-----------------------------------------------------
// The BLT importer stuff

void IgnoreChunk( FILE* file ) {
	char c;
	int parStack = 0;

	while( (c=fgetc(file)) != EOF ) {
		if( c == '{' )
			++parStack;
		if( c == '}' ) {
			if( parStack < 1 )
				return;
			--parStack;
		}
	}
}

void ReadGroupChunk( FILE* file, group* &groopy ) {
	char c;
	int pos = 0;
	char screenName[MAX_STR];
	person* tempperson=NULL;

	while( true )
	{
		pos = 0;

		// seek up to the first letter
		while( (c=fgetc(file)) != EOF && c != '}' ) {
			if( isalpha(c) || c == '"' )
				break;
		}
		if( isalnum(c) )
			fseek( file, -1, SEEK_CUR );
		else if( c == '"' );
		else return;

		// read the screen name
		while( (c=fgetc(file)) != EOF && (c == ' ' || isalnum(c)) )
			if( pos < MAX_STR-1 )
				screenName[pos++] = c;
		screenName[pos++] = '\0';

		if (groopy->scrnname==NULL)
		{
			groopy->scrnname = new person;
			tempperson=groopy->scrnname;
			strcpy( tempperson->name, screenName );
		}
		else
		{
			tempperson->next = new person;
			tempperson = tempperson->next;
			strcpy( tempperson->name, screenName );
		}
	}

	int parStack = 0;

	while( (c=fgetc(file)) != EOF ) {
		if( c == '{' )
			++parStack;
		if( c == '}' ) {
			if( parStack < 1 )
				return;
			--parStack;
		}
	}
}

bool GetChunkName( FILE* file, char* chunkName, bool& hasChunk ) {
	int pos = 0;
	char c;

	// seek up to the first letter
	while( (c=fgetc(file)) != EOF && c != '}' && !isalpha(c) );
	if( isalpha(c) )
		fseek( file, -1, SEEK_CUR );
	else
		return false;

	// get the category name
	while( (c=fgetc(file)) != EOF && isprint(c) && c != ' ' )
		if( pos < MAX_STR-1 )
			chunkName[pos++] = c;
	chunkName[pos++] = '\0';
	if( c == EOF )
		return false;

	// keep going until the end of the line...
	while( (c=fgetc(file)) != 10 && c != 13 && c != EOF && c != '{' );
	hasChunk = false;
	if( c == '{' )
		hasChunk = true;
	if( c == EOF )
		return false;
	return true;
}

void ReadBuddyChunk( FILE* file, group* &list ) {
	char chunkName[MAX_STR];
	group* tempgroup=NULL;
	bool hasChunk;

	while( GetChunkName( file, chunkName, hasChunk ) ) {
		if( hasChunk ){
			if( strcmp(chunkName,"list") == 0 )
			{
				while( GetChunkName( file, chunkName, hasChunk ) ) {
					if( list == NULL )
					{
						list = new group;
						strcpy( list->name, chunkName );
						tempgroup=list;
					}
					else
					{
						tempgroup->next = new group;
						strcpy( tempgroup->next->name, chunkName );
						tempgroup = tempgroup->next;
					}

					if( hasChunk )
						ReadGroupChunk( file, tempgroup );
				}
			}
			else
				IgnoreChunk(file);
		}
	}
}

void ReadBLTFile( FILE* file, group* &list ) {
	char chunkName[MAX_STR];
	bool hasChunk;

	while( GetChunkName( file, chunkName, hasChunk ) ) {
		if( hasChunk ) {
			if( strcmp(chunkName, "Buddy") == 0 )
				ReadBuddyChunk(file, list);
			else
				IgnoreChunk( file );
		}
	}
}

//-----------------------------------------------------

void killfirst(char* str)
{
  int p=-1;
  while (str[++p]!='\0')
    str[p]=str[p+1];
}

bool strcomp(char one[MAX_STR], char two[MAX_STR], int upto)
{
  int p=0; upto--;
  while ((one[p]!='\0') && (one[p]==two[p]) && (p<=upto))
    p++;
  return (one[p]==two[p]);
}

void ReadLine(ifstream& input, char line[MAX_STR], bool removetabs)
{
  int p=-1;
  do {
    if ((p!=-1) && removetabs && (line[p]=='\t')) p--;
    p++;
    input.get(line[p]);
  } while ((line[p]!='\n') && input && (p<MAX_STR));
  line[p]='\0';
}


void ReadBe(ifstream& input, group* &list)
{
  int p;
  group* tempgroup=NULL;
  person* tempperson=NULL;
  char line[MAX_STR];
  while (input)
  {
    ReadLine(input, line, true);
    if (strcomp(line, "<group", 5))
    {
      if (list==NULL)
      {
        list = new group;
        tempgroup=list;
      }
      else
      {
        tempgroup->next = new group;
        tempgroup = tempgroup->next;
      }
      p=13;
      tempgroup->scrnname = NULL;
      while ((line[p]!='\"') && (line[p]!='\0'))
        tempgroup->name[p-13] = line[p++];
      tempgroup->name[p-13]='\0';
      tempgroup->next = NULL;
    }
    else if( strcomp(line,"<buddy",5) && !strcomp(line,"<buddylist",9) )
    {
      if (tempgroup->scrnname==NULL)
      {
        tempgroup->scrnname = new person;
        tempperson=tempgroup->scrnname;
      }
      else
      {
        tempperson->next = new person;
        tempperson = tempperson->next;
      }
      p=13;
      while ((line[p]!='\"') && (line[p]!='\0'))
        tempperson->name[p-13] = line[p++];
      tempperson->name[p-13]='\0';
      tempperson->next = NULL;
    }
  }
}

void ReadWin(ifstream& input, group* &list)
{
  int p;
  group* tempgroup=NULL;
  person* tempperson=NULL;
  char line[MAX_STR];
  while (input)
  {
    ReadLine(input, line, true);
    if (strcomp(line, "group", 4))
    {
      if (list==NULL)
      {
        list = new group;
        tempgroup=list;
      }
      else
      {
        tempgroup->next = new group;
        tempgroup = tempgroup->next;
      }
      p=7;
      while ((line[p]!='\"') && (line[p]!='\0'))
        tempgroup->name[p-7] = line[p++];
      tempgroup->name[p-7]='\0';
      tempgroup->next = NULL;
    }
    else if (strcomp(line, "buddy", 4))
    {
      if (tempgroup->scrnname==NULL)
      {
        tempgroup->scrnname = new person;
        tempperson=tempgroup->scrnname;
      }
      else
      {
        tempperson->next = new person;
        tempperson = tempperson->next;
      }
      p=7;
      while ((line[p]!='\"') && (line[p]!='\0'))
        tempperson->name[p-7] = line[p++];
      tempperson->name[p-7]='\0';
      tempperson->next = NULL;
    }
  }
}


void ReadGAIM(ifstream& input, group* &list)
{
  int p;
  group* tempgroup=NULL;
  person* tempperson=NULL;
  char line[MAX_STR];
  while (input)
  {
    ReadLine(input, line, true);
    if (strcomp(line, "m ", 1))
        continue;
    if (strcomp(line, "g ", 1))
    {
      if (list==NULL)
      {
        list = new group;
        tempgroup=list;
      }
      else
      {
        tempgroup->next = new group;
        tempgroup = tempgroup->next;
      }
      p=2;
      while ((line[p]!='\n') && (line[p]!='\0'))
        tempgroup->name[p-2] = line[p++];
      tempgroup->name[p-2]='\0';
      tempgroup->next = NULL;
    }
    else if (strcomp(line, "b ", 1))
    {
      if (tempgroup->scrnname==NULL)
      {
        tempgroup->scrnname = new person;
        tempperson=tempgroup->scrnname;
      }
      else
      {
        tempperson->next = new person;
        tempperson = tempperson->next;
      }
      p=2;
      while ((line[p]!='\n') && (line[p]!='\0'))
        tempperson->name[p-2] = line[p++];
      tempperson->name[p-2]='\0';
      tempperson->next = NULL;
    }
  }
}


void NukeList( group* list ) {

	group* list2;
	person *temp, *temp2;
	while (list!=NULL)
	{
		temp = list->scrnname;
		while (temp!=NULL)
		{
			temp2 = temp->next;
			delete temp;
			temp = temp2;
		}
		list2 = list->next;
		delete list;
		list = list2;
	}
}


void ImportAIMList( entry_ref& ref, group*& list, int type ) {

	BString pathski;

	// conversion vars
	ifstream input;
	list=NULL;

	// path vars
	BPath path;
	BEntry entry(&ref);
	entry.GetPath(&path);
	pathski = path.Path();

	// sanity check
	if( !entry.Exists() )
		return;

	// import the BeAIM 1.x lists
	if( type == IMPORT_TYPE_BEAIM1X ) {
		input.open( pathski.String() );
		ReadBe(input, list);
		input.close();
	}

	// import the GAIM lists
	else if( type == IMPORT_TYPE_GAIM ) {
		input.open( pathski.String() );
		ReadGAIM(input, list);
		input.close();
	}

	// import the GAIM lists
	else if( type == IMPORT_TYPE_BLT ) {
		FILE* file = fopen( pathski.String(), "r" );
		ReadBLTFile( file, list );
		fclose(file);
	}
}

