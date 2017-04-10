TARGET = quatter

LIBS += ../Quatter/Urho3D/lib/libUrho3D.a \
    -lpthread \
    -ldl \
    -lGL

QMAKE_CXXFLAGS += -std=c++1y

INCLUDEPATH += \
    ../Quatter/Urho3D/include \
    ../Quatter/Urho3D/include/Urho3D/ThirdParty \

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    luckey.cpp \
    mastercontrol.cpp \
    inputmaster.cpp \
    quattercam.cpp \
    board.cpp \
    piece.cpp \
    master.cpp \
    effectmaster.cpp \
    square.cpp \
    yad.cpp \
    indicator.cpp

HEADERS += \
    luckey.h \
    mastercontrol.h \
    inputmaster.h \
    quattercam.h \
    board.h \
    piece.h \
    master.h \
    effectmaster.h \
    square.h \
    yad.h \
    indicator.h

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = $$PREFIX/bin
    }
    isEmpty(DATADIR) {
        DATADIR = ~/.local/share
    }
    DEFINES += DATADIR=\\\"$${DATADIR}/quatter\\\"

    target.path = $$BINDIR

    icon.files = quatter.svg
    icon.path = $$DATADIR/icons/

    pixmap.files = Resources/*
    pixmap.path = $$DATADIR/luckey/quatter/

    desktop.files = quatter.desktop
    desktop.path = $$DATADIR/applications/

    appdata.files = quatter.appdata.xml
    appdata.path = $$DATADIR/appdata/

    INSTALLS += target icon pixmap desktop appdata
}
