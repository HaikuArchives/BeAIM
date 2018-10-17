## Haiku Generic Makefile v2.6 ##

## Fill in this file to specify the project being created, and the referenced
## Makefile-Engine will do all of the hard work for you. This handles any
## architecture of Haiku.

# The name of the binary.
NAME = BeAIM
TARGET_DIR = BeAIM

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = APP

# 	If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG =

#	The following lines tell Pe and Eddie where the SRCS, RDEFS, and RSRCS are
#	so that Pe and Eddie can fill them in for you.
#%{
# @src->@

#	Specify the source files to use. Full paths or paths relative to the
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
#	Also note that spaces in folder names do not work well with this Makefile.
SRCS = BeAIM.cpp\
	Interface/AboutBox.cpp\
	Interface/AwayEditor.cpp\
	Interface/BarberPole.cpp\
	Interface/BitmapView.cpp\
	Interface/BlockListEditor.cpp\
	Interface/BuddyList.cpp\
	Interface/BuddyListEditor.cpp\
	Interface/ChatWindow.cpp\
	Interface/CheckListItem.cpp\
	Interface/ColorPicker.cpp\
	Interface/ColorView.cpp\
	Interface/CustomAwayMsgEditor.cpp\
	Interface/FancyView.cpp\
	Interface/GStatusView.cpp\
	Interface/GenericInput.cpp\
	Interface/HTMLStuff.cpp\
	Interface/HTMLView.cpp\
	Interface/ImporterWindow.cpp\
	Interface/InfoWindow.cpp\
	Interface/LessAnnoyingWindow.cpp\
	Interface/Linkify.cpp\
	Interface/LoginBox.cpp\
	Interface/MakSplitterView.cpp\
	Interface/NameStatusView.cpp\
	Interface/PassControl.cpp\
	Interface/PeopleEdit.cpp\
	Interface/PrefsWindow.cpp\
	Interface/ProfileEditor.cpp\
	Interface/ResultsWindow.cpp\
	Interface/SingleWindowBase.cpp\
	Interface/TextElement.cpp\
	Interface/TrayIcon.cpp\
	Managers/ClientManager.cpp\
	Managers/NetManager.cpp\
	Managers/PrefsManager.cpp\
	Managers/UserManager.cpp\
	Managers/WindowManager.cpp\
	Misc/AimBuddyConv.cpp\
	Misc/DLanguageClass.cpp\
	Misc/Globals.cpp\
	Misc/MiscStuff.cpp\
	Misc/RTEngine.cpp\
	Misc/classSound.cpp\
	Misc/classSoundMaster.cpp\
	Misc/myFilter.cpp\
	Network/DataContainer.cpp\
	Network/DataSender.cpp\
	Protocol/AIMConnection.cpp\
	Protocol/AIMDataTypes.cpp\
	Protocol/AIMDecoder.cpp\
	Protocol/AIMGlobalDefs.cpp\
	Protocol/AIMInfoFunctions.cpp\
	Protocol/AIMLogin.cpp\
	Protocol/AIMNetManager.cpp\
	Protocol/AIMProtocol.cpp\
	Protocol/AIMUser.cpp\
	Protocol/AIMUserFunctions.cpp\
	Protocol/md5c.c\
	XML/hashtable.c\
	XML/xmlparse.c\
	XML/xmlrole.c\
	XML/xmltok.c

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS = BeAIM.rdef Pix.rdef

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS =

# End Pe/Eddie support.
# @<-src@
#%}

#	Specify libraries to link against.
#	There are two acceptable forms of library specifications:
#	-	if your library follows the naming pattern of libXXX.so or libXXX.a,
#		you can simply specify XXX for the library. (e.g. the entry for
#		"libtracker.so" would be "tracker")
#
#	-	for GCC-independent linking of standard C++ libraries, you can use
#		$(STDCPPLIBS) instead of the raw "stdc++[.r4] [supc++]" library names.
#
#	- 	if your library does not follow the standard library naming scheme,
#		you need to specify the path to the library and it's name.
#		(e.g. for mylib.a, specify "mylib.a" or "path/mylib.a")
LIBS = be translation textencoding tracker game media network $(STDCPPLIBS)

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS =

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.
SYSTEM_INCLUDE_PATHS =

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS = Headers

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O2), or leave blank (for the default optimization level).
OPTIMIZE := SOME

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES =

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES = BEAIM_BONE

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS =

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS :=

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER :=

#	Specify any additional compiler flags to be used.
COMPILER_FLAGS = -fpermissive

#	Specify any additional linker flags to be used.
LINKER_FLAGS =

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION :=

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH =

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
