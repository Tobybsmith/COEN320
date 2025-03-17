TEMPLATE = app

QT += qml quick
CONFIG += c++11

RESOURCES += Display.qrc

qml.files = Display.qml

launch_modeall {
	CONFIG(debug, debug|release) {
	    DESTDIR = debug
	} else {
	    DESTDIR = release
	}
}

SOURCES += Display.cpp \
           src/Display.cpp
