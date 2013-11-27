# Makefile for BeAIM
#
# While I like BeIDE, the makefile engine is better
NAME = BeAIM
TYPE = APP

SRCS =	BeAIM.cpp\
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
	Protocol/rsamd5.c\
	XML/hashtable.c\
	XML/xmlparse.c\
	XML/xmlrole.c\
	XML/xmltok.c

RSRCS =	BeAIM.rsrc\
	Pix.rsrc

LIBS =	be root translation textencoding tracker stdc++.r4 game media
ifeq ($(BONE), 1)
	LIBS += bind socket
else
	LIBS += net
endif


LOCAL_INCLUDE_PATHS = Headers/
WARNINGS = ALL
DEFINES = 
ifeq ($(BONE), 1)
	DEFINES += BEAIM_BONE
endif
SYMBOLS = 
ifeq ($(DEBUG), 1)
	DEBUGGER = TRUE
	DEFINES += BEAIM_DEBUG
	DEBUG =
else
	OPTIMIZE = FULL
endif
COMPILER_FLAGS = 

all: default idlefilter

include /boot/develop/etc/makefile-engine

idlefilter:
	cd "Idle Time Filter" && make -f Makefile 

test: $(TARGET)
	cp $(NAME) /boot/apps/BeAIM
	cd /boot/apps/BeAIM
	./$(NAME)
