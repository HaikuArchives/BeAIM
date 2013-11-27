// DLanguageClass
//
// A language class with a compatible interface to YLanguageClass.
//
// Copyright (c) 2003 Kyle Donaldson <gile@uselink.net>
#include <stdio.h>
#include <AppKit.h>
//#include <StorageKit.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "DLanguageClass.h"

// Originally, I had planned to use STL for all of this (less code for me,
// and probably better error checking.  STL only seems to work on Dano.
// For everyone but me, STL has puked on <= R5
//
// So this is all done in the old-fashioned C style.  It's not pretty or nice,
// but it does work.

static int read_line (FILE * ifs, char *& key, char *& val)
{
	int ch = 0;
	register int i;
	key = new char[128];
	val = new char[1024];
	for (i = 0; (i < 128) && ((ch = fgetc(ifs)) != -1); i++)
		if ((ch == ' ') || (ch == '\r') || (ch == '\n'))
			break;
		else
			key[i] = ch;
	key[i] = '\0';
	if ((!strncmp(key, "//", 2)) || (ch == '\r') || (ch == '\n') || (ch == -1)) {
		if (ch != '\n')
			while (((ch = fgetc(ifs)) != '\n') && (!feof(ifs)));
		delete key;
		delete val;
		return -1;
	}
	while ((ch = fgetc(ifs)) != '"');
	for (i = 0; (i < 1024) && ((ch = fgetc(ifs)) != -1); i++)
		if ((ch == '\r') || (ch == '\n'))
			break;
		else
			val[i] = ch;
	val[i] = '\0';
	while (val[i] != '"') i--;
	val[i] = '\0';
	return 0;
}

void DLanguageClass::AddEntry (const char * k, const char * v)
{
	char ** tmp0, ** tmp1;
	tmp0 = (char **) realloc((char **) key, sizeof(char *) * (count + 2));
	tmp1 = (char **) realloc((char **) val, sizeof(char *) * (count + 2));
	if ((!tmp0) || (!tmp1))
		return;
	key = tmp0;
	val = tmp1;
	key[count] = strdup(k);
	val[count] = strdup(v);
	if ((!key[count]) || (!val[count]))
		return;
	count++;
}

// This loads the language file from the subdirectory 'Languages',
// which is in the same directory as the application that uses
// this object.
void DLanguageClass::LoadLang (void)
{
	app_info ai;
	FILE * in;
	char * k, * v;
	BPath path;
	if ((key) && (val)) {
		free((char **) key);
		free((char **) val);
		key = val = NULL;
		count = 0;
	}
	be_app->GetAppInfo(&ai);
	path = BPath(new BEntry(&ai.ref));
	path.GetParent(&path);
	path.Append("Languages");
	path.Append(file);
	in = fopen(path.Path(), "r");
	if (!in) {
		c_stat = B_ERROR;
		return;
	}
	while (!feof(in)) {
		if (read_line(in, k, v) != 0)
			continue;
		AddEntry(k, v);
		delete k;
		delete v;
	}
	fclose(in);
	c_stat = B_OK;
}

DLanguageClass::DLanguageClass ()
{
	count = 0;
	key = NULL;
	val = NULL;
	file = NULL;
}

status_t DLanguageClass::InitCheck (void)
{
	return c_stat;
}

const char * DLanguageClass::get (char * k)
{
	register int i;
	c_stat = B_OK;
	for (i = 0; i < count; i++)
		if (!strcmp(key[i], k))
			return (const char *) val[i];
	c_stat = B_ERROR;
	return k;
}

const char * DLanguageClass::Name (void)
{
	return (const char *) file;
}

void DLanguageClass::SetName (char * name)
{
	if (file)
		free(file);
	file = strdup(name);
	LoadLang();
}

DLanguageClass Language;
