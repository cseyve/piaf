# Generic build profile for OpenCV based plugins

SRCNAME = $$(SRCNAME)
message( Building $(SRCNAME))

TEMPLATE        =       app
CONFIG          =       warn_on release thread
LANGUAGE        =       C++
TARGET          =       $$(SRCNAME)
OBJECTS_DIR 	= 	.obj

# Include OpenCV definitions
include(../../main/opencv.pri)
PKG=fftw
system(pkg-config --exists $$PKG) {
	message( $$PKG v$$system(pkg-config --modversion $$PKG) found)
	LIBS += $$system(pkg-config --libs $$PKG)
	INCLUDEPATH += $$system(pkg-config --cflags $$PKG | sed s/-I//g)
	message("$$PKG found: INCLUDES=$$INCLUDEPATH")
}
else {
	unix: {
		LIBS += -L/usr/local/lib 
	
		exists(/usr/lib/libsfftw.so) {
			DEFINES += HAS_SFFTW
			LIBS += -lsfftw -lsrfftw
		}
		DEFINES += VERSION __LINUX_VERSION__
	}
}


linux*arm*: {
	LIBS += -L../../lib/arm
} else {
	LIBS += -L../../lib/x86
}
LIBS += -lSwPluginCore -lfftw3


linux-g++:TMAKE_CXXFLAGS += -g -Wall -O2 \
	-fexceptions -Wimplicit -Wreturn-type \
	-Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses  \
	-Wpointer-arith  -Wshadow

HEADERS =	
SOURCES =	$$(SRCNAME).cpp 

INCLUDEPATH += /usr/local/include/SwPlugin/
INCLUDEPATH += ../../piaflib/inc

DEPENDPATH +=		.
DEPENDPATH +=		$$INCLUDEPATH


# local link if not installed
LIBS += -L../../piaflib 


