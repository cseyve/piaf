# Generic build profile for OpenCV based plugins

SRCNAME = $$(SRCNAME)
message( Building $(SRCNAME))

TEMPLATE        =       app
CONFIG          =       warn_on release thread
LANGUAGE        =       C++
TARGET          =       $$(SRCNAME)
OBJECTS_DIR 	= 	.obj

# Include OpenCV definitions
include(../../main-qt4/opencv.pri)


unix:LIBS += -L/usr/local/lib 

unix:DEFINES += VERSION __LINUX_VERSION__

INCLUDEPATH += /usr/local/include/SwPlugin/
LIBS += -L/usr/local/lib/ -lSwPluginCore

linux-g++:TMAKE_CXXFLAGS += -g -Wall -O2 \
	-fexceptions -Wimplicit -Wreturn-type \
	-Wunused -Wswitch -Wcomment -Wuninitialized -Wparentheses  \
	-Wpointer-arith  -Wshadow

HEADERS =	
SOURCES =	$$(SRCNAME).cpp 

DEPENDPATH +=		.
DEPENDPATH +=		$$INCLUDEPATH

