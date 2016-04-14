# -------------------------------------------------
# Project created by QtCreator 2009-08-10T21:22:13
# -------------------------------------------------
TARGET = Colibri
QT = core gui

# For Qt5
greaterThan(QT_MAJOR_VERSION, 4): {
	QT += widgets
	QT -= network
	# for use in the code to make the difference
	DEFINES += _QT5
}



linux-g++* {
	DEFINES += _LINUX	
	UI_DIR = .ui
	MOC_DIR = .moc
	OBJ_DIR = .obj
	include(../main/ffmpeg.pri)
}
include(../main/opencv.pri)

TEMPLATE = app
LANGUAGE = C++

INCLUDEPATH += ../piaflib/inc
INCLUDEPATH += ../workflow/tools/inc

DEPENDPATH += $$INCLUDEPATH

win32 {
	message("Win32: No support for plugins")
# No support for plugins
	DEFINES += NO_PLUGIN
}

android* {
	message("Android: No support for plugins")
# No support for plugins
	DEFINES += NO_PLUGIN _ANDROID
	UI_DIR = .ui-android
	MOC_DIR = .moc-android
	OBJ_DIR = .obj-android
}

CONFIG += qt thread \
    warn_on \
    debug_and_release


linux-g++:TMAKE_CXXFLAGS += -Wall \
    -g \
    -O2 \
    -fexceptions \
    -Wimplicit \
    -Wreturn-type \
    -Wunused \
    -Wswitch \
    -Wcomment \
    -Wuninitialized \
    -Wparentheses \
    -Wpointer-arith

macx: {
    message("MacOS X specific options =================================================")
    ICON = icons/Colibri128.icns
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
}

# TARGET = $$join(TARGET,,,_debug)
# DEFINES += "TRANSLATION_DIR=\"Colibri.app/Contents/\""
linux-g++ { 
    message("Linux specific options =================================================")
    DEFINES += "TRANSLATION_DIR=/usr/share/"
}

win32:TARGET = $$join(TARGET,,d)

# }

SOURCES += src/main.cpp \
    src/colibrimainwindow.cpp \
	src/imgutils.cpp \
	../workflow/tools/src/imagewidget.cpp \
	../main/tools/src/swimage_utils.cpp \
	../main/tools/src/qimage_utils.cpp

HEADERS += inc/colibrimainwindow.h \
	inc/imgutils.h \
	../workflow/tools/inc/imagewidget.h \
	../main/tools/inc/swimage_utils.h \
	../main/tools/inc/qimage_utils.h

linux-g++* {
	HEADERS += \
		../piaflib/inc/SwPluginCore.h \
		../workflow/tools/inc/PiafFilter.h
	SOURCES += ../piaflib/src/SwPluginCore.cpp \
		../workflow/tools/src/PiafFilter.cpp \
		../main/acquisitions/video/src/swvideodetector.cpp \
		../main/tools/src/time_histogram.cpp
}

FORMS += ui/colibrimainwindow.ui
INCLUDEPATH += inc
INCLUDEPATH += .
INCLUDEPATH += ../main/inc
INCLUDEPATH += ../main/tools/inc/
INCLUDEPATH += ../main/acquisitions/video/inc/

DEPENDPATH += $$INCLUDEPATH
win32: {
	OBJECTS_DIR = obj
	MOC_DIR = moc
} else {
	OBJECTS_DIR = .obj
	MOC_DIR = .moc
}
RESOURCES += colibri.qrc

# # INSTALLATION
# target.path = /usr/local/colibri
# INSTALLS += target
# FINAL CONFIGURATION ==================================================
message( "")
message( "")
message( "FINAL CONFIGURATION ================================================== ")
message( "Configuration : ")
message( " config : $$CONFIG ")
message( " defs : $$DEFINES ")
message( " libs : $$LIBS ")
message( "FINAL CONFIGURATION ================================================== ")
message( "")
message( "")

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

