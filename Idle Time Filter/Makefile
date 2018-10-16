NAME = BeAIM_IdleTime_Filter.so
TARGET_DIR = ../BeAIM
TYPE = SHARED
SRCS = BeAIMIdleTimeFilter.cpp
LIBS = be
ifeq ($(DEBUG), 1)
	DEBUGGER = TRUE
endif
WARNINGS = ALL

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
