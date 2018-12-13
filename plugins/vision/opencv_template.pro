# Generic build profile for OpenCV based plugins
SRCNAME = $$(SRCNAME)
message( Building $$SRCNAME ...)

#include(../../main/opencv.pri)

TEMPLATE    = app
#CONFIG      = warn_on release thread
TARGET      = $$(SRCNAME)
#OBJECTS_DIR = .obj
DESTDIR = .
OBJECTS_DIR = .obj

HEADERS =
SOURCES =	$$(SRCNAME).cpp \
	../../main/tools/src/swimage_utils.cpp

# libSwPluginCore is in local lib, need it first

# Include OpenCV definitions
message("*** Include depending opencv.pri ***")
!exists(../../main/opencv.pri) {
	message("ERROR: ../../main/opencv.pri not found")
} else {
	message("    using ../../main/opencv.pri")
	include( ../../main/opencv.pri )
	message("    Done for opencv.pri: ok")
}

# If a local file exists for the plugin, add it
ADDPRI = $$(SRCNAME).pri
exists($$ADDPRI) {
	message( For src = $$SRCNAME an additional .pri exists: $$ADDPRI)
	include($$ADDPRI)
}

unix {
	message("Adding local libs for piaf library installation")
	LIBS += -L/usr/local/lib
	# If you need pg to profile your code
	#LIBS += -pg

	DEFINES += VERSION __LINUX_VERSION__
}

INCLUDEPATH += /usr/local/include/SwPlugin/
# add path to swimage_utils.h for IplImage <-> SwImage conversion
INCLUDEPATH += ../../main/tools/inc

# local include
INCLUDEPATH += ../../piaflib/inc/

# Make sure the include paths are watched for changes
#DEPENDPATH += $$INCLUDEPATH

# Add dependency to plugin library
LIBS += -lSwPluginCore

# in case the installation was not done, use local link
linux*arm*: {
	LIBS += -L../../lib/arm
} else {
	LIBS += -L../../lib/x86
}


linux-g++*: {
	TMAKE_CXXFLAGS += -g -Wall -O2 \
		-fexceptions -Wimplicit -Wreturn-type \
		-Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses  \
		-Wpointer-arith  -Wshadow
}


