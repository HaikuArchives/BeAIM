#ifndef _AIM_CONV_
#define _AIM_CONV_

#include <storage/Entry.h>

#define MAX_STR 256

struct person
{
  person() {
    next = NULL;
    name[0] = '\0';
  };
  char name[MAX_STR];
  person* next;
};

struct group
{
  group() {
     next = NULL;
     scrnname = NULL;
     name[0] = '\0';
  };
  char name[MAX_STR];
  person* scrnname;
  group* next;
};

void ImportAIMList( entry_ref&, group*&, int type );
void NukeList( group* );

const int IMPORT_TYPE_GAIM = 1;
const int IMPORT_TYPE_BLT = 2;
const int IMPORT_TYPE_BEAIM1X = 3;

#endif
