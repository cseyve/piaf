# Generic build profile for OpenCV based plugins

SRCNAME = $$(SRCNAME)
message( Building $(SRCNAME))

TEMPLATE        =       app
CONFIG          =       warn_on release thread
TARGET          =       $$(SRCNAME)
OBJECTS_DIR 	= 	.obj

# libSwPluginCore is in local lib, need it first

# Include OpenCV definitions
message("*** Include depending opencv.pri")
!exists(../../main/opencv.pri) {
	message("ERROR: ../../main/opencv.pri not found")
} else {
	include(../../main/opencv.pri)
}

# If a local file exists for the plugin, add it
ADDPRI = $$(SRCNAME).pri
exists($$ADDPRI) {
	message( For src = $$SRCNAME an additional .pri exists: $$ADDPRI)
	include($$ADDPRI)
}

unix:LIBS += -L/usr/local/lib 

#LIBS += -pg 

unix:DEFINES += VERSION __LINUX_VERSION__

INCLUDEPATH += /usr/local/include/SwPlugin/
# add path to swimage_utils.h for IplImage <-> SwImage conversion
INCLUDEPATH += ../../main/tools/inc

# local include
INCLUDEPATH += ../../piaflib/inc/


DEPENDPATH += $$INCLUDEPATH

LIBS += -L/usr/local/lib/ -lSwPluginCore

# in case the installation was not done, use local link
linux*arm*: {
	LIBS += -L../../lib/arm
} else {
	LIBS += -L../../lib/x86
}


linux-g++*:TMAKE_CXXFLAGS += -g -Wall -O2 \
	-fexceptions -Wimplicit -Wreturn-type \
	-Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses  \
	-Wpointer-arith  -Wshadow

HEADERS = 
SOURCES =	$$(SRCNAME).cpp \
	../../main/tools/src/swimage_utils.cpp

DEPENDPATH +=		.
DEPENDPATH +=		$$INCLUDEPATH

