// DLanguageClass
//
// A language class with a compatible interface to YLanguageClass.
//
// Copyright (c) 2003 Kyle Donaldson <gile@uselink.net>

#ifndef _DLANG_CLASS_H_
#define _DLANG_CLASS_H_

#include <storage/Path.h>
#include <string.h>
#include <stdio.h>

class DLanguageClass {
	public:
		DLanguageClass ();
		status_t InitCheck (void);
		const char * get (char *);
		const char * Name (void);
		void SetName (char *);
	private:
		void LoadLang (void);
		void AddEntry (const char *, const char *);
		char * file;
		status_t c_stat;
		char ** key;
		char ** val;
		int32 count;
};

// declared so you don't have to...
extern DLanguageClass Language;

#endif
