# this script try to locate opencv and set the right compilation options
# PG = opencv
# system(pkg-config --exists $$PG) {
# message(opencv v$$system(pkg-config --modversion $$PG)found)
# LIBS += $$system(pkg-config --libs $$PG)
# INCLUDEPATH += $$system(pkg-config --cflags $$PG | sed s/-I//g)
# } else {
# error($$PG "NOT FOUND => IT WILL NOT COMPILE")
# }
# For MacOS X

message("*** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG ")
message("Running ffmpeg.pri...............")
message("*** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG ")

LIBS_EXT = dylib

# Append avcodec
PG=libavcodec
# Version >= 56.* are not supported yet
#pkg-config --exists 'libavcodec <= 55.0.0' --print-errors
system(pkg-config --exists $$PG 'libavcodec <= 55.0.0') {

	message((ffmpeg/avcodec) $$PG v$$system(pkg-config --modversion $$PG) found)
	# Check if it is supported now: version >=56 are notsupported yet



#PG = libavcodec
#system(pkg-config --exists $$PG) {
#	unix: {
#		LIBAVC_MAJOR = $$system(pkg-config --modversion $$PG)
#	} else {
#		LIBAVC_MAJOR = $$system(pkg-config --modversion $$PG)
#	}
#	message(ffmpeg.pri: LIBAVC_MAJOR=$$LIBAVC_MAJOR)
#	message(ffmpeg.pri: $$PG v$$system(pkg-config --modversion $$PG) found)
#	LIBS += $$system(pkg-config --libs $$PG)
#	INCLUDEPATH += $$system(pkg-config --cflags $$PG | sed s/-I//g)


	DEFINES += HAS_FFMPEG
	LIBS += $$system(pkg-config --libs $$PG)
	INCLUDEPATH += $$system(pkg-config --cflags $$PG | sed s/-I//g)
} else {
	warning(ffmpeg.pri: $$PG "NOT FOUND => IT WILL NOT COMPILE")
}


# Append avformat
message("(ffmpeg) check libavformat...")
#system(pkg-config --exists $$PG) {
PG=libavformat
system(pkg-config --exists $$PG 'libavformat <= 55.9.9') {
	message((ffmpeg/avformat) $$PG v$$system(pkg-config --modversion $$PG) found)
	LIBS += $$system(pkg-config --libs $$PG)
	INCLUDEPATH += $$system(pkg-config --cflags $$PG | sed s/-I//g)


	DEFINES += HAS_AVFORMAT
} else {
	DEFINES -= HAS_FFMPEG
	warning($$PG "NOT FOUND => IT WILL NOT COMPILE")
}


# Append swscale
message("(ffmpeg) check libswscale...")
#system(pkg-config --exists $$PG) {
PG=libswscale
system(pkg-config --exists $$PG 'libswscale <= 2.9.9') {

	message((ffmpeg/swscale) $$PG v$$system(pkg-config --modversion $$PG) found)
	LIBS += $$system(pkg-config --libs $$PG)
	SWSCALEPKG = $$system(pkg-config --cflags $$PG | sed -e 's@-I@@g' )
	message(SWSCALEPKG = $$SWSCALEPKG)
	INCLUDEPATH += $$system(pkg-config --cflags $$PG | sed -e 's@-I@@g' )

	DEFINES += HAS_SWSCALE
} else {
	warning("(ffmpeg/$$PG) NOT FOUND => IT WILL NOT COMPILE")
}


message("(ffmpeg) check headers...")
linux-g++*:LIBS_EXT = so
INCLUDE_AVCODEC =
LIBSWSLIBDIR =
LIBSWSINCDIR = 
unix: {
	# Test if FFMEPG library is present
	exists( /usr/local/include/ffmpeg/avcodec.h ) {
				message("ffmpeg found in /usr/local/include.")
		INCLUDEPATH += /usr/local/include/ffmpeg
		INCLUDE_AVCODEC = /usr/local/include/ffmpeg
		
		LIBS += -L/usr/local/lib
		LIBSWSINCDIR = /usr/local/include/libswscale
		LIBSWSDIR = /usr/local/lib
	} else {
		exists( /usr/local/include/libavcodec/avcodec.h ) {
			# separated includes... damn ffmpeg daily modifications !!
			message("ffmpeg found in /usr/local/include/ffmpeg/libav*.")
			INCLUDEPATH += /usr/local/include/
                        INCLUDEPATH += /usr/local/include/libavutils

			INCLUDEPATH += /usr/local/include/libavcodec
			INCLUDEPATH += /usr/local/include/libavformat

			LIBSWSLIBDIR = /usr/local/lib
			LIBSWSINCDIR = /usr/local/include/libswscale
			LIBS += -L/usr/local/lib
		} else {
			exists( /usr/include/libavcodec/avcodec.h ) {
				message("ffmpeg found in /usr/include/libavcodec/libav*.")


				# separated includes... damn ffmpeg daily modifications !!
				INCLUDEPATH += /usr/include/libavcodec
				INCLUDEPATH += /usr/include/libavformat
				INCLUDEPATH += /usr/include/libavutils

				#INCLUDEPATH += /usr/include/ffmpeg
				LIBSWSLIBDIR = /usr/lib
				LIBSWSINCDIR = /usr/include/libswscale

				LIBS += -L/usr/lib 
			} else {
				exists( /usr/include/ffmpeg/avcodec.h ) {
					message("ffmpeg found in /usr/include.")
					INCLUDEPATH += /usr/include/ffmpeg
					LIBSWSLIBDIR = /usr/lib
					LIBSWSINCDIR = /usr/include/libswscale/
					LIBS += -L/usr/lib
				} else {
					message ( "ffmpeg NOT FOUND => IT WILL NOT COMPILE" )
				}
			}
		}
	}


	# Check if we need to link with libswscale
	exists($$LIBSWSINCDIR/swscale.h) {
		DEFINES += HAS_SWSCALE
		INCLUDEPATH += $$LIBSWSINCDIR
		LIBS += -lswscale
	}

	message("Testing if libs are installed in $$LIBSWSLIBDIR/libavcodec.$$LIBS_EXT")
	exists($$LIBSWSLIBDIR/libavcodec.$$LIBS_EXT) { 
		message("(ffmpeg) Finally we have FFMPEG for Unix!")
		DEFINES += HAS_FFMPEG
	}

	SWSCALE_H = $$LIBSWSLIBDIR/libswscale.$$LIBS_EXT
	message ( Testing SWScale lib = '$$SWSCALE_H' )

	LIBS += -lavutil -lavcodec -lavformat
}

message("(ffmpeg) check Windows...")
win32: exists("C:\Program Files\ffmpeg\include\ffmpeg") {
	DEFINES +=
	INCLUDEPATH += "C:\Program Files\ffmpeg\include\ffmpeg"
	LIBS += -L"C:\Program Files\ffmpeg\lib" \
		-L"C:\Program Files\ffmpeg\bin" \
		-lavcodec
	message("(ffmpeg) Finally we have FFMPEG for Windows!")
}

#just for DEBUG
#	DEFINES -= HAS_FFMPEG



message("FFMPEG: CONCLUSION:")
message("     - Includes : $$INCLUDEPATH")
message("END: *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG *** FFMPEG ")
